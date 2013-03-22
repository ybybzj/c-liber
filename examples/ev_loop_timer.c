#define _GNU_SOURCE
#include <common/dbg.h>
#include <system/nx.h>
#include <fevent_loop/event_loop.h>
#include <sys/timerfd.h>
static ev_monitor *monitor = NULL;

static int on_timeout(void*);

static void add_timeout_ev(int delay, int priority, ev_monitor *monitor);


int main()
{
	monitor = ev_monitor_create();
	check(monitor != NULL, goto onerr);

	// int i = 0;
	// for(;i < 10; i++)
	// 	add_timeout_ev( 1 , i%3 + 1, monitor);
	add_timeout_ev(1 , 1, monitor);
	add_timeout_ev(2 , 1, monitor);
	add_timeout_ev(3 , 1, monitor);

	check(ev_loop_run(monitor ,0) != -1, goto onerr);
	ev_monitor_free(monitor);
	exit(EXIT_SUCCESS);
onerr:
	exit(EXIT_FAILURE);
}

static int on_timeout(void *arg)
{
	// static int timeout_num = 0;

	int *tn = (int*)arg;
	println("Timeout %d time%s!", *tn, (*tn > 0 ? "s":""));
	*tn = *tn + 1;
	return *tn > 4 ? 0 : 1;
}

static void add_timeout_ev(int delay, int priority, ev_monitor *monitor)
{
	int *a;
	a = MALLOC(1, int);
	*a = 1;
	ev_set_interval( monitor, priority, delay, on_timeout, a, free);

}
