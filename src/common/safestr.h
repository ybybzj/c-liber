#ifndef __SAFE_STRING_H__
#define __SAFE_STRING_H__
#include <stdio.h>
#include <string.h>
#include "dbg.h"
//safe version
size_t strnlen_s(const char *s, size_t maxsize);

int strcpy_s(char * restrict s1, rsize_t s1max, const char * restrict s2);
int strncpy_s(char * restrict s1, rsize_t s1max, const char * restrict s2, rsize_t n);
int strcat_s(char * restrict s1, rsize_t s1max, const char * restrict s2);
int strncat_s(char * restrict s1, rsize_t s1max, const char * restrict s2, rsize_t n);

char *strtok_s(char * restrict s1, rsize_t * restrict s1max, const char * restrict s2, char ** restrict ptr);


//accessor
bool strstart(const char *s1, const char *s2, rsize_t s2max);
bool strend(const char *s1,rsize_t s1max, const char *s2, rsize_t s2max);

char *strdup_s(const char *restrict src,char *restrict des,size_t des_max);
// //strings on the heap
// char* strdup(const char*);
// char* strjoin(const char*,...);
#endif