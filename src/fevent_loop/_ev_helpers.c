#include "_ev_helpers.h"
#include <common/dbg.h>

int ev_assign_cb(ev_callback *cb_list, int cb_len, va_list argList)
{
	check(cb_list != NULL && cb_len >= 0, return -1);

	event_cb tc_ent = va_arg(argList,event_cb);
	while(!EV_TYPE_CB_IS_NIL(tc_ent))
	{
		ev_set events = tc_ent.et;
		int idx = 0;
		for(;idx < cb_len; idx++)
		{
			if(events & 0x01)
				*(cb_list+idx) = tc_ent.cb;
			events = events >> 1;
			if(events == 0)
				break;
		}
		tc_ent = va_arg(argList,event_cb);
	}
	return 0;
}
