#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

static Widget stdview_panel, stdview_menu_cmd, stdview_menu;
static Widget stdview_menu_btn[3];

/* ARGSUSED */
XtCallbackProc
choose_stdview_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k;
/*
 * Find out which transformation to do.
*/
  for (k=0; k<3; k++) {
    if (stdview_menu_btn[k] == w) {
      xg->std_type = k;
      break;
    }
  }

  for (k=0; k<3; k++)
    XtVaSetValues(stdview_menu_btn[k],
      XtNleftBitmap, None,
      NULL);

  XtVaSetValues(w,
    XtNleftBitmap, menu_mark,
    NULL);

  update_lims(xg);
  update_world(xg);
  world_to_plane(xg);
  plane_to_screen(xg);

  if (xg->is_xyplotting)
    init_ticks(&xg->xy_vars, xg);
  else if (xg->is_plotting1d)
    init_ticks(&xg->plot1d_vars, xg);
  plot_once(xg);
}

void
make_stdview_menu(xgobidata *xg)
/*
 * Build a menu to contain standardization options.
*/
{
  int j;
  static char *stdview_names[] = {
    "Min/Max", /* default */ "Mean/LargestDistance", "Median/LargestDistance"
  };

  stdview_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, stdview_panel,
    XtNlabel, (String) "Stdize view",
    XtNmenuName, (String) "Menu",
    NULL);
  if (mono) set_mono(stdview_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    stdview_menu_cmd, "Sc_StdizeMenu");

  stdview_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, stdview_menu_cmd,
    NULL);
  if (mono) set_mono(stdview_menu);

  for (j=0; j<3; j++) {
    stdview_menu_btn[j] = XtVaCreateWidget("Command",
      smeBSBObjectClass, stdview_menu,
      XtNlabel, (String) stdview_names[j],
      XtNleftMargin, (Dimension) 24,
      NULL);
    if (mono) set_mono(stdview_menu_btn[j]);
    XtAddCallback(stdview_menu_btn[j], XtNcallback,
      (XtCallbackProc) choose_stdview_cback, (XtPointer) xg);
  }

  XtManageChildren(stdview_menu_btn, 3);
}

void
init_stdview_menu(xgobidata *xg)
/*
 * Indicate the initial standardization type.
*/
{
  XtVaSetValues(stdview_menu_btn[xg->std_type],
    XtNleftBitmap, menu_mark,
    NULL);
}

void
make_stdview(xgobidata *xg, Widget parent)
{
  stdview_panel = XtVaCreateManagedWidget("StdizePanel",
    formWidgetClass, parent,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    NULL);
  if (mono) set_mono(stdview_panel);

  make_stdview_menu(xg);
  init_stdview_menu(xg);
}

