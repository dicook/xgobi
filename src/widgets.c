/* widgets.c */
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
#include <stdlib.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include <X11/Xatom.h>
#include <X11/Xproto.h>

#include "../bitmaps/leftarrow.xbm"
#include "../bitmaps/rightarrow.xbm"
#include "../bitmaps/uparrow.xbm"
#include "../bitmaps/downarrow.xbm"
#include "../bitmaps/plus.xbm"
#include "../bitmaps/minus.xbm"
#include "../bitmaps/lrarrow.xbm"

/*  bitmaps for checkboxes */
#include "../bitmaps/ton16.xbm"
#include "../bitmaps/toff16.xbm"

static Widget form1, main_panel;

#ifdef XPLORE
#define NFILEBTNS 7 /* xplore */
#else
#define NFILEBTNS 5
#endif

static Widget file_menu_cmd, file_menu, file_menu_btn[NFILEBTNS];
#define EXIT       file_menu_btn[NFILEBTNS-1]

/* NVIEWMODES is defined in xgobitypes.h */
static Widget view_menu_cmd, view_menu, view_menu_btn[NVIEWMODES];
static char *view_menu_labelname[] = {
  "View: 1DPlot",
  "View: XYPlot",
  "View: Rotate",
  "View: GrTour",
  "View: CoTour",
  "View: Scale",
  "View: Brush",
  "View: Ident",
  "View: LineEd",
  "View: MovePts",
};
char *view_menu_accel[] = {
  "d", "x", "r", "g", "c", "s", "b", "i", "l", "m",
};

/*
 * CORBA method:  returns the char ** view_menu_labelname which
 * is static in this file.  Also, returns the number of elements
 * in the array.
*/
char ** GetPlotViewTypes(int *num);


#ifdef XPLORE
#define NTOOLBTNS 15 /* xplore */
#else
#define NTOOLBTNS 13
#endif

Widget tool_menu_cmd, tool_menu, tool_menu_btn[NTOOLBTNS];

/*------ these all pertain to subsets of cases -------*/
#define EXCLUSION_BTN 0
#define SUBSET_BTN    1
/*------ these affect the current plot -------*/
#define SMOOTH_BTN    2
#define JITTER_BTN    3
#define INFER_BTN     4
/*------ open some new plotting window -------*/
#define PARCOORD_BTN  5
#define CLONE_BTN     6
#define SCATMAT_BTN   7
/*------ variable-wise, case-wise -------*/
#define VARTFORM_BTN  8
#define VARLIST_BTN   9
#define CASELIST_BTN  10
/*------ these both pertain to missing values -------*/
#define LAUNCH_MISSING_BTN   11
#define IMPUTE_BTN           12
/*------ XploRe -------*/
#ifdef XPLORE
#define XPLORE_BTN          13 /* xplore */
#define XPLORE_END_BTN      14 /* xplore */
#endif

#define NDISPLAYBTNS 7
static Widget display_menu_cmd, display_menu, display_menu_btn[NDISPLAYBTNS];
#define ADDAXES        0
#define ADDGRID        1
#define CENTERAXES     2
#define SHOWPOINTS     3
#define SHOWLINES      4
#define SHOWARROWS     5
#define CARRYVARS      6

void
init_options(xgobidata *xg)
{
  /* These are now on the brush panel */
  xg->link_points_to_points = (Boolean) appdata.linkGlyphBrush | 
    (Boolean) appdata.linkColorBrush;
  xg->link_points_to_lines =  False;
  xg->link_lines_to_lines =  (Boolean) appdata.linkLineBrush;
  xg->link_glyph_brushing =   appdata.linkGlyphBrush;
  xg->link_color_brushing =   appdata.linkColorBrush;
  xg->link_erase_brushing =   appdata.linkEraseBrush;

  xg->jump_brush =            appdata.jumpBrush;
  xg->reshape_brush =         appdata.reshapeBrush;
  xg->sync_brush =            appdata.syncBrush;

  /* This is now on the identify panel */
  xg->link_identify =         appdata.linkIdentify;

  xg->is_axes =               appdata.showAxes;
  xg->is_axes_centered =      True;
  xg->plot_the_points =       appdata.showPoints;
  xg->plot_the_arrows =       False;
  xg->connect_the_points =    appdata.showLines;
  xg->carry_vars =            appdata.carryVars;

  /* For cloning */
  xg->isCloned =              appdata.isCloned;
  xg->clone_PID =             appdata.clonePID;
  xg->clone_Time =            appdata.cloneTime;
  xg->clone_Type =            (enum clone_data_type) appdata.cloneType;
  xg->clone_Name =            appdata.cloneName;
  xg->delete_clone_data =     appdata.deleteCloneData;
}

void
set_mono(Widget w)
{
  XtVaSetValues(w,
    XtNforeground, appdata.fg,
    XtNbackground, appdata.bg,
    XtNborderColor, appdata.fg,
    NULL);
}

/* ARGSUSED */
void
set_Edit_Lines_cmd(xgobidata *xg, Boolean lgl)
{
  XtVaSetValues(view_menu_btn[LINEEDIT_MODE], XtNsensitive, lgl, NULL);
}

/* ARGSUSED */
void
set_Smooth_cmd(xgobidata *xg, Boolean lgl)
{
  XtVaSetValues(tool_menu_btn[SMOOTH_BTN], XtNsensitive, lgl, NULL);
}
     
/* ARGSUSED */
void
reset_Exit_cmd(xgobidata *xg)
{
  XtVaSetValues(EXIT, XtNsensitive, False, NULL);
}

