/* smooth.c */
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
 *             www.research.att.com/~andreas/               *
 *                                                          *
 ************************************************************/

#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#ifdef XPLORE
#define NSMOOTHERS 11 /*xplore*/
#else
#define NSMOOTHERS 4
#endif

static Widget smooth_panel[2], sm_cmd[5], sm_sbar[2], sm_label[2];
static Widget sm_menu_cmd, sm_menu;
static Widget sm_menu_btn[NSMOOTHERS];

static long *smptsx, *smptsy;
static int num_grps;
static unsigned long *grp_id, groups[NCOLORS];
static long *sm_pars;
static XSegment **sm_lines;
static int smchoice;
int *grp_id_sm;
int *n_smgp_pts;
int *sm_gp_npts;
unsigned long *sm_gp_colors; /* postscript */
long *x_sm;
long *y_sm;
static int num_pts;
Boolean is_smooth;
Boolean usegps;
Boolean drw_wnd;
Boolean fixd_win, fixd_pts;

#define SMOOTH_ON sm_cmd[0]
/*#define FIXDWINDOW sm_cmd[1]
#define FIXDPTS sm_cmd[2]*/
#define USEGPS sm_cmd[1]
#define STEP sm_sbar[0]
#define WIDTH sm_sbar[1]
#define STEP_LAB sm_label[0]
#define WIDTH_LAB sm_label[1]
#define SHOW_WINDOW sm_cmd[1]

/* --------- Dynamic allocation section ----------- */
void
alloc_smooth_arrays(xgobidata *xg)
/*
 * Dynamically allocate arrays.
*/
{
  Cardinal nr = (Cardinal) xg->nrows;
  int i;

  smptsx = (long *) XtMalloc(nr * sizeof(long));
  smptsy = (long *) XtMalloc(nr * sizeof(long));
  grp_id = (unsigned long *) XtMalloc(nr * sizeof(unsigned long));
  sm_pars = (long *) XtMalloc((Cardinal) 4 * sizeof(long));
  sm_lines = (XSegment **) XtMalloc(NCOLORS * sizeof(XSegment *));
  for (i=0; i<NCOLORS; i++)
    sm_lines[i] = (XSegment *) XtMalloc(nr * sizeof(XSegment));

  sm_gp_npts = (int *) XtMalloc(NCOLORS * sizeof(int));/* postscript */
  sm_gp_colors = (unsigned long *)
    XtMalloc(NCOLORS * sizeof(unsigned long));/* postscript */
}

void
free_smooth_arrays(void)
/*
 * Dynamically free arrays.
*/
{
  int i;

  XtFree((XtPointer) smptsx);
  XtFree((XtPointer) smptsy);
  XtFree((XtPointer) grp_id);
  XtFree((XtPointer) sm_pars);
  for (i=0; i<NCOLORS; i++)
   XtFree((XtPointer) sm_lines[i]);
  XtFree((XtPointer) sm_lines);

  XtFree((XtPointer) sm_gp_npts);/* postscript */
  XtFree((XtPointer) sm_gp_colors);/* postscript */
}

/* --------- End of dynamic allocation section ----------- */

void
init_smooth_vars(xgobidata *xg)
{
  xg->is_smoothing = False;

  sm_pars[0] = 1000;
  sm_pars[1] = 5000;

  usegps = False;
  drw_wnd = False;
  fixd_win = True;
  fixd_pts = False;
  smchoice = 0;
}

void
map_smooth(xgobidata *xg)
{
  if (xg->is_smoothing)
    XtMapWidget(smooth_panel[0]);
  else
    XtUnmapWidget(smooth_panel[0]);
}

