/* line_editor.c */
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
#include <math.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

static Widget line_edit_panel, le_cmd[5];

#define ADD le_cmd[0]
#define DELETE le_cmd[1]
#define SHOW_LINES le_cmd[2]
#define REMOVE_ALL le_cmd[3]
#define INCLUDE_IMP le_cmd[4]

XSegment *connecting_segs;
XSegment *arrowhead_segs;
Boolean is_le_adding, is_le_deleting;
static int *avec, *bvec;
static int le_start = -1, le_nearest_pt = -1, le_nearest_line = -1;

/* Draw lines to missing/imputed values or don't */
Boolean plot_imputed_values = True;


/* --------- Dynamic allocation section ----------- */
void
alloc_line_edit_arrays(xgobidata *xg)
/*
 * Dynamically allocate arrays.
*/
{
/*
 * It might be a good idea to use nrows_in_plot rather
 * than nrows for these.  It would require a lot of
 * Reallocing, but would enable XGobi to handle larger
 * matrices.
*/
  Cardinal nl = (Cardinal) xg->nlines;

  connecting_segs = (XSegment *)
    XtMalloc(nl * sizeof(XSegment));

/* for suggesting a directional arrow */
  arrowhead_segs = (XSegment *)
    XtMalloc(nl * sizeof(XSegment));

  xg->line_color_ids = (unsigned long *)
    XtMalloc(nl * sizeof(unsigned long));
  xg->line_color_now = (unsigned long *)
    XtMalloc(nl * sizeof(unsigned long));
  xg->line_color_prev = (unsigned long *)
    XtMalloc(nl * sizeof(unsigned long));
  xg->xed_by_new_brush = (unsigned short *)
    XtMalloc(nl * sizeof(unsigned short));
}

void
free_line_edit_arrays(xgobidata *xg)
/*
 * Dynamically free arrays.
*/
{
  XtFree((XtPointer) connecting_segs);
  XtFree((XtPointer) arrowhead_segs);

  XtFree((XtPointer) xg->line_color_ids);
  XtFree((XtPointer) xg->line_color_now);
  XtFree((XtPointer) xg->line_color_prev);
  XtFree((XtPointer) xg->xed_by_new_brush);
}

void
realloc_lines(xgobidata *xg)
{
  xg->connecting_lines = (connect_lines *)
    XtRealloc((XtPointer) xg->connecting_lines,
    (unsigned) xg->nlines * sizeof(connect_lines));

  connecting_segs = (XSegment *)
    XtRealloc((XtPointer) connecting_segs,
    (unsigned) xg->nlines * sizeof(XSegment));

  arrowhead_segs = (XSegment *)
    XtRealloc((XtPointer) arrowhead_segs,
    (unsigned) xg->nlines * sizeof(XSegment));

  xg->line_color_ids = (unsigned long *)
    XtRealloc((XtPointer) xg->line_color_ids,
    (unsigned) xg->nlines * sizeof(unsigned long));
  xg->line_color_now = (unsigned long *)
    XtRealloc((XtPointer) xg->line_color_now,
    (unsigned) xg->nlines * sizeof(unsigned long));
  xg->line_color_prev = (unsigned long *)
    XtRealloc((XtPointer) xg->line_color_prev,
    (unsigned) xg->nlines * sizeof(unsigned long));
  xg->xed_by_new_brush = (unsigned short *)
    XtRealloc((XtPointer) xg->xed_by_new_brush,
    (unsigned) xg->nlines * sizeof(unsigned short));
}

/* --------- End of dynamic allocation section ----------- */

void
create_default_lines(xgobidata *xg)
{
  int i;

  xg->nlines = xg->nrows - 1;
  xg->connecting_lines = (connect_lines *) XtRealloc(
    (XtPointer) xg->connecting_lines,
    (Cardinal) xg->nlines * sizeof(connect_lines));

  for (i=0; i<xg->nlines; i++)
  {
    xg->connecting_lines[i].a = i+1;
    xg->connecting_lines[i].b = i+2;
  }
}

void
init_line_edit_vars(xgobidata *xg)
{
  static Boolean firsttime = True;

  if (firsttime)
  {
    is_le_deleting = True;
    is_le_adding = False;

    firsttime = False;
  }
  xg->is_line_editing = False;
}

int
sort_by_abvecs(const void *arg1, const void *arg2)
{
  int val = 0;
  int *x1 = (int *) arg1;
  int *x2 = (int *) arg2;

  if ( (avec[*x1] < avec[*x2]) ||
     (avec[*x1] == avec[*x2] && bvec[*x1] < bvec[*x2]) )
  {
    val = -1;
  }
  else if ( (avec[*x1] > avec[*x2]) ||
        (avec[*x1] == avec[*x2] && bvec[*x1] > bvec[*x2]) )

  {
    val = 1;
  }

  return(val);
}

