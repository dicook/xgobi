/* make_axes.c */
/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  the xgobi code and associated documentation,  provided  *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *        Copyright (c) 1990, ..., 1996 Bellcore            *
 *                                                          *
 *  We welcome your questions and comments, and request     *
 *  that you share any modifications with us.               *
 *                                                          *
 *    Deborah F. Swayne            Dianne Cook              *
 *   dfs@research.att.com       dicook@iastate.edu          *
 *      (973) 360-8423    www.public.iastate.edu/~dicook/   *
 *                                                          *
 *                    Andreas Buja                          *
 *                andreas@research.att.com                  *
 *              www.research.att.com/~andreas/              *
 *                                                          *
 ************************************************************/

/*
 * The code in this file was written by Nancy Hubbell during the
 * summer of 1990.
*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#define DEFAULTNTICKS 10;

static lcoords axes[3];

/* --------- Dynamic allocation section ----------- */
void
alloc_axis_arrays(xgobidata *xg)
/*
 * Dynamically allocate arrays.
*/
{
  Cardinal nc = (Cardinal) xg->ncols;

  xg->nicelim = (lims *) XtMalloc(nc * sizeof(lims));
  xg->tickdelta = (float *) XtMalloc(nc * sizeof(float));
  xg->deci = (int *) XtMalloc(nc * sizeof(int));
}

void
free_axis_arrays(xgobidata *xg)
/*
 * Dynamically free arrays.
*/
{
  XtFree((XtPointer) xg->nicelim);
  XtFree((XtPointer) xg->tickdelta);
  XtFree((XtPointer) xg->deci);
}

/* --------- End of dynamic allocation section ----------- */

static void
make_axes(void)
/*
 * Calculate plane-coordinates of axes
*/
{
  float precis = PRECISION1;

/*   .(axes[0].x, axes[0].y
 *   |
 *   |
 *   |
 *   |
 *   |
 *   |
 *   .----------------. (axes[2].x, axes[2].y)
 * (axes[1].x, axes[1].y)
*/

  axes[2].x = (long)(precis + (0.1 * precis));
  axes[0].y = (long)(precis + (0.1 * precis));

}

void
convert_axes(tickinfo *tix, xgobidata *xg)
/*
 * convert plane-axes to screen-axes
*/
{
  int k;
  long ltmp;
  icoords tmin;

  ltmp = axes[2].x + xg->shift_wrld.x;
  xg->screen_axes[2].x = (int) ((ltmp * xg->is.x) >> EXP1);
  xg->screen_axes[2].x += xg->mid.x;

  /*
   * Note:  this is a subtraction though it's an addition for the points.
  */
  ltmp = axes[0].y - xg->shift_wrld.y;
  xg->screen_axes[0].y = (int) ((ltmp * xg->is.y) >> EXP1);
  xg->screen_axes[0].y += xg->mid.y;


/*
 * Find the screen value of the minimum x and y coordinates,
 * using the fact that the minimum value in world coordinates for
 * every column is -PRECISION1.
*/


  if (xg->is_xyplotting) {

    /*
     * Vertical axis
    */

    ltmp = (long) -PRECISION1 + xg->shift_wrld.x;
    tmin.x = (int) ((ltmp * xg->is.x) >> EXP1);
    tmin.x += xg->mid.x;

    if (tmin.x - tix->screen[0].x < 10)
      xg->screen_axes[0].x = xg->screen_axes[1].x = tix->screen[0].x - 10;
    else
      xg->screen_axes[0].x = xg->screen_axes[1].x = tix->screen[0].x;

    /*
     * Horizontal axis
    */

    ltmp = (long) -PRECISION1 - xg->shift_wrld.y;
    tmin.y = (int) ((ltmp * xg->is.y) >> EXP1);
    tmin.y += xg->mid.y;

    if (tix->screen[0].y - tmin.y < 10)
      xg->screen_axes[2].y = xg->screen_axes[1].y = tix->screen[0].y + 10;
    else
      xg->screen_axes[2].y = xg->screen_axes[1].y = tix->screen[0].y;

  } else if (xg->is_plotting1d) {

    int min, max;

    /*
     * Vertical axis:
     * Find the minimum x screen value of the data and subtract 10.
    */

    min = xg->screen[0].x ;
    for (k=1; k<xg->nrows_in_plot; k++)
      min = MIN(xg->screen[ xg->rows_in_plot[k] ].x, min);
    xg->screen_axes[0].x = xg->screen_axes[1].x = min - 10 ;

    /*
     * Horizontal axis:
     * Find the maximum y screen value of the data and add 10.
    */

    max = xg->screen[0].y ;
    for (k=1; k<xg->nrows_in_plot; k++)
      max = MAX(xg->screen[ xg->rows_in_plot[k] ].y, max);
    xg->screen_axes[1].y = xg->screen_axes[2].y = max + 10 ;
  }

}

