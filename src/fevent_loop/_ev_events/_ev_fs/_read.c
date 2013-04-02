#include "_read.h"
#include "../../event_loop.h"
#include <common/dbg.h>
#include <system/nx.h>
#include <system/nx/io/io.h>
#include <system/nx/io/ring_buffer.h>

#define EV_FS_READ_BUF_SIZE (8*(1<<10)) //8k
typedef struct _fs_read_data
{
	int fd;
	ring_buffer_t *rbuf;
	int eof;
	int err;
	ssize_t (*cb)(int, int, void*,size_t,void*,ev_monitor *);
	void *arg;
}fs_read_data;

static void fs_read_data_free(void *arg)
{
	if(arg != NULL)
	{
		fs_read_data *rdata  = (fs_read_data*)arg;
		// close(rdata->fd);
		(void)ring_buffer_free(rdata->rbuf);
		free(rdata);
	}
}

static ev_set on_read(fevent ev, ev_monitor *monitor)
{
	fs_read_data *rdata = (fs_read_data*)ev.data.ptr;
	ring_buffer_t *rbuf = rdata->rbuf;


	if(ring_buffer_free_space(rbuf) > 0 && rdata->eof == 0 && rdata->err == 0)
	{
	  	ssize_t n;
	  	if((n = read(ev.fd, ring_buffer_write_pos(rbuf), ring_buffer_free_space(rbuf))) > 0)
	    {
	    	ring_buffer_commit_write(rbuf, n);
	    }
	    else
	    {
	    	if (errno == EINTR)
	   		{
				debug("[read] signal caught!!!");
				return ev.events;
			}
	    	else
	    	{
	    		if(n == 0)
	    		{
	    			rdata->eof = 1;
			    	debug_Y("- EOF -");
	    		}
			    else
			    {
			    	rdata->err = errno;
					debug_R("read failed!");
			    }
	    	}

	    }
	}


	// debug("ring_buffer_data_count : %d",ring_buffer_data_count(rbuf));
	if(ring_buffer_data_count(rbuf) > 0 && !(rdata->err))
	{
	    ssize_t nread = rdata->cb(0,rdata->fd, ring_buffer_read_pos(rbuf),ring_buffer_data_count(rbuf),rdata->arg, monitor);
		if(nread >= 0)
		{
			ring_buffer_commit_read(rbuf, nread);
		}
		else
			ev.ev_data_free(ev.data.ptr);

		return nread >= 0 ? ev.events: EV_FIN;
	}else
	{
		if(rdata->err)
		{
			(void)rdata->cb(rdata->err,rdata->fd,NULL,0,rdata->arg, monitor);
		}
		if(rdata->eof)
		{
			(void)rdata->cb(0,rdata->fd,NULL,0,rdata->arg, monitor);
		}
		ev.ev_data_free(ev.data.ptr);
		return EV_FIN;
	}

}


void fs_read( ev_monitor *monitor,
			  int fd,
			  ssize_t (*read_cb)(int, int, void*,size_t, void*, ev_monitor*),
			  void *arg)
{
	fevent ev = ev_new_event();
	fs_read_data *rdata = MALLOC(1,fs_read_data);
	check(rdata != NULL, return);
	rdata->rbuf = ring_buffer_create(EV_FS_READ_BUF_SIZE);
	check(rdata->rbuf != NULL, free(rdata); return);

	rdata->fd = fd;
	rdata->eof = 0;
	rdata->err = 0;
	rdata->cb = read_cb;
	rdata->arg = arg;
	ev.fd = fd;
	ev.events = EV_ASYNC;
	ev.data.ptr = rdata;
	ev.ev_data_free = fs_read_data_free;
	ev_monitor_ctl(monitor, EV_CTL_ADD, ev, EV_ASYNC_CB(on_read));
}