Widget
CreateCommand(xgobidata *xg, char *label, Boolean sensitive, Widget href,
Widget vref, Widget parent, char *helpLabel)
{
  Widget pb;

  if (mono)
    pb = XtVaCreateWidget("Command",
      commandWidgetClass, parent,
      XtNlabel, (String) label,
      XtNsensitive, (Boolean) sensitive,
      XtNfromHoriz, (Widget) href,
      XtNfromVert, (Widget) vref,
      XtNforeground, appdata.fg,
      XtNbackground, appdata.bg,
      XtNborderColor, appdata.fg,
      NULL);
  else
    pb = XtVaCreateWidget("Command",
      commandWidgetClass, parent,
      XtNlabel, (String) label,
      XtNsensitive, (Boolean) sensitive,
      XtNfromHoriz, (Widget) href,
      XtNfromVert, (Widget) vref,
      NULL);

  add_pb_help(&xg->nhelpids.pb, pb, helpLabel);
  return(pb);
}

void
make_bitmaps(void) {
  toggle_off = XCreateBitmapFromData(display, DefaultRootWindow(display),
    (char *) toff16_bits, toff16_width, toff16_height);
  toggle_on = XCreateBitmapFromData(display, DefaultRootWindow(display),
    (char *) ton16_bits, ton16_width, ton16_height);
}

void
setToggleBitmap(Widget pb, Boolean set) {
  static Boolean initd = False;

  if (!initd) {
    make_bitmaps();
    initd = True;
  }

  if (set)
    XtVaSetValues(pb, XtNleftBitmap, toggle_on, NULL);
  else
    XtVaSetValues(pb, XtNleftBitmap, toggle_off, NULL);
}

Widget
CreateToggle(xgobidata *xg, char *label, Boolean sensitive,
Widget href, Widget vref, Widget radioref,
Boolean set, int type, Widget parent, char *helpLabel)
{
  Widget pb;
  char name[8];

  /*
   * Using the fallback resources, we force some
   * toggle widgets to follow "one of many" behavior
   * while others are permitted to be unselected.
  */
  if (type == ONE_OF_MANY)
    strcpy(name, "Toggle");
  else
    strcpy(name, "Command");

  if (mono)
    pb = XtVaCreateWidget(name,
      toggleWidgetClass, parent,
      XtNlabel, (String) label,
      XtNsensitive, (Boolean) sensitive,
      XtNfromHoriz, (Widget) href,
      XtNfromVert, (Widget) vref,
      XtNradioGroup, (Widget) radioref,
      XtNstate, (Boolean) set,
      XtNforeground, appdata.fg,
      XtNbackground, appdata.bg,
      XtNborderColor, appdata.fg,
      /*XtNborderWidth, 0,*/
      NULL);
  else
    pb = XtVaCreateWidget(name,
      toggleWidgetClass, parent,
      XtNlabel, (String) label,
      XtNsensitive, (Boolean) sensitive,
      XtNfromHoriz, (Widget) href,
      XtNfromVert, (Widget) vref,
      XtNradioGroup, (Widget) radioref,
      XtNstate, (Boolean) set,
      /*XtNborderWidth, 0,*/
      NULL);

  setToggleBitmap(pb, set);

  add_pb_help(&xg->nhelpids.pb, pb, helpLabel);
  return(pb);
}

void
build_labelled_menu(Widget *box, Widget *labelw, char *labelstr, Widget *cmd,
  Widget *menu, Widget *btn, char **btn_names, char **btn_nicknames, int nbtns,
  int initval, Widget parent, Widget vref, XtOrientation orientation,
  XFontStruct *fn, char *helplbl, xgobidata *xg)
{
  Dimension width = 0, maxwidth = 0;
  int longest = 0;
  int k;

  *box = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, parent,
    XtNorientation, (XtOrientation) orientation,
    XtNfromVert, vref,
    XtNhSpace, 1,
    XtNvSpace, 1,
    NULL);
  if (mono) set_mono(*box);

  /* Find the widest label */
  *labelw = XtVaCreateManagedWidget("Label",
    labelWidgetClass, *box,
    XtNlabel, labelstr,
    NULL);
  if (mono) set_mono(*labelw);

  for (k=0; k<nbtns; k++) {
    width = XTextWidth(fn, btn_nicknames[k], strlen(btn_nicknames[k])) +
      2*ASCII_TEXT_BORDER_WIDTH;
    if (width > maxwidth) {
      maxwidth = width;
      longest = k;
    }
  }

  *cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, *box,
    XtNlabel, (String) btn_nicknames[longest],
    XtNmenuName, (String) "Menu",
    XtNfromHoriz, *labelw,
    XtNresize, False,
    NULL);
  if (mono) set_mono(*cmd);
  if (xg != NULL)
    add_menupb_help(&xg->nhelpids.menupb,
      *cmd, helplbl);

  *menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, *cmd,
    NULL);
  if (mono) set_mono(*menu);

  for (k=0; k<nbtns; k++) {
    btn[k] = XtVaCreateWidget("Command",
      smeBSBObjectClass, *menu,
      XtNlabel, btn_names[k],
      NULL);
    if (mono) set_mono(btn[k]);
  }
  XtManageChildren(btn, (Cardinal) nbtns);

  XtVaSetValues(*cmd,
    XtNlabel, (String) btn_nicknames[initval],
    NULL);
}

Boolean
tour_on(xgobidata *xg)
{
  return(xg->plot_mode == GTOUR_MODE);
}

