#define _GNU_SOURCE
#include "_tp_thread.h"
#include "_tp_job.h"
#include <common/dbg.h>
#include <common/ds/dlist.h>
#include "../common.h"
#include "../sigutils.h"
#include <pthread.h>
#include <time.h>
typedef struct _tp_thread
{
	dl_ent_t l_ent;
	pthread_t tid;
	thr_pool *tp;
}tp_thread;


/*
 * @fullsigmask: signal set contains all signals,
 * which is used to block all signals for each thread.
 * signal handling should be conducted by main thread.
 */
static sigset_t fullsigmask;

#define TP_BLOCK_ALLSIG() \
sigset_t __org_sigset__;\
sigfillset(&fullsigmask);\
pthread_sigmask(SIG_SETMASK, &fullsigmask, &__org_sigset__)

#define TP_UNBLOCK_SIG() \
pthread_sigmask(SIG_SETMASK, &__org_sigset__, NULL)


/*------------thread wrapper functions----------------*/
static void worker_cleanup(tp_thread *t);
static void job_finish(tp_thread *t);
static void *create_worker(void *arg);

/*----------------------------------------------------*/

int tp_thread_spawn(thr_pool *tp)
{
	ERR_CLEAN_INIT();
	TP_BLOCK_ALLSIG();

	check(tp != NULL, goto onerr);
	tp_thread *t = MALLOC(1, tp_thread);
	check(t != NULL, goto onerr);
	ERR_CLEAN_PUSH(0,t, thread);

	t->tp = tp;
	INIT_LIST_HEAD(&t->l_ent);

	check_t(pthread_create(&t->tid,&tp->thr_attr,create_worker,t), goto onerr);



	TP_UNBLOCK_SIG();
	return 0;
onerr:
	ERR_CLEAN_BEGIN
		case 0:
			{
				free((tp_thread*)cleanup_resource);
				break;
			}
		ERR_CLEAN_END
		TP_UNBLOCK_SIG();
		return -1;


}

void tp_thread_queue_cancel(thr_pool *tp)
{
	check(tp != NULL, return);
	dl_ent_t *pos;
	list_for_each(pos, &tp->activeq)
	{
		tp_thread *t = list_entry(pos,tp_thread,l_ent);
		(void)pthread_cancel(t->tid);
	}
	list_for_each(pos, &tp->idleq)
	{
		tp_thread *t = list_entry(pos,tp_thread,l_ent);
		(void)pthread_cancel(t->tid);
	}
}

/*---------------------thread wrapper impementations-------------------------------*/

static inline void thread_idle(tp_thread *t)
{
	thr_pool *tp = t->tp;


	if(tp->state & POOL_WAIT)
		tp_notify_waiter(tp);


	while(list_empty(&tp->jobq) && !(tp->state&POOL_CANCEL))
	{	//worker turn into idle

		if(list_empty(&tp->activeq))
			tp->state &= ~POOL_BUSY;

		if(tp->t_total <= tp->t_min)
		{
			(void)pthread_cond_wait(&tp->pool_workcv,&tp->pool_mtx);
		}else
		{
			struct timespec tm_timeout;
			check(clock_gettime(CLOCK_REALTIME ,&tm_timeout) != -1, pthread_exit(NULL));
			tm_timeout.tv_sec += tp->timeout_sec;
			if (tp->timeout_sec == 0 || pthread_cond_timedwait(&tp->pool_workcv, &tp->pool_mtx, &tm_timeout) == ETIMEDOUT)
			{
				pthread_exit(NULL);
			}

		}
	}
}

static void *create_worker(void *arg)
{

	tp_thread *t = (tp_thread*)arg;
	thr_pool *tp = t->tp;

	(void)pthread_mutex_lock(&tp->pool_mtx);
	pthread_cleanup_push((pthread_cleanup_fn)worker_cleanup,t);
	list_move_tail(&t->l_ent,&tp->idleq);
	tp->t_total++;
	for(;;)
	{
		(void)pthread_sigmask(SIG_SETMASK, &fullsigmask, NULL);
		(void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		(void)pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

		thread_idle(t);
		if (tp->state & POOL_CANCEL)
			break;
		if(!list_empty(&tp->jobq))
		{
			tp_job *job = tp_job_dqueue(tp);
			list_move_tail(&t->l_ent,&tp->activeq);
			tp->state |= POOL_BUSY;
			(void)pthread_mutex_unlock(&tp->pool_mtx);
			pthread_cleanup_push((pthread_cleanup_fn)job_finish,t);
			job_cb j_cb = job->cb;
			void *arg = job->arg;
			free(job);

			(void)j_cb(arg);
			pthread_cleanup_pop(1);
		}

	}
	pthread_cleanup_pop(1);
	return NULL;
}


static void worker_cleanup(tp_thread *t)
{
	thr_pool *tp = t->tp;
	--tp->t_total;
	list_del(&t->l_ent);
	if (tp->state & POOL_CANCEL) {
		if (tp->t_total == 0)
		(void) pthread_cond_broadcast(&tp->pool_exitcv);
	}else if(!list_empty(&tp->jobq) && tp->t_total < tp->t_max)
	{
		(void)tp_thread_spawn(tp);
	}
	(void) pthread_mutex_unlock(&tp->pool_mtx);
	free(t);

}

static void job_finish(tp_thread *t)
{

	thr_pool *tp = t->tp;

	(void)pthread_mutex_lock(&tp->pool_mtx);
	list_move_tail(&t->l_ent,&tp->idleq);

	if(tp->state & POOL_WAIT)
		tp_notify_waiter(tp);

}



