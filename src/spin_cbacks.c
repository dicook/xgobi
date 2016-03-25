/* spin_cbacks.c */
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

static Widget spin_panel[3], spin_sbar ;
Widget spin_cmd[9];
static Widget spin_place_save_coefs;
static Widget spin_place_save_rmat, spin_place_read_rmat;

#define CHDIR  spin_cmd[0]
#define PAUSE  spin_cmd[1]
#define REINIT spin_cmd[2]
#define ROCK   spin_cmd[3]
#define INTERP spin_cmd[4]
#define YAXIS  spin_cmd[5]
#define XAXIS  spin_cmd[6]
#define OAXIS  spin_cmd[7]

void 
reset_spin_pause_cmd(xgobidata *xg)
{
  XtCallCallbacks(PAUSE, XtNcallback, (XtPointer) xg);
  XtVaSetValues(PAUSE, XtNstate, xg->is_spin_paused, NULL);
  
  setToggleBitmap(PAUSE, xg->is_spin_paused);
}

void
start_spin_proc(xgobidata *xg)
{
  Boolean run;

  if (!xg->is_spin_paused && xg->theta0 != 0.) {
    if (xg->is_spin_type.oblique) {
      if (xg->is_rocking)
        xg->run_rock_proc = True;
      else
        xg->run_spin_oblique_proc = True;

      run = xg->run_rock_proc || xg->run_spin_oblique_proc ;
    }
    else
    {
      if (xg->is_interp && !xg->run_interp_proc) {
        find_quadrant(xg);
        xg->run_interp_proc = True;
      }
      else if (xg->is_rocking)
        xg->run_rock_proc = True;
      else
        xg->run_spin_axis_proc = True;

      run = xg->run_interp_proc || xg->run_rock_proc || xg->run_spin_axis_proc;
    }

    if (run)
      (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
  }
}

void
stop_spin_proc(xgobidata *xg)
{
  if (xg->is_spin_type.oblique)
  {
    xg->run_spin_oblique_proc = False;
    xg->run_rock_proc = False;
  }
  else
  {
    xg->run_spin_axis_proc = False;
    xg->run_rock_proc = False;
    if (xg->run_interp_proc)
    {
      xg->run_interp_proc = False;
      reset_interp_proc();
    }
  }
}

void
reinit_spin(xgobidata *xg)
{
  XtCallCallbacks(REINIT, XtNcallback, (XtPointer) xg);
}

void
map_spin(xgobidata *xg, Boolean on)
{
  if (on)
  {
    XtMapWidget(spin_panel[0]);
    XtMapWidget(xg->spin_mouse);
  }
  else {
    XtUnmapWidget(spin_panel[0]);
    XtUnmapWidget(xg->spin_mouse);
  }
}

void
set_rspeed(float slidepos, xgobidata *xg)
{
  if (xg->is_spin_type.oblique) {
    if (slidepos < 0.05) {
      stop_spin_proc(xg);
      xg->theta0 = 0.0;
    }
    else {
      xg->theta0 = slidepos / 10.;
      start_spin_proc(xg);
    }
  }
  else {
    if (slidepos < 0.05) {
      stop_spin_proc(xg);
      xg->theta0 = 0.0;
    }
    else {
      xg->theta0 = slidepos / 10.;
    }
  }
}

/* ARGSUSED */
XtCallbackProc
rspeed_cback(Widget w, xgobidata *xg, XtPointer slideposp)
/*
 * Adjust the apparent speed of rotation using the slider bar.
 * Actually being altered is the angular distance between successive
 * replots, by means of changing theta0.
*/
{
  float slidepos = * (float *) slideposp;

  set_rspeed(slidepos, xg);
}

/* ARGSUSED */
XtCallbackProc
rchdir_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Change the direction of rotation.
*/
{
  change_spin_direction();
}

/* ARGSUSED */
XtCallbackProc
rinterp_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * With Y fixed, interpolate back and forth between two X variables.
*/
{
  if (!xg->is_interp)
  {
    stop_spin_proc(xg);

    xg->is_rocking = False;
    xg->is_interp = True;

    if (!xg->is_spin_paused && xg->theta0 != 0.0)
    {
      find_quadrant(xg);
      xg->run_interp_proc = True;
      (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
    }
  }
  else
  {
    xg->run_interp_proc = False;
    reset_interp_proc();
    if (xg->theta0 != 0.0 && !xg->is_spin_paused)
    {
      xg->run_spin_axis_proc = True;
      (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
    }
    xg->is_interp = False;
  }

  setToggleBitmap(ROCK, xg->is_rocking);
  setToggleBitmap(INTERP, xg->is_interp);
}

/* ARGSUSED */
XtCallbackProc
rock_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * With Y fixed, rock back and forth around a fixed point.
*/
{
  if (!xg->is_rocking)
  {
    stop_spin_proc(xg);

    xg->is_interp = False;
    xg->is_rocking = True;

    if (!xg->is_spin_paused && xg->theta0 != 0.0)
    {
      xg->run_rock_proc = True;
      (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
    }
  }
  else
  {
    xg->run_rock_proc = False;
    xg->is_rocking = False;
    start_spin_proc(xg);
  }

  setToggleBitmap(ROCK, xg->is_rocking);
  setToggleBitmap(INTERP, xg->is_interp);
}

/* ARGSUSED */
XtCallbackProc
spin_pause_on_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Stop the spinning without turning off the rotate button.
*/
{
  if (!xg->is_spin_paused) {
    xg->is_spin_paused = True;
    stop_spin_proc(xg);
  }
  else {
    xg->is_spin_paused = False;
    start_spin_proc(xg);
  }

  setToggleBitmap(PAUSE, xg->is_spin_paused);
}

/* ARGSUSED */
XtCallbackProc
spin_reinit_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Reinitialize rotation.
*/
{
/*
 * For variable circles.
*/
  xg->xax.x = RADIUS;
  xg->xax.y = xg->yax.x = 0;
  xg->yax.y = RADIUS;
  xg->zax.x = xg->zax.y = 0;

  xg->theta.yaxis = xg->theta.xaxis = 0.0;
  xg->cost.y = xg->cost.x = 1;
  xg->sint.y = xg->sint.x = 0;
  xg->icost.y = (int) (xg->cost.y * PRECISION2);
  xg->isint.y = (int) (xg->sint.y * PRECISION2);
  xg->icost.x = (int) (xg->cost.x * PRECISION2);
  xg->isint.x = (int) (xg->sint.x * PRECISION2);

  init_ob_rotate(xg);

  world_to_plane(xg);
  plane_to_screen(xg);
  plot_once(xg);

  refresh_vboxes(xg);
  if (xg->is_spin_type.oblique)
    draw_ob_var_lines(xg);
  else
    spin_var_lines(xg);
}

/* ARGSUSED */
XtCallbackProc
spin_xaxis_on_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Let the x axis be the axis of rotation.
*/
{
  if (xg->is_spin_type.xaxis) {
    stop_spin_proc(xg);
    xg->is_spin_type.xaxis = 0;
  }
  else
  {
    xg->is_spin_type.xaxis = True;
    xg->is_spin_type.yaxis = xg->is_spin_type.oblique = False;
    XtSetSensitive(spin_place_save_rmat, False);
    XtSetSensitive(spin_place_read_rmat, False);
    reset_last_touched(xg);

    world_to_plane(xg);
    plane_to_screen(xg);
    plot_once(xg);

    refresh_vboxes(xg);
    set_varsel_label(xg);

    start_spin_proc(xg);
  }

  setToggleBitmap(XAXIS, xg->is_spin_type.xaxis);
  setToggleBitmap(YAXIS, xg->is_spin_type.yaxis);
  setToggleBitmap(OAXIS, xg->is_spin_type.oblique);
}

/* ARGSUSED */
XtCallbackProc
spin_yaxis_on_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Let the x axis be the axis of rotation.
*/
{
  if (xg->is_spin_type.yaxis) {
    xg->is_spin_type.yaxis = 0;
    stop_spin_proc(xg);
  }
  else
  {
    xg->is_spin_type.yaxis = True;
    xg->is_spin_type.xaxis = xg->is_spin_type.oblique = False;
    XtSetSensitive(spin_place_save_rmat, False);
    XtSetSensitive(spin_place_read_rmat, False);
    reset_last_touched(xg);

    world_to_plane(xg);
    plane_to_screen(xg);
    plot_once(xg);

    refresh_vboxes(xg);
    set_varsel_label(xg);

    start_spin_proc(xg);
  }
  setToggleBitmap(XAXIS, xg->is_spin_type.xaxis);
  setToggleBitmap(YAXIS, xg->is_spin_type.yaxis);
  setToggleBitmap(OAXIS, xg->is_spin_type.oblique);
}

void
toggle_interpolation(Boolean sens)
{
  XtVaSetValues(INTERP,
    XtNsensitive, sens,
    NULL);
}

/* ARGSUSED */
XtCallbackProc
spin_oblique_on_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (xg->is_spin_type.oblique) {
    stop_spin_proc(xg);

    xg->is_spin_type.oblique = False;
    xg->is_interp = False;
    toggle_interpolation(True);
  }
  else
  {
    xg->is_spin_type.oblique = True;
    xg->is_spin_type.yaxis = xg->is_spin_type.xaxis = False;
    XtSetSensitive(spin_place_save_rmat, True);
    XtSetSensitive(spin_place_read_rmat, True);
    toggle_interpolation(False);

    refresh_vboxes(xg);
    set_varsel_label(xg);

    find_plot_center(xg);
    store_Rmat(xg);
    find_Rmat(xg);
    world_to_plane(xg);
    plane_to_screen(xg);
    plot_once(xg);

    start_spin_proc(xg);
  }
  setToggleBitmap(XAXIS, xg->is_spin_type.xaxis);
  setToggleBitmap(YAXIS, xg->is_spin_type.yaxis);
  setToggleBitmap(OAXIS, xg->is_spin_type.oblique);
}

/*____Beginning of initialization section____*/

void
make_rotate(xgobidata *xg)
{
  char str[30];
  Dimension max_width;

/*
 * Widest button label used in this panel.
*/
  sprintf(str, "Change Direction");
  max_width = XTextWidth(appdata.font, str, strlen(str));

/*
 * SpinPanel:  Rotation control panel, spin_panel[0]
*/
  spin_panel[0] = XtVaCreateManagedWidget("SpinPanel",
    boxWidgetClass, xg->box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
    NULL);
  if (mono) set_mono(spin_panel[0]);

  spin_sbar = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, spin_panel[0],
    XtNwidth, (Dimension) max_width,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(spin_sbar);
  add_sbar_help(&xg->nhelpids.sbar,
    spin_sbar, "Ro_Speed");
  XawScrollbarSetThumb(spin_sbar, 10.*THETA0, -1.);
  XtAddCallback(spin_sbar, XtNjumpProc,
    (XtCallbackProc) rspeed_cback, (XtPointer) xg);

  CHDIR = CreateCommand(xg, "Change Direction",
    True, (Widget) NULL, (Widget) NULL,
    spin_panel[0], "Ro_Chdir");
  PAUSE = CreateToggle(xg, "Pause",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, ANY_OF_MANY,
    spin_panel[0], "Ro_Pause");
  REINIT = CreateCommand(xg, "Reinit",
    True, (Widget) PAUSE, (Widget) CHDIR,
    spin_panel[0], "Ro_Reinit");
  XtManageChildren(&CHDIR, 3);
  XtAddCallback(CHDIR, XtNcallback,
    (XtCallbackProc) rchdir_cback, (XtPointer) xg);
  XtAddCallback(PAUSE, XtNcallback,
    (XtCallbackProc) spin_pause_on_cback, (XtPointer) xg);
  XtAddCallback(REINIT, XtNcallback,
    (XtCallbackProc) spin_reinit_cback, (XtPointer) xg);
/*
 * spin_panel[1] contains the buttons for interpolation and rocking,
 * which need to be mutually exclusive.
*/
  spin_panel[1] = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, spin_panel[0],
    NULL);
  if (mono) set_mono(spin_panel[1]);
  ROCK = CreateToggle(xg, "Rock",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, ANY_OF_MANY,
    spin_panel[1], "Ro_Rock");
  INTERP = CreateToggle(xg, "Interpolate",
    False, (Widget) NULL, (Widget) NULL, ROCK, False, ANY_OF_MANY,
    spin_panel[1], "Ro_Interp");
  XtManageChildren(&ROCK, 2);
  XtAddCallback(ROCK, XtNcallback,
    (XtCallbackProc) rock_cback, (XtPointer) xg);
  XtAddCallback(INTERP, XtNcallback,
    (XtCallbackProc) rinterp_cback, (XtPointer) xg);
/*
 * spin_panel[2] contains the buttons for determining the axis of
 * rotation, and these need to be mutually exclusive.
*/
  spin_panel[2] = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, spin_panel[0],
    NULL);
  if (mono) set_mono(spin_panel[2]);
  YAXIS = CreateToggle(xg, "Y Axis",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, ONE_OF_MANY,
    spin_panel[2], "Ro_Axis");
  XAXIS = CreateToggle(xg, "X Axis",
    True, (Widget) NULL, (Widget) NULL, YAXIS, False, ONE_OF_MANY,
    spin_panel[2], "Ro_Axis");
  OAXIS = CreateToggle(xg, "Oblique Axis",
    True, (Widget) NULL, (Widget) NULL, YAXIS, True, ONE_OF_MANY,
    spin_panel[2], "Ro_Axis");
  XtManageChildren(&YAXIS, 3);
  XtAddCallback(YAXIS, XtNcallback,
    (XtCallbackProc) spin_yaxis_on_cback, (XtPointer) xg);
  XtAddCallback(XAXIS, XtNcallback,
    (XtCallbackProc) spin_xaxis_on_cback, (XtPointer) xg);
  XtAddCallback(OAXIS, XtNcallback,
    (XtCallbackProc) spin_oblique_on_cback, (XtPointer) xg);

  spin_place_save_coefs = CreateCommand(xg, "Save Coeffs",
    True, (Widget) NULL, (Widget) NULL,
    spin_panel[0], "Ro_SaveCoefs");
  XtManageChild(spin_place_save_coefs);
  XtAddCallback(spin_place_save_coefs, XtNcallback,
    (XtCallbackProc) spin_place_save_coefs_popup, (XtPointer) xg);

  spin_place_save_rmat = CreateCommand(xg, "Save Rotation Mtrx",
    True, (Widget) NULL, (Widget) NULL,
    spin_panel[0], "Ro_SaveRMat");
  XtManageChild(spin_place_save_rmat);
  XtAddCallback(spin_place_save_rmat, XtNcallback,
    (XtCallbackProc) spin_place_save_rmat_popup, (XtPointer) xg);

  spin_place_read_rmat = CreateCommand(xg, "Read Rotation Mtrx",
    True, (Widget) NULL, (Widget) NULL,
    spin_panel[0], "Ro_SaveRMat");
  XtManageChild(spin_place_read_rmat);
  XtAddCallback(spin_place_read_rmat, XtNcallback,
    (XtCallbackProc) spin_place_read_rmat_popup, (XtPointer) xg);
}

void
rotate_on(xgobidata *xg)
/*
 * Turn rotation on and off.
*/
{
  int j;

/*
 *  If this mode is currently selected, turn it off.
*/
  if (xg->prev_plot_mode == ROTATE_MODE && xg->plot_mode != ROTATE_MODE)
  {
    XtRemoveEventHandler(xg->workspace, XtAllEvents,
      TRUE, (XtEventHandler) ob_button, (XtPointer) xg);
    XDefineCursor(display, XtWindow(xg->workspace), default_cursor);

    stop_spin_proc(xg);
    map_spin(xg, False);
  }
/* Else turn it on */
  else if (xg->prev_plot_mode != ROTATE_MODE &&
           xg->plot_mode == ROTATE_MODE)
  {
    XtAddEventHandler(xg->workspace,
      ButtonPressMask | ButtonReleaseMask |
      Button1MotionMask | Button2MotionMask,
      FALSE, (XtEventHandler) ob_button, (XtPointer) xg);
    XDefineCursor(display, XtWindow(xg->workspace), spin_cursor);

    if (!xg->is_spinning)
    {
      if (xg->is_plotting1d)
        free_txtr_var();
      else if (xg->is_touring && xg->is_princ_comp)
	set_sph_labs(xg, xg->nsph_vars);
/*        reset_var_labels(xg, PRINCCOMP_OFF);i think not needed anymore
                                             sphering transformation*/

      if (xg->carry_vars)
        carry_spin_vars(xg);

      xg->is_plotting1d = xg->is_xyplotting = False;
      xg->is_touring = False;
      xg->is_corr_touring = False;
      xg->is_spinning = True;

      find_plot_center(xg);
      update_lims(xg);
      update_world(xg);
      world_to_plane(xg);
      plane_to_screen(xg);
      plot_once(xg);

      for (j=0; j<xg->ncols_used; j++)
      {
        if (j == xg->spin_vars.x ||
          j == xg->spin_vars.y ||
          j == xg->spin_vars.z)
            xg->varchosen[j] = True;
        else
            xg->varchosen[j] = False;
      }
      refresh_vboxes(xg);
      set_varsel_label(xg);
    }

    map_spin(xg, True);
    /*
     * If we enter the rotation mode with is_spin_paused = False
     * and theta0 > 0, start spinning.
    */
    start_spin_proc(xg);
  }
}

#undef CHDIR
#undef PAUSE
#undef REINIT
#undef ROCK
#undef INTERP
#undef YAXIS
#undef XAXIS
#undef OAXIS