void
set_showlines_option(Boolean lgl, xgobidata *xg)
{
/*
 * This is needed because we're going to duplicate
 * this button on the line editing panel, and they
 * both need to show the same state, but we don't
 * want to call the callback twice.
*/
  XtVaSetValues(display_menu_btn[SHOWLINES], XtNstate, lgl, NULL);
  set_display_menu_marks(xg);
}

void
set_showarrows_option(Boolean lgl, xgobidata *xg)
{
  XtVaSetValues(display_menu_btn[SHOWARROWS], XtNstate, lgl, NULL);
  set_display_menu_marks(xg);
}

void
turn_on_showlines_option(xgobidata *xg)
{
/*
 * This is called when line editing is entered.
 * It sets xg->connect_the_points to True if it's False,
 * and calls the callback, and then makes certain the
 * toggle in the options menu is set to True.
*/
  if (!xg->connect_the_points) {
    XtCallCallbacks(display_menu_btn[SHOWLINES], XtNcallback, (XtPointer) xg);
    set_showlines_option(True, xg);
  }
}

void
make_plotwindow_mouse_labels(xgobidata *xg)
{
  Arg args[4];

  XtSetArg(args[0], XtNfromVert, (Widget) xg->workspace);
  XtSetArg(args[1], XtNtop, (XtEdgeType) XtChainBottom);
  XtSetArg(args[2], XtNbottom, (XtEdgeType) XtChainBottom);

  XtSetArg(args[3], XtNlabel, "Drag L or M: Move points ");
  xg->spin_mouse = XtCreateManagedWidget("MouseLabel",
    labelWidgetClass, xg->box1, args, 4);
  if (mono) set_mono(xg->spin_mouse);

  XtSetArg(args[3], XtNlabel, "Drag L: Move pts; Drag M: Shape pts ");
  xg->scale_mouse = XtCreateManagedWidget("MouseLabel",
    labelWidgetClass, xg->box1, args, 4);
  if (mono) set_mono(xg->scale_mouse);

  XtSetArg(args[3], XtNlabel, "Drag/Click L: paint; Drag M: shape ");
  xg->brush_mouse = XtCreateManagedWidget("MouseLabel",
    labelWidgetClass, xg->box1, args, 4);
  if (mono) set_mono(xg->brush_mouse);

  XtSetArg(args[3], XtNlabel, "L: Toggle sticky labels ");
  xg->identify_mouse = XtCreateManagedWidget("MouseLabel",
    labelWidgetClass, xg->box1, args, 4);
  if (mono) set_mono(xg->identify_mouse);

  XtSetArg(args[3], XtNlabel, "Drag L: Add line, M: Remove last ");
  xg->le_add_mouse = XtCreateManagedWidget("MouseLabel",
    labelWidgetClass, xg->box1, args, 4);
  if (mono) set_mono(xg->le_add_mouse);

  XtSetArg(args[3], XtNlabel, "L: Delete line, M: Put it back ");
  xg->le_delete_mouse = XtCreateManagedWidget("MouseLabel",
    labelWidgetClass, xg->box1, args, 4);
  if (mono) set_mono(xg->le_delete_mouse);

  XtSetArg(args[3], XtNlabel, "Drag L: Move point/group/all");
  xg->movepts_mouse = XtCreateManagedWidget("MouseLabel",
    labelWidgetClass, xg->box1, args, 4);
  if (mono) set_mono(xg->movepts_mouse);

  XtSetArg(args[3], XtNlabel, "Drag L or M: Manipulate Variable ");
  xg->tour_mouse = XtCreateManagedWidget("MouseLabel",
    labelWidgetClass, xg->box1, args, 4);
  if (mono) set_mono(xg->tour_mouse);

  XtSetArg(args[3], XtNlabel, "Drag L or M: Manipulate Variable ");
  xg->corr_mouse = XtCreateManagedWidget("MouseLabel",
    labelWidgetClass, xg->box1, args, 4);
  if (mono) set_mono(xg->corr_mouse);
}

void
make_plot_window(xgobidata *xg)
/*
 * PlotWindow: the plot window
*/
{
  xg->workspace = XtVaCreateManagedWidget("PlotWindow",
    labelWidgetClass, xg->box1,
    XtNresizable, (Boolean) True,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNright, (XtEdgeType) XtRubber,
    XtNbottom, (XtEdgeType) XtChainBottom,
    /*XtNbottom, (XtEdgeType) XtRubber,*/
    /*XtNfromVert, (Widget) xg->scale_mouse,*/
    XtNlabel, (String) "",
    NULL);
  if (mono) set_mono(xg->workspace);
  add_pb_help(&xg->nhelpids.pb,
    xg->workspace, "PlotWindow");

  XtAddEventHandler(xg->workspace, ExposureMask,
    FALSE, (XtEventHandler) expose_cback, (XtPointer) xg);
  XtAddEventHandler(xg->workspace, StructureNotifyMask,
    FALSE, (XtEventHandler) resize_cback, (XtPointer) xg);
}


void
make_arrows(xgobidata *xg) {
  Drawable root_window = RootWindowOfScreen(XtScreen(xg->shell));

  /*
   * Define arrows.  (depth is a global variable)
  */
  leftarr = XCreatePixmapFromBitmapData(display, root_window,
    leftarrow_bits, leftarrow_width, leftarrow_height,
    appdata.fg, appdata.bg, depth);
  rightarr = XCreatePixmapFromBitmapData(display, root_window,
    rightarrow_bits, rightarrow_width, rightarrow_height,
    appdata.fg, appdata.bg, depth);
  uparr = XCreatePixmapFromBitmapData(display, root_window,
    uparrow_bits, uparrow_width, uparrow_height,
    appdata.fg, appdata.bg, depth);
  downarr = XCreatePixmapFromBitmapData(display, root_window,
    downarrow_bits, downarrow_width, downarrow_height,
    appdata.fg, appdata.bg, depth);
  plus = XCreatePixmapFromBitmapData(display, root_window,
    plus_bits, plus_width, plus_height,
    appdata.fg, appdata.bg, depth);
  minus =  XCreatePixmapFromBitmapData(display, root_window,
    minus_bits, minus_width, minus_height,
    appdata.fg, appdata.bg, depth);

  /* Used in the missing panel */
  lrarr = XCreatePixmapFromBitmapData(display, root_window,
    lrarrow_bits, lrarrow_width, lrarrow_height,
    appdata.fg, appdata.bg, depth);
}

