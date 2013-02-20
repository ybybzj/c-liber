#ifndef __UNIX_COMMON_H__
#define __UNIX_COMMON_H__
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <common/dbg.h>
#include <common/utils.h>
static inline void terminate(bool useExit3)
{
	/* Dump core if EF_DUMPCORE environment variable is defined and
	is a nonempty string; otherwise call exit(3) or _exit(2),
	depending on the value of 'useExit3'. */
	char *s = getenv("EF_DUMPCORE");
	fflush(stderr);
	if(useExit3)
		fflush(stdout);
	if (s != NULL && *s != '\0')
		abort();
	else if (useExit3)
		exit(EXIT_FAILURE);
	else
		_exit(EXIT_FAILURE);
}

static inline void exit_s(bool useExit3)
{
	if (useExit3)
		exit(EXIT_SUCCESS);
	else
		_exit(EXIT_SUCCESS);
}

static inline void validate_cls_args(int argc, char *argv[], int required, const char *arg_list)
{
	if(argc < required || (argc >= 2 && strcmp(argv[1],"--help")==0)){
		fprintf(stderr,"Usage: %s %s\n",argv[0],arg_list);
		terminate(true);
	}
}

#define ClsArgToVal(stv,arg,out,argname) check(stv((arg),(out)) == 0, fprintf(stderr,"Invalid argument \"" argname "\": %s\n",(arg));terminate(true))
#define ClsArgCheck(A,argname) check(A,fprintf(stderr,"Argument \"" argname "\" voilates Constraint: %s\n",#A);terminate(true))	 
#endif