void
sort_connecting_lines(xgobidata *xg)
{
  int i;
  unsigned long *tvec;
  int *indx;
/*
 * sort_by_abvecs is used by qsort to put indx in the order of the
 * b's within the a's.
*/
  indx = (int *) XtMalloc((Cardinal) xg->nlines * sizeof(int));
  avec = (int *) XtMalloc((Cardinal) xg->nlines * sizeof(int));
  bvec = (int *) XtMalloc((Cardinal) xg->nlines * sizeof(int));
  tvec = (unsigned long *) XtMalloc((Cardinal) xg->nlines * sizeof(int));

  for (i=0; i<xg->nlines; i++)
  {
    avec[i] = xg->connecting_lines[i].a;
    bvec[i] = xg->connecting_lines[i].b;
    indx[i] = i;
  }

  qsort((char *) indx, xg->nlines, sizeof(int), sort_by_abvecs);

  for (i=0; i<xg->nlines; i++)
  {
    xg->connecting_lines[i].a = avec[indx[i]];
    xg->connecting_lines[i].b = bvec[indx[i]];
  }

  for (i=0; i<xg->nlines; i++)
    tvec[i] = xg->line_color_ids[i];
  for (i=0; i<xg->nlines; i++)
    xg->line_color_ids[i] = tvec[indx[i]];

  for (i=0; i<xg->nlines; i++)
    tvec[i] = xg->line_color_now[i];
  for (i=0; i<xg->nlines; i++)
    xg->line_color_now[i] = tvec[indx[i]];

  for (i=0; i<xg->nlines; i++)
    tvec[i] = xg->line_color_prev[i];
  for (i=0; i<xg->nlines; i++)
    xg->line_color_prev[i] = tvec[indx[i]];

  XtFree((XtPointer) avec);
  XtFree((XtPointer) bvec);
  XtFree((XtPointer) tvec);
  XtFree((XtPointer) indx);
}

Boolean
plotted_var_missing(int from, int to, xgobidata *xg)
{
  Boolean missing = False;

  if (xg->is_plotting1d) {
    if (xg->is_missing[from][xg->plot1d_vars.y] ||
        xg->is_missing[to][xg->plot1d_vars.y]) 
    {
      missing = True;
    }
  }
  else if (xg->is_xyplotting) {
    if (xg->is_missing[from][xg->xy_vars.x] ||
        xg->is_missing[to][xg->xy_vars.x] ||
        xg->is_missing[from][xg->xy_vars.y] ||
        xg->is_missing[to][xg->xy_vars.y])
    {
      missing = True;
    }
  }
  else if (xg->is_spinning) {
    if (xg->is_missing[from][xg->spin_vars.x] ||
        xg->is_missing[to][xg->spin_vars.x] ||
        xg->is_missing[from][xg->spin_vars.y] ||
        xg->is_missing[to][xg->spin_vars.y] ||
        xg->is_missing[from][xg->spin_vars.z] ||
        xg->is_missing[to][xg->spin_vars.z])
    {
      missing = True;
    }
  }
  else if (xg->is_touring) {
    int i;
    for (i=0; i<xg->numvars_t; i++) {
      if (xg->is_missing[from][xg->tour_vars[i]] ||
          xg->is_missing[to][xg->tour_vars[i]])
      {
        missing = True;
      }
    }
  }

  return missing;
}

int
find_nearest_line(icoords *cursor_pos, xgobidata *xg)
{
  int sqdist, near, j, lineid, xdist;
  int from, to;
  icoords a, b, distab, distac, d;
  float proj;
  Boolean doit;

  lineid = -1;
  if (xg->nlines) {
    xdist = sqdist = near = 1000 * 1000;
    for (j=0; j<xg->nlines; j++) {
      from = xg->connecting_lines[j].a - 1;
      to = xg->connecting_lines[j].b - 1;
      doit = True;

      if (!plot_imputed_values && plotted_var_missing(from, to, xg))
        doit = False;

      /* If either from or to is excluded or erased, move on */
      else if (xg->ncols == xg->ncols_used) {
        if (xg->clusv[(int) GROUPID(from)].excluded)
          doit = False;
        else if (xg->clusv[(int) GROUPID(to)].excluded)
          doit = False;
      }
      if (doit && (xg->erased[from] || xg->erased[to]))
        doit = False;

      if (doit) {
        a.x = xg->screen[from].x;
        a.y = xg->screen[from].y;
        b.x = xg->screen[to].x;
        b.y = xg->screen[to].y;

        distab.x = b.x - a.x;
        distab.y = b.y - a.y;
        distac.x = cursor_pos->x - a.x;
        distac.y = cursor_pos->y - a.y;

        /* vertical lines */
        if (distab.x == 0 && distab.y != 0) {
          sqdist = distac.x * distac.x;
          if (BETWEEN(a.y, b.y, cursor_pos->y))
            ;
          else {
            int yd = MIN(abs(distac.y), abs(cursor_pos->y - b.y));
            sqdist += (yd * yd);
          }

          if (sqdist <= near) {
            near = sqdist;
            lineid = j;
          }
        }

        /* horizontal lines */
        else if (distab.y == 0 && distab.x != 0) {
          sqdist = distac.y * distac.y ;
          if (sqdist <= near && (int) fabs((float) distac.x) < xdist) {
            near = sqdist;
            xdist = (int) fabs((float) distac.x) ;
            lineid = j;
          }
        }

        /* other lines */
        else if (distab.x != 0 && distab.y != 0) {
          proj = ((float) ((distac.x * distab.x) +
                           (distac.y * distab.y))) /
                 ((float) ((distab.x * distab.x) +
                           (distab.y * distab.y)));

          d.x = (int) (proj * (float) (b.x - a.x)) + a.x;
          d.y = (int) (proj * (float) (b.y - a.y)) + a.y;

          if (BETWEEN(a.x, b.x, d.x) && BETWEEN(a.y, b.y, d.y)) {
            sqdist = (cursor_pos->x - d.x) * (cursor_pos->x - d.x) +
                 (cursor_pos->y - d.y) * (cursor_pos->y - d.y);
          } else {
            sqdist = MIN(
             (cursor_pos->x - a.x) * (cursor_pos->x - a.x) +
             (cursor_pos->y - a.y) * (cursor_pos->y - a.y),
             (cursor_pos->x - b.x) * (cursor_pos->x - b.x) +
             (cursor_pos->y - b.y) * (cursor_pos->y - b.y) );
          }
          if (sqdist < near) {
            near = sqdist;
            lineid = j;
          }
        }
      }
    }
  }
  return(lineid);
}

