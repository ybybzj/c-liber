#ifndef __EVENT_TIMEOUT_H__
#define __EVENT_TIMEOUT_H__
#include "../event_loop.h"
#define EV_TIMEOUT_PERIOD 1
#define EV_TIMEOUT_ONCE 0

int add_timeout_ev( ev_monitor *monitor,
					int priority,
					int delay,					// expire time
					int (*tm_cb) (void*),       // timeout callback
					void *arg,					// argument passed to callback
					void (*arg_free)(void*),	// argument free function
					int is_interval);			// is a periodic timer, or just once

#endif
