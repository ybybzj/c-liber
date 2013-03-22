#ifndef __EVENT_MONITOR_CORE_EPOLL_H__
#define __EVENT_MONITOR_CORE_EPOLL_H__
#include "_ev_handle_event.h"
#include "../_ev_watch_item.h"
#include <sys/epoll.h>

struct _ev_monitor_handle *epoll_get_monitor_handle();

void epoll_free_monitor_handle(struct _ev_monitor_handle *handle);

int epoll_add(struct _ev_monitor_handle *, ev_watch_item *);
int epoll_mod(struct _ev_monitor_handle *, ev_watch_item *, fevent);
int epoll_del(struct _ev_monitor_handle *, ev_watch_item *);

int epoll_wait_events(struct _ev_monitor_handle *, ev_handle_event *, size_t, int);

#endif