void
line_edit_proc(xgobidata *xg)
{
  int root_x, root_y;
  unsigned int kb;
  Window root, child;
  icoords cpos;
  static int ocpos_x = 0, ocpos_y = 0;
  static int icount = 0;

/*
 * Get the current pointer position.
*/
  if (XQueryPointer(display, xg->plot_window, &root, &child,
            &root_x, &root_y, &cpos.x, &cpos.y, &kb))
  {
    /*
     * If the pointer is inside the plotting region ...
    */
    if ((0 < cpos.x && cpos.x < xg->max.x) &&
      (0 < cpos.y && cpos.y < xg->max.y))
    {
      icount = 1;
      /*
       * If the pointer has moved ...
      */
      if ((cpos.x != ocpos_x) || (cpos.y != ocpos_y))
      {
        ocpos_x = cpos.x;
        ocpos_y = cpos.y;

        if (is_le_adding)
          le_nearest_pt = find_nearest_point(&cpos, xg);

        else if (is_le_deleting &&
          (xg->connect_the_points||xg->plot_the_arrows))
        {
          le_nearest_line = find_nearest_line(&cpos, xg);
        }

        quickplot_once(xg);
      }
    }
    else
    {
      le_nearest_pt = -1;
      le_nearest_line = -1;
      if (icount)
      {
        /*
         * Plot once ... then there won't be an id in the plot
         * if the cursor is outside the plot window.
        */

        quickplot_once(xg);
        icount = 0;
      }
    }
  }
}

void
add_line(int a, int b, xgobidata *xg)
{
  int n = xg->nlines;

  xg->nlines++;

  realloc_lines(xg);
  xg->connecting_lines[n].a = a+1;
  xg->connecting_lines[n].b = b+1;

  xg->line_color_now[n]  = plotcolors.fg;
  xg->line_color_ids[n]  = plotcolors.fg;
  xg->line_color_prev[n] = plotcolors.fg;

  sort_connecting_lines(xg);
}


/* ARGSUSED */
XtEventHandler
add_line_button(Widget w, xgobidata *xg, XEvent *evnt, Boolean *dispatch)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;
  int root_x, root_y;
  unsigned int kb;
  Window root, child;
  icoords cpos;
  static int last_addeda = -1, last_addedb = -1;
  int i;

  switch (xbutton->type) {
    case ButtonPress:

      if (xbutton->button == 1) {
        if (le_nearest_pt != -1) {
          /*
           * Get the current pointer position.
          */
          if (XQueryPointer(display, xg->plot_window, &root, &child,
            &root_x, &root_y, &cpos.x, &cpos.y, &kb))
          {
            le_nearest_pt = find_nearest_point(&cpos, xg);
          }
        }
        le_start = le_nearest_pt;
      }
      else if (xbutton->button == 2 &&
           last_addeda > -1 && last_addedb > -1)
      /*
       * delete most recently added line, which is at
       * the end of the connecting_lines[] structure
      */
      {
        int link_to_remove;

        for (i=0; i<xg->nlines; i++) {
          if (xg->connecting_lines[i].a == last_addeda &&
            xg->connecting_lines[i].b == last_addedb)
          {
            link_to_remove = i;
            break;
          }
        }

        for (i=link_to_remove; i<(xg->nlines-1); i++)
        {
          xg->connecting_lines[i].a = xg->connecting_lines[i+1].a;
          xg->connecting_lines[i].b = xg->connecting_lines[i+1].b;

          xg->line_color_now[i]  = xg->line_color_now[i+1];
          xg->line_color_ids[i]  = xg->line_color_ids[i+1];
          xg->line_color_prev[i] = xg->line_color_prev[i+1];
        }

        xg->nlines--;
        realloc_lines(xg);
        last_addeda = last_addedb = -1;
      }

      break;

    case ButtonRelease:

      if (xbutton->button == 1) {
        if (le_start != -1 && le_nearest_pt != -1 &&
            (le_nearest_pt != le_start) )
        {
          if (le_start < le_nearest_pt)
            add_line(le_start, le_nearest_pt, xg);
          else
            add_line(le_nearest_pt, le_start, xg);

          last_addeda = xg->connecting_lines[xg->nlines-1].a;
          last_addedb = xg->connecting_lines[xg->nlines-1].b;
        }
        le_start = -1;
        le_nearest_pt = -1;
      }
      break;

  }
  plot_once(xg);
}

