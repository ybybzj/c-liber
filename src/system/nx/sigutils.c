#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "sigutils.h"
#include <string.h>
#include <stdarg.h>
#include <common/dbg.h>
#include <errno.h>

void printSigset(FILE *outf, const char *prefix, const sigset_t *sigset)
{
	int sig, cnt;
	cnt = 0;
	for(sig = 1; sig < NSIG; sig++)
	{
		if(sigismember(sigset,sig)){
			cnt++;
			fprintf(outf,"%s%d (%s)\n",prefix, sig, strsignal(sig));
		}
	}
	if(cnt == 0){
		fprintf(outf,"%s<empty signal set>\n",prefix);
	}
}
int printSigMask(FILE *outf, const char *msg)
{
	sigset_t currMask;
	
	check(sigprocmask(SIG_BLOCK, NULL, &currMask) != -1,return -1);
	if(msg != NULL)
	{
		fprintf(outf,"%s",msg);
	}
	printSigset(outf,"\t\t",&currMask);
	return 0;

}
int printPendingSigs(FILE *outf, const char *msg)
{
	sigset_t pendingSigs;
	check(sigpending(&pendingSigs) != -1, return -1);
	if(msg != NULL)
	{
		fprintf(outf,"%s",msg);
	}
	printSigset(outf,"\t\t",&pendingSigs);
	return 0;

}

void printSigInfo(const siginfo_t *si)
{
	println("\tsi_signo=%d, si_code=%d(%s), si_value=%d",
		si->si_signo, si->si_code,
		((si->si_code == SI_USER)?"SI_USER":(si->si_code == SI_QUEUE)?"SI_QUEUE":"other"),
		si->si_value.sival_int);
	println("\tsi_pid=%ld, si_uid=%ld",(long)si->si_pid,(long)si->si_uid);
}

sigset_t addSigToSet(sigset_t *out,...)
{
	sigset_t sset,resultset;
	sigemptyset(&sset);
	va_list argList;
	va_start(argList,out);
	int sig;
	while((sig = va_arg(argList,int)) != SIGNULL){
		sigaddset(&sset,sig);
	}
	va_end(argList);
	resultset = sset;
	if(out != NULL){
		sigorset(&resultset,out,&sset);
		*out = resultset;
	}
	return resultset;
}
sigset_t delSigFromSet(sigset_t *out,...)
{
	sigset_t sset,resultset;
	sigfillset(&sset);
	va_list argList;
	va_start(argList,out);
	int sig;
	while((sig = va_arg(argList,int)) != SIGNULL){
		sigdelset(&sset,sig);
	}
	va_end(argList);
	resultset = sset;
	if(out != NULL){
		sigandset(&resultset,out,&sset);
		*out = resultset;
	}
	return resultset;
}

int sig_handler_register(sigset_t sigmask, int opt, sig_handler handler, .../*signals*/)
{

	struct sigaction sa;
	sa = makeSigaction(sigmask, opt, handler);
	va_list argList;
	va_start(argList,handler);
	int sig, err;
	err = 0;
	while((sig = va_arg(argList,int)) != SIGNULL)
	{
		err = sigaction(sig, &sa, NULL);
		if(err == -1)
			break;
	}
	va_end(argList);
	return err;
}