void 
draw_window(xgobidata *xg)
{
  unsigned int tmpl;

  tmpl = (unsigned int) ((sm_pars[1] * xg->is.x) >> EXP1);
  XDrawRectangle(display, xg->pixmap0, copy_GC, 5, 5, tmpl, 
    (unsigned int) 10);
  tmpl = (unsigned int) ((sm_pars[0] * xg->is.x) >> EXP1);
  XDrawLine(display, xg->pixmap0, copy_GC, 5, 10, 5+tmpl, 10); 

/*  XCopyArea(display, xg->pixmap0, xg->plot_window, copy_GC,
    0, 0,
    xg->plotsize.width, xg->plotsize.height, 0, 0 );*/
}

void
ps_smooth(FILE *psfile, float fac, float xoff, float yoff, float minx, float maxy, XColor whitepix, XColor *rgb_table)
{
  XColor *rgb_lines;
  int i, j, k;
  /*int color_now = -1, color_used[NCOLORS];*/

  /*
   * Build a vector of XColor values for each line.
  */
   rgb_lines = (XColor *) XtMalloc((Cardinal)
     num_pts * sizeof(XColor));
   if (mono || !usegps) 
   { /* black line */
     for (i=0; i<(num_pts-1); i++) 
     {
       rgb_lines[i].red = whitepix.red;
       rgb_lines[i].green = whitepix.green;
       rgb_lines[i].blue = whitepix.blue;
     }
     for (i=0; i<(num_pts-1); i++) 
     {
       (void) fprintf(psfile, "%f %f %f 1 %f %f %f %f ln\n",
         (float) rgb_lines[i].red / (float) 65535,
         (float) rgb_lines[i].green / (float) 65535,
         (float) rgb_lines[i].blue / (float) 65535,
         (float) (sm_lines[0][i].x1 - minx) * fac + xoff,
         (float) (maxy - sm_lines[0][i].y1) * fac + yoff,
         (float) (sm_lines[0][i].x2 - minx) * fac + xoff,
         (float) (maxy - sm_lines[0][i].y2) * fac + yoff );
     }
   }
   else 
   { /* color lines */
     for (j=0; j<num_grps; j++)
     {
       for (k=0; k<ncolors; k++) 
       {
         if (sm_gp_colors[j] == color_nums[k]) 
         {
           for (i=0; i<sm_gp_npts[j]; i++)
           {
             rgb_lines[i].red = rgb_table[k].red;
             rgb_lines[i].green = rgb_table[k].green;
             rgb_lines[i].blue = rgb_table[k].blue;
           }
           break;
         }
       }

      /* r g b width x1 y1 x2 y2 ln */
      for (i=0; i<sm_gp_npts[j]; i++)
      {
        (void) fprintf(psfile, "%f %f %f 1 %f %f %f %f ln\n",
          (float) rgb_lines[i].red / (float) 65535,
          (float) rgb_lines[i].green / (float) 65535,
          (float) rgb_lines[i].blue / (float) 65535,
          (float) (sm_lines[j][i].x1 - minx) * fac + xoff,
          (float) (maxy - sm_lines[j][i].y1) * fac + yoff,
          (float) (sm_lines[j][i].x2 - minx) * fac + xoff,
          (float) (maxy - sm_lines[j][i].y2) * fac + yoff );
       }
     }
   }
   XtFree((XtPointer) rgb_lines);
}

