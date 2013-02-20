#ifndef __DLIST_H__
#define __DLIST_H__
/* This file is from Linux Kernel (include/linux/list.h) 
 * and modified by simply removing hardware prefetching of list items, 
 * and the syntax feature requirs GCC extension. 
 * Some useful list operations are added additionally.
 * Here by copyright, credits attributed to wherever they belong.
 * Jack Zheng (ybybzj@gmail.com)
 */

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct list_node {
	struct list_node *next, *prev;
} dl_ent_t;

typedef dl_ent_t list_head;
#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)


/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_node pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_node to use as a loop counter.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); \
        	pos = pos->next)
/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_node to use as a loop counter.
 * @head:	the head for your list.
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); \
        	pos = pos->prev)
        	
/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_node to use as a loop counter.
 * @n:		another &struct list_node to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)


/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(list_head *head)
{
	return head->next == head;
}

/**
 * list_size - get the count of list entries
 * @head: the list to count.
 */
static inline int list_size(list_head *head)
{
	int count = 0;
	struct list_node *p;
	list_for_each(p,head){
		count++;
	}
	return count;
}

/**
 * list_reverse - reverse the order of the entry in list
 * @head: the list to reverse.
 */
static inline void list_reverse(list_head *head)
{
	struct list_node *p, *n, *tmp;
	if(!list_empty(head))
	{
		list_for_each_safe(p,n,head)
		{
			tmp = p->prev;
			p->prev = p->next;
			p->next = tmp;
		}
		tmp = head->prev;
		head->prev = head->next;
		head->next = tmp;
	}
}

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_node *new,
			      struct list_node *prev,
			      struct list_node *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * list_unshift - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_unshift(struct list_node *new, list_head *head)
{
	__list_add(new, head, head->next);
}

/**
 * list_push - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_push(struct list_node *new, list_head *head)
{
	__list_add(new, head->prev, head);
}

/**
 * list_add_before - add a new entry 
 * @new: new entry to be added
 * @entry: list entry to add it before
 *
 * Insert a new entry before the specified entry.
 */
static inline void list_add_before(struct list_node *new, struct list_node *entry)
{
	__list_add(new, entry->prev, entry);
}

/**
 * list_add_after - add a new entry 
 * @new: new entry to be added
 * @entry: list entry to add it after
 *
 * Insert a new entry after the specified entry.
 */
static inline void list_add_after(struct list_node *new, struct list_node *entry)
{
	__list_add(new, entry, entry->next);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_node *prev, struct list_node *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_shift - deletes entry from list.
 * @head: the list head to delete from
 * return: the first entry of the list
 * delete and return the first entry after the specified list head, return NULL if list is empty
 * Note: list_empty on return entry return true.
 */
static inline struct list_node* list_shift(list_head *head)
{
	if(list_empty(head)) return NULL;
	struct list_node* first = head->next;
	__list_del(first->prev,first->next);
	INIT_LIST_HEAD(first); 
	return first;
}

/**
 * list_pop - deletes entry from list.
 * @head: the list head to delete from
 * return: the last entry of the list
 * delete and return the last entry before the specified list head, return NULL if list is empty
 * Note: list_empty on return entry return true after this.
 */
static inline struct list_node* list_pop(list_head *head)
{
	if(list_empty(head)) return NULL;
	struct list_node* last = head->prev;
	__list_del(last->prev,last->next);
	INIT_LIST_HEAD(last); 
	return last;
}



/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void list_del(struct list_node *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry); 
}

/**
 * list_move - delete from one list and add as another's head
 * @entry: the entry to move
 * @head: the head that will precede our entry
 */
static inline void list_move(struct list_node *entry, list_head *head)
{
        __list_del(entry->prev, entry->next);
        list_unshift(entry, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @entry: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(struct list_node *entry,
				  struct list_node *head)
{
        __list_del(entry->prev, entry->next);
        list_push(entry, head);
}



static inline void __list_splice(list_head *list,
				 list_head *head)
{
	struct list_node *first = list->next;
	struct list_node *last = list->prev;
	struct list_node *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice(list_head *list, list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void list_splice_init(struct list_node *list,
				    struct list_node *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head);
		INIT_LIST_HEAD(list);
	}
}

/**
 * list_contain - test if a entry belong the specified list
 * @entry: the entry to find.
 * @head: the list head to search
 * Note: use brute force search 
 */
static inline int list_contain(list_head *head, struct list_node *entry)
{
	struct list_node *before, *after;
	before = head->next;
	after = head->prev;
	do{
		if(entry == before || entry == after) return 1;
		before = before->next, after = after->prev;
	}while((before != head)&&(after != head)&&
		(before->prev != after) &&
		(before->prev != after->next));
	
	return 0;
}





#endif