#ifndef __COMMON_UTILS_H__
#define __COMMON_UTILS_H__


#include "ds/stack.h"
#include "def.h"

#define min(m,n) ((m)<(n)?(m):(n))
#define max(m,n) ((m)>(n)?(m):(n))

/*string to number functions*/
int stoi(const char *, int *);
int stol(const char *, long *);
int stoll(const char *, long long*);
int stod(const char *, double *);

/*---------------------------------------------*/
#define Check_mem_size(num,actions) check_err((num) < RSIZE_MAX && (num) > 0,actions,"invalid size_t param  ["#num"]: %lld", (long long)(num))

/*on error cleanup facility*/
typedef struct err_cleanup
{
	sstack_ent_t sent;
	int id;
	void *src;
} err_cleanup_t;
#define ERR_CLEAN_INIT() \
	SSTACK(_err_cleanup_sstack_)
#define ERR_CLEAN_PUSH(i,s,name) \
	err_cleanup_t cleanup_##name = {.id = (i), .src = (s)};\
	sstack_push(&(cleanup_##name.sent),&_err_cleanup_sstack_)

#define ERR_CLEAN_POP() \
	(void)sstack_pop(&_err_cleanup_sstack_)
#define ERR_CLEAN_BEGIN \
	while(!sstack_isempty(&_err_cleanup_sstack_))\
	{\
		err_cleanup_t *cleanup_entp = sstack_entry(sstack_pop(&_err_cleanup_sstack_),err_cleanup_t,sent);\
		void *cleanup_resource = cleanup_entp->src;\
		switch(cleanup_entp->id)\
		{\
			/*case*/
#define ERR_CLEAN_END }}
/*---------------------------------------------*/

/*bit operations*/
#define _sanitize_pos(pos) ((pos>0)?pos:0)
#define bit(val, pos) ((bit_t)((val>>(_sanitize_pos(pos)))&0x01))
#define setbit(val, pos) (val|(0x01<<(_sanitize_pos(pos))))
#define unsetbit(val, pos) (val&(~(0x01<<(_sanitize_pos(pos)))))
/*---------------------------------------------*/
#endif