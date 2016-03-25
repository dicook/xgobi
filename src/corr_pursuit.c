/* corr_pursuit.c */
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
#include <stdlib.h>
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

/* static char *index_names[NUM_INDICES]; */
Widget cp_plot_shell, cp_plot_form, cp_plot_box0, cp_plot_box1;
Widget cp_plot_wksp, cp_panel[1];
Widget cp_plot_cmd[3], cp_label[1];
static Window cp_plot_window;
static Pixmap cp_plot_pixmap;
static WidgetSize cp_wksp;

static float Ihat;
float *didx, *didy; /* derivatives filled 0-number of vars */
static float cp_index_max = 0.1;
static float cp_index_min = 0.;
static int count = 0;
static XRectangle *cp_index_rect;
static float *cp_index_array;
static XPoint *cp_index_pts;
extern GC tour_pp_GC, tour_pp_clear_GC;
static int nnew_bases = 0;
static int *new_basis_lines;
static int *optimz_circs; /* optimz circles */
static int noptimz_circs = 0;
static Boolean is_cp_lines = 1;
static Boolean is_cp_points = 0;
static unsigned int cp_line_width = 2;
static long **tv2;
static float *ax, *ay;
static XPoint *xpts;
static int cp_bw_points = 2;
static int max_cp_storage;

#define CP_PLOTMARGIN_TOP 0.05
#define LEN_NEW_BASIS_LINE 10
#define OPTIMZ_CIRC_WIDTH 20
#define OPTIMZ_CIRC_HEIGHT 20
#define OPTIMZ_CIRC_ANG1 245*64
#define OPTIMZ_CIRC_ANG2 50*64
#define OPTIMZ_OFFSET 10
#define CP_PANEL cp_panel[0]
#define MAX_NOPTIMZ_CIRCS 20 /* optimz circles */
#define FUDGE_FACTOR (xg->numvars_t+1)*2/7
#define CP_INDEX_LABEL cp_label[0]
#define CP_OPTIMZ cp_plot_cmd[0]

#define NULL_BTN cp_plot_cmd[0]

void
alloc_cp(xgobidata *xg)
{
  unsigned int nx = (unsigned int) xg->ncorr_xvars;
  unsigned int ny = (unsigned int) xg->ncorr_yvars;
  int i;

  didx = (float *) XtMalloc(nx*sizeof(float));
  didy = (float *) XtMalloc(ny*sizeof(float));
 
  ax = (float *) XtMalloc(nx*sizeof(float));
  ay = (float *) XtMalloc(ny*sizeof(float));
 
  tv2 = (long **) XtMalloc(
    (unsigned int) 2*sizeof(long *));
  for (i=0; i<2; i++)
    tv2[i] = (long *) XtMalloc(
      (unsigned int) xg->nrows*sizeof(long));

  xpts = (XPoint *) XtMalloc(
    (unsigned int) xg->nrows*sizeof(XPoint));
}

void
free_cp(xgobidata *xg)
{
  int i;

  XtFree((XtPointer) didx);
  XtFree((XtPointer) didy);
  XtFree((XtPointer) xpts);
  for (i=0; i<2; i++)
    XtFree((XtPointer) tv2[i]);
  XtFree((XtPointer) tv2);
}

void
reset_cp_plot()
{
  count = 0;
  nnew_bases = 0;
  noptimz_circs = 0;
}

