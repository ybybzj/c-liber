#define _GNU_SOURCE
#include "_thread_pool.h"
#include "_tp_thread.h"
#include "_tp_job.h"
#include <common/dbg.h>
#include <common/ds/dlist.h>
#include <system/nx.h>
#include <pthread.h>


/*
 * @pool_list: process scope maintained thread pool list,
 * which means per process can have multiple thread pools.
 */
static pthread_mutex_t thr_pool_list_lock = PTHREAD_MUTEX_INITIALIZER;
static list_head pool_list = LIST_HEAD_INIT(pool_list);



/*----------private functions------*/
static void
clone_attributes(pthread_attr_t *new_attr, pthread_attr_t *old_attr)
{
	struct sched_param param;
	void *addr;
	size_t size;
	int value;
	(void) pthread_attr_init(new_attr);
	if (old_attr != NULL) {
		(void) pthread_attr_getstack(old_attr, &addr, &size);
		/* don’t allow a non-NULL thread stack address */
		(void) pthread_attr_setstack(new_attr, NULL, size);
		(void) pthread_attr_getscope(old_attr, &value);
		(void) pthread_attr_setscope(new_attr, value);
		(void) pthread_attr_getinheritsched(old_attr, &value);
		(void) pthread_attr_setinheritsched(new_attr, value);
		(void) pthread_attr_getschedpolicy(old_attr, &value);
		(void) pthread_attr_setschedpolicy(new_attr, value);
		(void) pthread_attr_getschedparam(old_attr, &param);
		(void) pthread_attr_setschedparam(new_attr, &param);
		(void) pthread_attr_getguardsize(old_attr, &size);
		(void) pthread_attr_setguardsize(new_attr, size);
	}
	/* make all pool threads be detached threads */
	(void) pthread_attr_setdetachstate(new_attr, PTHREAD_CREATE_DETACHED);

}

/*-------------public functions------------*/

thr_pool *tp_create(uint_t t_num_min, uint_t t_num_max, int timeout_sec, pthread_attr_t *attr)
{
	ERR_CLEAN_INIT();

	check(t_num_min <= t_num_max, errno = EINVAL; goto onerr);

	thr_pool *tp = MALLOC(1,thr_pool);
	check(tp != NULL, errno = ENOMEM; goto onerr);
	ERR_CLEAN_PUSH(0,tp,tp);

	//initializing tp
	INIT_LIST_HEAD(&tp->l_ent);
	tp->t_min = t_num_min ? t_num_min: T_NUM_MIN_DEFAULT;
	tp->t_max = t_num_max ? t_num_max: T_NUM_MAX_DEFAULT;
	tp->timeout_sec = timeout_sec < 0 ? T_TIMEOUT_DEFAULT : timeout_sec;
	tp->t_total = 0;

	INIT_LIST_HEAD(&tp->activeq);
	INIT_LIST_HEAD(&tp->idleq);
	INIT_LIST_HEAD(&tp->jobq);

	check_t(pthread_mutex_init(&tp->pool_mtx,NULL), goto onerr);
	ERR_CLEAN_PUSH(1,&tp->pool_mtx, mtx);

	check_t(pthread_cond_init(&tp->pool_workcv, NULL), goto onerr);
	ERR_CLEAN_PUSH(2,&tp->pool_workcv, workcv);
	check_t(pthread_cond_init(&tp->pool_waitcv, NULL), goto onerr);
	ERR_CLEAN_PUSH(2,&tp->pool_waitcv, waitcv);
	check_t(pthread_cond_init(&tp->pool_exitcv, NULL), goto onerr);
	ERR_CLEAN_PUSH(2,&tp->pool_exitcv, exitcv);

	//modifying global thread pool list
	(void)pthread_mutex_lock(&thr_pool_list_lock);
	list_push(&tp->l_ent, &pool_list);
	(void)pthread_mutex_unlock(&thr_pool_list_lock);

	clone_attributes(&tp->thr_attr,attr);
	tp->state = 0;

	return tp;

	onerr:
		ERR_CLEAN_BEGIN
		case 0:
			{
				free((thr_pool*)cleanup_resource);
				break;
			}
		case 1:
			{
				pthread_mutex_destroy((pthread_mutex_t*)cleanup_resource);
				break;
			}
		case 2:
			{
				pthread_cond_destroy((pthread_cond_t*)cleanup_resource);
				break;
			}
		ERR_CLEAN_END
		return NULL;
}

