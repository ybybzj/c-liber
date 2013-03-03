/*
*	- Red Black Trees -
*
*	implemented according to the CLRS's "Introduction to Algorithms 3rd edition"
*
*	@ybybzj
*/
#ifndef __RED_BLACK_TREE_H__
#define __RED_BLACK_TREE_H__

#ifndef NULL
#define NULL ((void *)0)
#endif

/*reb black tree node, suppose to be embeded in another structure data*/
struct rb_node {
	unsigned long  __rb_parent_color; // the lowest bit stands for the color of the node
	struct rb_node *rb_right;
	struct rb_node *rb_left;
};
    
/*reb black tree */
struct rb_tree {
	struct rb_node *rb_root;
};

#define	RB_RED		0
#define	RB_BLACK	1
#define rb_parent(rb)   ((struct rb_node *)((rb)->__rb_parent_color & ~1)) //address of rb_node is at least aligned by 16bit(2bytes)

#define rb_color(rb)       ((rb)->__rb_parent_color & 1)
#define RB_EMPTY_TREE	(struct rb_tree) { NULL, }
#define	rb_entry(ptr, type, member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
#define rb_is_leaf(rb) ((rb) == NULL)
#define rb_is_root(rb) (!rb_is_leaf(rb) && rb_parent(rb) == NULL)
#define rb_is_lchild(rb) (!rb_is_root(rb) && rb_parent(rb)->rb_left == (rb))
#define rb_is_rchild(rb) (!rb_is_root(rb) && rb_parent(rb)->rb_right == (rb))
#define RB_IS_EMPTY_TREE(tree)  ((tree)->rb_root == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define RB_IS_ORPHAN(node)  \
	((node)->__rb_parent_color == (unsigned long)(node))
#define RB_MAKE_ORPHAN(node)  \
	((node)->__rb_parent_color = (unsigned long)(node))

static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->__rb_parent_color = rb_color(rb) | (unsigned long)p;
}

static inline void rb_set_color(struct rb_node *rb, int color)
{
	rb->__rb_parent_color = (rb->__rb_parent_color & ~1) | color;
}

static inline void rb_node_init(struct rb_node *rb)
{
	rb->rb_left = NULL;
	rb->rb_right = NULL;
	rb->__rb_parent_color = (unsigned long)NULL | RB_RED;
}

/*To use rbtrees you'll have to implement your own search function, 
  and you can choose to implement insert function yourself as well.
  This will avoid using callbacks and prevent dropping drammatically performances.*/
extern void rb_insert_fixup(struct rb_tree *, struct rb_node *); 
extern int rb_insert(struct rb_tree *, struct rb_node *, int (*cmp)(struct rb_node *,struct rb_node *));

extern void rb_erase(struct rb_tree *, struct rb_node *);

extern void rb_replace(struct rb_tree *, struct rb_node *, struct rb_node *);


/* Find logical next and previous nodes in a tree */
extern struct rb_node *rb_next(const struct rb_node *);
extern struct rb_node *rb_prev(const struct rb_node *);
extern struct rb_node *rb_first(const struct rb_tree *);
extern struct rb_node *rb_last(const struct rb_tree *);

extern void rb_destroy(struct rb_tree *, void (*freeFn)(struct rb_node *));



//--debug helpers--
extern void rb_print(struct rb_tree *tree, void (*prt_val)(struct rb_node*));
#endif