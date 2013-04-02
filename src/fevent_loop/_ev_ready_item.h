#ifndef __EVENT_READY_ITEM_H__
#define __EVENT_READY_ITEM_H__
#include "event_loop.h"
// #include "_ev_watch_item.h"
#include <common/ds/dlist.h>
#include <stdarg.h>

typedef struct _ev_ready_item
{
	ev_monitor *monitor;
	ev_callback cb_list[_EV_EVENT_MAX];
	fevent ev;
	dl_ent_t l_ent;
}ev_ready_item;




ev_ready_item *ev_ready_item_create(ev_monitor *, ev_callback*, int cb_len, fevent ev, ev_set);

void ev_ready_item_free(ev_ready_item*);

ev_set ev_ready_item_dispatch(ev_ready_item*);

int ev_ready_item_assign_cb(ev_ready_item*, va_list);

#endif
