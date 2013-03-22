
#include "event_loop.h"
#include "_ev_monitor_ops.h"
#include "_ev_events/_timeout.h"
#include "_ev_events/_signal.h"
#include <common/dbg.h>
#include <system/nx.h>

//default settings
#ifndef _EV_TIMEOUT
#define _EV_TIMEOUT 5 //unit: second
#endif


fevent ev_new_event()
{
	fevent ev;
	memset(&ev, 0, sizeof(fevent));
	ev.priority = 1;
	return ev;
}



int ev_loop_run(ev_monitor *monitor ,int flag)
{
	check(monitor != NULL, errno= EINVAL; return -1);
	check_err(!ev_monitor_is_running(monitor), return -1, "event_loop already running!");

	ev_monitor_set_running(monitor, EV_LOOP_RUN);

	int nready;
	while((!ev_monitor_watch_pool_is_empty(monitor) || flag&EV_EMPTY_NOEXIT) && ev_monitor_is_running(monitor))
	{
		int aq_empty = ev_monitor_ready_queue_is_empty(monitor);
		errno = 0;
		if(flag&EV_NOBLOCK || !aq_empty)
		{

			nready = ev_monitor_wait(monitor, 0);

		}else
		{
			nready = ev_monitor_wait(monitor, _EV_TIMEOUT);

			if(nready == 0)
			{
				debug("epoll_wait timeout!");
				continue;
			}
		}

		if(nready <= 0)
		{
			if(nready == 0)
			{
				if(aq_empty)
				continue;
			}else
			{
				if(errno == EINTR)
				{
					println("[epoll_wait] signal caught!!!");
					continue;
				}
				else
				{
					print_err("epoll_wait failed!");
					goto onerr;
				}
			}

		}

		ev_monitor_dispatch(monitor);
	}

	println("ev_loop_run exit!");
	return 0;

onerr:

	ev_loop_stop(monitor);
	return -1;

}

void ev_loop_stop(ev_monitor *monitor)
{
	if(monitor == NULL) return;

	ev_monitor_set_running(monitor,EV_LOOP_OFF);
}



int ev_set_timeout( ev_monitor *monitor,
					int priority,
					int delay,
					int (*tm_cb) (void*),
					void *arg,
					void (*arg_free)(void*))
{
	check(monitor != NULL, return -1);
	return add_timeout_ev( monitor,
						   priority,
						   delay,
						   tm_cb,
						   arg,
						   arg_free,
						   EV_TIMEOUT_ONCE);
}

int ev_set_interval( ev_monitor *monitor,
					int priority,
					int delay,
					int (*tm_cb) (void*),
					void *arg,
					void (*arg_free)(void*))
{
	check(monitor != NULL, return -1);

	return add_timeout_ev( monitor,
						   priority,
						   delay,
						   tm_cb,
						   arg,
						   arg_free,
						   EV_TIMEOUT_PERIOD);
}


int ev_on_signal(ev_monitor *monitor,
					int priority,
					int signal,					// expire time
					int (*sig_cb) (int,void*),       // signal callback return 0 when done with signal catch
					void *arg,					// argument passed to callback
					void (*arg_free)(void*))
{
	check(monitor != NULL, return -1);

	return add_signal_ev( monitor,
						 priority,
						 signal,
						 sig_cb,
						 arg,
					     arg_free);

}