void
cp_plot(xgobidata *xg, float cp_index_val, int restart, int ind_new_basis,
int resized)
{
  short x, y;
  int x1, y1, x2, y2;
  int i;
  char string[16];

  if (xg->cp_recalc_max_min)
  {
    cp_index_max = cp_index_val;
    cp_index_min = cp_index_val - (0.001 * cp_index_val);
    xg->cp_recalc_max_min = False;
  }
  if (restart && !resized)
    count = 0;
  else if (resized && !restart)  /* just need to redraw */
  {
    for (i=0; i<count; i++)
    {
      /* adjust rectangle to center of index point */
      cp_index_rect[i].y = cp_wksp.height -
        (1.-CP_PLOTMARGIN_TOP) * cp_wksp.height *
        (cp_index_array[i] - cp_index_min) /
        (cp_index_max - cp_index_min) - 1 ;

      cp_index_pts[i].y = cp_wksp.height -
        (1.-CP_PLOTMARGIN_TOP)*cp_wksp.height*
        (cp_index_array[i] - cp_index_min) /
        (cp_index_max - cp_index_min);
    }
    XFillRectangle(display, cp_plot_pixmap, tour_pp_clear_GC,
      0, 0, cp_wksp.width, cp_wksp.height);

    x1 = xg->xaxis_indent;
    x2 = xg->xaxis_indent;
    y1 = (int)(CP_PLOTMARGIN_TOP*FLOAT(cp_wksp.height));
    y2 = (int)(cp_wksp.height);
    XDrawLine(display, cp_plot_pixmap, tour_pp_GC, x1, y1, x2, y2);
               /*vertical axis*/
    sprintf(string, "%.2e", cp_index_max);
    XDrawString(display, cp_plot_pixmap, tour_pp_GC,
      5*ASCII_TEXT_BORDER_WIDTH, (y1+10),
      string, strlen(string));
    sprintf(string,"%.2e", cp_index_min);
    XDrawString(display, cp_plot_pixmap, tour_pp_GC,
      5*ASCII_TEXT_BORDER_WIDTH, y2, string,
      strlen(string));
    y1 = (int)(cp_wksp.height);
    x2 = (int) ((1.-CP_PLOTMARGIN_TOP) *
      (FLOAT(cp_wksp.width) - FLOAT(xg->xaxis_indent))) +
      xg->xaxis_indent;
    XDrawLine(display, cp_plot_pixmap, tour_pp_GC, x1, y1, x2, y2);
             /*horizontal axis*/

    y1 = y2 - LEN_NEW_BASIS_LINE;
    for (i=0; i<nnew_bases; i++)
      XDrawLine(display, cp_plot_pixmap, tour_pp_GC,
        new_basis_lines[i], y1, new_basis_lines[i], y2);

    for (i=0; i<noptimz_circs; i++)
      XFillArc(display, cp_plot_pixmap, tour_pp_GC,
        optimz_circs[i], y1, OPTIMZ_CIRC_WIDTH, OPTIMZ_CIRC_HEIGHT,
        OPTIMZ_CIRC_ANG1, OPTIMZ_CIRC_ANG2);

    if (is_cp_points)
      XDrawRectangles(display, cp_plot_pixmap, tour_pp_GC,
        cp_index_rect, count);
    if (is_cp_lines)
    {
      if (count > 0)
      {
        XSetLineAttributes(display, tour_pp_GC, cp_line_width,
          LineSolid, CapRound, JoinBevel);
        XDrawLines(display, cp_plot_pixmap, tour_pp_GC,
          cp_index_pts, count, CoordModeOrigin);
          /* count has
           * already been updated by 1 when we get into
           * this part, so there is no need to add 1
           * to count.*/
          XSetLineAttributes(display, tour_pp_GC, 1, LineSolid,
              CapRound, JoinMiter);
      }
    }

    XCopyArea(display, cp_plot_pixmap, cp_plot_window, tour_pp_GC,
      0, 0, cp_wksp.width, cp_wksp.height, 0, 0);
  }
  else  /* next point */
  {
    XFillRectangle(display, cp_plot_pixmap, tour_pp_clear_GC,
      0, 0, cp_wksp.width, cp_wksp.height);

/* rescale all projection index points if max or min value exceeded */
    if (cp_index_val > cp_index_max)
    {
      cp_index_max = cp_index_val;
      for (i=0; i<count; i++)
      {
        /* adjust rectangle to center of index point */
        cp_index_rect[i].y = cp_wksp.height -
          (1.-CP_PLOTMARGIN_TOP)*cp_wksp.height*
          (cp_index_array[i] - cp_index_min) /
          (cp_index_max - cp_index_min) - 1;

        cp_index_pts[i].y = cp_wksp.height -
          (1.-CP_PLOTMARGIN_TOP)*cp_wksp.height*
          (cp_index_array[i] - cp_index_min) /
          (cp_index_max - cp_index_min);
      }
    }

    if (cp_index_val < cp_index_min)
    {
      cp_index_min = cp_index_val;
      for (i=0; i<count; i++)
      {
        /* adjust rectangle to center of index point */
        cp_index_rect[i].y = cp_wksp.height -
          (1.-CP_PLOTMARGIN_TOP)*cp_wksp.height*
          (cp_index_array[i] - cp_index_min) /
          (cp_index_max - cp_index_min) - 1;

        cp_index_pts[i].y = cp_wksp.height -
          (1.-CP_PLOTMARGIN_TOP)*cp_wksp.height*
          (cp_index_array[i] - cp_index_min) /
          (cp_index_max - cp_index_min);
      }
    }

/* draw axes */
    x1 = xg->xaxis_indent;
    x2 = xg->xaxis_indent;
    y1 = (int)(CP_PLOTMARGIN_TOP*FLOAT(cp_wksp.height));
    y2 = (int)(cp_wksp.height);
    XDrawLine(display, cp_plot_pixmap, tour_pp_GC, x1, y1, x2, y2);
               /*vertical axis*/
    sprintf(string, "%.2e", cp_index_max);
    XDrawString(display, cp_plot_pixmap, tour_pp_GC,
      5*ASCII_TEXT_BORDER_WIDTH, (y1+10),
      string, strlen(string));
    sprintf(string,"%.2e", cp_index_min);
    XDrawString(display, cp_plot_pixmap, tour_pp_GC,
      5*ASCII_TEXT_BORDER_WIDTH, y2, string,
      strlen(string));
    y1 = (int)(cp_wksp.height);
    x2 = (int)((1.-CP_PLOTMARGIN_TOP) *
      (FLOAT(cp_wksp.width) - FLOAT(xg->xaxis_indent))) +
      xg->xaxis_indent;
    XDrawLine(display, cp_plot_pixmap, tour_pp_GC, x1, y1, x2, y2);
             /*horizontal axis*/
    x = count*cp_bw_points + xg->xaxis_indent;

/* draw line indicating a direction (new basis) change */
    if (ind_new_basis)
      new_basis_lines[nnew_bases++] = x;

    y1 = y2 - LEN_NEW_BASIS_LINE;
    for (i=0; i<nnew_bases; i++)
      XDrawLine(display, cp_plot_pixmap, tour_pp_GC,
        new_basis_lines[i], y1, new_basis_lines[i], y2);

/* draw circle indicating when optimz turned on */
    for (i=0; i<noptimz_circs; i++)
      XFillArc(display, cp_plot_pixmap, tour_pp_GC,
        optimz_circs[i], y1, OPTIMZ_CIRC_WIDTH, OPTIMZ_CIRC_HEIGHT,
        OPTIMZ_CIRC_ANG1, OPTIMZ_CIRC_ANG2);

/* calculate plot y-value of the projection index */
    y = cp_wksp.height -
      (1.-CP_PLOTMARGIN_TOP)*cp_wksp.height*
      (cp_index_val - cp_index_min) /
      (cp_index_max - cp_index_min);

/*
 * if the number of points is greater than the max plotting width
 * drop off the first and shift all points back one space.
*/
    if (count*cp_bw_points >= max_cp_storage)
    {
      for (i=0; i<(max_cp_storage-1); i++)
      {
        cp_index_array[i] = cp_index_array[i+1];
        cp_index_rect[i].y = cp_index_rect[i+1].y;
        cp_index_pts[i].y = cp_index_pts[i+1].y;
      }
      count--;
      cp_index_array[count] = cp_index_val;
      cp_index_rect[count].y = (short)(y-1); /* center rectangle */
      cp_index_pts[count].y = (short)y;
    /* need to put in shifting bitmaps and new_basis lines backwards */
      for (i=0; i<nnew_bases; i++)
        new_basis_lines[i] -= cp_bw_points;
      if (new_basis_lines[0] < xg->xaxis_indent)
      {
        for (i=0; i<nnew_bases-1; i++)
          new_basis_lines[i] = new_basis_lines[i+1];
        nnew_bases--;
      }
      for (i=0; i<noptimz_circs; i++)
        optimz_circs[i] -= cp_bw_points;
      if (noptimz_circs &&
        optimz_circs[0] < (xg->xaxis_indent - OPTIMZ_CIRC_WIDTH/2))
      {
        for (i=0; i<noptimz_circs-1; i++)
          optimz_circs[i] = optimz_circs[i+1];
        noptimz_circs--;
      }
    }
    else
    {
      cp_index_array[count] = cp_index_val;
      cp_index_rect[count].x = (short)(x-1); /* center rectangle */
      cp_index_rect[count].y = (short)(y-1); /* center rectangle */
      cp_index_rect[count].width = 2;
      cp_index_rect[count].height = 2;
      cp_index_pts[count].x = (short)x;
      cp_index_pts[count].y = (short)y;
    }
    if (is_cp_points)
      XDrawRectangles(display, cp_plot_pixmap, tour_pp_GC,
        cp_index_rect, count+1);
    if (is_cp_lines)
    {
      if (count > 0)
      {
        XSetLineAttributes(display, tour_pp_GC, cp_line_width,
          LineSolid, CapRound, JoinBevel);
        XDrawLines(display, cp_plot_pixmap, tour_pp_GC,
          cp_index_pts, count+1, CoordModeOrigin);
        XSetLineAttributes(display, tour_pp_GC, 1, LineSolid,
          CapRound, JoinMiter);
      }
    }

    XCopyArea(display, cp_plot_pixmap, cp_plot_window, tour_pp_GC,
      0, 0, cp_wksp.width, cp_wksp.height, 0, 0);
    count++;
  }
}

