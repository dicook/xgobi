/* rpc_xgobi.c */
/***********************************************************************
 * Permission is hereby granted to any individual or institution       *
 * for use, copying, or redistribution of the AV2XGobi C code          *
 * and associated documentation, provided such code and documentation  *
 * are not sold for profit and the following copyright notice is       *
 * retained in the code and documentation:                             *
 *                                                                     *
 *   Copyright (c) 1995, 1996, 1997 Iowa State University              *
 *                                                                     *
 * We encourage you to share questions, comments and modifications.    *
 *                                                                     *
 *   Juergen Symanzik (symanzik@iastate.edu)                           *
 *   Dianne Cook (dicook@iastate.edu)                                  *
 *   James J. Majure (majure@iastate.edu)                              *
 *   Martin Schneider (maschn@informatik.uni-marburg.de)               *
 *                                                                     *
 * Parts of the code in this file originated from the original XGobi.  *
 *                                                                     *
 * The basic idea for the code in this file originated from the        *
 * file math.c provided by ESRI. However, no part of it has been       *
 * reused in this code. For completeness, we maintain the original     *
 * Copyright notice:                                                   *
 *                                                                     *
 ***********************************************************************/

/* @(#)math.c   1.1 4/28/94 10:20:01
*
*-----------------------------------------------------------------------
*
*HB {Manual} {001011} {IAC}
*
*-----------------------------------------------------------------------
*
*N  Provide a suite of mathematical functions. 
*
*-----------------------------------------------------------------------
*
*P  Purpose:
*
*   This file contains a suite of mathematical functions, that are 
*   invoked from functions.c. These functions are used to build a
*   math server whose functions can be invoked from AML using the 
*   IAC interface.
*
*E
*
*-----------------------------------------------------------------------
*
*A  Arguments:
*
*   {{instr,outstr}}
*
*   {instr    <Input>    === (char *) Input string argument, if any}
*   {outstr   <Output>   === (char *) Results of the operation}
*
*E
*
*-----------------------------------------------------------------------
*
*H  History:
*
*    Ravi Narasimhan          [4/27/94]          Original coding.
*E
*HE
*-----------------------------------------------------------------------
*/

/* !!!MS!!! */
#if defined RPC_USED || defined DCE_RPC_USED

#ifdef RPC_USED

#define CHAR_TYPE char
#define RPC_FIRST_LINE()
#define RPC_LAST_LINE()
#define RETURN return

#else

#include "rpc_dce.h"
#include "rpc_dceclient.h"

#define CHAR_TYPE idl_char
#define RPC_FIRST_LINE()	pthread_mutex_lock(&mutex);DTEXT("In RPC ...")
#define RPC_LAST_LINE()		DTEXT(" und heraus\n");pthread_mutex_unlock(&mutex)
#define RETURN                  DTEXT(" und heraus mit Fehler!\n");pthread_mutex_unlock(&mutex);return

#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/timeb.h>

#include "xincludes.h"
#include <X11/keysym.h>
#include "xgobitypes.h"
#include "xgobivars.h"

/* !!!MS!!! */
/* Use the extern Variable declaration */
#undef PROGINTERN
#include "rpc_vars.h"
#include "rpc_xgobi.h"
#include "rpc_aiiac.h"

#define NCDFTYPES 2

typedef struct{
        int fir_obs;
        int sec_obs;
      } vario_lag_record;


static Boolean datapflag = True; 
/* calling program supplies pointer to data? */

static float **datap = (float **) NULL;
static short **missingp = (short **) NULL;
/* datapflag = T; pointers to data and missing */

static Boolean mv_missing_values_present = False;
static Boolean mv_is_missing_values_xgobi = False;
static int mv_nmissing = 0;

static int  nr = 0,
            nc = 0;
/* datapflag = T; number of rows and cols */

static char **rowp = (char **) NULL, 
            **colp = (char **) NULL;
/* datapflag = T; pointers to row and col labels */

static char name[256];

static int *size = (int *) NULL;
static int *color = (int *) NULL;
static int *glyph = (int *) NULL;
static int *erase = (int *) NULL;
static int *lastf = (int *) NULL;

static float *cdf_xvalues = (float *) NULL;
static int *cdf_xranks = (int *) NULL;
static int *cdf_inv_xranks = (int *) NULL;
static int **cdf_bitmap = (int **) NULL;
static int *cdf_brush_bitmap = (int *) NULL;
static int *cdf_onebits = (int *) NULL;
static double *cdf_weights = (double *) NULL;
static int **cdf_types = (int **) NULL;
static int cdf_def_point_color = 0;
static int *cdf_def_point_size = (int *) NULL;
static int *cdf_def_point_glyph = (int *) NULL;
static int *cdf_brush_point_size = (int *) NULL;
static int *cdf_brush_point_glyph = (int *) NULL;

static int vario_p = 0;
static int vario_n = 0;
static float vario_dist = 0.0;
static float **vario_values = (float **) NULL;
static short **vario_missings = (short **) NULL;
static int *vario_np_e_p1 = (int *) NULL;
static int *vario_np_e_p2 = (int *) NULL;
static unsigned int **vario_bin_map=(unsigned int **) NULL;
static vario_lag_record *vario_w = (vario_lag_record *) NULL;
static int **vario_ptarr1 = (int **) NULL;
static int **vario_ptarr2 = (int **) NULL;

static int vario2_p = 0;
static int vario2_n = 0;
static float vario2_dist = 0.0;
static float **vario2_values = (float **) NULL;
static short **vario2_missings = (short **) NULL;
static int *vario2_np_e_p1 = (int *) NULL;
static int *vario2_np_e_p2 = (int *) NULL;
static unsigned int **vario2_bin_map=(unsigned int **) NULL;
static vario_lag_record *vario2_w = (vario_lag_record *) NULL;
static int **vario2_ptarr1 = (int **) NULL;
static int **vario2_ptarr2 = (int **) NULL;

static int lag_p = 0;
static int lag_n = 0;
static float lag_dist = 0.0;
static float **lag_values = (float **) NULL;
static short **lag_missings = (short **) NULL;
static int *lag_np_e_p1 = (int *) NULL;
static int *lag_np_e_p2 = (int *) NULL;
static unsigned int **lag_bin_map=(unsigned int **) NULL;
static vario_lag_record *lag_w = (vario_lag_record *) NULL;
static int **lag_ptarr1 = (int **) NULL;
static int **lag_ptarr2 = (int **) NULL;

static int is_init = False;
static int has_data = False;
static int has_symbols = False;
static int is_cdf_init = False;
static int has_cdf_data = False;
static int has_cdf_symbols = False;
static int has_cdf_bitmap = False;
static int has_cdf_weights = False;
static int cdf_default_bitmap = False;
static int cdf_default_types = False;
static int is_vario_init = False;
static int has_vario_data = False;
static int has_vario_symbols = False;
static int is_vario2_init = False;
static int has_vario2_data = False;
static int has_vario2_symbols = False;
static int is_lag_init = False;
static int has_lag_data = False;
static int has_lag_symbols = False;

static char hostname[HOSTNAMELEN];

static unsigned long last_server_number = 0L;

int status;


reset_io_read_menu_cmd()
{}

reset_io_line_read_menu_cmd()
{}


/*
   Usage:     In - colp[ncols] : ncols x Str[COLLABLEN]
   Function:  01
*/

void RPC_Send_Colnames (instr, outstr)

	CHAR_TYPE *instr, outstr[100];

