#ifndef __EVENT_READY_ITEM_H__
#define __EVENT_READY_ITEM_H__
#include "event_loop.h"
#include "_ev_watch_item.h"
#include <common/ds/dlist.h>

typedef struct _ev_ready_item
{
	ev_monitor *monitor;
	ev_watch_item *w;
	event ev;
	dl_ent_t l_ent;
}ev_ready_item;




ev_ready_item *ev_ready_item_create(ev_monitor *, ev_watch_item*, ev_set);

void ev_ready_item_free(ev_ready_item*);

ev_set ev_ready_item_dispatch(ev_ready_item*);


#endif