void
cp_update_label(float cp_index_val)
{
  char str[16];

  sprintf(str, "CPIndx %.2e", cp_index_val);
  XtVaSetValues(CP_INDEX_LABEL, XtNstring, (String) str, NULL);
}

/* pursuit index */
void
cp_index(xgobidata *xg, int restart_indic, int newbase_indic)
{
  get_corr_index(xg, &Ihat, didx, didy);
  cp_update_label(Ihat);
  cp_plot(xg,Ihat,restart_indic,newbase_indic,0);
}

void
make_cp_pixmap(void)
{
  /* depth is a global variable */
  cp_plot_pixmap = XCreatePixmap(display, cp_plot_window,
    cp_wksp.width, cp_wksp.height, depth);
  XFillRectangle(display, cp_plot_pixmap, tour_pp_clear_GC,
    0, 0, cp_wksp.width, cp_wksp.height);
}

void
reset_corr_optimz(Boolean set_val)
{
  XtVaSetValues(CP_OPTIMZ, XtNstate, set_val, NULL);
}

/*
void
set_sens_corr_optimz(sens)
  Boolean sens;
{
  XtVaSetValues(CP_OPTIMZ, XtNsensitive, sens, NULL);
}
*/

/* ARGSUSED */
XtEventHandler
cpresize_cback(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
/*
 * If the window is resized, recalculate the size of the plot window, and
 * then, if points are plotted, clear and redraw.  The variable selection
 * windows also need to be redrawn.
*/
{
  float ftmp;
  WidgetSize cp_wksp_old;
  int i = 0;
  int strt_val;

  cp_wksp_old.width = cp_wksp.width;
  cp_wksp_old.height = cp_wksp.height;
  XtVaGetValues(cp_plot_wksp,
    XtNwidth, &cp_wksp.width,
    XtNheight, &cp_wksp.height, NULL);
  if ((cp_wksp_old.width != cp_wksp.width) ||
    (cp_wksp_old.height != cp_wksp.height))
  {
    XFreePixmap(display, cp_plot_pixmap);
    make_cp_pixmap();
    ftmp = (float) ((int) cp_wksp.width - (int) xg->xaxis_indent);
    max_cp_storage = (int) ((1.-CP_PLOTMARGIN_TOP) * ftmp);

    count--; /* decrement it to reflect actual number of points
                in arrays */ 

    if (count*cp_bw_points > max_cp_storage)
    {
      strt_val = count - (int)(max_cp_storage/cp_bw_points);
      for (i=0; i<(int)(max_cp_storage/cp_bw_points); i++)
      {
        cp_index_array[i] = cp_index_array[i+strt_val];
        cp_index_rect[i].y = cp_index_rect[i+strt_val].y;
        cp_index_pts[i].y = cp_index_pts[i+strt_val].y;
      }
      if (nnew_bases > 0)
      {
        for (i=0; i<nnew_bases; i++)
          new_basis_lines[i] -= (count*cp_bw_points - max_cp_storage);
        while (new_basis_lines[0] < xg->xaxis_indent)
        {
          for (i=0; i<nnew_bases-1; i++)
            new_basis_lines[i] = new_basis_lines[i+1];
          nnew_bases--;
        }
      }
      if (noptimz_circs > 0)
      {
        for (i=0; i<noptimz_circs; i++)
          optimz_circs[i] -= (count*cp_bw_points - max_cp_storage);
        while ((noptimz_circs) && 
          optimz_circs[0] < (xg->xaxis_indent - OPTIMZ_CIRC_WIDTH/2))
        {
          for (i=0; i<noptimz_circs-1; i++)
            optimz_circs[i] = optimz_circs[i+1];
          noptimz_circs--;
        }
      }
      count = max_cp_storage / cp_bw_points;
    }

/*
 * allocating these to be max_pp_storage+1 in size since it
 * seems to fix a funny allocation bug that purify finds.
*/
    cp_index_array = (float *) XtRealloc((char *) cp_index_array,
      (unsigned int) (max_cp_storage+1) * sizeof(float));
    cp_index_rect = (XRectangle *) XtRealloc((char *) cp_index_rect,
      (unsigned int) (max_cp_storage+1) * sizeof(XRectangle));
    cp_index_pts = (XPoint *) XtRealloc((char *) cp_index_pts,
      (unsigned int) (max_cp_storage+1) * sizeof(XPoint));
    new_basis_lines = (int *) XtRealloc((char *) new_basis_lines,
      (unsigned int) (max_cp_storage+1) * sizeof(int));

    /*    cp_plot(xg, abs(Ihat), 0, 0, 1);* bugfix - Sep '98*/
    cp_plot(xg, Ihat, 0, 0, 1);/* bugfix - Sep '98*/
  }
}

/* ARGSUSED */
XtEventHandler
cpexpose_cback(Widget w, XtPointer client_data, XEvent *evnt, Boolean *cont)
/*
 * If the plot window is fully or partially exposed, clear and redraw.
*/
{
  if (evnt->xexpose.count == 0)  /* Compress expose events */
    XCopyArea(display, cp_plot_pixmap, cp_plot_window, tour_pp_GC,
      0, 0, cp_wksp.width, cp_wksp.height, 0, 0);
}

/* ARGSUSED */
XtCallbackProc
null_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
}

