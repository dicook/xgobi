/* @(#)aiclient.h	1.2 4/14/94 10:19:11 */
#define INITVAL      -9999
#define MAXSERVERS   100

typedef struct IACSERVERS_STRUCT {
  char *hostname;
  u_long program_number;
  u_long version_number;
  CLIENT *aiclnt_handle;
  } IACSERVERS;                    /* Ports to remote servers */

/*

int aiconnect    (char *host, u_long prognum, u_long vernum);
int aidisconnect (int server_id);
int airequest    (int server_id, u_long ProcNum, char *ReqStr, 
                  char *RetStr, double timeout);

*/
