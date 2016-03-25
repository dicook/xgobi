/* xyplot.c */
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

static Widget xyplot_panel;

static Widget cycle_panel, xyplot_rc;
static Widget xy_fixx_cmd, xy_fixy_cmd;
Widget xy_cycle_cmd;
static Widget xy_cycle_delay_sbar, xy_chdir_cmd;
static int fixx = 0, fixy = 0;
static int xy_cycle_dir = 1;
static XtIntervalId cycle_timeout_id = 0L;
static float cycle_delay = 2.0;

Boolean
set_xyplotxvar(xgobidata *xg, int varno) {
  int prev;
  Boolean newvar = true;

  /* If it's the currently chosen x variable, do nothing */
  if (varno == xg->xy_vars.x)
    newvar = false;
  /*
   * If it's the current y variable, switch the two.
  */
  else if (varno == xg->xy_vars.y)
  {
    xg->xy_vars.y = xg->xy_vars.x;
    xg->xy_vars.x = varno;
    refresh_vbox(xg, xg->xy_vars.y, 0);
    refresh_vbox(xg, xg->xy_vars.x, 0);
  }
  /*
   * Otherwise choose the new x.
  */
  else {
    prev = xg->xy_vars.x;
    xg->varchosen[prev] = False;
    xg->xy_vars.x = varno;
    xg->varchosen[varno] = True;
    refresh_vbox(xg, prev, 1);
    refresh_vbox(xg, xg->xy_vars.x, 1);
  }

  return newvar;
}
Boolean
set_xyplotyvar(xgobidata *xg, int varno) {
  int prev;
  Boolean newvar = true;

  /* If it's the currently chosen y variable, do nothing */
  if (varno == xg->xy_vars.y)
    newvar = false;
  /* 
   * If it's the current x variable, switch the two.
  */ 
  else if (varno == xg->xy_vars.x)
  {
    xg->xy_vars.x = xg->xy_vars.y;
    xg->xy_vars.y = varno;
    refresh_vbox(xg, xg->xy_vars.y, 0);
    refresh_vbox(xg, xg->xy_vars.x, 0);
  }
  /* 
   * Otherwise choose the new y.
  */
  else
  {
    prev = xg->xy_vars.y;
    xg->varchosen[prev] = False;
    xg->xy_vars.y = varno;
    xg->varchosen[varno] = True;
    refresh_vbox(xg, prev, 1);
    refresh_vbox(xg, xg->xy_vars.y, 1);
  }
  return newvar;
}

Boolean
xy_varselect(int varno, int button, int state, xgobidata *xg)
/*
 * This is used only during simple xy plotting.
*/
{
  /*
   * If it's a left click ...
  */
  if (button == 1 && state != 8)  /* ... and alt not pressed */
    return (set_xyplotxvar(xg, varno));
  /*
   * If it's a middle click ...  or alt-left
  */
  else if (button == 2 || (button == 1 && state == 8))
    return (set_xyplotyvar(xg, varno));

  /* to please the compiler ... */
  return false;
}

void
init_xyplot_vars(xgobidata *xg)
{
  xg->is_xyplotting = True;
  xg->is_xy_cycle = False;
  xg->xy_vars.x = 0;
  xg->xy_vars.y = 1;
}

/* ARGSUSED */
XtCallbackProc
xy_cycle_delay_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  float fslidepos = * (float *) slideposp;

  cycle_delay = 1.0 / (2.0 * fslidepos + 0.2) ;
}

/* ARGSUSED */
XtCallbackProc
xy_fixx_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  fixx = !fixx;
  setToggleBitmap(w, fixx);
}

/* ARGSUSED */
XtCallbackProc
xy_fixy_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  fixy = !fixy;
  setToggleBitmap(w, fixy);
}

void
replot_xy(xgobidata *xg)
{
  init_ticks(&xg->xy_vars, xg);

  world_to_plane(xg);
  plane_to_screen(xg);
  plot_once(xg);
}

void
cycle_fixedx(xgobidata *xg)
{
  int varno;

  if (xy_cycle_dir == 1)
  {
    varno = xg->xy_vars.y + 1;

    if (varno == xg->xy_vars.x)
       varno++;

    if (varno == xg->ncols_used)
    {
      varno = 0;
      if (varno == xg->xy_vars.x)
         varno++;
    }
  } else {
    varno = xg->xy_vars.y - 1;

    if (varno == xg->xy_vars.x)
       varno--;

    if (varno < 0)
    {
      varno = xg->ncols_used-1;
      if (varno == xg->xy_vars.x)
         varno--;
    }
  }

  xg->varchosen[xg->xy_vars.y] = False;
  refresh_vbox(xg, xg->xy_vars.y, 1);
  xg->xy_vars.y = varno;
  xg->varchosen[xg->xy_vars.y] = True;
  refresh_vbox(xg, xg->xy_vars.y, 1);

  replot_xy(xg);
  XSync(display, False);
}

