#ifndef __EVENT_FS_OPEN_H__
#define __EVENT_FS_OPEN_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "../../fevent.h"
void fs_open(ev_monitor *m, 									// ev_monitor
			 const char *fpath, 								// file path
			 int access_mode, 									// access mode bit-mask
			 int perm_mode, 									// permit mode bit-mask
			 void (*cb)(int, int, ev_monitor*));				// open callback with arguments(errno, opened_fd, ev_monitor)

#ifdef __cplusplus
}
#endif

#endif
