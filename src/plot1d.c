/* plot1d.c */
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

#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include <X11/keysym.h>

/*
 * min and max for 'forget it' dotplot's texturing axis;
 * they're defined so as to locate the plot in the center of
 * the window, with the texturing values returned on a range
 * of [0,100].
 *
 * The min and max are calculated on the fly for the ash.
*/
#define FORGETITAXIS_MIN -100.
#define FORGETITAXIS_MAX 200.

Boolean plot1d_vertically;

#define ASH     0
#define DOTPLOT 1
int plot1d_type = ASH;
Widget plot1d_type_cmd[2];

static Widget plot1d_panel;
static Widget plot1d_delay_sbar, plot1d_chdir_cmd;
Widget plot1d_cycle_cmd;
static int plot1d_cycle_dir = 1;
static XtIntervalId cycle_timeout_id = 0;
static float delay = 2.0;

static float *plot1d_data;
static lims   plot1d_lim;

Widget nASHes_sbar, nASHes_label;
int nbins = 200;
int nASHes = 20;

void
init_plot1d_vars(xgobidata *xg)
{
  xg->is_plotting1d = False;
  xg->is_plot1d_cycle = False;

  plot1d_vertically = False;
  xg->plot1d_vars.x = 0 ;
  xg->plot1d_vars.y = -1;
}

Boolean
set_plot1dvar(xgobidata *xg, int varno) {
  int prev;
  Boolean newvar = false;

  /*
   * If we're dotplotting, and the selected variable is the
   * current variable, pretend it's a new variable, and set
   * newvar to true in order to generate a new plot1d.
  */
  if (plot1d_type == DOTPLOT)
    newvar = (plot1d_vertically && varno == xg->plot1d_vars.y ||
             !plot1d_vertically && varno == xg->plot1d_vars.x);

  /*
   * Otherwise choose the new variable
  */
  if (!newvar) {
    prev = (xg->plot1d_vars.y != -1) ? xg->plot1d_vars.y : xg->plot1d_vars.x;
    xg->varchosen[prev] = False;
    refresh_vbox(xg, prev, 1);

    xg->plot1d_vars.y = (plot1d_vertically) ? varno : -1;
    xg->plot1d_vars.x = (plot1d_vertically) ? -1 : varno;

    xg->varchosen[varno] = True;
    refresh_vbox(xg, varno, 1);
    newvar = true;
  }

  return newvar;
}


Boolean
plot1d_varselect(int varno, int button, int state, xgobidata *xg)
{
  if (button == 1 && state != 8)
    plot1d_vertically = False;
  else if ((button == 1 && state == 8) || button == 2) /* alt-left = middle */
    plot1d_vertically = True;
  else
    return 0;

  return (set_plot1dvar(xg, varno));
}

void
plot1d_reproject(xgobidata *xg)
{
/*
 * Project the y variable down from the ncols-dimensional world_data[]
 * to the 2-dimensional array planar[]; get the x variable directly
 * from plot1d_data[].
*/
  int i, k;
  float rdiff = plot1d_lim.max - plot1d_lim.min;
  float ftmp;
  float precis = PRECISION1;
  int jvar = (plot1d_vertically) ? xg->plot1d_vars.y : xg->plot1d_vars.x;

  for (i=0; i<xg->nrows_in_plot; i++) {
    k = xg->rows_in_plot[i];

    /*
     * Use plot1d_data[i] not [k] because plot1d_data[]
     * is of length xg->nrows_in_plot rather than xg->nrows.
    */
    ftmp = -1.0 + 2.0*(plot1d_data[i] - plot1d_lim.min)/rdiff;

    /*
     * Now here's a charming kludge:  since we'd rather have the
     * jitter perpendicular to the axes, subtract it from the
     * selected variable and swap it onto the jitter variable.
     * November 1999 -- dfs
    */
    if (plot1d_vertically) {
      xg->planar[k].x = (long) (precis * ftmp) + xg->jitter_data[k][jvar];
      xg->planar[k].y = xg->world_data[k][jvar] - xg->jitter_data[k][jvar];
    } else {
      xg->planar[k].x = xg->world_data[k][jvar] - xg->jitter_data[k][jvar];
      xg->planar[k].y = (long) (precis * ftmp) + xg->jitter_data[k][jvar];
    }
  }
}

