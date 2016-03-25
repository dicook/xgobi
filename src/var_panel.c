/* var_panel.c */
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

#include "../bitmaps/xy.xbm"

extern xgobidata xgobi;

static Widget var_mouse, var_mouse_tform, var_mouse_tour, var_mouse_corr;
static Widget var_mouse_oblique, var_mouse_plot1d;
static int cursor_in_panel = 0 ;
static int last_touched = 0 ;

static int focus_var = -1;

static struct {
	Widget cascade;
	Widget btn[10];
} varsel_menu;

/* ARGSUSED */
XtEventHandler
set_focus_var(Widget w, xgobidata *xg, XEvent *evnt)
{
  int j;
  for (j=0; j<xg->ncols_used; j++)
    if (xg->vardraww[j] == w)
      break;

  focus_var = (evnt->type == EnterNotify) ? j : -1;
  refresh_vbox(xg, j, False);
}

/* ARGSUSED */
XtEventHandler
set_var_tform_label(Widget w, xgobidata *xg, XEvent *evnt)
{
  if (evnt->type == EnterNotify) {
    XtUnmapWidget(var_mouse);
    XtUnmapWidget(var_mouse_plot1d);
    XtUnmapWidget(var_mouse_oblique);
    XtUnmapWidget(var_mouse_tour);
    XtUnmapWidget(var_mouse_corr);
    XtMapWidget(var_mouse_tform);
  }
  else if (evnt->type == LeaveNotify)
  {
    XtUnmapWidget(var_mouse_tform);
    if (xg->is_touring && !xg->is_backtracking && !xg->is_local_scan)
      XtMapWidget(var_mouse_tour);
    else if (xg->is_spinning && xg->is_spin_type.oblique)
      XtMapWidget(var_mouse_oblique);
    else if (xg->is_plotting1d)
      XtMapWidget(var_mouse_plot1d);
    else if (xg->is_corr_touring && !(corr_backtracking(xg)))
      XtMapWidget(var_mouse_corr);
    else if (xg->is_xyplotting ||
        (xg->is_spinning && !xg->is_spin_type.oblique))
      XtMapWidget(var_mouse);
  }
}

void
set_varsel_label(xgobidata *xg)
/*
 * Set the label above the variable selection panel.
*/
{
  if (xg->is_touring)
  {
    XtUnmapWidget(var_mouse);
    XtUnmapWidget(var_mouse_oblique);
    XtUnmapWidget(var_mouse_plot1d);
    XtUnmapWidget(var_mouse_corr);
    if (xg->is_backtracking || xg->is_local_scan)
      XtUnmapWidget(var_mouse_tour);
    else
      XtMapWidget(var_mouse_tour);
  }
  else if (xg->is_corr_touring)
  {
    XtUnmapWidget(var_mouse);
    XtUnmapWidget(var_mouse_oblique);
    XtUnmapWidget(var_mouse_plot1d);
    XtUnmapWidget(var_mouse_tour);
    if (corr_backtracking(xg))
      XtUnmapWidget(var_mouse_corr);
    else
      XtMapWidget(var_mouse_corr);
  }
  else if (xg->is_spinning && xg->is_spin_type.oblique)
  {
    XtUnmapWidget(var_mouse);
    XtUnmapWidget(var_mouse_plot1d);
    XtUnmapWidget(var_mouse_tour);
    XtUnmapWidget(var_mouse_corr);
    XtMapWidget(var_mouse_oblique);
  }
  else if (xg->is_xyplotting ||
       (xg->is_spinning && !xg->is_spin_type.oblique))
  {
    XtUnmapWidget(var_mouse_tour);
    XtUnmapWidget(var_mouse_corr);
    XtUnmapWidget(var_mouse_plot1d);
    XtUnmapWidget(var_mouse_oblique);
    XtMapWidget(var_mouse);
  }
  else if (xg->is_plotting1d)
  {
    XtUnmapWidget(var_mouse_tour);
    XtUnmapWidget(var_mouse_corr);
    XtUnmapWidget(var_mouse_oblique);
    XtUnmapWidget(var_mouse);
    XtMapWidget(var_mouse_plot1d);
  }
}

int
spin_varsel_ax_from_menu(int varno, int button, xgobidata *xg)
{
/*
 * Here x and y are plotted horizontally; z is plotted vertically.
 * or
 * x and y are plotted vertically; z is plotted horizontally.
*/
  int prev = -1, newvar = 1;

  if (button == 1) {  /* new x variable */

    if (varno == xg->spin_vars.x) 
      ;
    else if (varno == xg->spin_vars.z) /* exchange the two */
      xg->spin_vars.z = xg->spin_vars.x; 
    else if (varno == xg->spin_vars.y) /* exchange the two */
      xg->spin_vars.y = xg->spin_vars.x;
    else {  /* new variable */
      prev = xg->spin_vars.x;
    }

    xg->spin_vars.x = varno;
    last_touched = varno;

  } else if (button == 2) {  /* new y variable */

    if (varno == xg->spin_vars.y) 
      ;
    else if (varno == xg->spin_vars.x) { /* exchange the two */
      xg->spin_vars.x = xg->spin_vars.y; 
    } else if (varno == xg->spin_vars.z) { /* exchange the two */
      xg->spin_vars.z = xg->spin_vars.y;
    } else {  /* new variable */
      prev = xg->spin_vars.y;
    }

    xg->spin_vars.y = varno;
    last_touched = varno;

  } else if (button == 3) {  /* new z variable */

    if (varno == xg->spin_vars.z)
      newvar = 0;

    else if (varno == xg->spin_vars.x) {
      last_touched = xg->spin_vars.x = xg->spin_vars.z;
    } else if (varno == xg->spin_vars.y) {
      last_touched = xg->spin_vars.y = xg->spin_vars.z;
    } else {
      prev = xg->spin_vars.z;
    }
    xg->spin_vars.z = varno;
  }

  if (prev != -1) {
    xg->varchosen[prev] = False;
    refresh_vbox(xg, prev, 1);
    xg->varchosen[varno] = True;
  }

  return(newvar);
}

int
spin_varsel_oblique_from_menu(int varno, int button, xgobidata *xg)
{
  int prev = -1, newvar = 1;

  if (button == 1) {  /* new x variable */
    if (varno == xg->spin_vars.x)
      newvar = 0;
    else if (varno == xg->spin_vars.y)
      xg->spin_vars.y = xg->spin_vars.x;
    else if (varno == xg->spin_vars.z)
      xg->spin_vars.z = xg->spin_vars.x;
    else
      prev = xg->spin_vars.x;

    xg->spin_vars.x = varno;

  } else if (button == 2) {  /* new y variable */
    if (varno == xg->spin_vars.y)
      newvar = 0;
    else if (varno == xg->spin_vars.x)
      xg->spin_vars.x = xg->spin_vars.y;
    else if (varno == xg->spin_vars.z)
      xg->spin_vars.z = xg->spin_vars.y;
    else
      prev = xg->spin_vars.y;

    xg->spin_vars.y = varno;

  } else if (button == 3) {  /* new z variable */
    if (varno == xg->spin_vars.z)
      newvar = 0;
    else if (varno == xg->spin_vars.x)
      xg->spin_vars.x = xg->spin_vars.z;
    else if (varno == xg->spin_vars.y)
      xg->spin_vars.y = xg->spin_vars.z;
    else
      prev = xg->spin_vars.z;

    xg->spin_vars.z = varno;
  }

  last_touched = varno;

  if (prev != -1) {
    xg->varchosen[prev] = False;
    refresh_vbox(xg, prev, 1);
    xg->varchosen[varno] = True;
  }

  return(newvar);
}


