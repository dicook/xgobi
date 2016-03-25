/* brush_init.c */
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

static Widget brush_panel[4] ;
static Widget brush_active_cmd;
static Widget glyph_menu_cmd, glyph_menu, glyph_menu_btn[NGLYPHS+1] ;
static Widget color_menu_cmd, color_menu, color_menu_btn[NCOLORS+3];
static Widget glyph_wksp;
static Widget br_type[3], br_update;
static Widget reset_menu_cmd, reset_menu, reset_menu_btn[4];
static Widget brush_points_cmd, brush_lines_cmd;

Widget br_opt_menu_cmd, br_opt_menu, br_opt_menu_btn[3];

Widget br_linkopt_menu_cmd, br_linkopt_menu, br_linkopt_menu_btn[6];
#define LINK_P_TO_P 0
#define LINK_L_TO_L 1
#define LINK_P_TO_L 2
#define LINK_COLOR  3
#define LINK_GLYPH  4
#define LINK_ERASE  5

#define PERST_CMD br_type[0]
#define TRANS_CMD br_type[1]
#define UNDO_CMD br_type[2]

#define JUMP_BRUSH    0
#define RESHAPE_BRUSH 1
#define SYNC_BRUSH    2

#define PANEL0 brush_panel[0]
#define PANEL1 brush_panel[1]
#define PANEL2 brush_panel[2]
#define PANEL3 brush_panel[3]

void
draw_current_glyph(xgobidata *xg)
{
/*
 * Draw the current glyph in the current color in the
 * glyph_wksp.  If no glyph is chosen, draw a border.
*/
  Drawable gwin = XtWindow(glyph_wksp);
  icoords xypos[1];
  unsigned long col;

  /* Clear it */
  XFillRectangle(display, gwin, clear_GC, 0, 0, 40, 40);

/*
 * The center is at 20, 20 ... 
*/
  xypos[0].x = xypos[0].y = 20;

  /* Set the foreground color */
  if (!mono) {
    col = (xg->color_id == plotcolors.bg) ? plotcolors.fg : xg->color_id;
    XSetForeground(display, copy_GC, col);
  }

  if (xg->is_glyph_painting) {
    int type = xg->glyph_id.type;
    int size = xg->glyph_id.size;

    if (type == PLUS_GLYPH) {
      XSegment segv[2];
      build_plus(xypos, 0, segv, 0, size);
      XDrawSegments(display, gwin, copy_GC,
        segv, 2);
    }
    else if (type == X_GLYPH) {
      XSegment segv[2];
      build_x(xypos, 0, segv, 0, size);
      XDrawSegments(display, gwin, copy_GC,
        segv, 2);
    }
    else if (type == OPEN_RECTANGLE_GLYPH) {
      XRectangle rectv[1];
      build_rect(xypos, 0, rectv, 0, size);
      XDrawRectangles(display, gwin, copy_GC, rectv, 1);
    }
    else if (type == FILLED_RECTANGLE_GLYPH) {
      XRectangle rectv[1];
      build_rect(xypos, 0, rectv, 0, size);
      XFillRectangles(display, gwin, copy_GC, rectv, 1);
    }
    else if (type == OPEN_CIRCLE_GLYPH) {
      XArc circ[1];
      build_circle(xypos, 0, circ, 0, size);
      XDrawArcs(display, gwin, copy_GC, circ, 1);
    }
    else if (type == FILLED_CIRCLE_GLYPH) {
      XArc circ[1];
      build_circle(xypos, 0, circ, 0, size);
      XFillArcs(display, gwin, copy_GC, circ, 1);
    }
    else if (type == POINT_GLYPH) {
      XDrawPoint(display, gwin, copy_GC, xypos[0].x, xypos[0].y);
    }

  /* If color painting only */
  } else if (xg->is_color_painting) {
    XDrawRectangle(display, gwin, copy_GC, 0, 0, 39, 39);
  }

  /* If painting in the background color, signal that with a diagonal */
  if (xg->color_id == plotcolors.bg)
    XDrawLine(display, gwin, copy_GC, 0, 0, 40, 40);

  XFlush(display);
  XSync(display, False);

}

