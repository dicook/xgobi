/***********************************************************************
 * Permission is hereby granted to any individual or institution       *
 * for use, copying, or redistribution of the AV2XGobi C code          *
 * and associated documentation, provided such code and documentation  *
 * are not sold for profit and the following copyright notice is       *
 * retained in the code and documentation:                             *
 *                                                                     *
 *   Copyright (c) 1995 Iowa State University                          *
 *                                                                     *
 * We encourage you to share questions, comments and modifications.    *
 *                                                                     *
 *   Juergen Symanzik (symanzik@iastate.edu)                           *
 *   Dianne Cook (dicook@iastate.edu)                                  *
 *   James J. Majure (majure@iastate.edu)                              *
 *                                                                     *
 * The code in this file is based on the file aiclient.c               *
 * provided by ESRI.                                                   *
 * It has been modified to provide the functionality required for      *
 * the AV2XGOBI link.                                                  *
 *                                                                     *
 ***********************************************************************/

/* @(#)aiclient.c	1.8 1/25/95 10:57:31 */

#ifdef RPC_USED

#include <rpc/rpc.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "rpc_aiiac.h"
#include "rpc_client.h"
#include "rpc_aiiac_err.h"

IACSERVERS iacservers[MAXSERVERS];
int aiclient_init = 0;
static double curtimeout = -1.0;
static struct timeval maxtime;


/*
*-----------------------------------------------------------------------
*
*HB {NoManual} {001011} {aiiac}
*
*-----------------------------------------------------------------------
*
*N  {aiclientinit} Initialize the IAC client module.
*
*-----------------------------------------------------------------------
*
*P  Purpose:
*            This function initializes the IAC client module. If called
*            multiple times, it does nothing after the first invocation.
*            It is automatically called by getserver_id. 
*E
*
*-----------------------------------------------------------------------
*
*A  Arguments:
*
*   {{}}
*
*E
*
*-----------------------------------------------------------------------
*
*H  History:
*
*   Ravi Narasimhan              [2/10/94]          Original coding.
*E
*CHE
*-----------------------------------------------------------------------
*/

void aiclientinit ()

{
 int i;

   if (aiclient_init) return;

   for(i=0;i<MAXSERVERS;i++)
     iacservers[i].program_number = INITVAL;

   aiclient_init = 1;
}



/*
*-----------------------------------------------------------------------
*
*HB {NoManual} {001011} {aiiac}
*
*-----------------------------------------------------------------------
*
*N  {find_empty_server_slot} Find empty space in server storage.
*
*-----------------------------------------------------------------------
*
*P  Purpose:
*            This function finds empty space in the server storage.
*E
*
*-----------------------------------------------------------------------
*
*A  Arguments:
*
*   {{}}
*   {RETURN <Output> === (int) empty space index:
*                              >=  0 - empty space index
*                               = -3 - no more space left}
*
*E
*
*-----------------------------------------------------------------------
*
*H  History:
*
*    Ravi Narasimhan          [2/15/93]          Original coding.
*E
*CHE
*-----------------------------------------------------------------------
*/

int find_empty_server_slot()

{
 int i,return_status;


   return_status = ERR_REACHED_MAX_SERVERS;
   for(i=0;i<MAXSERVERS;i++)
     if (iacservers[i].program_number == INITVAL) {
       return_status = i;
       break;
       }

#ifdef DEBUG
   fprintf(stderr,"client:    found port slot %d\n",return_status);
#endif

   return(return_status);
}



/*
*-----------------------------------------------------------------------
*
*HB {Manual} {001011} {aiiac}
*
*-----------------------------------------------------------------------
*
*N  {aiconnect} Connect to a server.
*
*-----------------------------------------------------------------------
*
*P  Purpose:
*    This function connects to a server identified by the hostname,
*    program-number and version-number. It returns an integer server-id,
*    which is used for further communication with the server.
*E
*
*-----------------------------------------------------------------------
*
*A  Arguments:
*
*   {{host,prognum,vernum}}
*   {host    <Input>      === (char *) Hostname}
*   {prognum <Input>      === (unsigned long int) Program number}
*   {vernum  <Input>      === (unsigned long int) Version number}
*   {RETURN  <Output>     === (int)   >= 0 && < 100 Server identifier 
*                                     101 Reached Max. Servers - 100
*                                     109 Could not connect to server}
*
*
*E
*
*-----------------------------------------------------------------------
*
*H  History:
*
*   Ravi Narasimhan              [2/10/94]          Original coding.
*E
*CHE
*-----------------------------------------------------------------------
*/

int aiconnect (host, prognum, vernum)

char *host;
u_long prognum;
u_long vernum;

{
 int server_id;
 
#ifdef DEBUG
   fprintf(stderr,"CLIENT:--> aiconnect()\n");
#endif

     if (! aiclient_init) aiclientinit ();

/* Find empty port slot to hold server info. */

     if ((server_id = find_empty_server_slot()) == ERR_REACHED_MAX_SERVERS) 
	return ERR_REACHED_MAX_SERVERS;
       
/* Create client handle */

     iacservers[server_id].aiclnt_handle = 
		clnt_create (host,prognum,vernum,"tcp");
     if (iacservers[server_id].aiclnt_handle == NULL) {
        clnt_pcreateerror ("aiconnect");
	return ERR_NO_CLIENT_HANDLE;
     }

/* Store server information */

     iacservers[server_id].hostname =  (char *) malloc (strlen (host) + 1);
     if (iacservers[server_id].hostname != NULL)
	strcpy (iacservers[server_id].hostname, host);
     iacservers[server_id].program_number = prognum;
     iacservers[server_id].version_number = vernum;

/* Create UNIX authentication */

     iacservers[server_id].aiclnt_handle -> cl_auth = authunix_create_default ();

#ifdef DEBUG
   fprintf(stderr,"aiconnect:    return value is %d \n",server_id);
#endif

   return server_id;
}



