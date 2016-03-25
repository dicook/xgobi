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

void
get_planar_range(xg, x_min, x_max, y_min, y_max)
  xgobidata *xg;
  long *x_min, *x_max, *y_min, *y_max;
{
  int i, m;

  *x_min = xg->planar[0].x;
  *x_max = xg->planar[0].x;
  *y_min = xg->planar[0].y;
  *y_max = xg->planar[0].y;
  for (m=1; m<xg->nrows_in_plot; m++)
  {
    i = xg->rows_in_plot[m];
    if (xg->planar[i].x < *x_min) *x_min = xg->planar[i].x;
    else if (xg->planar[i].x > *x_max) *x_max = xg->planar[i].x;
    if (xg->planar[i].y < *y_min) *y_min = xg->planar[i].y;
    else if (xg->planar[i].y > *y_max) *y_max = xg->planar[i].y;
  }
}