/* ARGSUSED */
static XtCallbackProc
varsel_choose_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Widget label = XtParent(XtParent(w));
  int varno = -1;
  Boolean newvar = false;

  for (varno=0; varno<xg->ncols_used; varno++)
    if (xg->varlabw[varno] == label)
      break;

  if (xg->is_plotting1d) {
    extern Boolean plot1d_vertically;
    plot1d_vertically = (varsel_menu.btn[1] == w);
    newvar = set_plot1dvar(xg, varno);
    if (newvar)
      plot1d_texture_var(xg);
  }
  else if (xg->is_xyplotting) {

    if (varsel_menu.btn[0] == w)
      newvar = set_xyplotxvar(xg, varno);
    else if (varsel_menu.btn[1] == w)
      newvar = set_xyplotyvar(xg, varno);
  }
  else if (xg->is_touring) {

    if (varsel_menu.btn[0] == w) {
      set_tourvar(xg, varno);
    }
    else if (varsel_menu.btn[1] == w) {
      set_manip_var(xg, varno);
    }
    else if (varsel_menu.btn[2] == w) {
      set_frozen_var(xg, varno);
    }
  }
  else if (xg->is_corr_touring) {
    if (varsel_menu.btn[0] == w) {
      set_xcorrvar(xg, varno);
    }
    else if (varsel_menu.btn[1] == w) {
      set_ycorrvar(xg, varno);
    }
    else if (varsel_menu.btn[2] == w) {
      set_cmanip_var(xg, 1, varno);
    }
    else if (varsel_menu.btn[3] == w) {
      set_cmanip_var(xg, 2, varno);
    }
    else if (varsel_menu.btn[5] == w) {
      set_cxfrozen_var(xg, varno);
    }
    else if (varsel_menu.btn[6] == w) {
      set_cyfrozen_var(xg, varno);
    }
  } else if (xg->is_spinning) {
    int menu_button;
    if (varsel_menu.btn[0] == w) menu_button = 1;
    else if (varsel_menu.btn[1] == w) menu_button = 2;
    else menu_button = 3;

/*
 * If I alter the code such that both yaxis and xaxis rotate
 * around the z axis, then I don't need separate variable
 * selection routines for them.
*/
    if (xg->is_spin_type.yaxis)
      spin_varsel_ax_from_menu(varno, menu_button, xg);
    else if (xg->is_spin_type.xaxis)
      spin_varsel_ax_from_menu(varno, menu_button, xg);
    else if (xg->is_spin_type.oblique)
      spin_varsel_oblique_from_menu(varno, menu_button, xg);

    refresh_vbox(xg, xg->spin_vars.x, 1);
    refresh_vbox(xg, xg->spin_vars.y, 1);
    refresh_vbox(xg, xg->spin_vars.z, 1);
  }

  if (newvar)
  {
    world_to_plane(xg);
    plane_to_screen(xg);

    if (xg->is_xyplotting)
      init_ticks(&xg->xy_vars, xg);
    else if (xg->is_plotting1d)
      init_ticks(&xg->plot1d_vars, xg);

    /*
     * If brushing, screen coordinates just changed and
     * need to re-bin points before plotting.
    */
    if (xg->is_brushing) {
      assign_points_to_bins(xg);

      if (xg->brush_mode == transient)
        reinit_transient_brushing(xg);
    }

    plot_once(xg);
  }

  XtDestroyWidget((Widget) varsel_menu.cascade);
}

/* ARGSUSED */
XtEventHandler
rm_varsel_menu(Widget w, xgobidata *xg, XEvent *evnt)
/*
 * Remove menus for choosing a data transformation.
*/
{
  if ((Widget) varsel_menu.cascade != NULL)
    XtDestroyWidget((Widget) varsel_menu.cascade);
}

/* ARGSUSED */
XtEventHandler
add_varsel_menu(Widget w, xgobidata *xg, XEvent *evnt)
/*
 * Add menus for choosing a data transformation.
*/
{
  int k, varno, nitems;
  static char *varsel_menu_name[10];

  if (xg->is_plotting1d)
  {
    varsel_menu_name[0] = "Select in X   L";
    varsel_menu_name[1] = "Select in Y   M",
    nitems = 2;
  }
  else if (xg->is_xyplotting) {
    varsel_menu_name[0] =  "Select as X   L";
    varsel_menu_name[1] =  "Select as Y   M";
    nitems = 2;
  }
  else if (xg->is_touring)
  {
    varsel_menu_name[0] =  "Tour  L,M";
    varsel_menu_name[1] =  "Manip  <Shft> L,M";
    varsel_menu_name[2] =  "Freeze  <Ctrl> L,M";
    nitems = 3;
  }
  else if (xg->is_corr_touring) {
    varsel_menu_name[0] =  "Tour X   L";
    varsel_menu_name[1] =  "Tour Y   M";
    varsel_menu_name[2] =  "Manip X  <Shft> L";
    varsel_menu_name[3] =  "Manip Y  <Shft> M";
    varsel_menu_name[4] =  "Freeze X  <Ctrl> L";
    varsel_menu_name[5] =  "Freeze Y <Ctrl> M";
    nitems = 6;
  }
  else if (xg->is_spinning) {
    varsel_menu_name[0] =  "X";
    varsel_menu_name[1] =  "Y";
    varsel_menu_name[2] =  "Z";
    nitems = 3;
  }

  for (varno=0; varno<xg->ncols; varno++)
    if (xg->varlabw[varno] == w)
      break;

  varsel_menu.cascade = XtVaCreatePopupShell("menu",
    simpleMenuWidgetClass, xg->varlabw[varno],
    XtNinput, True,
    NULL);
  if (mono) set_mono(varsel_menu.cascade);

  for (k=0; k<nitems; k++) {
    varsel_menu.btn[k] = XtVaCreateManagedWidget("Command",
      smeBSBObjectClass, varsel_menu.cascade,
      XtNlabel, (String) varsel_menu_name[k],
      XtNleftMargin, (Dimension) 5,
      NULL);
    if (mono) set_mono(varsel_menu.btn[k]);
    XtAddCallback(varsel_menu.btn[k], XtNcallback,
      (XtCallbackProc) varsel_choose_cback, (XtPointer) xg);
  }

}

