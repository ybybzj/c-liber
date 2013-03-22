#define _GNU_SOURCE
#include <common/dbg.h>
#include <system/nx.h>
#include "_timeout.h"
#include <sys/timerfd.h>

typedef struct {
	int is_interval;
	int (*tm_cb) (void*);
	void *arg;
	void (*arg_free)(void*);
}ev_tm_data;

static ev_tm_data *ev_tm_data_create( int (*tm_cb) (void*),
									void *arg,
									void (*arg_free)(void*),
									int is_interval)
{
	ev_tm_data *et = MALLOC(1,ev_tm_data);
	check(et != NULL, return NULL);
	et->tm_cb = tm_cb;
	et->arg = arg;
	et->arg_free = arg_free;
	et->is_interval = is_interval;
	return et;
}

static void ev_tm_data_free(ev_tm_data *et)
{
	if(et != NULL)
	{
		if(et->arg_free != NULL)
		{
			et->arg_free(et->arg);
		}
		free(et);
	}
}

static void get_itimerspec(unsigned int sec, struct itimerspec *its, int is_interval)
{
	check(its != NULL, return);
	its->it_value.tv_sec = sec;
	its->it_value.tv_nsec = 0;
	its->it_interval.tv_sec = (is_interval ? sec : 0);
	its->it_interval.tv_nsec = 0;
	return;
}
static inline struct itimerspec zero_itimerspec()
{
	struct itimerspec ts;
	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	return ts;
}

static int make_timeout_ev( int priority,
							int (*tm_cb) (void*),
							void *arg,
							void (*arg_free)(void*),
							int is_interval,
							fevent *out_ev)
{
	fevent ev_timeout = ev_new_event();

	ev_timeout.events = EV_READ;
	ev_timeout.priority = priority;
	int timerfd;

	check( (timerfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK)) != -1, return -1);

	ev_timeout.fd = timerfd;

	ev_timeout.data.ptr = ev_tm_data_create( tm_cb, arg,arg_free, is_interval);
	check(ev_timeout.data.ptr != NULL, close(timerfd); return -1);

	ev_timeout.ev_data_free = (void (*)(void *))ev_tm_data_free;

	*out_ev = ev_timeout;
	return 0;

}

static ev_set on_timeout(fevent ev, ev_monitor *monitor UNUSED)
{
	int (*tm_cb) (void*);
	ev_tm_data *et = (ev_tm_data*)ev.data.ptr;
	tm_cb = et->tm_cb;
	uint64_t exNum = 0;
	read(ev.fd, &exNum, sizeof(uint64_t));

	if(tm_cb)
	{
		int ret = tm_cb(et->arg);
		if(et->is_interval && ret)
			return ev.events;
		else{
			struct itimerspec z_ts = zero_itimerspec();
			timerfd_settime(ev.fd, 0, &z_ts, NULL);
			return EV_FIN;
		}
	}
	return EV_FIN;
}

int add_timeout_ev( ev_monitor *monitor,
					int priority,
					int delay,
					int (*tm_cb) (void*),
					void *arg,
					void (*arg_free)(void*),
					int is_interval)
{
	fevent ev_timeout;
	check(make_timeout_ev(priority,
					tm_cb,
					arg,
					arg_free,
					is_interval,
					&ev_timeout) != -1, return -1);
	struct itimerspec ts;
	get_itimerspec(delay, &ts, is_interval);
	timerfd_settime(ev_timeout.fd, 0, &ts, NULL);
	check(ev_monitor_ctl(monitor, EV_CTL_ADD, ev_timeout, EV_RD_CB(on_timeout)) != -1, return -1);
	return 0;
}