int
cp_derivs_equal_zero(xgobidata *xg)
{
  int j;
  float tmpf, tol = 0.0000001;
  int retn_val = 1;

  tmpf = 0.;
  for (j=0; j<xg->ncorr_xvars; j++)
    tmpf += (didx[j]*didx[j]);
  tmpf /= ((xg->ncorr_xvars+xg->ncorr_yvars)*(cp_index_max - cp_index_min));
  if (tmpf > tol)
    retn_val = 0;
  else
  {
    tmpf = 0.;
    for (j=0; j<xg->ncorr_yvars; j++)
      tmpf += (didy[j]*didy[j]);
    tmpf /= ((xg->ncorr_xvars+xg->ncorr_yvars)*(cp_index_max - cp_index_min));
    if (tmpf > tol)
      retn_val = 0;
  }
  return(retn_val);
}

void
write_msg_in_cp_window(void)
{
  char str[64];
  int strlength;
/*
 * If the font is big, the string is right-truncated.
 * Should be like this:  draw it flush right unless that
 * causes left truncation; in that case, draw it flush left.
 * Consider calling this in expose routine?
*/
  sprintf(str,"%s","Derivatives ZERO: Turn Optimz Off to Continue");
  strlength = strlen(str);
  XDrawString(display, cp_plot_pixmap, tour_pp_GC,
    (int) (cp_wksp.width - XTextWidth(appdata.font, str, strlength) - 70),
    40, str, strlength);
  XCopyArea(display, cp_plot_pixmap, cp_plot_window, tour_pp_GC,
    0, 0, cp_wksp.width, cp_wksp.height, 0, 0);
}

