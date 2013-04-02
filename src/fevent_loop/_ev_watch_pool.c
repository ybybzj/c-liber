#include "_ev_watch_pool.h"

#include "_ev_monitor_ops.h"
#include "_ev_monitor_core.h"
#include "_ev_ready_queue.h"
#include <common/dbg.h>
#include <common/ds/rbtree.h>
#include <system/nx.h>
#include <pthread.h>

struct _ev_watch_pool
{
	ev_monitor *monitor;
	pthread_mutex_t mtx;
	struct rb_tree watch_tree;
}; //ev_watch_pool

ev_watch_pool *ev_watch_pool_create(ev_monitor* monitor)
{
	check(monitor != NULL, errno = EINVAL; return NULL);

	ev_watch_pool *wp = MALLOC(1, ev_watch_pool);
	check_mem( wp != NULL, return NULL, "ev_watch_pool allocation");

	check_t(pthread_mutex_init(&wp->mtx,NULL), free(wp); return NULL);
	wp->monitor = monitor;

	wp->watch_tree = (struct rb_tree){NULL};

	return wp;
}



void ev_watch_pool_free(ev_watch_pool *wp)
{
	if(wp != NULL)
	{
		(void)pthread_mutex_lock(&wp->mtx);
		rb_destroy(&wp->watch_tree, ev_watch_tree_node_free);
		(void)pthread_mutex_unlock(&wp->mtx);

		pthread_mutex_destroy(&wp->mtx);

		free(wp);
	}
}

int ev_watch_pool_is_empty(ev_watch_pool *wp)
{
	return RB_IS_EMPTY_TREE(&wp->watch_tree);
}

int ev_watch_pool_add(ev_watch_pool *wp, fevent ev, va_list argList)
{
	check(wp != NULL, errno=EINVAL; return -1);

	ev_watch_item *w = ev_watch_item_create(ev);
	check(w != NULL, return -1);

	(void)ev_watch_item_assign_cb(w, argList);
	int ret = 0;
	(void)pthread_mutex_lock(&wp->mtx);
		check(ev_watch_tree_add(&wp->watch_tree, w) != -1,
			ev_watch_item_free(w);
			ret = -1);
	(void)pthread_mutex_unlock(&wp->mtx);


	check(ev_monitor_core_add(wp->monitor->mcore, w) != -1, goto onerr);

	return ret;
onerr:
	(void)pthread_mutex_lock(&wp->mtx);
		ev_watch_tree_delete(&wp->watch_tree, ev.fd);
	(void)pthread_mutex_unlock(&wp->mtx);
	ev_watch_item_free(w);
	return -1;
}
int ev_watch_pool_del(ev_watch_pool *wp, int fd)
{
	check(wp != NULL, errno=EINVAL; return -1);
	int ret = 0;
	ev_watch_item *w;

	(void)pthread_mutex_lock(&wp->mtx);
	w = ev_watch_tree_delete(&wp->watch_tree, fd);
	(void)pthread_mutex_unlock(&wp->mtx);

	if(w != NULL)
	{
		check(ev_monitor_core_del(wp->monitor->mcore, w) != -1, ret = -1);
		ev_watch_item_free(w);
	}
	return ret;
}

int ev_watch_pool_mod(ev_watch_pool *wp, fevent ev, va_list argList)
{
	check(wp != NULL, errno=EINVAL; return -1);

	ev_watch_item *w;

	(void)pthread_mutex_lock(&wp->mtx);
	w = ev_watch_tree_search(&wp->watch_tree, ev.fd);
	(void)pthread_mutex_unlock(&wp->mtx);

	if(w == NULL) return 0;


	check(ev_monitor_core_mod(wp->monitor->mcore, w, ev) != -1, return -1);


	(void)pthread_mutex_lock(&wp->mtx);
		w->ev.events = ev.events;
		(void)ev_watch_item_assign_cb(w, argList);
	(void)pthread_mutex_unlock(&wp->mtx);


	return 0;
}

ev_watch_item *ev_watch_pool_search(ev_watch_pool *wp, int fd)
{
	check(wp != NULL, errno = EINVAL; return NULL);
	ev_watch_item *w;
	(void)pthread_mutex_lock(&wp->mtx);
	w = ev_watch_tree_search(&wp->watch_tree, fd);
	(void)pthread_mutex_unlock(&wp->mtx);

	return w;
}