void
cycle_fixedy(xgobidata *xg)
{
  int varno;

  if (xy_cycle_dir == 1)
  {
    varno = xg->xy_vars.x + 1;

    if (varno == xg->xy_vars.y)
       varno++;

    if (varno == xg->ncols_used)
    {
      varno = 0;
      if (varno == xg->xy_vars.y)
        varno++;
    }
  } else {
    varno = xg->xy_vars.x - 1;

    if (varno == xg->xy_vars.y)
       varno--;

    if (varno < 0)
    {
      varno = xg->ncols_used-1;
      if (varno == xg->xy_vars.y)
        varno--;
    }
  }

  xg->varchosen[xg->xy_vars.x] = False;
  refresh_vbox(xg, xg->xy_vars.x, 1);
  xg->xy_vars.x = varno;
  xg->varchosen[xg->xy_vars.x] = True;
  refresh_vbox(xg, xg->xy_vars.x, 1);

  replot_xy(xg);
  XSync(display, False);
}

void
cycle_xy(xgobidata *xg)
{
  int jx, jy;
  int do_label;

  jx = xg->xy_vars.x;
  jy = xg->xy_vars.y;

  if (xy_cycle_dir == 1) {

    if ((jx == xg->ncols_used-1) ||
        (jx == xg->ncols_used-2 && jy == xg->ncols_used-1) )
    {
      jx = 0;
      jy = jx+1;
    }
    else if (jy < jx) {
      jy = jx+1;
    }
    else if (jy == xg->ncols_used-1) {
      jx++;
      jy = jx+1;
    }
    else
      jy++;

  } else {

    if ( jx == 0 || (jx == 1 && jy == 0) ) {
      jx = xg->ncols_used-1;
      jy = jx-1;
    }
    else if (jy > jx) {
      jy = jx-1;
    }
    else if (jy == 0) {
      jx--;
      jy = jx-1;
    }
    else
      jy--;
  }

  if (jx != xg->xy_vars.x)
  {
    do_label = !(xg->varchosen[jx]);

    xg->varchosen[xg->xy_vars.x] = False;
    refresh_vbox(xg, xg->xy_vars.x, do_label);
    xg->xy_vars.x = jx;
    xg->varchosen[xg->xy_vars.x] = True;
    refresh_vbox(xg, xg->xy_vars.x, do_label);
  }

  do_label = !(xg->varchosen[jy]);

  xg->varchosen[xg->xy_vars.y] = False;
  refresh_vbox(xg, xg->xy_vars.y, do_label);
  xg->xy_vars.y = jy;
  xg->varchosen[xg->xy_vars.y] = True;
  refresh_vbox(xg, xg->xy_vars.y, do_label);
  XSync(display, False);

  replot_xy(xg);
}

/* ARGSUSED */
static void
turn_on_xy_animation(xgobidata *xg, XtIntervalId id)
{
  xg->is_xy_cycle = True;
  (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);

  setToggleBitmap(xy_cycle_cmd, xg->is_xy_cycle);
}

void
xy_cycle_proc(xgobidata *xg)
{
  if (fixx)
    cycle_fixedx(xg);
  else if (fixy)
    cycle_fixedy(xg);
  else
    cycle_xy(xg);

  xg->is_xy_cycle = False;
  cycle_timeout_id = XtAppAddTimeOut(app_con,
     (unsigned long) (1000*cycle_delay),
     (XtTimerCallbackProc) turn_on_xy_animation, (XtPointer) xg);
}

/* ARGSUSED */
XtCallbackProc
xy_cycle_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (!xg->is_xy_cycle && cycle_timeout_id == 0L) {
    xg->is_xy_cycle = True;
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
  }
  else {
    XtRemoveTimeOut(cycle_timeout_id);
    cycle_timeout_id = 0L;
    xg->is_xy_cycle = False;
  }

  setToggleBitmap(w, xg->is_xy_cycle);
}

/* ARGSUSED */
XtCallbackProc
xy_chdir_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  xy_cycle_dir = -1 * xy_cycle_dir;
}

void
xy_reproject(xgobidata *xg)
{
/*
 * Project the data down from the ncols_used-dimensional world_data[]
 * to the 2-dimensional array planar[].
*/
  int j;
  int ix = xg->xy_vars.x;
  int iy = xg->xy_vars.y;

  for (j=0; j<xg->nrows; j++) {
    xg->planar[j].x = xg->world_data[j][ix];
    xg->planar[j].y = xg->world_data[j][iy];
  }
}

void
map_xyplot(Boolean on)
{
  if (on)
    XtMapWidget(xyplot_panel);
  else
    XtUnmapWidget(xyplot_panel);
}

