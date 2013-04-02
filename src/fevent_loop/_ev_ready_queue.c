#include <common/dbg.h>
#include "_ev_ready_queue.h"
#include "_ev_ready_item.h"
#include "_ev_monitor_ops.h"
#include "_ev_watch_item.h"
#include <common/ds/dlist.h>
#include <system/nx.h>
// #include <system/nx/pthread_pool.h>
// #include <system/nx/thread_pool/thread_pool.h>
#ifndef _EV_TIMEOUT
#define _EV_THREAD_MAX_NUM 100
#endif

struct _ev_ready_queue
{
	ev_monitor *monitor;
	// ev_watch_pool *wpool;
	pthread_mutex_t mtx;
	list_head rqueue[EV_PRIORITY_MAX];
	thr_pool *tp;
}; //ev_ready_queue



/*--------------ev_ready_queue operations-----------------------------*/
ev_ready_queue *ev_ready_queue_create(ev_monitor *monitor)
{
	ERR_CLEAN_INIT();
	check(monitor != NULL, return NULL);
	ev_ready_queue *rq = MALLOC(1, ev_ready_queue);
	check_mem(rq != NULL, return NULL, "ev_ready_queue_create");
	ERR_CLEAN_PUSH(0,rq,rq);
	rq->monitor = monitor;

	check_t(pthread_mutex_init(&rq->mtx,NULL), goto onerr);
	ERR_CLEAN_PUSH(1,&rq->mtx,rq_mtx);

	check((rq->tp = tp_create(0, _EV_THREAD_MAX_NUM, -1, NULL)) != NULL , goto onerr);

	int i;
	for(i = 0; i< EV_PRIORITY_MAX; i++)
	{
		list_head *head = &(rq->rqueue[i]);
		INIT_LIST_HEAD(head);
	}
	return rq;
onerr:
	ERR_CLEAN_BEGIN
		case 0:
			{
				free((ev_ready_queue*)cleanup_resource);
				break;
			}
		case 2:
			{
				pthread_mutex_destroy((pthread_mutex_t*)cleanup_resource);
				break;
			}
		ERR_CLEAN_END
		return NULL;
}

void ev_ready_queue_free(ev_ready_queue *rq)
{
	if(rq != NULL)
	{
		tp_wait(rq->tp);
		tp_destroy(rq->tp);

		(void)pthread_mutex_lock(&rq->mtx);
		int i = 0;
		for(; i < EV_PRIORITY_MAX; i++)
			while(!list_empty(&rq->rqueue[i]))
			{
				ev_ready_item *ri = list_entry(list_shift(&rq->rqueue[i]), ev_ready_item, l_ent);
				ev_ready_item_free(ri);
			}
		(void)pthread_mutex_unlock(&rq->mtx);

		pthread_mutex_destroy(&rq->mtx);

		free(rq);
	}
}


static int ev_ready_queue_highest_priority(ev_ready_queue *rq)
{

	int i = 0;
	(void)pthread_mutex_lock(&rq->mtx);
	for(; i < EV_PRIORITY_MAX; i++)
	{
		list_head *head = &(rq->rqueue[i]);
		if( !list_empty(head))
			break;
	}
	(void)pthread_mutex_unlock(&rq->mtx);
	return i != EV_PRIORITY_MAX ? i : -1;
}


int ev_ready_queue_is_empty(ev_ready_queue *rq)
{
	return (rq!= NULL) && (ev_ready_queue_highest_priority(rq) == -1);
}

int ev_ready_queue_wait_equeue(ev_ready_queue *rq, int fd, ev_set events)
{
	check(rq != NULL, errno = EINVAL; return -1);
	ev_watch_item *w = ev_watch_pool_search(rq->monitor->wpool, fd);


	check(w != NULL ,return -1);
	ev_ready_item *ri = ev_ready_item_create(rq->monitor, w->cb_list, _EV_EVENT_MAX, w->ev, events);

	check(ri != NULL, return -1);

	int priority = ri->ev.priority;
	list_head *ready_list = &(rq->rqueue[priority]);

	(void)pthread_mutex_lock(&rq->mtx);
	list_push(&ri->l_ent, ready_list);
	(void)pthread_mutex_unlock(&rq->mtx);
	return 0;
}


