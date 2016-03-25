/* rpc_xpl.c */
/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  this  code and associated documentation,  provided      *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *    Copyright (c) 1997 Sigbert Klinke                     *
 *                       <sigbert@wiwi.hu-berlin.de>        *
 *  All Rights Reserved.                                    *
 *                                                          *
 ************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>

#include "xincludes.h"
#include <X11/keysym.h>
#include "xgobitypes.h"
#include "xgobivars.h"

#define PROGINTERN
#include "rpc_xgobi.h"
#include "rpc_vars.h"
#include "rpc_aiiac.h"

static char message[MSGLENGTH];

extern xgobidata xgobi;
extern int groupsort();
extern double *xsort;
extern int *gsort;

#ifdef XPLORE

void startxplore (xgobidata *xg)

{ 
  Boolean RunWorkProcs ();
  
  char command[256];


  rpc_server_main (xg_server_number, (u_long) 1);
  
  sprintf (command, "$XPL4HOME/xplore -ini $XPL4HOME/XploRe.ini -startup $XPL4LIB/xgobistartup.xpl -server %x -client %x -link xgobi &",
    xpl_server_number, xg_server_number); 
  system (command);

  sleep(5);
 
  xpl_server_id = rpc_client_main (xpl_server_number, (u_long) 1);

  if (xpl_server_id >= 0)
  {
    active_xplore_buttons();
    xg -> xplore_flag = True;
  
    (void) XtAppAddWorkProc (app_con, RunWorkProcs, NULL);
  }
  else
  {
    sprintf (message, "XGobi: Cannot start XploRe.\n");
    show_message(message, (xgobidata *) &xgobi);
  }
  
  rpc_xplore_pass_info ((xgobidata *) &xgobi);
}


void stopxplore (xgobidata *xg)

{
  char *RetStr;
  extern int status;

  double time = TIMEOUT;

  RetStr = (char *) XtMalloc ((Cardinal) 512);

  status = airequest (xpl_server_id, 100, "", RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe request #100 (Terminate XploRe) - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
  }

  inactive_xplore_buttons();
  xg -> xplore_flag = False;

  XtFree ((XtPointer) RetStr);
}


void build_vectord (int n, double *x, char *outstr)

{
  int i;
  char tmp[80];

  sprintf (tmp, "#(");
  strcat (outstr, tmp);
  for (i=0; i < n-1; i++)
    {
      sprintf (tmp, "%g,", x[i]);
      strcat (outstr,tmp);
    }
   sprintf (tmp, "%g)", x[i]);
   strcat (outstr,tmp);
}


void build_vectori (int n, int *x, char *outstr)

{
  int i;
  char tmp[80];

  sprintf (tmp, "#(");
  strcat (outstr, tmp);
  for (i=0; i < n - 1; i++)
    {
      sprintf (tmp, "%i,", x[i]);
      strcat (outstr,tmp);
    }
   sprintf (tmp, "%i)", x[i]);
   strcat (outstr,tmp);
}


void build_lvectorf (xgobidata *xg, int n, char *outstr)

{
  int i;
  int first = TRUE;
  char tmp[80];

  strcat (outstr, "(");
  for (i = 0; i < xg->ncols_used; i++)
    if (xg->selectedvars[i])
      {
        if (first)
        {
          if (xg->missing_values_present)
          {
            if (xg->is_missing[n][i])
              sprintf (tmp, "NaN");
            else
              sprintf (tmp, "%g", xg->raw_data[n][i]);
          }
          else
            sprintf (tmp, "%g", xg->raw_data[n][i]);
          first = FALSE;
        }
        else
          {
            if (xg->missing_values_present)
            {
              if (xg->is_missing[n][i])
                sprintf (tmp, "~NaN");
              else
                sprintf (tmp, "~%g", xg->raw_data[n][i]);
            }
            else
              sprintf (tmp, "~%g", xg->raw_data[n][i]);
          }
        strcat (outstr, tmp);
      }
  strcat (outstr, ")");
}