void
make_varboxes(xgobidata *xg)
{
  int j;

  for (j=0; j<xg->ncols-1; j++)
  {
    xg->varboxw[j] = XtVaCreateWidget("VarForm",
      boxWidgetClass, xg->var_panel,
      NULL);
    if (mono) set_mono(xg->varboxw[j]);
  }
  /*
   * Create a variable box for the variable that can
   * be defined in brushing,
  */
  xg->varboxw[xg->ncols-1] = XtVaCreateWidget("VarForm",
    boxWidgetClass, xg->var_panel,
    XtNmappedWhenManaged, (Boolean) False,
    NULL);
  if (mono) set_mono(xg->varboxw[xg->ncols-1]);

  for (j=0; j<xg->ncols; j++) {
    xg->varlabw[j] = XtVaCreateManagedWidget("VarLabel",
      menuButtonWidgetClass, xg->varboxw[j],
      XtNlabel, (String) xg->collab[j],
      /*XtNleftBitmap, (Pixmap) NULL,*/
      NULL);
    if (mono) set_mono(xg->varlabw[j]);

    add_menupb_help(&xg->nhelpids.menupb,
      xg->varlabw[j], "VarPanel");

    XtAddEventHandler(xg->varlabw[j],
      ButtonPressMask, FALSE,
      (XtEventHandler) add_varsel_menu, 
      (XtPointer) xg);
    XtAddEventHandler(xg->varlabw[j],
      ButtonReleaseMask, FALSE,
      (XtEventHandler) rm_varsel_menu, 
      (XtPointer) xg);
  }

  for (j=0; j<xg->ncols; j++) {
    xg->vardraww[j] = XtVaCreateManagedWidget("VarWindow",
      labelWidgetClass, xg->varboxw[j],
      XtNfromVert, (Widget) xg->varlabw[j],
      XtNlabel, (String) "",
      NULL);
    if (mono) set_mono(xg->vardraww[j]);

    if (j == xg->ncols-1)
      XtVaSetValues(xg->vardraww[j], XtNborderWidth, 1, NULL);

    add_pb_help(&xg->nhelpids.pb,
      xg->vardraww[j], "VarPanel");

    XtAddEventHandler(xg->vardraww[j],
      ExposureMask, FALSE,
      (XtEventHandler) varexpose, (XtPointer) xg);
    XtAddEventHandler(xg->vardraww[j],
      ButtonPressMask | ShiftMask | ControlMask | KeyPressMask | 
      KeyReleaseMask, FALSE,
      (XtEventHandler) varselect, (XtPointer) xg);
    XtAddEventHandler(xg->vardraww[j],
      EnterWindowMask | LeaveWindowMask, FALSE,
      (XtEventHandler) set_focus_var, (XtPointer) xg);
  }

  xg->varchosen[0] = True;
  xg->varchosen[1] = True;
  for (j=2; j<xg->ncols; j++)
    xg->varchosen[j] = False;
}

void
make_var_mouses(xgobidata *xg)
{
/*
 * VarMouseLabel: the variable selection mouse label for use in
 * xyplotting and axis rotation and correlation tour (var_mouse),
 * grand tour (var_mouse_tour),
 * oblique rotation (var_mouse_oblique),
 * and the var transformation mouse label (var_mouse_tform).
*/
  Arg args[5];

  XtSetArg(args[0], XtNfromVert, xg->var_panel);
  XtSetArg(args[1], XtNtop, XtChainBottom);
  XtSetArg(args[2], XtNbottom, XtChainBottom);

  XtSetArg(args[3], XtNlabel, "L: Select X,  M: Y ");
  var_mouse = XtCreateManagedWidget("VarMouseLabel",
    labelWidgetClass, xg->box2, args, 4);
  if (mono) set_mono(var_mouse);

/* The remainder are not initially mapped */
  XtSetArg(args[3], XtNmappedWhenManaged, False);

  XtSetArg(args[4], XtNlabel, "L: Select X,  M: Y ");
  var_mouse_plot1d = XtCreateManagedWidget("VarMouseLabel",
    labelWidgetClass, xg->box2, args, 5);
  if (mono) set_mono(var_mouse_plot1d);

  XtSetArg(args[4], XtNlabel, "L/M: Select Variable ");
  var_mouse_oblique = XtCreateManagedWidget("VarMouseLabel",
    labelWidgetClass, xg->box2, args, 5);
  if (mono) set_mono(var_mouse_oblique);

  XtSetArg(args[4], XtNlabel, "L/M: Toggle (see Help)");
  var_mouse_tour = XtCreateManagedWidget("VarMouseLabel",
    labelWidgetClass, xg->box2, args, 5);
  if (mono) set_mono(var_mouse_tour);

  XtSetArg(args[4], XtNlabel, "L: X, M: Y (see Help)");
  var_mouse_corr = XtCreateManagedWidget("VarMouseLabel",
    labelWidgetClass, xg->box2, args, 5);
  if (mono) set_mono(var_mouse_corr);

  XtSetArg(args[4], XtNlabel, "L: Transform Variable ");
  var_mouse_tform = XtCreateManagedWidget("VarMouseLabel",
    labelWidgetClass, xg->box2, args, 5);
  if (mono) set_mono(var_mouse_tform);
}

void
make_varpanel(xgobidata *xg)
{
  int j;
  Dimension width, height;

/*
 * VarPanel: contains all variable selection widgets.
*/
  XtVaGetValues(xg->workspace, XtNheight, &height, NULL);

  xg->var_panel = XtVaCreateManagedWidget("VarPanel",
    boxWidgetClass, xg->box2,
    XtNheight, (Dimension) height,
    /*XtNfromVert, (Widget) var_mouse,*/
    NULL);
  if (mono) set_mono(xg->var_panel);

  xg->varboxw = (Widget *) XtMalloc(
    (Cardinal) xg->ncols * sizeof(Widget));
  xg->varlabw = (Widget *) XtMalloc(
    (Cardinal) xg->ncols * sizeof(Widget));
  xg->vardraww = (Widget *) XtMalloc(
    (Cardinal) xg->ncols * sizeof(Widget));
  xg->varchosen = (Boolean *) XtMalloc(
    (Cardinal) xg->ncols * sizeof(Boolean));

  make_varboxes(xg);
  XtManageChildren(xg->varboxw, (unsigned int) xg->ncols);

/*
 * Force the variable drawing area to be square; assign the
 * value of radius which is used in drawing the lines.
*/
  XtVaGetValues(xg->vardraww[0], XtNwidth, &width, NULL);
  xg->radius = width/2 - 1;

  for (j=0; j<xg->ncols; j++)
    XtVaSetValues(xg->vardraww[j], XtNheight, (Dimension) width, NULL);

  make_var_mouses(xg);
}

void
draw_last_touched(xgobidata *xg)
/*
 * If the cursor is inside the var_panel widget, then draw the
 * circle indicating which variable is last_touched.
*/
{
  if (cursor_in_panel)
  {
    XDrawArc(display, XtWindow(xg->vardraww[last_touched]),
      varpanel_copy_GC,
      (int) (xg->radius - 5), (int) (xg->radius - 5),
      (unsigned int) 10, (unsigned int) 10,
      0, 360*64);
  }
}

void
get_vlab_colors(xgobidata *xg, WidgetColors *vlab_colors)
{
  int j;

  XtVaGetValues(xg->varlabw[0],
    XtNforeground, &vlab_colors->fg,
    XtNbackground, &vlab_colors->bg,
    XtNborderColor, &vlab_colors->border, NULL);

  for (j=1; j<xg->ncols_used; j++) {
    if (xg->varchosen[j] == False) {
      XtVaGetValues(xg->varlabw[j],
        XtNforeground, &vlab_colors->fg,
        XtNbackground, &vlab_colors->bg,
        XtNborderColor, &vlab_colors->border, NULL);
      break;
    }
  }
}