/*
*-----------------------------------------------------------------------
*
*HB {Manual} {001011} {aiiac}
*
*-----------------------------------------------------------------------
*
*N  {aidisconnect} Disconnect from a server.
*
*-----------------------------------------------------------------------
*
*P  Purpose:
*    This function disconnects from a server identified by the server-id.
*E
*
*-----------------------------------------------------------------------
*
*A  Arguments:
*
*   {{server-id}}
*   {server-id    <Input>      === (int) Server Identifier}
*   {RETURN       <Output>     === (int)   = 0 Bad Server identifier 
*                                            1 Okay} 
*
*E
*
*-----------------------------------------------------------------------
*
*H  History:
*
*   Ravi Narasimhan              [2/10/94]          Original coding.
*E
*CHE
*-----------------------------------------------------------------------
*/

int aidisconnect (server_id)

int server_id;

{
  if (! aiclient_init) return 0;
  if (server_id < 0                    || 
      server_id > MAXSERVERS              ||
      iacservers[server_id].program_number == INITVAL) return 0;
 
  iacservers[server_id].program_number = INITVAL;   /* Mark slot as available */
  free (iacservers[server_id].hostname);            /* Free string storage */
  clnt_destroy (iacservers[server_id].aiclnt_handle); /* Free client
handle storage */
  
  return 1;
}



/*
*-----------------------------------------------------------------------
*
*HB {Manual} {001011} {aiiac}
*
*-----------------------------------------------------------------------
*
*N  {airequest} Send a request to a server.
*
*-----------------------------------------------------------------------
*
*P  Purpose:
*    This function sends a request to execute a specified procedure number
*    to a server, with a string as an argument for the procedure. The results
*    of the request are concatenated to the status of the request 
*    (which is a string KEYWORD)  and returned with the return string. 
*    The two pieces of information are delimited by SPACE.  
*
*    When a request is made to an ArcInfo server, the following protocol has
*    to be observed.
*    Procedure Number - 1  
*              Execute the string argument as a command in the
*                           current environment
*              The return value is a token number converted to a string
*    Procedure Number - 2 
*              Get the status of request identified by token number string
*              The return value is the status of the job and the results of
*                  the job, if the job is completed.
*                  QUEUE             - Job is in queue
*                  PROCESSING        - Job is being processed by server
*                  RESULTS Results   - Job done, the result string follows 
*                  DONE              - Job done, results have already been 
*                                                retrieved.
*               
*E
*
*-----------------------------------------------------------------------
*
*A  Arguments:
*
*   {{server-id, ProcedureNum, RequestString, ReturnString, timeout}}
*   {server-id    <Input>      === (int) Server identifier returned by aiconnect}
*   {ProcedureNum <Input>      === (unsigned long int) Procedure Number}
*   {RequestString <Input>     === (char *) Argument for procedure number}
*   {ReturnString <Output>     === (char *) Return value from server}
*   {timeout      <Input>      === (double) maximum time in secs. to wait for
*                                          acknowledgement}
*   {RETURN  <Output>     === (int)   =   0   Okay 
*                                       100 Bad server id.
*                                       110 Timed out
*                                       199 Other error}
*
*
*E
*
*-----------------------------------------------------------------------
*
*H  History:
*
*   Ravi Narasimhan              [2/10/94]          Original coding.
*E
*CHE
*-----------------------------------------------------------------------
*/

int airequest (server_id, ProcNum, ReqStr, RetStr, timeout)

int server_id;
u_long ProcNum;
char *ReqStr;
char *RetStr;
double timeout;

{
 enum clnt_stat client_status;

#ifdef DEBUG
   fprintf(stderr,"airequest ()\n");
#endif

   if (! aiclient_init) return ERR_BAD_SERVER;
   if (server_id < 0                       || 
       server_id > MAXSERVERS-1            ||
       iacservers[server_id].program_number == INITVAL) 
     return ERR_BAD_SERVER;

   if (timeout != curtimeout) {
     maxtime.tv_sec  = (int)timeout;
     maxtime.tv_usec = (int)((timeout - (float)maxtime.tv_sec) * 1000000.0);
#ifdef DEBUG
   fprintf (stderr,"calling clnt_control with timeout %f\n",timeout);
#endif
     clnt_control(iacservers[server_id].aiclnt_handle,
                  CLSET_TIMEOUT,
                  (char *) &maxtime);
     curtimeout = timeout;
   }

   client_status = clnt_call(iacservers[server_id].aiclnt_handle,
                             ProcNum,
                             (xdrproc_t) xdr_wrapstring, (char *) &ReqStr,
                             (xdrproc_t) xdr_wrapstring, (char *) &RetStr,
                             maxtime);
 
   if (client_status != RPC_SUCCESS) {
     clnt_perror (iacservers[server_id].aiclnt_handle,
                  "XGobi aiclient airequest");
     printf ("XGobi RetStr = #%s#\n", RetStr);
     if (client_status == RPC_TIMEDOUT) return ERR_TIMED_OUT;

     return ERR_GENERAL_ERROR;
   }

   return(ERR_NOERROR);
}

#endif