{
  int i = 0;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!is_init && !is_cdf_init)
  {
     sprintf (outstr, "01.01");
     RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "01.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%s", colp[i++]))
  {
     sprintf (outstr, "01.02");
     RETURN;
  } 
  
  while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nc))
    sscanf (tokptr, "%s", colp[i++]);

  if (i != nc)
  {
     sprintf (outstr, "01.02");
     RETURN;
  }

  sprintf (outstr, "01.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - rowp[nrows] : nrows x Str[ROWLABLEN]
   Function:  02
*/

void RPC_Send_Rownames (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!is_init && !is_cdf_init)
  {
     sprintf (outstr, "02.01");
     RETURN;
  } 

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "02.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%s", rowp[i++]))
  {
     sprintf (outstr, "02.02");
     RETURN;
  } 
  
  while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr))
    sscanf (tokptr, "%s", rowp[i++]);

  if (i != nr)
  {
     sprintf (outstr, "02.02");
     RETURN;
  }

  sprintf (outstr, "02.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - NULL	
   Function:  03
*/

void RPC_Clone_XGobi (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  RPC_FIRST_LINE();

  if (!xgobi.xgobi_is_up)
  {
     sprintf (outstr, "03.01");
     RETURN;
  }

  Clone_XGobi ();

  sprintf (outstr, "03.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - NULL	
   Function:  04
*/

void RPC_Xfer_Colornames(instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  extern AppData appdata;
  
  RPC_FIRST_LINE();

  sprintf(outstr, "%d %d %d %s %s %s %s %s %s %s %s %s %s", 
    NGLYPHSIZES, NGLYPHTYPES, NCOLORS,
    appdata.brushColor0, appdata.brushColor1, appdata.brushColor2,
    appdata.brushColor3, appdata.brushColor4, appdata.brushColor5,
    appdata.brushColor6, appdata.brushColor7, appdata.brushColor8,
    appdata.brushColor9);

  RPC_LAST_LINE();
}



void free_old_pointers ()

{
  int i;


  is_init = has_data = has_symbols =
    is_cdf_init = has_cdf_data = has_cdf_symbols = has_cdf_bitmap = has_cdf_weights =
    cdf_default_bitmap = cdf_default_types =
    is_vario_init = has_vario_data = has_vario_symbols =
    is_vario2_init = has_vario2_data = has_vario2_symbols =
    is_lag_init = has_lag_data = has_lag_symbols = False;

  if (datap)
  {
    for (i = 0; i < nr; i++)
      XtFree ((XtPointer) datap[i]);
    XtFree ((XtPointer) datap);
    datap = NULL;
  }

  if (missingp)
  {
    for (i = 0; i < nr; i++)
      XtFree ((XtPointer) missingp[i]);
    XtFree ((XtPointer) missingp);
    missingp = NULL;
  }

  mv_missing_values_present = False;
  mv_is_missing_values_xgobi = False;
  mv_nmissing = 0;

  if (rowp)
  {
    for (i = 0; i < nr; i++)
      XtFree ((XtPointer) rowp[i]);
    XtFree ((XtPointer) rowp);
    rowp = NULL;
  }

  if (colp)
  {
    for (i = 0; i < nc + 1; i++)
      XtFree ((XtPointer) colp[i]);
    XtFree ((XtPointer) colp);
    colp = NULL;
  }

  if (size)
  {
    XtFree ((XtPointer) size);
    size = NULL;
  }

  if (color)
  {
    XtFree ((XtPointer) color);
    color = NULL;
  }

  if (glyph)
  {
    XtFree ((XtPointer) glyph);
    glyph = NULL;
  }

  if (erase)
  {
    XtFree ((XtPointer) erase);
    erase = NULL;
  }

  if (lastf)
  {
    XtFree ((XtPointer) lastf);
    lastf = NULL;
  }

  if (cdf_xvalues)
  {
    XtFree ((XtPointer) cdf_xvalues);
    cdf_xvalues = NULL;
  }

  if (cdf_xranks)
  {
    XtFree ((XtPointer) cdf_xranks);
    cdf_xranks = NULL;
  }

  if (cdf_inv_xranks)
  {
    XtFree ((XtPointer) cdf_inv_xranks);
    cdf_inv_xranks = NULL;
  }

  if (cdf_bitmap)
  {
    for (i = 0; i < NCOLORS; i++)
      if (cdf_bitmap[i])
        XtFree ((XtPointer) cdf_bitmap[i]);
    XtFree ((XtPointer) cdf_bitmap);
    cdf_bitmap = NULL;
  }

  if (cdf_brush_bitmap)
  {
    XtFree ((XtPointer) cdf_brush_bitmap);
    cdf_brush_bitmap = NULL;
  }

  if (cdf_onebits)
  {
    XtFree ((XtPointer) cdf_onebits);
    cdf_onebits = NULL;
  }

  if (cdf_weights)
  {
    XtFree ((XtPointer) cdf_weights);
    cdf_weights = NULL;
  }

  if (cdf_types)
  {
    for (i = 0; i < NCOLORS; i++)
      if (cdf_types[i])
        XtFree ((XtPointer) cdf_types[i]);
    XtFree ((XtPointer) cdf_types);
    cdf_types = NULL;
  }

  if (cdf_def_point_size)
  {
    XtFree ((XtPointer) cdf_def_point_size);
    cdf_def_point_size = NULL;
  }

  if (cdf_def_point_glyph)
  {
    XtFree ((XtPointer) cdf_def_point_glyph);
    cdf_def_point_glyph = NULL;
  }

  if (cdf_brush_point_size)
  {
    XtFree ((XtPointer) cdf_brush_point_size);
    cdf_brush_point_size = NULL;
  }

  if (cdf_brush_point_glyph)
  {
    XtFree ((XtPointer) cdf_brush_point_glyph);
    cdf_brush_point_glyph = NULL;
  }

  if (vario_values)
  {
    for (i = 0; i < vario_n; i++)
      XtFree ((XtPointer) vario_values[i]);
    XtFree ((XtPointer) vario_values);
    vario_values = NULL;
  }

  if (vario_missings)
  {
    for (i = 0; i < vario_n; i++)
      XtFree ((XtPointer) vario_missings[i]);
    XtFree ((XtPointer) vario_missings);
    vario_missings = NULL;
  }

  if (vario_np_e_p1)
  {
    XtFree ((XtPointer) vario_np_e_p1);
    vario_np_e_p1 = NULL;
  }

  if (vario_np_e_p2)
  {
    XtFree ((XtPointer) vario_np_e_p2);
    vario_np_e_p2 = NULL;
  }

  if (vario_w)
  {
    XtFree ((XtPointer) vario_w);
    vario_w = NULL;
  }

  if (vario_bin_map)
  {
    for (i = 0; i < vario_n; i++)
      XtFree ((XtPointer) vario_bin_map[i]);
    XtFree ((XtPointer) vario_bin_map);
    vario_bin_map = NULL;
  }

  if (vario_ptarr1)
  {
    for (i = 0; i < vario_n; i++)
      XtFree ((XtPointer) vario_ptarr1[i]);
    XtFree ((XtPointer) vario_ptarr1);
    vario_ptarr1 = NULL;
  }

  if (vario_ptarr2)
  {
    for (i = 0; i < vario_n; i++)
      XtFree ((XtPointer) vario_ptarr2[i]);
    XtFree ((XtPointer) vario_ptarr2);
    vario_ptarr2 = NULL;
  }

  vario_p = 0;
  vario_n = 0;
  vario_dist = 0.0;


  if (vario2_values)
  {
    for (i = 0; i < vario2_n; i++)
      XtFree ((XtPointer) vario2_values[i]);
    XtFree ((XtPointer) vario2_values);
    vario2_values = NULL;
  }

  if (vario2_values)
  {
    for (i = 0; i < vario2_n; i++)
      XtFree ((XtPointer) vario2_missings[i]);
    XtFree ((XtPointer) vario2_missings);
    vario2_missings = NULL;
  }

  if (vario2_np_e_p1)
  {
    XtFree ((XtPointer) vario2_np_e_p1);
    vario2_np_e_p1 = NULL;
  }

  if (vario2_np_e_p2)
  {
    XtFree ((XtPointer) vario2_np_e_p2);
    vario2_np_e_p2 = NULL;
  }

  if (vario2_w)
  {
    XtFree ((XtPointer) vario2_w);
    vario2_w = NULL;
  }

  if (vario2_bin_map)
  {
    for (i = 0; i < vario2_n; i++)
      XtFree ((XtPointer) vario2_bin_map[i]);
    XtFree ((XtPointer) vario2_bin_map);
    vario2_bin_map = NULL;
  }

  if (vario2_ptarr1)
  {
    for (i = 0; i < vario2_n; i++)
      XtFree ((XtPointer) vario2_ptarr1[i]);
    XtFree ((XtPointer) vario2_ptarr1);
    vario2_ptarr1 = NULL;
  }

  if (vario2_ptarr2)
  {
    for (i = 0; i < vario2_n; i++)
      XtFree ((XtPointer) vario2_ptarr2[i]);
    XtFree ((XtPointer) vario2_ptarr2);
    vario2_ptarr2 = NULL;
  }

  vario2_p = 0;
  vario2_n = 0;
  vario2_dist = 0.0;


 if (lag_values)
  {
    for (i = 0; i < lag_n; i++)
      XtFree ((XtPointer) lag_values[i]);
    XtFree ((XtPointer) lag_values);
    lag_values = NULL;
  }

 if (lag_missings)
  {
    for (i = 0; i < lag_n; i++)
      XtFree ((XtPointer) lag_missings[i]);
    XtFree ((XtPointer) lag_missings);
    lag_missings = NULL;
  }

  if (lag_np_e_p1)
  {
    XtFree ((XtPointer) lag_np_e_p1);
    lag_np_e_p1 = NULL;
  }

  if (lag_np_e_p2)
  {
    XtFree ((XtPointer) lag_np_e_p2);
    lag_np_e_p2 = NULL;
  }

  if (lag_w)
  {
    XtFree ((XtPointer) lag_w);
    lag_w = NULL;
  }

  if (lag_bin_map)
  {
    for (i = 0; i < lag_n; i++)
      XtFree ((XtPointer) lag_bin_map[i]);
    XtFree ((XtPointer) lag_bin_map);
    lag_bin_map = NULL;
  }

  if (lag_ptarr1)
  {
    for (i = 0; i < lag_n; i++)
      XtFree ((XtPointer) lag_ptarr1[i]);
    XtFree ((XtPointer) lag_ptarr1);
    lag_ptarr1 = NULL;
  }

  if (lag_ptarr2)
  {
    for (i = 0; i < lag_n; i++)
      XtFree ((XtPointer) lag_ptarr2[i]);
    XtFree ((XtPointer) lag_ptarr2);
    lag_ptarr2 = NULL;
  }

  lag_p = 0;
  lag_n = 0;
  lag_dist = 0.0;
}



int split_color_glyph_size (pointtype, lastf, size, glyph, erase, color)

int pointtype;
int *lastf, *size, *glyph, *erase, *color;

{
  int remainder;

  *lastf = pointtype;

  *size = pointtype / 1000 + 1;
  if (*size < 1 || *size > NGLYPHSIZES)
  {
    return (1);
  }

  remainder = pointtype % 1000;
  *glyph = remainder / 100 + 1;
  if (*glyph < 1 || *glyph > NGLYPHTYPES)
  {
    return (1);
  }

  remainder = remainder % 100;
  *erase = remainder / (NCOLORS + 1);
  if ((*erase != 0) && (*erase != 1))
  {
    return (1);
  }

  if (!mono)
  {
    *color = remainder % (NCOLORS + 1);
  }

  return (0);
}



int return_color_glyph_size (pointtype, color, glyph, size)

int pointtype, *color, *glyph, *size;

{
  int remainder;

  *size = pointtype / 1000 + 1;
  if (*size < 1 || *size > NGLYPHSIZES)
  {
    return (1);
  }

  remainder = pointtype % 1000;
  *glyph = remainder / 100 + 1;
  if (*glyph < 1 || *glyph > NGLYPHTYPES)
  {
    return (1);
  }

  remainder = remainder % 100;

  if (!mono)
  {
    *color = remainder % (NCOLORS + 1);
  }

  return (0);
}



/*
   Usage:     In - (name, ncols, nrows) : (String, int, int)
   Function:  11
*/

void RPC_Init_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;

  RPC_FIRST_LINE();

  free_old_pointers ();

  if (sscanf (instr, "%s %d %d\n", name, &nc, &nr) < 3)
  {
     sprintf (outstr, "11.01");
     RETURN;
  } 

  /* Allocate data space */
  datap = (float **) XtMalloc ((Cardinal) nr * sizeof (float *));
  for (i = 0; i < nr; i++)
    datap[i] = (float *) XtMalloc ((Cardinal) (nc + 1) * sizeof (float));

  /* Allocate missing space */
  missingp = (short **) XtMalloc ((Cardinal) nr * sizeof (short *));
  for (i = 0; i < nr; i++)
    missingp[i] = (short *) XtMalloc ((Cardinal) (nc + 1) * sizeof (short));

  /* Allocate row space */
  rowp = (char **) XtMalloc ((Cardinal) nr * sizeof (char *));
  for (i = 0; i < nr; i++)
    rowp[i] = (char *) XtMalloc ((Cardinal) ROWLABLEN * sizeof (char));

  /* Create default row labels */
  for (i = 0; i < nr; i++)
    (void) sprintf (rowp[i], "%d", i + 1);

  /* Allocate column space */
  colp = (char **) XtMalloc ((Cardinal) (nc + 1) * sizeof (char *));
  for (i = 0; i < nc + 1; i++)
    colp[i] = (char *) XtMalloc ((Cardinal) COLLABLEN * sizeof (char));

  /* Create default column labels */
  for (i = 0; i < nc + 1; i++)
    (void) sprintf (colp[i], "%d", i + 1);

  /* Allocate space for size, color, glyph, erase, and lastf */
  size = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  color = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  glyph = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  erase = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  lastf = (int *) XtMalloc ((Cardinal) nr * sizeof (int));

  is_init = True;
  sprintf (outstr, "11.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - datap[nrows, ncols] : (nrows * ncols) x float
   Function:  12
*/

void RPC_Send_Init_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int datcount = 0,
      i = 0, j = 0;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!is_init || has_data)
  {
     sprintf (outstr, "12.01");
     RETURN;
  } 

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "12.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! strcmp (tokptr, "NA") || ! strcmp (tokptr, "na"))
  {
    missingp[i][j] = 1;
    mv_missing_values_present = true;
    mv_nmissing++;
    datap[i][j] = 0.0;
  }
  else 
    if (! sscanf (tokptr, "%f", &datap[i][j]))
    {
       sprintf (outstr, "12.02");
       RETURN;
    } 
    else
       missingp[i][j] = 0;

  datcount++;

  while (((tokptr = strtok (NULL, " ")) != NULL) && (datcount < nc * nr))
  {
    i = datcount / nc;   /* row */
    j = datcount % nc;   /* column */

    if (! strcmp (tokptr, "NA") || ! strcmp (tokptr, "na"))
    {
      missingp[i][j] = 1;
      mv_missing_values_present = true;
      mv_nmissing++;
      datap[i][j] = 0.0;
    }
    else
      {
        sscanf (tokptr, "%f", &datap[i][j]);
        missingp[i][j] = 0;
      }
    datcount++;
  }

  if (datcount != nc * nr)
  {
     sprintf (outstr, "12.02");
     RETURN;
  }

  has_data = True;
  sprintf (outstr, "12.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - size_color_glyph[nrows] : nrows x int	
   Function:  13
*/

void RPC_Send_Init_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0;
  int pointtype;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!has_data || has_symbols)
  {
     sprintf (outstr, "13.01");
     RETURN;
  } 

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "13.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%d", &pointtype))
  {
    sprintf (outstr, "13.02");
    RETURN;
  }

  if (split_color_glyph_size (pointtype, &lastf[i], &size[i], &glyph[i], &erase[i], &color[i]))
  {
    sprintf (outstr, "13.03");
    RETURN;
  }
  i++;

  while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr))
  {
    sscanf (tokptr, "%d", &pointtype);
    
    if (split_color_glyph_size (pointtype, &lastf[i], &size[i], &glyph[i], &erase[i], &color[i]))
    {
      sprintf (outstr, "13.03");
      RETURN;
    }
    i++;
  }

  if (i != nr)
  {
    sprintf (outstr, "13.02");
    RETURN;
  }

  has_symbols = True;
  sprintf (outstr, "13.00");

  RPC_LAST_LINE();
}



void reinit_2 (xg)
  xgobidata *xg;
{
  int i, j, k;

  xg->nrows = nr;
  xg->nlinkable = xg->nrows;
  xg->ncols_used = nc;
  xg->ncols = nc + 1;

  xg->raw_data = (float **) XtMalloc ((Cardinal) xg->nrows * sizeof (float *));
  for (k = 0; k < xg->nrows; k++)
    xg->raw_data[k] = (float *) XtMalloc ((Cardinal) xg->ncols * sizeof (float));

  for (i = 0; i < xg->nrows; i++) /* copy data from datap */
    for (j = 0; j < xg->ncols_used; j++)
      xg->raw_data[i][j] = datap[i][j];

  xg->is_missing = (short **) XtMalloc ((Cardinal) xg->nrows * sizeof (short *));
  for (i = 0; i < xg->nrows; i++)
    xg->is_missing[i] = (short *) XtMalloc ((Cardinal) xg->ncols * sizeof (short));

  for (i = 0; i < xg->nrows; i++) /* copy missings from missingp */
    for (j = 0; j < xg->ncols_used; j++)
      xg->is_missing[i][j] = missingp[i][j];

  xg->missing_values_present = mv_missing_values_present;
  xg->is_missing_values_xgobi = mv_is_missing_values_xgobi;
  xg->nmissing = mv_nmissing;

/*
  (void) read_array(filename, xg);
*/
  fill_extra_column(xg);
  (void) read_extra_resources("stdin");

  (void) read_collabels(NULL, True, xg);
  for (i = 0; i< xg->ncols; i++)
  {
    strcpy (xg->collab[i], colp[i]);
    strcpy (xg->collab_tform1[i], xg->collab[i]);
    strcpy (xg->collab_tform2[i], xg->collab[i]);
  }

  (void) read_rowlabels(NULL, True, xg);
  for (i = 0; i< xg->nrows; i++)
    strcpy (xg->rowlab[i], rowp[i]);
}



/*
   Usage:     In - NULL	
   Function:  14
*/

