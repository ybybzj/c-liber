#ifndef __DBG_H__
#define __DBG_H__

#include "def.h"
#include "clsclr.h"
#include <string.h>

/*
*	Debugger facilities
*/
#ifdef NDEBUG
#define debug(M,...) 
#else
#define debug(M,...) fprintf(stderr, cls_s("DEBUG",cls_Bold;cls_hCyan)" %s:%d: " M "\n",__FILE__,__LINE__,##__VA_ARGS__)			
#endif

#define str_errno() (errno == 0 ? "None" : strerror(errno))

#define print_err(M,...) fprintf(stderr, cls_s("[ERROR]",cls_Bold;cls_hRed) cls_s(" (%s:%d: errno: %s) " M,cls_hBlack) "\n", __FILE__, __LINE__, str_errno(), ##__VA_ARGS__)

#define print_warn(M,...) fprintf(stderr, cls_s("[WARN]",cls_Bold;cls_hYellow) cls_s(" (%s:%d: errno: %s) " M,cls_hBlack) "\n", __FILE__, __LINE__, str_errno(), ##__VA_ARGS__)

#define print_info(M,...) fprintf(stderr, cls_s("[INFO]",cls_hBlue) " (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check_err(A, actions, M, ...) if(!(A)){ print_err(M, ##__VA_ARGS__); errno = 0; actions;}

#define sentinel(actions, M, ...) {print_err(M, ##__VA_ARGS__); errno = 0; actions;}

#define check_debug(A, actions, M, ...) if(!(A)){ debug(M, ##__VA_ARGS__); errno = 0; actions;}

#define check(A,actions) if(!(A)){print_err(#A); errno = 0;actions;}
//use for pthread funtions
#define check_t(A,action) do{\
	int s;\
	s = (A);\
    if (s != 0){\
        fprintf(stderr, cls_s("[ERROR]",cls_Bold;cls_hRed) cls_s(" (%s:%d: errno: %s) " #A,cls_hBlack) "\n", __FILE__, __LINE__, strerror(s));\
    	action;\
    }\
}while(0);

#define log_err(outputStream, M, ...) if((outputStream) != NULL){fprintf((outputStream), "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, str_errno(), ##__VA_ARGS__)}

#define log_warn(outputStream, M, ...) if((outputStream) != NULL){fprintf((outputStream), "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, str_errno(), ##__VA_ARGS__)}

#define log_info(outputStream, M, ...) if((outputStream) != NULL){fprintf((outputStream), "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)}

#define println(M,...) printf(M "\n",##__VA_ARGS__)

#endif