void
reset_3d_cmds(xgobidata *xg)
{
  Boolean sens = True;

  if (xg->ncols_used < 3)
    sens = False;

  XtVaSetValues(view_menu_btn[ROTATE_MODE],
    XtNsensitive, (Boolean) sens,
    NULL);
  XtVaSetValues(view_menu_btn[GTOUR_MODE],
    XtNsensitive, (Boolean) sens,
    NULL);
  XtVaSetValues(view_menu_btn[CTOUR_MODE],
    XtNsensitive, (Boolean) sens,
    NULL);
}

/*ARGSUSED*/
XtActionProc
KeyFileMenu(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;

  if (strcmp(params[0], "0") == 0) {
    XtCallCallbacks(EXIT, XtNcallback, (XtPointer) xg);
  }
}

void
make_file_menu(xgobidata *xg, Widget parent)
{
  int j;

  static char *file_menu_accel[] = {
    "Q",
  };

  static char *file_menu_names[] = {
    "Read ...",
    "Save (extend current file set) ...",
    "Save (create new file set) ...",
    "Print ...",

#ifdef XPLORE
    "XploRe (pass variables) ...",
    "XploRe (pass projection) ...",
#endif

    "Quit (Q)",
  };

  char *accelerators;
  accelerators = XtMalloc(512 * sizeof(char));
  sprintf(accelerators,
    ":<Key>%s:KeyFileMenu(%d)",  /* modifiers:  Alt, Ctrl, Shift<Key>%s */
     file_menu_accel[0], 0);

  file_menu_cmd = XtVaCreateManagedWidget("Command",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "File",
    XtNmenuName, (String) "Menu",
    NULL);
  if (mono) set_mono(file_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    file_menu_cmd, "FileMenu");

  file_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, file_menu_cmd,
    XtNaccelerators, XtParseAcceleratorTable(accelerators),
    NULL);
  if (mono) set_mono(file_menu);

  for (j=0; j<NFILEBTNS; j++)
  {
    file_menu_btn[j] = XtVaCreateWidget("Command",
      smeBSBObjectClass, file_menu,
      XtNlabel, (String) file_menu_names[j],
      NULL);
    if (mono) set_mono(file_menu_btn[j]);

#ifdef XPLORE
    /* Add a line after the 3rd and 5th entries */
    if (j == 3 || j == 5)
#else
    /* Add a line after the 3rd and 4th entries */
    if (j == 2 || j == 3)
#endif

      (void) XtVaCreateManagedWidget("Line",
        smeLineObjectClass, file_menu,
        NULL);
  }

#ifdef XPLORE
  /* Initially make the XploRe buttons insensitive. */
  XtVaSetValues(file_menu_btn[4],
     XtNsensitive, False,
     NULL);
  XtVaSetValues(file_menu_btn[5],
     XtNsensitive, False,
     NULL);
#endif

  XtManageChildren(file_menu_btn, NFILEBTNS);

  XtAddCallback(file_menu_btn[0], XtNcallback,
    (XtCallbackProc) open_import_xgobi_popup_cback, (XtPointer) xg);
  XtAddCallback(file_menu_btn[1], XtNcallback,
    (XtCallbackProc) open_extend_xgobi_popup_cback, (XtPointer) xg);
  XtAddCallback(file_menu_btn[2], XtNcallback,
    (XtCallbackProc) open_new_xgobi_popup_cback, (XtPointer) xg);
  XtAddCallback(file_menu_btn[3], XtNcallback,
    (XtCallbackProc) print_panel_cback, (XtPointer) xg);

#ifdef XPLORE
  XtAddCallback(file_menu_btn[4], XtNcallback,
    (XtCallbackProc) pass_data_xplore_cback, (XtPointer) xg);
  XtAddCallback(file_menu_btn[5], XtNcallback,
    (XtCallbackProc) pass_projection_xplore_cback, (XtPointer) xg);
#endif

/* I probably need a variable to handle this -- how do I
 * know whether I was called as a function or I'm running
 * as a standalone xgobi?
*/
/*
  if (parent)
    XtAddCallback(EXIT, XtNcallback,
      (XtCallbackProc) exit_panel_cback, (XtPointer) xg);
  else
*/
    XtAddCallback(EXIT, XtNcallback,
      (XtCallbackProc) exit_solo_cback, (XtPointer) xg);

  XtFree((char *) accelerators);
}

