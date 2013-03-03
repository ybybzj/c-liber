#include <unit_test/minunit.h>
#include <common/ds/tsearch_tree.h>

void *tst_startup(void)
{
	tstree_t *tstp = (tstree_t*)malloc(sizeof(tstree_t));
	check(tstp != NULL,return NULL);
	tst_init(tstp);
	return tstp;
}

void tst_teardown(void *arg)
{
	tstree_t *tstp = (tstree_t*)arg;
	if(tstp == NULL) return;
	tst_destroy(*tstp,NULL);
	free(tstp);
	return;
}

bool test_tst_insert(void *arg)
{
	tstree_t tst = *((tstree_t*)arg);
	mu_assert(tst_insert(&tst, "key", (void*)((long long)12), NULL) == 0, "tst insert should be success!");
	mu_assert(tst.key_num == 1, "tst.key_num should  be 1 after insert!");
	return true;
}

bool test_tst_search(void *arg){
	tstree_t tst = *((tstree_t*)arg);
	mu_assert(tst_insert(&tst, "key", "key_val",NULL) == 0, "tst insert should be success!");
	mu_assert(tst_insert(&tst, "ke", "ke_val",NULL) == 0, "tst insert should be success!");
	mu_assert(tst_insert(&tst, "kei", "kei_val",NULL) == 0, "tst insert should be success!");
	mu_assert(tst.head != NULL, "tst.head != NULL!");
	mu_assert(tst.key_num == 3, "tst.key_num should  be 3 after insert!");
	tst_print(tst);
	char *out = NULL;
	mu_assert(tst_search(tst, "kei", (void *)(&out)) == 1, "tst search should be success!");
	println("out : %s", out);
	char *old = NULL;
	mu_assert(tst_insert(&tst, "key", "key_val1",(void *)(&old)) == 1, "tst insert should be replacing old value!");

	mu_assert(tst_search(tst, "key", (void *)(&out)) == 1, "tst search should be success!");
	println("after out : %s, old: %s", out, old);
	return true;
}

SUITE_START(NULL)
testcase_startup(tst_startup);
testcase_teardown(tst_teardown);
mu_run_test(test_tst_insert);
mu_run_test(test_tst_search);

SUITE_END(NULL)

RUN_TESTS()