/* ARGSUSED */
static XtEventHandler
glyph_expose(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  draw_current_glyph(xg);
}

static Boolean
choose_color(xgobidata *xg, int color_btn)
{
  int i;
  Boolean is_cpainting;
  Arg args[5];

/*
 * Turn off margin indicator on each menu item.
*/
  XtSetArg(args[0], XtNleftBitmap, (Pixmap) None);
  for (i=0; i<ncolors+1; i++)
    XtSetValues(color_menu_btn[i], args, 1);

/*
 * Shade the menu button to indicate that it is presently inactive.
*/
  if (color_btn == 0)
    is_cpainting = False;
  else {
    is_cpainting = True;
    xg->color_id = (unsigned long) color_nums[color_btn-1];
  }

  XtVaSetValues(color_menu_btn[color_btn],
    XtNleftBitmap, (Pixmap) menu_mark,
    NULL);

  return(is_cpainting);
}

/* ARGSUSED */
XtCallbackProc
choose_color_cback (Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Choose brushing color:  only available on color displays.
*/
{
  int j;
  int color_btn;

  for (j=0; j<ncolors+1; j++) {
    if (color_menu_btn[j] == w) {
      color_btn = j;
      break;
    }
  }

  if (xg->brush_mode == undo)
  {
    char message[MSGLENGTH];
    sprintf(message, "Cannot change color when in undo brushing mode\n");
    show_message(message, xg);
  }
  else
  {
    xg->is_color_painting = choose_color(xg, color_btn);
    draw_current_glyph(xg);

    for (j=0; j<xg->nrows; j++) {
      xg->color_prev[j] = xg->color_ids[j];

      /* experiment; dfs */
      xg->glyph_prev[j].type = xg->glyph_ids[j].type;
      xg->glyph_prev[j].size = xg->glyph_ids[j].size;
    }

    plot_once(xg);
  }
}

void
reinit_brush_colors(xgobidata *xg)
{
  int j;
  Boolean fg_present = false, bg_present = false;

  /*
   * Find out whether the foreground and background colors of the
   * plot_window are included in the brushing colors.  If not, then
   * append them to the list of colors and add them to the color
   * brushing menu.
  */
  for (j=0; j<ncolors; j++) {
    if (color_nums[j] == plotcolors.fg)
      fg_present = true;
    else if (color_nums[j] == plotcolors.bg)
      bg_present = true;
  }

  if (!bg_present) {
    color_nums[ncolors] = plotcolors.fg;
    color_names[ncolors] = "Default";
    color_menu_btn[ncolors+1] = XtVaCreateManagedWidget("Command",
      smeBSBObjectClass, color_menu,
      XtNlabel, (String) color_names[ncolors],
      XtNleftMargin, (Dimension) 24,
      XtNforeground, color_nums[ncolors],
      NULL);
    XtAddCallback(color_menu_btn[ncolors+1], XtNcallback,
      (XtCallbackProc) choose_color_cback, (XtPointer) xg);
    ncolors++;
  }

  if (!fg_present) {
    color_nums[ncolors] = plotcolors.bg;
    color_names[ncolors] = "<Background color>";
    color_menu_btn[ncolors+1] = XtVaCreateManagedWidget("Command",
      smeBSBObjectClass, color_menu,
      XtNlabel, (String) color_names[ncolors],
      XtNleftMargin, (Dimension) 24,
      XtNforeground, plotcolors.fg,
      NULL);
    XtAddCallback(color_menu_btn[ncolors+1], XtNcallback,
      (XtCallbackProc) choose_color_cback, (XtPointer) xg);
    ncolors++;
  }

  if (ncolors > NCOLORS+2)
    fprintf(stderr, "Error in reinit_brush_colors: Too many colors\n");
}

/*
 * 1-5     +
 * 6-10    x
 * 11-15   orect
 * 16-20   crect
 * 21-25   ocirc
 * 26-30   ccirc
 * 31      point
*/
void
find_glyph_type_and_size(int gid, glyphv *glyph)
{
  glyph->type = ( (gid-1) / NGLYPHSIZES ) + 1 ;
  glyph->size = ( (gid-1) % NGLYPHSIZES ) + 1 ;
}

static Boolean
choose_glyph(xgobidata *xg, int glyph_btn)
{
  Boolean is_gpainting;
  glyphv glyph;
  int i;
  Arg args[5];

  XtSetArg(args[0], XtNleftBitmap, (Pixmap) None);
  for (i=0; i<NGLYPHS+1; i++)
    XtSetValues(glyph_menu_btn[i], args, 1);

  if (glyph_btn == 0) {
    is_gpainting = False;
  }
  else {
    is_gpainting = True;

    find_glyph_type_and_size(glyph_btn, &glyph);
    xg->glyph_id.type = glyph.type ;
    xg->glyph_id.size = glyph.size ;
  }

  XtSetArg(args[0], XtNleftBitmap, (Pixmap) menu_mark);
  XtSetValues(glyph_menu_btn[glyph_btn], args, 1);

  return(is_gpainting);
}

/* ARGSUSED */
XtCallbackProc
choose_glyph_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  int j;
  int glyph_btn;

  for (j=0; j<NGLYPHS+1; j++)
  {
    if (glyph_menu_btn[j] == w)
    {
      glyph_btn = j;
      break;
    }
  }

  if (xg->brush_mode == undo)
  {
    char message[MSGLENGTH];
    sprintf(message, "Cannot change glyph when in undo brushing mode\n");
    show_message(message, xg);
  }
  else
  {
    xg->is_glyph_painting = choose_glyph(xg, glyph_btn);
    draw_current_glyph(xg);

    for (j=0; j<xg->nrows; j++)
    {
      xg->glyph_prev[j].type = xg->glyph_ids[j].type;
      xg->glyph_prev[j].size = xg->glyph_ids[j].size;

      /* experiment; dfs */
      xg->color_prev[j] = xg->color_ids[j];
    }
  }
}

