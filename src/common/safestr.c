#include "safestr.h"
#include "utils.h"
#include <errno.h>
#define _zeroStr(str) do{\
*(str) = '\0';\
}while(0)

#define CHARSET_MAX 256
//safe version

char *strdup_s(const char *restrict src,char *restrict des,size_t des_max)
{
	check(src != NULL && des != NULL && des_max > 0, return NULL);
	size_t copy_len = strnlen_s(src, des_max - 1);
	size_t i = 0;
	while(i < copy_len)
	{
		*(des+i) = *(src + i);
		i++;
	}
	*(des+i) = '\0';
	return des;
}

size_t strnlen_s(const char *s, size_t maxsize)
{
	if(s == NULL) return 0;
	if(maxsize == 0) maxsize = RSIZE_MAX;
	char *p = (char*)s;
	size_t num_pass = 0;
	while(*p++ != '\0' && num_pass < maxsize)
	{
		num_pass++;
	}
	
	return num_pass;
}

int strcpy_s(char * restrict s1, rsize_t s1max, const char * restrict s2)
{
	check_err(s1 != NULL && s2 != NULL,return -1,"[strcpy_s]: Invalid Null pointers!");
	Check_mem_size(s1max,return -1);
	size_t s2len = strnlen_s(s2,s1max);
	check_err(s1max > s2len,_zeroStr(s1);return -1,"[strcpy_s]: len of dest string is not long enough!");
	memcpy(s1,s2,s2len);
	*(s1+s2len) = '\0';
	return 0;
}
int strncpy_s(char * restrict s1, rsize_t s1max, const char * restrict s2, rsize_t n)
{
	check_err(s1 != NULL && s2 != NULL,return -1,"[strncpy_s]: Invalid Null pointers!");
	Check_mem_size(s1max,return -1);
	Check_mem_size(n,return -1);
	size_t s2len = strnlen_s(s2,s1max);
	check_err(n < s1max || s1max > s2len,_zeroStr(s1);return -1,"[strncpy_s]: len of dest string is not long enough!");
	size_t copy_size = min(n,s2len);
	memcpy(s1,s2,copy_size);
	*(s1+copy_size) = '\0';
	return 0;
}
int strcat_s(char * restrict s1, rsize_t s1max, const char * restrict s2)
{
	check_err(s1 != NULL && s2 != NULL,return -1,"[strcat_s]: Invalid Null pointers!");
	Check_mem_size(s1max,return -1);
	size_t s1len = strnlen_s(s1,s1max);
	size_t m = s1max - s1len;
	size_t s2len = strnlen_s(s2,m);
	check_err(m > s2len,_zeroStr(s1);return -1,"[strcat_s]: len of dest string is not long enough!");
	memcpy(s1+s1len,s2,s2len);
	*(s1+s1len+s2len) = '\0';
	return 0;
}
int strncat_s(char * restrict s1, rsize_t s1max, const char * restrict s2, rsize_t n)
{
	check_err(s1 != NULL && s2 != NULL,return -1,"[strncat_s]: Invalid Null pointers!");
	Check_mem_size(s1max,return -1);
	Check_mem_size(n,return -1);
	size_t s1len = strnlen_s(s1,s1max);
	size_t m = s1max - s1len;
	size_t s2len = strnlen_s(s2,m);
	check_err(n < m || m > s2len,_zeroStr(s1);return -1,"[strncat_s]: len of dest string is not long enough!");
	size_t copy_size = min(n,s2len);
	memcpy(s1+s1len,s2,copy_size);
	*(s1+s1len+copy_size) = '\0';
	return 0;
}

static inline int contain_char(const char *clist, char c)
{
	char *p = (char*)clist;
	while(*p != '\0' && *p != c)
		p++;
	return *p != '\0';
}
char *strtok_s(char * restrict s1, rsize_t * restrict s1max, const char * restrict s2, char ** restrict ptr)
{
	check_err(s1max != NULL && s2 != NULL && ptr != NULL,return NULL,"[strtok_s]: Invalid Null pointers!");
	check_err(s1 != NULL || *ptr != NULL,return NULL, "[strtok_s]: Invalid Null pointers!");
	check_err(strnlen_s(s2,CHARSET_MAX) < CHARSET_MAX, return NULL,"[strtok_s]: Too many separator characters!");
	char *search_str = (s1 != NULL ? s1 : *ptr);
	
	rsize_t search_len = min(*s1max,strnlen_s(search_str,*s1max));
	if(search_len == 0) return NULL;
	
	char *token_start, *token_end;
	rsize_t num_pass = 0;
	token_start = search_str;
	while(contain_char(s2,*token_start) && num_pass<search_len)
	{
		token_start++;
		num_pass++;
	}	
	if(num_pass == search_len) {
		*s1max = 0;
		return NULL;
	}
	token_end = token_start;
	while(!contain_char(s2,*token_end)&& num_pass<search_len)
	{
		token_end++;
		num_pass++;
	}
	*s1max = search_len - num_pass;
	*token_end = '\0';
	if(*s1max != 0) {
		*ptr = token_end+1;
		*s1max -= 1;
	}
	return token_start;

}



bool strstart(const char *s1, const char *s2, rsize_t s2max)
{
	check(s1 != NULL && s2 != NULL, return false);

	return strncmp(s1,s2,strnlen_s(s2,s2max)) == 0;
}
bool strend(const char *s1,rsize_t s1max, const char *s2, rsize_t s2max)
{
	check(s1 != NULL && s2 != NULL, return false);
	rsize_t s1len = strnlen_s(s1,s1max);
	rsize_t s2len = strnlen_s(s2,s2max);
	if(s1len < s2len) return false;
	char *p = (char*)s1 + s1len-s2len;
	return strncmp(p,s2,s2len) == 0;
}