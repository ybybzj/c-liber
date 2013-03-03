#define _GNU_SOURCE

#include <system/nx.h>
#include <system/nx/pthread_pool.h>
#include <signal.h>

void *job(void *arg, const thread_ent_t *t)
{
	int job_id = (long long)arg;
	println("[Job %d] start...",job_id);
	thread_ent_stat_print((thread_ent_t *)t);
	sleep((job_id % 10) + 1);
	println("[Job %d] finish...",job_id);
	return NULL;
}

void handler(int sig)
{
	println("signal %d received!", sig);
}
int main(int argc, char *argv[])
{
	int t_num_min,t_num_max,timeout_sec; 
	validate_cls_args(argc, argv, 1, "[thread-num-min thread-num-max timeout-sec]");
	if(argc > 1)
	{
		ClsArgToVal(stoi,argv[1],&t_num_min,"thread-num-min");
		ClsArgCheck(t_num_min >= 0,"thread-num-min");
	}else{
		t_num_min = 0;
	}
	if(argc > 2)
	{
		ClsArgToVal(stoi,argv[2],&t_num_max,"thread-num-max");
		ClsArgCheck(t_num_max >= 0,"thread-num-max");
	}else{
		t_num_max = 0;
	}
	if(argc > 3)
	{
		ClsArgToVal(stoi,argv[3],&timeout_sec,"timeout-sec");
	}else{
		timeout_sec = -1;
	}

	sigset_t fulsigset,origset,emptyset;
	sigemptyset(&emptyset);
	struct sigaction sa;
	sa.sa_mask = emptyset;
	sa.sa_flags = 0;
	sa.sa_handler = handler;
	sigfillset(&fulsigset);
	sigprocmask(SIG_SETMASK,&fulsigset,&origset);
	sigaction(SIGINT,&sa,NULL); 
	sigprocmask(SIG_SETMASK,&origset,NULL);
	thr_pool_t tp = NULL;
	thr_pool_init(&tp,(uint_t)t_num_min,(uint_t)t_num_max,timeout_sec,NULL);
	int i = 0;
	for(;i<15000;i++)
	{
		thr_pool_queue(tp, job, (void *)((long long)i));
	}
	
	// thr_pool_stat_print(tp);
	thr_pool_wait(tp);
	// thr_pool_stat_print(tp);
	thr_pool_destroy(&tp);
	exit_s(true);
}