void
make_color_menu(xgobidata *xg, Widget parent, Widget vref)
{
  int i, j;

/*
 * Menu for choosing the current brushing color.
*/
  color_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Color",
    XtNmenuName, (String) "Menu",
    XtNfromVert, vref,
    NULL);
  if (mono)
  {
    set_mono(color_menu_cmd);
    XtVaSetValues(color_menu_cmd,
      XtNsensitive, (Boolean) False,
      NULL);
  }
  add_menupb_help(&xg->nhelpids.menupb,
    color_menu_cmd, "Br_ColorMenu");

  color_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, color_menu_cmd,
    NULL);

/*
 * Color brushing starts out disabled for now ...
*/
  color_menu_btn[0] = XtVaCreateWidget("Command",
    smeBSBObjectClass, color_menu,
    XtNlabel, (String) "Disable Color Painting",
    XtNleftMargin, (Dimension) 24,
    NULL);

  for (j=0; j<ncolors; j++) {
    color_menu_btn[j+1] = XtVaCreateWidget("Command",
      smeBSBObjectClass, color_menu,
      XtNlabel, (String) color_names[j],
      XtNleftMargin, (Dimension) 24,
      XtNforeground, color_nums[j],
      NULL);
  }
  XtManageChildren(color_menu_btn, ncolors+1);

  for (i=0; i<ncolors+1; i++) {
    XtAddCallback(color_menu_btn[i], XtNcallback,
      (XtCallbackProc) choose_color_cback, (XtPointer) xg);
  }
}