void
refresh_vlab(int varno, xgobidata *xg)
/*
 * this routine will check what highlighting the variable has
 * and will change it or leave it the same according to whether
 * caller wants a highlighted or a dehighlighted label.
*/
{
  static int init = 0;
  Boolean chosen = xg->varchosen[varno];
  static WidgetColors vlab_colors;

  if (!init)
  {
    get_vlab_colors(xg, &vlab_colors);
    init = 1;
  }

  if (chosen)
  {
    XtVaSetValues(xg->varlabw[varno],
      XtNforeground, vlab_colors.bg,
      XtNbackground, vlab_colors.fg, NULL);
  }
  else
  {
    XtVaSetValues(xg->varlabw[varno],
      XtNforeground, vlab_colors.fg,
      XtNbackground, vlab_colors.bg, NULL);
  }
}

void
refresh_vbox(xgobidata *xg, int varno, int do_label)
/*
 * the purpose of this function is to refresh the variable bars
 * appropriately when changing between modes
 * and to refresh the labels.
*/
{
  int x, y;
  Widget vwsp = xg->vardraww[varno];
  Window vwin = XtWindow(vwsp);
  unsigned int r = xg->radius;
  XGCValues *gcv, gcv_inst;
  Boolean initd = False;

  if (!initd) {
    int j;
    long varfg, varbg;
    Pixmap xyp;
    XColor fc, bc;
    Colormap cmap = DefaultColormap(display, DefaultScreen(display));

    xyp = XCreateBitmapFromData(display,
        DefaultRootWindow(display),
        (char *) xy_bits, xy_width, xy_height);

    XtVaGetValues(vwsp,
      XtNforeground, &varfg,
      XtNbackground, &varbg,
      NULL);

    fc.pixel = varfg;
    bc.pixel = varbg;
    XQueryColor(display, cmap, &fc);
    XQueryColor(display, cmap, &bc);

    xy_cursor = XCreatePixmapCursor(display, xyp, xyp,
      &fc, &bc, 1, 1);

    for (j=0; j<xg->ncols; j++)
      XDefineCursor(display, XtWindow(xg->vardraww[j]), xy_cursor);

    initd = True;
  }

  XGetGCValues(display, varpanel_xor_GC, GCCapStyle|GCJoinStyle, &gcv_inst);
  gcv = &gcv_inst;
  XClearWindow(display, vwin);

  if (varno == focus_var)
    XSetLineAttributes(display, varpanel_xor_GC, 3, LineSolid,
      gcv->cap_style, gcv->join_style);
  else if (xg->varchosen[varno])
    XSetLineAttributes(display, varpanel_xor_GC, 2, LineSolid,
      gcv->cap_style, gcv->join_style);

  XDrawArc(display, vwin, varpanel_xor_GC, 0, 0,
    (unsigned int) 2*r, (unsigned int) 2*r,
    0, 360*64);

  if (xg->is_xyplotting) {
    if (xg->varchosen[varno]) {
      if (varno == xg->xy_vars.x) {
        ChooseX(vwin, r, r);
      } else if (varno == xg->xy_vars.y) {
        ChooseY(vwin, r, r);
      }
    }
  }

  else if (xg->is_plotting1d) {
    if (xg->varchosen[varno]) {
      if (varno == xg->plot1d_vars.y) {
        ChooseY(vwin, r, r);
      } else if (varno == xg->plot1d_vars.x) {
        ChooseX(vwin, r, r);
      }
    }
  }

  else if (xg->is_spinning)
  {
    static Boolean spin_initd = false;
    static int rx, ry;
    if (!spin_initd) {
      int ascent, descent, direction;
      XCharStruct overall;
      XTextExtents(appdata.font, "x", 1,
        &direction, &ascent, &descent, &overall);
      ry = r + ascent + descent - 1;
      rx = r + r/4;
      spin_initd = true;
    }
    if (xg->varchosen[varno]) {
      if (xg->is_spin_type.yaxis) {   /* Using vertical axis */
        if (varno == xg->spin_vars.x) {
          ChooseX(vwin, r, r * xg->ocost.y/PRECISION2);
          XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "x", 2);
        } else if (varno == xg->spin_vars.y) {
          ChooseX(vwin, r, r * xg->osint.y/PRECISION2);
          XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "y", 2);
        } else if (varno == xg->spin_vars.z) {
          ChooseY(vwin, r, r);
          XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "z", 2);
        }
      }
      else if (xg->is_spin_type.xaxis) {  /* Using horizontal axis */
        if (varno == xg->spin_vars.x) {
          ChooseY(vwin, r, r * xg->ocost.x/PRECISION2);
          XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "x", 2);
        } else if (varno == xg->spin_vars.y) {
          ChooseY(vwin, r, r * xg->osint.x/PRECISION2);
          XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "y", 2);
        } else if (varno == xg->spin_vars.z) {
          ChooseX(vwin, r, r);
          XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "z", 2);
        }
      }
      else if (xg->is_spin_type.oblique) {
        float fr = (float) r;
        if (varno == xg->spin_vars.x) {
          xg->xax.x = INT(fr * xg->Rmat0[0][0]);
          xg->xax.y = INT(fr * xg->Rmat0[1][0]);
          ChooseVar(vwin, r, xg->xax.x, xg->xax.y);
          XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "x", 2);
        } else if (varno == xg->spin_vars.y) {
          xg->yax.x = INT(fr * xg->Rmat0[0][1]);
          xg->yax.y = INT(fr * xg->Rmat0[1][1]);
          ChooseVar(vwin, r, xg->yax.x, xg->yax.y);
          XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "y", 2);
        } else if (varno == xg->spin_vars.z) {
          xg->zax.x = INT(fr * xg->Rmat0[0][2]);
          xg->zax.y = INT(fr * xg->Rmat0[1][2]);
          ChooseVar(vwin, r, xg->zax.x, xg->zax.y);
          XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "z", 2);
        }
      }
      if (varno == last_touched)
        draw_last_touched(xg);
    }
  }
  else if (xg->is_touring) {
    x = INT(FLOAT(r) * xg->uold[0][varno]);
    y = INT(FLOAT(r) * xg->uold[1][varno]);
    ChooseVar(vwin, r, x, y);
    if (varno == xg->manip_var)
        draw_manip_var(xg);
    draw_frozen_var(xg);
  }
  else if (xg->is_corr_touring) {
    calc_var_xy(xg, (int) r, varno, &x, &y);
    ChooseVar(vwin, r, x, y);
    if (varno == xg->corr_xmanip_var || varno == xg->corr_ymanip_var)
      draw_cmanip_var(xg);
    draw_cfrozen_var(xg);
  }

  if (xg->varchosen[varno] || varno == focus_var)
    XSetLineAttributes(display, varpanel_xor_GC, 0, LineSolid,
      gcv->cap_style, gcv->join_style);

  if (do_label)
    refresh_vlab(varno, xg);
}

