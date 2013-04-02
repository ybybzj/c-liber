#include <common/dbg.h>
#include <system/nx.h>
#include <system/nx/io/io.h>
#include "../../event_loop.h"
#include "_open.h"

typedef struct _fs_open_data
{
	char *fpath;
	int access;
	int perm;
	void (*cb)(int err, int fd, ev_monitor *monitor);
}fs_open_data;

static void fs_open_data_free(void *arg)
{
	// debug("fs_open_data_free");
	if(arg != NULL)
		free((fs_open_data*)arg);
}


static ev_set on_open(fevent ev, ev_monitor *monitor)
{
	fs_open_data *fdata = (fs_open_data *)ev.data.ptr;
	int fd;
	fd = open(fdata->fpath, fdata->access, fdata->perm);
	if(fd == -1)
	{
		fdata->cb(errno,-1, monitor);
	}else
		fdata->cb(0, fd, monitor);

	ev.ev_data_free(ev.data.ptr);
	return EV_FIN;

}


void fs_open(ev_monitor *m,
			 const char *fpath,
			 int access_mode,
			 int perm_mode,
			 void (*cb)(int, int, ev_monitor*))
{
	check(fpath != NULL, return);
	fevent ev = ev_new_event();
	fs_open_data *fdata = MALLOC(1,fs_open_data);
	check(fdata != NULL, return);
	fdata->fpath = (char*)fpath;
	fdata->access = access_mode;
	fdata->perm = perm_mode;
	fdata->cb = cb;
	ev.events = EV_ASYNC;
	ev.data.ptr = fdata;
	ev.ev_data_free = fs_open_data_free;
	ev_monitor_ctl(m, EV_CTL_ADD, ev, EV_ASYNC_CB(on_open));
}