void
plot_smooth(xgobidata *xg)
{
  int i, j, k, n, from;
  unsigned long color_now = -1, color_used[NCOLORS];
  long tmpl;
  Boolean new_grp = True, used, *break_line;

  if (num_pts < 1)
    return;
  /* nothing there to plot */
  
  /* check for no points in smooth window */
  if (fixd_win)
  {
    break_line = (Boolean *) XtMalloc((Cardinal) num_pts*sizeof(Boolean));
    for (j=0; j<num_grps; j++)
    {
      for (i=1; i<num_pts; i++)
      {
        break_line[i] = False;
        if (x_sm[i]-x_sm[i-1] > sm_pars[1])
          break_line[i] = True;
      }
    }
  }

  /* scale points into screen */
  for (i=0; i<num_pts; i++)
  {
    tmpl = x_sm[i] + xg->shift_wrld.x;
    x_sm[i] = (int) ((tmpl * xg->is.x) >> EXP1);
    x_sm[i] += xg->mid.x;

    tmpl = y_sm[i] - xg->shift_wrld.y;
    y_sm[i] = (int) ((tmpl * xg->is.y) >> EXP1);
    y_sm[i] += xg->mid.y;
  }

  /* if mono, num_grps is 1 */
  if (mono || !usegps) 
  {
    for (i=0; i<(num_pts-1); i++) 
    {
      sm_lines[0][i].x1 = (short) x_sm[i];
      sm_lines[0][i].x2 = (short) x_sm[i+1];
      sm_lines[0][i].y1 = (short) y_sm[i];
      sm_lines[0][i].y2 = (short) y_sm[i+1];
    }
    XDrawSegments(display, xg->pixmap0, copy_GC,
      sm_lines[0], num_pts-1);
  } 
  else 
  {
    for (k=0; k<num_grps; k++)
    {
      n=0;
      for (i=0; i<num_pts; i++)
      {
        if (new_grp)
        {
          if (color_now != groups[grp_id_sm[i]-1])
          {
            used = False;
            for (j=0; j<k; j++)
              if (groups[grp_id_sm[i]-1] == color_used[j])
                used = True;
            if (!used)
            {
              color_now = groups[grp_id_sm[i]-1];
              from = i;
              new_grp = False;
            }
          }
        }
        else
        {
          if (color_now == groups[grp_id_sm[i]-1])
          {
            if (fixd_win && break_line[i])
            {
              XSetForeground(display, copy_GC, color_now );
              XDrawSegments(display, xg->pixmap0, copy_GC,
                sm_lines[k], n);
              sm_gp_npts[k] = n;/* postscript */
              sm_gp_colors[k] = color_now;/* postscript */
              from = i;
              n = 0;
            }
            else
            {
              sm_lines[k][n].x1 = (short) x_sm[from];
              sm_lines[k][n].x2 = (short) x_sm[i];
              sm_lines[k][n].y1 = (short) y_sm[from];
              sm_lines[k][n].y2 = (short) y_sm[i];
              from = i;
              n++;
            }
          }
        }
      }
      color_used[k] = color_now;
      new_grp = True;
      XSetForeground(display, copy_GC, color_now );
      XDrawSegments(display, xg->pixmap0, copy_GC,
        sm_lines[k], n);
      sm_gp_npts[k] = n;/* postscript */
      sm_gp_colors[k] = color_now;/* postscript */
    }
  }
  if (fixd_win)
    XtFree((XtPointer) break_line);
  if (drw_wnd)
    draw_window(xg);
}

