#ifndef __EVENT_HELPERS_H__
#define __EVENT_HELPERS_H__
#include "event_loop.h"
#include <stdarg.h>
int ev_assign_cb(ev_callback *cb_list, int cb_len, va_list argList);

#endif
