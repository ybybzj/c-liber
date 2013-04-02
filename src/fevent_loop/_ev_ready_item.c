#include <common/dbg.h>
#include "_ev_ready_item.h"
#include "_ev_helpers.h"
ev_ready_item *ev_ready_item_create(ev_monitor *monitor, ev_callback *cb_list, int cb_len, fevent ev, ev_set events)
{
	check(monitor != NULL , errno = EINVAL; return NULL);
	ev_ready_item *ri = MALLOC(1,ev_ready_item);
	memset(ri, 0, sizeof(ev_ready_item));
	check_mem(ri != NULL, return NULL,"ev_ready_item allocation");
	ri->monitor = monitor;
	if(cb_list != NULL && cb_len > 0)
		memcpy(ri->cb_list, cb_list, sizeof(ev_callback)*cb_len);
	ri->ev = ev;
	ri->ev.events = events;
	INIT_LIST_HEAD(&ri->l_ent);
	return ri;
}

void ev_ready_item_free(ev_ready_item *ri)
{
	if(ri != NULL)
		free(ri);
}


ev_set ev_ready_item_dispatch(ev_ready_item *ri)
{

	check(ri != NULL, errno = EINVAL; return 0);
	ev_set events = ri->ev.events;
	ev_set ret_ev = 0;
	ev_callback cb;

	ev_monitor *monitor = ri->monitor;

	int idx;
	for(idx = 0; idx < _EV_EVENT_MAX - 1; idx++)
	{
		cb = ri->cb_list[idx];
		if((events&0x01) && cb)
		{
			ev_set ev = cb(ri->ev,monitor);

			if(!(ev&EV_IGN))
				ret_ev |= ev;
		}

		events = events >>1;
		if(events == 0)
			break;
	}

	return ret_ev;

}


int ev_ready_item_assign_cb(ev_ready_item *ri, va_list argList)
{
	check(ri != NULL, return -1);

	return ev_assign_cb(ri->cb_list, _EV_EVENT_MAX, argList);
}
