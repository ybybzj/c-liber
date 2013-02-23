#include "dbg.h"
#include "safestr.h"
#include "console.h"

int cls_prompt(const char *prompt, char *out, size_t maxlen)
{
	check(out != NULL && maxlen > 0,errno = EINVAL; return -1);
	printf("%s ",prompt);
	fflush(stdout);
	size_t len;
	if(fgets(out, maxlen, stdin) != NULL){
		len = strnlen_s(out,maxlen);
		if(*(out + len - 1) == '\n')
		{
			*(out + len - 1) = '\0';
			len--;	
		}else{
			printf("\n");
		}
	}else{
		*out = '\0';
		len = 0;
		printf("\n");
		if(ferror(stdin) != 0){
			print_err("cls_input error");
			return -1;
		}	
	}
	return (int)len;
}