void
make_glyph_menu(xgobidata *xg, Widget parent, Widget vref, Widget href)
{
  int j;
  static char *glyph_names[] = {
    "Tiny Plus (1)",
    "Small Plus (2)",
    "Medium Plus (3)",
    "Large Plus (4)",
    "Jumbo Plus (5)",
    "Tiny X (6)",
    "Small X (7)",
    "Medium X (8)",
    "Large X (9)",
    "Jumbo X (10)",
    "Tiny Open Rectangle (11)",
    "Small Open Rectangle (12)",
    "Medium Open Rectangle (13)",
    "Large Open Rectangle (14)",
    "Jumbo Open Rectangle (15)",
    "Tiny Filled Rectangle (16)",
    "Small Filled Rectangle (17)",
    "Medium Filled Rectangle (18)",
    "Large Filled Rectangle (19)",
    "Jumbo Filled Rectangle (20)",
    "Tiny Open Circle (21)",
    "Small Open Circle (22)",
    "Medium Open Circle (23)",
    "Large Open Circle (24)",
    "Jumbo Open Circle (25)",
    "Tiny Filled Circle (26)",
    "Small Filled Circle (27)",
    "Medium Filled Circle (28)",
    "Large Filled Circle (29)",
    "Jumbo Filled Circle (30)",
    "Point (31)"
  };

/*
 * Menu for choosing the current brushing glyph.
*/
  glyph_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Glyph",
    XtNmenuName, (String) "Menu",
    XtNfromVert, vref,
    XtNfromHoriz, href,
    NULL);
  if (mono) set_mono(glyph_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    glyph_menu_cmd, "Br_GlyphMenu");

  glyph_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, glyph_menu_cmd,
    NULL);
  if (mono) set_mono(glyph_menu);
/*
 * Set the background of the initial glyph.
*/
  glyph_menu_btn[0] = XtVaCreateWidget("Command",
    smeBSBObjectClass, glyph_menu,
    XtNlabel, (String) "Disable Glyph Painting",
    XtNleftMargin, (Dimension) 24,
    XtNleftBitmap, menu_mark,
    NULL);
  if (mono) set_mono(glyph_menu_btn[0]);

  /*for (j=0; j<NGLYPHS+1; j++)*/
  for (j=0; j<NGLYPHS; j++)
  {
    glyph_menu_btn[j+1] = XtVaCreateWidget("Command",
      smeBSBObjectClass, glyph_menu,
      XtNlabel, (String) glyph_names[j],
      XtNleftMargin, (Dimension) 24,
      XtNleftBitmap, None,
      NULL);
    if (mono) set_mono(glyph_menu_btn[j+1]);
  }
  XtManageChildren(glyph_menu_btn, NGLYPHS+1);

  for (j=0; j<NGLYPHS+1; j++)
    XtAddCallback(glyph_menu_btn[j], XtNcallback,
      (XtCallbackProc) choose_glyph_cback, (XtPointer) xg);
}

void
make_reset_menu(xgobidata *xg, Widget parent, Widget vref)
/*
 * Build a menu to contain brushing reset operations.
*/
{
  int j;
  static char *reset_names[] = {
    "Reset brush size",
    "Reset point colors",
    "Reset line colors",
    "Reset glyphs"
  };

  reset_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Reset",
    XtNmenuName, (String) "Menu",
    XtNfromVert, vref,
    NULL);
  if (mono) set_mono(reset_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    reset_menu_cmd, "Br_ResetMenu");

  reset_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, reset_menu_cmd,
    NULL);
  if (mono) set_mono(reset_menu);

  for (j=0; j<4; j++)
  {
    reset_menu_btn[j] = XtVaCreateWidget("Command",
      smeBSBObjectClass, reset_menu,
      XtNlabel, (String) reset_names[j],
      NULL);
    if (mono) set_mono(reset_menu_btn[j]);
  }
  XtManageChildren(reset_menu_btn, 4);

  XtAddCallback(reset_menu_btn[0], XtNcallback,
    (XtCallbackProc) reset_brush_cback, (XtPointer) xg);
  XtAddCallback(reset_menu_btn[1], XtNcallback,
    (XtCallbackProc) reset_point_colors_cback, (XtPointer) xg);
  XtAddCallback(reset_menu_btn[2], XtNcallback,
    (XtCallbackProc) reset_line_colors_cback, (XtPointer) xg);
  XtAddCallback(reset_menu_btn[3], XtNcallback,
    (XtCallbackProc) reset_glyphs_cback, (XtPointer) xg);
}

