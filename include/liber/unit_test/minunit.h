#undef NDEBUG
#ifndef __MINUNIT_H__
#define __MINUNIT_H__

#include <common/dbg.h>
typedef void *(*testcase_startup)(void);
typedef void (*testcase_teardown)(void *);
typedef void (*testsuite_setup)(void);

typedef struct {
	int total;
	int failed;
	bool isPassed;
	testcase_startup tc_startup;
	testcase_teardown tc_teardown;
	testsuite_setup ts_startup, ts_teardown;
	void *data;
}Test_suite;



#define SUITE_START(suite_startup) Test_suite testAll(bool showDetail){ \
Test_suite ts = {.total=0,.failed =0,.isPassed = true, .tc_startup = NULL, .tc_teardown = NULL,\
 .ts_startup = (suite_startup), .ts_teardown = NULL, .data = NULL};\
if(ts.ts_startup!= NULL) ts.ts_startup();

#define testcase_startup(case_startup) do{ts.tc_startup = (case_startup);}while(0)
#define testcase_teardown(case_teardown) do{ts.tc_teardown = (case_teardown);}while(0)

#define SUITE_END(suite_teardown) ts.ts_teardown = (suite_teardown);\
if(showDetail) println();\
if(ts.ts_teardown!= NULL) ts.ts_teardown();\
return ts;}

#define mu_assert(test, message,...) check_err(test,return false,message,##__VA_ARGS__)

#define mu_run_test(test) do{ \
		if(showDetail) println("\n  <-- [%s] testing...", cls_s(#test,cls_hMagenta)); \
		ts.total++; \
		if(ts.tc_startup != NULL){ts.data = ts.tc_startup();}\
		if(test(ts.data)) { if(showDetail) println("  -->  " cls_s( " --PASS-- ",cls_hGreen));} \
		else { if(showDetail)  println("  -->  " cls_s( " --FAIL-- ",cls_hRed)); ts.isPassed = false;ts.failed++;} \
		if(ts.tc_teardown != NULL){ts.tc_teardown(ts.data);ts.data = NULL;}\
}while(0)

#define RUN_TESTS() int main(int argc, char **argv) { \
	bool showDetail = true; \
	if(argc == 2){ \
		showDetail = false; \
	} \
	println("\n<==== Running Tests: "cls_s("%s",cls_Bold;cls_hYellow), argv[0]); \
	Test_suite result = testAll(showDetail); \
	if(result.isPassed) { \
		println("====> Test Result: Total " cls_s("%d",cls_hYellow) " tests, " cls_s("All Passed",cls_hGreen),result.total); \
		exit(0); \
	} else { \
		println("====> Test Result: Total " cls_s("%d",cls_hYellow) " tests, " cls_s("%d Passed",cls_hGreen) ", " cls_s("%d Failed",cls_hRed), result.total, result.total-result.failed, result.failed); \
		exit(1); \
	} \
}
#endif