#define _GNU_SOURCE
//#define TP_DEBUG
#include "common.h"
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <common/ds/dlist.h>
#include "pthread_pool.h"

#define T_NUM_MIN_DEFAULT 5
#define T_NUM_MAX_DEFAULT 100
#define T_TIMEOUT_DEFAULT 2

#define T_EXIT_SUCCESS ((void*)0)
#define T_EXIT_FAILED(errno) ((void*)((long long)errno))

#define TP_BLOCK_ALLSIG() \
sigset_t __org_sigset__;\
sigfillset(&fullsigmask);\
pthread_sigmask(SIG_SETMASK, &fullsigmask, &__org_sigset__)

#define TP_UNBLOCK_SIG() \
pthread_sigmask(SIG_SETMASK, &__org_sigset__, NULL)

typedef void (*pthread_cleanup_fn)(void *);
struct thread_pool
{
	dl_ent_t list_ent;
	uint_t t_num_min;
	uint_t t_num_max;
	int timeout_sec;
	uint_t t_num_curr;  	// total number of threads in all three lists
	uint_t t_num_idle;

	pthread_mutex_t pool_mtx;
	pthread_cond_t pool_joincv; /* synchronization in pool_destory() */
	pthread_cond_t pool_workcv; /* inform idle workers that a new job joined to job list */
	pthread_cond_t pool_waitcv; /* synchronization in pool_wait() */

	pthread_attr_t *thr_attr;

	/*thread lists*/
	list_head active_list;      // threads running jobs
	list_head idle_list;		// idle threads waiting for new jobs
	list_head unjoined_list;	// dead threads waiting for join

	/*job list*/
	list_head job_list;
};

struct thread_ent
{
	dl_ent_t list_ent;
	pthread_t tid;
	thr_pool_t tp;
};

typedef struct job
{
	dl_ent_t list_ent;
	void *(*job_fn)(void *, const thread_ent_t *);
	void *arg;
}job_t;

/*
 * @pool_list: process scope maintained thread pool list,
 * which means per process can have multiple thread pools.
 */
static pthread_mutex_t thr_pool_list_lock = PTHREAD_MUTEX_INITIALIZER;
static list_head pool_list = LIST_HEAD_INIT(pool_list);

/*
 * @fullsigmask: signal set contains all signals,
 * which is used to block all signals for each thread.
 * signal handling should be conducted by main thread.
 */
static sigset_t fullsigmask;

//static function declaration
static void worker_cleanup(thread_ent_t *t);
static void job_finish(thread_ent_t *t);
static void *create_worker(void *arg);

static pthread_attr_t *clone_attributes(pthread_attr_t *old_attr);


/**
 * thr_pool_initialize - create a thread pool
 * @tpp:				the pointer to thr_pool_t type data which to be assigned a new create thread pool.
 * @t_num_min:		the number of the least threads required by the pool. if 0, use default value.
 * @t_num_max:		the number of the most threads required by the pool. if 0, use default value.
 * @timeout_sec:	the time span of the idle thread will wait until terminate. if < 0, use default value.
 * @attr:			not useful right now
 * return:			0 on success, otherwise on error
 * (*tpp) will be a valid pointer to thread pool struct, or NULL on error.
 */
