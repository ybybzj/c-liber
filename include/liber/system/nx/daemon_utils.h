#ifndef __DAEMON_UTILS_H__
#define __DAEMON_UTILS_H__
/* Bit-mask values for 'flags' argument of becomeDaemon() */

#define DAEMON_NO_CHDIR         01    // Don't chdir("/") 
#define DAEMON_NO_CLOSE_FDS     02    // Don't close all open files
#define DAEMON_NO_NULL_STDFDS   04    // Don't reopen stdin, stdout, and stderr to /dev/null
#define DAEMON_NO_UMASK0        010   // Don't do a umask(0) and don't close all open files



int beDaemon(int flags);
#endif