void
refresh_vboxes(xgobidata *xg)
{
  int j;
  int do_label = 1;

  for (j=0; j<xg->ncols_used; j++)
    refresh_vbox(xg, j, do_label);
  if (xg->is_spinning)
    draw_last_touched(xg);
  if (xg->is_touring)
  {
    draw_manip_var(xg);
    draw_frozen_var(xg);
  }
  if (xg->is_corr_touring)
  {
    draw_cmanip_var(xg);
    draw_cfrozen_var(xg);
  }
}


/********** Rotation **************/

/* ARGSUSED */
XtActionProc
ShowTarget(Widget w, XEvent *event, String *params, Cardinal nparams)
{
/*
 * If the cursor is outside the variable selection panel, check
 * for its entry.
*/
  XCrossingEvent *evnt = (XCrossingEvent *) event;

  if (xgobi.is_spinning &&
    !cursor_in_panel &&
    evnt->type == EnterNotify)
  {
    cursor_in_panel = 1;
    draw_last_touched(&xgobi);
  }
}

/* ARGSUSED */
XtActionProc
DontShowTarget(Widget w, XEvent *event, String *params, Cardinal nparams)
{
/*
 * If the cursor is inside the variable selection panel, look for
 * LeaveNotify events.  The detail member of the XCrossing Event
 * structure is NotifyInferior if the cursor has entered the window
 * belonging to a child widget of the window receiving the event.
*/
  XCrossingEvent *evnt = (XCrossingEvent *) event;

  if (xgobi.is_spinning &&
    cursor_in_panel &&
    evnt->type == LeaveNotify &&
    evnt->detail != NotifyInferior)
  {
    cursor_in_panel = 0;
    refresh_vbox(&xgobi, last_touched, 0);
  }
}

void
reset_last_touched(xgobidata *xg)
{
/*
 * Reset the last_touched variable when moving between
 * y axis and x axis rotation.
*/
  if (xg->is_spin_type.yaxis)
  {
    if (last_touched == xg->spin_vars.y)
      last_touched = xg->spin_vars.x;
  }
  else if (xg->is_spin_type.xaxis)
  {
    if (last_touched == xg->spin_vars.x)
      last_touched = xg->spin_vars.y;
  }
}

void
spin_var_lines(xgobidata *xg)
{
  Window vwin;
  static Boolean spin_initd = false;
  unsigned int r = xg->radius;
  static int rx, ry;
  XGCValues *gcv, gcv_inst;
  XGetGCValues(display, varpanel_xor_GC, GCCapStyle|GCJoinStyle, &gcv_inst);
  gcv = &gcv_inst;

  XSetLineAttributes(display, varpanel_xor_GC, 2, LineSolid,
    gcv->cap_style, gcv->join_style);

  if (!spin_initd) {
    int ascent, descent, direction;
    XCharStruct overall;
    XTextExtents(appdata.font, "x", 1,
      &direction, &ascent, &descent, &overall);
    ry = r + ascent + descent - 1;
    rx = r + r/4;
    spin_initd = true;
  }
    
  if (xg->is_spin_type.yaxis)   /* Using vertical axis */
  {
    vwin = XtWindow(xg->vardraww[xg->spin_vars.x]);
    ChooseX(vwin, xg->radius, xg->radius * xg->ocost.y/PRECISION2);
    ChooseX(vwin, xg->radius, xg->radius * xg->icost.y/PRECISION2);
    XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "x", 2);

    vwin = XtWindow(xg->vardraww[xg->spin_vars.y]);
    ChooseX(vwin, xg->radius, xg->radius * xg->osint.y/PRECISION2);
    ChooseX(vwin, xg->radius, xg->radius * xg->isint.y/PRECISION2);
    XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "y", 2);

    xg->ocost.y = xg->icost.y;
    xg->osint.y = xg->isint.y;
  }
  /*
   * Using horizontal axis in the plane of the screen
  */
  else if (xg->is_spin_type.xaxis)
  {
    vwin = XtWindow(xg->vardraww[xg->spin_vars.x]);
    ChooseY(vwin, xg->radius, xg->radius * xg->ocost.x/PRECISION2);
    ChooseY(vwin, xg->radius, xg->radius * xg->icost.x/PRECISION2);
    XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "x", 2);

    vwin = XtWindow(xg->vardraww[xg->spin_vars.y]);
    ChooseY(vwin, xg->radius, xg->radius * xg->osint.x/PRECISION2);
    ChooseY(vwin, xg->radius, xg->radius * xg->isint.x/PRECISION2);
    XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "y", 2);

    xg->ocost.x = xg->icost.x;
    xg->osint.x = xg->isint.x;
  }

  XSetLineAttributes(display, varpanel_xor_GC, 0, LineSolid,
    gcv->cap_style, gcv->join_style);

  draw_last_touched(xg);
}

int
spin_varsel_oblique(int varno, xgobidata *xg)
{
  int prev, newvar = 1;

  if (varno == xg->spin_vars.x ||
    varno == xg->spin_vars.y ||
    varno == xg->spin_vars.z)
   {
    prev = last_touched;
    last_touched = varno;
    refresh_vbox(xg, prev, 0);
    newvar = 0;
  }
  else
  {
    if (last_touched == xg->spin_vars.x)
      xg->spin_vars.x = varno;
    else if (last_touched == xg->spin_vars.y)
      xg->spin_vars.y = varno;
    else if (last_touched == xg->spin_vars.z)
      xg->spin_vars.z = varno;

    prev = last_touched;
    xg->varchosen[prev] = False;
    refresh_vbox(xg, prev, 1);

    last_touched = varno;
    xg->varchosen[varno] = True;
  }
  return(newvar);
}

int
spin_varsel_ax(int varno, int button, int state, xgobidata *xg)
{
/*
 * Here x and y are plotted horizontally; z is plotted vertically.
 * or
 * x and y are plotted vertically; z is plotted horizontally.
*/
  int prev = -1, newvar = 1;
  Boolean leftClick = (button == 1 && state != 8); /* alt not pressed */
  Boolean rightClick = (button == 2 || button == 1 && state == 8);

  if ((xg->is_spin_type.yaxis && leftClick) ||
      (xg->is_spin_type.xaxis && rightClick))
  {

    if (varno == xg->spin_vars.x || varno == xg->spin_vars.y)
      ;

    else if (varno == xg->spin_vars.z) {

      /*
       * If it's the chosen y variable, exchange y and last_touched.
      */
      if (last_touched == xg->spin_vars.x) {
        last_touched = xg->spin_vars.z = xg->spin_vars.x;
        xg->spin_vars.x = varno;
      } else if (last_touched == xg->spin_vars.y) {
        last_touched = xg->spin_vars.z = xg->spin_vars.y;
        xg->spin_vars.y = varno;
      }
    }
    /*
     * Else, the new variable replaces last_touched.
    */
    else {
      prev = last_touched;
      if (last_touched == xg->spin_vars.x) {
        xg->spin_vars.x = varno;
      } else if (last_touched == xg->spin_vars.y) {
        xg->spin_vars.y = varno;
      }
    }
    last_touched = varno;

  } else if ((xg->is_spin_type.yaxis && rightClick) ||
             (xg->is_spin_type.xaxis && leftClick))
  {

    /*
     * Is it already a chosen z variable?  If so, ignore.
    */
    if (varno == xg->spin_vars.z)
      newvar = 0;
    /*
     * If it's the chosen x variable, exchange z for the chosen x.
    */
    else if (varno == xg->spin_vars.x) {
      last_touched = xg->spin_vars.x = xg->spin_vars.z;
    } else if (varno == xg->spin_vars.y) {
      last_touched = xg->spin_vars.y = xg->spin_vars.z;
    } else {
      prev = xg->spin_vars.z;
    }
    xg->spin_vars.z = varno;
  }

  if (prev != -1) {
    xg->varchosen[prev] = False;
    refresh_vbox(xg, prev, 1);
    xg->varchosen[varno] = True;
  }

  return(newvar);
}