int
reset_plot_mode(Cardinal mode, xgobidata *xg)
{
/*
 * Rotation and grand tour do not work when there are fewer
 * than three columns.
*/
  Boolean threeD = True;

  if (xg->ncols_used < 3)
    threeD = False;

  switch (mode) {
   case PLOT1D_MODE:
    plot1d_on(xg);
    break;
   case XYPLOT_MODE:
    xyplot_on(xg);
    break;
   case ROTATE_MODE:
    if (threeD)
      rotate_on(xg);
    break;
   case GTOUR_MODE:
    if (threeD)
      grand_tour_on(xg);
    break;
   case CTOUR_MODE:
    if (threeD)
      corr_tour_on(xg);
    break;
   case SCALE_MODE:
    scaling_on(xg);
    break;
   case BRUSH_MODE:
    brush_on(xg);
    break;
   case IDENTIFY_MODE:
    identify_on(xg);
    break;
   case LINEEDIT_MODE:
    line_editor_on(xg);
    break;
   case MOVEPTS_MODE:
    move_points_on(xg);
    break;
   default:
    /* Failed: unknown index. */
    return (-1);
  }
  return mode;
}

/* ARGSUSED */
XtCallbackProc
view_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Set the xgobi mode for plots and interaction
*/
{
  int j;

  xg->prev_plot_mode = xg->plot_mode;
  for (j=0; j<NVIEWMODES; j++) {
    if (view_menu_btn[j] == w) {
      xg->plot_mode = j;
      break;
    }
  }
  XtVaSetValues(view_menu_cmd,
    XtNlabel, (String) view_menu_labelname[xg->plot_mode],
    NULL);

  if (xg->plot_mode != xg->prev_plot_mode) {
    /* turn off previous mode */
    reset_plot_mode(xg->prev_plot_mode, xg);
    /* turn on new mode */
    reset_plot_mode(xg->plot_mode, xg);
  }
}

int
SetPlotModeIndex(int pmode, xgobidata *xg)
{
  int old = xg->plot_mode;

  if (pmode != xg->plot_mode) {
    xg->prev_plot_mode = xg->plot_mode;
    xg->plot_mode = pmode;

    /* turn off previous mode */
    reset_plot_mode(xg->prev_plot_mode, xg);
    /* turn on new mode */
    reset_plot_mode(xg->plot_mode, xg);

    XtVaSetValues(view_menu_cmd,
      XtNlabel, (String) view_menu_labelname[xg->plot_mode],
      NULL);
  }
  return (old);
}


/*ARGSUSED*/
XtActionProc
SetPlotMode(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;
  int pmode;

  pmode = atoi(params[0]);
  SetPlotModeIndex(pmode, xg);
}

/*
 * Changes to the specified view identified by name.
 * This is the name of the label on the menu with the
 * "View:" prefix and key accelerator removed.
*/
int SetPlotModeName(const char *name, xgobidata *xg)
{
  /*
   * Look up the index corresponding to the specified identifier.
   * The identifier should come from the list.
  */
  
  int num = -1;
  char **names = GetPlotViewTypes(&num);
  char *tmp;
  int id = -1;
  int i;

  for (i=0; i<num; i++) {
    /* Match by simple index. */
    tmp = names[i] + strlen("View: ");
    if (strcmp(tmp, name) == 0) {
      id = (Cardinal) i;
      break;
    }
  }
  if (id > -1)
    id = SetPlotModeIndex((Cardinal) id, xg);

  return (id);
}


/*
 * CORBA: returns an array (and its length) or the menu names
 * for controlling the plot/view type.
*/
char **
GetPlotViewTypes(int *num) 
{
  *num = sizeof(view_menu_labelname) / sizeof(view_menu_labelname[0]) ;
  return (view_menu_labelname);
}

void
make_view_menu(xgobidata *xg, Widget parent)
{
  int j, k;
  Dimension width, maxwidth;
  int longest = 0;

  static char *view_menu_fullname[] = {
    "1DPlot (d)",
    "XYPlot (x)",
    "Rotation (r)",
    "Grand Tour (g)",
    "Correlation Tour (c)",
    "Scale (s)",
    "Brush (b)",
    "Identify (i)",
    "Line Editing (l)",
    "Move Points (m)",
  };
  char *accelerators;
  accelerators = XtMalloc(512 * sizeof(char));

  sprintf(accelerators,
    "<Key>%s:SetPlotMode(%d)\n\
     <Key>%s:SetPlotMode(%d)\n\
     <Key>%s:SetPlotMode(%d)\n\
     <Key>%s:SetPlotMode(%d)\n\
     <Key>%s:SetPlotMode(%d)\n\
     <Key>%s:SetPlotMode(%d)\n\
     <Key>%s:SetPlotMode(%d)\n\
     <Key>%s:SetPlotMode(%d)\n\
     <Key>%s:SetPlotMode(%d)\n\
     <Key>%s:SetPlotMode(%d)",
     view_menu_accel[PLOT1D_MODE], PLOT1D_MODE,
     view_menu_accel[XYPLOT_MODE], XYPLOT_MODE,
     view_menu_accel[ROTATE_MODE], ROTATE_MODE,
     view_menu_accel[GTOUR_MODE], GTOUR_MODE,
     view_menu_accel[CTOUR_MODE], CTOUR_MODE,
     view_menu_accel[SCALE_MODE], SCALE_MODE,
     view_menu_accel[BRUSH_MODE], BRUSH_MODE,
     view_menu_accel[IDENTIFY_MODE], IDENTIFY_MODE,
     view_menu_accel[LINEEDIT_MODE], LINEEDIT_MODE,
     view_menu_accel[MOVEPTS_MODE], MOVEPTS_MODE);

  maxwidth = 0;
  longest = k = 0;
  for (k=0; k<NVIEWMODES; k++)
  {
    /*
     * add 3 borderwidths, 2 at the edges and one between the
     * menu bitmap and text, and add the width of menu bitmap
    */
    width = XTextWidth(appdata.font, view_menu_labelname[k],
      strlen(view_menu_labelname[k])) + 3*ASCII_TEXT_BORDER_WIDTH + 12;
    if (width > maxwidth) {
      maxwidth = width;
      longest = k;
    }
  }

  xg->plot_mode = xg->prev_plot_mode = XYPLOT_MODE;
  view_menu_cmd = XtVaCreateManagedWidget("Command",
    menuButtonWidgetClass, parent,
    /* Create the widget using the longest label */
    XtNlabel, (String) view_menu_labelname[longest],
    XtNwidth, maxwidth,
    XtNresize, False,
    XtNmenuName, (String) "Menu",
    NULL);
  if (mono) set_mono(view_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    view_menu_cmd, "ViewMenu");

  view_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, view_menu_cmd,
    XtNinput, True,
    XtNaccelerators, XtParseAcceleratorTable(accelerators),
    NULL);
  if (mono) set_mono(view_menu);

  for (j=0; j<NVIEWMODES; j++) {
    view_menu_btn[j] = XtVaCreateWidget("Command",
      smeBSBObjectClass, view_menu,
      XtNlabel, (String) view_menu_fullname[j],
      NULL);
    if (mono) set_mono(view_menu_btn[j]);

    XtAddCallback(view_menu_btn[j], XtNcallback,
      (XtCallbackProc) view_menu_cback, (XtPointer) xg);

    /* Add a line after the 5th entry */
    if (j == 4)
      (void) XtVaCreateManagedWidget("Line",
          smeLineObjectClass, view_menu,
          NULL);
  }

  XtManageChildren(view_menu_btn, NVIEWMODES);
