#include "rbtree.h"
#include "../dbg.h"


/*
 * red-black trees properties:  
 *
 *  1) A node is either red or black
 *  2) The root is black
 *  3) All leaves (NULL) are black
 *  4) Both children of every red node are black
 *  5) Every simple path from the node to descendant leaves contains the same number
 *     of black nodes.
 *
 */


#define rb_gparent(rb)   ((struct rb_node *)((rb_parent(rb))->__rb_parent_color & ~1))
#define rb_sibling(rb) (rb_is_lchild(rb) ? rb_parent(rb)->rb_right : rb_parent(rb)->rb_left)
#define __rb_color(pc)     ((pc) & 1)
#define __rb_is_black(pc)  __rb_color(pc)
#define __rb_is_red(pc)    (!__rb_color(pc))

#define rb_is_red(rb)      ((rb) != NULL && __rb_is_red((rb)->__rb_parent_color))
#define rb_is_black(rb)    (((rb)==NULL) || __rb_is_black((rb)->__rb_parent_color)) //nil node as leaf is black



/*---------------rotation operations----------------->*/

static inline void rb_left_rotate(struct rb_tree *rbt, struct rb_node *root)
{
	/*
		  |                                |
		(root)                          (new_root)
		 /  \ 							  /     \
	    a 	(new_root)    --->         (root)    c
	   		 /    \                    /    \
		    b      c                  a      b
	*/
	struct rb_node *new_root = root->rb_right;
	root->rb_right = new_root->rb_left;
	if(!rb_is_leaf(new_root->rb_left))
		rb_set_parent(new_root->rb_left,root);
	rb_set_parent(new_root,rb_parent(root));
	if(rb_is_root(root))
		rbt->rb_root = new_root;
	else if(rb_is_lchild(root))
		rb_parent(root)->rb_left = new_root;
	else
		rb_parent(root)->rb_right = new_root;
	new_root->rb_left = root;
	rb_set_parent(root,new_root);
}

static inline void rb_right_rotate(struct rb_tree *rbt, struct rb_node *root)
{
	/*
			    |                         |
			  (root)                   (new_root)
			  /    \ 				    /     \
		 (new_root) a 	    --->       b     (root) 
		    /    \  		           		 /    \
		   b      c                  		c      a
	*/
	struct rb_node *new_root = root->rb_left;
	root->rb_left = new_root->rb_right;
	if(!rb_is_leaf(new_root->rb_right))
		rb_set_parent(new_root->rb_right,root);
	rb_set_parent(new_root,rb_parent(root));
	if(rb_is_root(root))
		rbt->rb_root = new_root;
	else if(rb_is_lchild(root))
		rb_parent(root)->rb_left = new_root;
	else
		rb_parent(root)->rb_right = new_root;
	new_root->rb_right = root;
	rb_set_parent(root,new_root);
}
/*<---------------rotation operations-----------------*/


/*---------------insert fixup operations----------------->*/
//possible 3 cases encountered are depicted on p.317 in CLRS's book
void rb_insert_fixup(struct rb_tree *tree, struct rb_node *insert_node)
{
	if(tree == NULL || RB_IS_EMPTY_TREE(tree) || insert_node == NULL)
		return;

	while(rb_is_red(rb_parent(insert_node)))
	{
		if(rb_is_lchild(rb_parent(insert_node)))
		{
			struct rb_node *pr = rb_gparent(insert_node)->rb_right;
			if(rb_is_red(pr))// case 1
			{
				rb_set_color(rb_parent(insert_node),RB_BLACK);    
				rb_set_color(pr,RB_BLACK); 						  
				rb_set_color(rb_gparent(insert_node),RB_RED);	  
				insert_node = rb_gparent(insert_node);            
			}else 
			{
				if(rb_is_rchild(insert_node)) //case 2
				{
					insert_node = rb_parent(insert_node);
					rb_left_rotate(tree,insert_node);
				}
				//case 3
				rb_set_color(rb_parent(insert_node), RB_BLACK);
				rb_set_color(rb_gparent(insert_node), RB_RED);
				rb_right_rotate(tree, rb_gparent(insert_node));
			}
		}else
		{
			struct rb_node *pl = rb_gparent(insert_node)->rb_left;
			if(rb_is_red(pl))// case 1
			{
				rb_set_color(rb_parent(insert_node),RB_BLACK);    
				rb_set_color(pl,RB_BLACK); 						  
				rb_set_color(rb_gparent(insert_node),RB_RED);	  
				insert_node = rb_gparent(insert_node);            
			}else 
			{
				if(rb_is_lchild(insert_node)) //case 2
				{
					insert_node = rb_parent(insert_node);
					rb_right_rotate(tree,insert_node);
				}
				//case 3
				rb_set_color(rb_parent(insert_node), RB_BLACK);
				rb_set_color(rb_gparent(insert_node), RB_RED);
				rb_left_rotate(tree, rb_gparent(insert_node));
			}
		}
	}
	rb_set_color(tree->rb_root,RB_BLACK); //property 2
}
/*<---------------insert fixup operations-----------------*/

