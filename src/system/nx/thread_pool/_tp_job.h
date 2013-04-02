#ifndef __THREAD_POOL_JOB_H__
#define __THREAD_POOL_JOB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "_thread_pool.h"
typedef struct _tp_job
{
	dl_ent_t l_ent;
	job_cb cb;
	void *arg;

}tp_job;

int tp_job_equeue(thr_pool *tp,job_cb j_cb,void *arg);
tp_job *tp_job_dqueue(thr_pool *tp);
void tp_job_queue_destroy(thr_pool *tp);

#ifdef __cplusplus
}
#endif

#endif
