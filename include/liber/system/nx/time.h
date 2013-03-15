#ifndef __UNIX_TIME_H__
#define __UNIX_TIME_H__
#include <time.h>
#define TM_UNIT_S 0
#define TM_UNIT_MS 1
#define TM_UNIT_US 2
#define TM_UNIT_NS 3

long get_timestamp(int unit);

char * current_time_r(const char *tfmt, char *outstr, size_t maxsize);

char * current_time(const char *tfmt);

#endif