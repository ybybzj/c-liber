#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__
#include <stdint.h>


/*-------------public data type--------------------*/

typedef uint32_t ev_set;

#define _EV_EVENT_MAX 8                  //number of total event types
enum ev_type{
	EV_READ = (1<<0),
	EV_WRITE = (1<<1),
	EV_ERREV = (1<<(_EV_EVENT_MAX - 1)),
	//special event type, which cannot be received by ev_callback
	EV_IGN = (1<<(_EV_EVENT_MAX + 1)),
	EV_FINISHED = (1<<(_EV_EVENT_MAX +2)),
	EV_UNKNOWN = 0
};


typedef struct _ev_monitor ev_monitor;

 #define EV_PRIORITY_MAX 5              //0 is highest
typedef struct _event {
	int fd;
	int priority;
	ev_set events;
	union ev_data{
		long i;
		double d;
		void *ptr;
	}data; 
	void (*ev_data_free)(void *);
} event;
 

typedef ev_set (*ev_callback)(event, ev_monitor*);
typedef struct{
	enum ev_type et;
	ev_callback cb; // return events that need to be watched next,  don't bother if return EV_FINISHED
}event_cb;
#define EV_TYPE_CB_NIL	((event_cb){.et = EV_UNKNOWN})   //argument sentinel
#define EV_TYPE_CB_IS_NIL(tc_ent)		((tc_ent).et == EV_UNKNOWN)
#define EV_CB(e,c) ((event_cb){.et = e,.cb = c})




ev_monitor *ev_monitor_create();

#define EV_CTL_ADD (0x01)
#define EV_CTL_DEL (0x02)
#define EV_CTL_MOD (0x04)
int ev_monitor_ctl_f(ev_monitor *monitor, int flag, event ev, ...);
#define ev_monitor_ctl(m, f, e, ...) \
	ev_monitor_ctl_f(m, f, e, ##__VA_ARGS__, EV_TYPE_CB_NIL)

#define EV_NOBLOCK (0x01)
#define EV_EMPTY_NOEXIT (0x02)
int ev_loop_run(ev_monitor *monitor ,int flag);
void ev_loop_stop(ev_monitor *monitor);

void ev_monitor_free(ev_monitor *monitor);

// #define prt_evs(e,fmt, ...) printf(fmt ": %s%s%s,%s%s\n", ##__VA_ARGS__, e&EV_READ ? "EV_READ":"", e&EV_WRITE ? "EV_WRITE":"", e&EV_ERREV ? "EV_ERREV":"",
// 		e&EV_IGN ? "EV_IGN":"", e&EV_FINISHED ? "EV_FINISHED":"");

#endif