int
set_deci(float tickdelt)
/*
 * Figure out how many decimal places each tickdelta contains
*/
{
  int j;
  int is_set;
  float deci;
  double ddelt = (double) tickdelt;

  j=0;
  is_set = 0;
  while (!is_set) {
    if (j == 10 ||
      (fabs(ddelt) > .1 &&
        ( fabs(ddelt - floor(ddelt)) < .0001 ||
          fabs(ddelt - ceil(ddelt)) < .0001 )))
    {
      deci = j;
      is_set = 1;
    }
    else
    {
      ddelt = 10.0 * ddelt;
      j++;
    }
  }
  return(deci);
}

void
number_length(float xy_tick, int *length)
/* find the length of the number */
{
  int j;
  int is_num;
  float tmpnum;

  j = 0;
  tmpnum = xy_tick;
  is_num = 0;
  while (!is_num) {
    if ((int)tmpnum == 0) {
      *length = j;
      is_num = 1;
    }
    else {
      tmpnum = tmpnum/10;
      j++;
    }
  }
}

int
plot_too_big(xgobidata *xg)
/*
 * Find out if the labels are too close to the left or the bottom
 * of the plotting window.
*/
{
  int shrink = 0;
/*
 * Make sure the code here matches add_x_axis() and add_y_axis() in
 * plot_once.c.
*/
  if ((xg->screen_axes[1].x - xg->maxwidth - 7) < 5)
    shrink = 1;

  if (shrink == 0) {
    if ((xg->screen_axes[1].y + 2*FONTHEIGHT(appdata.plotFont) + 14) >
        (xg->max.y - 5) )
    {
       shrink = 1;
    }
  }

/*
 * make sure the plot can't get <too> small.
*/
  if (xg->screen_axes[1].y - xg->screen_axes[0].y <= 20 ||
      xg->screen_axes[2].x - xg->screen_axes[1].x <= 20)
  {
       shrink = 0;
  }

  return(shrink);
}


void
extend_axes(int xindx, int yindx, tickinfo *tix, xgobidata *xg)
/*
 * If the last tick is larger than the end of an axis, then extend the axis.
*/
{
  int k;

  Boolean add_vertical_axis = xg->is_xyplotting ||
                         (xg->is_plotting1d && xg->plot1d_vars.y != -1);
  Boolean add_horizontal_axis = xg->is_xyplotting ||
                         (xg->is_plotting1d && xg->plot1d_vars.x != -1);

  if (add_horizontal_axis) {
    k = tix->screen[tix->nticks[xindx]-1].x;
    if (xg->screen_axes[2].x < k)
      xg->screen_axes[2].x = k;
  }

  if (add_vertical_axis) {
    k = tix->screen[tix->nticks[yindx]-1].y;
    if (xg->screen_axes[0].y > k)
      xg->screen_axes[0].y = k;
  }
}

