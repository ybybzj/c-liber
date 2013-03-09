#define _GNU_SOURCE
#include "_timer.h"
#include <common/dbg.h>
#include <system/nx.h>
#include <sys/timerfd.h>

int timer_prepare(event *ev)
{
	check(ev != NULL, return -1);
	int timerfd;
	check((timerfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK)) != -1, return -1);
	ev->fd = timerfd;
	println("timerfd: %d, delay: %d", timerfd, ev->time_delay);	
	return 0;
}



int timer_start(event *ev)
{
	check(ev != NULL, return -1);
	struct itimerspec ts;
	ts.it_value.tv_sec = ev->time_delay;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	timerfd_settime(ev->fd, 0, &ts, NULL);
	return 0;
}


int timer_stop(event *ev)
{
	check(ev != NULL, return -1);
	struct itimerspec ts;
	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	timerfd_settime(ev->fd, 0, &ts, NULL);
	return 0;
}