/* ARGSUSED */
static XtEventHandler
delete_line_button(Widget w, xgobidata *xg, XEvent *evnt, Boolean *dispatch)
{
  int root_x, root_y;
  unsigned int kb;
  Window root, child;
  icoords cpos;
  XButtonEvent *xbutton = (XButtonEvent *) evnt;
  int i;
  static int last_deleteda = -1, last_deletedb = -1;
  static unsigned long last_color_now, last_color_ids, last_color_prev;

  if (xbutton->type == ButtonPress)
  {
    if (xbutton->button == 1 && le_nearest_line > -1)
    {
      last_deleteda = xg->connecting_lines[le_nearest_line].a;
      last_deletedb = xg->connecting_lines[le_nearest_line].b;

      last_color_now  = xg->line_color_now[le_nearest_line];
      last_color_ids  = xg->line_color_ids[le_nearest_line];
      last_color_prev = xg->line_color_prev[le_nearest_line];

      for (i=le_nearest_line; i<(xg->nlines-1); i++)
      {
        xg->connecting_lines[i].a = xg->connecting_lines[i+1].a;
        xg->connecting_lines[i].b = xg->connecting_lines[i+1].b;

        xg->line_color_now[i]  = xg->line_color_now[i+1];
        xg->line_color_ids[i]  = xg->line_color_ids[i+1];
        xg->line_color_prev[i] = xg->line_color_prev[i+1];
      }
      xg->nlines--;
      realloc_lines(xg);
      /*
       * Get the current pointer position.
      */
      if (XQueryPointer(display, xg->plot_window, &root, &child,
        &root_x, &root_y, &cpos.x, &cpos.y, &kb))
      {
        le_nearest_line = find_nearest_line(&cpos, xg);
      }
    }
    else if (xbutton->button == 2 && last_deleteda > -1)
    {
      xg->nlines++;
      realloc_lines(xg);
      xg->connecting_lines[xg->nlines-1].a = last_deleteda;
      xg->connecting_lines[xg->nlines-1].b = last_deletedb;
      last_deleteda = last_deletedb = -1;

      xg->line_color_now[xg->nlines-1]  = last_color_now;
      xg->line_color_ids[xg->nlines-1]  = last_color_ids;
      xg->line_color_prev[xg->nlines-1] = last_color_prev;
    }
  }
  plot_once(xg);
}

void
map_line_edit(xgobidata *xg)
{
  if (xg->is_line_editing)
    XtMapWidget(line_edit_panel);
  else
    XtUnmapWidget(line_edit_panel);
}

/* ARGSUSED */
XtCallbackProc
add_lines_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 *  Initiate line adding mode.
*/
{
  if (!is_le_adding)
  {
    is_le_adding = True;
    le_start = -1;
    XtAddEventHandler(xg->workspace,
      ButtonPressMask | ButtonReleaseMask,
      FALSE, (XtEventHandler) add_line_button, (XtPointer) xg);
    XtMapWidget(xg->le_add_mouse);
  }
  else
  {
    XtRemoveEventHandler(xg->workspace,
      XtAllEvents,
      TRUE, (XtEventHandler) add_line_button, (XtPointer) xg);
    is_le_adding = False;
    le_start = -1;
    XtUnmapWidget(xg->le_add_mouse);
  }

  setToggleBitmap(w, is_le_adding);
}

/* ARGSUSED */
XtCallbackProc
delete_lines_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Initiate line deletion mode
*/
{
  if (!is_le_deleting)
  {
    is_le_deleting = True;
    XtAddEventHandler(xg->workspace,
      ButtonPressMask | ButtonReleaseMask,
      FALSE, (XtEventHandler) delete_line_button, (XtPointer) xg);
    XtMapWidget(xg->le_delete_mouse);
    le_nearest_line = -1;
  }
  else
  {
    XtRemoveEventHandler(xg->workspace,
      XtAllEvents, TRUE,
      (XtEventHandler) delete_line_button, (XtPointer) xg);
    is_le_deleting = False;
    le_start = -1;
    XtUnmapWidget(xg->le_delete_mouse);
  }
  setToggleBitmap(w, is_le_deleting);
}

