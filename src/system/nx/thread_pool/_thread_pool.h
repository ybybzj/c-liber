#ifndef __PTHREAD_POOL_PRIVATE_H__
#define __PTHREAD_POOL_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "../thread_pool.h"
#include <common/ds/dlist.h>
#define T_NUM_MIN_DEFAULT 5
#define T_NUM_MAX_DEFAULT 100
#define T_TIMEOUT_DEFAULT 2
/* pool_flags */

#define POOL_BUSY 0X01		/* has job waiting, or has active working threads*/

#define POOL_WAIT 0x02        /* waiting in thr_pool_wait() */
#define POOL_CANCEL 0x04     /* pool is being canceled */


struct _thread_pool
{
	dl_ent_t l_ent;
	uint_t t_min;
	uint_t t_max;
	int timeout_sec;
	int state;				// pool state flags only one state can be assigned at any time

	pthread_mutex_t pool_mtx;
	uint_t t_total;  	// total number of threads in the pool

	pthread_cond_t pool_workcv; /* inform idle workers that a new job joined to job list */
	pthread_cond_t pool_waitcv; /* synchronization in pool_wait() */
	pthread_cond_t pool_exitcv; /* check for  last thread in the pool has exited*/

	pthread_attr_t thr_attr;

	/*thread queue*/
	list_head activeq;      // threads running jobs
	list_head idleq;		// idle threads waiting for new jobs


	/*job queue*/
	list_head jobq;
};

void tp_notify_waiter(thr_pool *tp);
#ifdef __cplusplus
}
#endif

#endif
