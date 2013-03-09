#include <common/dbg.h>
#include "_ev_monitor_core.h"


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
#include "_ev_monitor_core/_timer.h"
#define ev_get_monitor_handle() epoll_get_monitor_handle()
#define ev_free_monitor_handle(h) epoll_free_monitor_handle((h))
#define ev_monitor_core_timer_prepare(e) timer_prepare((e))
#define ev_monitor_handle_add(h,w) epoll_add((h), (w))
#define ev_monitor_handle_mod(h,w,e) epoll_mod((h), (w), (e))
#define ev_monitor_handle_del(h,w) epoll_del((h), (w))
#define ev_monitor_handle_wait(h, evlist, l, timeout) epoll_wait_events((h), (evlist), (l), (timeout))
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

int ev_monitor_core_event_prepare(event *ev)
{
	check(ev != NULL, return -1);
	if(ev->events&EV_TIMEOUT)
		return ev_monitor_core_timer_prepare(ev);
	return 0;
}

int ev_monitor_core_add(ev_monitor_core *mcore,  ev_watch_item *w)
{
	check(mcore != NULL, return -1);
	check(ev_monitor_core_event_prepare(&w->ev) != -1, return -1);
	return ev_monitor_handle_add(mcore->handle, w);
}

int ev_monitor_core_del(ev_monitor_core *mcore,  ev_watch_item *w)
{
	check(mcore != NULL, return -1);
	return ev_monitor_handle_del(mcore->handle, w);
}

int ev_monitor_core_mod(ev_monitor_core *mcore,  ev_watch_item *w, event ev)
{
	check(mcore != NULL, return -1);
	return ev_monitor_handle_mod(mcore->handle, w, ev);
}



static int make_active(ev_ready_queue *rqueue, ev_handle_event *evlist, int nready)
{
	
	int i, nequeued = 0;
	for(i = 0; i< nready; i++)
	{
		if(ev_ready_queue_equeue(rqueue, evlist[i].fd, evlist[i].events) == 0)
			nequeued++;
	}
	return nequeued;
}


int ev_monitor_core_wait(ev_monitor_core *mcore, ev_ready_queue *rq, int timeout)
{
	check(mcore != NULL, return -1);
	ev_handle_event evlist[_EV_MAX_TRIGGER_EVENT];
	int nready = ev_monitor_handle_wait(mcore->handle, evlist, _EV_MAX_TRIGGER_EVENT, timeout);
	if(nready > 0)
		make_active(rq, evlist, nready);
	return nready;
}