void
plot1d_texture_var(xgobidata *xg)
{
/*
 * Set up the next dot plot.
*/
  int i;
  float *yy;
  float del = 1.;
  int option = 1, stages = 3;
  int jvar = (plot1d_vertically) ? xg->plot1d_vars.y : xg->plot1d_vars.x;
  extern int do_ash();

  /*
   * yy is a temporary variable.  It's used by textur, and then
   * junked immediately afterward.
  */
  yy = (float *) XtMalloc((Cardinal) xg->nrows_in_plot * sizeof(float));

  for (i=0; i<xg->nrows_in_plot; i++)
    yy[i] = xg->tform2[ xg->rows_in_plot[i] ][jvar];

  if (plot1d_type == ASH) {
    float min, max;
    extern void do_ash1d(float *, int, int, int, float *, float *, float *);
/*
    int nbins = MAX((int) (xg->scale.x * xg->plotsize.width),
                    (int) (xg->scale.y * xg->plotsize.height));
*/

    do_ash1d(yy, xg->nrows_in_plot, nbins, nASHes, plot1d_data, &min, &max);
    plot1d_lim.min = min;
    plot1d_lim.max = max;
  }
  else {
    plot1d_lim.min = FORGETITAXIS_MIN ;
    plot1d_lim.max = FORGETITAXIS_MAX ;
    textur(yy, plot1d_data, xg->nrows_in_plot, option, del, stages);
  }

  XtFree((XtPointer) yy);
}

void
free_txtr_var()
{
  XtFree((XtPointer) plot1d_data);
}

/* ARGSUSED */
static void
turn_on_plot1d_animation(xgobidata *xg, XtIntervalId id)
{
  xg->is_plot1d_cycle = True;
  (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);

  setToggleBitmap(plot1d_cycle_cmd, xg->is_plot1d_cycle);
}

void
plot1d_cycle_proc(xgobidata *xg)
{
  int varno, varno_prev;

  varno_prev = (plot1d_vertically) ? xg->plot1d_vars.y : xg->plot1d_vars.x;

  if (plot1d_cycle_dir == 1)
  {
    varno = varno_prev + 1;
    if (varno > xg->ncols_used-1)
      varno = 0;
  } else {
    varno = varno_prev - 1;
    if (varno < 0)
      varno = xg->ncols_used-1;
  }
  xg->varchosen[varno_prev] = False;
  refresh_vbox(xg, varno_prev, 1);

  if (plot1d_vertically)
    xg->plot1d_vars.y = varno;
  else
    xg->plot1d_vars.x = varno;

  xg->varchosen[varno] = True;
  refresh_vbox(xg, varno, 1);

  plot1d_texture_var(xg);
  world_to_plane(xg);      /* just calls plot1d_reproject */
  plane_to_screen(xg);
  init_ticks(&xg->plot1d_vars, xg);
  plot_once(xg);
  XSync(display, False);

  xg->is_plot1d_cycle = False;
  cycle_timeout_id =  XtAppAddTimeOut(app_con,
     (unsigned long) (1000*delay),
     (XtTimerCallbackProc) turn_on_plot1d_animation, (XtPointer) xg);
}

void
reset_plot1d_type(xgobidata *xg)
{
  XtVaSetValues(plot1d_type_cmd[DOTPLOT],
    XtNstate, plot1d_type == DOTPLOT,
    NULL);
  setToggleBitmap(plot1d_type_cmd[DOTPLOT], plot1d_type == DOTPLOT);

  XtVaSetValues(plot1d_type_cmd[ASH],
    XtNstate, plot1d_type == ASH,
    NULL);
  setToggleBitmap(plot1d_type_cmd[ASH], plot1d_type == ASH);

  XtSetSensitive(nASHes_sbar, plot1d_type == ASH);

  plot1d_texture_var(xg);

  update_world(xg);
  world_to_plane(xg);
  plane_to_screen(xg);
  init_ticks(&xg->plot1d_vars, xg);
  plot_once(xg);
}
/* ARGSUSED */
XtCallbackProc
ptype_dotplot_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  plot1d_type = DOTPLOT;
  reset_plot1d_type(xg);
}
/* ARGSUSED */
XtCallbackProc
ptype_ash_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  plot1d_type = ASH;
  reset_plot1d_type (xg);
}

