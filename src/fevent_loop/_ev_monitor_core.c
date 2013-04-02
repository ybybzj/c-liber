#include <common/dbg.h>
#include "_ev_monitor_core.h"
#include "_ev_monitor_ops.h"
#include "_ev_ready_queue.h"

#include "_ev_monitor_core/_ev_handle_event.h"

#ifndef _EV_MAX_TRIGGER_EVENT
#define _EV_MAX_TRIGGER_EVENT 20
#endif


typedef struct _ev_monitor_handle ev_monitor_handle;

struct _ev_monitor_core
{
	ev_monitor_handle *handle;
	ev_monitor *monitor;
}; //ev_monitor_core

#ifdef EV_USE_EPOLL
#include "_ev_monitor_core/_epoll.h"
#define ev_get_monitor_handle() epoll_get_monitor_handle()
#define ev_free_monitor_handle(h) epoll_free_monitor_handle((h))
#define ev_monitor_handle_add(h,w) epoll_add((h), (w))
#define ev_monitor_handle_mod(h,w,e) epoll_mod((h), (w), (e))
#define ev_monitor_handle_del(h,w) epoll_del((h), (w))
#define ev_monitor_handle_wait(h, evlist, l, timeout) epoll_wait_events((h), (evlist), (l), (timeout))
#define ev_monitor_handle_notify(h) epoll_notify((h))
#define ev_monitor_handle_clear_notification(h) epoll_clear_notification((h))
#endif

ev_monitor_core *ev_monitor_core_create(ev_monitor *monitor)
{
	ev_monitor_core *mcore = MALLOC(1,ev_monitor_core);
	check_mem(mcore != NULL, return NULL,"ev_monitor_core allocation");

	check((mcore->handle = ev_get_monitor_handle()) != NULL, free(mcore); return NULL);

	mcore->monitor = monitor;

	return mcore;
}

void ev_monitor_core_free(ev_monitor_core *mcore)
{
	if(mcore != NULL)
	{
		ev_free_monitor_handle(mcore->handle);
		free(mcore);
	}
}

int ev_monitor_core_add(ev_monitor_core *mcore,  ev_watch_item *w)
{
	check(mcore != NULL && w != NULL, return -1);

	return ev_monitor_handle_add(mcore->handle, w);
}

int ev_monitor_core_del(ev_monitor_core *mcore,  ev_watch_item *w)
{
	check(mcore != NULL && w != NULL, return -1);

	return ev_monitor_handle_del(mcore->handle, w);
}

int ev_monitor_core_mod(ev_monitor_core *mcore,  ev_watch_item *w, fevent ev)
{
	check(mcore != NULL && w != NULL, return -1);

	return ev_monitor_handle_mod(mcore->handle, w, ev);
}



static int make_active(ev_ready_queue *rqueue, ev_handle_event *evlist, int nready)
{

	int i, nequeued = 0;
	for(i = 0; i< nready; i++)
	{
		if(evlist[i].fd != -1 &&ev_ready_queue_wait_equeue(rqueue, evlist[i].fd, evlist[i].events) == 0)
			nequeued++;
	}
	return nequeued;
}

int ev_monitor_core_notify(ev_monitor_core *mcore)
{
	return ev_monitor_handle_notify(mcore->handle);
}

int ev_monitor_core_clear_notification(ev_monitor_core *mcore)
{
	return ev_monitor_handle_clear_notification(mcore->handle);
}

int ev_monitor_core_wait(ev_monitor_core *mcore, ev_ready_queue *rq, int timeout)
{
	check(mcore != NULL, return -1);
	ev_handle_event evlist[_EV_MAX_TRIGGER_EVENT];
	int nready = ev_monitor_handle_wait(mcore->handle, evlist, _EV_MAX_TRIGGER_EVENT, timeout);
	make_active(rq, evlist, nready);
	// if(ev_ready_queue_is_empty(rq))
	// 	debug_R("[ev_monitor_handle_wait] rq is empty!!!");
	return nready < 0 ? -1 : ev_ready_queue_nready(rq);
}
