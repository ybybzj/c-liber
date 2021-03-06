#include <common/dbg.h>
#include <system/nx.h>
#include "_ev_watch_item.h"
#include "_ev_helpers.h"




ev_watch_item *ev_watch_item_create(fevent ev)
{
	ev_watch_item *w = MALLOC(1,ev_watch_item);
	check_mem(w != NULL, return NULL,"ev_watch_item_create");

	w->ev = ev;
	memset(w->cb_list, 0, sizeof(ev_callback) * _EV_EVENT_MAX);
	rb_node_init(&w->rb);
	return w;
}

void ev_watch_item_free(ev_watch_item *w)
{
	if(w == NULL)
		return;
	if((w->ev).ev_data_free != NULL)
		(w->ev).ev_data_free((w->ev).data.ptr);
	close(w->ev.fd); //close opened file descriptor
	free(w);
}

int ev_watch_item_assign_cb(ev_watch_item *w, va_list argList)
{
	check(w != NULL, return -1);

	return ev_assign_cb(w->cb_list, _EV_EVENT_MAX, argList);
}


//rbtree operations
int ev_watch_tree_add(struct rb_tree *tree, ev_watch_item *w)
{
	check(tree != NULL && w != NULL, return -1);
	struct rb_node *p, *node, *insert_node;
	p = NULL;
	node = tree->rb_root;

	insert_node = &w->rb;
	while(node != NULL)
	{
		p = node;
		if( w->ev.fd < (rb_entry(p,ev_watch_item,rb)->ev).fd)
			node = node->rb_left;
		else if( w->ev.fd > (rb_entry(p,ev_watch_item,rb)->ev).fd)
			node = node->rb_right;
		else
		{
			print_err("ev_watch node %d already exists!",w->ev.fd);
			return -1;
		}
	}

	rb_set_parent(insert_node,p);

	if(p == NULL)
		tree->rb_root = insert_node;
	else if(  w->ev.fd < (rb_entry(p,ev_watch_item,rb)->ev).fd)
		p->rb_left = insert_node;
	else
		p->rb_right = insert_node;

	rb_insert_fixup(tree, insert_node);
	return 0;
}


ev_watch_item *ev_watch_tree_search(struct rb_tree *tree, int fd)
{
	check(tree != NULL, return NULL);
	struct rb_node *node;
	node = tree->rb_root;

	while(node != NULL)
	{
		int val = (rb_entry(node,ev_watch_item,rb)->ev).fd;
		if(fd == val)
			break;
		else if(fd < val)
			node = node->rb_left;
		else
			node = node->rb_right;
	}
	return node != NULL ? rb_entry(node, ev_watch_item, rb) : NULL;
}

ev_watch_item *ev_watch_tree_delete(struct rb_tree *tree, int fd)
{
	check(tree != NULL, return NULL);
	ev_watch_item *w = ev_watch_tree_search(tree, fd);
	if(w != NULL)
	{
		struct rb_node *del_node = &w->rb;
		rb_erase(tree, del_node);
	}
	return w;
}


void ev_watch_tree_node_free(struct rb_node *node)
{
	if(node != NULL)
		ev_watch_item_free(rb_entry(node, ev_watch_item, rb));
}