void RPC_Make_XGobi (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;

  RPC_FIRST_LINE();

  if (strcmp (instr, "LINKED") == 0)
    /* Data originates from XGobi itself !! */
  {  
     has_symbols = True;
     nr = xgobi.nrows_in_plot,
     sprintf (outstr, "14.00");
     RETURN;
  }

  if (!has_symbols)
  {
    sprintf (outstr, "14.01");
    RETURN;
  }

  if (xgobi.xgobi_is_up)
  {
    int ncols_prev = xgobi.ncols;
    int ncols_used_prev = xgobi.ncols_used;

    DTEXT("Reinit 1... ");
    reinit_1 (&xgobi, ncols_prev, ncols_used_prev);
    DTEXT("Reinit 2... ");
    reinit_2 (&xgobi);
    DTEXT("Reinit 3... ");
    reinit_3 (&xgobi, NULL, name, ncols_prev, ncols_used_prev);
    DTEXT("Ok ");
  }
  else
    if (!make_xgobi (datapflag, NULL, datap, name,
          mv_missing_values_present, missingp,
          mv_is_missing_values_xgobi, mv_nmissing,
          nr, rowp, nc + 1, colp,
          0, (connect_lines *) NULL, &xgobi, prog_shell, True))
    {
       sprintf (outstr, "14.02");
       RETURN;
    }

  for (i = 0; i < nr; i++)
  {
    xgobi.color_ids[i] = xgobi.color_now[i] =
      xgobi.color_prev[i] = color_nums[color[i]];
    xgobi.glyph_ids[i].type = xgobi.glyph_now[i].type = 
      xgobi.glyph_prev[i].type = glyph[i];
    xgobi.glyph_ids[i].size = xgobi.glyph_now[i].size = 
      xgobi.glyph_prev[i].size = size[i];
    xgobi.erased[i] = erase[i];
    xgobi.last_forward[i] = lastf[i];
  }

  xgobi.nlinkable = xgobi.nrows; /* allow linked brushing for all points */
  xgobi.xy_vars.x = 0; /* do not swap x & y axis */
  xgobi.xy_vars.y = 1;
  xgobi.connect_the_points = False; /* do not connect the points */
  set_showlines (xgobi.connect_the_points);
  xgobi.clone_Type = ArcData;
  if (xgobi.xplore_flag)
    active_xplore_buttons();

  /* enable options allowed for data mode */
  reset_Exit_cmd (&xgobi);
  set_Edit_Lines_cmd (&xgobi, True);
  set_brush_menu_cmd (True);
  /*set_erase_menu_cmd (&xgobi, True);*/  /* dfs erase */
  reset_io_read_menu_cmd (&xgobi);
  reset_io_line_read_menu_cmd (&xgobi);

  is_running = True;
  update_required = True;
  xgobi.xgobi_is_up = True;
  sprintf (outstr, "14.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - size_color_glyph[nrows] : nrows x int
   Function:  15
*/

void RPC_Update_All_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0;
  int pointtype;
  char *tokptr;
  Boolean prev_is_color_painting;
  enum {persistent, transient, undo} brush_mode_old;

  RPC_FIRST_LINE();

  if (!has_symbols || !xgobi.xgobi_is_up)
  {
    sprintf (outstr, "15.01");
    RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "15.02");
     RETURN;
  } 

  tokptr = strtok (instr, " ");
  last_server_number = 0L;
  if (!sscanf (tokptr, "%x", &last_server_number))
  {
    sprintf (outstr, "15.02");
    RETURN;
  }

  while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr))
  {
    sscanf (tokptr, "%d", &pointtype);

    if (split_color_glyph_size (pointtype, &lastf[i], &size[i], &glyph[i], &erase[i], &color[i]))
    {
      sprintf (outstr, "15.03");
      RETURN;
    }

    i++;
  }

  if (i != nr)
  {
    sprintf (outstr, "15.02");
    RETURN;
  }

  for (i = 0; i < nr; i++)
  {
    xgobi.color_ids[i] = xgobi.color_now[i] =
      xgobi.color_prev[i] = color_nums[color[i]];
    xgobi.glyph_ids[i].type = xgobi.glyph_now[i].type = 
      xgobi.glyph_prev[i].type = glyph[i];
    xgobi.glyph_ids[i].size = xgobi.glyph_now[i].size = 
      xgobi.glyph_prev[i].size = size[i];
    xgobi.erased[i] = erase[i];
    /* xgobi.last_forward[i] = lastf[i]; */
    /* do not set here - data might be forwarded in xfer_brus_info */
  }

  /* Plot */
  prev_is_color_painting = xgobi.is_color_painting;
  xgobi.is_color_painting = True && (!mono);
  plot_once (&xgobi);
  xgobi.is_color_painting = prev_is_color_painting;

  /* forward brushing information to other XGobis - enforce persistent brushing */
  brush_mode_old = xgobi.brush_mode;
  xgobi.brush_mode = persistent;
  copy_brushinfo_to_senddata (&xgobi);
  br_update_cback (NULL, &xgobi, NULL);
  xgobi.brush_mode = brush_mode_old;

  update_required = False;
  sprintf (outstr, "15.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - list of index:size_color_glyph : list of (int:int)
   Function:  16
*/

void RPC_Update_Some_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;
  int pointtype;
  int lastf, size, glyph, erase, color;
  char *tokptr;
  Boolean prev_is_color_painting;
  enum brush_mode_type brush_mode_old;

  RPC_FIRST_LINE();

  if (!has_symbols || !xgobi.xgobi_is_up)
  {
    sprintf (outstr, "16.01");
    RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "16.02");
     RETURN;
  } 

  tokptr = strtok (instr, " ");
  last_server_number = 0L;
  if (!sscanf (tokptr, "%x", &last_server_number))
  {
    sprintf (outstr, "16.02");
    RETURN;
  }

  while ((tokptr = strtok (NULL, " ")) != NULL)
  {
    if (sscanf (tokptr, "%d:%d", &i, &pointtype) < 2)
    {
      sprintf (outstr, "16.02");
      RETURN;
    }

    if (i < 0 || i >= nr )
    {
      sprintf (outstr, "16.03");
      RETURN;
    }

    if (split_color_glyph_size (pointtype, &lastf, &size, &glyph, &erase, &color))
    {
      sprintf (outstr, "16.04");
      RETURN;
    }

    xgobi.color_ids[i] = xgobi.color_now[i] =
      xgobi.color_prev[i] = color_nums[color];
    xgobi.glyph_ids[i].type = xgobi.glyph_now[i].type = 
      xgobi.glyph_prev[i].type = glyph;
    xgobi.glyph_ids[i].size = xgobi.glyph_now[i].size = 
      xgobi.glyph_prev[i].size = size;
    xgobi.erased[i] = erase;
    /* xgobi.last_forward[i] = lastf; */
    /* do not set here - data might be forwarded in xfer_brus_info */
  }

  /* Plot */
  prev_is_color_painting = xgobi.is_color_painting;
  xgobi.is_color_painting = True && (!mono);
  plot_once (&xgobi);
  xgobi.is_color_painting = prev_is_color_painting;

  /* forward brushing information to other XGobis - enforce persistent brushing */
  brush_mode_old = xgobi.brush_mode;
  xgobi.brush_mode = persistent;
  copy_brushinfo_to_senddata (&xgobi);
  br_update_cback (NULL, &xgobi, NULL);
  xgobi.brush_mode = brush_mode_old;

  update_required = False;
  sprintf (outstr, "16.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - (name, ncols, nrows) : (String, int, int)
   Function:  21
*/

void RPC_Init_CDF_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{

  int i;

  RPC_FIRST_LINE();

  free_old_pointers ();

  if (sscanf (instr, "%s %d %d\n", name, &nc, &nr) < 3)
  {
     sprintf (outstr, "21.01");
     RETURN;
  }

  /* Allocate data space */
  datap = (float **) XtMalloc ((Cardinal) nr * sizeof (float *));
  for (i = 0; i < nr; i++)
    datap[i] = (float *) XtMalloc ((Cardinal) (nc + 1) * sizeof (float));

  /* Allocate missing space */
  missingp = (short **) XtMalloc ((Cardinal) nr * sizeof (short *));
  for (i = 0; i < nr; i++)
    missingp[i] = (short *) XtMalloc ((Cardinal) (nc + 1) * sizeof (short));

  /* Allocate row space */
  rowp = (char **) XtMalloc ((Cardinal) nr * sizeof (char *));
  for (i = 0; i < nr; i++)
    rowp[i] = (char *) XtMalloc ((Cardinal) ROWLABLEN * sizeof (char));

  /* Create default row labels */
  for (i = 0; i < nr; i++)
    (void) sprintf (rowp[i], "%d", i + 1);

  /* Allocate column space */
  colp = (char **) XtMalloc ((Cardinal) (nc + 1) * sizeof (char *));
  for (i = 0; i < nc + 1; i++)
    colp[i] = (char *) XtMalloc ((Cardinal) COLLABLEN * sizeof (char));

  /* Create default column labels */
  for (i = 0; i < nc + 1; i++)
    (void) sprintf (colp[i], "%d", i + 1);

  /* Allocate space for size, color, glyph, erase, lastf, xvalues, xranks, inv_xranks, brush_bitmap, and weights */
  size = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  color = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  glyph = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  erase = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  lastf = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  cdf_xvalues = (float *) XtMalloc ((Cardinal) nr * sizeof (float));
  cdf_xranks = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  cdf_inv_xranks = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  cdf_brush_bitmap = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  cdf_weights = (double *) XtMalloc ((Cardinal) nr * sizeof (double));

  /* Allocate space for bitmap, one_bits, and types */
  cdf_bitmap = (int **) XtMalloc ((Cardinal) NCOLORS * sizeof (int *));
  cdf_onebits = (int *) XtMalloc ((Cardinal) NCOLORS * sizeof (int));
  cdf_types = (int **) XtMalloc ((Cardinal) NCOLORS * sizeof (int *));

  /* Allocate space for default/brush size and glyph */
  cdf_def_point_size = (int *) XtMalloc ((Cardinal) NCDFTYPES * sizeof (int));
  cdf_def_point_glyph = (int *) XtMalloc ((Cardinal) NCDFTYPES * sizeof (int));
  cdf_brush_point_size = (int *) XtMalloc ((Cardinal) NCDFTYPES * sizeof (int));
  cdf_brush_point_glyph = (int *) XtMalloc ((Cardinal) NCDFTYPES * sizeof (int));

  /* Create default values */
  cdf_def_point_color = 0;
  for (i = 0; i < NCOLORS; i++)
  {
    cdf_bitmap[i] = NULL;
    cdf_onebits[i] = 0;
    cdf_types[i] = NULL;
  }

  is_cdf_init = True;
  sprintf (outstr, "21.00");

  RPC_LAST_LINE();
}



int float_compare (elm1, elm2)

float *elm1, *elm2;

{
  if (*elm1 < *elm2) 
    return (-1);
  else if (*elm1 == *elm2)
    return (0);
  else 
    return (1);
}



/*
   Usage:     In - datap[nrows, ncols - 1] : (nrows * (ncols - 1)) x float
   Function:  22
*/

void RPC_Send_Init_CDF_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int datcount = 0,
      i = 0, j = 0;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!is_cdf_init || has_cdf_data)
  {
     sprintf (outstr, "22.01");
     RETURN;
  }

  if (nc == 2)
  {
    if ((instr == NULL) || !strcmp (instr, ""))
    {
       sprintf (outstr, "22.02");
       RETURN;
    } 

    tokptr = strtok (instr," ");
    if (! sscanf (tokptr, "%f", &cdf_xvalues[i++]))
    {
       sprintf (outstr, "22.02");
       RETURN;
    } 

    while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr))
      sscanf (tokptr, "%f", &cdf_xvalues[i++]);

    if (i != nr)
    {
       sprintf (outstr, "22.02");
       RETURN;
    }

    for (i = 0; i < nr; i++)   /* determine xranks */
    {
      cdf_xranks[i] = 0;

      for (j = 0; j < nr; j++)
        if ((cdf_xvalues[i] > cdf_xvalues[j]) || 
            ((cdf_xvalues[i] == cdf_xvalues[j]) && (i > j)))
          cdf_xranks[i]++;
    }

    for (i = 0; i < nr; i++)   /* determine inv_xranks */
      cdf_inv_xranks[cdf_xranks[i]] = i;

    qsort ((char *) cdf_xvalues, nr, sizeof (float), float_compare);  /* sort data */

    for (i = 0; i < nr; i++)
      datap[i][1] = cdf_xvalues[i];
  }
  else 
    {
      i = 0;
      j = 1;

      if ((instr == NULL) || !strcmp (instr, ""))
      {
         sprintf (outstr, "22.03");
         RETURN;
      } 

      tokptr = strtok (instr," ");
      if (! sscanf (tokptr, "%f", &datap[i][j]))
      {
         sprintf (outstr, "22.03");
         RETURN;
      } 
      datcount++;

      while (((tokptr = strtok (NULL, " ")) != NULL) && (datcount < (nc - 1) * nr))
      {
        i = datcount / (nc - 1);       /* row */
        j = datcount % (nc - 1) + 1;   /* column */
        sscanf (tokptr, "%f", &datap[i][j]);
        datcount++;
      }

      if (datcount != (nc - 1) * nr)
      {
         sprintf (outstr, "22.03");
         RETURN;
      }
    }
  
  has_cdf_data = True;
  sprintf (outstr, "22.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - size_color_glyph[NCDFTYPES] : NCDFTYPES x int	
   Function:  23
*/

void RPC_Send_Init_CDF_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0;
  int pointtype, dummy;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!has_cdf_data || has_cdf_symbols)
  {
     sprintf (outstr, "23.01");
     RETURN;
  } 

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "23.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%d", &pointtype))
  {
    sprintf (outstr, "23.02");
    RETURN;
  }

  if (return_color_glyph_size (pointtype, &cdf_def_point_color, &cdf_def_point_glyph[0], &cdf_def_point_size[0]))
    {
      sprintf (outstr, "23.03");
      RETURN;
    }
  i++;

  while (((tokptr = strtok (NULL, " ")) != NULL) && (i < NCDFTYPES))
  {
    sscanf (tokptr, "%d", &pointtype);

    if (return_color_glyph_size (pointtype, &dummy, &cdf_def_point_glyph[i], &cdf_def_point_size[i]))
      {
        sprintf (outstr, "23.03");
        RETURN;
      }
    i++;
  }

  if (i != NCDFTYPES)
  {
     sprintf (outstr, "23.02");
     RETURN;
  }

  has_cdf_symbols = True;
  sprintf (outstr, "23.00");

  RPC_LAST_LINE();
}



void reinit_2_cdfs (xg, numcdfs)

xgobidata *xg;
int numcdfs;

{
  int i, j, k;

  int ncols_prev = xg->ncols;
  int ncols_used_prev = xg->ncols_used;

  reinit_1 (&xgobi, ncols_prev, ncols_used_prev);
  
  xg->nrows = numcdfs * nr;
  xg->nlinkable = xg->nrows;
  xg->ncols_used = nc;
  xg->ncols = nc + 1;

  xg->raw_data = (float **) XtMalloc ((Cardinal) xg->nrows * sizeof (float *));
  for (k = 0; k < xg->nrows; k++)
    xg->raw_data[k] = (float *) XtMalloc ((Cardinal) xg->ncols * sizeof (float));

  for (i = 0; i < numcdfs; i++) /* copy data from datap */
    for (j = 0; j < nr; j++)
    {
      xg->raw_data[i * nr + j][0] = 0;
      for (k = 1; k < xg->ncols_used; k++)
        xg->raw_data[i * nr + j][k] = datap[j][k];
    }

  k = 0;
  for (i = 0; i < NCOLORS; i++) /* calculate cdfs */
    if (cdf_bitmap[i] && cdf_types[i])
    {
      if (cdf_types[i][0])
      {
        ecdf (xg->raw_data + k * nr, cdf_bitmap[i], nr, nc - 1, &cdf_onebits[i]);
        k++;
      }
      if (cdf_types[i][1])
      {
        if (has_cdf_weights)
        {
          weighted_ecdf (xg->raw_data + k * nr, cdf_bitmap[i], nr, nc - 1, 
            &cdf_onebits[i], cdf_weights);
        }
        else
          {
            show_message ("No weight vector available for weighted CDF.\n", xg->shell);
            ecdf (xg->raw_data + k * nr, cdf_bitmap[i], nr, nc - 1, &cdf_onebits[i]);
          }
        k++;
      }
    }

/*
  (void) read_array(filename, xg);
*/
  fill_extra_column(xg);
  (void) read_extra_resources("stdin");

  (void) read_collabels(NULL, True, xg);
  for (i = 0; i< xg->ncols; i++)
  {
    strcpy (xg->collab[i], colp[i]);
    strcpy (xg->collab_tform1[i], xg->collab[i]);
    strcpy (xg->collab_tform2[i], xg->collab[i]);
  }

  (void) read_rowlabels(NULL, True, xg);
  for (i = 0; i < numcdfs; i++)
    for (j = 0; j < nr; j++)
      strcpy (xg->rowlab[i * nr + j], rowp[j]);

  reinit_3 (&xgobi, NULL, name, ncols_prev, ncols_used_prev);
}



void redraw_cdfs()

