#ifndef __PTHREAD_POOL_H__
#define __PTHREAD_POOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <stdint.h>

#ifndef uint_t
#define uint_t uint32_t
#endif

typedef struct _thread_pool thr_pool;
typedef void (*job_cb)(void *);
typedef void (*pthread_cleanup_fn)(void *);

/**
 * tp_create - create a thread pool
 * @t_num_min:		the number of the least threads required by the pool. if 0, use default value.
 * @t_num_max:		the number of the most threads required by the pool. if 0, use default value.
 * @timeout_sec:	the time span of the idle thread will wait until terminate. if < 0, use default value.
 * @attr:			the pointer to an attribute object which specify behavior that is different from default thread creation behavior
 * return:			new thr_pool pointer on success, NULL on error
 */
thr_pool *tp_create(uint_t t_num_min, uint_t t_num_max, int timeout_sec, pthread_attr_t *attr);

/**
 * tp_queue - create a job and put it into pool's job queue
 * @tp:				the pointer to the thread pool.
 * @j_cb:			the job routine, info of the worker thread will be passed in as second argument
 * @arg:			the data pass to the job routine.
 * return:			0 on success, otherwise on error
 * Note: thread_ent_t pointer data should never be used in free function, and only use helper function to manipulate it.
 */
int tp_queue(thr_pool *tp, job_cb j_cb, void *arg);

/**
 * tp_wait - block until all the jobs are finished and no more active threads in the pool
 * @tp:				the pointer to the thread pool.
 */
void tp_wait(thr_pool *tp);

/**
 * tp_cancel - block until all the ongoing jobs are canceled, no more active threads in the pool
 *			   and last thread in the pool has exited.
 * @tp:				the pointer to the thread pool.
 */
void tp_cancel(thr_pool *tp);

/**
 * tp_destroy - cancel the working threads and reap all the threads in the pool, then reclaim the thread pool
 * @tp:			the pointer to the thr_pool type data.
 */
void tp_destroy(thr_pool *tp);

/**
 * tp_is_busy - return true if thread pool has unfinished jobs or active threads left
 * @tp:					  the pointer to the thread pool.
 */
int tp_is_busy(thr_pool *tp);

//status inspectors
void thr_pool_stat_print(thr_pool *tp);
#ifdef __cplusplus
}
#endif

#endif