int
spin_varselect(int varno, int button, int state, xgobidata *xg)
/*
 * This is the routine called during rotation to handle changes in
 * variables entering the plot.  Call refresh_vbox() with instructions
 * to call refresh_vlab().
*/
{
  int newvar = 1;

  if (xg->is_spin_type.yaxis || xg->is_spin_type.xaxis)
    newvar = spin_varsel_ax(varno, button, state, xg);

  else if (xg->is_spin_type.oblique)
    newvar = spin_varsel_oblique(varno, xg);

  if (newvar)
    refresh_vboxes(xg);

  return(newvar);
}

void
draw_ob_var_lines(xgobidata *xg)
{
  Window vwin;
  static Boolean spin_initd = false;
  int r = xg->radius;
  static int rx, ry;
  float fr = (float) xg->radius;
  XGCValues *gcv, gcv_inst;
  XGetGCValues(display, varpanel_xor_GC, GCCapStyle|GCJoinStyle, &gcv_inst);
  gcv = &gcv_inst;

  XSetLineAttributes(display, varpanel_xor_GC, 2, LineSolid,
    gcv->cap_style, gcv->join_style);

  if (!spin_initd) {
    int ascent, descent, direction;
    XCharStruct overall;
    XTextExtents(appdata.font, "x", 1,
      &direction, &ascent, &descent, &overall);
    ry = r + ascent + descent - 1;
    rx = r + r/4;
    spin_initd = true;
  }

  vwin = XtWindow(xg->vardraww[xg->spin_vars.x]);
  ChooseVar(vwin, r, xg->xax.x, xg->xax.y);
  xg->xax.x = INT(fr * xg->Rmat0[0][0]);
  xg->xax.y = INT(fr * xg->Rmat0[1][0]);
  ChooseVar(vwin, r, xg->xax.x, xg->xax.y);
  XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "x", 2);

  vwin = XtWindow(xg->vardraww[xg->spin_vars.y]);
  ChooseVar(vwin, r, xg->yax.x, xg->yax.y);
  xg->yax.x = INT(fr * xg->Rmat0[0][1]);
  xg->yax.y = INT(fr * xg->Rmat0[1][1]);
  ChooseVar(vwin, r, xg->yax.x, xg->yax.y);
  XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "y", 2);

  vwin = XtWindow(xg->vardraww[xg->spin_vars.z]);
  ChooseVar(vwin, r, xg->zax.x, xg->zax.y);
  xg->zax.x = INT(fr * xg->Rmat0[0][2]);
  xg->zax.y = INT(fr * xg->Rmat0[1][2]);
  ChooseVar(vwin, r, xg->zax.x, xg->zax.y);
  XDrawString(display, vwin, varpanel_copy_GC, rx, ry, "z", 2);

  XSetLineAttributes(display, varpanel_xor_GC, 0, LineSolid,
    gcv->cap_style, gcv->join_style);

  draw_last_touched(xg);
}

/********** Grand Tour ************/

void
tour_var_lines(xgobidata *xg)
{
  int j;
  int x, y;
  int r = xg->radius;
  float fr = (float) xg->radius;
  Window vwin;
  XGCValues *gcv, gcv_inst;
  XGetGCValues(display, varpanel_xor_GC, GCCapStyle|GCJoinStyle, &gcv_inst);
  gcv = &gcv_inst;

  if (xg->is_touring) {
    for (j=0; j<xg->ncols_used; j++) {
      if (xg->varchosen[j])
        XSetLineAttributes(display, varpanel_xor_GC, 2, LineSolid,
          gcv->cap_style, gcv->join_style);
    
      vwin = XtWindow(xg->vardraww[j]);
      x = INT(fr * xg->uold[0][j]);
      y = INT(fr * xg->uold[1][j]);
      ChooseVar(vwin, r, x, y);
      x = INT(fr * xg->u[0][j]);
      y = INT(fr * xg->u[1][j]);
      ChooseVar(vwin, r, x, y);
      xg->uold[0][j] = xg->u[0][j];
      xg->uold[1][j] = xg->u[1][j];

      if (xg->varchosen[j])
        XSetLineAttributes(display, varpanel_xor_GC, 0, LineSolid,
          gcv->cap_style, gcv->join_style);
    }
  }
}

int
add_variable(xgobidata *xg, int varno)
{
  int found, i, j;

  xg->varchosen[varno] = True;
  found = 0;
  if (varno < xg->tour_vars[0])
  {
    for (j=xg->numvars_t; j>0; j--)
      xg->tour_vars[j] = xg->tour_vars[j-1];
    xg->tour_vars[0] = varno;
    xg->numvars_t += 1;
    found = 1;
  }
  i = 0;
  while (!found && i < xg->numvars_t-1 )
  {
    if ( varno > xg->tour_vars[i]  &&
         varno < xg->tour_vars[i+1] )
    {
      for (j=xg->numvars_t; j>i; j--)
        xg->tour_vars[j+1] = xg->tour_vars[j];
       xg->tour_vars[i+1] = varno;
       xg->numvars_t += 1;
       found = 1;
    }
    i++;
  }
  if (!found && (varno > xg->tour_vars[xg->numvars_t-1]))
  {
    xg->tour_vars[xg->numvars_t] = varno;
    xg->numvars_t += 1;
    found = 1;
  }

  return(found);
}

void
remove_variable(xgobidata *xg, int varno)
{
  int i, j, found, itmp;

  /* for frozen variables we want this to remain true -
     if this routine is ever called for another purpose this line will 
     need to be changed */
  xg->varchosen[varno] = True;
  i = 0;
  found = 0;
  while (!found)
  {
    if (varno == xg->tour_vars[i])
    {
      itmp = i;

      xg->numvars_t -= 1;
      for (j=itmp; j<xg->numvars_t; j++)
        xg->tour_vars[j] = xg->tour_vars[j+1];

      found = 1;
      refresh_vbox(xg, varno, 1);
    }
    i++;
  } /* end while */

}


int
tour_varselect(int varno, int state, xgobidata *xg)
/*
 * This routine is called during grand touring to handle changes
 * in variables included in plot. It calls refresh_vbox() with
 * further call to refresh_vlab() to handle variable box refreshes
 * and label highlighting.
*/
{
  int newvar = 0;

  if (state == 0)
    set_tourvar(xg, varno);
  else if (state == 1)
    set_manip_var(xg, varno);
  else if (state == 4)
    set_frozen_var(xg, varno);
  
  return(newvar);
}

