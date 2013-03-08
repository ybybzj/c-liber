#ifndef __EVENT_HANDLE_EVENT_H__
#define __EVENT_HANDLE_EVENT_H__
#include "../event_loop.h"
typedef struct _ev_handle_event
{
	ev_set events;
	int fd;
}ev_handle_event;
#endif