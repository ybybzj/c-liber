#ifndef __EVENT_SIGNAL_H__
#define __EVENT_SIGNAL_H__
#include "../event_loop.h"
int add_signal_ev( ev_monitor *monitor,
					int priority,
					int signal,					// expire time
					int (*sig_cb) (int,void*),       // signal callback return 0 when done with signal catch
					void *arg,					// argument passed to callback
					void (*arg_free)(void*));	// argument free function

void detach_signal(int sig);	// donnot monitor signal sig
#endif