int rb_insert(struct rb_tree *tree, struct rb_node *insert_node, int (*cmp)(struct rb_node *,struct rb_node *))
{
	check(tree != NULL && cmp != NULL, return -1);
	struct rb_node *p, *node;
	p = NULL;
	node = tree->rb_root;

	rb_node_init(insert_node);

	while(node != NULL)
	{
		p = node;
		if(cmp(insert_node,p) < 0)
			node = node->rb_left;
		else
			node = node->rb_right;
	}

	rb_set_parent(insert_node,p);

	if(p == NULL)
		tree->rb_root = insert_node;
	else if(cmp(insert_node,p) < 0)
		p->rb_left = insert_node;
	else
		p->rb_right = insert_node;

	rb_insert_fixup(tree, insert_node);

	return 0;
}


static struct rb_node *__rb_min(const struct rb_node *root)
{
	struct rb_node *min;
	min = (struct rb_node *)root;
	while(min != NULL && min->rb_left != NULL)
		min = min->rb_left;
	return min;
}

static struct rb_node *__rb_max(const struct rb_node *root)
{
	struct rb_node *max;
	max = (struct rb_node *)root;
	while(max != NULL && max->rb_right != NULL)
		max = max->rb_right;
	return max;
}

struct rb_node *rb_first(const struct rb_tree *tree)
{
	return tree ? __rb_min(tree->rb_root) : NULL;
}

struct rb_node *rb_last(const struct rb_tree *tree)
{
	return tree ? __rb_max(tree->rb_root) : NULL;
}


struct rb_node *rb_next(const struct rb_node *node)
{
	if(node == NULL) return NULL;
	struct rb_node *next = node->rb_right;
	if(next == NULL)
	{
		if(rb_is_root(node))
			return NULL;
		else if(rb_is_lchild(node))
		{
			return rb_parent(node);
		}
		else{
			struct rb_node *n = rb_parent(node);
			while(n != NULL && rb_is_rchild(n))
				n = rb_parent(n);
			return rb_is_root(n) ? NULL : rb_parent(n);
		}
	}
	else
		return __rb_min(next);
	
}
			

struct rb_node *rb_prev(const struct rb_node *node)
{
	struct rb_node *prev = node->rb_left;
	if(prev == NULL)
	{
		if(rb_is_root(node))
			return NULL;
		else if(rb_is_rchild(node))
			return rb_parent(node);
		else{
			struct rb_node *n = rb_parent(node);
			while(n != NULL && rb_is_lchild(n))
				n = rb_parent(n);
			return rb_is_root(n) ? NULL : rb_parent(n);
		}
	}
	else
		return __rb_max(prev);
}

/*---------------delete fixup and transplant operations----------------->*/
//replace sub-tree u with v
static inline void rb_transplant(struct rb_tree *tree, struct rb_node *u, struct rb_node *v)
{
	if(rb_is_root(u))
		tree->rb_root = v;
	else if(rb_is_lchild(u))
		rb_parent(u)->rb_left = v;
	else
		rb_parent(u)->rb_right = v;
	if(v != NULL)
		rb_set_parent(v,rb_parent(u));
}
//depict on p.329 in CLRS's book
static void rb_delete_fixup(struct rb_tree *tree, struct rb_node *pnode, struct rb_node *node)
{
	if(pnode == NULL)
	{
		if(node != NULL)
			rb_set_color(node,RB_BLACK); // property 2
		return;
	}
	while(!rb_is_root(node) && rb_is_black(node))
	{
		struct rb_node *p = rb_is_leaf(node) ? pnode : rb_parent(node);

		if(node == p->rb_left)
		{
			struct rb_node *w = p->rb_right;
			if(rb_is_red(w))							//case 1
			{
				rb_set_color(w,RB_BLACK);
				rb_set_color(p,RB_RED);
				rb_left_rotate(tree,p);
				w = p->rb_right;						//fall through, continue to check case 2 or 4
			}

			if(rb_is_black(w->rb_left) && rb_is_black(w->rb_right)) // case 2
			{
				rb_set_color(w,RB_RED);
				node = p;
			}else 												
			{
				if(rb_is_black(w->rb_right))			//case 3
				{
					rb_set_color(w,RB_RED);
					rb_set_color(w->rb_left,RB_BLACK);
					rb_right_rotate(tree,w);
					w = w->rb_left;                     //fall in to case 4
				}
				rb_set_color(w,rb_color(p));
				rb_set_color(p,RB_BLACK);
				rb_set_color(w->rb_right,RB_BLACK);
				rb_left_rotate(tree,p);
				node = tree->rb_root;
			}
		}else
		{
			struct rb_node *w = p->rb_left;
			if(rb_is_red(w))							//case 1
			{
				rb_set_color(w,RB_BLACK);
				rb_set_color(p,RB_RED);
				rb_right_rotate(tree,p);
				w = p->rb_left;						//fall through, continue to check case 2 or 4
			}

			if(rb_is_black(w->rb_left) && rb_is_black(w->rb_right)) // case 2
			{
				rb_set_color(w,RB_RED);
				node = p;
			}else 												
			{
				if(rb_is_black(w->rb_left))			//case 3
				{
					rb_set_color(w,RB_RED);
					rb_set_color(w->rb_right,RB_BLACK);
					rb_left_rotate(tree,w);
					w = w->rb_right;                     //fall in to case 4
				}
				rb_set_color(w,rb_color(p));
				rb_set_color(p,RB_BLACK);
				rb_set_color(w->rb_left,RB_BLACK);
				rb_right_rotate(tree,p);
				node = tree->rb_root;
			}
		}
	}
	rb_set_color(node,RB_BLACK);
}
/*<---------------delete fixup and transplant operations-----------------*/

