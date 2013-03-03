#include <unit_test/minunit.h>
#include <common/ds/rbtree.h>

typedef struct {
	int val;
	struct rb_node rb;
}rb_int_t;

static int rb_int_cmp(struct rb_node *insert_node, struct rb_node *node)
{
	return rb_entry(insert_node, rb_int_t,rb)->val - rb_entry(node,rb_int_t, rb)->val;
}

static int rb_int_insert(struct rb_tree *tree, int val)
{
	check(tree != NULL, return -1);

	rb_int_t *vnode;
	check((vnode = (rb_int_t*)malloc(sizeof(rb_int_t))) != NULL, goto on_mem_err);
	vnode->val = val;
	rb_insert(tree, &vnode->rb, rb_int_cmp);

	return 0;
on_mem_err:
	return -1;
}


static struct rb_node* rb_int_search(struct rb_tree *tree, int key)
{
	
	if(tree == NULL || RB_IS_EMPTY_TREE(tree)) return NULL;
	struct rb_node *node = tree->rb_root;
	while(node != NULL)
	{
		int val = rb_entry(node,rb_int_t,rb)->val;
		if(key == val)
			return node;
		else if(key < val)
			node = node->rb_left;
		else
			node = node->rb_right;
	}
	return node;
}

static void rb_int_free(struct rb_node *node)
{
	if(node == NULL) return;
	free(rb_entry(node,rb_int_t,rb));
}

static void rb_int_prt_val(struct rb_node *node)
{
	if(node == NULL) return;
	printf("%d",rb_entry(node,rb_int_t,rb)->val);
}


static void rb_int_print_list(struct rb_tree *tree)
{
	if(tree == NULL) return;
	struct rb_node *node = rb_first(tree);
	while(node != NULL)
	{
		rb_int_prt_val(node);
		node = rb_next(node);
		if(node != NULL)
			printf(",");	
	}
	println();
}

static struct rb_tree tree = {NULL};



bool test_rb_insert(void *arg UNUSED)
{
	println("before insert:");
	rb_print(&tree,rb_int_prt_val);
	int i;
	for(i = 1; i < 5; i++)
	{
		rb_int_insert(&tree, i);
	}

	for(i = 1; i < 5; i++)
	{
		rb_int_insert(&tree, i);
	}
	println("after insert:");
	rb_print(&tree,rb_int_prt_val);

	println("list print:");
	rb_int_print_list(&tree);

	rb_destroy(&tree, rb_int_free);
	println("after destroy:");
	rb_print(&tree,rb_int_prt_val);
	return true;
}

bool test_rb_delete(void *arg UNUSED)
{
	int i;
	for(i = 1; i < 11; i++)
	{
		rb_int_insert(&tree, i);
	}

	println("before delete:");
	rb_print(&tree,rb_int_prt_val);
	rb_int_print_list(&tree);

	struct rb_node *node = rb_int_search(&tree, 5);
	mu_assert(node != NULL, "search key 5 should be successful!");

	rb_erase(&tree,node);
	println("after delete:");
	rb_print(&tree,rb_int_prt_val);
	rb_int_print_list(&tree);

	rb_destroy(&tree, rb_int_free);
	return true;

}

SUITE_START(NULL)
// testcase_startup(tst_startup);
// testcase_teardown(tst_teardown);
// mu_run_test(test_rb_insert);
mu_run_test(test_rb_delete);

SUITE_END(NULL)

RUN_TESTS()