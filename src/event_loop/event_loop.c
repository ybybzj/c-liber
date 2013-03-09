#include "event_loop.h"
#include "event_loop/_ev_monitor_core.h"
#include "event_loop/_ev_watch_pool.h"
#include "event_loop/_ev_ready_queue.h"

#include <common/dbg.h>
#include <system/nx.h>
#include <pthread.h>
#include <stdarg.h>

//default settings
#ifndef _EV_TIMEOUT
#define _EV_TIMEOUT 5 //unit: second
#endif



struct _ev_monitor{
	ev_monitor_core *mcore;
	ev_watch_pool *wpool;
	ev_ready_queue *rqueue;
	int running;
	pthread_mutex_t run_mtx;

}; // ev_monitor

ev_monitor *ev_monitor_create()
{
	ERR_CLEAN_INIT();

	ev_monitor *monitor = (ev_monitor*)malloc(sizeof(ev_monitor));
	check_err(monitor != NULL,return NULL,"memory error!");
	ERR_CLEAN_PUSH(0,monitor,mntr);

	monitor->running = 0;

	check((monitor->mcore = ev_monitor_core_create(monitor)) != NULL, goto onerr);
	ERR_CLEAN_PUSH(1,monitor->mcore,mcore);

	check_t(pthread_mutex_init(&monitor->run_mtx,NULL), goto onerr);
	ERR_CLEAN_PUSH(2,&monitor->run_mtx,run_mtx);

	check((monitor->wpool = ev_watch_pool_create(monitor->mcore)) != NULL, goto onerr);
	ERR_CLEAN_PUSH(3,monitor->wpool,wpool);

	check((monitor->rqueue = ev_ready_queue_create(monitor, monitor->wpool)) != NULL, goto onerr);



	return monitor;

onerr:
	ERR_CLEAN_BEGIN
		case 0:
			{
				free((ev_monitor*)cleanup_resource);
				break;
			}
		case 1:
			{
				ev_monitor_core_free((ev_monitor_core*)cleanup_resource);
				break;
			}
		case 2:
			{
				pthread_mutex_destroy((pthread_mutex_t*)cleanup_resource);
				break;
			}
		case 3:
		{
			ev_watch_pool_free((ev_watch_pool*)cleanup_resource);
			break;
		}
		
		ERR_CLEAN_END
		return NULL;
}


int ev_monitor_ctl_f(ev_monitor *monitor, int flag, event ev, ...)
{
	int ret = 0;
	va_list argList;
	va_start(argList,ev);
	
	switch(flag)
	{
		case EV_CTL_ADD:
		{
			check(ev.priority >= 0 && ev.priority < EV_PRIORITY_MAX, errno = EINVAL; ret = -1; break);
			ret = ev_watch_pool_add(monitor->wpool,ev,argList);
			break;
		}
		case EV_CTL_DEL:
		{
			ret = ev_watch_pool_del(monitor->wpool, ev.fd);
			break;
		}
		case EV_CTL_MOD:
		{
			if(!(ev.events & EV_IGN))
				check(ev.priority >= 0 && ev.priority < EV_PRIORITY_MAX, errno = EINVAL; ret = -1; break);
			ret = ev_watch_pool_mod(monitor->wpool,ev,argList);
			break;
		}
		default:
		{
			errno = EINVAL;
			print_err("unknown operation for event monitor!");
			ret = -1;
			break;
		}
	}
	va_end(argList);
	return ret;
}


int ev_loop_run(ev_monitor *monitor ,int flag)
{
	check(monitor != NULL, errno= EINVAL; return -1);
	check_err(monitor->running == 0, return -1, "event_loop already running!");

	(void)pthread_mutex_lock(&monitor->run_mtx);
	monitor->running = 1;
	(void)pthread_mutex_unlock(&monitor->run_mtx);

	int nready;
	while((!ev_watch_pool_is_empty(monitor->wpool)|| flag&EV_EMPTY_NOEXIT) && monitor->running)
	{
		int aq_empty = ev_ready_queue_is_empty(monitor->rqueue);
		errno = 0;
		if(flag&EV_NOBLOCK || !aq_empty)
		{
			
			nready = ev_monitor_core_wait(monitor->mcore, monitor->rqueue, 0);

		}else
		{
			nready = ev_monitor_core_wait(monitor->mcore, monitor->rqueue, _EV_TIMEOUT);

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

		ev_ready_queue_dispatch(monitor->rqueue);
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

	(void)pthread_mutex_lock(&monitor->run_mtx);
	monitor->running = 0;
	(void)pthread_mutex_unlock(&monitor->run_mtx);
}



void ev_monitor_free(ev_monitor *monitor)
{
	if(monitor == NULL) return;

	ev_loop_stop(monitor);
	pthread_mutex_destroy(&monitor->run_mtx);
	ev_ready_queue_free(monitor->rqueue);
	ev_watch_pool_free(monitor->wpool);
	ev_monitor_core_free(monitor->mcore);

	free(monitor);
}