int
find_minindex(lims *limits, xgobidata *xg)
/*
 * Find the index of the column that has the smallest lim[].min.
*/
{
  int j, minindex;
  float ftmp;

  minindex = 0;
  ftmp = limits[0].min;
  for (j=0; j<xg->ncols_used; j++) {
    if (limits[j].min < ftmp) {
      ftmp = limits[j].min;
      minindex = j;
    }
  }
  return(minindex);
}

int
find_maxwidth(lims *limits, xgobidata *xg)
/*
 * Find the longest tick label for all of the variables, and
 * use maxwidth to determine whether the plot needs to be
 * repositioned.
*/
{
  int j;
  float tmp, ftmp;
  char str[50];
  int width, length, maxwidth;

  maxwidth = 0;
  for (j=0; j<xg->ncols_used; j++) {
    tmp = xg->nicelim[j].min;
    while (tmp + xg->tickdelta[j] <=
         limits[j].max + xg->tickdelta[j]/3)
    {
      tmp = tmp + xg->tickdelta[j];
    }

    if ((fabs((double) tmp) >= 10000. ||
       xg->deci[j] > 4) &&
       tmp != 0.0)
    {

      /* Use exponential notation */
      (void) number_length(tmp, &length);
      ftmp = tmp/ (float) pow((double)10, (double)(length-1));
      if (ftmp == (float)(int)ftmp)
        (void) sprintf (str, "%.0e", tmp);
      else
        (void) sprintf (str, "%.2e", tmp);
    }
    else {
      switch(xg->deci[j]) {
        case 0:
          (void) sprintf(str, "%3.0f", tmp);
          break;
        case 1:
          (void) sprintf(str, "%3.1f", tmp);
          break;
        case 2:
          (void) sprintf(str, "%3.2f", tmp);
          break;
        case 3:
          (void) sprintf(str, "%3.3f", tmp);
          break;
        case 4:
          (void) sprintf(str, "%3.4f", tmp);
          break;
      }
    }
    width = XTextWidth(appdata.plotFont, str, strlen(str));
    if (width > maxwidth)
      maxwidth = width;
  }
  return(maxwidth);
}


float
NiceValue(float x)
{
  float fvalue;

  if (x <= 0)
    fvalue = 0.0;
  else
  {
    double dx = (double) x;
    double dilx, lx, vv1, vv2, vv3, vv4;

    lx = log(dx) / log(10.0);
    dilx = floor(lx);

    vv1 = pow(10.0, dilx);
    vv2 = vv1 * 2.0;
    vv3 = vv1 * 5.0;
    vv4 = vv1 * 10.0;

    if ((fabs(dx - vv1) < fabs(dx - vv2))
     && (fabs(dx - vv1) < fabs(dx - vv3))
     && (fabs(dx - vv1) < fabs(dx - vv4)))
      fvalue = (float) vv1;

    else if ((fabs(dx - vv2) < fabs(dx - vv3))
        && (fabs(dx - vv2) < fabs(dx - vv4)))
      fvalue = (float) vv2;

    else if (fabs(dx - vv3) < fabs(dx - vv4))
      fvalue = (float) vv3;

    else
      fvalue = (float) vv4;
  }

  return(fvalue);
}

void
SetNiceRange(int j, xgobidata *xg)
/*
 * ticks.nticks[]: number of ticks used by each column
 * tickdelta: increment between subsequent ticks for each column
*/
{
  float nicearg;

  xg->ticks.nticks[j] = DEFAULTNTICKS;
  nicearg = (xg->nicelim[j].max - xg->nicelim[j].min) /
               (float) (xg->ticks.nticks[j] - 1) ;
  xg->tickdelta[j] = NiceValue( nicearg );

  xg->nicelim[j].min = (float) floor( (double) (
    xg->nicelim[j].min / xg->tickdelta[j])) * xg->tickdelta[j];
  xg->nicelim[j].max = (float) ceil( (double) (
    xg->nicelim[j].max / xg->tickdelta[j])) * xg->tickdelta[j];

  /* add .01 for rounding */
  xg->ticks.nticks[j] = 1 +
    (.01 + (xg->nicelim[j].max - xg->nicelim[j].min) / xg->tickdelta[j]);

  /*
   * Initialize ticks0.nticks[]
  */
  xg->ticks0.nticks[j] = xg->ticks.nticks[j];
}