{
  int i, j, k, l, m, n;
  int numcdfs;


  numcdfs = 0;
  for (i = 0; i < NCOLORS; i++)
    if (cdf_bitmap[i] && cdf_types[i])
      for (j = 0; j < NCDFTYPES; j++)
        if (cdf_types[i][j])
          numcdfs++;

  if (numcdfs == 0)
  {
    /* Create a default cdf */
    numcdfs = 1;
    cdf_default_bitmap = cdf_default_types = True;

    cdf_bitmap[cdf_def_point_color] = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
    for (i = 0; i < nr; i++)
      cdf_bitmap[cdf_def_point_color][i] = 1;

    cdf_onebits[cdf_def_point_color] = nr;

    cdf_types[cdf_def_point_color] = (int *) XtMalloc ((Cardinal) NCDFTYPES * sizeof (int));
    cdf_types[cdf_def_point_color][0] = 1;
    cdf_types[cdf_def_point_color][1] = 0;
  }

  reinit_2_cdfs (&xgobi, numcdfs); /* reinitialize Xgobi for cdfs */

  k = 0;
  for (i = 0; i < NCOLORS; i++)
    if (cdf_bitmap[i] && cdf_types[i])
      for (l = 0; l < NCDFTYPES; l++)
        if (cdf_types[i][l])
          for (j = 0; j < nr; j++)
            if (cdf_bitmap[i][j])
              /* assign color/glyph/size or erase value */
              {
                xgobi.color_ids[k] = xgobi.color_now[k] =
                  xgobi.color_prev[k] = color_nums[i];
                xgobi.glyph_ids[k].type = xgobi.glyph_now[k].type = 
                  xgobi.glyph_prev[k].type = cdf_def_point_glyph[l]; 
                xgobi.glyph_ids[k].size = xgobi.glyph_now[k].size = 
                  xgobi.glyph_prev[k].size = cdf_def_point_size[l];
                xgobi.erased[k] = 0;
                xgobi.last_forward[k] = glyph_color_pointtype (&xgobi, k);
                k++;
              }
            else
              {
                xgobi.color_ids[k] = xgobi.color_now[k] =
                  xgobi.color_prev[k] = color_nums[i];
                xgobi.glyph_ids[k].type = xgobi.glyph_now[k].type = 
                  xgobi.glyph_prev[k].type = cdf_def_point_glyph[l];
                xgobi.glyph_ids[k].size = xgobi.glyph_now[k].size = 
                  xgobi.glyph_prev[k].size = 1; /* size 1 as a default */
                xgobi.erased[k] = 1;
                xgobi.last_forward[k] = glyph_color_pointtype (&xgobi, k);
                k++;
              }

  if (nc == 2)
  {
    /* number of possible lines is number of all (1-bits minus one) */
    xgobi.nlines = 0;
    for (i = 0; i < NCOLORS; i++)
      if (cdf_bitmap[i] && cdf_types[i])
        for (l = 0; l < NCDFTYPES; l++)
          if (cdf_types[i][l])
            if (cdf_onebits[i] >= 2)
              xgobi.nlines += (cdf_onebits[i] - 1);

    realloc_lines (&xgobi); /* reallocate memory for lines */

    k = 0;
    m = 0;
    for (l = 0; l < NCOLORS; l++)
      if (cdf_bitmap[l] && cdf_types[l])
        for (n = 0; n < NCDFTYPES; n++)
          if (cdf_types[l][n])
          {
            for (i = 0; i < nr; i++) 
              if (cdf_bitmap[l][i])
                for (j = i + 1; j < nr; j++)
                  if (cdf_bitmap[l][j])
                  {
                    /* initialize lines */
                    xgobi.connecting_lines[k].a = m * nr + i + 1;
                    xgobi.connecting_lines[k].b = m * nr + j + 1;
                    /* assign line color */
                    if (mono)
                      xgobi.line_color_ids[k] = xgobi.line_color_now[k] =
                        xgobi.line_color_prev[k] = color_nums[0];
                    else
                      xgobi.line_color_ids[k] = xgobi.line_color_now[k] =
                        xgobi.line_color_prev[k] = color_nums[l];

                    xgobi.xed_by_new_brush[k] = 0;
                    k++;
                    j = nr;
                  }
            m++;
          }

    xgobi.connect_the_points = True;
    set_showlines (xgobi.connect_the_points);
  }
  else
    {
      xgobi.nlines = 0;
      realloc_lines (&xgobi); /* reallocate empty memory for lines */
      xgobi.connect_the_points = False; /* do not connect the points */
      set_showlines (xgobi.connect_the_points);
    }
}



/*
   Usage:     In - NULL	
   Function:  24
*/

void RPC_Make_CDF_XGobi (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;

  RPC_FIRST_LINE();

  if (!has_cdf_symbols)
  {
    sprintf (outstr, "24.01");
    RETURN;
  }

  for (i = 0; i < nr; i++) /* assign dummy values */
      datap[i][0] = 0.5;

  datap[0][0] = 0.0;
  datap[nr - 1][0] = 1.0;

  if (!xgobi.xgobi_is_up)
  {
    if (!make_xgobi (datapflag, NULL, datap, name,
          mv_missing_values_present, missingp,
          mv_is_missing_values_xgobi, mv_nmissing,
          nr, rowp, nc + 1, colp,
  	  0, (connect_lines *) NULL, &xgobi, prog_shell, True))
    {
       sprintf (outstr, "24.02");
       RETURN;
    }
  }

  xgobi.nlinkable = xgobi.nrows; /* allow linked brushing for all points */
  if (nc == 2)
    xgobi.clone_Type = CDF1;
  else
    if (nc > 2)
      xgobi.clone_Type = CDFm;
  if (xgobi.xplore_flag)
    active_xplore_buttons();

  redraw_cdfs();

  /* disable options not required for cdf mode */
  reset_Exit_cmd (&xgobi);
  set_Edit_Lines_cmd (&xgobi, False);
  set_brush_menu_cmd (False);
  /*set_erase_menu_cmd (&xgobi, False);*/  /* dfs erase */
  reset_io_read_menu_cmd (&xgobi);
  reset_io_line_read_menu_cmd (&xgobi);

  is_running = True;
  update_required = True;
  xgobi.xgobi_is_up = True;
  sprintf (outstr, "24.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - (pointcolor, cdf_bitmap[pointcolor][nrows]) : (int, nrows x {0,1}) 
   Function:  25
*/

void RPC_Add_CDF_Bitmap (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0;
  int pointcolor;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!has_cdf_symbols || !xgobi.xgobi_is_up)
  {
     sprintf (outstr, "25.01");
     RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "25.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%d", &pointcolor))
  {
     sprintf (outstr, "25.02");
     RETURN;
  }

  if (pointcolor < 0 || pointcolor >= NCOLORS)
  {
     sprintf (outstr, "25.03");
     RETURN;
  }

  if (cdf_default_bitmap && cdf_bitmap[cdf_def_point_color])
  {
    /* Delete bitmap for default cdf */
    XtFree ((XtPointer) cdf_bitmap[cdf_def_point_color]);
    cdf_bitmap[cdf_def_point_color] = NULL;
    cdf_onebits[cdf_def_point_color] = 0;
    cdf_default_bitmap = False;
  }

  if (!cdf_bitmap[pointcolor])
    cdf_bitmap[pointcolor] = (int *) XtMalloc ((Cardinal) nr * sizeof (int));

  if (nc == 2)
    while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr))
      sscanf (tokptr, "%d", &cdf_bitmap[pointcolor][cdf_xranks[i++]]);
  else
    while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr))
      sscanf (tokptr, "%d", &cdf_bitmap[pointcolor][i++]);

  if (i != nr)
  {
     sprintf (outstr, "25.02");
     RETURN;
  }

  has_cdf_bitmap = True;
  sprintf (outstr, "25.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - (pointcolor, cdftype) : (int, {0,1,2})
   Function:  26
*/

void RPC_Add_CDF_Type (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int pointcolor, cdftype;

  RPC_FIRST_LINE();

  if (!has_cdf_bitmap || !xgobi.xgobi_is_up)
  {
     sprintf (outstr, "26.01");
     RETURN;
  }

  if (sscanf (instr, "%d %d\n", &pointcolor, &cdftype) < 2)
  {
     sprintf (outstr, "26.02");
     RETURN;
  }

  if (pointcolor < 0 || pointcolor >= NCOLORS)
  {
     sprintf (outstr, "26.03");
     RETURN;
  }

  if (!cdf_bitmap[pointcolor])
  {
     sprintf (outstr, "26.04");
     RETURN;
  }

  if (cdftype < 0 || cdftype >= 3)
  {
     sprintf (outstr, "26.05");
     RETURN;
  }

  if (cdf_default_types && cdf_types[cdf_def_point_color])
  {
    /* Delete types for default cdf */
    XtFree ((XtPointer) cdf_types[cdf_def_point_color]);
    cdf_types[cdf_def_point_color] = NULL;
    cdf_default_types = False;
  }

  if (!cdf_types[pointcolor])
    cdf_types[pointcolor] = (int *) XtMalloc ((Cardinal) NCDFTYPES * sizeof (int));

  switch (cdftype)
  {
    case 0: cdf_types[pointcolor][0] = 1;
            cdf_types[pointcolor][1] = 0;
            break;
    case 1: cdf_types[pointcolor][0] = 0;
            cdf_types[pointcolor][1] = 1;
            break;
    case 2: cdf_types[pointcolor][0] = 1;
            cdf_types[pointcolor][1] = 1;
            break;
  }

  redraw_cdfs();

  is_running = True;
  update_required = True;
  sprintf (outstr, "26.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - pointcolor : int
   Function:  27
*/

void RPC_Delete_CDF_Bitmap_and_Type (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int pointcolor;

  RPC_FIRST_LINE();

  if (!has_cdf_bitmap || !xgobi.xgobi_is_up)
  {
     sprintf (outstr, "27.01");
     RETURN;
  }

  if (!sscanf (instr, "%d\n", &pointcolor))
  {
     sprintf (outstr, "27.02");
     RETURN;
  }

  if (pointcolor < 0 || pointcolor >= NCOLORS)
  {
     sprintf (outstr, "27.03");
     RETURN;
  }

  if (cdf_bitmap[pointcolor])
  {
    XtFree ((XtPointer) cdf_bitmap[pointcolor]);
    cdf_bitmap[pointcolor] = NULL;
    cdf_onebits[pointcolor] = 0;
  }

  if (cdf_types[pointcolor])
  {
    XtFree ((XtPointer) cdf_types[pointcolor]);
    cdf_types[pointcolor] = NULL;
  }

  redraw_cdfs();

  is_running = True;
  update_required = True;
  sprintf (outstr, "27.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - cdf_brush_bitmap[nrows] : nrows x {0,1}
   Function:  28
*/

void RPC_Brush_All_CDF_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0, j, k, l;
  char *tokptr;
  int pointtype, dummy;
  Boolean prev_is_color_painting;
  enum brush_mode_type brush_mode_old;

  RPC_FIRST_LINE();

  if (!has_cdf_symbols || !xgobi.xgobi_is_up)
  {
     sprintf (outstr, "28.01");
     RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "28.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%d", &pointtype))
  {
    sprintf (outstr, "28.02");
    RETURN;
  }

  if (return_color_glyph_size (pointtype, &dummy, &cdf_brush_point_glyph[0], &cdf_brush_point_size[0]))
    {
      sprintf (outstr, "28.03");
      RETURN;
    }
  i++;

  while (i < NCDFTYPES)
    if ((tokptr = strtok (NULL, " ")) != NULL)
      {
        sscanf (tokptr, "%d", &pointtype);

        if (return_color_glyph_size (pointtype, &dummy, &cdf_brush_point_glyph[i], &cdf_brush_point_size[i]))
          {
            sprintf (outstr, "28.03");
            RETURN;
          }
        i++;
      }
    else
      {
        sprintf (outstr, "28.02");
        RETURN;
      }

  if (nc == 2)
  {
    while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr + NCDFTYPES))
      sscanf (tokptr, "%d", &cdf_brush_bitmap[cdf_xranks[(i++) - NCDFTYPES]]);
  }
  else
    {
      while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr + NCDFTYPES))
        sscanf (tokptr, "%d", &cdf_brush_bitmap[(i++) - NCDFTYPES]);
    }

  if (i != nr + NCDFTYPES)
  {
     sprintf (outstr, "28.02");
     RETURN;
  }

  k = 0;
  for (i = 0; i < NCOLORS; i++)
    if (cdf_bitmap[i] && cdf_types[i])
      for (l = 0; l < NCDFTYPES; l++)
        if (cdf_types[i][l])
          for (j = 0; j < nr; j++)
            if (cdf_brush_bitmap[j])
              /* assign size for brushed points */
              {
                xgobi.color_ids[k] = xgobi.color_now[k] =
                  xgobi.color_prev[k] = color_nums[i];
                xgobi.glyph_ids[k].type = xgobi.glyph_now[k].type = 
                  xgobi.glyph_prev[k].type = cdf_brush_point_glyph[l];
                if (xgobi.erased[k])
                  xgobi.glyph_ids[k].size = xgobi.glyph_now[k].size = 
                    xgobi.glyph_prev[k].size = 1;
                else
                  xgobi.glyph_ids[k].size = xgobi.glyph_now[k].size = 
                    xgobi.glyph_prev[k].size = cdf_brush_point_size[l];
                xgobi.last_forward[k] = glyph_color_pointtype (&xgobi, k);
                k++;
              }
            else
              {
                xgobi.color_ids[k] = xgobi.color_now[k] =
                  xgobi.color_prev[k] = color_nums[i];
                xgobi.glyph_ids[k].type = xgobi.glyph_now[k].type = 
                  xgobi.glyph_prev[k].type = cdf_def_point_glyph[l];
                if (xgobi.erased[k])
                  xgobi.glyph_ids[k].size = xgobi.glyph_now[k].size = 
                    xgobi.glyph_prev[k].size = 1;
                else
                  xgobi.glyph_ids[k].size = xgobi.glyph_now[k].size = 
                    xgobi.glyph_prev[k].size = cdf_def_point_size[l];
                xgobi.last_forward[k] = glyph_color_pointtype (&xgobi, k);
                k++;
              }

  /* Plot */
  prev_is_color_painting = xgobi.is_color_painting;
  xgobi.is_color_painting = True && (!mono);
  plot_once (&xgobi);
  xgobi.is_color_painting = prev_is_color_painting;

  /* forward brushing information to other XGobis - enforce persistent brushing */
  brush_mode_old = xgobi.brush_mode;
  xgobi.brush_mode = persistent;
  copy_brushinfo_to_senddata (&xgobi);
  br_update_cback (NULL, &xgobi, NULL);
  xgobi.brush_mode = brush_mode_old;

  update_required = False;
  sprintf (outstr, "28.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - cdf_weights[nrows] : nrows x double
   Function:  29
*/

void RPC_Send_CDF_Weights (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!has_cdf_symbols)
  {
     sprintf (outstr, "29.01");
     RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "29.02");
     RETURN;
  } 

  if (nc == 2)
  {
    tokptr = strtok (instr," ");
    if (! sscanf (tokptr, "%lf", &cdf_weights[cdf_xranks[i++]]))
    {
       sprintf (outstr, "29.02");
       RETURN;
    }

    while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr))
      sscanf (tokptr, "%lf", &cdf_weights[cdf_xranks[i++]]);
  }
  else
    {
      tokptr = strtok (instr," ");
      if (! sscanf (tokptr, "%lf", &cdf_weights[i++]))
      {
         sprintf (outstr, "29.02");
         RETURN;
      }

      while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nr))
        sscanf (tokptr, "%lf", &cdf_weights[i++]);
    }

  if (i != nr)
  {
     sprintf (outstr, "29.02");
     RETURN;
  }

  has_cdf_weights = True;
  sprintf (outstr, "29.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - (name, p, n, distance) : (String, int, int, double)
   Function:  31
*/

void RPC_Init_VARIO_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;

  RPC_FIRST_LINE();

  free_old_pointers ();

  if (sscanf (instr, "%s %d %d %f\n", name, &vario_p, &vario_n, &vario_dist) < 4)
  {
     sprintf (outstr, "31.01");
     RETURN;
  } 

  vario_values = (float **) XtMalloc ((Cardinal) vario_n * sizeof (float *));
  for (i = 0; i < vario_n; i++)
    vario_values[i] = (float *) XtMalloc ((Cardinal) (vario_p + 2) * sizeof (float));

  vario_missings = (short **) XtMalloc ((Cardinal) vario_n * sizeof (short *));
  for (i = 0; i < vario_n; i++)
    vario_missings[i] = (short *) XtMalloc ((Cardinal) (vario_p + 2) * sizeof (short));

  is_vario_init = True;
  sprintf (outstr, "31.00");

  RPC_LAST_LINE();
}



