#include "_ev_monitor_ops.h"
#include "_ev_monitor_core.h"
#include "_ev_watch_pool.h"
#include "_ev_ready_queue.h"
#include <common/dbg.h>
#include <system/nx.h>
#include <pthread.h>
#include <stdarg.h>

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

int ev_monitor_ctl_f(ev_monitor *monitor, int flag, fevent ev, ...)
{
	check(monitor != NULL, return -1);
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


//internal use monitor functions, don't need to check monitor != NULL
void ev_monitor_set_running(ev_monitor *monitor, int run_flag)
{
	(void)pthread_mutex_lock(&monitor->run_mtx);
	monitor->running = run_flag;
	(void)pthread_mutex_unlock(&monitor->run_mtx);
}



int ev_monitor_is_running(ev_monitor *monitor)
{
	return monitor->running != EV_LOOP_OFF;
}

int ev_monitor_watch_pool_is_empty(ev_monitor *monitor)
{
	return ev_watch_pool_is_empty(monitor->wpool);
}
int ev_monitor_ready_queue_is_empty(ev_monitor *monitor)
{
	return ev_ready_queue_is_empty(monitor->rqueue);
}

int ev_monitor_wait(ev_monitor *monitor, int timeout)
{
	return ev_monitor_core_wait(monitor->mcore, monitor->rqueue, timeout);
}

int ev_monitor_dispatch(ev_monitor *monitor)
{
	return ev_ready_queue_dispatch(monitor->rqueue);
}
