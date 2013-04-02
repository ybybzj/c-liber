#ifndef __EVENT_MONITOR_CORE_H__
#define __EVENT_MONITOR_CORE_H__
typedef struct _ev_monitor_core ev_monitor_core;
#include "_ev_watch_item.h"
#include "event_loop.h"
#include "_ev_ready_queue.h"
#define EV_USE_EPOLL

ev_monitor_core *ev_monitor_core_create(ev_monitor *);
void ev_monitor_core_free(ev_monitor_core *);

int ev_monitor_core_add(ev_monitor_core *mcore,  ev_watch_item *w);
int ev_monitor_core_del(ev_monitor_core *mcore,  ev_watch_item *w);
int ev_monitor_core_mod(ev_monitor_core *mcore,  ev_watch_item *w, fevent ev);

int ev_monitor_core_notify(ev_monitor_core *mcore);
int ev_monitor_core_clear_notification(ev_monitor_core *mcore);
int ev_monitor_core_wait(ev_monitor_core *mcore, ev_ready_queue *rq, int timeout);
#endif
