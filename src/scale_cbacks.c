/* scale_cbacks.c */
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

#include <stdlib.h>
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

static Widget scale_shift_panel, scale_type_cmd ;
static Widget scale_panel[2], scale_cmd[6], shift_cmd[4] ;
static Widget scale_reset_cmd;
static Widget shift_panel[2], shift_reset_cmd ;

void
map_scaling(xgobidata *xg, Boolean scaling)
{
  if (scaling) {
    XtMapWidget(scale_shift_panel);
    XtMapWidget(xg->scale_mouse);
  }
  else {
    XtUnmapWidget(scale_shift_panel);
    XtUnmapWidget(xg->scale_mouse);
  }
}

/* ARGSUSED */
XtCallbackProc
scale_type_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  xg->plot_square = !xg->plot_square ;
  plane_to_screen(xg);

  if (xg->is_xyplotting)
    init_ticks(&xg->xy_vars, xg);
  else if (xg->is_plotting1d)
    init_ticks(&xg->plot1d_vars, xg);

  plot_once(xg);

  setToggleBitmap(w, xg->plot_square);
}

/* ARGSUSED */
XtEventHandler
scale_release_cback(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
    xg->run_scale_proc = False;
}

/* ARGSUSED */
XtCallbackProc
scale_reset_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Reset the screen image to original scale.
*/
{
  xg->scale.x = xg->scale0.x;
  xg->scale.y = xg->scale0.y;

  plane_to_screen(xg);

  if (xg->is_xyplotting) {
    convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
    convert_ticks(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
    extend_axes(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);

  } else if (xg->is_plotting1d) {
    convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
    convert_ticks(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
    extend_axes(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
  }

  plot_once(xg);
}

/* ARGSUSED */
XtEventHandler
shift_release_cback(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
    xg->run_shift_proc = False;
}

/* ARGSUSED */
XtCallbackProc
shift_reset_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Reset the screen image to original center.
*/
{
  xg->shift_wrld.x = 0;
  xg->shift_wrld.y = 0;
  find_plot_center(xg);
  plane_to_screen(xg);

  if (xg->is_xyplotting) {
    convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
    convert_ticks(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
    extend_axes(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
  }

  else if (xg->is_plotting1d) {
    convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
    convert_ticks(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
    extend_axes(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
  }

  plot_once(xg);
}

/*____Initialization section____*/

/* ARGSUSED */
XtEventHandler
scale_press_cback(Widget w, xgobidata *xg, XEvent *evt, Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evt;
  int j;

  if (xbutton->button == 1 || xbutton->button == 2) {
    for (j=0; j<6; j++) {
      if (scale_cmd[j] == w) {
        xg->scaling_btn = j;
        break;
      }
    }
    reinit_nsteps();
    xg->run_scale_proc = True;
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
  }
}

/* ARGSUSED */
XtEventHandler
shift_press_cback(Widget w, xgobidata *xg, XEvent *evt, Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evt;
  int j;

  if (xbutton->button == 1 || xbutton->button == 2) {
    for (j=0; j<4; j++) {
      if (shift_cmd[j] == w) {
        xg->scaling_btn = j;
        break;
      }
    }
    reinit_nsteps();
    xg->run_shift_proc = True;
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
  }
}

/*ARGSUSED*/
XtActionProc
StopScaleByArrow(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;
  xg->run_scale_proc = False;
}

/*ARGSUSED*/
XtActionProc
StartScaleByArrow(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;

  int indx = atoi(params[0]);
  if (indx >= 0 && indx <= 5) {
    xg->scaling_btn = indx;
    reinit_nsteps();
    xg->run_scale_proc = True;
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
  }
}

void
make_scaling(xgobidata *xg)
{
  int j;
  char *accelerators;
  accelerators = XtMalloc(512 * sizeof(char));
  sprintf(accelerators,
    "<KeyPress>%s:StartScaleByArrow(%d)\n\
     <KeyRelease>%s:StopScaleByArrow(%d)\n\
     <KeyPress>%s:StartScaleByArrow(%d)\n\
     <KeyRelease>%s:StopScaleByArrow(%d)\n\
     <KeyPress>%s:StartScaleByArrow(%d)\n\
     <KeyRelease>%s:StopScaleByArrow(%d)\n\
     <KeyPress>%s:StartScaleByArrow(%d)\n\
     <KeyRelease>%s:StopScaleByArrow(%d)\n\
     <KeyPress>%s:StartScaleByArrow(%d)\n\
     <KeyRelease>%s:StopScaleByArrow(%d)\n\
     <KeyPress>%s:StartScaleByArrow(%d)\n\
     <KeyRelease>%s:StopScaleByArrow(%d)",
     "Up",    0,
     "Up",    0,
     "plus",  1,
     "plus",  1,
     "Right", 2,
     "Right", 2,
     "Down",  3,
     "Down",  3,
     "minus", 4,
     "minus", 4,
     "Left",  5,
     "Left",  5);

/*
 * ScaleShiftPanel
*/
  scale_shift_panel = XtVaCreateManagedWidget("ScaleShiftPanel",
    boxWidgetClass, xg->box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
    NULL);
  if (mono) set_mono(scale_shift_panel);

/*
 * ScalePanel
*/
  scale_panel[0] = XtVaCreateManagedWidget("ScalePanel",
    boxWidgetClass, scale_shift_panel,
    NULL);
  if (mono) set_mono(scale_panel[0]);
/*
 * Reset scale control button
*/
  scale_reset_cmd = CreateCommand(xg, "Reset Scale",
    True, (Widget) NULL, (Widget) NULL,
    scale_panel[0], "Sc_ResetScale");
  XtManageChild(scale_reset_cmd);
/*
 * Panel of arrow and pushbutton widgets for scale control
*/
  scale_panel[1] = XtVaCreateManagedWidget("Panel",
    formWidgetClass, scale_panel[0],
    XtNdefaultDistance, (Dimension) 0,
    XtNaccelerators, XtParseAcceleratorTable(accelerators),
    NULL);
  if (mono) set_mono(scale_panel[1]);

  scale_cmd[0] = XtVaCreateWidget("Icon",
    labelWidgetClass, scale_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) uparr,
    NULL);
  if (mono) set_mono(scale_cmd[0]);
  add_pb_help(&xg->nhelpids.pb,
    scale_cmd[0], "Sc_ScaleCtrl");

  scale_cmd[1] = XtVaCreateWidget("Icon",
    labelWidgetClass, scale_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromHoriz, (Widget) scale_cmd[0],
    XtNbitmap, (Pixmap) plus,
    NULL);
  if (mono) set_mono(scale_cmd[1]);
  add_pb_help(&xg->nhelpids.pb, scale_cmd[1], "Sc_ScaleCtrl");

  scale_cmd[2] = XtVaCreateWidget("Icon",
    labelWidgetClass, scale_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) rightarr,
    XtNfromHoriz, (Widget) scale_cmd[1],
    NULL);
  if (mono) set_mono(scale_cmd[2]);
  add_pb_help(&xg->nhelpids.pb,
    scale_cmd[2], "Sc_ScaleCtrl");

  scale_cmd[3] = XtVaCreateWidget("Icon",
    labelWidgetClass, scale_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) downarr,
    XtNfromVert, (Widget) scale_cmd[0],
    NULL);
  if (mono) set_mono(scale_cmd[3]);
  add_pb_help(&xg->nhelpids.pb,
    scale_cmd[3], "Sc_ScaleCtrl");

  scale_cmd[4] = XtVaCreateWidget("Icon",
    labelWidgetClass, scale_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) minus,
    XtNfromVert, (Widget) scale_cmd[1],
    XtNfromHoriz, (Widget) scale_cmd[3],
    NULL);
  if (mono) set_mono(scale_cmd[4]);
  add_pb_help(&xg->nhelpids.pb, scale_cmd[4], "Sc_ScaleCtrl");

  scale_cmd[5] = XtVaCreateWidget("Icon",
    labelWidgetClass, scale_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) leftarr,
    XtNfromVert, (Widget) scale_cmd[2],
    XtNfromHoriz, (Widget) scale_cmd[4],
    NULL);
  if (mono) set_mono(scale_cmd[5]);
  add_pb_help(&xg->nhelpids.pb,
    scale_cmd[5], "Sc_ScaleCtrl");

  XtManageChildren(scale_cmd, 6);
  XtFree((char *) accelerators);

/*
 * Make square plot or use max window size.
*/
   scale_type_cmd = CreateToggle(xg, "Aspect 1:1", True,
    (Widget) NULL, (Widget) scale_panel[1], (Widget) NULL,
    xg->plot_square, ANY_OF_MANY, scale_panel[0], "Sc_PlotSquare");
  XtManageChild(scale_type_cmd);
  XtAddCallback(scale_type_cmd, XtNcallback,
    (XtCallbackProc) scale_type_cback, (XtPointer) xg);

/*
 * ShiftPanel
*/
  shift_panel[0] = XtVaCreateManagedWidget("ShiftPanel",
    boxWidgetClass, scale_shift_panel,
    NULL);
  if (mono) set_mono(shift_panel[0]);
/*
 * Shift reset control button
*/
  shift_reset_cmd = CreateCommand(xg, "Reset Shift",
    True, (Widget) NULL, (Widget) NULL,
    shift_panel[0], "Sc_ResetShift");
  XtManageChild(shift_reset_cmd);
/*
 * Panel of pushbutton and arrow widgets for shift control.
*/
  shift_panel[1] = XtVaCreateManagedWidget("Panel",
    formWidgetClass, shift_panel[0],
    XtNdefaultDistance, (Dimension) 0,
    NULL);
  if (mono) set_mono(shift_panel[1]);

  shift_cmd[0] = XtVaCreateWidget("Icon",
    labelWidgetClass, shift_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) uparr,
    NULL);
  if (mono) set_mono(shift_cmd[0]);
  add_pb_help(&xg->nhelpids.pb,
    shift_cmd[0], "Sc_ShiftCtrl");

  shift_cmd[1] = XtVaCreateWidget("Icon",
    labelWidgetClass, shift_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromHoriz, (Widget) shift_cmd[0],
    XtNbitmap, (Pixmap) rightarr,
    NULL);
  if (mono) set_mono(shift_cmd[1]);
  add_pb_help(&xg->nhelpids.pb,
    shift_cmd[1], "Sc_ShiftCtrl");

  shift_cmd[2] = XtVaCreateWidget("Icon",
    labelWidgetClass, shift_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromVert, (Widget) shift_cmd[0],
    XtNbitmap, (Pixmap) downarr,
    NULL);
  if (mono) set_mono(shift_cmd[2]);
  add_pb_help(&xg->nhelpids.pb,
    shift_cmd[2], "Sc_ShiftCtrl");

  shift_cmd[3] = XtVaCreateWidget("Icon",
    labelWidgetClass, shift_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromHoriz, (Widget) shift_cmd[2],
    XtNfromVert, (Widget) shift_cmd[1],
    XtNbitmap, (Pixmap) leftarr,
    NULL);
  if (mono) set_mono(shift_cmd[3]);
  add_pb_help(&xg->nhelpids.pb,
    shift_cmd[3], "Sc_ShiftCtrl");

  XtManageChildren(shift_cmd, 4);

/*
 * Scaling and shifting callbacks
*/
  for (j=0; j<6; j++) {
    XtAddEventHandler(scale_cmd[j], ButtonPressMask,
      FALSE, (XtEventHandler) scale_press_cback, (XtPointer) xg);
    XtAddEventHandler(scale_cmd[j], ButtonReleaseMask,
      FALSE, (XtEventHandler) scale_release_cback, (XtPointer) xg);
  }

  for (j=0; j<4; j++) {
    XtAddEventHandler(shift_cmd[j], ButtonPressMask,
      FALSE, (XtEventHandler) shift_press_cback, (XtPointer) xg);
    XtAddEventHandler(shift_cmd[j], ButtonReleaseMask,
      FALSE, (XtEventHandler) shift_release_cback, (XtPointer) xg);
  }

  XtAddCallback(scale_reset_cmd, XtNcallback,
    (XtCallbackProc) scale_reset_cback, (XtPointer) xg);
  XtAddCallback(shift_reset_cmd, XtNcallback,
    (XtCallbackProc) shift_reset_cback, (XtPointer) xg);


  make_stdview(xg, scale_shift_panel);
}

void
scaling_on(xgobidata *xg)
{
  if (xg->prev_plot_mode == SCALE_MODE && xg->plot_mode != SCALE_MODE) {
    /*
     * Remove event handler for button presses in the workspace widget.
    */
    XtRemoveEventHandler(xg->workspace,
      XtAllEvents, TRUE,
      (XtEventHandler) scale_button_event, (XtPointer) xg);
    xg->is_scaling = False;
    map_scaling(xg, False);
    XUndefineCursor(display, XtWindow(xg->workspace));
  }
  else if (xg->prev_plot_mode != SCALE_MODE &&
           xg->plot_mode == SCALE_MODE)
  {
    XDefineCursor(display, XtWindow(xg->workspace), scale_cursor);
    /*
     * Event handler for button presses in the workspace widget.
    */
    XtAddEventHandler(xg->workspace, ButtonPressMask | ButtonReleaseMask,
      FALSE, (XtEventHandler) scale_button_event, (XtPointer) xg);
    xg->is_scaling = 1;
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);

    map_scaling(xg, True);
  }

  quickplot_once(xg);  /* to add or remove crosshair from view */
}