void
cp_dir(xgobidata *xg)
{
  int i, j;
  float tmpf1, tmpf2;
  float eps = 0.5;

/* call cp_index to update P's, and Rp's */
  cp_index(xg,0,1);
  if (cp_derivs_equal_zero(xg))
  {
    stop_corr_proc(xg);
    write_msg_in_cp_window();
  }
  else
  {
    if (xg->ncorr_xvars > 1)
    {
      for (i=0; i<xg->ncorr_xvars; i++)
        ax[i] = didx[i];
      tmpf1 = calc_norm(ax, xg->ncorr_xvars);
      for (i=0; i<xg->ncorr_xvars; i++)
        didx[i] = (float)((double)didx[i]/(double)tmpf1);
      for (i=0; i<xg->ncorr_xvars; i++)
        didx[i] = (float)((double)didx[i]*(double)eps);
      for (j=0; j<xg->ncorr_xvars; j++)
        xg->cu1[0][xg->corr_xvars[j]] = xg->cu[0][xg->corr_xvars[j]]
          + didx[j];
      norm(xg->cu1[0], xg->ncols_used);
    }
    if (xg->ncorr_yvars > 1)
    {
      for (i=0; i<xg->ncorr_yvars; i++)
        ay[i] = didy[i];
      tmpf2 = calc_norm(ay, xg->ncorr_yvars);
      tmpf2 *= tmpf2;
      tmpf1 = sqrt((double) (tmpf1+tmpf2));
      for (i=0; i<xg->ncorr_yvars; i++)
        didy[i] *= (eps/tmpf1);
      for (j=0; j<xg->ncorr_yvars; j++)
        xg->cu1[1][xg->corr_yvars[j]] = xg->cu[1][xg->corr_yvars[j]]
          + didy[j];
      norm(xg->cu1[1], xg->ncols_used);
    }

    init_corr_basis(xg);
  }
}