/* ARGSUSED */
XtCallbackProc
nASHes_cback (Widget w, xgobidata *xg, XtPointer slideposp)
{
  float fslidepos = * (float *) slideposp;
  char str[16];

  nASHes = (int) (nbins * (fslidepos+.01) / 2.0);

  sprintf(str, "%1.3f", (float) nASHes/(float) nbins);
  XtVaSetValues(nASHes_label, XtNlabel, str, NULL);

  plot1d_texture_var(xg);
  world_to_plane(xg);
  plane_to_screen(xg);
  init_ticks(&xg->plot1d_vars, xg);
  plot_once(xg);
  world_to_plane(xg);
  plane_to_screen(xg);
  init_ticks(&xg->plot1d_vars, xg);
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
plot1d_cycle_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Turn on cycling.
*/
{
  if (!xg->is_plot1d_cycle && cycle_timeout_id == 0L) {
    xg->is_plot1d_cycle = True;
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
  }
  else {
    XtRemoveTimeOut(cycle_timeout_id);
    cycle_timeout_id = 0L;
    xg->is_plot1d_cycle = False;
  }

  setToggleBitmap(plot1d_cycle_cmd, xg->is_plot1d_cycle);
}

/* ARGSUSED */
XtCallbackProc
plot1d_cycle_delay_cback(Widget w, xgobidata *xg, XtPointer slideposp)
/*
 * Change the cycling speed.
*/
{
  float fslidepos = * (float *) slideposp;
  delay = 1.0 / (2.0 * fslidepos + 0.2) ;
}

/* ARGSUSED */
XtCallbackProc
plot1d_chdir_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Change the direction of cycling.
*/
{
  plot1d_cycle_dir = -1 * plot1d_cycle_dir;
}

static void
map_plot1d(Boolean on)
{
  if (on)
    XtMapWidget(plot1d_panel);
  else
    XtUnmapWidget(plot1d_panel);
}


void
make_plot1d(xgobidata *xg)
{
/*
 * Plot1DPanel: plot1d_panel
*/
  char str[35];
  Dimension width, max_width;
  Widget ptype_box, cycle_box, ash_box;
/*
 * Widest button label used in this panel.
*/
  (void) sprintf(str, "ASH smoothness");
  max_width = XTextWidth(appdata.font, str, strlen(str));

  plot1d_panel = XtVaCreateManagedWidget("Plot1DPanel",
    boxWidgetClass, xg->box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
    XtNorientation, (XtOrientation) XtorientVertical,
    NULL);
  if (mono) set_mono(plot1d_panel);

  ptype_box = XtVaCreateManagedWidget("Plot1DPanel",
    boxWidgetClass, plot1d_panel,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNorientation, (XtOrientation) XtorientVertical,
    NULL);
  if (mono) set_mono(ptype_box);

  plot1d_type_cmd[DOTPLOT] = CreateToggle(xg, "Plot1D", True,
    (Widget) NULL, (Widget) NULL, (Widget) NULL,
    (plot1d_type == DOTPLOT), ONE_OF_MANY, ptype_box, "Plot1D");
  XtManageChild(plot1d_type_cmd[DOTPLOT]);
  XtAddCallback(plot1d_type_cmd[DOTPLOT], XtNcallback,
    (XtCallbackProc) ptype_dotplot_cback, (XtPointer) xg);

  plot1d_type_cmd[ASH] = CreateToggle(xg, "1D ASH", True,
    (Widget) NULL, (Widget) NULL, plot1d_type_cmd[DOTPLOT],
    (plot1d_type == ASH), ONE_OF_MANY, ptype_box, "Plot1D");
  XtManageChild(plot1d_type_cmd[ASH]);
  XtAddCallback(plot1d_type_cmd[ASH], XtNcallback,
    (XtCallbackProc) ptype_ash_cback, (XtPointer) xg);

/* ASH parameter */
  ash_box = XtVaCreateManagedWidget("Plot1DPanel",
    boxWidgetClass, plot1d_panel,
    NULL);

  (void) XtVaCreateManagedWidget("Label",
    labelWidgetClass, ash_box,
    XtNlabel, "ASH smoothness",
    NULL);

  nASHes_sbar = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, ash_box,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    XtNwidth, (Dimension) max_width,
    NULL);
  if (mono) set_mono(nASHes_sbar);
  /*
   * range of the scrollbar: 0 to 1.0
   * desired range:  .01 + (0 to .5), which is nASHes/nbins
   * 
   * (float) (nASHes) / (float) nbins = (slidepos+.01) / 2;
   * nASHes = (int) (nbins * (slidepos+.01) / 2.0);

   * slidepos = 2 * (float) nASHes / (float) nbins - .01;
  */
  XawScrollbarSetThumb(nASHes_sbar,
    2 * (float) nASHes / (float) nbins  - .01,  /*-- 2*20/200 - .01 --*/
    -1.);
  XtAddCallback(nASHes_sbar, XtNjumpProc,
    (XtCallbackProc) nASHes_cback, (XtPointer) xg);
  add_sbar_help(&xg->nhelpids.sbar,
    nASHes_sbar, "Plot1D");

  sprintf(str, "%1.4f", 0.9999);
  width = XTextWidth(appdata.font, str, strlen(str));
  sprintf(str, "%1.3f", (float)nASHes/(float)nbins);
  nASHes_label = XtVaCreateManagedWidget("Plot1D",
    labelWidgetClass,  ash_box,
    XtNlabel,    str,
    XtNwidth,    width,
    NULL);
  if (mono) set_mono(nASHes_label);
/*  */

  cycle_box = XtVaCreateManagedWidget("Plot1DPanel",
    boxWidgetClass, plot1d_panel,
    NULL);

  plot1d_cycle_cmd = CreateToggle(xg, "Cycle",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, ANY_OF_MANY,
    cycle_box, "Plot1D_Cycle");
  XtManageChild(plot1d_cycle_cmd);
  XtAddCallback(plot1d_cycle_cmd, XtNcallback,
    (XtCallbackProc) plot1d_cycle_cback, (XtPointer) xg);

  plot1d_delay_sbar = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, cycle_box,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    XtNwidth, (Dimension) max_width,
    XtNfromVert, (Widget) plot1d_cycle_cmd,
    NULL);
  if (mono) set_mono(plot1d_delay_sbar);

  /*
   * delay = 1 / (2*slidepos + .2)
   * slidepos = 1 / (2*delay) - .1
  */
  XawScrollbarSetThumb(plot1d_delay_sbar, 1.0 / (2.0 * delay) - 0.1, -1.);
  XtAddCallback(plot1d_delay_sbar, XtNjumpProc,
    (XtCallbackProc) plot1d_cycle_delay_cback, (XtPointer) xg);
  add_sbar_help(&xg->nhelpids.sbar,
    plot1d_delay_sbar, "Plot1D_Cycle");

  plot1d_chdir_cmd = CreateCommand(xg, "Change direction",
    True, (Widget) NULL, (Widget) NULL,
    cycle_box, "Plot1D_Cycle");
  XtManageChild(plot1d_chdir_cmd);
  XtAddCallback(plot1d_chdir_cmd, XtNcallback,
    (XtCallbackProc) plot1d_chdir_cback, (XtPointer) NULL);

}

