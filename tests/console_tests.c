#include <unit_test/minunit.h>
#include <string.h>
#include <common/safestr.h>
void *tc_startup()
{
	println("startup! %d",strnlen_s("hello world!",0));
	char *str = malloc(strnlen_s("hello world!",0)  + 1);
	strncpy(str,"hello world!", strlen("hello world!"));
	return str;
}

void tc_teardown(void *arg UNUSED)
{
	println("teardown! %s", (char *)arg);
	if(arg != NULL)
		free(arg);
}

bool test_input(void *arg UNUSED){
	println("test1 arg: %s", (char *)arg);
	strncpy(arg,"hello ybyb", strlen("hello ybyb"));
	return true;
}
bool test_input2(void *arg UNUSED){
	println("test2 arg: %s", (char *)arg);
	return true;
}

SUITE_START(NULL)
testcase_startup(tc_startup);
testcase_teardown(tc_teardown);

mu_run_test(test_input);
mu_run_test(test_input2);

SUITE_END(NULL)

RUN_TESTS()