void return_vario_pair (i, j, k)

int i;
int *j, *k;

{
  *j = vario_w[i].fir_obs;
  *k = vario_w[i].sec_obs;
}



/*
   Usage:     In - datap[n, p + 2] : (n * (p + 2)) x float
   Function:  32
*/

void RPC_Send_Init_VARIO_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int datcount = 0,
      i = 0, j = 0, k;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!is_vario_init || has_vario_data)
  {
     sprintf (outstr, "32.01");
     RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "32.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! strcmp (tokptr, "NA") || ! strcmp (tokptr, "na"))
  {
    vario_missings[i][j] = 1;
    vario_values[i][j] = 0.0;
  }
  else 
    if (! sscanf (tokptr, "%f", &vario_values[i][j]))
    {
       sprintf (outstr, "32.02");
       RETURN;
    } 
    else
       vario_missings[i][j] = 0;

  datcount++;

  while (((tokptr = strtok (NULL, " ")) != NULL) && (datcount < vario_n * (vario_p + 2)))
  {
    i = datcount / (vario_p + 2);   /* row */
    j = datcount % (vario_p + 2);   /* column */

    if (! strcmp (tokptr, "NA") || ! strcmp (tokptr, "na"))
    {
      vario_missings[i][j] = 1;
      vario_values[i][j] = 0.0;
    }
    else
      {
        sscanf (tokptr, "%f", &vario_values[i][j]);
        vario_missings[i][j] = 0;
      }
    datcount++;
  }

  if (datcount != vario_n * (vario_p + 2))
  {
     sprintf (outstr, "32.02");
     RETURN;
  }

  /* Allocate space for vario counters */
  vario_np_e_p1 = (int *) XtMalloc (vario_n * sizeof(int));
  vario_np_e_p2 = (int *) XtMalloc (vario_n * sizeof(int));
  for (i = 0; i < vario_n; i++)
  { 
    vario_np_e_p1[i] = 0;
    vario_np_e_p2[i] = 0;
  }

  /* Allocate space for vario bin_map */
  k =  ceil ((double) vario_n / (double) (sizeof (unsigned int) * 8));
  vario_bin_map = (unsigned int **) XtMalloc ((Cardinal) vario_n * sizeof (unsigned int *));
  for (i = 0; i < vario_n; i++)
  {
    vario_bin_map[i] = (unsigned int *) XtMalloc ((Cardinal) k * sizeof (unsigned int));
    for (j = 0; j < k; j++)
      vario_bin_map[i][j] = 0;
  }

  /* Determine bitmap */
  nr = 0;
  nc = vario_p * vario_p + 5;
  construct_vario_lag_bitmap (vario_n, vario_p, vario_values, vario_missings,
    vario_dist, &nr, vario_np_e_p1, vario_np_e_p2, vario_bin_map);

  /* Allocate data space */
  datap = (float **) XtMalloc ((Cardinal) nr * sizeof (float *));
  for (i = 0; i < nr; i++)
    datap[i] = (float *) XtMalloc ((Cardinal) (nc + 1) * sizeof (float));
 
  /* Allocate missing space */
  missingp = (short **) XtMalloc ((Cardinal) nr * sizeof (short *));
  for (i = 0; i < nr; i++)
    missingp[i] = (short *) XtMalloc ((Cardinal) (nc + 1) * sizeof (short));

  vario_w = (vario_lag_record *) XtMalloc (nr * sizeof(vario_lag_record));

  /* Allocate space for vario pointers */
  vario_ptarr1 = (int **) XtMalloc (vario_n * sizeof (int *));
  vario_ptarr2 = (int **) XtMalloc (vario_n * sizeof (int *));
  for (i = 0; i < vario_n; i++)
  {
    vario_ptarr1[i] = (int *) XtMalloc (vario_np_e_p1[i] * sizeof(int));
    vario_ptarr2[i] = (int *) XtMalloc (vario_np_e_p2[i] * sizeof(int));
  }

  for (i = 0; i < vario_n; i++)
  {
    vario_np_e_p1[i] = 0;
    vario_np_e_p2[i] = 0;
  }

  /* Calculate values for datap */
  construct_vario_data (vario_n, vario_p, vario_values, vario_missings,
     datap, missingp, &mv_missing_values_present, &mv_nmissing,
     vario_np_e_p1, vario_np_e_p2, vario_bin_map,
     vario_w, vario_ptarr1, vario_ptarr2);

  /* Allocate row space */
  rowp = (char **) XtMalloc ((Cardinal) nr * sizeof (char *));
  for (i = 0; i < nr; i++)
    rowp[i] = (char *) XtMalloc ((Cardinal) ROWLABLEN * sizeof (char));

  /* Create default row labels */
  for (i = 0; i < nr; i++)
  {
    return_vario_pair (i, &j, &k);
    (void) sprintf (rowp[i], "#%d-#%d", j, k);
  }

  /* Allocate column space */
  colp = (char **) XtMalloc ((Cardinal) (nc + 1) * sizeof (char *));
  for (i = 0; i < nc + 1; i++)
    colp[i] = (char *) XtMalloc ((Cardinal) COLLABLEN * sizeof (char));

  /* Create default column labels */
  i = 0;
  (void) sprintf (colp[i++], "d");
  (void) sprintf (colp[i++], "A");
  (void) sprintf (colp[i++], "sinA");
  (void) sprintf (colp[i++], "cosA");

  for (j = 0; j < vario_p; j++)
    for (k = 0; k < vario_p; k++)
      (void) sprintf (colp[i++], "g_%d%d", j + 1, k + 1);
  (void) sprintf (colp[i++], "D");
  (void) sprintf (colp[i], "%d", i + 1);

  /* Allocate space for size, color, glyph, erase, and lastf */
  size = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  color = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  glyph = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  erase = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  lastf = (int *) XtMalloc ((Cardinal) nr * sizeof (int));

  has_vario_data = True;
  sprintf (outstr, "32.00.%d.%d", nr, nc);

  RPC_LAST_LINE();
}



/*
   Usage:     In - size_color_glyph : int	
   Function:  33
*/

