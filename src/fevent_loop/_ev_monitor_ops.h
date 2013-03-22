#ifndef __EVENT_MONITOR_OPERATIONS_H__
#define __EVENT_MONITOR_OPERATIONS_H__
#include "ev_monitor.h"

#define EV_LOOP_RUN 1
#define EV_LOOP_OFF 0

void ev_monitor_set_running(ev_monitor *monitor,int run_flag);

int ev_monitor_is_running(ev_monitor *monitor);

int ev_monitor_watch_pool_is_empty(ev_monitor *monitor);
int ev_monitor_ready_queue_is_empty(ev_monitor *monitor);

int ev_monitor_wait(ev_monitor *monitor, int timeout);

int ev_monitor_dispatch(ev_monitor *monitor);

#endif