/* ARGSUSED */
XtCallbackProc
corr_optimz_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int x, y2;

  if (!xg->is_corr_optimz)
  {
    xg->is_corr_optimz = True;
    corr_event_handlers(xg, 0);
    XtUnmapWidget(xg->corr_mouse);
    xg->new_corr_dir_flag = True;
  }
  else
  {
    xg->is_corr_optimz = False;
    corr_event_handlers(xg, 1);
    XtMapWidget(xg->corr_mouse);
    start_corr_proc(xg);
  }

  zero_corr_taus();
/*  zero_corr_tincs();
  zero_cindx_prev();*/
  set_sens_corr_reinit_cmd(xg, !xg->is_corr_optimz);
  
  x = count*cp_bw_points + xg->xaxis_indent;
  y2 = (int)(cp_wksp.height) - 10;
  optimz_circs[noptimz_circs++] = x - OPTIMZ_OFFSET;
  XFillArc(display, cp_plot_pixmap, tour_pp_GC,
    optimz_circs[noptimz_circs-1], y2, OPTIMZ_CIRC_WIDTH, OPTIMZ_CIRC_HEIGHT,
    OPTIMZ_CIRC_ANG1, OPTIMZ_CIRC_ANG2);
  XCopyArea(display, cp_plot_pixmap, cp_plot_window, tour_pp_GC,
    0, 0, cp_wksp.width, cp_wksp.height, 0, 0);

  setToggleBitmap(w, xg->is_corr_optimz);
}