/* Now reset the mode */
  XtVaSetValues(view_menu_cmd,
    XtNlabel, (String) view_menu_labelname[XYPLOT_MODE],
    NULL);

  XtFree((char *) accelerators);
}

void
set_display_menu_marks(xgobidata *xg)
{
  XtVaSetValues(display_menu_btn[ADDAXES],
    XtNleftBitmap, (xg->is_axes) ? (Pixmap) menu_mark : (Pixmap) None,
    NULL);
  XtVaSetValues(display_menu_btn[ADDGRID],
    XtNleftBitmap, (xg->add_gridlines) ? (Pixmap) menu_mark : (Pixmap) None,
    NULL);
  XtVaSetValues(display_menu_btn[CENTERAXES],
    XtNleftBitmap, (xg->is_axes_centered) ? (Pixmap) menu_mark : (Pixmap) None,
    NULL);
  XtVaSetValues(display_menu_btn[SHOWPOINTS],
    XtNleftBitmap, (xg->plot_the_points) ? (Pixmap) menu_mark : (Pixmap) None,
    NULL);
  XtVaSetValues(display_menu_btn[SHOWARROWS],
    XtNleftBitmap, (xg->plot_the_arrows) ? (Pixmap) menu_mark : (Pixmap) None,
    NULL);
  XtVaSetValues(display_menu_btn[SHOWLINES],
    XtNleftBitmap,
      (xg->connect_the_points) ? (Pixmap) menu_mark : (Pixmap) None,
    NULL);
  XtVaSetValues(display_menu_btn[CARRYVARS],
    XtNleftBitmap, (xg->carry_vars) ? (Pixmap) menu_mark : (Pixmap) None,
    NULL);
}

/* ARGSUSED */
static XtCallbackProc
display_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<NDISPLAYBTNS; btn++)
    if (display_menu_btn[btn] == w)
      break;

  switch (btn) {
    case ADDAXES :
      xg->is_axes = !xg->is_axes;
      plot_once(xg);
      break;
    case ADDGRID :
      xg->add_gridlines = !xg->add_gridlines;
      plot_once(xg);
      break;
    case CENTERAXES :
      xg->is_axes_centered = !xg->is_axes_centered;
      plot_once(xg);
      break;
    case SHOWPOINTS :
      xg->plot_the_points = !xg->plot_the_points;
      plot_once(xg);
      break;
    case SHOWLINES :  /* undirected lines */
      xg->connect_the_points = !xg->connect_the_points;

      if (xg->connect_the_points)
        xg->plot_the_arrows = False;

      /*
       * Set this to 1 in case there are line colors.
      */
      xg->got_new_paint = True;
      plot_once(xg);
      break;

    case SHOWARROWS : /* directed lines */
      xg->plot_the_arrows = !xg->plot_the_arrows;

      if (xg->plot_the_arrows)
        xg->connect_the_points = False;

      xg->got_new_paint = True;
      plot_once(xg);
      break;
    case CARRYVARS :
      xg->carry_vars = !xg->carry_vars;
      break;
  }

  /*
   * Show/don't show all the lines. Make sure the duplicate
   * toggle on the line editor panel has the right value.
  */
  set_showlines(xg->connect_the_points || xg->plot_the_arrows);

  set_display_menu_marks(xg);
}

/*ARGSUSED*/
XtActionProc
KeyDisplayMenu(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;
  int accel = atoi(params[0]);
  XtCallCallbacks(display_menu_btn[accel], XtNcallback, (XtPointer) xg);
}
/*ARGSUSED*/
XtActionProc
KeyToolMenu(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;

  if (strcmp(params[0], "8") == 0) {
    XtCallCallbacks(tool_menu_btn[8], XtNcallback, (XtPointer) xg);
  }
}


