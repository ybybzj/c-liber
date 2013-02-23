#define _GNU_SOURCE
#include "time.h"
#include <common/dbg.h>
#include "common.h"
#ifdef _POSIX_THREADS
#include <pthread.h>
#endif
long get_timestamp(int unit)
{
	struct timespec now;
	check(clock_gettime(CLOCK_REALTIME ,&now) != -1, return -1);
	long result;
	switch(unit)
	{
		case TM_UNIT_S:
			result = (long)now.tv_sec;
			break;
		case TM_UNIT_MS:
			result = (long)now.tv_sec*1000 + now.tv_nsec/1000000;
			break;
		case TM_UNIT_US:
			result = (long)now.tv_sec*1000000 + now.tv_nsec/1000;
		case TM_UNIT_NS:
			result = (long)now.tv_sec*1e9 + now.tv_nsec;
			break;
		default:
			result = -1;
			break;
	}
	return result;
}

/*-----------current time functions--------------------*/



#define CURT_STR_LEN 256
#ifdef _POSIX_THREADS
static char * _current_time_t(const char *tfmt);
#else
static char * _current_time(const char *tfmt);
#endif

char * current_time(const char *tfmt)
{
#ifdef _POSIX_THREADS
	return _current_time_t(tfmt);
#else
	return _current_time(tfmt);
#endif
}


//reentrant
char * current_time_r(const char *tfmt, char *outstr, size_t maxsize)
{
	time_t t;
	size_t s;
	struct tm *tm;

	t = time(NULL);
	
	check((tm = localtime(&t)) != NULL, return NULL);

	s = strftime(outstr,maxsize,(tfmt != NULL)? tfmt: "%c", tm);

	return (s == 0)? NULL : outstr;
}

//per thread
#ifdef _POSIX_THREADS
static pthread_key_t cur_time_key;

static void create_ct_key(void);
static void free_ct_key(void *);
static char * _current_time_t(const char *tfmt)
{
	static pthread_once_t ct_key_created = PTHREAD_ONCE_INIT;
	pthread_once(&ct_key_created, create_ct_key);
	char *buf;

	buf = pthread_getspecific(cur_time_key);
	if(buf == NULL)
	{
		check((buf = (char*)calloc(CURT_STR_LEN,sizeof(char))) != NULL, return "(unknown/t)");
		check_t(pthread_setspecific(cur_time_key,buf),return "(unknown/t)");
	}
	check(current_time_r(tfmt, buf, CURT_STR_LEN) != NULL, return "(unknown/t)");
	return buf;

}
static void create_ct_key()
{
	check_t(pthread_key_create(&cur_time_key, free_ct_key),print_err("pthread_key_create failed!"));
}
static void free_ct_key(void *buf)
{
	if(buf == NULL) return;

	printf("buf: %p\n",buf);
	free(buf);
}
#else

//non-reentrant
static char ct_str[CURT_STR_LEN];
static char* _current_time(const char *tfmt)
{
	check(current_time_r(tfmt, ct_str, CURT_STR_LEN) != NULL, return "(unknown/t)");
	return ct_str;
}
#endif