void
generate_ticks(int xindx, int yindx, lims *limits, tickinfo *tix, xgobidata *xg)
/*
 *  Generate a vector of x ticks and y ticks.
*/
{
  int j;
  Boolean add_vertical_axis = xg->is_xyplotting ||
                         (xg->is_plotting1d && xg->plot1d_vars.y != -1);
  Boolean add_horizontal_axis = xg->is_xyplotting ||
                         (xg->is_plotting1d && xg->plot1d_vars.x != -1);
  

  if (add_horizontal_axis) {
    tix->xticks[0] = xg->nicelim[xindx].min;
    j = 1;
    while (tix->xticks[j-1] + xg->tickdelta[xindx] <=
       limits[xindx].max + xg->tickdelta[xindx]/2)
    {
      tix->xticks[j] = tix->xticks[j-1] + xg->tickdelta[xindx];
      if (j++ == NTICKS-1)
      {
        fprintf(stderr, "warning: (generate_ticks) too many x ticks\n");
        break;
      }
    }
    tix->nticks[xindx] = j;
  }

  if (add_vertical_axis) {
    tix->yticks[0] = xg->nicelim[yindx].min;
    j = 1;
    while (tix->yticks[j-1] + xg->tickdelta[yindx] <=
         limits[yindx].max + xg->tickdelta[yindx]/2)
    {
      tix->yticks[j] = tix->yticks[j-1] + xg->tickdelta[yindx];
      if (j++ == NTICKS-1)
      {
        fprintf(stderr, "warning: (generate_ticks) too many y ticks\n");
        break;
      }
    }
    tix->nticks[yindx] = j;
  }
}

void
scale_ticks(int xindx, int yindx, lims *limits, tickinfo *tix, xgobidata *xg)
/*
 * calculate plane-coordinates of ticks
*/
{
  int j;
  float tmpf;
  float precis = PRECISION1;
  Boolean add_vertical_axis = xg->is_xyplotting ||
                         (xg->is_plotting1d && xg->plot1d_vars.y != -1);
  Boolean add_horizontal_axis = xg->is_xyplotting ||
                         (xg->is_plotting1d && xg->plot1d_vars.x != -1);

  if (add_horizontal_axis) {
    for (j=0; j<tix->nticks[xindx]; j++) {
      tmpf = -1.0 + 2.0*(tix->xticks[j] - limits[xindx].min)
        /(limits[xindx].max - limits[xindx].min);
      tix->plane[j].x = (long) (precis * tmpf);
    }
  }

  if (add_vertical_axis) {
    for (j=0; j<tix->nticks[yindx]; j++) {
      tmpf = -1.0 + 2.0*(tix->yticks[j] - limits[yindx].min)
        /(limits[yindx].max - limits[yindx].min);
      tix->plane[j].y = (long) (precis * tmpf);
    }
  }
}

void
convert_ticks(int xindx, int yindx, tickinfo *tix, xgobidata *xg)
/*
 * Convert plane-ticks to screen-ticks
*/
{
  int j;
  long ltmp;
  Boolean add_vertical_axis = xg->is_xyplotting ||
                         (xg->is_plotting1d && xg->plot1d_vars.y != -1);
  Boolean add_horizontal_axis = xg->is_xyplotting ||
                         (xg->is_plotting1d && xg->plot1d_vars.x != -1);

  if (add_horizontal_axis) {
    for (j=0; j<tix->nticks[xindx]; j++) {
      ltmp = tix->plane[j].x + xg->shift_wrld.x;
      tix->screen[j].x = (int) ((ltmp * xg->is.x) >> EXP1);
      tix->screen[j].x += xg->mid.x;
    }
  }

  if (add_vertical_axis) {
    for (j=0; j<tix->nticks[yindx]; j++) {
      /*
       *  Note:  this is a subtraction though it's an addition for the points.
      */
      ltmp = tix->plane[j].y - xg->shift_wrld.y;
      tix->screen[j].y = (int) ((ltmp * xg->is.y) >> EXP1);
      tix->screen[j].y += xg->mid.y;
    }
  }
}

