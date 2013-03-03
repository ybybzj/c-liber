#include "tsearch_tree.h"
#include <common/dbg.h>
#include <stdlib.h>
#define TCHAR '\0'
#define isvnode(node) ((node)->c == TCHAR)
#define vnode_val(vnode) ((void *)((vnode)->e))



struct tst_node 
{
	int c;
	struct tst_node *l, *e, *r; //l -> smaller, e -> equal, r -> bigger
};


static tst_node_t* new_tst_node(tst_node_t *e,int c)
{
	if(e == NULL) return NULL;
	tst_node_t* new_node = NULL;
	check((new_node = malloc(sizeof(*new_node))) != NULL, return NULL);
	new_node->c = c;
	new_node->l = NULL;
	new_node->e = e;
	new_node->r = NULL;
	if(isvnode(e))
		e->r = new_node;
	return new_node;
}

static tst_node_t* new_tst_vnode(void *val)
{
	tst_node_t* new_vnode = NULL;
	check((new_vnode = malloc(sizeof(*new_vnode))) != NULL, return NULL);
	new_vnode->c = TCHAR;
	new_vnode->l = NULL;
	new_vnode->e = (tst_node_t*)val;
	new_vnode->r = NULL;
	return new_vnode;
}


int tst_init(tstree_t *tstp)
{
	check(tstp != NULL, return -1);
	tstp->head = NULL;
	tstp->key_num = 0;
	return 0;
}

static void tst_destroy_r(tst_node_t *head,void (*freeFn)(void *))
{
	if(head == NULL) return;
	if(!isvnode(head)){
		tst_destroy_r(head->l,freeFn);
		tst_destroy_r(head->e,freeFn);
		tst_destroy_r(head->r,freeFn);
	}else{//isvnode: e, r cannot be destroyed by tst_destroy_r
		if(freeFn != NULL)
			freeFn(vnode_val(head));
		tst_destroy_r(head->l,freeFn);
	}
	free(head);
	return;
}
void tst_destroy(tstree_t tst,void (*freeFn)(void *))
{
	tst_destroy_r(tst.head,freeFn);
	tst.head = NULL;
	tst.key_num = 0;
}

static int tst_search_r(tst_node_t *head, const char *key, void **out, int idx)
{
	int key_c = *(key+idx);
	if(head == NULL) return 0;
	if(!isvnode(head))
	{
		if(key_c == TCHAR) return 0;
		if(key_c < head->c) return tst_search_r(head->l, key, out, idx);
		if(key_c == head->c) return tst_search_r(head->e, key, out, idx + 1);
		if(key_c > head->c) return tst_search_r(head->r, key, out, idx);
	}
	if(key_c == TCHAR) 
	{
		if(out != NULL)
			*out = vnode_val(head);
		return 1;
	}else
	{
		return tst_search_r(head->l, key, out, idx);
	}
}

int tst_search(tstree_t tst, const char *key, void **out)
{
	return tst_search_r(tst.head, key, out, 0);
}



static tst_node_t* makeNullHead(const char *key, void *val, int idx)
{
	int key_c = *(key+idx);
	if(key_c == TCHAR)
		return new_tst_vnode(val);
	else
	{
		tst_node_t *tnode, *result_node;
		tnode = makeNullHead(key,val,idx+1);
		result_node = new_tst_node(tnode,key_c);
		check(result_node != NULL, tst_destroy_r(tnode,NULL));//prevent memory leak
		return result_node;
	}
}

static int insertHeadR(tst_node_t **headp, tst_node_t *p, const char *key, void *val, void **oval, int idx)
{
	int key_c = *(key+idx);
	
	check(headp != NULL, return -1);
	if(*headp == NULL)
	{
		tst_node_t *head = makeNullHead(key, val, idx);
		check(head != NULL, return -1);
		*headp = head;
		return 0;
	} 
	else if(isvnode(*headp))
	{
		tst_node_t *head = *headp;
		if(key_c == TCHAR)
		{
			if(oval != NULL)
				*oval = vnode_val(head);
			head->e = (tst_node_t*)val;
			return 1;
		}
		
		return insertHeadR(&head->l, head, key, val, oval, idx);
	}else
	{
		tst_node_t *head = *headp;
		if(key_c == TCHAR)
		{
			tst_node_t *vnode = new_tst_vnode(val);
			check(vnode != NULL, return -1);
			vnode->l = head;
			vnode->r = p;
			*headp = vnode;
			return 0;
		}
		if(key_c < head->c)
		{
			return insertHeadR(&head->l, head, key, val, oval, idx);
		}else if(key_c == head->c)
		{
			return insertHeadR(&head->e, head, key, val, oval, idx+1);
		}else //(key_c > head->c)
		{
			return insertHeadR(&head->r, head, key, val, oval, idx);
		}
	}
}

int tst_insert(tstree_t *tstp, const char *key, void *val, void **oval)
{
	check(tstp != NULL, return -1);
	int result = insertHeadR(&tstp->head, NULL, key, val, oval, 0);
	if(result == 0) tstp->key_num++;
	return result;
}


static int tst_delete_r(tst_node_t **headp, const char *key, void (*freeFn)(void *), int idx)
{
	check(headp != NULL, return -1);
	int key_c = *(key+idx);
	if(*headp == NULL) return 0;
	tst_node_t *head = *headp;
	if(!isvnode(*headp))
	{
		
		if(key_c == TCHAR) return 0;
		if(key_c < head->c) 
		{
			if(tst_delete_r(&head->l, key, freeFn, idx)==-1) 
				return -1;
		}else if(key_c == head->c) 
		{
			if(tst_delete_r(&head->e, key, freeFn, idx + 1) == -1) 
				return -1;
		}else{
			if(tst_delete_r(&head->r, key, freeFn, idx) == -1)
				return -1;
		}

		if (head->l == NULL && head->e == NULL && head->r == NULL)
		{
			free(head);
			*headp = NULL;
		}
		return 0;
	}
	if(key_c == TCHAR) 
	{
		if(freeFn != NULL)
			freeFn(vnode_val(head));
		head->r->e = head->l;
		free(head);
		return 0;

	}else
	{
		return tst_delete_r(&head->l, key, freeFn, idx);
	}
}


int tst_delete(tstree_t *tstp, const char *key, void (*freeFn)(void *))
{
	check(tstp != NULL, return -1);
	int result = tst_delete_r(&tstp->head, key, freeFn, 0);
	if(result == 0) tstp->key_num--;
	return result;
}

/*for debug*/
static void print_head(tst_node_t *head, int indent, char link)
{
	
	int i = 0;
	
	if(head == NULL)
	{
		return;
	}else
	{
		for(;i < indent; i++)
		{
			printf(" ");
		}
		printf("+-<%c>",link);
		if(!isvnode(head))
			printf("(%c, %p)\n",head->c, head);
		else
			printf("[T, %p](l:%p, r:%p)\n",head, head->l, head->r);
	}
	
}

static void tst_print_r(tst_node_t *head, int indent, char link)
{
	print_head(head, indent, link);
	if(head != NULL)
	{
		if(!isvnode(head))
		{
			tst_print_r(head->l, indent + 5, 'l');
			tst_print_r(head->e, indent + 5, 'e');
			tst_print_r(head->r, indent + 5, 'r');	
		}else
		{
			tst_print_r(head->l, indent + 5, 'e');
		}
	}
	
}

void tst_print(tstree_t tst)
{
	if(tst.head == NULL) printf("   <empty tst>\n");
	tst_print_r(tst.head, 3, 't');
}



