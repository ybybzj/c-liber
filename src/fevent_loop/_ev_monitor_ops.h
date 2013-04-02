#ifndef __EVENT_MONITOR_OPERATIONS_H__
#define __EVENT_MONITOR_OPERATIONS_H__
#include "ev_monitor.h"
#include "_ev_monitor_core.h"
#include "_ev_watch_pool.h"
#include "_ev_ready_queue.h"
#include <pthread.h>
struct _ev_monitor{
	ev_monitor_core *mcore;
	ev_watch_pool *wpool;
	ev_ready_queue *rqueue;
	int running;
	pthread_mutex_t run_mtx;

}; // ev_monitor

#define EV_LOOP_RUN 1
#define EV_LOOP_OFF 0

void ev_monitor_set_running(ev_monitor *monitor,int run_flag);

int ev_monitor_is_running(ev_monitor *monitor);

int ev_monitor_watch_pool_is_empty(ev_monitor *monitor);
int ev_monitor_ready_queue_is_empty(ev_monitor *monitor);
int ev_monitor_ready_queue_is_running(ev_monitor *monitor);
int ev_monitor_wait(ev_monitor *monitor, int timeout);
int ev_monitor_change_notify(ev_monitor *monitor);
int ev_monitor_clear_notification(ev_monitor *monitor);
int ev_monitor_dispatch(ev_monitor *monitor);

#endif
