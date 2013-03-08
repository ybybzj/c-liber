#ifndef __EVENT_WATCH_POOL_H__
#define __EVENT_WATCH_POOL_H__
typedef struct _ev_watch_pool ev_watch_pool;
#include "event_loop.h"
#include "_ev_monitor_core.h"
#include "_ev_watch_item.h"
#include <stdarg.h>

ev_watch_pool *ev_watch_pool_create(ev_monitor_core *);
void ev_watch_pool_free(ev_watch_pool *);

int ev_watch_pool_is_empty(ev_watch_pool *);

int ev_watch_pool_add(ev_watch_pool *,event, va_list);
int ev_watch_pool_del(ev_watch_pool *, int);
int ev_watch_pool_mod(ev_watch_pool *,event, va_list);

ev_watch_item *ev_watch_pool_search(ev_watch_pool *, int);
#endif