void
smooth_data(xgobidata *xg)
{
  int i, k, m;
  Boolean new_grp;

  num_grps = 1;
  num_pts = 0;
  for (i=0; i<xg->nrows_in_plot; i++)
  {
    m = xg->rows_in_plot[i];
    smptsx[i] = xg->planar[m].x;
    smptsy[i] = xg->planar[m].y;
    /* If mono, or not using groups, set the number of groups to one */
    if (mono || !usegps) 
    {
      num_grps = 1;
      groups[0] = plotcolors.fg;
      grp_id[i] = 1;
    } 
    else 
    {
      if (i == 0)
      {
        groups[0] = xg->color_now[m];
        grp_id[0] = 1;
      }
      else
      {
        new_grp = True;
        k = 0;
        while (new_grp && k<num_grps)
        {
          if (xg->color_now[m] == groups[k])
          {
            new_grp = False;
            grp_id[i] = k+1;
          }
          k++;
        }
        if (new_grp)
        {
          groups[num_grps] = xg->color_now[m];
          num_grps++;
          grp_id[i] = num_grps;
        }
      }
    }
  }

  if (smchoice == 0)
  {
   if (fixd_win)
      mean_smoother(smptsx,smptsy,xg->nrows_in_plot,sm_pars,
        num_grps,grp_id,&num_pts,xg->nrows);
  }

  else if (smchoice == 1)
  {
    if (fixd_win)
      median_smoother(smptsx,smptsy,xg->nrows_in_plot,sm_pars,
        num_grps,grp_id,&num_pts,xg->nrows);
  }

  else if (smchoice == 2)
  {
    if (fixd_win)
      nadaraya_watson_smoother(smptsx,smptsy,xg->nrows_in_plot,sm_pars,
        num_grps,grp_id,&num_pts);
  }

  else if (smchoice == 3)
  {
    if (fixd_win)
      spline_smoother(smptsx,smptsy,xg->nrows_in_plot,sm_pars,
        num_grps,grp_id,&num_pts);
  }

#ifdef XPLORE

  else if (smchoice == 4)
  {
    if (fixd_win)
      xplore_smoother("xgobilinreg", smchoice, smptsx, smptsy,
        xg->nrows_in_plot,sm_pars, num_grps,grp_id,&num_pts);
  }
  else if (smchoice == 5)
  {
    if (fixd_win)
      xplore_smoother("xgobisknn", smchoice, smptsx,smptsy,
        xg->nrows_in_plot,sm_pars, num_grps,grp_id,&num_pts);
  }
  else if (smchoice == 6)
  {
    if (fixd_win)
      xplore_smoother("xgobiknn", smchoice, smptsx,smptsy,
        xg->nrows_in_plot,sm_pars, num_grps,grp_id,&num_pts);
  }
  else if (smchoice == 7)
  {
    if (fixd_win)
      xplore_smoother("xgobilowess", smchoice, smptsx,smptsy,
        xg->nrows_in_plot,sm_pars, num_grps,grp_id,&num_pts);
  }
  else if (smchoice == 8)
  {
    if (fixd_win)
      xplore_smoother("xgobilocpol", smchoice, smptsx,smptsy,
        xg->nrows_in_plot,sm_pars, num_grps,grp_id,&num_pts);
  }
  else if (smchoice == 9)
  {
    if (fixd_win)
      xplore_smoother("xgobineunet", smchoice, smptsx,smptsy,
        xg->nrows_in_plot,sm_pars, num_grps,grp_id,&num_pts);
  }
  else if (smchoice == 10)
  {
    if (fixd_win)
      xplore_smoother("xgobiisotonic", smchoice, smptsx,smptsy,
        xg->nrows_in_plot,sm_pars, num_grps,grp_id,&num_pts);
  }

#endif

  plot_smooth(xg);
}


/* ARGSUSED */
XtCallbackProc
smooth_on_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  xg->is_smoothing = !xg->is_smoothing;
  setToggleBitmap(w, xg->is_smoothing);
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
smooth_data_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
fixd_window_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (fixd_win) {
    fixd_win = False;
    fixd_pts = True;
  }
  else {
    fixd_win = True;
    fixd_pts = False;
  }
}

/* ARGSUSED */
XtCallbackProc
fixd_pts_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
}

/* ARGSUSED */
XtCallbackProc
choose_smooth_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int j;
  Arg args[5];

  XtSetArg(args[0], XtNleftBitmap, (Pixmap) None);
  for (j=0; j<NSMOOTHERS; j++)
    XtSetValues(sm_menu_btn[j], args, 1);
  for (j=0; j<NSMOOTHERS; j++) {
    if (sm_menu_btn[j] == w) {
      smchoice = j;
      break;
    }
  }

  XtVaSetValues(sm_menu_btn[smchoice],
    XtNleftBitmap, (Pixmap) menu_mark,
    NULL);
}

/* ARGSUSED */
XtCallbackProc
dummy_smooth_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{

#ifdef XPLORE

  int j;

  if (xg->xplore_flag)
    for (j=4; j<NSMOOTHERS; j++)
      XtVaSetValues(sm_menu_btn[j],
        XtNsensitive, (Boolean) True,
        NULL);
  else
    for (j=4; j<NSMOOTHERS; j++)
      XtVaSetValues(sm_menu_btn[j],
        XtNsensitive, (Boolean) False,
        NULL);

#endif

}

