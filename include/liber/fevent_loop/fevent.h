#ifndef __EVENT_H__
#define __EVENT_H__
#include <stdint.h>
#include <string.h>
/*-------------public data type--------------------*/

typedef uint32_t ev_set;

#define _EV_EVENT_MAX 8                  //number of total event types
enum fev_type{
	EV_READ = (1<<0),
	EV_WRITE = (1<<1),
	EV_ERR = (1<<2),
	//special event type, which cannot be received by ev_callback
	EV_IGN = (1<<(_EV_EVENT_MAX + 1)),
	EV_FIN = (1<<(_EV_EVENT_MAX +2)),
	EV_UNKNOWN = 0
};

 #define EV_PRIORITY_MAX 6              //1 is highest, lowest is (EV_PRIORITY_MAX - 1); 0 is reserved for internal use
typedef struct _fevent {
	int fd;
	int priority;
	ev_set events;
	union ev_data{
		long integer;
		double decimal;
		void *ptr;
	}data;
	void (*ev_data_free)(void *);
} fevent;

typedef struct _ev_monitor ev_monitor;
typedef ev_set (*ev_callback)(fevent, ev_monitor*);
typedef struct{
	enum fev_type et;
	ev_callback cb; // return events that need to be watched next,  don't bother if return EV_FINISHED
}event_cb;
#define EV_TYPE_CB_NIL	((event_cb){.et = EV_UNKNOWN})   //argument sentinel
#define EV_TYPE_CB_IS_NIL(tc_ent)		((tc_ent).et == EV_UNKNOWN)
#define EV_RD_CB(c) ((event_cb){.et = EV_READ,.cb = c})
#define EV_WR_CB(c) ((event_cb){.et = EV_WRITE,.cb = c})
#define EV_ERR_CB(c) ((event_cb){.et = EV_ERR,.cb = c})
#endif
