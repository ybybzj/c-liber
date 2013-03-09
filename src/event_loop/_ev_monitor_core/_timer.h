#ifndef __EVENT_TIMER_H__
#define __EVENT_TIMER_H__
#include "../event_loop.h"

int timer_prepare(event *ev);
int timer_start(event *ev);
#endif