void
set_br_linkopt_menu_marks(xgobidata *xg)
{
  XtVaSetValues(br_linkopt_menu_btn[LINK_P_TO_P],
    XtNleftBitmap, (xg->link_points_to_points) ? menu_mark : None,
    NULL);

  XtVaSetValues(br_linkopt_menu_btn[LINK_L_TO_L],
    XtNleftBitmap, (xg->link_lines_to_lines) ? menu_mark : None,
    NULL);

  XtVaSetValues(br_linkopt_menu_btn[LINK_P_TO_L],
    XtNleftBitmap, (xg->link_points_to_lines) ? menu_mark : None,
    NULL);

  XtVaSetValues(br_linkopt_menu_btn[LINK_COLOR],
    XtNleftBitmap, (xg->link_color_brushing) ? menu_mark : None,
    NULL);

  XtVaSetValues(br_linkopt_menu_btn[LINK_GLYPH],
    XtNleftBitmap, (xg->link_glyph_brushing) ? menu_mark : None,
    NULL);

  XtVaSetValues(br_linkopt_menu_btn[LINK_ERASE],
    XtNleftBitmap, (xg->link_erase_brushing) ? menu_mark : None,
    NULL);
}

/* ARGSUSED */
XtCallbackProc
br_linkopt_menu_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<5; btn++)
    if (br_linkopt_menu_btn[btn] == w)
      break;

  switch (btn) {

    case LINK_P_TO_P :
      xg->link_points_to_points = !xg->link_points_to_points;
      if (xg->link_points_to_points) {
        xg->link_points_to_lines = False;
        XtSetSensitive(br_linkopt_menu_btn[LINK_GLYPH], True);
        XtSetSensitive(br_linkopt_menu_btn[LINK_ERASE], True);
      }
      break;

    case LINK_L_TO_L :
      xg->link_lines_to_lines = !xg->link_lines_to_lines;
      if (xg->link_lines_to_lines) {
        xg->link_points_to_lines = False;
        XtSetSensitive(br_linkopt_menu_btn[LINK_GLYPH], True);
        XtSetSensitive(br_linkopt_menu_btn[LINK_ERASE], True);
      }
      break;

    case LINK_P_TO_L :
      xg->link_points_to_lines = !xg->link_points_to_lines;
      if (xg->link_points_to_lines) {
        xg->link_points_to_points = False;
        xg->link_lines_to_lines = False;
        /* linking of glyphs or erasing is undefined here */
        xg->link_erase_brushing = False;
        xg->link_glyph_brushing = False;
        XtSetSensitive(br_linkopt_menu_btn[LINK_GLYPH], False);
        XtSetSensitive(br_linkopt_menu_btn[LINK_ERASE], False);
      }
      break;

    case LINK_COLOR :
      xg->link_color_brushing = !xg->link_color_brushing;
      break;
    case LINK_GLYPH :
      xg->link_glyph_brushing = !xg->link_glyph_brushing;
      break;
    case LINK_ERASE :
      xg->link_erase_brushing = !xg->link_erase_brushing;
      break;
  }

  /*
   * If points -> points is false and points -> lines is also
   * false, then it makes no sense to have erase/glyph/color
   * linking true -- turn them all off.
  */
  if (!xg->link_points_to_points && !xg->link_points_to_lines) {
    xg->link_erase_brushing = False;
    xg->link_glyph_brushing = False;
    xg->link_color_brushing = False;
  }

  set_br_linkopt_menu_marks(xg);
}