static void
make_display_menu(xgobidata *xg, Widget parent)
{
  int k;

  static char *menu_accel[] = {
    "a", "n"
  };

  static char *display_menu_name[] = {
    "Add axes to plot (a)",
    "Add gridlines to plot (n)",
    "Center axes in 3D+ modes",
    "Plot the points",
    "Show undirected lines",
    "Show directed lines",
    "Carry variables between views",
  };

  char *accelerators;
  accelerators = XtMalloc(512 * sizeof(char));
  sprintf(accelerators,
    "<Key>%s:KeyDisplayMenu(%d)\n\
     <Key>%s:KeyDisplayMenu(%d)",  /* modifiers:  Alt, Ctrl, Shift<Key>%s */
     menu_accel[ADDAXES], ADDAXES,
     menu_accel[ADDGRID], ADDGRID);

  display_menu_cmd = XtVaCreateManagedWidget("Command",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Display",
    XtNmenuName, (String) "Menu",
    NULL);
  if (mono) set_mono(display_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    display_menu_cmd, "DisplayOptions");

  display_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, display_menu_cmd,
    XtNaccelerators, XtParseAcceleratorTable(accelerators),
    XtNinput, True,
    NULL);
  if (mono) set_mono(display_menu);

  for (k=0; k<NDISPLAYBTNS; k++)
  {
    display_menu_btn[k] = XtVaCreateWidget("Command",
      smeBSBObjectClass, display_menu,
      XtNleftMargin, (Dimension) 24,
      XtNleftBitmap, menu_mark,
      XtNlabel, (String) display_menu_name[k],
      NULL);
    if (mono) set_mono(display_menu_btn[k]);

    XtAddCallback(display_menu_btn[k], XtNcallback,
      (XtCallbackProc) display_menu_cback, (XtPointer) xg);

    /* Add a line after the 6th entry */
    if (k == 5)
      (void) XtVaCreateManagedWidget("Line",
          smeLineObjectClass, display_menu,
          NULL);
  }

  XtManageChildren(display_menu_btn, NDISPLAYBTNS);

  XtFree((char *) accelerators);
}

void
turn_on_xyplotting(xgobidata *xg)
{
  XtCallCallbacks(view_menu_btn[XYPLOT_MODE], XtNcallback, (XtPointer) xg);
}

void
turn_off_cprof_plotting(xgobidata *xg)
{
  XtCallCallbacks(tool_menu_btn[PARCOORD_BTN], XtNcallback, (XtPointer) xg);
}

void
set_sens_missing_menu_btns(Boolean sens)
{
  XtSetSensitive(tool_menu_btn[LAUNCH_MISSING_BTN], sens);
  XtSetSensitive(tool_menu_btn[IMPUTE_BTN], sens);
}

void
make_tool_menu(xgobidata *xg, Widget parent)
{
  int j;

  static char *tool_menu_fullname[] = {
    "Hide or exclude ...",
    "Sample or subset ...",
    "Smooth ...",
    "Jitter ...",
    "Graphical inference ...",
    "Parallel coordinates plot ...",
    "Clone XGobi ...",
    "Scatterplot matrix ...",
    "Variable transformation ... (t)",
    "Variable list ...",
    "Case list ...",
    "Launch missing data XGobi ...",
    "Impute missing values ...",

  #ifdef XPLORE
    "Start XploRe ...",
    "Stop XploRe ...",
  #endif

  };

  static char *menu_accel[] = {
    "t",
  };

  char *accelerators;
  accelerators = XtMalloc(512 * sizeof(char));
  sprintf(accelerators,
    "<Key>%s:KeyToolMenu(%d)",  /* modifiers:  Alt, Ctrl, Shift<Key>%s */
     menu_accel[0], 8);

  tool_menu_cmd = XtVaCreateManagedWidget("ToolMenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel,    "Tools",
    XtNmenuName, "Menu",
    NULL);
  if (mono) set_mono(tool_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    tool_menu_cmd, "ToolMenu");

  tool_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, tool_menu_cmd,
    XtNaccelerators, XtParseAcceleratorTable(accelerators),
    XtNinput, True,
    NULL);
  if (mono) set_mono(tool_menu);

  for (j=0; j<NTOOLBTNS; j++)
  {
    tool_menu_btn[j] = XtVaCreateWidget("Command",
      smeBSBObjectClass, tool_menu,
      XtNlabel, (String) tool_menu_fullname[j],
      NULL);
    if (mono) set_mono(tool_menu_btn[j]);

    /* Add some lines ... */

#ifdef XPLORE
    if (j == 1 || j == 4 || j == 7 || j == 10 || j == 11)
#else
    if (j == 1 || j == 4 || j == 7 || j == 10)
#endif

      (void) XtVaCreateManagedWidget("Line",
          smeLineObjectClass, tool_menu,
          NULL);
  }

  set_sens_missing_menu_btns(xg->missing_values_present);

  if (xg->nrgroups > 0)
    XtSetSensitive(tool_menu_btn[SCATMAT_BTN], False);

  XtAddCallback(tool_menu_btn[CLONE_BTN], XtNcallback,
    (XtCallbackProc) clone_xgobi_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[JITTER_BTN], XtNcallback,
    (XtCallbackProc) open_jitter_popup_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[LAUNCH_MISSING_BTN], XtNcallback,
    (XtCallbackProc) launch_missing_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[IMPUTE_BTN], XtNcallback,
    (XtCallbackProc) open_imputation_popup_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[SMOOTH_BTN], XtNcallback,
    (XtCallbackProc) open_smooth_popup_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[PARCOORD_BTN], XtNcallback,
    (XtCallbackProc) cprof_plot_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[SUBSET_BTN], XtNcallback,
    (XtCallbackProc) subset_panel_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[VARTFORM_BTN], XtNcallback,
    (XtCallbackProc) open_tform_popup_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[VARLIST_BTN], XtNcallback,
    (XtCallbackProc) PopupVarlist, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[CASELIST_BTN], XtNcallback,
    (XtCallbackProc) PopupCaselist, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[SCATMAT_BTN], XtNcallback,
    (XtCallbackProc) scatmat_xgobi_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[INFER_BTN], XtNcallback,
    (XtCallbackProc) open_infer_popup_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[EXCLUSION_BTN], XtNcallback,
    (XtCallbackProc) open_exclusion_popup_cback, (XtPointer) xg);

