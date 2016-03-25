/************************************************************************/
/*                                                                      */
/* File "rpc_dceerror.h"                                                */
/*                                                                      */
/* Author : Martin Schneider                                            */
/* Date   : 28/07/99                                                    */
/*                                                                      */
/************************************************************************/

/* Managing DCE-Errors */

#define ABORT  0
#define RESUME 1

#define CHECK_DCE_ERROR(status,action) \
	if (status != rpc_s_ok) { \
		static unsigned char error_text[100]; \
		unsigned32 rc; \
		dce_error_inq_text(status,error_text,&rc); \
		fprintf(stderr,"Error: %s (File %s, Line %d)\n", \
			error_text,__FILE__,__LINE__); \
		if (action==ABORT) exit(1); \
	}
