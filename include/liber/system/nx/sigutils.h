#ifndef __SIGUTILS_H__
#define __SIGUTILS_H__

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

typedef void (*sig_handler)(int);
typedef void (*siginfo_handler)(int, siginfo_t *, void *);
#ifndef SIGNULL
#define SIGNULL 0
#endif
//all functions are not async-signal-safe
void printSigset(FILE *outf, const char *prefix, const sigset_t *sigset);
int printSigMask(FILE *outf, const char *msg);
int printPendingSigs(FILE *outf, const char *msg);

void printSigInfo(const siginfo_t *si);

//signal helpers
sigset_t addSigToSet(sigset_t *out,...);
sigset_t delSigFromSet(sigset_t *out,...);

int sig_handler_register(sigset_t sigmask, int opt, sig_handler handler, .../*signals*/);

#define emptysigset() (addSigToSet(NULL,SIGNULL))
#define allsigset() (delSigFromSet(NULL,SIGNULL))

static inline struct sigaction makeSigaction(const sigset_t sa_mask, int sa_flags, sig_handler handler)
{
	struct sigaction sa;
	sa.sa_mask = sa_mask;
	sa.sa_flags = sa_flags;
	sa.sa_handler = handler;
	return sa;
}

static inline struct sigaction makeInfoSigaction(const sigset_t sa_mask, int sa_flags, siginfo_handler handler)
{
	struct sigaction sa;
	sa.sa_mask = sa_mask;
	sa.sa_flags = SA_SIGINFO|sa_flags;
	sa.sa_sigaction = handler;
	return sa;
}

#define BLOCK_SIG(action_flag,...) \
sigset_t __org_sigset__, __block_sigset__;\
__block_sigset__ = addSigToSet(NULL,##__VA_ARGS__,SIGNULL);\
sigprocmask(action_flag,&__block_sigset__,&__org_sigset__)

#define BLOCK_ALLSIG_BUT(action_flag,...) \
sigset_t __org_sigset__, __block_sigset__;\
__block_sigset__ = delSigFromSet(NULL,##__VA_ARGS__,SIGNULL);\
sigprocmask(action_flag,&__block_sigset__,&__org_sigset__)

#define UNBLOCK_SIG() \
sigprocmask(SIG_SETMASK,&__org_sigset__,NULL)

#define SIG_HDL_REG(sa_mask, sa_flags, handler,...) \
sig_handler_register(sa_mask, sa_flags, handler, ##__VA_ARGS__,SIGNULL)
#endif