#include <unit_test/minunit.h>
#include <system/nx.h>
#include <system/nx/time.h>
#include <common/console.h>
#include <common/ev_emitter.h>

#include <common/ds/tsearch_tree.h>
void *tc_startup(void)
{
	ev_emitter_t *emitter = (ev_emitter_t*)malloc(sizeof(ev_emitter_t));
	check(emitter != NULL, return NULL);
	check(ev_emitter_init(emitter) != -1,free(emitter);return NULL);
	return emitter;
}

void tc_teardown(void *arg)
{
	ev_emitter_t *emitter = (ev_emitter_t*)arg;
	if(emitter == NULL) return;
	ev_emitter_destroy(emitter);
	free(emitter);
	return;
}

// bool test_cur_time(void *arg UNUSED){
// 	println("current time: %s", current_time("%T"));
// 	char input[1024];
// 	int len = cls_prompt("prompt>", input, 1024);
// 	println("input => (%d)%s",len, input);
// 	return true;
// }
static void timeout_cb(int arg_len, arg_data_t arguments[static arg_len])
{
	check(arg_len >= 1 && argisp(arguments[0]), return);
	char *str = (char*)argtop(arguments[0]);
	println("timeout message: %s", str);
}

bool test_emitter(void *arg UNUSED){
	int err;
	ev_emitter_t *emitter = (ev_emitter_t*)arg;
	check(emitter!= NULL,return false);
	err = ev_emitter_once(emitter,"timeout",timeout_cb,NULL);
	mu_assert(err == 0,"ev_emitter_on should be succeed!");
	// tst_print(emitter->ev_cb_map);
	sleep(4);
	err = ev_emitter_emit(emitter,"timeout", ptoarg("timeout event happened!"));
	err = ev_emitter_emit(emitter,"timeout", ptoarg("timeout event happened again!"));
	mu_assert(err == 0,"ev_emitter_emit should be succeed!");
	return true;
}

SUITE_START(NULL)
testcase_startup(tc_startup);
testcase_teardown(tc_teardown);
mu_run_test(test_emitter);

SUITE_END(NULL)

RUN_TESTS()