void
make_br_linkopt_menu(xgobidata *xg, Widget parent, Widget href, Widget vref)
{
  int k;

  static char *br_linkopt_menu_name[] = {
    "Link points <-> points",
    "Link lines <-> lines",
    "Link points <-> lines",
    "Link color brushing",
    "Link glyph brushing",
    "Link hidden state",
  };

  br_linkopt_menu_cmd = XtVaCreateManagedWidget("LinkBrushButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Link",
    XtNmenuName, (String) "Menu",
    XtNfromHoriz, href,
    XtNfromVert, vref,
    NULL);
  if (mono) set_mono(br_linkopt_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    br_linkopt_menu_cmd, "LinkBrush");

  br_linkopt_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, br_linkopt_menu_cmd,
    XtNinput, True,
    NULL);
  if (mono) set_mono(br_linkopt_menu);

  for (k=0; k<6; k++) {
    br_linkopt_menu_btn[k] = XtVaCreateManagedWidget("Command",
      smeBSBObjectClass, br_linkopt_menu,
      XtNleftMargin, (Dimension) 24,
      XtNleftBitmap, menu_mark,
      XtNlabel, (String) br_linkopt_menu_name[k],
      NULL);
    if (mono) set_mono(br_linkopt_menu_btn[k]);

    if (k==2) {
      Widget line = XtVaCreateManagedWidget("Line",
        smeLineObjectClass, br_linkopt_menu,
        NULL);
      if (mono) set_mono(line);
    }

    XtAddCallback(br_linkopt_menu_btn[k], XtNcallback,
      (XtCallbackProc) br_linkopt_menu_cback, (XtPointer) xg);
  }

}

void
set_br_opt_menu_marks(xgobidata *xg)
{
  if (xg->jump_brush)
    XtVaSetValues(br_opt_menu_btn[JUMP_BRUSH],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
  else
    XtVaSetValues(br_opt_menu_btn[JUMP_BRUSH],
      XtNleftBitmap, (Pixmap) None,
      NULL);

  if (xg->reshape_brush)
    XtVaSetValues(br_opt_menu_btn[RESHAPE_BRUSH],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
  else
    XtVaSetValues(br_opt_menu_btn[RESHAPE_BRUSH],
      XtNleftBitmap, (Pixmap) None,
      NULL);

  if (xg->sync_brush)
    XtVaSetValues(br_opt_menu_btn[SYNC_BRUSH],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
  else
    XtVaSetValues(br_opt_menu_btn[SYNC_BRUSH],
      XtNleftBitmap, (Pixmap) None,
      NULL);
}

/* ARGSUSED */
XtCallbackProc
br_opt_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<3; btn++)
    if (br_opt_menu_btn[btn] == w)
      break;

  switch (btn) {
    case JUMP_BRUSH :
      xg->jump_brush = !xg->jump_brush;
      break;
    case RESHAPE_BRUSH :
      xg->reshape_brush = !xg->reshape_brush;
      break;
    case SYNC_BRUSH :
      xg->sync_brush = !xg->sync_brush;
      break;
  }
  set_br_opt_menu_marks(xg);
}

void
make_br_opt_menu(xgobidata *xg, Widget parent, Widget href, Widget vref)
{
  int k;

  static char *br_opt_menu_name[] = {
    "Brush jumps to cursor",
    "Reshape brush when re-entering brushing",
    "Update linked brushing continuously",
  };

  br_opt_menu_cmd = XtVaCreateManagedWidget("LinkBrushButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Options",
    XtNmenuName, (String) "Menu",
    XtNfromHoriz, href,
    XtNfromVert, vref,
    NULL);
  if (mono) set_mono(br_opt_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    br_opt_menu_cmd, "IdentifyLink");

  br_opt_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, br_opt_menu_cmd,
    XtNinput, True,
    NULL);
  if (mono) set_mono(br_opt_menu);

  for (k=0; k<3; k++)
  {
    br_opt_menu_btn[k] = XtVaCreateWidget("Command",
      smeBSBObjectClass, br_opt_menu,
      XtNleftMargin, (Dimension) 24,
      XtNleftBitmap, menu_mark,
      XtNlabel, (String) br_opt_menu_name[k],
      NULL);
    if (mono) set_mono(br_opt_menu_btn[k]);

    XtAddCallback(br_opt_menu_btn[k], XtNcallback,
      (XtCallbackProc) br_opt_menu_cback, (XtPointer) xg);
  }

  XtManageChildren(br_opt_menu_btn, 3);
}

