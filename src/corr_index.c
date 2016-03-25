/* corr_index.c */
/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  this  code and associated documentation,  provided      *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *     Copyright (c) of code not pertaining to manual       *
 *     control is owned by Dianne Cook (1994, 1995).        *
 *     Copyright (c) of code pertaining to manual           *
 *     control of the correlation tour is owned jointly by  *
 *     Dianne Cook and AT&T Bell Labs (1995).               *
 *  All Rights Reserved.                                    *
 *                                                          *
 *  We welcome your questions and comments, and request     *
 *  that you share any modifications with us.               *
 *                                                          *
 *      Dianne Cook                Andreas Buja             *
 *    dicook@iastate.edu     andreas@research.att.com       *
 * www.public.iastate.edu/~dicook/                          *
 *                           www.research.att.com/~andreas/ *
 *                                                          *
 ************************************************************/

/*
 * The code in this file was written with a great deal of help
 * from Phil Jones, a graduate student at Iowa State University.
*/

#include <math.h>
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

static double xsum, ysum, xbar, ybar, xy, x2, y2,
              *z1sum, *z2sum, *z1bar, *z2bar, *xz1, *xz2, *yz1, *yz2;

void
alloc_corr_index(xgobidata *xg)
{
  /*  unsigned int px = (unsigned int) xg->ncorr_xvars;
  unsigned int py = (unsigned int) xg->ncorr_yvars;*/
  unsigned int px = (unsigned int) xg->ncols;
  unsigned int py = (unsigned int) xg->ncols;
  static int firsttime = 1;

  if (firsttime == 1)
  {
    z1sum = (double *) XtMalloc(px * sizeof(double));
    z2sum = (double *) XtMalloc(py * sizeof(double));
    z1bar = (double *) XtMalloc(px * sizeof(double));
    z2bar = (double *) XtMalloc(py * sizeof(double));
    xz1 = (double *) XtMalloc(px * sizeof(double));
    xz2 = (double *) XtMalloc(px * sizeof(double));
    yz1 = (double *) XtMalloc(py * sizeof(double));
    yz2 = (double *) XtMalloc(py * sizeof(double));
  }
}

void
free_corr_index(xgobidata *xg)
{
  if (z1sum != NULL)
    XtFree((XtPointer) z1sum);
  if (z2sum != NULL)
    XtFree((XtPointer) z2sum);
  if (z1bar != NULL)
    XtFree((XtPointer) z1bar);
  if (z2bar != NULL)
    XtFree((XtPointer) z2bar);
  if (xz1 != NULL)
    XtFree((XtPointer) xz1);
  if (xz2 != NULL)
    XtFree((XtPointer) xz2);
  if (yz1 != NULL)
    XtFree((XtPointer) yz1);
  XtFree((XtPointer) yz2);
}

/* Note get_corr_index returns the squared correlation coefficient
   and associated derivatives.
*/
void
get_corr_index(xgobidata *xg, float *corr_index, float *didx, float *didy)
{
  int i, j, m;
  int p1 = xg->ncorr_xvars;
  int p2 = xg->ncorr_yvars;
  int nr = xg->nrows_in_plot;

  xsum = ysum = xy = x2 = y2 = 0;
  for (j=0; j<p1; j++)
    z1sum[j] = xz1[j] = yz1[j] = 0;
  for (j=0; j<p2; j++)
    z2sum[j] = xz2[j] = yz2[j] = 0;

  for (m=0; m<nr; m++)
  {
    i = xg->rows_in_plot[m];
    xsum += xg->planar[i].x;
    ysum += xg->planar[i].y;
    for (j=0; j<p1; j++)
      z1sum[j] += xg->world_data[i][xg->corr_xvars[j]];
    for (j=0; j<p2; j++)
      z2sum[j] += xg->world_data[i][xg->corr_yvars[j]];
  }

  xbar = xsum/nr;
  ybar = ysum/nr;
  for (j=0; j<p1; j++)
    z1bar[j] = z1sum[j]/nr;
  for (j=0; j<p2; j++)
    z2bar[j] = z2sum[j]/nr;

  for (m=0; m<nr; m++)
  {
    i = xg->rows_in_plot[m];
    xy += (xg->planar[i].x - xbar)*(xg->planar[i].y - ybar);
    x2 += (xg->planar[i].x - xbar)*(xg->planar[i].x - xbar);
    y2 += (xg->planar[i].y - ybar)*(xg->planar[i].y - ybar);
    for (j=0; j<p1; j++)
    {
      xz1[j] += (xg->planar[i].x - xbar)*
	(xg->world_data[i][xg->corr_xvars[j]] - z1bar[j]);
      yz1[j] += (xg->planar[i].y - ybar)*
	(xg->world_data[i][xg->corr_xvars[j]] - z1bar[j]);
    }
    for (j=0; j<p2; j++)
    {
      xz2[j] += (xg->planar[i].x - xbar)*
	(xg->world_data[i][xg->corr_yvars[j]] - z2bar[j]);
      yz2[j] += (xg->planar[i].y - ybar)*
	(xg->world_data[i][xg->corr_yvars[j]] - z2bar[j]);
    }
  }

  *corr_index = (float) xy*xy/(x2*y2);
  for (j=0; j<p1; j++)
    didx[j] = (float) (2*xy*(x2*yz1[j] - xy*xz1[j])/(x2*x2*y2));
  for (j=0; j<p2; j++)
    didy[j] = (float) (2*xy*(y2*xz2[j] - xy*yz2[j])/(x2*y2*y2));
}

