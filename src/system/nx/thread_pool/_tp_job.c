#include "_tp_job.h"
#include <common/dbg.h>
#include <common/ds/dlist.h>

int tp_job_equeue(thr_pool *tp, job_cb j_cb, void *arg)
{
	check(tp != NULL && j_cb != NULL, return -1);
	tp_job *j = MALLOC(1, tp_job);
	check(j != NULL, return -1);
	j->cb = j_cb;
	j->arg = arg;
	INIT_LIST_HEAD(&j->l_ent);

	list_push(&j->l_ent,&tp->jobq);
	tp->state |= POOL_BUSY;
	if (!list_empty(&tp->idleq))
		(void) pthread_cond_signal(&tp->pool_workcv);
	return 0;
}

tp_job *tp_job_dqueue(thr_pool *tp)
{
	check(tp != NULL , return NULL);
	tp_job *j = (list_entry(list_shift(&tp->jobq),tp_job,l_ent));
	return j;
}

void tp_job_queue_destroy(thr_pool *tp)
{
	check(tp != NULL , return);
	while(!list_empty(&tp->jobq))
	{
		tp_job *j = (list_entry(list_shift(&tp->jobq),tp_job,l_ent));
		free(j);
	}
}