int rpc_xplore_smoother (char *smoother, int choice,
                          int n, double *x, double *y, int *g,
                          long smoothparam, double *ye)

{ 
  char *DataStr;
  char *RequStr;
  char *RetStr;
  char tempRetStr[1024];
  extern int server_id;
  extern int status;

  double time = TIMEOUT;

  char tmp[80], *tok;  
  int i;


  DataStr = (char *) XtMalloc ((Cardinal) 512 + n * 30);
  RequStr = (char *) XtMalloc ((Cardinal) 512 + n * 30);
  RetStr = (char *) XtMalloc ((Cardinal) 512 + n * 30);

  strcpy (DataStr, ""); /* concatenate output string */

  sprintf (DataStr, "XGobiSmooth=%s(", smoother);
  build_vectord (n, x, DataStr);
  strcat (DataStr, ",");
  build_vectord (n, y, DataStr);
  strcat (DataStr, ",");
  build_vectori (n, g, DataStr);
  
  if ((choice >= 5) && (choice <= 9))
    /* xplore smoothers 4 (LINEAR) and 10 (ISOTONIC) do not need a parameter */
  {
    strcat (DataStr, ",");
    sprintf (tmp, "%ld", -smoothparam); 
       /* negative value indicates special purpose to XploRe smooth function */
    strcat (DataStr, tmp);
  }

  strcat (DataStr, ")");

  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe smooth request #1 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }

  sprintf(RequStr, "XGobiSmooth");
  sprintf(RetStr, "");
  
  status = airequest (xpl_server_id, 3, RequStr, tempRetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe smooth request #2 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }

  while (strlen(tempRetStr) == 1023)
  {
    strcat(RetStr, tempRetStr);
    
    status = airequest (xpl_server_id, 3, RequStr, tempRetStr, time);
    if (status != 0) 
    {
      sprintf (message, "XGobi: Error in submitting XploRe smooth request #3 - %d\n", status);
      show_message(message, (xgobidata *) &xgobi);
      XtFree ((XtPointer) DataStr);
      XtFree ((XtPointer) RequStr);
      XtFree ((XtPointer) RetStr);
      return(1);
    }
  }
  
  strcat(RetStr, tempRetStr);
  
    
  i = 1;
  tok   = strtok (RetStr, " \t\n");  
  ye[0] = atof(tok);
  while (tok = strtok(NULL, " \t\n"))
  {
    ye[i] = atof(tok);
    i++;
  }
  
  XtFree ((XtPointer) DataStr);
  XtFree ((XtPointer) RequStr);
  XtFree ((XtPointer) RetStr);
  return(0);
}  


int rpc_xplore_pass_info (xgobidata *xg)

{ 
  char *DataStr;
  char *RequStr;
  char *RetStr;
  extern int status;

  double time = TIMEOUT;


  DataStr = (char *) XtMalloc ((Cardinal) 512);
  RequStr = (char *) XtMalloc ((Cardinal) 512);
  RetStr = (char *) XtMalloc ((Cardinal) 512);

  sprintf (DataStr, "xg1=0"); /* create output string (xg1) */
  sprintf (RetStr, " ");
    
  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe (pass info) request #1 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }


  sprintf (DataStr, "clientport1=\"0x%x\"", xg_server_number); /* create output string (clientport1) */
  sprintf (RetStr, " ");
    
  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe (pass info) request #1 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }

  XtFree ((XtPointer) DataStr);
  XtFree ((XtPointer) RequStr);
  XtFree ((XtPointer) RetStr);
  return(0);
}  


int rpc_xplore_pass_data (xgobidata *xg)

{ 
  char *DataStr;
  char *RequStr;
  char *RetStr;
  extern int status;

  double time = TIMEOUT;

  char tmp[80];
  int n = xg->nrows_in_plot * xg->ncols_used;
  int i, symbol;
  int first = 1;
  

  /* make sure at least one variable has been selected */
  for (i = 0; i < xg->ncols_used; i++)
    if (xg->selectedvars[i])
      break;
  if (i == xg->ncols_used)
  {
    show_message ("Select at least one variable in the Variable List.\n", xg);
    return (1);
  }

  DataStr = (char *) XtMalloc ((Cardinal) 512 + n * 15);
  RequStr = (char *) XtMalloc ((Cardinal) 512 + n * 15);
  RetStr = (char *) XtMalloc ((Cardinal) 512 + n * 15);

  sprintf (DataStr, "XGobiData="); /* concatenate output string (Data) */
  sprintf (RetStr, " ");
    
  build_lvectorf (xg, 0, DataStr);

  for (i = 1; i < xg->nrows_in_plot; i++)
  {
    strcat (DataStr, "|");
    build_lvectorf (xg, i, DataStr);
  }

  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe (pass variables) request #1 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }


  sprintf (DataStr, "XGobiVars="); /* concatenate output string (Vars) */
  sprintf (RetStr, " ");

  for (i = 0; i < xg->ncols_used; i++)
    if (xg->selectedvars[i])
    {
      if (first)
      {
        sprintf (tmp, "\"%s\"", xg->collab[i]);
        first = 0;
      }
      else
        sprintf (tmp, "|\"%s\"", xg->collab[i]);
      strcat (DataStr, tmp);
    }

  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe (pass variables) request #2 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }


  sprintf (DataStr, "XGobiInfo=getxgobiinfo("); /* concatenate output string (Info) */

  /*make sure we do not update XGobi when first brushing in XploRe */
  symbol = glyph_color_pointtype (xg, 0);
  xg->last_forward[0] = symbol;
  sprintf (tmp, "%i", symbol);
  strcat (DataStr, tmp);

  for (i = 1; i < xg->nrows_in_plot; i++)
  {
    symbol = glyph_color_pointtype (xg, i);
    xg->last_forward[i] = symbol;
    sprintf (tmp, "|%i", symbol);
    strcat (DataStr, tmp);
  }
  
  strcat (DataStr, ")");

  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe (pass variables) request #3 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }


  sprintf (DataStr, "setmaskp(XGobiData,XGobiInfo.xplcolor,XGobiInfo.xplsymbol,XGobiInfo.xplsize)"); 
    /* create output string (setmaskp) */

  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe (pass variables) request #4 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }

  XtFree ((XtPointer) DataStr);
  XtFree ((XtPointer) RequStr);
  XtFree ((XtPointer) RetStr);
  return(0);
}  


int rpc_xplore_pass_projection (xgobidata *xg)

{ 
  char *DataStr;
  char *RequStr;
  char *RetStr;
  extern int status;

  double time = TIMEOUT;

  char tmp[80];
  int n = xg->nrows_in_plot * 2;
  int i, symbol;


  DataStr = (char *) XtMalloc ((Cardinal) (512 + n * 15));
  RequStr = (char *) XtMalloc ((Cardinal) (512 + n * 15));
  RetStr = (char *) XtMalloc ((Cardinal) (512 + n * 15));

  sprintf (DataStr, "XGobiProjection="); /* concatenate output string */
  sprintf (RetStr, " ");

  for (i=0; i < xg->nrows_in_plot - 1; i++)
    {
      sprintf (tmp, "(%d~%d)|", xg->planar[i].x, xg->planar[i].y);
      strcat (DataStr,tmp);
    }
    
  sprintf (tmp, "(%d~%d)", xg->planar[i].x, xg->planar[i].y);
  strcat (DataStr,tmp);

  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe (pass projection) request #1 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }

  sprintf (DataStr, "XGobiInfo=getxgobiinfo("); /* concatenate output string (Info) */

  /*make sure we do not update XGobi when first brushing in XploRe */
  symbol = glyph_color_pointtype (xg, 0);
  xg->last_forward[0] = symbol; 
  sprintf (tmp, "%i", symbol);
  strcat (DataStr, tmp);

  for (i = 1; i < xg->nrows_in_plot; i++)
  {
    symbol = glyph_color_pointtype (xg, i);
    xg->last_forward[i] = symbol;
    sprintf (tmp, "|%i", symbol);
    strcat (DataStr, tmp);
  }
  
  strcat (DataStr, ")");

  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe (pass projection) request #2 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }

  sprintf (DataStr, "setmaskp(XGobiProjection,XGobiInfo.xplcolor,XGobiInfo.xplsymbol,XGobiInfo.xplsize)"); 
    /* create output string (setmaskp) */

  sprintf (RequStr, "%x %s", xg_server_number, DataStr);

  status = airequest (xpl_server_id, 1, RequStr, RetStr, time);
  if (status != 0) 
  {
    sprintf (message, "XGobi: Error in submitting XploRe (pass projection) request #3 - %d\n", status);
    show_message(message, (xgobidata *) &xgobi);
    XtFree ((XtPointer) DataStr);
    XtFree ((XtPointer) RequStr);
    XtFree ((XtPointer) RetStr);
    return(1);
  }

  XtFree ((XtPointer) DataStr);
  XtFree ((XtPointer) RequStr);
  XtFree ((XtPointer) RetStr);
  return(0);
}  


void xplore_smoother(char *smoother,  int choice, 
                     long *x, long *y, int n, long *sm_pars, 
                     int num_grp, int *grp_id, int *pnum_pts)

/* Di uses global variables x_sm and y_sm for smoothing !! */

{
  extern long *x_sm;
  extern long *y_sm;
  extern int *grp_id_sm;

  int i, *g, *index;
  double *xsk, *ysk, *ye;

  if (x_sm)
    XtFree ((char *) x_sm);
  x_sm = (long *) XtMalloc (n*sizeof(long));

  if (y_sm)
    XtFree ((char *) y_sm);
  y_sm = (long *) XtMalloc (n*sizeof(long));

  if (grp_id_sm)
    XtFree ((char *) grp_id_sm);
  grp_id_sm = (int *) XtMalloc (n*sizeof(int));

  xsk = (double *) XtMalloc (n*sizeof(double));
  ysk = (double *) XtMalloc (n*sizeof(double));
  index = (int *) XtMalloc (n*sizeof(int));

  for (i=0; i<n; i++)
  {
    xsk[i] = x[i];
    ysk[i] = y[i];
    index[i] = i;
  } 
  
  ye = (double *) XtMalloc (n*sizeof(double));
  g  = (int *) XtMalloc (n*sizeof(int));
  
  for (i=0; i<n; i++)
    g[i] = grp_id[i]-1;    
 
  if (rpc_xplore_smoother (smoother, choice, n, xsk, ysk, g, *sm_pars, ye))
    /* 0 is OK, 1 is Error */
  {
    *pnum_pts = 0;
    return;
  }
 
  xsort = xsk;
  gsort = g;

  qsort ((char *) index, n, sizeof(int), groupsort);

  for (i=0; i<n; i++)
  {
    x_sm[i] = xsk[index[i]];
    y_sm[i] = ye[index[i]];
    grp_id_sm[i] = g[index[i]]+1; 
  }

  *pnum_pts = n;

  XtFree ((XtPointer) index);
  XtFree ((XtPointer) xsk);
  XtFree ((XtPointer) ysk);
  XtFree ((XtPointer) ye);
  XtFree ((XtPointer) g);
}

#endif


/* ARGSUSED */
XtCallbackProc
pass_data_xplore_cback (Widget w, xgobidata *xg, XtPointer callback_data)

{

#ifdef XPLORE

  rpc_xplore_pass_data (xg);
  
#endif

}


/* ARGSUSED */
XtCallbackProc
pass_projection_xplore_cback(Widget w, xgobidata *xg, XtPointer callback_data)

{

#ifdef XPLORE

  rpc_xplore_pass_projection (xg);
  
#endif

}