void
make_brush(xgobidata *xg)
{
  Widget hide_cmd;

/*
 * BrushPanel:  define but don't map.  It goes in the same spot
 * occupied by spin_panel[0].
*/
  PANEL0 = XtVaCreateManagedWidget("BrushPanel",
    formWidgetClass, xg->box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
    XtNorientation, (XtOrientation) XtorientVertical,
    NULL);
  if (mono) set_mono(PANEL0);
/*
 * Button to turn active painting on and off.
*/
  brush_active_cmd = CreateToggle(xg, "Brush on",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, True, ANY_OF_MANY,
    PANEL0, "Br_BrushOn");
  XtManageChild(brush_active_cmd);
  XtAddCallback(brush_active_cmd, XtNcallback,
    (XtCallbackProc) brush_active_cback, (XtPointer) xg);

/*
 * Panel for specifying line and/or point brushing.
*/
  PANEL1 = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, PANEL0,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    XtNfromVert, brush_active_cmd,
    NULL);
  if (mono) set_mono(PANEL1);
  brush_points_cmd = CreateToggle(xg, "Points",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, True, ANY_OF_MANY,
    PANEL1, "Br_PointsLines");
  XtManageChild(brush_points_cmd );
  XtAddCallback(brush_points_cmd, XtNcallback,
    (XtCallbackProc) brush_points_cback, (XtPointer) xg);

/*
 * Only activate this if the machine is not monochrome
*/
  brush_lines_cmd = CreateToggle(xg, "Lines",
    !mono, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, ANY_OF_MANY,
    PANEL1, "Br_PointsLines");
  XtManageChild(brush_lines_cmd );
  XtAddCallback(brush_lines_cmd, XtNcallback,
    (XtCallbackProc) brush_lines_cback, (XtPointer) xg);
/*
 * Menus for choosing the brushing color and glyph.
*/
  make_color_menu(xg, PANEL0, PANEL1 );  /* ... parent, vref */
  make_glyph_menu(xg, PANEL0, PANEL1, color_menu_cmd );  /* ... vref, href */

/*
 * How about a small window with the current glyph drawn in it,
 * in the current color?
*/
  glyph_wksp = XtVaCreateManagedWidget("PlotWindow",
    labelWidgetClass, PANEL0,
    XtNresizable, (Boolean) False,
    XtNfromVert, (Widget) color_menu_cmd,
    XtNlabel, (String) "",
    XtNbackground, plotcolors.bg,
    XtNwidth, 40,
    XtNheight, 40,
    NULL);
  if (mono) set_mono(glyph_wksp);
  add_pb_help(&xg->nhelpids.pb,
    xg->workspace, "Brushing");
  XtAddEventHandler(glyph_wksp, ExposureMask,
    FALSE, (XtEventHandler) glyph_expose, (XtPointer) xg);

