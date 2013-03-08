#ifndef __EVENT_MONITOR_CORE_H__
#define __EVENT_MONITOR_CORE_H__
typedef struct _ev_monitor_core ev_monitor_core;

#include "event_loop.h"
#include "_ev_ready_queue.h"
#define EV_USE_EPOLL

ev_monitor_core *ev_monitor_core_create(ev_monitor *);
void ev_monitor_core_free(ev_monitor_core *);

int ev_monitor_core_ctl(ev_monitor_core *mcore, int opt, int fd, event ev); // opt: 'a','m','d'

int ev_monitor_core_wait(ev_monitor_core *mcore, ev_ready_queue *rq, int timeout);
#endif