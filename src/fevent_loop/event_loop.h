#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__
#include "fevent.h"
#include "ev_monitor.h"

fevent ev_new_event();

#define EV_NOBLOCK (0x01)
#define EV_EMPTY_NOEXIT (0x02)
int ev_loop_run(ev_monitor *monitor ,int flag);
void ev_loop_stop(ev_monitor *monitor);


int ev_set_timeout( ev_monitor *monitor,
					int priority,
					int delay,					// expire time
					int (*tm_cb) (void*),       // timeout callback
					void *arg,					// argument passed to callback
					void (*arg_free)(void*));	// argument free function

int ev_set_interval( ev_monitor *monitor,
					int priority,
					int delay,					// expire time
					int (*tm_cb) (void*),       // timeout callback
					void *arg,					// argument passed to callback
					void (*arg_free)(void*));	// argument free function

int ev_on_signal(ev_monitor *monitor,
					int priority,
					int signal,					// signal to catch
					int (*sig_cb) (int,void*),       // signal callback return 0 when done with signal catch
					void *arg,					// argument passed to callback
					void (*arg_free)(void*));	// argument free function
#define prt_evs(e,fmt, ...) printf(fmt ": %s%s%s%s,%s%s\n", ##__VA_ARGS__, e&EV_TIMEOUT ? "EV_TIMEOUT":"", \
e&EV_READ ? "EV_READ":"", e&EV_WRITE ? "EV_WRITE":"", e&EV_ERREV ? "EV_ERREV":"",e&EV_IGN ? "EV_IGN":"", e&EV_FINISHED ? "EV_FINISHED":"");

#endif
