#ifndef __EVENT_FS_READ_H__
#define __EVENT_FS_READ_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "../../fevent.h"
#include <system/nx.h>
void fs_read(
			ev_monitor *monitor,												//ev_monitor
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
