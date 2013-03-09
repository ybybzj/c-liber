#include <common/dbg.h>
#include <event_loop/event_loop.h>

static ev_monitor *monitor = NULL;

static ev_set on_timeout(event, ev_monitor *);

static void add_timeout_ev(int delay, int priority, ev_monitor *monitor);

int main()
{
	monitor = ev_monitor_create();
	check(monitor != NULL, goto onerr);

	int i = 0;
	for(;i < 10; i++)
		add_timeout_ev(i%2 + 1 , i%3 + 1, monitor);
	
	check(ev_loop_run(monitor ,0) != -1, goto onerr);
	ev_monitor_free(monitor);
	exit(EXIT_SUCCESS);
onerr:
	exit(EXIT_FAILURE);
}

static ev_set on_timeout(event ev, ev_monitor *monitor UNUSED)
{
	// static int timeout_num = 0;
	println("[timerfd %d] Timeout !", ev.fd);
	
	check(ev_monitor_ctl(monitor, EV_CTL_DEL, ev) != -1, return EV_FINISHED);
	
	return EV_FINISHED;
}

static void add_timeout_ev(int delay, int priority, ev_monitor *monitor)
{
	event ev_timeout;
	memset(&ev_timeout, 0, sizeof(event));
	ev_timeout.events = EV_TIMEOUT;
	ev_timeout.time_delay = delay;
	ev_timeout.priority = priority;
	check(ev_monitor_ctl(monitor, EV_CTL_ADD, ev_timeout, EV_CB(EV_TIMEOUT,on_timeout)) != -1, return);

}