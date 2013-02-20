#include "utils.h"
#include "dbg.h"
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

static inline int _stoll(const char *str, long long*out, const char *fn)
{
	char *endptr;
	char *nonempty = (char *)str;
	long long result;
	while(isspace(*nonempty) && *nonempty != '\0')
		nonempty++;
	check_err(*nonempty != '\0', return -1, "[%s] empty string!", fn);
	errno = 0;
	result = strtoll(nonempty,&endptr,0);
	check_err(errno != ERANGE, return -1,"[%s] out of range!", fn);
	check_err(*endptr == '\0', return -1, "[%s] failed!", fn);
	*out = result;
	return 0;
}

int stoi(const char *str, int *out)
{
	long long result;
	int err = _stoll(str, &result, "stoi");
	if(err != 0) return err;
	check_err(result <= INT_MAX && result >= INT_MIN, return -1, "[stoi] outof range!");
	*out = (int)result;
	return 0;
}

int stol(const char *str, long *out)
{
	long long result;
	int err = _stoll(str, &result, "stol");
	if(err != 0) return err;
	check_err(result <= LONG_MAX && result >= LONG_MIN, return -1, "[stol] outof range!");
	*out = (long)result;
	return 0;
}

int stoll(const char *str, long long*out)
{
	return _stoll(str, out, "stoll");
}

int stod(const char *str, double *out)
{
	char *endptr;
	char *nonempty = (char *)str;
	double result;
	while(isspace(*nonempty) && *nonempty != '\0')
		nonempty++;
	check_err(*nonempty != '\0', return -1, "[stod] empty string!");
	errno = 0;
	result = strtod(nonempty,&endptr);
	check_err(errno != ERANGE, return -1,"[stod] out of range!");
	check_err(*endptr == '\0', return -1, "[stod] failed!");
	*out = result;
	return 0;
}