int tp_queue(thr_pool *tp, job_cb j_cb, void *arg)
{
	/*
	 * produce job to job list
	 */

	check(j_cb != NULL, errno = EINVAL; return -1);
	(void) pthread_mutex_lock(&tp->pool_mtx);

	check(tp_job_equeue(tp,j_cb,arg) != -1, pthread_mutex_unlock(&tp->pool_mtx);return -1);

	/*
	 * check the pool state to fullfill the workers requirement
	 */

	 if(list_empty(&tp->idleq) && tp->t_total < tp->t_max && !list_empty(&tp->jobq))
	 {
	 	check(tp_thread_spawn(tp) != -1, pthread_mutex_unlock(&tp->pool_mtx);return -1);
	 }
	 (void)pthread_mutex_unlock(&tp->pool_mtx);
	 return 0;

}

void tp_wait(thr_pool *tp)
{
	(void) pthread_mutex_lock(&tp->pool_mtx);
	pthread_cleanup_push((pthread_cleanup_fn)pthread_mutex_unlock, &tp->pool_mtx);
	while (!(list_empty(&tp->activeq) && list_empty(&tp->jobq))) {
		tp->state |= POOL_WAIT;
		(void) pthread_cond_wait(&tp->pool_waitcv, &tp->pool_mtx);
	}
	pthread_cleanup_pop(1);
	/* pthread_mutex_unlock(&pool->pool_mutex); */

}

void tp_cancel(thr_pool *tp)
{
	(void) pthread_mutex_lock(&tp->pool_mtx);
	tp->state |= POOL_CANCEL;

	tp_job_queue_destroy(tp);

	tp_thread_queue_cancel(tp);
	(void) pthread_mutex_unlock(&tp->pool_mtx);

	tp_wait(tp);

	(void) pthread_mutex_lock(&tp->pool_mtx);
	pthread_cleanup_push((pthread_cleanup_fn)pthread_mutex_unlock, &tp->pool_mtx);
	while (tp->t_total != 0) {
		(void) pthread_cond_wait(&tp->pool_exitcv, &tp->pool_mtx);
	}
	pthread_cleanup_pop(1);
}

void tp_destroy(thr_pool *tp)
{
	if(tp == NULL) return;

	tp_cancel(tp);

	//reclaim thread pool
	(void)pthread_mutex_lock(&thr_pool_list_lock);
	list_del(&tp->l_ent);
	(void)pthread_mutex_unlock(&thr_pool_list_lock);

	pthread_mutex_destroy(&tp->pool_mtx);
	pthread_cond_destroy(&tp->pool_workcv);
	pthread_cond_destroy(&tp->pool_waitcv);
	pthread_cond_destroy(&tp->pool_exitcv);
	free(tp);
}


int tp_is_busy(thr_pool *tp)
{
	int ret;
	(void) pthread_mutex_lock(&tp->pool_mtx);
	ret = !list_empty(&tp->jobq) || !list_empty(&tp->activeq);
	(void) pthread_mutex_unlock(&tp->pool_mtx);
	return ret;
}

/*-----------protected funtions-------------*/
void tp_notify_waiter(thr_pool *tp)
{
	if (list_empty(&tp->activeq) && list_empty(&tp->jobq)) {
		tp->state &= ~POOL_WAIT;
		(void) pthread_cond_broadcast(&tp->pool_waitcv);
	}
}


//status inspectors
void thr_pool_stat_print(thr_pool *tp)
{

	(void)pthread_mutex_lock(&tp->pool_mtx);
	println("|-----thread pool status-----|");
	println("thr_pool address: %p",tp);
	println("t_min : %u",tp->t_min);
	println("t_max : %u",tp->t_max);
	println("t_total : %u",tp->t_total);
	println("timeout_sec : %d",tp->timeout_sec);
	println("active_queue : %d",list_size(&tp->activeq));
	println("idle_queue : %d",list_size(&tp->idleq));
	println("job_queue : %d",list_size(&tp->jobq));
	println("|----------------------------|");
	(void)pthread_mutex_unlock(&tp->pool_mtx);

}
