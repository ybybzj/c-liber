#ifndef __EVENT_READY_QUEUE_H__
#define __EVENT_READY_QUEUE_H__
typedef struct _ev_ready_queue ev_ready_queue;

#include "event_loop.h"
#include "_ev_watch_pool.h"
#include <stdarg.h>

ev_ready_queue *ev_ready_queue_create(ev_monitor *);
void ev_ready_queue_free(ev_ready_queue *);

int ev_ready_queue_is_empty(ev_ready_queue *);

int ev_ready_queue_wait_equeue(ev_ready_queue *, int, ev_set);

int ev_ready_queue_make_ready(ev_ready_queue *, fevent, va_list);

int ev_ready_queue_dispatch(ev_ready_queue *);

int ev_ready_queue_is_running(ev_ready_queue *);

int ev_ready_queue_nready(ev_ready_queue *rq);
#endif
