#include <common/dbg.h>
#include "_ev_ready_item.h"

ev_ready_item *ev_ready_item_create(ev_monitor *monitor, ev_watch_item *w, ev_set events)
{
	check(monitor != NULL && w != NULL, errno = EINVAL; return NULL);
	ev_ready_item *ri = MALLOC(1,ev_ready_item);
	check_mem(ri != NULL, return NULL,"ev_ready_item allocation");
	ri->monitor = monitor;
	ri->w = w;
	

	check(w != NULL, free(ri); return NULL);
	
	ri->ev = w->ev;
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
	ev_callback cb_list[_EV_EVENT_MAX];

	ev_watch_item_cb_copy(ri->w, cb_list, _EV_EVENT_MAX);

	ev_monitor *monitor = ri->monitor;
	if(events&EV_ERREV)
	{
		cb = cb_list[_EV_EVENT_MAX - 1];
		if(cb)
			cb(ri->ev, monitor);
		return 0;
	}else
	{
		int idx;
		for(idx = 0; idx < _EV_EVENT_MAX - 1; idx++)
		{
			cb = cb_list[idx];
			
			if((events&0x01) && cb)
			{
				ev_set ev = cb(ri->ev,monitor);
				if(!(ev&EV_FINISHED))
					ret_ev |= ev;
			}
			
			events = events >>1;
			if(events == 0)
				break;
		}

		return ret_ev;
	}	
}