void
set_showlines(Boolean lgl)
{
/*
 * This is needed because we're going to duplicate
 * this button on the line editing panel, and they
 * both need to show the same state, but we don't
 * want to call the callback twice.
*/
  XtVaSetValues(SHOW_LINES, XtNstate, lgl, NULL);
  setToggleBitmap(SHOW_LINES, lgl);
}

/* ARGSUSED */
XtCallbackProc
show_all_lines_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Show/don't show all the lines.  Make sure the duplicate
 * toggle in the options menu has the right state.
*/
{
  extern Widget display_menu_btn[];

  if (xg->connect_the_points || xg->plot_the_arrows)
    xg->connect_the_points = xg->plot_the_arrows = False;
  else
    xg->connect_the_points = True;

  /*
   * These three functions are in widgets.c, allowing us to
   * make sure the menu matches the values of these variables.
  */
  set_showlines_option(xg->connect_the_points, xg);
  set_showarrows_option(xg->plot_the_arrows, xg);

  /*
   * Set this to 1 in case there are line colors.
  */
  xg->got_new_paint = True;
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
remove_all_lines_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Remove all the lines -- this isn't just not showing them,
 * it's removing them altogether.
*/
{
  xg->nlines = 0;
  le_start = -1;

  plot_once(xg);
}

/* ARGSUSED */
static XtCallbackProc
plot_imputed_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  plot_imputed_values = !plot_imputed_values;
  setToggleBitmap(w, plot_imputed_values);

  plot_once(xg);
}

/* ------ Initialization section ------ */

void
make_line_editor(xgobidata *xg)
{
  /*
   * LineEditPanel
  */
  line_edit_panel = XtVaCreateManagedWidget("LineEditPanel",
    boxWidgetClass, xg->box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
    NULL);
  if (mono) set_mono(line_edit_panel);
  /*
   * Add Lines
  */
  ADD = CreateToggle(xg, "Add",
    True, (Widget) NULL, (Widget) NULL,
    (Widget) NULL, False, ONE_OF_MANY,
    line_edit_panel, "LE_AddLines");
  /*
   * Delete Lines
  */
  DELETE = CreateToggle(xg, "Delete",
    True, (Widget) NULL, (Widget) NULL,
    (Widget) ADD, True, ONE_OF_MANY,
    line_edit_panel, "LE_RmLines");

  /*
   * Show lines or don't; duplicate of button on Options panel.
  */
  SHOW_LINES = CreateToggle(xg, "Show lines",
    True, (Widget) NULL, (Widget) NULL,
    (Widget) NULL, True, ANY_OF_MANY,
    line_edit_panel, "LE_ShowLines");

  /*
   * Remove all connecting lines button.
  */
  REMOVE_ALL = CreateCommand(xg, "Remove all lines",
    True, (Widget) NULL, (Widget) NULL,
    line_edit_panel, "LE_RmAllLines");

  /*
   * Draw lines to missings/imputed values or don't:  set the
   * button to false, and disable it, if no missing values are
   * present.
  */
  INCLUDE_IMP = (Widget) CreateToggle(xg, "Include missings",
    xg->missing_values_present,
    (Widget) NULL, (Widget) NULL, (Widget) NULL,
    plot_imputed_values && xg->missing_values_present,
    ANY_OF_MANY, line_edit_panel, "CProfPlot");

  XtManageChildren(le_cmd, 5);

  XtAddCallback(ADD, XtNcallback,
    (XtCallbackProc) add_lines_cback, (XtPointer) xg);
  XtAddCallback(DELETE, XtNcallback,
    (XtCallbackProc) delete_lines_cback, (XtPointer) xg);
  XtAddCallback(SHOW_LINES, XtNcallback,
    (XtCallbackProc) show_all_lines_cback, (XtPointer) xg);
  XtAddCallback(REMOVE_ALL, XtNcallback,
    (XtCallbackProc) remove_all_lines_cback, (XtPointer) xg);
  XtAddCallback(INCLUDE_IMP, XtNcallback,
    (XtCallbackProc) plot_imputed_cback, (XtPointer) xg);
}


/* ------------------ End of initialization section  -------------- */
/* ------------------ Routines for the final drawing -------------- */

void
draw_heavy_line(int lineid, Drawable win, xgobidata *xg)
{
  int start = xg->connecting_lines[lineid].a-1;
  int end   = xg->connecting_lines[lineid].b-1;

  if (!mono)
    XSetForeground(display, copy_GC, xg->line_color_now[lineid]);
  XSetLineAttributes(display, copy_GC,
    2, LineSolid, CapButt, JoinMiter);

  XDrawLine(display, win, copy_GC,
    xg->screen[start].x, xg->screen[start].y,
    xg->screen[end].x, xg->screen[end].y);

  if (!mono)
    XSetForeground(display, copy_GC, plotcolors.fg);
  XSetLineAttributes(display, copy_GC,
    0, LineSolid, CapButt, JoinMiter);
}

