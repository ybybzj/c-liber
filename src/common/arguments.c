#include "arguments.h"
#include "dbg.h"
int make_arguments(va_list argList, arg_data_t *arguments, int len)
{
	check(len > 0 && arguments != NULL, errno = EINVAL;return -1);
	int arg_len = 0;
	arg_data_t arg;
	while(!argisfin(arg = va_arg(argList,arg_data_t)) && len > 0){
		arguments[arg_len++] = arg;
		len--;
	}
	return arg_len;
}