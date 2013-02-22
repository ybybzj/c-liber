#include <unit_test/minunit.h>
#include <system/nx/time.h>


bool test_cur_time(void *arg UNUSED){
	println("current time: %s", current_time("%T"));
	return true;
}


SUITE_START(NULL)

mu_run_test(test_cur_time);

SUITE_END(NULL)

RUN_TESTS()