/* ARGSUSED */
XtCallbackProc
step_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  float slidepos = * (float *) slideposp;

  sm_pars[0] = (long) (slidepos * (float) PRECISION1 / 9. + 100.);
  if (sm_pars[0] > (long) ((float) sm_pars[1]/3.))
  {
    if ((float) sm_pars[0]*3. > (float) PRECISION1 / 9. + 100.)
    {
      sm_pars[0] = (long) ((float) sm_pars[1]/3.);
      XawScrollbarSetThumb(STEP,
      (float)(sm_pars[0]-100)*9./(float) PRECISION1, -1.);
    }
    else
    {
      sm_pars[1] = (long) ((float) sm_pars[0]*3.);
      XawScrollbarSetThumb(WIDTH,
      (float)(sm_pars[1]-100)*3./(float) PRECISION1, -1.);
    }
  }
  else if (sm_pars[0] < (long) ((float) sm_pars[1])/10.)
  { 
    if ((float) sm_pars[0]*10. < 1000.)
    {
      sm_pars[0] = (long) ((float) sm_pars[1]/10.);
      XawScrollbarSetThumb(STEP,
      (float)(sm_pars[0]-100)*9./(float) PRECISION1, -1.);
    }
    else
    {
      sm_pars[1] = (long) ((float) sm_pars[0]*10.);
      XawScrollbarSetThumb(WIDTH,
      (float)(sm_pars[1]-1000)*3./(float) PRECISION1, -1.);
    }
  }
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
width_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  float slidepos = * (float *) slideposp;

  sm_pars[1] = (long) (slidepos * (float) PRECISION1 * 2. / 3. + 1000.);
/*   printf("%d\n",sm_pars[1]); */
  sm_pars[0] = (long) ((float) sm_pars[1] / 3.);
/*  if (sm_pars[0] > (long) ((float) sm_pars[1])/3.)
  { 
    sm_pars[0] = (long) ((float) sm_pars[1]/3.);
    XawScrollbarSetThumb(STEP,
    (float)(sm_pars[0]-100)*9./(float) PRECISION1, -1.);
  }
  else if (sm_pars[0] < (long) ((float) sm_pars[1])/10.)
  { 
    sm_pars[0] = (long) ((float) sm_pars[1]/10.);
    XawScrollbarSetThumb(STEP,
    (float)(sm_pars[0]-100)*9./(float) PRECISION1, -1.);
  }*/
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
usegps_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  usegps = !usegps;
  setToggleBitmap(w, usegps);

  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
show_window_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  drw_wnd = !drw_wnd;
  setToggleBitmap(w, drw_wnd);

  plot_once(xg);
}

/* ------ Initialization section ------ */

static void
make_sm_menu(xgobidata *xg, Widget parent)
{
  int j;
  static char *sm_names[] = {
    "Mean",
    "Median",
    "Nadaraya-Watson",
    "Spline",

#ifdef XPLORE    

    "LINEAR",
    "SYMMETRIZED KNN",
    "KNN",
    "LOWESS",
    "LOCAL POLYNOMIAL",
    "NEURAL NETWORK",
    "ISOTONIC"

#endif

  };

  sm_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Smoothers",
    XtNmenuName, (String) "Menu",
    NULL);
  if (mono) set_mono(sm_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    sm_menu_cmd, "Sm_Menu");

  sm_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, sm_menu_cmd,
    NULL);
  if (mono) set_mono(sm_menu);

  XtAddCallback(sm_menu, XtNpopupCallback,
    (XtCallbackProc) dummy_smooth_cback, (XtPointer) xg);

  for (j=0; j<NSMOOTHERS; j++) {
    sm_menu_btn[j] = XtVaCreateWidget("Command",
      smeBSBObjectClass, sm_menu,
      XtNlabel, (String) sm_names[j],
      XtNleftMargin, (Dimension) 24,
      XtNleftBitmap, (Pixmap) None,
      NULL);
    if (mono) set_mono(sm_menu_btn[0]);
    XtAddCallback(sm_menu_btn[j], XtNcallback,
      (XtCallbackProc) choose_smooth_cback, (XtPointer) xg);
  }