int thr_pool_init(thr_pool_t *tpp, uint_t t_num_min, uint_t t_num_max, int timeout_sec, pthread_attr_t *attr)
{
	TP_BLOCK_ALLSIG();
	ERR_CLEAN_INIT();

	int ret = 0;
	check(tpp != NULL, ret = EINVAL; goto onerr);
	check(t_num_min <= t_num_max, ret = EINVAL; goto onerr);

	thr_pool_t tp = (thr_pool_t)malloc(sizeof(struct thread_pool));
	check(tp != NULL, ret = ENOMEM; goto onerr);
	ERR_CLEAN_PUSH(0,tp,tp);

	//initializing tp
	INIT_LIST_HEAD(&tp->list_ent);
	tp->t_num_min = t_num_min ? t_num_min: T_NUM_MIN_DEFAULT;
	tp->t_num_max = t_num_max ? t_num_max: T_NUM_MAX_DEFAULT;
	tp->timeout_sec = timeout_sec < 0 ? T_TIMEOUT_DEFAULT : timeout_sec;
	tp->t_num_curr = 0;
	tp->t_num_idle = 0;

	INIT_LIST_HEAD(&tp->active_list);
	INIT_LIST_HEAD(&tp->idle_list);
	INIT_LIST_HEAD(&tp->job_list);
	INIT_LIST_HEAD(&tp->unjoined_list);

	check_t(ret = pthread_mutex_init(&tp->pool_mtx,NULL), goto onerr);
	ERR_CLEAN_PUSH(1,&tp->pool_mtx, mtx);
	check_t(ret = pthread_cond_init(&tp->pool_joincv, NULL), goto onerr);
	ERR_CLEAN_PUSH(2,&tp->pool_joincv, join);
	check_t(ret = pthread_cond_init(&tp->pool_workcv, NULL), goto onerr);
	ERR_CLEAN_PUSH(2,&tp->pool_workcv, work);
	check_t(ret = pthread_cond_init(&tp->pool_waitcv, NULL), goto onerr);
	ERR_CLEAN_PUSH(2,&tp->pool_waitcv, wait);

	//modifying global thread pool list
	(void)pthread_mutex_lock(&thr_pool_list_lock);
	list_push(&tp->list_ent, &pool_list);
	(void)pthread_mutex_unlock(&thr_pool_list_lock);
	tp->thr_attr = clone_attributes(attr);
	*tpp = tp;
	TP_UNBLOCK_SIG();
	return 0;

	onerr:
		ERR_CLEAN_BEGIN
		case 0:
			{
				free((thr_pool_t)cleanup_resource);
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
		TP_UNBLOCK_SIG();
		return ret;

}

/**
 * thr_pool_queue - create a job and put it into pool's job queue
 * @tp:				the pointer to the thread pool.
 * @job_fn:			the job routine
 * @arg:			the data pass to the job routine.
 * return:			0 on success, otherwise on error
 */

int thr_pool_queue(thr_pool_t tp, void *(*job_fn)(void *, const thread_ent_t *), void *arg)
{
	/*
	 * produce job to job list
	 */
	TP_BLOCK_ALLSIG();

	ERR_CLEAN_INIT();

	int ret = 0;

	check(job_fn != NULL, ret = EINVAL; goto onerr);
	job_t *jobp = (job_t*)malloc(sizeof(job_t));
	check(jobp != NULL, ret = ENOMEM; goto onerr);
	ERR_CLEAN_PUSH(0, jobp, jobp);

	//initialize job
	jobp->job_fn = job_fn;
	jobp->arg = arg;
	INIT_LIST_HEAD(&jobp->list_ent);



	//modifying joblist
	(void)pthread_mutex_lock(&tp->pool_mtx);
	list_push(&jobp->list_ent,&tp->job_list);
	(void)pthread_mutex_unlock(&tp->pool_mtx);

	/*
	 * check the pool state to fullfill the workers requirement
	 */
	 (void)pthread_mutex_lock(&tp->pool_mtx);
	 ERR_CLEAN_PUSH(1,&tp->pool_mtx,mtx2);

	 if(list_empty(&tp->idle_list) && tp->t_num_curr < tp->t_num_max && !list_empty(&tp->job_list))
	 {
	 	pthread_t tid;
	 	//create thread info data
	 	thread_ent_t *t = (thread_ent_t*)malloc(sizeof(thread_ent_t));
	 	check(t != NULL, ret = ENOMEM; goto onerr);
	 	ERR_CLEAN_PUSH(2,t,t);
	 	t->tp = tp;
	 	INIT_LIST_HEAD(&t->list_ent);

	 	check_t(ret = pthread_create(&tid,tp->thr_attr,create_worker,t), goto onerr);

	 	ERR_CLEAN_POP();
	 	tp->t_num_curr++;
	 }

	 ERR_CLEAN_POP();
	 (void)pthread_mutex_unlock(&tp->pool_mtx);

	 (void)pthread_cond_broadcast(&tp->pool_workcv);
	 TP_UNBLOCK_SIG();

	 return 0;
	 /*
	 * clean up on error
	 */
	 onerr:
	 	ERR_CLEAN_BEGIN
	 	case 0:
	 	{
	 		free((job_t*)cleanup_resource);
	 		break;
	 	}
	 	case 1:
	 	{
	 		(void)pthread_mutex_unlock((pthread_mutex_t*)cleanup_resource);
	 		break;
	 	}
	 	case 2:
	 	{
	 		free((thread_ent_t*)cleanup_resource);
	 		break;
	 	}
	 	ERR_CLEAN_END
	 	TP_UNBLOCK_SIG();
	 	return ret;
}

/**
 * thr_pool_wait - block until all the jobs are finished and no more active threads in the pool
 * @tp:				the pointer to the thread pool.
 */
void thr_pool_wait(thr_pool_t tp)
{
	TP_BLOCK_ALLSIG();
	(void)pthread_mutex_lock(&tp->pool_mtx);
	while(!(list_empty(&tp->active_list) && list_empty(&tp->job_list)))
	{
#ifdef TP_DEBUG
		println("[thr_pool_wait] before => active_list : %d, job_list : %d", list_size(&tp->active_list), list_size(&tp->job_list));
#endif
		(void)pthread_cond_wait(&tp->pool_waitcv, &tp->pool_mtx);
#ifdef TP_DEBUG
		println("[thr_pool_wait] after => active_list : %d, job_list : %d", list_size(&tp->active_list), list_size(&tp->job_list));
#endif
	}
	(void)pthread_mutex_unlock(&tp->pool_mtx);
	TP_UNBLOCK_SIG();
}

/**
 * thr_pool_wait_actuve - block until all the active threads are finished and the rest of jobs are dismissed.
 * @tp:					  the pointer to the thread pool.
 */
void thr_pool_wait_active(thr_pool_t tp)
{
	TP_BLOCK_ALLSIG();
	(void)pthread_mutex_lock(&tp->pool_mtx);
	while(!list_empty(&tp->job_list))
	{
		job_t *jobp = list_entry(list_shift(&tp->job_list), job_t, list_ent);
		free(jobp);
	}

	while(!(list_empty(&tp->active_list)))
	{
#ifdef TP_DEBUG
		println("[thr_pool_wait] before => active_list : %d, job_list : %d", list_size(&tp->active_list), list_size(&tp->job_list));
#endif
		(void)pthread_cond_wait(&tp->pool_waitcv, &tp->pool_mtx);
#ifdef TP_DEBUG
		println("[thr_pool_wait] after => active_list : %d, job_list : %d", list_size(&tp->active_list), list_size(&tp->job_list));
#endif
	}
	(void)pthread_mutex_unlock(&tp->pool_mtx);
	TP_UNBLOCK_SIG();
}

/**
 * thr_pool_cancel - block until all the ongoing jobs are canceled and no more active threads in the pool
 * @tp:				the pointer to the thread pool.
 */
void thr_pool_cancel(thr_pool_t tp)
{
	TP_BLOCK_ALLSIG();

	//cancel the idle and active threads
	(void)pthread_mutex_lock(&tp->pool_mtx);
#ifdef TP_DEBUG
	println("[thr_pool_destroy] BEFORE CANCEL => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d",
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
#endif
	while(!list_empty(&tp->job_list))
	{
		job_t *jobp = list_entry(list_shift(&tp->job_list), job_t, list_ent);
		free(jobp);
	}

	dl_ent_t *pos;
	list_for_each(pos, &tp->active_list)
	{
		thread_ent_t *t = list_entry(pos,thread_ent_t,list_ent);
		(void)pthread_cancel(t->tid);
	}
	list_for_each(pos, &tp->idle_list)
	{
		thread_ent_t *t = list_entry(pos,thread_ent_t,list_ent);
		(void)pthread_cancel(t->tid);
	}
	// (void)pthread_mutex_unlock(&tp->pool_mtx);

	//reap all the threads
	// (void)pthread_mutex_lock(&tp->pool_mtx);
	for(;;)
	{
		// println("destroy join list: %d, curr : %d",list_size(&tp->unjoined_list),tp->t_num_curr);
		while(list_empty(&tp->unjoined_list) && tp->t_num_curr > 0)
		{
#ifdef TP_DEBUG
			println("[thr_pool_destroy] before wait reap => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d",
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
#endif
			pthread_cond_wait(&tp->pool_joincv, &tp->pool_mtx);
#ifdef TP_DEBUG
			println("[thr_pool_destroy] after wait reap => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d",
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
#endif
		}
#ifdef TP_DEBUG
		println("[thr_pool_destroy] before reap => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d",
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
#endif
		if(!list_empty(&tp->unjoined_list))
		{
			thread_ent_t *t = list_entry(list_shift(&tp->unjoined_list), thread_ent_t, list_ent);
			tp->t_num_curr--;
			pthread_join(t->tid,NULL);
			free(t);

		}
		list_for_each(pos, &tp->idle_list) // in case of idle timeout
		{
			thread_ent_t *t = list_entry(pos,thread_ent_t,list_ent);
			(void)pthread_cancel(t->tid);
		}
#ifdef TP_DEBUG
		println("[thr_pool_destroy] after reap => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d",
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
#endif
		if(list_empty(&tp->unjoined_list) && tp->t_num_curr <= 0)
			break;
	}
	(void)pthread_mutex_unlock(&tp->pool_mtx);
	TP_UNBLOCK_SIG();
	#ifdef TP_DEBUG
			println("[thr_pool_cancel] finished => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d",
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
	#endif
}

/**
 * thr_pool_destroy - cancel the working threads and reap all the threads in the pool, then reclaim the thread pool
 * @tpp:				the pointer to the thr_pool_t type data.
 * after this, *tpp will equal to NULL.
 */
void thr_pool_destroy(thr_pool_t *tpp)
{
	if(tpp == NULL) return;


	thr_pool_t tp = *tpp;
	thr_pool_cancel(tp);

	//reclaim thread pool
	(void)pthread_mutex_lock(&thr_pool_list_lock);
	list_del(&tp->list_ent);
	(void)pthread_mutex_unlock(&thr_pool_list_lock);

	pthread_mutex_destroy(&tp->pool_mtx);
	pthread_cond_destroy(&tp->pool_joincv);
	pthread_cond_destroy(&tp->pool_workcv);
	pthread_cond_destroy(&tp->pool_waitcv);
	free(tp);
	*tpp = NULL;


}

/**************************************/
static inline void thread_idle(thread_ent_t *t);
static void *create_worker(void *arg)
{
	(void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	(void)pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	pthread_sigmask(SIG_SETMASK, &fullsigmask, NULL);//block all signals
	thread_ent_t *t = (thread_ent_t*)arg;
	thr_pool_t tp = t->tp;
	t->tid = pthread_self();

	(void)pthread_mutex_lock(&tp->pool_mtx);
	pthread_cleanup_push((pthread_cleanup_fn)worker_cleanup,t);


	for(;;)
	{
		pthread_testcancel();   // enable to be cancelled

		thread_idle(t);

		job_t *jobp = (list_entry(list_shift(&tp->job_list),job_t,list_ent));
		list_move_tail(&t->list_ent,&tp->active_list);
		pthread_cleanup_push((pthread_cleanup_fn)job_finish,t);
		(void)pthread_mutex_unlock(&tp->pool_mtx);

		job_t job = *jobp;
		free(jobp);
		(void)(*job.job_fn)(job.arg,t);
		pthread_cleanup_pop(1);
		(void)pthread_mutex_lock(&tp->pool_mtx);
	}
	pthread_cleanup_pop(1);
	return t;
}


static void worker_cleanup(thread_ent_t *t)
{
	thr_pool_t tp = t->tp;
	#ifdef TP_DEBUG
		println("[worker_cleanup %p] enter => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d", t,
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
	#endif
	list_move_tail(&t->list_ent,&tp->unjoined_list);

	if((uint_t)(list_size(&tp->unjoined_list))>tp->t_num_curr/2)
	{
#ifdef TP_DEBUG
		println("[worker_cleanup %p] before reap => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d", t,
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
#endif
		while(!list_empty(&tp->unjoined_list))
		{
			thread_ent_t *t = list_entry(list_shift(&tp->unjoined_list),thread_ent_t,list_ent);
			pthread_join(t->tid,NULL);
			tp->t_num_curr--;
			free(t);
		}
#ifdef TP_DEBUG
		println("[worker_cleanup %p] after reap => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d", t,
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
#endif
	}
	(void)pthread_mutex_unlock(&tp->pool_mtx);
	(void)pthread_cond_signal(&tp->pool_joincv);
}

static void job_finish(thread_ent_t *t)
{

	thr_pool_t tp = t->tp;

	(void)pthread_mutex_lock(&tp->pool_mtx);
	list_del(&t->list_ent);
	#ifdef TP_DEBUG
		println("[job_finish %p]  => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d", t,
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
	#endif
	(void)pthread_mutex_unlock(&tp->pool_mtx);

	(void)pthread_cond_signal(&tp->pool_waitcv);


}

static inline void thread_idle(thread_ent_t *t)
{
	thr_pool_t tp = t->tp;
	struct timespec tm_timeout;
		check(clock_gettime(CLOCK_REALTIME ,&tm_timeout) != -1, pthread_exit(T_EXIT_FAILED(errno)));
		tm_timeout.tv_sec += tp->timeout_sec;

		while(list_empty(&tp->job_list))
		{	//worker turn into idle

			tp->t_num_idle++;
			list_move_tail(&t->list_ent,&tp->idle_list);
#ifdef TP_DEBUG
		println("[thread_idle %p] idle => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d", t,
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr);
#endif
			int wait_ret = pthread_cond_timedwait(&tp->pool_workcv,&tp->pool_mtx, &tm_timeout);
			list_del(&t->list_ent);
			tp->t_num_idle--;
			if(wait_ret == ETIMEDOUT)//idle timeout
			{
#ifdef TP_DEBUG
		println("[thread_idle %p] timeout => active: %d, idle : %d, job : %d, unjoined : %d, curr_num : %d, min : %d", t,
			list_size(&tp->active_list),list_size(&tp->idle_list),list_size(&tp->job_list),list_size(&tp->unjoined_list),tp->t_num_curr,tp->t_num_min);
#endif
				if(tp->t_num_idle > tp->t_num_min)
				{
					pthread_exit(T_EXIT_SUCCESS);
				}else{//when tp->t_num_curr no more than tp->t_num_min, thread should idle or wait for cancel.
					tm_timeout.tv_sec += (tp->timeout_sec > 0 ? tp->timeout_sec : T_TIMEOUT_DEFAULT);
				}
			}

		}
}



static pthread_attr_t *
clone_attributes(pthread_attr_t *old_attr)
{
	struct sched_param param;
	void *addr;
	size_t size;
	int value;
	int detachstate;

	if (old_attr != NULL) {
		pthread_attr_t *new_attr = malloc(sizeof(pthread_attr_t));
		check(new_attr != NULL, return NULL);
		(void) pthread_attr_init(new_attr);
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

		(void) pthread_attr_getdetachstate (old_attr, &detachstate);
		(void) pthread_attr_setdetachstate(new_attr, detachstate);
		return new_attr;
	}else
	{
		return NULL;
	}

}

/*************************************
- helpers
*************************************/
void thr_pool_stat_print(thr_pool_t tp)
{
	(void)pthread_mutex_lock(&tp->pool_mtx);
	println("thread pool %p:",tp);
	println("t_num_min : %u",tp->t_num_min);
	println("t_num_max : %u",tp->t_num_max);
	println("t_num_curr : %u",tp->t_num_curr);
	println("t_num_idle : %u",tp->t_num_idle);
	println("timeout_sec : %d",tp->timeout_sec);
	println("active_list : %d",list_size(&tp->active_list));
	println("idle_list : %d",list_size(&tp->idle_list));
	println("unjoined_list : %d",list_size(&tp->unjoined_list));
	println("job_list : %d",list_size(&tp->job_list));

	(void)pthread_mutex_unlock(&tp->pool_mtx);
}


void thread_ent_stat_print(thread_ent_t *t)
{

	println("[pool %p thread %p]",t->tp,t);

}
