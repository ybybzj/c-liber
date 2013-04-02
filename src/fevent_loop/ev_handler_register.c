#include "ev_handler_register.h"
#include <common/dbg.h>
#include "_ev_events/_timeout.h"
#include "_ev_events/_signal.h"
#include "_ev_events/_ev_fs.h"

int ev_set_timeout( ev_monitor *monitor,
					int priority,
					int delay,
					int (*tm_cb) (void*),
					void *arg,
					void (*arg_free)(void*))
{
	check(monitor != NULL, return -1);
	return add_timeout_ev( monitor,
						   priority,
						   delay,
						   tm_cb,
						   arg,
						   arg_free,
						   EV_TIMEOUT_ONCE);
}

int ev_set_interval( ev_monitor *monitor,
					int priority,
					int delay,
					int (*tm_cb) (void*),
					void *arg,
					void (*arg_free)(void*))
{
	check(monitor != NULL, return -1);

	return add_timeout_ev( monitor,
						   priority,
						   delay,
						   tm_cb,
						   arg,
						   arg_free,
						   EV_TIMEOUT_PERIOD);
}


int ev_on_signal(ev_monitor *monitor,
					int priority,
					int signal,					// expire time
					int (*sig_cb) (int,void*),       // signal callback return 0 when done with signal catch
					void *arg,					// argument passed to callback
					void (*arg_free)(void*))
{
	check(monitor != NULL, return -1);

	return add_signal_ev( monitor,
						 priority,
						 signal,
						 sig_cb,
						 arg,
					     arg_free);

}


void ev_fs_open( ev_monitor *monitor,
				 const char *fpath,
				 int access_mode,
				 int perm_mode,
				 void (*open_cb)(int, int, ev_monitor*))
{
	check(monitor != NULL, return);
	fs_open( monitor,
			 fpath,
			 access_mode,
			 perm_mode,
			 open_cb);
}

void ev_fs_read(ev_monitor *monitor,												//ev_monitor
				int fd,																//opened fd
				ssize_t (*read_cb)(int, int, void*,size_t,void*, ev_monitor *),
				//read_callback with arguments(errno,buf_read_pos, available_read_size, arg_passed_in,monitor),
				//return size of bytes consumed
				void *arg 															// arg passed in
				)
{
	check(monitor != NULL, return);
	fs_read(
			monitor,
			fd,
			read_cb,
			arg);
}