#ifdef XPLORE

  for (j=4; j<NSMOOTHERS; j++)
    XtVaSetValues(sm_menu_btn[j],
      XtNsensitive, (Boolean) False,
      NULL);

#endif

  XtManageChildren(sm_menu_btn, NSMOOTHERS);
}

static void
init_smooth_menu(void)
{
  XtVaSetValues(sm_menu_btn[smchoice],
    XtNleftBitmap, menu_mark,
    NULL);
}

static Widget spopup = (Widget) NULL;
/* ARGSUSED */
static XtCallbackProc
close_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtPopdown(spopup);
  xg->is_smoothing = False;
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
open_smooth_popup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  static Boolean initd = False;
  char str[128];
  Dimension wdth;

  if (!initd)
  {
    Dimension width, height;
    Position x, y;
    Widget sframe, close_cmd;

    XtVaGetValues(w,
      XtNwidth, &width,
      XtNheight, &height, NULL);
    XtTranslateCoords(w,
      (Position) (width/2), (Position) (height/2), &x, &y);

    /*
     * Create the missing data popup
    */
    spopup = XtVaCreatePopupShell("MissingData",
      topLevelShellWidgetClass, XtParent(w),
      XtNinput,            (Boolean) True,
      XtNallowShellResize, (Boolean) True,
      XtNtitle,            (String) "Smooth",
      XtNiconName,         (String) "Smooth",
      XtNx,                x,
      XtNy,                y,
      NULL);
    if (mono) set_mono(spopup);

    /*
     * Create a paned widget so the 'Click here ...'
     * can be all across the bottom.
    */
    sframe = XtVaCreateManagedWidget("Form",
      panedWidgetClass, spopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      NULL);

  smooth_panel[0] = XtVaCreateManagedWidget("SmoothPanel",
    boxWidgetClass, sframe,
/*
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
*/
    NULL);
  if (mono) set_mono(smooth_panel[0]);

  SMOOTH_ON = CreateToggle(xg, "Smooth On",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL,
    True, ANY_OF_MANY, smooth_panel[0], "Smooth");
  XtManageChild(SMOOTH_ON);
  XtAddCallback(SMOOTH_ON, XtNcallback,
    (XtCallbackProc) smooth_on_cback, (XtPointer) xg);

/*
  smooth_panel[1] = XtVaCreateManagedWidget("SmoothPanel",
    boxWidgetClass, smooth_panel[0],
    NULL);
  if (mono) set_mono(smooth_panel[1]);

  FIXDWINDOW = CreateToggle(xg, "Window",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, True, ONE_OF_MANY,
    smooth_panel[1], "Smooth");
  FIXDPTS = CreateToggle(xg, "Points",
    True, (Widget) NULL, (Widget) NULL, FIXDWINDOW, False, ONE_OF_MANY,
    smooth_panel[1], "Smooth");
  XtManageChildren(&FIXDWINDOW, 2);
  XtAddCallback(FIXDWINDOW, XtNcallback,
    (XtCallbackProc) fixd_window_cback, (XtPointer) xg);
  XtAddCallback(FIXDPTS, XtNcallback,
    (XtCallbackProc) fixd_pts_cback, (XtPointer) xg);
*/

  make_sm_menu(xg, smooth_panel[0]);
  init_smooth_menu();

/*  sprintf(str, "%s", "Step");
  wdth = XTextWidth(appdata.font, str, strlen(str));

  STEP_LAB = XtVaCreateManagedWidget("TourLabel",
    asciiTextWidgetClass, smooth_panel[0],
    XtNstring, (String) "Step",
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) (wdth + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNfromVert, (Widget) sm_menu_cmd,
    NULL);
  if (mono) set_mono(STEP_LAB);

  STEP = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, smooth_panel[0],
    XtNfromVert, (Widget) sm_menu_cmd,
    XtNfromHoriz, (Widget) STEP_LAB,
    XtNwidth, (Dimension) (2*wdth + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNheight, (Dimension) 16,
    XtNhorizDistance, (Dimension) 0,
    XtNvertDistance, (int) 0,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(STEP);
  XawScrollbarSetThumb(STEP,
    (float) (sm_pars[0]-100) * 9./(float) PRECISION1, -1.);
  XtAddCallback(STEP, XtNjumpProc,
    (XtCallbackProc) step_cback, (XtPointer) xg);
  add_sbar_help(&xg->nhelpids.sbar,
    STEP, "Sm_Step");
*/
  sprintf(str, "%s", "Width");
  wdth = XTextWidth(appdata.font, str, strlen(str));

  WIDTH_LAB = XtVaCreateManagedWidget("TourLabel",
    asciiTextWidgetClass, smooth_panel[0],
    XtNstring, (String) "Width",
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) (wdth + 2*ASCII_TEXT_BORDER_WIDTH),
/*     XtNfromVert, (Widget) STEP_LAB, */
    XtNfromVert, (Widget) sm_menu_cmd,
    NULL);
  if (mono) set_mono(WIDTH_LAB);

  WIDTH = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, smooth_panel[0],
    XtNfromHoriz, (Widget) WIDTH_LAB,