void
set_varpanel_for_receive_tour(xgobidata *xg)
{
  int j, var;
  void refresh_vlab();

  if (xg->tour_link_state == receive)  /* If receive was just turned on ... */
  {
    for (j=0; j<xg->ncols_used; j++)
    {
      if (xg->varchosen[j])
      {
        xg->varchosen[j] = False;
        refresh_vbox(xg, j, True);
      }
    }
  }
  else  /* else if receive was just turned off ... */
  {
    for (j=0; j<xg->numvars_t; j++)
    {
      var = xg->tour_vars[j];
      xg->varchosen[var] = True;
      refresh_vbox(xg, j, True);
    }
  }
}

void
map_group_var(xgobidata *xg)
{
  XtVaSetValues(xg->varlabw[xg->ncols-1],
    XtNlabel, (String) xg->collab_tform2[xg->ncols-1],
    NULL);
  XtMapWidget(xg->varboxw[xg->ncols-1]);
}

void
carry_spin_vars(xgobidata *xg)
{
  int j;

  if (xg->is_plotting1d) {
   if (xg->plot1d_vars.y != -1) {
      xg->spin_vars.y = xg->plot1d_vars.y;
      for (j=0; j<xg->ncols; j++)
        if (j != xg->spin_vars.y) {
          xg->spin_vars.x = j;
          break;
        }
    } else {
      xg->spin_vars.x = xg->plot1d_vars.x;
      for (j=0; j<xg->ncols; j++)
        if (j != xg->spin_vars.x) {
          xg->spin_vars.y = j;
          break;
        }
    }

    for (j=0; j<xg->ncols; j++) {
      if (j != xg->spin_vars.x && j != xg->spin_vars.y) {
        xg->spin_vars.z = j;
        break;
      }
    }
  }
  else if (xg->is_xyplotting) {
    xg->spin_vars.x = xg->xy_vars.x;
    xg->spin_vars.y = xg->xy_vars.y;
    for (j=0; j<xg->ncols; j++)
      if (j != xg->spin_vars.x && j != xg->spin_vars.y) {
        xg->spin_vars.z = j;
        break;
      }
  }
  else if (xg->is_touring) {
    xg->spin_vars.x = xg->tour_vars[0];
    xg->spin_vars.z = xg->tour_vars[2];
    xg->spin_vars.y = xg->tour_vars[1];
  }
  else if (xg->is_corr_touring) {
    xg->spin_vars.x = xg->corr_xvars[0];
    xg->spin_vars.z = xg->corr_xvars[1];
    xg->spin_vars.y = xg->corr_yvars[0];
  }

  /*
   * The target variable may have been deselected.
  */
  if (last_touched != xg->spin_vars.x &&
    last_touched != xg->spin_vars.y &&
    last_touched != xg->spin_vars.z)
  {
    last_touched = xg->spin_vars.z;
  }

  reinit_spin(xg);
  reset_last_touched(xg);
}

void
carry_tour_vars(xgobidata *xg)
{
  int i, inc;

  if (xg->is_plotting1d) {
    inc = 0;
    for (i=0; i<xg->numvars_t; i++) {
      if (xg->tour_vars[i] == xg->plot1d_vars.y ||
          xg->tour_vars[i] == xg->plot1d_vars.x)
      {
        inc = 1;
        break;
      }
    }

    if (inc) {
      if (xg->tour_vars[1] == xg->plot1d_vars.y ||
          xg->tour_vars[1] == xg->plot1d_vars.x)
        ;
      else {
        for (i=0; i<xg->numvars_t; i++) {
          if (xg->tour_vars[i] == xg->plot1d_vars.y ||
              xg->tour_vars[i] == xg->plot1d_vars.x)
          {
            xg->tour_vars[i] = xg->tour_vars[1];
            xg->tour_vars[1] = (xg->plot1d_vars.y != -1) ?
              xg->plot1d_vars.y : xg->plot1d_vars.x;
            break;
          }
        }
      }
    } else {
      xg->tour_vars[xg->numvars_t] = xg->tour_vars[1];
      xg->tour_vars[1] = (xg->plot1d_vars.y != -1) ?
        xg->plot1d_vars.y : xg->plot1d_vars.x;
      xg->numvars_t++;
    }
  }

  else if (xg->is_xyplotting) {
  /*
   * First see if xy_vars.x is included.
  */
    inc = 0;
    for (i=0; i<xg->numvars_t; i++) {
      if (xg->tour_vars[i] == xg->xy_vars.x) {
        inc = 1;
        break;
      }
    }

    if (inc) {
      if (xg->tour_vars[0] == xg->xy_vars.x)
        ;
      else {
        for (i=0; i<xg->numvars_t; i++) {
          if (xg->tour_vars[i] == xg->xy_vars.x) {
            xg->tour_vars[i] = xg->tour_vars[0];
            xg->tour_vars[0] = xg->xy_vars.x;
            break;
          }
        }
      }
    } else {
      xg->tour_vars[xg->numvars_t] = xg->tour_vars[0];
      xg->tour_vars[0] = xg->xy_vars.x;
      xg->numvars_t++;
    }

  /*
   * Then see if xy_vars.y is included.
  */
    inc = 0;
    for (i=0; i<xg->numvars_t; i++) {
      if (xg->tour_vars[i] == xg->xy_vars.y) {
        inc = 1;
        break;
      }
    }

    if (inc) {
      if (xg->tour_vars[1] == xg->xy_vars.y)
        ;
      else {
        for (i=0; i<xg->numvars_t; i++) {
          if (xg->tour_vars[i] == xg->xy_vars.y) {
            xg->tour_vars[i] = xg->tour_vars[1];
            xg->tour_vars[1] = xg->xy_vars.y;
            break;
          }
        }
      }
    } else {
      xg->tour_vars[xg->numvars_t] = xg->tour_vars[1];
      xg->tour_vars[1] = xg->xy_vars.y;
      xg->numvars_t++;
    }
  }

  else if (xg->is_spinning) {
    /*
     * Switch to three variables ...
    */
    xg->tour_vars[0] = xg->spin_vars.x;
    xg->tour_vars[1] = xg->spin_vars.y;
    xg->tour_vars[2] = xg->spin_vars.z;
    xg->numvars_t = 3;
    reinit_tour(xg);
  }
  else if (xg->is_corr_touring) {
    xg->numvars_t = 0;

    for (i=0; i<xg->ncorr_xvars; i++) {
      xg->tour_vars[i] = xg->corr_xvars[i];
      xg->numvars_t++ ;
    }
    for (i=0;i<xg->ncorr_yvars;i++) {
      xg->tour_vars[i+xg->ncorr_xvars] = xg->corr_yvars[i];
      xg->numvars_t++ ;
    }
    reinit_tour(xg);
  }
}

