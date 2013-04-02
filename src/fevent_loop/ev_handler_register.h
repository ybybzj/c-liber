#ifndef __EVENT_HANDLER_REGISTER_H__
#define __EVENT_HANDLER_REGISTER_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "fevent.h"
#include <system/nx.h>
int ev_set_timeout( ev_monitor *monitor,
					int priority,
					int delay,					// expire time
					int (*tm_cb) (void*),       // timeout callback
					void *arg,					// argument passed to callback
					void (*arg_free)(void*));	// argument free function

int ev_set_interval( ev_monitor *monitor,
					int priority,
					int delay,					// expire time
					int (*tm_cb) (void*),       // timeout callback
					void *arg,					// argument passed to callback
					void (*arg_free)(void*));	// argument free function

int ev_on_signal(ev_monitor *monitor,
					int priority,
					int signal,					// signal to catch
					int (*sig_cb) (int,void*),       // signal callback return 0 when done with signal catch
					void *arg,					// argument passed to callback
					void (*arg_free)(void*));	// argument free function

void ev_fs_open( ev_monitor *monitor, 									// ev_monitor
				 const char *fpath, 								// file path
				 int access_mode, 									// access mode bit-mask
				 int perm_mode, 									// permit mode bit-mask
				 void (*open_cb)(int, int, ev_monitor*));				// open callback with arguments(errno, opened_fd, ev_monitor)

void ev_fs_read(ev_monitor *monitor,												//ev_monitor
				int fd,																//opened fd
				ssize_t (*read_cb)(int, int, void*,size_t,void*, ev_monitor *),
				//read_callback with arguments(errno,buf_read_pos, available_read_size, arg_passed_in,monitor),
				//return amount of bytes consumed
				void *arg 															// arg passed in
				);
#ifdef __cplusplus
}
#endif

#endif
