#ifndef __THREAD_POOL_THREAD_H__
#define __THREAD_POOL_THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "_thread_pool.h"


int tp_thread_spawn(thr_pool *tp);
void tp_thread_queue_cancel(thr_pool *tp);
#ifdef __cplusplus
}
#endif

#endif