void
carry_xyplot_vars(xgobidata *xg)
{
/*
 * When entering XYPlot from another mode with carry_vars
 * set to True, do this ...
*/
  if (xg->is_plotting1d) {
    if (xg->xy_vars.y == xg->plot1d_vars.y)
      ;
    else if (xg->xy_vars.x == xg->plot1d_vars.x)
      ;
    else if (xg->xy_vars.x == xg->plot1d_vars.y) {
      xg->xy_vars.x = xg->xy_vars.y;
      xg->xy_vars.y = xg->plot1d_vars.y;

    } else if (xg->xy_vars.y == xg->plot1d_vars.x) {
      xg->xy_vars.y = xg->xy_vars.x;
      xg->xy_vars.x = xg->plot1d_vars.x;
    }
  }
  else if (xg->is_spinning) {
    xg->xy_vars.y = xg->spin_vars.y;
    xg->xy_vars.x = xg->spin_vars.x;
  }
  else if (xg->is_touring) {
    xg->xy_vars.y = xg->tour_vars[1];
    xg->xy_vars.x = xg->tour_vars[0];
  }
  else if (xg->is_corr_touring) {
    xg->xy_vars.x = xg->corr_xvars[0];
    xg->xy_vars.y = xg->corr_yvars[0];
  }
}

void
carry_plot1d_vars(xgobidata *xg)
{
/*
 * When entering Plot1D from another mode with carry_vars
 * set to True, do this ...
*/

  if (xg->is_xyplotting) {
    if (xg->plot1d_vars.y != -1)
      xg->plot1d_vars.y = xg->xy_vars.y;
    else
      xg->plot1d_vars.x = xg->xy_vars.x;
  }

  else if (xg->is_spinning) {
    if (xg->plot1d_vars.y != -1)
      xg->plot1d_vars.y = xg->spin_vars.y;
    else
      xg->plot1d_vars.x = xg->spin_vars.x;
  }

  else if (xg->is_touring) {
    if (xg->plot1d_vars.y != -1)
      xg->plot1d_vars.y = xg->tour_vars[1];
    else
      xg->plot1d_vars.x = xg->tour_vars[0];
  }

  else if (xg->is_corr_touring) {
    if (xg->plot1d_vars.y != -1)
      xg->plot1d_vars.y = xg->corr_yvars[0];
    else
      xg->plot1d_vars.x = xg->corr_xvars[0];
  }
}

/* dfs: asking di for help with this one */
void
carry_corr_vars(xgobidata *xg)
{
  int i, inc;

  if (xg->is_plotting1d) {
    inc = 0;
    for (i=0; i<xg->ncorr_yvars; i++) {
      if (xg->corr_xvars[i] == xg->plot1d_vars.y) {
        inc = 1;
        break;
      }
    }

    if (inc) {
      if (i != 0) {
        xg->corr_yvars[i] = xg->corr_yvars[0];
        xg->corr_yvars[0] = xg->plot1d_vars.y;
      }
    } else {
      xg->corr_yvars[xg->ncorr_yvars] = xg->corr_yvars[0];
      xg->corr_yvars[0] = xg->plot1d_vars.y;
      xg->ncorr_yvars++;
    }
  }

  else if (xg->is_xyplotting) {
  /*
   * First see if xy_vars.x is included.
  */
    inc = 0;
    for (i=0; i<xg->ncorr_xvars; i++) {
      if (xg->corr_xvars[i] == xg->xy_vars.x) {
        inc = 1;
        break;
      }
    }

    if (inc) {
      if (i != 0) {
        xg->corr_xvars[i] = xg->corr_xvars[0];
        xg->corr_xvars[0] = xg->xy_vars.x;
      }
    } else {
      xg->corr_xvars[xg->ncorr_xvars] = xg->corr_xvars[0];
      xg->corr_xvars[0] = xg->xy_vars.x;
      xg->ncorr_xvars++;
    }

  /*
   * Then see if xy_vars.y is included.
  */
    inc = 0;
    for (i=0; i<xg->ncorr_yvars; i++) {
      if (xg->corr_yvars[i] == xg->xy_vars.y) {
        inc = 1;
        break;
      }
    }

    if (inc) {
      if (i != 0) {
        xg->corr_yvars[i] = xg->corr_yvars[0];
        xg->corr_yvars[0] = xg->xy_vars.y;
      }
    } else {
      xg->corr_yvars[xg->ncorr_yvars] = xg->corr_yvars[0];
      xg->corr_yvars[0] = xg->xy_vars.y;
      xg->ncorr_yvars++;
    }
  }

  else if (xg->is_spinning) {
    /*
     * Switch to three variables ...
    */
    xg->corr_xvars[0] = xg->spin_vars.x;
    xg->corr_xvars[1] = xg->spin_vars.z;
    xg->corr_yvars[0] = xg->spin_vars.y;
    xg->ncorr_xvars = 2;
    xg->ncorr_yvars = 1;
    reinit_corr(xg);
  }
  else if (xg->is_touring) {
    /* take variable with largest y contribution to be single y variable */
    float max;

    max = xg->tour_vars[0];
    for (i=0; i<xg->numvars_t; i++)
      if (xg->u[1][i] > max)
        max = xg->tour_vars[i];
    xg->corr_yvars[0] = max;
    xg->ncorr_yvars = 1;    

    xg->ncorr_xvars = 0;
    for (i=0; i<xg->numvars_t; i++)
      if (xg->tour_vars[i] != xg->corr_yvars[0])
        xg->corr_xvars[xg->ncorr_xvars++] = xg->tour_vars[i];
    reinit_corr(xg);
  }
}

/********** For New Data **********/

void
destroy_varsel_widgets(int ncols_prev, xgobidata *xg)
{
  int j;

  XtUnmapWidget(xg->var_panel);
  XtUnmanageChild(xg->var_panel);
  /*
   * Destroy all the variable boxes.
  */
  XtUnmanageChildren(xg->varboxw, ncols_prev);
  for (j=0; j<ncols_prev; j++)
    XtDestroyWidget(xg->varboxw[j]);
}

void
reset_var_panel(xgobidata *xg)
{
  int j;

  /*
   * Reallocate the variable panel and menus for the new ncols.
  */
  xg->varboxw = (Widget *)
    XtRealloc((char *) xg->varboxw,
    (Cardinal) xg->ncols * sizeof(Widget));
  xg->varlabw = (Widget *)
    XtRealloc((char *) xg->varlabw,
    (Cardinal) xg->ncols * sizeof(Widget));
  xg->vardraww = (Widget *)
    XtRealloc((char *) xg->vardraww,
    (Cardinal) xg->ncols * sizeof(Widget));
  xg->varchosen = (Boolean *)
    XtRealloc((char *) xg->varchosen,
    (Cardinal) xg->ncols * sizeof(Boolean));

  /*
   * Create the new variable boxes.
  */
  make_varboxes(xg);

  for (j=0; j<xg->ncols; j++)
  {
    XtVaSetValues(xg->vardraww[j],
      XtNwidth, (Dimension) (2*(xg->radius+1)),
      XtNheight, (Dimension) (2*(xg->radius+1)), NULL);
    /* This line is important! */
    XtRealizeWidget(xg->varboxw[j]);
  }
  XtManageChildren(xg->varboxw, (unsigned int) xg->ncols);

  XtMapWidget(xg->var_panel);

  for (j=0; j<xg->ncols_used; j++)
    refresh_vbox(xg, j, 1);
}