/*
 * Next, the labels to indicate the brushing type: persistent, transient,
 * undo.
*/
  PANEL2 = XtVaCreateManagedWidget("Panel",
    formWidgetClass, PANEL0,
    XtNfromVert, glyph_wksp,
    NULL);
  if (mono) set_mono(PANEL2);

  /* Transient brushing is the default */
  PERST_CMD = CreateToggle(xg, "Persistent",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, ONE_OF_MANY,
    PANEL2, "Brush_Modes");
  XtVaSetValues(PERST_CMD, 
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainRight,
    NULL);
  TRANS_CMD = CreateToggle(xg, "Transient",
    True, (Widget) NULL, PERST_CMD, PERST_CMD, True, ONE_OF_MANY,
    PANEL2, "Brush_Modes");
  XtVaSetValues(TRANS_CMD, 
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainRight,
    NULL);
  UNDO_CMD = CreateToggle(xg, "Undo",
    True, (Widget) NULL, TRANS_CMD, PERST_CMD, False, ONE_OF_MANY,
    PANEL2, "Brush_Modes");
  XtVaSetValues(UNDO_CMD, 
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainRight,
    NULL);
  XtManageChildren(br_type, 3);

  XtAddCallback(PERST_CMD, XtNcallback,
    (XtCallbackProc) br_perst_cback, (XtPointer) xg);
  XtAddCallback(TRANS_CMD, XtNcallback,
    (XtCallbackProc) br_trans_cback, (XtPointer) xg);
  XtAddCallback(UNDO_CMD, XtNcallback,
    (XtCallbackProc) br_undo_cback, (XtPointer) xg);

  hide_cmd = CreateCommand(xg, "Hide or exclude ...",
    True, (Widget) NULL, (Widget) PANEL2,
    PANEL0, "HideOrExclude");
  XtManageChild(hide_cmd);
  XtAddCallback(hide_cmd, XtNcallback,
    (XtCallbackProc) open_exclusion_popup_cback, (XtPointer) xg);
  if (xg->is_scatmat)
    XtSetSensitive(hide_cmd, False);

  br_update = CreateCommand(xg, "Send Update",
    True, (Widget) NULL, (Widget) hide_cmd,
    PANEL0, "Br_SendUpdate");
  XtManageChild(br_update);
  XtAddCallback(br_update, XtNcallback,
    (XtCallbackProc) br_update_cback, (XtPointer) xg);

/*
 * Labels to indicate brushing commands: reset brush or points.
*/
  make_reset_menu(xg, PANEL0, br_update );

  /*
   * If a .nlinkable file is used, then deletion
   * is to be disabled.
  */
/*
  if (xg->nlinkable != xg->nrows)
    set_deletion(False);
*/
  /* */

  make_br_linkopt_menu(xg, PANEL0, (Widget) NULL, reset_menu_cmd);
  make_br_opt_menu(xg, PANEL0, (Widget) NULL, br_linkopt_menu_cmd);

}

void
update_linked_brushing(xgobidata *xg) {
  XtCallCallbacks(br_update, XtNcallback, (XtPointer) xg);
}

void
reset_br_types(xgobidata *xg)
{
  XtVaSetValues(PERST_CMD,
    XtNstate, xg->brush_mode == persistent,
    NULL);
  setToggleBitmap(PERST_CMD, xg->brush_mode == persistent);

  XtVaSetValues(TRANS_CMD,
    XtNstate, xg->brush_mode == transient,
    NULL);
  setToggleBitmap(TRANS_CMD, xg->brush_mode == transient);

  XtVaSetValues(UNDO_CMD,
    XtNstate, xg->brush_mode == undo,
    NULL);
  setToggleBitmap(UNDO_CMD, xg->brush_mode == undo);
}

void
set_brush_menu_cmd(Boolean lgl)
{
  XtVaSetValues(color_menu_cmd,
    XtNsensitive, (Boolean) lgl,
    NULL);
  XtVaSetValues(PERST_CMD,
    XtNsensitive, (Boolean) lgl,
    NULL);
  XtVaSetValues(UNDO_CMD,
    XtNsensitive, (Boolean) lgl,
    NULL);
}


void
init_brush_menus()
{
/*
 * Now add the mark beside the default values.
*/
  XtVaSetValues(color_menu_btn[0],
    XtNleftBitmap, menu_mark,
    NULL);
  XtVaSetValues(glyph_menu_btn[0],
    XtNleftBitmap, menu_mark,
    NULL);
}

void
map_brush(xgobidata *xg, Boolean brushon)
{
  if (brushon)
  {
    XtMapWidget( PANEL0 );
    XtMapWidget( xg->brush_mouse );
  }
  else
  {
    XtUnmapWidget( PANEL0 );
    XtUnmapWidget(xg->brush_mouse);
  }
}

#undef PANEL0
#undef PANEL1
#undef PANEL2
#undef PANEL3