/*
void 
map_cp_panel(on)
  Boolean on;
{
  if (on)
    XtMapWidget(CP_PANEL);
  else
    XtUnmapWidget(CP_PANEL);
}
*/

/* ARGSUSED */
XtCallbackProc
corr_pursuit_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  float ftmp;
  static int firsttime = 1;

  if (xg->is_corr_pursuit)
  {
    if (xg->is_corr_optimz)
    {
      xg->is_corr_optimz = False;
      reset_corr_optimz(False);
    }

    reset_cp_plot();
    XtPopdown(cp_plot_shell);
    XtFree((XtPointer) cp_index_array);
    XtFree((XtPointer) cp_index_rect);
    XtFree((XtPointer) cp_index_pts);
    XtFree((XtPointer) new_basis_lines);
    XtFree((XtPointer) optimz_circs);
    XFreePixmap(display, cp_plot_pixmap);
    nnew_bases = 0;
    noptimz_circs = 0;
    free_cp(xg);
    if (xg->ncorr_xvars > 1 || xg->ncorr_yvars > 1)
    {
      xg->is_corr_pursuit = False;
    }
    else
      reset_corr_pursuit_cmd(xg, 1);

    XtUnmapWidget(CP_PANEL);
  }
  else
  {
    xg->is_corr_pursuit = True;
    xg->is_corr_sphered = True;
    reset_corr_sphere_cmd(xg,1);
    alloc_corr_index(xg);
    alloc_cp(xg);
    XtPopup(cp_plot_shell, XtGrabNone);
    XRaiseWindow(display, XtWindow(cp_plot_shell));

    if (firsttime)
      set_wm_protocols(cp_plot_shell);

    XtVaGetValues(cp_plot_wksp,
      XtNwidth, &cp_wksp.width,
      XtNheight, &cp_wksp.height, NULL);
    cp_plot_window = XtWindow(cp_plot_wksp);
    make_cp_pixmap();
    ftmp = (float) ((int) cp_wksp.width - (int) xg->xaxis_indent);
    max_cp_storage = (int) ((1.-CP_PLOTMARGIN_TOP) * ftmp);

    cp_index_array = (float *) XtMalloc(
      (unsigned int) (max_cp_storage+1) * sizeof(float));
    cp_index_rect = (XRectangle *) XtMalloc(
      (unsigned int) (max_cp_storage+1) * sizeof(XRectangle));
    cp_index_pts = (XPoint *) XtMalloc(
      (unsigned int) (max_cp_storage+1) * sizeof(XPoint));
    new_basis_lines = (int *) XtMalloc(
      (unsigned int) (max_cp_storage+1) * sizeof(int));

    optimz_circs = (int *) XtMalloc(
      (unsigned int) MAX_NOPTIMZ_CIRCS * sizeof(int));

    XtMapWidget(CP_PANEL);
    cp_index(xg,0,0);
  }

  setToggleBitmap(w, xg->is_corr_pursuit);
  setToggleBitmap(w, xg->is_corr_optimz);
}

