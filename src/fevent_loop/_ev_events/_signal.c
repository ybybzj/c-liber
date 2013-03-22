#define _GNU_SOURCE
#include "_signal.h"

#include <common/dbg.h>
#include <system/nx.h>
#include <system/nx/sigutils.h>
#include <system/nx/io/io.h>
#include <pthread.h>
typedef struct _ev_sig_data ev_sig_data;
static ev_sig_data *es_data[NSIG] = {NULL};
static pthread_mutex_t wfd_mtx = PTHREAD_MUTEX_INITIALIZER;

struct _ev_sig_data{
	int signal;
	int wfd;
	int (*sig_cb) (int,void*);
	void *arg;
	void (*arg_free)(void*);
};

static ev_sig_data *ev_sig_data_create( int (*sig_cb) (int,void*),
									void *arg,
									void (*arg_free)(void*),
									int signal,
									int wfd)
{
	ev_sig_data *es = MALLOC(1,ev_sig_data);
	check(es != NULL, return NULL);
	es->sig_cb = sig_cb;
	es->arg = arg;
	es->arg_free = arg_free;
	es->signal = signal;
	es->wfd = wfd;
	return es;
}

static void ev_sig_data_free(ev_sig_data *es)
{
	if(es != NULL)
	{
		if(es->arg_free != NULL)
		{
			es->arg_free(es->arg);
		}
		close(es->wfd);
		pthread_mutex_lock(&wfd_mtx);
		es_data[es->signal] = NULL;
		pthread_mutex_unlock(&wfd_mtx);
		free(es);
	}
}

static int make_signal_ev( int priority,
							int (*sig_cb) (int,void*),
							void *arg,
							void (*arg_free)(void*),
							int signal,
							fevent *out_ev)
{
	fevent ev_signal = ev_new_event();

	ev_signal.events = EV_READ;
	ev_signal.priority = priority;
	int pfd[2];

	check(pipe(pfd) != -1, return -1);
	check(make_unblock(pfd[0]) != -1, close(pfd[0]); close(pfd[1]); return -1);
	check(make_unblock(pfd[1]) != -1, close(pfd[0]); close(pfd[1]); return -1);




	ev_signal.fd = pfd[0];
	ev_signal.data.ptr = ev_sig_data_create( sig_cb, arg,arg_free, signal, pfd[1]);
	check(ev_signal.data.ptr != NULL, close(pfd[0]); close(pfd[1]); return -1);

	ev_signal.ev_data_free = (void (*)(void *))ev_sig_data_free;

	*out_ev = ev_signal;
	return 0;
}

static ev_set on_sigev(fevent ev, ev_monitor *monitor UNUSED)
{
	int rfd = ev.fd;
	char ch;
	int save_err = errno;
	errno = 0;
	for(;;)
	{
		if( read(rfd, &ch, 1) == -1 )
		{
			if(errno == EINTR)
				continue;
			else if(errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else
			{
				print_err("[on_sigev] read from %d failed!", rfd);
				errno = save_err;
				return EV_FIN;
			}
		}
	}

	errno = save_err;
	ev_sig_data *es = (ev_sig_data*)ev.data.ptr;

	if(es->sig_cb)
	{
		int ret = es->sig_cb(es->signal,es->arg);
		if(ret == 0)
		{
			DEFAULT_SIG(es->signal);
		}
		return ret? ev.events : EV_FIN;
	}
	return EV_FIN;
}

//signal handler

static void s_handler(int sig)
{
	int save_err = errno;
	ev_sig_data *es = es_data[sig];
	if(es == NULL || (write(es->wfd, "x", 1) == -1 && !(errno == EAGAIN || errno == EWOULDBLOCK)))
	{
		print_err("[ev_sig_catch_handler]: write to wfd failed! es(%p)", es);
		return;
	}
	errno = save_err;

}

static int catch_signal(int sig, ev_sig_data *es)
{
	pthread_mutex_lock(&wfd_mtx);
	es_data[sig] = es;
	pthread_mutex_unlock(&wfd_mtx);
	return SIG_HDL_REG(emptysigset(), SA_RESTART, s_handler, sig);
}

int add_signal_ev( ev_monitor *monitor,
					int priority,
					int signal,
					int (*sig_cb) (int,void*),
					void *arg,
					void (*arg_free)(void*))
{
	check_err(es_data[signal] == NULL, return -1, "[add_signal_ev]: signal %d is already monitored! es(%p)", signal, es_data[signal]);
	fevent ev_signal;
	check(make_signal_ev( priority,
							sig_cb,
							arg,
							arg_free,
							signal,
							&ev_signal) != -1, return -1);
	ev_sig_data *es = (ev_sig_data*)ev_signal.data.ptr;
	check( catch_signal(signal,es) != -1, close(ev_signal.fd);ev_sig_data_free(es);return -1);
	check(ev_monitor_ctl(monitor, EV_CTL_ADD, ev_signal, EV_RD_CB(on_sigev)) != -1, return -1);
	return 0;
}