void
make_xyplot(xgobidata *xg)
{
/*
 * XYPlotPanel: xyplot_panel
*/
  char str[35];
  Dimension max_width;
/*
 * Widest button label used in this panel.
*/
  sprintf(str, "Change Direction");
  max_width = XTextWidth(appdata.font, str, strlen(str));

  xyplot_panel = XtVaCreateManagedWidget("XYPlotPanel",
    boxWidgetClass, xg->box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) True,
    XtNorientation, (XtOrientation) XtorientVertical,
    NULL);
  if (mono) set_mono(xyplot_panel);

  cycle_panel = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, xyplot_panel,
    XtNhorizDistance, 5,
    XtNvertDistance, 5,
    NULL);
  if (mono) set_mono(cycle_panel);

  xyplot_rc = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, cycle_panel,
    XtNhSpace, (int) 3,
    XtNvSpace, (int) 1,
    XtNborderWidth, (int) 0,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(xyplot_rc);

  xy_fixx_cmd = CreateToggle(xg, "Fix X",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, ANY_OF_MANY,
    xyplot_rc, "XYPlot_Cycle");
  XtManageChild(xy_fixx_cmd);
  XtAddCallback(xy_fixx_cmd, XtNcallback,
     (XtCallbackProc) xy_fixx_cback, (XtPointer) xg);

  xy_fixy_cmd = CreateToggle(xg, "Fix Y",
    True, xy_fixx_cmd, (Widget) NULL, xy_fixx_cmd, False, ANY_OF_MANY,
    xyplot_rc, "XYPlot_Cycle");
  XtManageChild(xy_fixy_cmd);
  XtAddCallback(xy_fixy_cmd, XtNcallback,
    (XtCallbackProc) xy_fixy_cback, (XtPointer) xg);

  xy_cycle_cmd = CreateToggle(xg, "Cycle",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, ANY_OF_MANY,
    cycle_panel, "XYPlot_Cycle");
  XtManageChild(xy_cycle_cmd);
  XtAddCallback(xy_cycle_cmd, XtNcallback,
    (XtCallbackProc) xy_cycle_cback, (XtPointer) xg);

  xy_cycle_delay_sbar = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, cycle_panel,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    XtNwidth, (Dimension) max_width,
    XtNfromVert, (Widget) xy_cycle_cmd,
    NULL);
  if (mono) set_mono(xy_cycle_delay_sbar);

  /*
   * delay = 1 / (2*slidepos + .2)
   * slidepos = 1 / (2*delay) - .1
  */
  XawScrollbarSetThumb(xy_cycle_delay_sbar,
    1.0 / (2.0 * cycle_delay) - 0.1,
    -1.);
  XtAddCallback(xy_cycle_delay_sbar, XtNjumpProc,
    (XtCallbackProc) xy_cycle_delay_cback, (XtPointer) xg);
  add_sbar_help(&xg->nhelpids.sbar,
    xy_cycle_delay_sbar, "XYPlot_Cycle");

  xy_chdir_cmd = CreateCommand(xg, "Change Direction",
    True, (Widget) NULL, (Widget) NULL,
    cycle_panel, "XYPlot_Cycle");
  XtManageChild(xy_chdir_cmd);
  XtAddCallback(xy_chdir_cmd, XtNcallback,
    (XtCallbackProc) xy_chdir_cback, (XtPointer) NULL );
}

void
xyplot_on(xgobidata *xg)
/*
 * Make two-variable plots.
*/
{
  int j;
  Boolean cycle_on;

/*
 *  If this mode is currently selected, turn it off.
*/
  if (xg->prev_plot_mode == XYPLOT_MODE && xg->plot_mode != XYPLOT_MODE) {
    map_xyplot(False);

    xg->is_xyplotting = True;

    xg->is_xy_cycle = False;
    if (cycle_timeout_id)
    {
      XtRemoveTimeOut(cycle_timeout_id);
      cycle_timeout_id = 0L;
    }
/* Else turn it on */
  } else if (xg->prev_plot_mode != XYPLOT_MODE &&
             xg->plot_mode == XYPLOT_MODE)
  {
    map_xyplot(True);

    if (!xg->is_xyplotting)
    {
      if (xg->is_plotting1d)
        free_txtr_var();
      else if (xg->is_touring && xg->is_princ_comp)
	set_sph_labs(xg, xg->nsph_vars);
/*        reset_var_labels(xg, PRINCCOMP_OFF); i think not needed anymore
                                             sphering transformation*/

      if (xg->carry_vars)
        carry_xyplot_vars(xg);

      xg->is_plotting1d = False;
      xg->is_spinning = False;
      xg->is_touring = False;
      xg->is_corr_touring = False;
      xg->is_xyplotting = True;

      update_lims(xg);
      update_world(xg);
      world_to_plane(xg);
      plane_to_screen(xg);

      init_tickdelta(xg);
      init_ticks(&xg->xy_vars, xg);

      plot_once(xg);
    /*
     * Reinitialize the sin and cos variables.
    */
      init_trig(xg);

      for (j=0; j<xg->ncols_used; j++)
      {
        if (j == xg->xy_vars.x ||
          j == xg->xy_vars.y)
            xg->varchosen[j] = True;
        else
            xg->varchosen[j] = False;
      }

      set_varsel_label(xg);
      refresh_vboxes(xg);
    }

    XtVaGetValues(xy_cycle_cmd, XtNstate, &cycle_on, NULL);
    if (cycle_on) {
      xg->is_xy_cycle = True;
      (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
    }
  }
  setToggleBitmap(xy_cycle_cmd, xg->is_xy_cycle);
}
