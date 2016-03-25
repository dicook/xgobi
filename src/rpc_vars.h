/************************************************************************/
/*                                                                      */
/* File "rpc_vars.h"                                                    */
/*                                                                      */
/* Author : Martin Schneider, Juergen Symanzik                          */
/* Date   : 28/07/99                                                    */
/*                                                                      */
/************************************************************************/

#ifndef PROGINTERN
#define PROG_ extern
#else
#define PROG_
#endif

#ifdef DEBUG

#define DTEXT(str) fprintf(stderr,str)
#define DTEXT1(str,p1) fprintf(stderr,str,p1)
#define DTEXT2(str,p1,p2) fprintf(stderr,str,p1,p2)

#else

#define DTEXT(str)
#define DTEXT1(str,p1)
#define DTEXT2(str,p1,p2)

#endif

#ifdef RPC_USED

#include <rpc/rpc.h>
/*
 * You might want some global variables
 */
PROG_ unsigned long xg_server_number /* previously av_client_number */;
PROG_ unsigned long av_server_number;
PROG_ unsigned long xpl_server_number;
PROG_ unsigned long vg_server_number;
PROG_ int av_server_id;
PROG_ int xpl_server_id;
PROG_ int vg_server_id;
PROG_ int is_running;

#endif


#ifdef DCE_RPC_USED

#include <dce/dce.h>
#include <dce/pthread.h>
#include "rpc_dce.h"
#include "rpc_dceerror.h"

/* For the xgobi server */
PROG_ pthread_mutex_t mutex;
PROG_ pthread_t t_handle;
PROG_ unsigned char *pszProtocolSequence[100];
PROG_ unsigned char *pszEndpoint;
PROG_ unsigned int   cMaxCalls;
PROG_ rpc_binding_vector_p_t pbvBindings;
PROG_ int is_running;

#define MAX_CALL_THREADS 10
/* For the RPC-calls xgobi->WorldView (f.i.) */
PROG_ unsigned char pszNWAddressCl[100];
PROG_ pthread_mutex_t mutexCl;
PROG_ anzThreads;
/* valid values starting always at index 0 */
PROG_ pthread_t t_handles_client[MAX_CALL_THREADS];


#endif

