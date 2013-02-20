#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
/*common facilities*/
#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
typedef enum{false,true} bool;
#define inline 
#define restrict
#endif

typedef unsigned char byte;
typedef size_t rsize_t;
typedef uint8_t bit_t;

#ifndef RSIZE_MAX
#define RSIZE_MAX (SIZE_MAX>>1)
#endif

#define memberp_to_structp(mptr, stype, mname) \
	((stype *)((byte *)(mptr)-offsetof((stype),(mname)))
#endif

//GCC COMPILE FLAG
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#define NORETURN __attribute__((__noreturn__))
#define FMTCHK(m,n) __attribute__((format(printf, m, n))) //m,n can only be 1,2,3...
#else
#define UNUSED
#define NORETURN
#define FMTCHK(m,n)
#endif