static void
find_line_colors_used(xgobidata *xg, int *ncolors_used,
unsigned long *colors_used)
{
  Boolean new_color;
  int i, k;

  /*
   * Need this line here to initialize -- especially to cover
   * the odd case that you're using xgobi in mono mode on a
   * color display.
  */
  colors_used[0] = xg->line_color_now[0];
  /*
   * Loop once through line_color_now[], collecting the colors
   * currently in use into the line_colors_used[] vector.
  */
  if ( (xg->is_color_painting /*&& xg->is_line_painting*/) ||
     xg->got_new_paint)
  {
    *ncolors_used = 1;
    for (i=0; i<xg->nlines; i++)
    {
      new_color = True;
      for (k=0; k<*ncolors_used; k++)
      {
        if (colors_used[k] == xg->line_color_now[i])
        {
          new_color = False;
          break;
        }
      }
      if (new_color)
      {
        colors_used[*ncolors_used] = xg->line_color_now[i];
        (*ncolors_used)++;
      }
    }
  }
}

void
draw_connecting_lines(xgobidata *xg)
{
  int j, k;
  int from, to;
  int nlines_in_plot;
  unsigned long current_color;
  static unsigned long line_colors_used[NCOLORS+2];
  static int nline_colors_used = 1;
  XGCValues *gcv, gcv_inst;
  Boolean doit;

  XGetGCValues(display, copy_GC, GCCapStyle|GCJoinStyle, &gcv_inst);
  gcv = &gcv_inst;

  if (!mono) {
    find_line_colors_used(xg, &nline_colors_used, line_colors_used);

    /*
     * Now loop through line_colors_used[], plotting the glyphs of each
     * color in a group.
    */
    for (k=0; k<nline_colors_used; k++) {
      current_color = line_colors_used[k];
      nlines_in_plot = 0;

      for (j=0; j<xg->nlines; j++) {
        from = xg->connecting_lines[j].a - 1;
        to = xg->connecting_lines[j].b - 1;
        doit = True;

        /* If not plotting imputed values, and one is missing, skip it */
        if (!plot_imputed_values && plotted_var_missing(from, to, xg))
          doit = False;
        /* If either from or to is excluded, move on */
        else if (xg->ncols == xg->ncols_used) {
          if (xg->clusv[(int)GROUPID(from)].excluded)
            doit = False;
          else if (xg->clusv[(int)GROUPID(to)].excluded)
            doit = False;
        }
        /* If either from or to is erased, move on */
        if (doit && (xg->erased[from] || xg->erased[to]))
          doit = False;

        if (doit) {
          if (xg->line_color_now[j] == current_color) {
            connecting_segs[nlines_in_plot].x1 = xg->screen[from].x;
            connecting_segs[nlines_in_plot].y1 = xg->screen[from].y;
            connecting_segs[nlines_in_plot].x2 = xg->screen[to].x;
            connecting_segs[nlines_in_plot].y2 = xg->screen[to].y;

            if (xg->plot_the_arrows) {
              /*
               * Add thick piece of the lines to suggest a directional arrow
              */
              arrowhead_segs[nlines_in_plot].x1 =
                (int) (.2*xg->screen[from].x + .8*xg->screen[to].x);
			  arrowhead_segs[nlines_in_plot].y1 =
                (int) (.2*xg->screen[from].y + .8*xg->screen[to].y);
			  arrowhead_segs[nlines_in_plot].x2 =
                xg->screen[to].x;
              arrowhead_segs[nlines_in_plot].y2 =
                xg->screen[to].y;
            }
            nlines_in_plot++;
          }
        }
      }
      if (!mono)
        XSetForeground(display, copy_GC, current_color);
      XDrawSegments(display, xg->pixmap0, copy_GC,
        connecting_segs, nlines_in_plot);

      if (xg->plot_the_arrows) {

        XSetLineAttributes(display, copy_GC, 3, LineSolid,
          gcv->cap_style, gcv->join_style);
        XDrawSegments(display, xg->pixmap0, copy_GC,
          arrowhead_segs, nlines_in_plot);
        XSetLineAttributes(display, copy_GC, 0, LineSolid,
          gcv->cap_style, gcv->join_style);
      }
    }
  }
  else    /* if mono */
  {
    nlines_in_plot = 0;
    for (j=0; j<xg->nlines; j++)
    {
      from = xg->connecting_lines[j].a - 1;
      to = xg->connecting_lines[j].b - 1;

      /* If not plotting imputed values, and one is missing, skip it */
      if (!plot_imputed_values && plotted_var_missing(from, to, xg))
        ;

      else if (!xg->erased[from] && !xg->erased[to]) {
        connecting_segs[nlines_in_plot].x1 = xg->screen[from].x;
        connecting_segs[nlines_in_plot].y1 = xg->screen[from].y;
        connecting_segs[nlines_in_plot].x2 = xg->screen[to].x;
        connecting_segs[nlines_in_plot].y2 = xg->screen[to].y;

        if (xg->plot_the_arrows) {
          /*
           * Add thick piece of the lines to suggest a directional arrow
          */
          arrowhead_segs[nlines_in_plot].x1 =
            (int) (.2*xg->screen[from].x + .8*xg->screen[to].x);
          arrowhead_segs[nlines_in_plot].y1 =
            (int) (.2*xg->screen[from].y + .8*xg->screen[to].y);
          arrowhead_segs[nlines_in_plot].x2 =
            xg->screen[to].x;
          arrowhead_segs[nlines_in_plot].y2 =
            xg->screen[to].y;
        }
        nlines_in_plot++;
      }
    }
    XDrawSegments(display, xg->pixmap0, copy_GC,
      connecting_segs, nlines_in_plot);
    if (xg->plot_the_arrows) {
      XSetLineAttributes(display, copy_GC, 3, LineSolid,
        gcv->cap_style, gcv->join_style);
      XDrawSegments(display, xg->pixmap0, copy_GC,
        arrowhead_segs, nlines_in_plot);
      XSetLineAttributes(display, copy_GC, 0, LineSolid,
        gcv->cap_style, gcv->join_style);
    }
  }
}

