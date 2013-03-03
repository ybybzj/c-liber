#ifndef __TERNARY_SEARCH_TREE_H__
#define __TERNARY_SEARCH_TREE_H__
typedef struct tst_node tst_node_t;
typedef struct tst_s
{
	tst_node_t *head;
	int key_num;
}tstree_t;

int tst_init(tstree_t *tstp);
void tst_destroy(tstree_t tst,void (*freeFn)(void *));//freeFn is destroy function for val;
int tst_insert(tstree_t *tstp, const char *key, void *val, void **oval);// if key exists, replace old val with val and return old val through oval pointer
int tst_search(tstree_t tst, const char *key, void **out);//>0 search hit, 0 search miss; if out == NULL, test key's existence
int tst_delete(tstree_t *tstp, const char *key, void (*freeFn)(void *));//freeFn is destroy function for val;

//debug helpers
void tst_print(tstree_t tst);
#endif