void
init_axes(xgobidata *xg, Boolean firsttime)
/*
 * When reading in data, use the limits determined using min-max
 * standardization method to define the axes for xy plotting.
*/
{
  int j;

  if (firsttime)
    make_axes();

  for (j=0; j<xg->ncols_used; j++)
  {
    /*
     * Find the nicelim values, which are chosen to make reasonable
     * placement of the tick marks.
    */
    xg->nicelim[j].min = xg->lim0[j].min;
    xg->nicelim[j].max = xg->lim0[j].max;
    SetNiceRange(j, xg);
    xg->deci[j] = set_deci(xg->tickdelta[j]);
  }
  xg->minindex = find_minindex(xg->lim0, xg);
  generate_ticks(xg->minindex, xg->minindex, xg->lim0, &xg->ticks0, xg);
  scale_ticks(xg->minindex, xg->minindex, xg->lim0, &xg->ticks0, xg);

  /* This part is redone when shift and scale change */
  convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
  convert_axes(&xg->ticks0, xg);

  xg->maxwidth = find_maxwidth(xg->lim0, xg);
  while (plot_too_big(xg))
  {
    xg->scale0.x = .97 * xg->scale0.x;
    xg->scale.x = xg->scale0.x;
    xg->scale0.y = .97 * xg->scale0.y;
    xg->scale.y = xg->scale0.y;

    plane_to_screen(xg);
    convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
  }
}

void
init_tickdelta(xgobidata *xg)
{
/*
 * Initialize the limits and tickdelta for the current variables.
*/
  int j;

  for (j=0; j<xg->ncols_used; j++)
  {
    xg->nicelim[j].min = xg->lim[j].min;
    xg->nicelim[j].max = xg->lim[j].max;
    SetNiceRange(j, xg);
    xg->deci[j] = set_deci(xg->tickdelta[j]);
  }
}

void
init_ticks(icoords *vars, xgobidata *xg)
{
/*
 * Set the ticks for the current standardization method.
 *
 * Construct current ticks.
 * This is redone when choosing new variables or when
 * re-entering xyplotting.
*/
  /* use minindex and ticks0 to set the axes */
  convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
  convert_axes(&xg->ticks0, xg);
  /* now generate and scale current ticks */
  generate_ticks(vars->x, vars->y, xg->lim, &xg->ticks, xg);
  scale_ticks(vars->x, vars->y, xg->lim, &xg->ticks, xg);
  convert_ticks(vars->x, vars->y, &xg->ticks, xg);
  extend_axes(vars->x, vars->y, &xg->ticks, xg);
}

/* For drawing ticks and axes; used in plot_once() */

int
check_x_axis(xgobidata *xg, int xvar, tickinfo *tix)
/*
 * Check whether the first ticks are to the left of the perpendicular axis.
 * If so, they will simply not be drawn.
*/
{
  int i;
  int xt = 0;

  for (i=0; i<tix->nticks[xvar]; i++)
    if (xg->screen_axes[1].x > tix->screen[i].x)
      xt++;
  return(xt);
}

int
check_y_axis(xgobidata *xg, int yvar, tickinfo *tix)
/*
 * Check whether the first ticks are to the left of the perpendicular axis.
 * If so, they will simply not be drawn.
*/
{
  int i;
  int yt = 0;

  for (i=0; i<tix->nticks[yvar]; i++)
    if (xg->screen_axes[1].y < tix->screen[i].y)
      yt = yt + 1;
  return(yt);
}

