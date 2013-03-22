#ifndef __EVENT_WATCH_ITEM_H__
#define __EVENT_WATCH_ITEM_H__
#include "event_loop.h"
#include <common/ds/rbtree.h>
#include <stdarg.h>
typedef struct _ev_watch_item
{
	fevent ev;
	ev_callback cb_list[_EV_EVENT_MAX];
	struct rb_node rb;
} ev_watch_item;

ev_watch_item *ev_watch_item_create(fevent);
void ev_watch_item_free(ev_watch_item *);
int ev_watch_item_assign_cb(ev_watch_item *, va_list);
ev_callback *ev_watch_item_cb_copy(ev_watch_item *, ev_callback *, int);

//rbtree operations
int ev_watch_tree_add(struct rb_tree *, ev_watch_item *);
ev_watch_item *ev_watch_tree_delete(struct rb_tree *, int);
ev_watch_item *ev_watch_tree_search(struct rb_tree *, int);
void ev_watch_tree_node_free(struct rb_node *node);

#endif
