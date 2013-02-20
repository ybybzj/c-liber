/* Author: Jack Zheng (ybybzj@gmail.com)
 * 
 * This is a thread pool implementation based on  
 * "Multithreaded Programming Guide" from Oracle.
 */
#ifndef __PTHREAD_POOL_H__
#define __PTHREAD_POOL_H__
#include <pthread.h>
#include <stdint.h>

#ifndef uint_t
#define uint_t uint32_t
#endif
typedef struct thread_pool *thr_pool_t; // thread pool struct pointer
typedef struct thread_ent thread_ent_t; // thread info struct
/**
 * thr_pool_initialize - create a thread pool
 * @tpp:			the pointer to thr_pool_t type data which to be assigned a new create thread pool.
 * @t_num_min:		the number of the least threads required by the pool. if 0, use default value.
 * @t_num_max:		the number of the most threads required by the pool. if 0, use default value.
 * @timeout_sec:	the time span of the idle thread will wait until terminate. if < 0, use default value.
 * @attr:			not useful right now
 * return:			0 on success, otherwise on error
 * (*tpp) will be a valid pointer to thread pool struct, or NULL on error.
 */
int thr_pool_init(thr_pool_t *tpp, uint_t t_num_min, uint_t t_num_max, int timeout_sec, pthread_attr_t *attr);

/**
 * thr_pool_queue - create a job and put it into pool's job queue
 * @tp:				the pointer to the thread pool.
 * @job_fn:			the job routine, info of the worker thread will be passed in as second argument 
 * @arg:			the data pass to the job routine.
 * return:			0 on success, otherwise on error
 * Note: thread_ent_t pointer data should never be used in free function, and only use helper function to manipulate it.
 */
int thr_pool_queue(thr_pool_t tp, void *(*job_fn)(void *,const thread_ent_t *), void *arg);

/**
 * thr_pool_wait - block until all the jobs are finished and no more active threads in the pool
 * @tp:				the pointer to the thread pool.
 */
void thr_pool_wait(thr_pool_t tp);

/**
 * thr_pool_destroy - cancel the working threads and reap all the threads in the pool, then reclaim the thread pool
 * @tpp:			the pointer to the thr_pool_t type data.
 * after this, *tpp will equal to NULL.
 */
void thr_pool_destroy(thr_pool_t *tpp);

//helpers
void thr_pool_stat_print(thr_pool_t tp);
void thread_ent_stat_print(thread_ent_t *t);
#endif