#ifdef XPLORE
  XtAddCallback(tool_menu_btn[XPLORE_BTN], XtNcallback,
    (XtCallbackProc) start_xplore_cback, (XtPointer) xg);
  XtAddCallback(tool_menu_btn[XPLORE_END_BTN], XtNcallback,
    (XtCallbackProc) stop_xplore_cback, (XtPointer) xg);
#endif

  /*
   * If a .nlinkable file is used, then subsetting
   * is to be disabled.
  XtVaSetValues(tool_menu_btn[SUBSET_BTN],
    XtNsensitive, (xg->nlinkable == xg->nrows),
    NULL);
  */

#ifdef XPLORE
   XtVaSetValues(tool_menu_btn[XPLORE_END_BTN],
     XtNsensitive, False,
     NULL);
#endif

  /*
   * If there's no missing data, disable the 'missing' buttons.
   * Ditto if this is already the missing values xgobi.
  */
  if (!xg->missing_values_present || xg->is_missing_values_xgobi) {
    XtVaSetValues(tool_menu_btn[LAUNCH_MISSING_BTN],
      XtNsensitive, False,
      NULL);
    XtVaSetValues(tool_menu_btn[IMPUTE_BTN],
      XtNsensitive, False,
      NULL);
  }

  XtManageChildren(tool_menu_btn, NTOOLBTNS);

  XtFree((char *) accelerators);
}

void active_xplore_buttons ()

{
#ifdef XPLORE

   XtVaSetValues(tool_menu_btn[XPLORE_BTN], /* Start XploRe */
     XtNsensitive, False,
     NULL);

   XtVaSetValues(tool_menu_btn[XPLORE_END_BTN], /* End XploRe */
     XtNsensitive, True,
     NULL);

   XtVaSetValues(file_menu_btn[4], /* XploRe (pass variables) */
     XtNsensitive, True,
     NULL);

   XtVaSetValues(file_menu_btn[5], /* XploRe (pass projection) */
     XtNsensitive, True,
     NULL);

#endif
}

void inactive_xplore_buttons ()
{
#ifdef XPLORE

   XtVaSetValues(tool_menu_btn[XPLORE_BTN], /* Start XploRe */
     XtNsensitive, True,
     NULL);

   XtVaSetValues(tool_menu_btn[XPLORE_END_BTN], /* End XploRe */
     XtNsensitive, False,
     NULL);

   XtVaSetValues(file_menu_btn[4], /* XploRe (pass variables) */
     XtNsensitive, False,
     NULL);

   XtVaSetValues(file_menu_btn[5], /* XploRe (pass projection) */
     XtNsensitive, False,
     NULL);

#endif
}

void
make_widgets(xgobidata *xg)
{
  Widget info_menu_cmd, info_menu, info_menu_btn[2];

  xg->form0 = XtVaCreateManagedWidget("Form0",
    formWidgetClass, xg->shell,
    NULL);
  if (mono) set_mono(xg->form0);

/*
 * Define main_panel, the main control panel
*/
  main_panel = XtVaCreateManagedWidget("MainPanel",
    boxWidgetClass, xg->form0,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    NULL);
  if (mono) set_mono(main_panel);

  make_file_menu(xg, main_panel);
  make_view_menu(xg, main_panel);
  make_tool_menu(xg, main_panel);
  make_display_menu(xg, main_panel);

  info_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, main_panel,
    XtNlabel, (String) "Info",
    XtNmenuName, (String) "Menu",
    /*XtNfromHoriz, IO,*/
    NULL);
  if (mono) set_mono(info_menu_cmd);

/* Build the info menu right here */
  info_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, info_menu_cmd,
    NULL);
  info_menu_btn[0] = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, info_menu,
    XtNlabel, (String) "About XGobi ... ",
    NULL);
  XtAddCallback(info_menu_btn[0], XtNcallback,
    (XtCallbackProc) about_xgobi_cback, (XtPointer) xg);
  info_menu_btn[1] = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, info_menu,
    XtNlabel, (String) "About help ... ",
    NULL);
  XtAddCallback(info_menu_btn[1], XtNcallback,
    (XtCallbackProc) help_cback, (XtPointer) xg);
/* End of info menu */

/*
 * Paned widget to help users adjust the plot window and the
 * variable selection panel.
*/
  form1 = XtVaCreateManagedWidget("Form1",
    panedWidgetClass, xg->form0,
    XtNfromVert, (Widget) main_panel,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainRight,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtRubber,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(form1);

  xg->box0 = XtVaCreateManagedWidget("Box0",
    formWidgetClass, form1,
    XtNmin, 5,
    NULL);
  if (mono) set_mono(xg->box0);

  xg->box1 = XtVaCreateManagedWidget("Box1",
    formWidgetClass, form1,
    XtNmin, 5,
    NULL);
  if (mono) set_mono(xg->box1);

  xg->box2 = XtVaCreateManagedWidget("Box2",
    formWidgetClass, form1,
    XtNmin, 5,
    NULL);
  if (mono) set_mono(xg->box2);
}

#undef NDISPLAYBTNS
#undef NFILEBTNS
#undef NTOOLBTNS