void
add_x_axis(xgobidata *xg, icoords *vars, int xstart, tickinfo *tix)
{
  int j, src, dest, right_edge, modval;
  Boolean modval_ok;
  char str[50];
  XPoint vpnts[2];
  int nticks = tix->nticks[vars->x];
  int *width;
  char **lbls;

  lbls = (char **) XtMalloc((Cardinal) nticks * sizeof (char *));
  for (j=0; j<nticks; j++)
    lbls[j] = (char *) XtMalloc(64 * sizeof(char));
  width = (int *) XtMalloc((Cardinal) nticks * sizeof (int));

  /*
   * Draw x axis
  */
  for (j=0; j<2; j++) {
    vpnts[j].x = xg->screen_axes[j+1].x;
    vpnts[j].y = xg->screen_axes[j+1].y;
  }
  XDrawLines(display, xg->pixmap0, copy_GC, vpnts, 2, CoordModeOrigin);

  /*
   * Set modval, the interval for plotting tick labels, while
   * copying tick labels into lbls
  */

  for (j=xstart; j<nticks; j++) {
    find_tick_label(xg, vars->x, j, tix->xticks, str);
    for (src=0, dest=0; src<(INT(strlen(str))+1); src++)
      if (str[src] != ' ')
        str[dest++] = str[src];
    width[j] = XTextWidth(appdata.plotFont, str, strlen(str));
    strcpy(lbls[j], str);
  }

  modval = 1;
  while (true) {
    modval_ok = true;
    right_edge = 0;
    for (j=xstart; j<nticks; j++) {
      if (j % modval == 0) {
        if (tix->screen[j].x - width[j]/2 <= right_edge+2) { /* 2: margin */
          modval++;
          modval_ok = false;
          break;
        } else right_edge = tix->screen[j].x + width[j]/2;
      }
    }

    if (j == xstart || modval_ok) break;
  }

  for (j=xstart; j<nticks; j++) {
    if (j % modval == 0) {
      XDrawImageString(display, xg->pixmap0, copy_GC,
        tix->screen[j].x - width[j]/2,
        xg->screen_axes[1].y + FONTHEIGHT(appdata.plotFont)+7,
        lbls[j], strlen(lbls[j]));
      XDrawLine(display, xg->pixmap0, copy_GC,
        tix->screen[j].x, xg->screen_axes[1].y,
        tix->screen[j].x, xg->screen_axes[1].y+6);
    } else {
      XDrawLine(display, xg->pixmap0, copy_GC,
        tix->screen[j].x, xg->screen_axes[1].y,
        tix->screen[j].x, xg->screen_axes[1].y+3);
    }
  }

  /*
   * Draw X Axis label
  */
  strcpy(str, xg->collab_tform2[vars->x]);
  XDrawImageString(display, xg->pixmap0, copy_GC,
    xg->screen_axes[1].x +
     (xg->screen_axes[2].x - xg->screen_axes[1].x)/2 - 
       XTextWidth(appdata.plotFont, str, strlen(str))/2,
    xg->screen_axes[1].y + 2*FONTHEIGHT(appdata.plotFont) + 14,
    str,
    strlen(str));

  XtFree((XtPointer) width);
  for (j=0; j<nticks; j++)
    XtFree((XtPointer) lbls[j]);
  XtFree((XtPointer) lbls);
  return;
}

