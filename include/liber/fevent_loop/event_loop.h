#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__
#include "fevent.h"
#include "ev_monitor.h"
#include "ev_handler_register.h"

fevent ev_new_event();

#define EV_NOBLOCK (0x01)
#define EV_EMPTY_NOEXIT (0x02)
int ev_loop_run(ev_monitor *monitor ,int flag);
void ev_loop_stop(ev_monitor *monitor);



#define prt_evs(e,fmt, ...) printf(fmt ": %s%s%s%s,%s%s\n", ##__VA_ARGS__, e&EV_TIMEOUT ? "EV_TIMEOUT":"", \
e&EV_READ ? "EV_READ":"", e&EV_WRITE ? "EV_WRITE":"", e&EV_ERREV ? "EV_ERREV":"",e&EV_IGN ? "EV_IGN":"", e&EV_FINISHED ? "EV_FINISHED":"");

#endif
