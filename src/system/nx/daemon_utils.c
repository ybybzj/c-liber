#include "common.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "daemon_utils.h"

/* Maximum file descriptors to close if sysconf(_SC_OPEN_MAX) is indeterminate */
#define DAEMON_MAX_CLOSE  8192          

int beDaemon(int flags)
{
	int maxfd, fd;
	switch(fork())                        /* Become background process */
	{
		case -1:
			return -1;
		case 0:
			break;                         /* Child falls through... */
		default:
			exit_s(false);				   /* while parent terminates */
	}

	check(setsid() != -1, return -1);      /* Become leader of new session */

	switch(fork())						   /* Ensure we are not session leader */
	{
		case -1:
			return -1;
		case 0:
			break;
		default:
			exit_s(false);
	}

	if(!(flags&DAEMON_NO_CHDIR))
		chdir("/");

	if(!(flags&DAEMON_NO_UMASK0))
		umask(0);
	
	if(!(flags&DAEMON_NO_CLOSE_FDS))
	{
		maxfd = sysconf(_SC_OPEN_MAX);
		if(maxfd == -1) maxfd = DAEMON_MAX_CLOSE;
		for(fd = 0; fd < maxfd; fd++)
			close(fd);
	}

	if(!(flags&DAEMON_NO_NULL_STDFDS))
	{
		close(STDIN_FILENO);
		check((fd = open("/dev/null",O_RDWR)) == STDIN_FILENO, return -1);
		check((dup2(STDIN_FILENO,STDOUT_FILENO)) == STDOUT_FILENO, return -1);
		check((dup2(STDIN_FILENO,STDERR_FILENO)) == STDERR_FILENO, return -1);
	}

	return 0;
}