void
draw_diamond_around_point(int id, Drawable win, xgobidata *xg)
{
  XPoint diamond[5];

  diamond[0].x = diamond[4].x = xg->screen[id].x - 3;
  diamond[0].y = diamond[4].y = xg->screen[id].y;

  diamond[1].x = xg->screen[id].x;
  diamond[1].y = xg->screen[id].y - 3;

  diamond[2].x = xg->screen[id].x + 3;
  diamond[2].y = xg->screen[id].y;

  diamond[3].x = xg->screen[id].x;
  diamond[3].y = xg->screen[id].y + 3;

  if (!mono)  XSetForeground(display, copy_GC, plotcolors.fg);
  XDrawLines(display, win, copy_GC,
    diamond, 5, CoordModeOrigin);
}

void
draw_dotted_line(int start, int end, Drawable win, xgobidata *xg)
{
  XSetLineAttributes(display, copy_GC,
    0, LineOnOffDash, CapButt, JoinMiter);

  XDrawLine(display, win, copy_GC,
    xg->screen[start].x, xg->screen[start].y,
    xg->screen[end].x, xg->screen[end].y);

  XSetLineAttributes(display, copy_GC,
    0, LineSolid, CapButt, JoinMiter);
}

void
draw_editing_lines(xgobidata *xg)
{
  if (is_le_adding)
  {
    if (le_start != -1)
    {
      draw_diamond_around_point(le_start, xg->plot_window, xg);
      if (le_nearest_pt != -1 && is_le_adding)
        draw_dotted_line(le_start, le_nearest_pt, xg->plot_window, xg);
    }
    else
    {
      if (le_nearest_pt > -1)
        draw_diamond_around_point(le_nearest_pt, xg->plot_window, xg);
    }
  }
  else if (is_le_deleting && le_nearest_line > -1 &&
    (xg->connect_the_points||xg->plot_the_arrows) )
  {
    draw_heavy_line(le_nearest_line, xg->plot_window, xg);
  }
}