/*     XtNfromVert, (Widget) STEP_LAB, */
    XtNfromVert, (Widget) sm_menu_cmd,
    XtNwidth, (Dimension) (2*wdth + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNheight, (Dimension) 16,
    XtNhorizDistance, (Dimension) 0,
    XtNvertDistance, (int) 0,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(WIDTH);
  XawScrollbarSetThumb(WIDTH,
    (float) (sm_pars[1]-100) * 3./((float) PRECISION1 * 2.), -1.);
  XtAddCallback(WIDTH, XtNjumpProc,
    (XtCallbackProc) width_cback, (XtPointer) xg);
  add_sbar_help(&xg->nhelpids.sbar,
    WIDTH, "Sm_Width");

/* dfs */
  USEGPS = CreateToggle(xg, "Use color groups",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, 
    ANY_OF_MANY, smooth_panel[0], "Sm_UseGps");
    XtManageChild(USEGPS);
  XtAddCallback(USEGPS, XtNcallback,
    (XtCallbackProc) usegps_cback, (XtPointer) xg);
  /* If mono, this button has no meaning; make it insensitive */
  if (mono)
    XtVaSetValues(USEGPS, XtNsensitive, False, NULL);

  SHOW_WINDOW = CreateToggle(xg, "Show Window",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, 
    ANY_OF_MANY,
    smooth_panel[0], "Sm_Show");
  XtManageChild(SHOW_WINDOW);
  XtAddCallback(SHOW_WINDOW, XtNcallback,
    (XtCallbackProc) show_window_cback, (XtPointer) xg);

    close_cmd = XtVaCreateManagedWidget("Close",
      commandWidgetClass, sframe,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close_cmd);
    XtAddCallback(close_cmd, XtNcallback,
      (XtCallbackProc) close_cback, (XtPointer) xg);
  }

  XtPopup(spopup, (XtGrabKind) XtGrabNone);
  XRaiseWindow(display, XtWindow(spopup));

  if (!initd)
  {
    init_smooth_menu();
    set_wm_protocols(spopup);
    initd = True;
  }

  xg->is_smoothing = True;
  plot_once(xg);

}

#undef NSMOOTHERS
#undef SMOOTH_ON
/*#undef FIXDWINDOW
#undef FIXDPTS*/
#undef USEGPS
#undef WIDTH 
#undef STEP 
#undef WIDTH_LAB 
#undef STEP_LAB 
#undef SHOW_WINDOW