void
plot1d_on(xgobidata *xg)
/*
 * Turn on and off the plot1d mode.
*/
{
  int j;
  Boolean cycle_on;
  extern xgobidata xgobi;
  int jvar = (plot1d_vertically) ? xg->plot1d_vars.y : xg->plot1d_vars.x;

/*
 *  If this mode is currently selected, turn it off.
*/
  if (xg->prev_plot_mode == PLOT1D_MODE && xg->plot_mode != PLOT1D_MODE) {
    map_plot1d(False);

    xg->is_plot1d_cycle = False;
    if (cycle_timeout_id)
    {
      XtRemoveTimeOut(cycle_timeout_id);
      cycle_timeout_id = 0L;
    }
  }
  /* Else turn it on */
  else if (xg->plot_mode == PLOT1D_MODE &&
           xg->prev_plot_mode != PLOT1D_MODE)
  {
    map_plot1d(True);

    if (!xg->is_plotting1d) {
      if (xg->is_touring && xg->is_princ_comp) {
	set_sph_labs(xg, xg->nsph_vars);
        xg->is_touring = False;
/*        reset_var_labels(xg, PRINCCOMP_OFF);i think not needed anymore
                                             sphering transformation*/
      }

      if (xg->carry_vars)
        carry_plot1d_vars(xg);

      xg->is_xyplotting = xg->is_spinning = xg->is_touring = False;
      xg->is_plotting1d = True;
      xg->is_corr_touring = False;

      plot1d_data = (float *) XtMalloc( (Cardinal)
        xg->nrows_in_plot * sizeof(float));

      plot1d_texture_var(xg);

      update_lims(xg);
      update_world(xg);
      world_to_plane(xg);
      plane_to_screen(xg);

      init_ticks(&xg->plot1d_vars, xg);

      plot_once(xg);
      /*
       * Reinitialize the sin and cos variables.  Why???
      */
      init_trig(xg);

      for (j=0; j<xg->ncols; j++) {
        if (j == jvar)
          xg->varchosen[j] = True;
        else
          xg->varchosen[j] = False;
      }

      set_varsel_label(xg);
      refresh_vboxes(xg);
    }
    /*
     * If the cycle button is activated, start cycling.
    */
    XtVaGetValues(plot1d_cycle_cmd, XtNstate, &cycle_on, NULL);

    if (cycle_on) {
      xg->is_plot1d_cycle = True;
      (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
    }
  }

  setToggleBitmap(plot1d_cycle_cmd, xg->is_plot1d_cycle);
}