void
line_editor_on(xgobidata *xg)
{
/*
 *  If this mode is currently selected, turn it off.
*/
  if (xg->prev_plot_mode == LINEEDIT_MODE && xg->plot_mode != LINEEDIT_MODE)
  {
    if (is_le_adding)
    {
      XtRemoveEventHandler(xg->workspace,
        XtAllEvents, TRUE,
        (XtEventHandler) add_line_button, (XtPointer) xg);
      XtUnmapWidget(xg->le_add_mouse);
    }
    else if (is_le_deleting)
    {
      XtRemoveEventHandler(xg->workspace,
        XtAllEvents, TRUE,
        (XtEventHandler) delete_line_button, (XtPointer) xg);
      XtUnmapWidget(xg->le_delete_mouse);
    }
    xg->is_line_editing = False;
    map_line_edit(xg);

  }
  /* Else turn it on */
  else if (xg->plot_mode == LINEEDIT_MODE &&
           xg->prev_plot_mode != LINEEDIT_MODE)
  {
    le_nearest_pt = -1;
    xg->is_line_editing = True;
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
    if (is_le_adding)
    {
      XtAddEventHandler(xg->workspace,
        ButtonPressMask | ButtonReleaseMask,
        FALSE, (XtEventHandler) add_line_button, (XtPointer) xg);
      (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
      XtMapWidget(xg->le_add_mouse);
    }
    else if (is_le_deleting)
    {
      XtAddEventHandler(xg->workspace,
        ButtonPressMask | ButtonReleaseMask,
        FALSE, (XtEventHandler) delete_line_button, (XtPointer) xg);
      (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
      XtMapWidget(xg->le_delete_mouse);
      le_nearest_line = -1;
    }
    map_line_edit(xg);
/*
 * Since it makes no sense to use the line editor if the
 * lines aren't drawn, draw them.
*/
    if (!xg->connect_the_points && !xg->plot_the_arrows)
      turn_on_showlines_option(xg) ;

    plot_once(xg);
  }

}

/* line groups section */

/*
 * This needs a lot of code to maintain the lgroups structures
 * as lines are added and deleted.  If I make it exactly the
 * same size as rgroups, though, then I don't have to reallocate
 * and reorder the vector of lgroups structures, I just have to
 * adjust the els and nels members.  id won't change either; it's
 * fixed when the file is read.  (Lines can be added, but not points)
*/

void
free_lgroups(xgobidata *xg) {
  int i, j;
  for (i=0; i<xg->nlgroups; i++) {
    for (j=0; j<xg->lgroups[i].nels; j++) {
      XtFree((XtPointer) xg->lgroups[i].els);
    }
  }
  XtFree((XtPointer) xg->lgroups);
  XtFree((XtPointer) xg->lgroup_ids);
}

void
set_lgroups(Boolean init, xgobidata *xg) {
  Boolean found_lg;
  int i, k, gp;
  long *nels;

  if (!init)
   if (xg->nlgroups > 0)
     free_lgroups(xg);

  xg->nlgroups = 0;

  if (xg->nrgroups > 0) {  /* if there are no rgroups, there are no lgroups */
    nels = (long *) XtMalloc(xg->nlines * sizeof(long));
    xg->lgroup_ids = (long *) XtMalloc(xg->nlines * sizeof(long));
    for (i=0; i<xg->nlines; i++) {
      nels[i] = MAX(10, xg->nlines/10);
      xg->lgroup_ids[i] = -1;  /* each line initially belongs to no group */
    }

    /* Let there be exactly as many nlgroups as nrgroups */
    xg->nlgroups = xg->nrgroups;
    xg->lgroups = (rg_struct *) XtMalloc(xg->nlgroups * sizeof(rg_struct));
    for (i=0; i<xg->nlgroups; i++) {
      xg->lgroups[i].els = (long *)
        XtMalloc((unsigned int) nels[i] * sizeof(long));
      xg->lgroups[i].nels = 0;
      xg->lgroups[i].id = xg->rgroups[i].id;
      xg->lgroups[i].excluded = False;
    }

    /*
     * On this sweep, populate lgroup_ids.
    */
    for (i=0; i<xg->nlines; i++) {
      /*
       * If the two endpoints of the line are both in the same
       * row group, the lines should also be in the same group.
       * Otherwise, lgroup_ids will be -1.
      */
      if ((gp = xg->rgroup_ids[ xg->connecting_lines[i].a - 1 ]) == 
          xg->rgroup_ids[ xg->connecting_lines[i].b - 1 ])
      {
        xg->lgroup_ids[i] = gp;
      }
    }

    /*
     * On this sweep, find out how many groups there are and how
     * many elements are in each group
    */
    for (i=0; i<xg->nlines; i++) {
      found_lg = False;

      if (xg->lgroup_ids[i] != -1) {  /* this line is not in a group */

        for (k=0; k<xg->nlgroups; k++) {

          /* if we've found this id before ... */
          if (xg->lgroups[k].id == xg->lgroups[ xg->lgroup_ids[i] ].id) {

            /* Reallocate els[k] if necessary */
            if (xg->lgroups[k].nels == nels[k]) {
              nels[k] *= 2;
              xg->lgroups[k].els = (long *)
                XtRealloc((XtPointer) xg->lgroups[k].els,
                  (unsigned) (nels[k] * sizeof(long)));
            }

            /* Add the element, increment the element counter */
            xg->lgroups[k].els[ xg->lgroups[k].nels ] = i;
            xg->lgroups[k].nels++;

            found_lg = True;
            break;  /* I think; not tested */
          }
        }

        /* If it's a new group id, add it */
        if (!found_lg) {
          xg->lgroups[xg->nlgroups].id = xg->lgroup_ids[i];
          xg->lgroups[xg->nlgroups].nels = 1;
          xg->lgroups[xg->nlgroups].els[0] = i;
          xg->lgroup_ids[i] = xg->nlgroups;
          xg->nlgroups++;
        }
      }
    }

/*
  printf("number of line groups: %d\n", xg->nlgroups);
  for (k=0; k<xg->nlgroups; k++) {
    printf("id: %d, nelements: %d\n", xg->lgroups[k].id, xg->lgroups[k].nels);
    for (i=0; i<xg->lgroups[k].nels; i++)
      printf(" %d ", xg->lgroups[k].els[i]);
    printf("\n");
  }
*/

/*
  printf("lgroup pointers\n");
  for (i=0; i<xg->nlines; i++)
    printf(" %d ", xg->lgroup_ids[i]);
  printf("\n");
*/

    /* Now reallocate the arrays within each rgroups structure */
    for (k=0; k<xg->nlgroups; k++)
      xg->lgroups[k].els = (long *)
        XtRealloc((XtPointer) xg->lgroups[k].els,
          /* can I recalloc something to be 0?  could set MIN to 1 */
          (unsigned) (xg->lgroups[k].nels * sizeof(long)));

    XtFree((XtPointer) nels);
  }
}
