#ifndef __EVENT_MONITOR_H__
#define __EVENT_MONITOR_H__
#include "fevent.h"


ev_monitor *ev_monitor_create();
void ev_monitor_free(ev_monitor *monitor);

#define EV_CTL_ADD (0x01)
#define EV_CTL_DEL (0x02)
#define EV_CTL_MOD (0x04)
int ev_monitor_ctl_f(ev_monitor *monitor, int flag, fevent ev, ...);
#define ev_monitor_ctl(m, f, e, ...) \
	ev_monitor_ctl_f(m, f, e, ##__VA_ARGS__, EV_TYPE_CB_NIL)


#endif