/*---------------erase function-----------------*/

void rb_erase(struct rb_tree *tree, struct rb_node *node)
{
	if(tree == NULL || RB_IS_EMPTY_TREE(tree) || node == NULL)
		return;
	struct rb_node *del_node, *pdel_node, *cld_node;
	int del_color;
	del_node = node;
	del_color = rb_color(del_node);
	pdel_node = rb_parent(del_node);
	

	if(del_node->rb_left == NULL)
	{
		cld_node = del_node->rb_right;
		rb_transplant(tree,del_node,del_node->rb_right);
	}else if(del_node->rb_right == NULL)
	{
		cld_node = del_node->rb_left;
		rb_transplant(tree, del_node, del_node->rb_left);
	}else
	{
		del_node = rb_next(node);
		del_color = rb_color(del_node);
		pdel_node = rb_parent(del_node);
		cld_node = del_node->rb_right;
		if(pdel_node == node)
		{
			pdel_node = del_node;
		}else
		{
			rb_transplant(tree, del_node, del_node->rb_right);
			del_node->rb_right = node->rb_right;
			rb_set_parent(node->rb_right,del_node);
		}
		rb_transplant(tree, node, del_node);
		rb_set_parent(node->rb_left,del_node);
		del_node->rb_left = node->rb_left;
		rb_set_color(del_node,rb_color(node));
	}

	if(del_color == RB_BLACK)
		rb_delete_fixup(tree, pdel_node,cld_node);


	RB_MAKE_ORPHAN(node);
}


void rb_replace(struct rb_tree *tree, struct rb_node *old, struct rb_node *new)
{
	if(tree == NULL || RB_IS_EMPTY_TREE(tree) || old == NULL || new == NULL)
		return;

	rb_transplant(tree,old,new);
	new->rb_right = old->rb_right;
	new->rb_left = old->rb_left;
	rb_set_color(new,rb_color(old));

	RB_MAKE_ORPHAN(old);
}

static void __rb_destroy_r(struct rb_node *node,  void (*freeFn)(struct rb_node *))
{
	if(freeFn == NULL || node == NULL) return;
	__rb_destroy_r(node->rb_left,  freeFn);
	__rb_destroy_r(node->rb_right,  freeFn);
	freeFn(node);
}

void rb_destroy(struct rb_tree *tree, void (*freeFn)(struct rb_node *))
{
	if(tree == NULL) return;
	__rb_destroy_r(tree->rb_root,  freeFn);
	tree->rb_root = NULL;
}


//helpers

static void rb_print_r(struct rb_node *root, const char *indicator, int indent, void (*prt_val)(struct rb_node*))
{

	int i = 0;
	for(;i < indent; i++)
		printf(" ");
	if(root == NULL)
	{
		println("-%s(B) <nil>",indicator);
		return;
	}else{
		printf("-%s(%.*s) s<%p> p<%p>: ",indicator,2,rb_color(root) ? "B":"R",root,rb_parent(root));
		if(prt_val != NULL)
			prt_val(root);
		println();
		rb_print_r(root->rb_left, "l", indent + 6, prt_val);
		rb_print_r(root->rb_right, "r", indent + 6, prt_val);
	} 

}

void rb_print(struct rb_tree *tree,void (*prt_val)(struct rb_node*))
{
	if(tree == NULL || RB_IS_EMPTY_TREE(tree)){
		println("<empty tree>");
		return;
	}
		rb_print_r(tree->rb_root, "t", 6, prt_val);
}
