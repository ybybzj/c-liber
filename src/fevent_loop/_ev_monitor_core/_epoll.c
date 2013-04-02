#include "_epoll.h"
#include <common/dbg.h>
#include <system/nx.h>
// #include <system/nx/io/io.h>
struct _ev_monitor_handle{
	int epfd;
	int pfd[2];				// pipe use for monitor change notification
};
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
/*----------------------------------------------------------------------------------*/
struct _ev_monitor_handle *epoll_get_monitor_handle()
{

	ERR_CLEAN_INIT();
	struct _ev_monitor_handle *handle = MALLOC(1, struct _ev_monitor_handle);
	check_mem(handle != NULL, return NULL, "epoll_get_monitor_handle allocation");
	ERR_CLEAN_PUSH(0,handle,handle);

	check((handle->epfd = epoll_create(10)) != -1, goto onerr);
	ERR_CLEAN_PUSH(1,&handle->epfd,epfd);

	check(pipe(handle->pfd) != -1, goto onerr);
	ERR_CLEAN_PUSH(1,&handle->pfd[0],pfd0);
	ERR_CLEAN_PUSH(1,&handle->pfd[1],pfd1);
	check(make_unblock(handle->pfd[0]) != -1, goto onerr);
	check(make_unblock(handle->pfd[1]) != -1, goto onerr);

	struct epoll_event epev;
	epev.events = EPOLLIN;
	epev.data.ptr = handle;
	check(epoll_ctl(handle->epfd, EPOLL_CTL_ADD, handle->pfd[0], &epev) != -1, goto onerr);
	return handle;

onerr:
	ERR_CLEAN_BEGIN
		case 0:
			{
				free((struct _ev_monitor_handle *)cleanup_resource);
				break;
			}
		case 1:
			{
				int *fdp = (int*)cleanup_resource;

				close(*fdp);
				break;
			}

	ERR_CLEAN_END
	return NULL;
}

void epoll_free_monitor_handle(struct _ev_monitor_handle *handle)
{
	if(handle != NULL)
	{
		struct epoll_event epev;
		epoll_ctl(handle->epfd, EPOLL_CTL_DEL,handle->pfd[0], &epev);
		close(handle->epfd);
		close(handle->pfd[0]);
		close(handle->pfd[1]);
		free(handle);
	}
}


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
		if((unsigned char*)evlist[i].data.ptr == (unsigned char*)h)
		{
			// debug("epoll_notify");
			check(epoll_clear_notification(h) != -1, return -1);
			he_list[i].events = 0;
			he_list[i].fd = -1;
		}else
		{
			ev_watch_item *w = (ev_watch_item *)evlist[i].data.ptr;
			he_list[i].events = to_custom_events(evlist[i].events, w->ev.events);
			he_list[i].fd = w->ev.fd;
		}

 	}
	return nready;
}

int epoll_notify(struct _ev_monitor_handle *h)
{
	check(h != NULL, return -1);
	int wfd = h->pfd[1];
	if(write(wfd, "x", 1) == -1 && !(errno == EAGAIN || errno == EWOULDBLOCK))
	{
		print_err("[epoll_notify]: write to wfd(%d) failed!", wfd);
		return -1;
	}
	return 0;
}

int epoll_clear_notification(struct _ev_monitor_handle *h)
{
	check(h != NULL, return -1);
	int rfd = h->pfd[0];
	char ch;
	for(;;)
	{
		if( read(rfd, &ch, 1) == -1 )
		{
			if(errno == EINTR)
				continue;
			else if(errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else
			{
				print_err("[epoll_clear_notification] read from %d failed!", rfd);
				return -1;
			}
		}
	}
	return 0;
}
