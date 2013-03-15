#ifndef __FUNCTION_H__
#define __FUNCTION_H__
#include <stdarg.h>
typedef enum{ArgType_Int,ArgType_Float,ArgType_Ptr,ArgType_Null,ArgType_Fin} ArgType;
typedef struct{
	ArgType type;
	union{
		long l;
		double d;
		void *ptr;
	}data;
}arg_data_t;

#define ftoarg(f) ((arg_data_t){.type = ArgType_Float,.data.d = (double)(f)})
#define itoarg(i) ((arg_data_t){.type = ArgType_Int,.data.l = (long)(i)})
#define ptoarg(p) ((arg_data_t){.type = ArgType_Ptr,.data.ptr = (void*)(p)})
#define argtif(a) ((a).data.d)
#define argtoi(a) ((a).data.l)
#define argtop(a) ((a).data.ptr)

#define nullarg() ((arg_data_t){.type = ArgType_Null})
#define finarg() ((arg_data_t){.type = ArgType_Fin})
#define argisf(arg) ((arg).type == ArgType_Float)
#define argisi(arg) ((arg).type == ArgType_Int)
#define argisp(arg) ((arg).type == ArgType_Ptr)
#define argisnull(arg) ((arg).type == ArgType_Null)
#define argisfin(arg) ((arg).type == ArgType_Fin)

#define Arguments int arg_len, arg_data_t arguments[static arg_len]

#define ARGUMENT_MAX 10
int make_arguments(va_list argList, arg_data_t *arguments, int len);
#endif