static int ev_ready_queue_make_ready_ri(ev_ready_queue *rq, ev_ready_item *ri)
{
	check(rq != NULL && ri != NULL, return -1);
	int priority = ri->ev.priority;
	list_head *ready_list = &(rq->rqueue[priority]);
	(void)pthread_mutex_lock(&rq->mtx);
	list_push(&ri->l_ent, ready_list);
	(void)ev_monitor_change_notify(rq->monitor);
	(void)pthread_mutex_unlock(&rq->mtx);
	return 0;
}

int ev_ready_queue_make_ready(ev_ready_queue *rq, fevent ev, va_list argList)
{

	check(rq != NULL, errno = EINVAL; return -1);
	ev_ready_item *ri = ev_ready_item_create(rq->monitor, NULL, _EV_EVENT_MAX, ev, ev.events);
	check(ri != NULL, return -1);
	check(ev_ready_item_assign_cb(ri, argList) != -1, ev_ready_item_free(ri); return -1);

	return ev_ready_queue_make_ready_ri(rq, ri);
}


static int ev_ready_queue_dqueue(ev_ready_queue *rq, int priority, ev_ready_item **out)
{
	check(rq != NULL && out != NULL, errno=EINVAL; return -1);

	list_head *ready_list = &(rq->rqueue[priority]);
	if(list_empty(ready_list))
	{
		*out = NULL;
		return 0;
	}

	(void)pthread_mutex_lock(&rq->mtx);
	struct list_node *l_ent = list_shift(ready_list);
	(void)pthread_mutex_unlock(&rq->mtx);

	*out = list_entry(l_ent, ev_ready_item, l_ent);
	return 1;
}


static void event_handler(void *arg)
{
	ev_ready_item *ri = (ev_ready_item*)arg;
	if(ri == NULL) return ;
	fevent ev = ri->ev;
	ev_monitor *monitor = ri->monitor;
	ev.events = ev_ready_item_dispatch(ri);
	ri->ev = ev;
	if(ev.events&EV_FIN)
	{
		check(ev_monitor_ctl(monitor, EV_CTL_DEL, ev) != -1,goto clean_up);
	}else if(ev.events != 0)
	{

		if(ev.events&EV_ASYNC)
		{
			check(ev_ready_queue_make_ready_ri(monitor->rqueue, ri) != -1, goto clean_up);
			return ;
		}else{
			check(ev_monitor_ctl(monitor, EV_CTL_MOD, ev) != -1, goto clean_up);
		}
	}
clean_up:
	ev_ready_item_free(ri);
	return ;
}

int ev_ready_queue_dispatch(ev_ready_queue *rq)
{
	check(rq != NULL, errno = EINVAL; return -1);

	int hpriority = ev_ready_queue_highest_priority(rq);
	if(hpriority == -1)
		return 0;

	ev_ready_item *ri;
	int i = 0;
	while(ev_ready_queue_dqueue(rq, hpriority, &ri) == 1)
	{
		tp_queue(rq->tp, event_handler, (void*)ri);
		i++;
	}
	(void)ev_monitor_clear_notification(rq->monitor);
	return 0;

}



int ev_ready_queue_is_running(ev_ready_queue *rq)
{
	return tp_is_busy(rq->tp);
}

int ev_ready_queue_nready(ev_ready_queue *rq)
{
	int i = 0;
	int nready = 0;
	(void)pthread_mutex_lock(&rq->mtx);
	for(; i < EV_PRIORITY_MAX; i++)
	{
		list_head *head = &(rq->rqueue[i]);
		if( !list_empty(head))
		{
			nready = list_size(head);
			break;
		}
	}
	(void)pthread_mutex_unlock(&rq->mtx);
	return nready;
}
