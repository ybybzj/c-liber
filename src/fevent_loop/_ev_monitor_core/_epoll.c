#include "_epoll.h"
#include <common/dbg.h>
#include <system/nx.h>
struct _ev_monitor_handle{
	int epfd;
};

struct _ev_monitor_handle *epoll_get_monitor_handle()
{
	struct _ev_monitor_handle *handle = MALLOC(1, struct _ev_monitor_handle);
	check_mem(handle != NULL, return NULL, "epoll_get_monitor_handle allocation");

	check((handle->epfd = epoll_create(10)) != -1, free(handle); return NULL);
	return handle;
}

void epoll_free_monitor_handle(struct _ev_monitor_handle *handle)
{
	if(handle != NULL)
	{
		close(handle->epfd);
		free(handle);
	}
}

/*-----------------epoll helpers---------------------------*/
static inline struct epoll_event make_epev(uint32_t events, ev_watch_item *w)
{
	struct epoll_event epev;
	epev.events = events;
	epev.data.ptr = w;
	return epev;
}


static inline uint32_t to_ep_events(ev_set custom)
{
	uint32_t events = 0;
	if(custom&EV_READ)
		events |= EPOLLIN;
	if(custom&EV_WRITE)
		events |= EPOLLOUT;
	if(custom&EV_ERR)
		events |= EPOLLPRI;
	return events|EPOLLONESHOT;
}

static inline ev_set to_custom_events(uint32_t ep_events, ev_set old_cevts)
{
	uint32_t events = 0;
	if(old_cevts&EV_READ && (ep_events&EPOLLIN ||ep_events&EPOLLHUP || ep_events&EPOLLERR))
		events |= EV_READ;
	if(old_cevts&EV_WRITE && (ep_events&EPOLLOUT || ep_events&EPOLLERR))
		events |= EV_WRITE;
	if(ep_events&EPOLLPRI)
		events |= EV_ERR;
	return events;
}
/*-------------------------------------------------------------*/

int epoll_add(struct _ev_monitor_handle *h, ev_watch_item *w)
{
	check(h != NULL && w != NULL, errno = EINVAL; return -1);
	uint32_t events = to_ep_events(w->ev.events);
	check(events != 0, return -1);
	struct epoll_event epev = make_epev(events, w);

	return epoll_ctl(h->epfd, EPOLL_CTL_ADD, w->ev.fd, &epev);
}

int epoll_mod(struct _ev_monitor_handle *h, ev_watch_item *w, fevent ev)
{
	check(h != NULL && w != NULL, errno = EINVAL; return -1);


	uint32_t evts = to_ep_events(ev.events);
	if(evts == 0)
		return 0;

	struct epoll_event epev = make_epev(evts, w);

	return epoll_ctl(h->epfd, EPOLL_CTL_MOD, w->ev.fd, &epev);
}

int epoll_del(struct _ev_monitor_handle *h, ev_watch_item *w)
{
	check(h != NULL && w != NULL, errno = EINVAL; return -1);
	struct epoll_event epev;
	return epoll_ctl(h->epfd, EPOLL_CTL_DEL, w->ev.fd, &epev);
}


int epoll_wait_events(struct _ev_monitor_handle *h, ev_handle_event *he_list, size_t len, int timeout_s)
{
	check(h != NULL && he_list != NULL, errno = EINVAL; return -1);
	struct epoll_event evlist[len];

	int nready = epoll_wait(h->epfd, evlist, len, timeout_s * 1000);
	int i = 0;
	for(; i < nready; i++)
	{
		ev_watch_item *w = (ev_watch_item *)evlist[i].data.ptr;
		he_list[i].events = to_custom_events(evlist[i].events, w->ev.events);
		he_list[i].fd = w->ev.fd;
 	}
	return nready;
}