void RPC_Send_Init_VARIO_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0;
  int pointtype;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!has_vario_data || has_vario_symbols)
  {
     sprintf (outstr, "33.01");
     RETURN;
  } 

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "33.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%d", &pointtype))
  {
    sprintf (outstr, "33.02");
    RETURN;
  }

  for (i = 0; i < nr ; i++)
    if (split_color_glyph_size (pointtype, &lastf[i], &size[i], &glyph[i], &erase[i], &color[i]))
    {
      sprintf (outstr, "33.03");
      RETURN;
    }

  has_vario_symbols = True;
  sprintf (outstr, "33.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - NULL	
   Function:  34
*/

void RPC_Make_VARIO_XGobi (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;

  RPC_FIRST_LINE();

  if (!has_vario_symbols)
  {
    sprintf (outstr, "34.01");
    RETURN;
  }

  if (xgobi.xgobi_is_up)
  {
    int ncols_prev = xgobi.ncols;
    int ncols_used_prev = xgobi.ncols_used;

    reinit_1 (&xgobi, ncols_prev, ncols_used_prev);
    reinit_2 (&xgobi);
    reinit_3 (&xgobi, NULL, name, ncols_prev, ncols_used_prev);
  }
  else
    if (!make_xgobi (datapflag, NULL, datap, name,
          mv_missing_values_present, missingp,
          mv_is_missing_values_xgobi, mv_nmissing,
          nr, rowp, nc + 1, colp,
  	  0, (connect_lines *) NULL, &xgobi, prog_shell, True))
    {
       sprintf (outstr, "34.02");
       RETURN;
    }

  for (i = 0; i < nr; i++)
  {
    xgobi.color_ids[i] = xgobi.color_now[i] =
      xgobi.color_prev[i] = color_nums[color[i]];
    xgobi.glyph_ids[i].type = xgobi.glyph_now[i].type = 
      xgobi.glyph_prev[i].type = glyph[i];
    xgobi.glyph_ids[i].size = xgobi.glyph_now[i].size = 
      xgobi.glyph_prev[i].size = size[i];
    xgobi.erased[i] = erase[i];
    xgobi.last_forward[i] = lastf[i];
  }

  xgobi.nlinkable = xgobi.nrows; /* allow linked brushing for all points */
  xgobi.xy_vars.x = 0; /* do not swap x & y axis */
  xgobi.xy_vars.y = 1;
  xgobi.connect_the_points = False; /* do not connect the points */
  set_showlines (xgobi.connect_the_points);
  xgobi.clone_Type = ArcData;
  if (xgobi.xplore_flag)
    active_xplore_buttons();

  /* enable options allowed for data mode */
  reset_Exit_cmd (&xgobi);
  set_Edit_Lines_cmd (&xgobi, True);
  set_brush_menu_cmd (True);
  /*set_erase_menu_cmd (&xgobi, True);*/  /* dfs erase */
  reset_io_read_menu_cmd (&xgobi);
  reset_io_line_read_menu_cmd (&xgobi);

  is_running = True;
  update_required = True;
  xgobi.xgobi_is_up = True;
  sprintf (outstr, "34.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - brushtype, list of index:size_color_glyph : int, list of (int:int)
   Function:  35
*/

void RPC_Update_Some_VARIO_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  char TempStr[20];
  char *DataStr = NULL;

  int i, j, k, l, m, n;
  int brushtype, pointtype;
  char *tokptr;
  Boolean prev_is_color_painting;
  enum brush_mode_type brush_mode_old;

  RPC_FIRST_LINE();

  DataStr = (char *) XtMalloc ((Cardinal) xgobi.nrows_in_plot * 20);

  strcpy (DataStr, "35.00."); /* concatenate output string */

  if (!has_vario_symbols || !xgobi.xgobi_is_up)
  {
    sprintf (outstr, "35.01");
    XtFree ((XtPointer) DataStr);
    RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
    sprintf (outstr, "35.02");
    XtFree ((XtPointer) DataStr);
    RETURN;
  } 

  tokptr = strtok (instr," ");
  if (sscanf (tokptr, "%d", &brushtype) < 1)
  {
    sprintf (outstr, "35.02");
    XtFree ((XtPointer) DataStr);
    RETURN;
  }

  if ((brushtype < 0) || (brushtype > 3))
  {
    sprintf (outstr, "35.03");
    XtFree ((XtPointer) DataStr);
    RETURN;
  }

  if (brushtype == 3)
    while ((tokptr = strtok (NULL, " ")) != NULL)
    {
      if (sscanf (tokptr, "%d:%d:%d", &i, &j, &pointtype) < 3)
      {
        sprintf (outstr, "35.02");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      if ((i < 0) || (i >= vario_n) || (j < 0) || (j >= vario_n))
      {
        sprintf (outstr, "35.04");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      k = 0;
      l = vario_ptarr1[i][k];
      while ((k < vario_np_e_p1[i]) && (vario_w[l].sec_obs != j))
      {
        k++;
        l = vario_ptarr1[i][k];
      }

      if (split_color_glyph_size (pointtype, &lastf[l], &size[l], &glyph[l], &erase[l], &color[l]))
      {
        sprintf (outstr, "35.05");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      xgobi.color_ids[l] = xgobi.color_now[l] =
        xgobi.color_prev[l] = color_nums[color[l]];
      xgobi.glyph_ids[l].type = xgobi.glyph_now[l].type = 
        xgobi.glyph_prev[l].type = glyph[l];
      xgobi.glyph_ids[l].size = xgobi.glyph_now[l].size = 
        xgobi.glyph_prev[l].size = size[l];
      xgobi.erased[l] = erase[l];
      xgobi.last_forward[l] = lastf[l];
      return_vario_pair (l, &m, &n);
      sprintf (TempStr, "%d:%d:%d ", m, n, pointtype);
      strcat (DataStr, TempStr);
    }
  else
    while ((tokptr = strtok (NULL, " ")) != NULL)
    {
      if (sscanf (tokptr, "%d:%d", &i, &pointtype) < 2)
      {
        sprintf (outstr, "35.02");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      if ((i < 0) || (i >= vario_n))
      {
        sprintf (outstr, "35.04");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      if ((brushtype == 0) || (brushtype == 1))
      {
        for (j = 0; j < vario_np_e_p1[i]; j++)
        {
          l = vario_ptarr1[i][j];

          if (split_color_glyph_size (pointtype, &lastf[l], &size[l], &glyph[l], &erase[l], &color[l]))
          {
            sprintf (outstr, "35.05");
            XtFree ((XtPointer) DataStr);
            RETURN;
          }

          xgobi.color_ids[l] = xgobi.color_now[l] =
            xgobi.color_prev[l] = color_nums[color[l]];
          xgobi.glyph_ids[l].type = xgobi.glyph_now[l].type = 
            xgobi.glyph_prev[l].type = glyph[l];
          xgobi.glyph_ids[l].size = xgobi.glyph_now[l].size = 
            xgobi.glyph_prev[l].size = size[l];
          xgobi.erased[l] = erase[l];
          xgobi.last_forward[l] = lastf[l];
          return_vario_pair (l, &m, &n);
          sprintf (TempStr, "%d:%d:%d ", m, n, pointtype);
          strcat (DataStr, TempStr);
        }
      }

      if ((brushtype == 0) || (brushtype == 2))
      {
        for (j = 0; j < vario_np_e_p2[i]; j++)
        {
          l = vario_ptarr2[i][j];

          if (split_color_glyph_size (pointtype, &lastf[l], &size[l], &glyph[l], &erase[l], &color[l]))
          {
            sprintf (outstr, "35.05");
            XtFree ((XtPointer) DataStr);
            RETURN;
          }

          xgobi.color_ids[l] = xgobi.color_now[l] =
            xgobi.color_prev[l] = color_nums[color[l]];
          xgobi.glyph_ids[l].type = xgobi.glyph_now[l].type = 
            xgobi.glyph_prev[l].type = glyph[l];
          xgobi.glyph_ids[l].size = xgobi.glyph_now[l].size = 
            xgobi.glyph_prev[l].size = size[l];
          xgobi.erased[l] = erase[l];
          xgobi.last_forward[l] = lastf[l];
          return_vario_pair (l, &m, &n);
          sprintf (TempStr, "%d:%d:%d ", m, n, pointtype);
          strcat (DataStr, TempStr);
        }
      } 
    }

  /* Plot */
  prev_is_color_painting = xgobi.is_color_painting;
  xgobi.is_color_painting = True && (!mono);
  plot_once (&xgobi);
  xgobi.is_color_painting = prev_is_color_painting;

  /* forward brushing information to other XGobis - enforce persistent brushing */
  brush_mode_old = xgobi.brush_mode;
  xgobi.brush_mode = persistent;
  copy_brushinfo_to_senddata (&xgobi);
  br_update_cback (NULL, &xgobi, NULL);
  xgobi.brush_mode = brush_mode_old;

  update_required = False;
  sprintf (outstr, DataStr);
  XtFree ((XtPointer) DataStr);

  RPC_LAST_LINE();
}



/*
   Usage:     In - (name, p, n, distance) : (String, int, int, double)
   Function:  41
*/

void RPC_Init_LAG_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;

  RPC_FIRST_LINE();

  free_old_pointers ();

  if (sscanf (instr, "%s %d %d %f\n", name, &lag_p, &lag_n, &lag_dist) < 4)
  {
     sprintf (outstr, "41.01");
     RETURN;
  } 

  lag_values = (float **) XtMalloc ((Cardinal) lag_n * sizeof (float *));
  for (i = 0; i < lag_n; i++)
    lag_values[i] = (float *) XtMalloc ((Cardinal) (lag_p + 2) * sizeof (float));

  lag_missings = (short **) XtMalloc ((Cardinal) lag_n * sizeof (short *));
  for (i = 0; i < lag_n; i++)
    lag_missings[i] = (short *) XtMalloc ((Cardinal) (lag_p + 2) * sizeof (short));

  is_lag_init = True;
  sprintf (outstr, "41.00");

  RPC_LAST_LINE();
}



void return_lag_pair (i, j, k)

int i;
int *j, *k;

{
  *j = lag_w[i].fir_obs;
  *k = lag_w[i].sec_obs;
}



/*
   Usage:     In - datap[n, p + 2] : (n * (p + 2)) x float
   Function:  42
*/

void RPC_Send_Init_LAG_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int datcount = 0,
      i = 0, j = 0, k;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!is_lag_init || has_lag_data)
  {
     sprintf (outstr, "42.01");
     RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "42.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! strcmp (tokptr, "NA") || ! strcmp (tokptr, "na"))
  {
    lag_missings[i][j] = 1;
    lag_values[i][j] = 0.0;
  }
  else 
    if (! sscanf (tokptr, "%f", &lag_values[i][j]))
    {
       sprintf (outstr, "42.02");
       RETURN;
    } 
    else
       lag_missings[i][j] = 0;

  datcount++;

  while (((tokptr = strtok (NULL, " ")) != NULL) && (datcount < lag_n * (lag_p + 2)))
  {
    i = datcount / (lag_p + 2);   /* row */
    j = datcount % (lag_p + 2);   /* column */

    if (! strcmp (tokptr, "NA") || ! strcmp (tokptr, "na"))
    {
      lag_missings[i][j] = 1;
      lag_values[i][j] = 0.0;
    }
    else
      {
        sscanf (tokptr, "%f", &lag_values[i][j]);
        lag_missings[i][j] = 0;
      }
    datcount++;
  }

  if (datcount != lag_n * (lag_p + 2))
  {
     sprintf (outstr, "42.02");
     RETURN;
  }

  /* Allocate space for vario counters */
  lag_np_e_p1 = (int *) XtMalloc (lag_n * sizeof(int));
  lag_np_e_p2 = (int *) XtMalloc (lag_n * sizeof(int));
  for (i = 0; i < lag_n; i++)
  { 
    lag_np_e_p1[i] = 0;
    lag_np_e_p2[i] = 0;
  }

  /* Allocate space for vario bin_map */
  k =  ceil ((double) lag_n / (double) (sizeof (unsigned int) * 8));
  lag_bin_map = (unsigned int **) XtMalloc ((Cardinal) lag_n * sizeof (unsigned int *));
  for (i = 0; i < lag_n; i++)
  {
    lag_bin_map[i] = (unsigned int *) XtMalloc ((Cardinal) k * sizeof (unsigned int));
    for (j = 0; j < k; j++)
      lag_bin_map[i][j] = 0;
  }

  /* Determine bitmap */
  nr = 0;
  nc = lag_p * 2 + 4;
  construct_vario_lag_bitmap (lag_n, lag_p, lag_values, lag_missings,
    lag_dist, &nr, lag_np_e_p1, lag_np_e_p2, lag_bin_map);

  /* Allocate data space */
  datap = (float **) XtMalloc ((Cardinal) nr * sizeof (float *));
  for (i = 0; i < nr; i++)
    datap[i] = (float *) XtMalloc ((Cardinal) (nc + 1) * sizeof (float));
 
  /* Allocate missing space */
  missingp = (short **) XtMalloc ((Cardinal) nr * sizeof (short *));
  for (i = 0; i < nr; i++)
    missingp[i] = (short *) XtMalloc ((Cardinal) (nc + 1) * sizeof (short));

  lag_w = (vario_lag_record *) XtMalloc (nr * sizeof(vario_lag_record));

  /* Allocate space for vario pointers */
  lag_ptarr1 = (int **) XtMalloc (lag_n * sizeof (int *));
  lag_ptarr2 = (int **) XtMalloc (lag_n * sizeof (int *));
  for (i = 0; i < lag_n; i++)
  {
    lag_ptarr1[i] = (int *) XtMalloc (lag_np_e_p1[i] * sizeof(int));
    lag_ptarr2[i] = (int *) XtMalloc (lag_np_e_p2[i] * sizeof(int));
  }

  for (i = 0; i < lag_n; i++)
  {
    lag_np_e_p1[i] = 0;
    lag_np_e_p2[i] = 0;
  }

  /* Calculate values for datap */
  construct_lag_data (lag_n, lag_p, lag_values, lag_missings,
     datap, missingp, &mv_missing_values_present, &mv_nmissing,
     lag_np_e_p1, lag_np_e_p2, lag_bin_map,
     lag_w, lag_ptarr1, lag_ptarr2);

  /* Allocate row space */
  rowp = (char **) XtMalloc ((Cardinal) nr * sizeof (char *));
  for (i = 0; i < nr; i++)
    rowp[i] = (char *) XtMalloc ((Cardinal) ROWLABLEN * sizeof (char));

  /* Create default row labels */
  for (i = 0; i < nr; i++)
  {
    return_lag_pair (i, &j, &k);
    (void) sprintf (rowp[i], "#%d-#%d", j, k);
  }

  /* Allocate column space */
  colp = (char **) XtMalloc ((Cardinal) (nc + 1) * sizeof (char *));
  for (i = 0; i < nc + 1; i++)
    colp[i] = (char *) XtMalloc ((Cardinal) COLLABLEN * sizeof (char));

  /* Create default column labels */
  i = 0;
  (void) sprintf (colp[i++], "d");
  (void) sprintf (colp[i++], "A");
  (void) sprintf (colp[i++], "sinA");
  (void) sprintf (colp[i++], "cosA");

  for (k = 0; k < lag_p; k++)
  {
    (void) sprintf (colp[i++], "Var%d_i", k + 1);
    (void) sprintf (colp[i++], "Var%d_j", k + 1);
  }
  (void) sprintf (colp[i], "%d", i + 1);

  /* Allocate space for size, color, glyph, erase, and lastf */
  size = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  color = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  glyph = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  erase = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  lastf = (int *) XtMalloc ((Cardinal) nr * sizeof (int));

  has_lag_data = True;
  sprintf (outstr, "42.00.%d.%d", nr, nc);

  RPC_LAST_LINE();
}



/*
   Usage:     In - size_color_glyph : int	
   Function:  43
*/

void RPC_Send_Init_LAG_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0;
  int pointtype;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!has_lag_data || has_lag_symbols)
  {
     sprintf (outstr, "43.01");
     RETURN;
  } 

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "43.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%d", &pointtype))
  {
    sprintf (outstr, "43.02");
    RETURN;
  }

  for (i = 0; i < nr ; i++)
    if (split_color_glyph_size (pointtype, &lastf[i], &size[i], &glyph[i], &erase[i], &color[i]))
    {
      sprintf (outstr, "43.03");
      RETURN;
    }

  has_lag_symbols = True;
  sprintf (outstr, "43.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - NULL	
   Function:  44
*/

void RPC_Make_LAG_XGobi (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;

  RPC_FIRST_LINE();

  if (!has_lag_symbols)
  {
    sprintf (outstr, "44.01");
    RETURN;
  }


  if (xgobi.xgobi_is_up)
  {
    int ncols_prev = xgobi.ncols;
    int ncols_used_prev = xgobi.ncols_used;

    reinit_1 (&xgobi, ncols_prev, ncols_used_prev);
    reinit_2 (&xgobi);
    reinit_3 (&xgobi, NULL, name, ncols_prev, ncols_used_prev);
  }
  else
    if (!make_xgobi (datapflag, NULL, datap, name,
          mv_missing_values_present, missingp,
          mv_is_missing_values_xgobi, mv_nmissing,
          nr, rowp, nc + 1, colp,
  	  0, (connect_lines *) NULL, &xgobi, prog_shell))
    {
       sprintf (outstr, "44.02");
       RETURN;
    }

  for (i = 0; i < nr; i++)
  {
    xgobi.color_ids[i] = xgobi.color_now[i] =
      xgobi.color_prev[i] = color_nums[color[i]];
    xgobi.glyph_ids[i].type = xgobi.glyph_now[i].type = 
      xgobi.glyph_prev[i].type = glyph[i];
    xgobi.glyph_ids[i].size = xgobi.glyph_now[i].size = 
      xgobi.glyph_prev[i].size = size[i];
    xgobi.erased[i] = erase[i];
    xgobi.last_forward[i] = lastf[i];
  }

  xgobi.nlinkable = xgobi.nrows; /* allow linked brushing for all points */
  xgobi.xy_vars.x = 0; /* do not swap x & y axis */
  xgobi.xy_vars.y = 1;
  xgobi.connect_the_points = False; /* do not connect the points */
  set_showlines (xgobi.connect_the_points);
  xgobi.clone_Type = ArcData;
  if (xgobi.xplore_flag)
    active_xplore_buttons();

  /* enable options allowed for data mode */
  reset_Exit_cmd (&xgobi);
  set_Edit_Lines_cmd (&xgobi, True);
  set_brush_menu_cmd (True);
  /*set_erase_menu_cmd (&xgobi, True);*/  /* dfs erase */
  reset_io_read_menu_cmd (&xgobi);
  reset_io_line_read_menu_cmd (&xgobi);

  is_running = True;
  update_required = True;
  xgobi.xgobi_is_up = True;
  sprintf (outstr, "44.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - brushtype, list of index:size_color_glyph : int, list of (int:int)
   Function:  45
*/

void RPC_Update_Some_LAG_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  char TempStr[20];
  char *DataStr = NULL;

  int i, j, k, l, m, n;
  int brushtype, pointtype;
  char *tokptr;
  Boolean prev_is_color_painting;
  enum brush_mode_type brush_mode_old;

  RPC_FIRST_LINE();

  DataStr = (char *) XtMalloc ((Cardinal) xgobi.nrows_in_plot * 20);

  strcpy (DataStr, "45.00."); /* concatenate output string */

  if (!has_lag_symbols || !xgobi.xgobi_is_up)
  {
    sprintf (outstr, "45.01");
    RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
    sprintf (outstr, "45.02");
    XtFree ((XtPointer) DataStr);
    RETURN;
  } 

  tokptr = strtok (instr," ");
  if (sscanf (tokptr, "%d", &brushtype) < 1)
  {
    sprintf (outstr, "45.02");
    XtFree ((XtPointer) DataStr);
    RETURN;
  }

  if ((brushtype < 0) || (brushtype > 3))
  {
    sprintf (outstr, "45.03");
    XtFree ((XtPointer) DataStr);
    RETURN;
  }

  if (brushtype == 3)
    while ((tokptr = strtok (NULL, " ")) != NULL)
    {
      if (sscanf (tokptr, "%d:%d:%d", &i, &j, &pointtype) < 3)
      {
        sprintf (outstr, "45.02");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      if ((i < 0) || (i >= lag_n) || (j < 0) || (j >= lag_n))
      {
        sprintf (outstr, "45.04");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      k = 0;
      l = lag_ptarr1[i][k];
      while ((k < lag_np_e_p1[i]) && (lag_w[l].sec_obs != j))
      {
        k++;
        l = lag_ptarr1[i][k];
      }

      if (split_color_glyph_size (pointtype, &lastf[l], &size[l], &glyph[l], &erase[l], &color[l]))
      {
        sprintf (outstr, "45.05");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      xgobi.color_ids[l] = xgobi.color_now[l] =
        xgobi.color_prev[l] = color_nums[color[l]];
      xgobi.glyph_ids[l].type = xgobi.glyph_now[l].type = 
        xgobi.glyph_prev[l].type = glyph[l];
      xgobi.glyph_ids[l].size = xgobi.glyph_now[l].size = 
        xgobi.glyph_prev[l].size = size[l];
      xgobi.erased[l] = erase[l];
      xgobi.last_forward[l] = lastf[l];
      return_lag_pair (l, &m, &n);
      sprintf (TempStr, "%d:%d:%d ", m, n, pointtype);
      strcat (DataStr, TempStr);
    }
  else
    while ((tokptr = strtok (NULL, " ")) != NULL)
    {
      if (sscanf (tokptr, "%d:%d", &i, &pointtype) < 2)
      {
        sprintf (outstr, "45.02");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      if ((i < 0) || (i >= lag_n))
      {
        sprintf (outstr, "45.04");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      if ((brushtype == 0) || (brushtype == 1))
      {
        for (j = 0; j < lag_np_e_p1[i]; j++)
        {
          l = lag_ptarr1[i][j];

          if (split_color_glyph_size (pointtype, &lastf[l], &size[l], &glyph[l], &erase[l], &color[l]))
          {
            sprintf (outstr, "45.05");
            XtFree ((XtPointer) DataStr);
            RETURN;
          }

          xgobi.color_ids[l] = xgobi.color_now[l] =
            xgobi.color_prev[l] = color_nums[color[l]];
          xgobi.glyph_ids[l].type = xgobi.glyph_now[l].type = 
            xgobi.glyph_prev[l].type = glyph[l];
          xgobi.glyph_ids[l].size = xgobi.glyph_now[l].size = 
            xgobi.glyph_prev[l].size = size[l];
          xgobi.erased[l] = erase[l];
          xgobi.last_forward[l] = lastf[l];
          return_lag_pair (l, &m, &n);
          sprintf (TempStr, "%d:%d:%d ", m, n, pointtype);
          strcat (DataStr, TempStr);
        }
      }

      if ((brushtype == 0) || (brushtype == 2))
      {
        for (j = 0; j < lag_np_e_p2[i]; j++)
        {
          l = lag_ptarr2[i][j];

          if (split_color_glyph_size (pointtype, &lastf[l], &size[l], &glyph[l], &erase[l], &color[l]))
          {
            sprintf (outstr, "45.05");
            XtFree ((XtPointer) DataStr);
            RETURN;
          }

          xgobi.color_ids[l] = xgobi.color_now[l] =
            xgobi.color_prev[l] = color_nums[color[l]];
          xgobi.glyph_ids[l].type = xgobi.glyph_now[l].type = 
            xgobi.glyph_prev[l].type = glyph[l];
          xgobi.glyph_ids[l].size = xgobi.glyph_now[l].size = 
            xgobi.glyph_prev[l].size = size[l];
          xgobi.erased[l] = erase[l];
          xgobi.last_forward[l] = lastf[l];
          return_lag_pair (l, &m, &n);
          sprintf (TempStr, "%d:%d:%d ", m, n, pointtype);
          strcat (DataStr, TempStr);
        }
      } 
    }

  /* Plot */
  prev_is_color_painting = xgobi.is_color_painting;
  xgobi.is_color_painting = True && (!mono);
  plot_once (&xgobi);
  xgobi.is_color_painting = prev_is_color_painting;

  /* forward brushing information to other XGobis - enforce persistent brushing */
  brush_mode_old = xgobi.brush_mode;
  xgobi.brush_mode = persistent;
  copy_brushinfo_to_senddata (&xgobi);
  br_update_cback (NULL, &xgobi, NULL);
  xgobi.brush_mode = brush_mode_old;

  update_required = False;
  sprintf (outstr, DataStr);
  XtFree ((XtPointer) DataStr);

  RPC_LAST_LINE();
}



/*
   Usage:     In - colp[lag_p] : lag_p x Str[COLLABLEN]
   Function:  46
*/

void RPC_Send_LAG_Colnames (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 4;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!has_lag_data)
  {
     sprintf (outstr, "46.01");
     RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "46.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%s", colp[i]))
  {
     sprintf (outstr, "46.02");
     RETURN;
  } 

  if (strlen (colp[i]) <= COLLABLEN - 2)
    strcpy (colp[i + 1], colp[i]);
  else
  {
    strncpy (colp[i + 1], colp[i], COLLABLEN - 2);
    strcpy (colp[i], colp[i + 1]);
  }

  strcat (colp[i], "_i");
  strcat (colp[i + 1], "_j");
  i += 2;

  
  while (((tokptr = strtok (NULL, " ")) != NULL) && (i < nc))
  {
    sscanf (tokptr, "%s", colp[i]);

    if (strlen (colp[i]) <= COLLABLEN - 2)
      strcpy (colp[i + 1], colp[i]);
    else
    {
      strncpy (colp[i + 1], colp[i], COLLABLEN - 2);
      strcpy (colp[i], colp[i + 1]);
    }

    strcat (colp[i], "_i");
    strcat (colp[i + 1], "_j");
    i += 2;
  }

  if (i != nc)
  {
     sprintf (outstr, "46.02");
     RETURN;
  }

  sprintf (outstr, "46.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - (name, p, n, distance) : (String, int, int, double)
   Function:  51
*/

void RPC_Init_VARIO2_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;

  RPC_FIRST_LINE();

  free_old_pointers ();

  if (sscanf (instr, "%s %d %d %f\n", name, &vario2_p, &vario2_n, &vario2_dist) < 4)
  {
     sprintf (outstr, "51.01");
     RETURN;
  } 

  vario2_values = (float **) XtMalloc ((Cardinal) vario2_n * sizeof (float *));
  for (i = 0; i < vario2_n; i++)
    vario2_values[i] = (float *) XtMalloc ((Cardinal) (vario2_p + 2) * sizeof (float));

  vario2_missings = (short **) XtMalloc ((Cardinal) vario2_n * sizeof (short *));
  for (i = 0; i < vario2_n; i++)
    vario2_missings[i] = (short *) XtMalloc ((Cardinal) (vario2_p + 2) * sizeof (short));

  is_vario2_init = True;
  sprintf (outstr, "51.00");

  RPC_LAST_LINE();
}



void return_vario2_pair (i, j, k)

int i;
int *j, *k;

{
  *j = vario2_w[i].fir_obs;
  *k = vario2_w[i].sec_obs;
}



/*
   Usage:     In - datap[n, p + 2] : (n * (p + 2)) x float
   Function:  52
*/

void RPC_Send_Init_VARIO2_Data (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int datcount = 0,
      i = 0, j = 0, k;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!is_vario2_init || has_vario2_data)
  {
     sprintf (outstr, "52.01");
     RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "52.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! strcmp (tokptr, "NA") || ! strcmp (tokptr, "na"))
  {
    vario2_missings[i][j] = 1;
    vario2_values[i][j] = 0.0;
  }
  else 
    if (! sscanf (tokptr, "%f", &vario2_values[i][j]))
    {
       sprintf (outstr, "52.02");
       RETURN;
    } 
    else
       vario2_missings[i][j] = 0;

  datcount++;

  while (((tokptr = strtok (NULL, " ")) != NULL) && (datcount < vario2_n * (vario2_p + 2)))
  {
    i = datcount / (vario2_p + 2);   /* row */
    j = datcount % (vario2_p + 2);   /* column */

    if (! strcmp (tokptr, "NA") || ! strcmp (tokptr, "na"))
    {
      vario2_missings[i][j] = 1;
      vario2_values[i][j] = 0.0;
    }
    else
      {
        sscanf (tokptr, "%f", &vario2_values[i][j]);
        vario2_missings[i][j] = 0;
      }
    datcount++;
  }

  if (datcount != vario2_n * (vario2_p + 2))
  {
     sprintf (outstr, "52.02");
     RETURN;
  }

  /* Allocate space for vario counters */
  vario2_np_e_p1 = (int *) XtMalloc (vario2_n * sizeof(int));
  vario2_np_e_p2 = (int *) XtMalloc (vario2_n * sizeof(int));
  for (i = 0; i < vario2_n; i++)
  { 
    vario2_np_e_p1[i] = 0;
    vario2_np_e_p2[i] = 0;
  }

  /* Allocate space for vario bin_map */
  k =  ceil ((double) vario2_n / (double) (sizeof (unsigned int) * 8));
  vario2_bin_map = (unsigned int **) XtMalloc ((Cardinal) vario2_n * sizeof (unsigned int *));
  for (i = 0; i < vario2_n; i++)
  {
    vario2_bin_map[i] = (unsigned int *) XtMalloc ((Cardinal) k * sizeof (unsigned int));
    for (j = 0; j < k; j++)
      vario2_bin_map[i][j] = 0;
  }

  /* Determine bitmap */
  nr = 0;
  nc = (vario2_p * (vario2_p + 1)) / 2 + 5;
  construct_vario_lag_bitmap (vario2_n, vario2_p, vario2_values, vario2_missings,
    vario2_dist, &nr, vario2_np_e_p1, vario2_np_e_p2, vario2_bin_map);

  /* Allocate data space */
  datap = (float **) XtMalloc ((Cardinal) nr * sizeof (float *));
  for (i = 0; i < nr; i++)
    datap[i] = (float *) XtMalloc ((Cardinal) (nc + 1) * sizeof (float));
 
  /* Allocate missing space */
  missingp = (short **) XtMalloc ((Cardinal) nr * sizeof (short *));
  for (i = 0; i < nr; i++)
    missingp[i] = (short *) XtMalloc ((Cardinal) (nc + 1) * sizeof (short));

  vario2_w = (vario_lag_record *) XtMalloc (nr * sizeof(vario_lag_record));

  /* Allocate space for vario pointers */
  vario2_ptarr1 = (int **) XtMalloc (vario2_n * sizeof (int *));
  vario2_ptarr2 = (int **) XtMalloc (vario2_n * sizeof (int *));
  for (i = 0; i < vario2_n; i++)
  {
    vario2_ptarr1[i] = (int *) XtMalloc (vario2_np_e_p1[i] * sizeof(int));
    vario2_ptarr2[i] = (int *) XtMalloc (vario2_np_e_p2[i] * sizeof(int));
  }

  for (i = 0; i < vario2_n; i++)
  {
    vario2_np_e_p1[i] = 0;
    vario2_np_e_p2[i] = 0;
  }

  /* Calculate values for datap */
  construct_vario2_data (vario2_n, vario2_p, vario2_values, vario2_missings,
     datap, missingp, &mv_missing_values_present, &mv_nmissing,
     vario2_np_e_p1, vario2_np_e_p2, vario2_bin_map,
     vario2_w, vario2_ptarr1, vario2_ptarr2);

  /* Allocate row space */
  rowp = (char **) XtMalloc ((Cardinal) nr * sizeof (char *));
  for (i = 0; i < nr; i++)
    rowp[i] = (char *) XtMalloc ((Cardinal) ROWLABLEN * sizeof (char));

  /* Create default row labels */
  for (i = 0; i < nr; i++)
  {
    return_vario2_pair (i, &j, &k);
    (void) sprintf (rowp[i], "#%d-#%d", j, k);
  }

  /* Allocate column space */
  colp = (char **) XtMalloc ((Cardinal) (nc + 1) * sizeof (char *));
  for (i = 0; i < nc + 1; i++)
    colp[i] = (char *) XtMalloc ((Cardinal) COLLABLEN * sizeof (char));

  /* Create default column labels */
  i = 0;
  (void) sprintf (colp[i++], "d");
  (void) sprintf (colp[i++], "A");
  (void) sprintf (colp[i++], "sinA");
  (void) sprintf (colp[i++], "cosA");

  for (j = 0; j < vario2_p; j++)
    for (k = j; k < vario2_p; k++)
      (void) sprintf (colp[i++], "g_%d%d", j + 1, k + 1);
  (void) sprintf (colp[i++], "D");
  (void) sprintf (colp[i], "%d", i + 1);

  /* Allocate space for size, color, glyph, erase, and lastf */
  size = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  color = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  glyph = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  erase = (int *) XtMalloc ((Cardinal) nr * sizeof (int));
  lastf = (int *) XtMalloc ((Cardinal) nr * sizeof (int));

  has_vario2_data = True;
  sprintf (outstr, "52.00.%d.%d", nr, nc);

  RPC_LAST_LINE();
}



/*
   Usage:     In - size_color_glyph : int	
   Function:  53
*/

void RPC_Send_Init_VARIO2_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i = 0;
  int pointtype;
  char *tokptr;

  RPC_FIRST_LINE();

  if (!has_vario2_data || has_vario2_symbols)
  {
     sprintf (outstr, "53.01");
     RETURN;
  } 

  if ((instr == NULL) || !strcmp (instr, ""))
  {
     sprintf (outstr, "53.02");
     RETURN;
  } 

  tokptr = strtok (instr," ");
  if (! sscanf (tokptr, "%d", &pointtype))
  {
    sprintf (outstr, "53.02");
    RETURN;
  }

  for (i = 0; i < nr ; i++)
    if (split_color_glyph_size (pointtype, &lastf[i], &size[i], &glyph[i], &erase[i], &color[i]))
    {
      sprintf (outstr, "53.03");
      RETURN;
    }

  has_vario2_symbols = True;
  sprintf (outstr, "53.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - NULL	
   Function:  54
*/

void RPC_Make_VARIO2_XGobi (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  int i;

  RPC_FIRST_LINE();

  if (!has_vario2_symbols)
  {
    sprintf (outstr, "54.01");
    RETURN;
  }

  if (xgobi.xgobi_is_up)
  {
    int ncols_prev = xgobi.ncols;
    int ncols_used_prev = xgobi.ncols_used;

    reinit_1 (&xgobi, ncols_prev, ncols_used_prev);
    reinit_2 (&xgobi);
    reinit_3 (&xgobi, NULL, name, ncols_prev, ncols_used_prev);
  }
  else
    if (!make_xgobi (datapflag, NULL, datap, name,
          mv_missing_values_present, missingp,
          mv_is_missing_values_xgobi, mv_nmissing,
          nr, rowp, nc + 1, colp,
  	  0, (connect_lines *) NULL, &xgobi, prog_shell, True))
    {
       sprintf (outstr, "54.02");
       RETURN;
    }

  for (i = 0; i < nr; i++)
  {
    xgobi.color_ids[i] = xgobi.color_now[i] =
      xgobi.color_prev[i] = color_nums[color[i]];
    xgobi.glyph_ids[i].type = xgobi.glyph_now[i].type = 
      xgobi.glyph_prev[i].type = glyph[i];
    xgobi.glyph_ids[i].size = xgobi.glyph_now[i].size = 
      xgobi.glyph_prev[i].size = size[i];
    xgobi.erased[i] = erase[i];
    xgobi.last_forward[i] = lastf[i];
  }

  xgobi.nlinkable = xgobi.nrows; /* allow linked brushing for all points */
  xgobi.xy_vars.x = 0; /* do not swap x & y axis */
  xgobi.xy_vars.y = 1;
  xgobi.connect_the_points = False; /* do not connect the points */
  set_showlines (xgobi.connect_the_points);
  xgobi.clone_Type = ArcData;
  if (xgobi.xplore_flag)
    active_xplore_buttons();

  /* enable options allowed for data mode */
  reset_Exit_cmd (&xgobi);
  set_Edit_Lines_cmd (&xgobi, True);
  set_brush_menu_cmd (True);
  /*set_erase_menu_cmd (&xgobi, True);*/  /* dfs erase */
  reset_io_read_menu_cmd (&xgobi);
  reset_io_line_read_menu_cmd (&xgobi);

  is_running = True;
  update_required = True;
  xgobi.xgobi_is_up = True;
  sprintf (outstr, "54.00");

  RPC_LAST_LINE();
}



/*
   Usage:     In - brushtype, list of index:size_color_glyph : int, list of (int:int)
   Function:  55
*/

void RPC_Update_Some_VARIO2_Symbols (instr, outstr)

CHAR_TYPE *instr, *outstr;

{
  char TempStr[20];
  char *DataStr = NULL;

  int i, j, k, l, m, n;
  int brushtype, pointtype;
  char *tokptr;
  Boolean prev_is_color_painting;
  enum brush_mode_type brush_mode_old;

  RPC_FIRST_LINE();

  DataStr = (char *) XtMalloc ((Cardinal) xgobi.nrows_in_plot * 20);

  strcpy (DataStr, "55.00."); /* concatenate output string */

  if (!has_vario2_symbols || !xgobi.xgobi_is_up)
  {
    sprintf (outstr, "55.01");
    XtFree ((XtPointer) DataStr);
    RETURN;
  }

  if ((instr == NULL) || !strcmp (instr, ""))
  {
    sprintf (outstr, "55.02");
    XtFree ((XtPointer) DataStr);
    RETURN;
  } 

  tokptr = strtok (instr," ");
  if (sscanf (tokptr, "%d", &brushtype) < 1)
  {
    sprintf (outstr, "55.02");
    XtFree ((XtPointer) DataStr);
    RETURN;
  }

  if ((brushtype < 0) || (brushtype > 3))
  {
    sprintf (outstr, "55.03");
    XtFree ((XtPointer) DataStr);
    RETURN;
  }

  if (brushtype == 3)
    while ((tokptr = strtok (NULL, " ")) != NULL)
    {
      if (sscanf (tokptr, "%d:%d:%d", &i, &j, &pointtype) < 3)
      {
        sprintf (outstr, "55.02");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      if ((i < 0) || (i >= vario2_n) || (j < 0) || (j >= vario2_n))
      {
        sprintf (outstr, "55.04");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      k = 0;
      l = vario2_ptarr1[i][k];
      while ((k < vario2_np_e_p1[i]) && (vario2_w[l].sec_obs != j))
      {
        k++;
        l = vario2_ptarr1[i][k];
      }

          if (split_color_glyph_size (pointtype, &lastf[l], &size[l], &glyph[l], &erase[l], &color[l]))
          {
            sprintf (outstr, "55.05");
            XtFree ((XtPointer) DataStr);
            RETURN;
          }

          xgobi.color_ids[l] = xgobi.color_now[l] =
            xgobi.color_prev[l] = color_nums[color[l]];
          xgobi.glyph_ids[l].type = xgobi.glyph_now[l].type = 
            xgobi.glyph_prev[l].type = glyph[l];
          xgobi.glyph_ids[l].size = xgobi.glyph_now[l].size = 
            xgobi.glyph_prev[l].size = size[l];
          xgobi.erased[l] = erase[l];
          xgobi.last_forward[l] = lastf[l];
          return_vario2_pair (l, &m, &n);
          sprintf (TempStr, "%d:%d:%d ", m, n, pointtype);
          strcat (DataStr, TempStr);
    }
  else
    while ((tokptr = strtok (NULL, " ")) != NULL)
    {
      if (sscanf (tokptr, "%d:%d", &i, &pointtype) < 2)
      {
        sprintf (outstr, "55.02");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      if ((i < 0) || (i >= vario2_n))
      {
        sprintf (outstr, "55.04");
        XtFree ((XtPointer) DataStr);
        RETURN;
      }

      if ((brushtype == 0) || (brushtype == 1))
      {
        for (j = 0; j < vario2_np_e_p1[i]; j++)
        {
          l = vario2_ptarr1[i][j];

          if (split_color_glyph_size (pointtype, &lastf[l], &size[l], &glyph[l], &erase[l], &color[l]))
          {
            sprintf (outstr, "55.05");
            XtFree ((XtPointer) DataStr);
            RETURN;
          }

          xgobi.color_ids[l] = xgobi.color_now[l] =
            xgobi.color_prev[l] = color_nums[color[l]];
          xgobi.glyph_ids[l].type = xgobi.glyph_now[l].type = 
            xgobi.glyph_prev[l].type = glyph[l];
          xgobi.glyph_ids[l].size = xgobi.glyph_now[l].size = 
            xgobi.glyph_prev[l].size = size[l];
          xgobi.erased[l] = erase[l];
          xgobi.last_forward[l] = lastf[l];
          return_vario2_pair (l, &m, &n);
          sprintf (TempStr, "%d:%d:%d ", m, n, pointtype);
          strcat (DataStr, TempStr);
        }
      }

      if ((brushtype == 0) || (brushtype == 2))
      {
        for (j = 0; j < vario2_np_e_p2[i]; j++)
        {
          l = vario2_ptarr2[i][j];

          if (split_color_glyph_size (pointtype, &lastf[l], &size[l], &glyph[l], &erase[l], &color[l]))
          {
            sprintf (outstr, "55.05");
            XtFree ((XtPointer) DataStr);
            RETURN;
          }

          xgobi.color_ids[l] = xgobi.color_now[l] =
            xgobi.color_prev[l] = color_nums[color[l]];
          xgobi.glyph_ids[l].type = xgobi.glyph_now[l].type = 
            xgobi.glyph_prev[l].type = glyph[l];
          xgobi.glyph_ids[l].size = xgobi.glyph_now[l].size = 
            xgobi.glyph_prev[l].size = size[l];
          xgobi.erased[l] = erase[l];
          xgobi.last_forward[l] = lastf[l];
          return_vario2_pair (l, &m, &n);
          sprintf (TempStr, "%d:%d:%d ", m, n, pointtype);
          strcat (DataStr, TempStr);
        }
      } 
    }

  /* Plot */
  prev_is_color_painting = xgobi.is_color_painting;
  xgobi.is_color_painting = True && (!mono);
  plot_once (&xgobi);
  xgobi.is_color_painting = prev_is_color_painting;

  /* forward brushing information to other XGobis - enforce persistent brushing */
  brush_mode_old = xgobi.brush_mode;
  xgobi.brush_mode = persistent;
  copy_brushinfo_to_senddata (&xgobi);
  br_update_cback (NULL, &xgobi, NULL);
  xgobi.brush_mode = brush_mode_old;

  update_required = False;
  sprintf (outstr, DataStr);
  XtFree ((XtPointer) DataStr);

  RPC_LAST_LINE();
}

/* !!!MS!!! */
#endif

/*********************/
/* Client-Operations */
/*********************/

#ifdef DCE_RPC_USED

void dce_rpc_client_init()
{
	unsigned32 status;
	unsigned32 val;
	unsigned char *pszProtCl      = (unsigned char *)"ncacn_ip_tcp";
	unsigned char *pszEndpointCl  = NULL;
	unsigned char *pszStrBinCl    = NULL;

	pthread_mutex_init(&mutexCl,pthread_mutexattr_default);
	anzThreads = 0;

	rpc_string_binding_compose(0,
                               pszProtCl,
                               pszNWAddressCl,
                               pszEndpointCl,
                               0,  
                               &pszStrBinCl,
                               &status);
	CHECK_DCE_ERROR(status,ABORT);
	rpc_binding_from_string_binding(pszStrBinCl,
                               &xgobi_client_binding_handle,
                               &status);
	CHECK_DCE_ERROR(status,ABORT);
	/* Klappt nicht auf SGI und tcp/ip. Selbst wenn der Timeout
	   auf 0 gesetzt wird! */
	rpc_mgmt_set_com_timeout(xgobi_client_binding_handle,
		rpc_c_binding_default_timeout,&status);
	CHECK_DCE_ERROR(status,ABORT);
	rpc_mgmt_inq_com_timeout(xgobi_client_binding_handle,&val,&status);
	CHECK_DCE_ERROR(status,ABORT);
	DTEXT1("Timeout Value = %d\n",val);
	rpc_string_free(&pszStrBinCl,&status);
	CHECK_DCE_ERROR(status,ABORT);
}

void dce_rpc_client_done()
{
	/* DCE Free */
	unsigned32 status;

	rpc_binding_free(&xgobi_client_binding_handle,&status); 
	CHECK_DCE_ERROR(status,ABORT);
}

void dce_rpc_client_freemem(pthread_addr_t arg)
{
  char *DataStr = (char*) arg;

  DTEXT1("Cleanup called, arg=%d\n",(int) arg);
  /* Free the memory - DataStr is a local variable! */
  XtFree ((XtPointer) DataStr);
}

long GetTickCount()
{
	struct timeb tp;
	ftime(&tp);

	return ((tp.time%1000000)*1000)+tp.millitm;
}

/* Don`t create more than MAX_THREADS calling Threads */
void dce_rpc_client_call(pthread_addr_t arg)
{
  char *DataStr = (char*) arg;
  int thread_in_field,i;
  int id,rval,tried=0,t1,t2;

  /* cleanup function init */
  pthread_cleanup_push(dce_rpc_client_freemem,arg);
  id = (int) pthread_self();

  pthread_mutex_lock(&mutexCl);

  if (anzThreads>=MAX_CALL_THREADS)
  {
    sleep(5); /* Wait for Termination of a RPC caller */

    if (anzThreads>=MAX_CALL_THREADS)
    {
      DTEXT1("%d: Kill all Threads\n",id);
      /* Kill all Threads */
      for (i=0 ; i<anzThreads ; i++)
      {
        /* rval = pthread_cancel(t_handles_client[i]); */
        DTEXT2("%d: Return von pthread_cancel: %d\n",id,rval);
      }
      anzThreads = 0;
    }
  }
  DTEXT2("%d: AnzThreads = %d\n",id,anzThreads);

  thread_in_field = anzThreads++;
  t_handles_client[thread_in_field] = pthread_self();
  DTEXT2("%d: thread_in_field=%d\n",id,thread_in_field);
  DTEXT2("%d: arg=%d\n",id,(int) arg);

  /* herausgenommen 990820 pthread_mutex_unlock(&mutexCl); */

  tried=0;
  while (tried<3)
  {
    t1 = GetTickCount();
    TRY
    {
      Xfer_Brushinfo(DataStr);
	tried=999;
    }
    CATCH_ALL
    {
      t2 = GetTickCount();
      DTEXT1("Exception, Zeit: %d\n",t2-t1);
      tried++;
      sleep(tried); /* in immer groesseren Abstaenden */
    }
    ENDTRY
  }
  
  if (tried==3)
  {
    dce_rpc_client_done();
    dce_rpc_client_init();
  }


  /* herausgenommen 990820 pthread_mutex_lock(&mutexCl); */
  /* Copy last entry to current */
  t_handles_client[thread_in_field] = t_handles_client[anzThreads--];
  pthread_mutex_unlock(&mutexCl);
  
  DTEXT1("%d: Normal pthread-ending\n",id);
  /* calls the cleanup function */
  pthread_cleanup_pop(1);
  pthread_exit(0);
}

#endif

#ifdef RPC_USED

int rpc_client_main (cl_prognum, cl_vernum)

unsigned long cl_prognum;
unsigned long cl_vernum;

{
  int server_id;

  gethostname(hostname, HOSTNAMELEN);
  
  server_id = aiconnect (hostname, cl_prognum, cl_vernum);
  if (server_id > 99)
  {
    printf ("XGobi: Error in connecting to server - %d\n", server_id);
    return (-1);
  }
  else
    return (server_id);
}

#endif


void xfer_brushinfo (xg)

xgobidata * xg;

{
  char TempStr[20];
  char *DataStr = NULL;
  char *RequStr = NULL;
  char *RetStr = NULL;

  double time = TIMEOUT;

  int i, j, k;
  int pointtype;
  
#ifdef DCE_RPC_USED
  pthread_t t_handle;
  int ret;
#endif

  DataStr = (char *) XtMalloc ((Cardinal) 512 + xg -> nrows_in_plot * 20);
  RequStr = (char *) XtMalloc ((Cardinal) 512 + xg -> nrows_in_plot * 20);
  RetStr = (char *) XtMalloc ((Cardinal) 512 + xg -> nrows_in_plot * 20);


  strcpy (DataStr, ""); /* concatenate output string */

  if (has_symbols) /* case: normal data */
  {
    for (i = 0; i < xg -> nrows_in_plot; i++)
    {
      pointtype = glyph_color_pointtype (xg, xg -> rows_in_plot[i]);

      if (xg -> last_forward[xg -> rows_in_plot[i]] != pointtype)
      {
        sprintf (TempStr, "%d:%d ", xg -> rows_in_plot[i], pointtype);
        strcat (DataStr, TempStr);
        xg -> last_forward[xg -> rows_in_plot[i]] = pointtype;
      }
    }
  }
  else
    if (has_cdf_bitmap || (cdf_default_bitmap && cdf_default_types)) /* case: cdf data */
    {
      for (i = 0; i < xg -> nrows_in_plot; i++)
      {
        pointtype = glyph_color_pointtype (xg, xg -> rows_in_plot[i]);

        if (xg -> last_forward[xg -> rows_in_plot[i]] != pointtype)
        {
          if (nc == 2)
            sprintf (TempStr, "%d:%d ", cdf_inv_xranks[(xg -> rows_in_plot[i]) % nr], pointtype);
          else
            sprintf (TempStr, "%d:%d ", (xg -> rows_in_plot[i]) % nr, pointtype);

          strcat (DataStr, TempStr);
          xg -> last_forward[xg -> rows_in_plot[i]] = pointtype;
        }
      }
    }
    else
      if (has_vario_symbols) /* case: vario data */
      {
        for (i = 0; i < xg -> nrows_in_plot; i++)
        {
          pointtype = glyph_color_pointtype (xg, xg -> rows_in_plot[i]);

          if (xg -> last_forward[xg -> rows_in_plot[i]] != pointtype)
          {
            return_vario_pair (xg -> rows_in_plot[i], &j, &k);
            sprintf (TempStr, "%d:%d:%d ", j, k, pointtype);
            strcat (DataStr, TempStr);
            xg -> last_forward[xg -> rows_in_plot[i]] = pointtype;
          }
        }
      }
      else
        if (has_lag_symbols) /* case: lag data */
        {
          for (i = 0; i < xg -> nrows_in_plot; i++)
          {
            pointtype = glyph_color_pointtype (xg, xg -> rows_in_plot[i]);

            if (xg -> last_forward[xg -> rows_in_plot[i]] != pointtype)
            {
              return_lag_pair (xg -> rows_in_plot[i], &j, &k);
              sprintf (TempStr, "%d:%d:%d ", j, k, pointtype);
              strcat (DataStr, TempStr);
              xg -> last_forward[xg -> rows_in_plot[i]] = pointtype;
            }
          }
        }
      else
        if (has_vario2_symbols) /* case: vario2 data */
        {
          for (i = 0; i < xg -> nrows_in_plot; i++)
          {
            pointtype = glyph_color_pointtype (xg, xg -> rows_in_plot[i]);

            if (xg -> last_forward[xg -> rows_in_plot[i]] != pointtype)
            {
              return_vario2_pair (xg -> rows_in_plot[i], &j, &k);
              sprintf (TempStr, "%d:%d:%d ", j, k, pointtype);
              strcat (DataStr, TempStr);
              xg -> last_forward[xg -> rows_in_plot[i]] = pointtype;
            }
          }
        }

  if (strcmp (DataStr, "") && xg -> xgobi_is_up) 
     /* make RPC call with output string */
  {
    DTEXT("Wants to make an RPC-call\n");
#ifdef RPC_USED
    if (xg -> arcview_flag && (last_server_number != av_server_number))
    {
      sprintf (RequStr, "av.Run(\"XG.RPC.Brush.Some\",{\"%s\"})", DataStr);
      status = airequest (av_server_id, BRUSH_ARCVIEW, RequStr, RetStr, time);
      if (status != 0) 
        printf ("XGobi: Error in submitting ArcView request - %d\n", status);
    }

    if (xg -> xplore_flag && (last_server_number != xpl_server_number))
    {
      sprintf (RequStr, "%x %s", xg_server_number, DataStr);
      status = airequest (xpl_server_id, BRUSH_XPLORE, RequStr, RetStr, time);
      if (status != 0) 
        printf ("XGobi: Error in submitting XploRe request - %d\n", status);
    }

    if (xg -> virgis_flag && (last_server_number != vg_server_number))
    {
      sprintf (RequStr, "%x %s", xg_server_number, DataStr);
      status = airequest (vg_server_id, BRUSH_VIRGIS, RequStr, RetStr, time);
      if (status != 0) 
        printf ("XGobi: Error in submitting VirGIS request - %d\n", status);
    }
#else
    /* DCE-RPC */
	/* p-Thread to rpc_server_listen */
    ret = pthread_create(&t_handle,pthread_attr_default,
          (void *(*)(void *)) dce_rpc_client_call,(pthread_addr_t) DataStr);
#endif

    last_server_number = -1;
  }

  XtFree ((XtPointer) RetStr);
  XtFree ((XtPointer) RequStr);
  /* Bei DCE erfolgt die Rueckgabe des Speichers im anderen Thread */
#ifdef RPC_USED
  XtFree ((XtPointer) DataStr);
#endif

}