void
make_cp_plot(xgobidata *xg, Widget parent)
{
  char ftitle[64];

  sprintf(ftitle, "%s", "Correlation Pursuit: Correlation Index");

  cp_plot_shell = XtVaCreatePopupShell("PPshell",
    topLevelShellWidgetClass, parent,
    XtNtitle, (String) ftitle,
    XtNiconName, (String) ftitle,
    NULL);
  if (mono) set_mono(cp_plot_shell);

  cp_plot_form = XtVaCreateManagedWidget("PPForm",
    panedWidgetClass, cp_plot_shell,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtRubber,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtRubber,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(cp_plot_form);

  cp_plot_box0 = XtVaCreateManagedWidget("Box",
    formWidgetClass, cp_plot_form,
    NULL);
  if (mono) set_mono(cp_plot_box0);

  CP_PANEL = XtVaCreateManagedWidget("CorrPanel",
    formWidgetClass, cp_plot_box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
    NULL);
  if (mono) set_mono(cp_panel[0]);

  NULL_BTN = (Widget) CreateToggle(xg, "Null",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, True, ANY_OF_MANY,
    cp_panel[0], "Corr");
  XtManageChild(NULL_BTN);
  XtAddCallback(NULL_BTN, XtNcallback,
    (XtCallbackProc) null_cback, (XtPointer) xg);

/* cp plot workspace */
  cp_plot_box1 = XtVaCreateManagedWidget("Box",
    formWidgetClass, cp_plot_form,
    NULL);
  if (mono) set_mono(cp_plot_box1);

  cp_plot_wksp = XtVaCreateManagedWidget("PPplot",
    labelWidgetClass, cp_plot_box1,
    XtNresizable, (Boolean) True,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNright, (XtEdgeType) XtRubber,
    XtNbottom, (XtEdgeType) XtRubber,
    XtNlabel, (String) "",
    NULL);
  if (mono) set_mono(cp_plot_wksp);

  XtAddEventHandler(cp_plot_wksp,
    ExposureMask,
    FALSE, (XtEventHandler) cpexpose_cback, (XtPointer) NULL);
  XtAddEventHandler(cp_plot_wksp,
    StructureNotifyMask,
    FALSE, (XtEventHandler) cpresize_cback, (XtPointer) xg);
}

void
make_cp_panel(xgobidata *xg, Widget panel)
{
  char str[64];
  Dimension width;

/*
 * Panel for projection pursuit: now initiate it unmapped, mapping
 * it only when PP is turned on.  This panel and the section tour
 * panel will occupy the same space.
*/
  CP_PANEL = XtVaCreateManagedWidget("TourPPPanel",
    formWidgetClass, xg->box0,
    XtNfromVert, (Widget) panel,
    XtNmappedWhenManaged, (Boolean) False,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    NULL);
  if (mono) set_mono(CP_PANEL);

/*
 * label to record projection pursuit index
*/

  sprintf(str, "CPIndx %.2e", -999.99);
  width = XTextWidth(appdata.font, str, strlen(str));

  CP_INDEX_LABEL = XtVaCreateManagedWidget("TourLabel",
    asciiTextWidgetClass, CP_PANEL,
    XtNstring, (String) " ",
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNdisplayCaret, (Boolean) False,
    NULL);
  if (mono) set_mono(CP_INDEX_LABEL);

/*
 * Button to do active or passive projection pursuit
*/
  CP_OPTIMZ = CreateToggle(xg, "Optimz",
    True, (Widget) NULL, CP_INDEX_LABEL, (Widget) NULL, False,
    ANY_OF_MANY,
    CP_PANEL, "Corr_Optmz");
  XtManageChild(CP_OPTIMZ);
  XtAddCallback(CP_OPTIMZ, XtNcallback,
    (XtCallbackProc) corr_optimz_cback, (XtPointer) xg);

}