void
add_y_axis(xgobidata *xg, icoords *vars, int ystart, tickinfo *tix)
{
  int j, top_edge, modval, height, src, dest;
  Boolean modval_ok;
  char str[50];
  XPoint vpnts[2];
  /*Boolean plot1d_vertically = (xg->plot1d_vars.y != -1);*/

  int nticks = tix->nticks[vars->y];
  /*int nxticks = (xg->is_plotting1d) ? 0 : tix->nticks[vars->x];*/
  char **lbls;

  lbls = (char **) XtMalloc((Cardinal) nticks * sizeof (char *));
  for (j=0; j<nticks; j++)
    lbls[j] = (char *) XtMalloc(64 * sizeof(char));

/*
 * Draw y axis
*/
  for (j=0; j<2; j++) {
    vpnts[j].x = xg->screen_axes[j].x;
    vpnts[j].y = xg->screen_axes[j].y;
  }
  XDrawLines(display, xg->pixmap0, copy_GC, vpnts, 2, CoordModeOrigin);

  /*
   * Set modval, the interval for plotting tick labels, while
   * copying tick labels into lbls
  */

  for (j=ystart; j<nticks; j++) {
    find_tick_label(xg, vars->y, j, tix->yticks, str);
    for (src=0, dest=0; src<(INT(strlen(str))+1); src++)
      if (str[src] != ' ')
        str[dest++] = str[src];
    strcpy(lbls[j], str);
  }

  height = FONTHEIGHT(appdata.plotFont); 
  modval = 1;
  while (true) {
    modval_ok = true;
    top_edge = xg->plotsize.height;
    for (j=ystart; j<nticks; j++) {
      if (j % modval == 0) {
        if (tix->screen[j].y + height/2 >= top_edge+2) { /* 2: margin */
          modval++;
          modval_ok = false;
          break;
        } else top_edge = tix->screen[j].y - height/2;
      }
    }

    if (j == ystart || modval_ok) break;
  }

  height = (appdata.plotFont)->max_bounds.ascent; 
  for (j=ystart; j<nticks; j++) {
    if (j % modval == 0) {
      XDrawImageString(display, xg->pixmap0, copy_GC,
        xg->screen_axes[1].x -
        XTextWidth(appdata.plotFont, lbls[j], strlen(lbls[j])) - 7,
        tix->screen[j].y + height/2,
        lbls[j], strlen(lbls[j]));

      XDrawLine(display, xg->pixmap0, copy_GC,
        xg->screen_axes[1].x,   tix->screen[j].y,
        xg->screen_axes[1].x-6, tix->screen[j].y);
    } else {
      XDrawLine(display, xg->pixmap0, copy_GC,
        xg->screen_axes[1].x,   tix->screen[j].y,
        xg->screen_axes[1].x-3, tix->screen[j].y);
    }
  }

  /*
   * Draw Y Axis label
  */
  strcpy(str, xg->collab_tform2[vars->y]);
  XDrawImageString(display, xg->pixmap0, copy_GC,
    xg->screen_axes[0].x - strlen(str)/2 - 6,
    xg->screen_axes[0].y - 10,
    str,
    INT(strlen(str)));

  for (j=0; j<nticks; j++)
    XtFree((XtPointer) lbls[j]);
  XtFree((XtPointer) lbls);
  return;
}


void
add_y_gridlines(xgobidata *xg, int jvar, int ystart, tickinfo *tix)
{
/*
 * Add grid lines to the plot.  Each mode will have to be handled
 * separately.
*/
  int j;

  /*
   * Draw y grid lines
  */
  for (j=ystart; j<tix->nticks[jvar]; j++) {
    XDrawLine(display, xg->pixmap0, copy_GC,
      xg->screen_axes[1].x, tix->screen[j].y,
      xg->screen_axes[2].x, tix->screen[j].y);
  }
}

void
add_x_gridlines(xgobidata *xg, int jvar, int xstart, tickinfo *tix)
{
/*
 * Add grid lines to the plot.  Each mode will have to be handled
 * separately.
*/
  int j;

  /*
   * Draw x grid lines
  */
  for (j=xstart; j<tix->nticks[jvar]; j++) {
    XDrawLine(display, xg->pixmap0, copy_GC,
      tix->screen[j].x, xg->screen_axes[0].y,
      tix->screen[j].x, xg->screen_axes[1].y);
  }
}
