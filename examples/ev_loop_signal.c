#define _GNU_SOURCE
#include <common/dbg.h>
#include <system/nx.h>
#include <system/nx/sigutils.h>
#include <fevent_loop/event_loop.h>
static ev_monitor *monitor = NULL;

static int on_sig(int,void*);




int main()
{
	monitor = ev_monitor_create();
	check(monitor != NULL, goto onerr);
	ev_on_signal(monitor, 1, SIGINT, on_sig, (void*)monitor, NULL);
	ev_on_signal(monitor, 1, SIGQUIT, on_sig, (void*)monitor, NULL);



	check(ev_loop_run(monitor ,0) != -1, goto onerr);
	ev_monitor_free(monitor);
	exit(EXIT_SUCCESS);
onerr:
	exit(EXIT_FAILURE);
}

static int on_sig(int sig, void *arg)
{
	ev_monitor *monitor = (ev_monitor*)arg;
	println("Signal %s Catched!!!", (sig == SIGINT ? "SIGINT" : (sig == SIGQUIT ? "SIGQUIT" : "SIGUNKNOWN")));
	if(sig == SIGQUIT)
	{
		ev_loop_stop(monitor);
		return 0;
	}
	return 1;
}

