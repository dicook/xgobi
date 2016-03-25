/* brush.c */
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
#include <math.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

int button1down, button2down ;

static brush_coords brush_pos ;
/* static corner (x1, y1); corner where the cursor goes (x2,y2) */

static Boolean asyncchanged = False;

/* --------- Dynamic allocation section ----------- */

void
alloc_brush_arrays(xgobidata *xg)
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
  Cardinal nr = (Cardinal) xg->nrows;
  register unsigned int i, j;
  int nhbins = NHBINS;
  int nvbins = NVBINS;
  unsigned int nsend;

  xg->excluded = (unsigned short *) XtCalloc(nr, sizeof(unsigned short));

  xg->under_new_brush = (unsigned short *) XtMalloc(
    nr * sizeof(unsigned short));

  xg->glyph_ids = (glyphv *) XtMalloc(nr * sizeof(glyphv));
  xg->glyph_now = (glyphv *) XtMalloc(nr * sizeof(glyphv));
  xg->glyph_prev = (glyphv *) XtMalloc(nr * sizeof(glyphv));

  xg->color_ids = (unsigned long *) XtMalloc(
        nr * sizeof(unsigned long));
  xg->color_now = (unsigned long *) XtMalloc(
        nr * sizeof(unsigned long));
  xg->color_prev = (unsigned long *) XtMalloc(
        nr * sizeof(unsigned long));

  /* binning the plot window */
  xg->binarray = (Cardinal ***) XtMalloc(
    (Cardinal) nvbins * sizeof(Cardinal **) );
  for (i=0; i<nvbins; i++)
  {
    xg->binarray[i] = (Cardinal **) XtMalloc(
      (Cardinal) nhbins * sizeof(Cardinal *) );
    for (j=0; j<nhbins; j++)
      xg->binarray[i][j] = (Cardinal *) XtMalloc(sizeof(Cardinal));
  }

  xg->bincounts = (Cardinal **) XtMalloc(
    (unsigned int) nvbins * sizeof(Cardinal *) );
  for (i=0; i<nvbins; i++)
    xg->bincounts[i] = (Cardinal *) XtMalloc(
      (Cardinal) nhbins * sizeof(Cardinal) );

/*
 * nsend is the max of the length needed for linked for brushing
 * identification.  It's used by many linked modes.
*/
  nsend = 9 + 2* ((xg->nrgroups) ? xg->nrgroups : xg->nlinkable);
  xg->senddata = (unsigned long *) XtMalloc(
        (unsigned int) nsend * sizeof(unsigned long));
  for (i=0; i<nsend; i++) xg->senddata[i] = (unsigned long) 0;
}

void
free_brush_arrays(xgobidata *xg)
/*
 * Dynamically free arrays.
*/
{
  int j,k;

  XtFree((XtPointer) xg->under_new_brush);

  XtFree((XtPointer) xg->glyph_ids);
  XtFree((XtPointer) xg->glyph_now);
  XtFree((XtPointer) xg->glyph_prev);

  XtFree((XtPointer) xg->color_ids);
  XtFree((XtPointer) xg->color_now);
  XtFree((XtPointer) xg->color_prev);

  for (k=0; k<xg->nvbins; k++)
  {
    for (j=0; j<xg->nhbins; j++)
      XtFree((XtPointer) xg->binarray[k][j]);
    XtFree((XtPointer) xg->binarray[k]);
  }
  XtFree((XtPointer) xg->binarray);

  for (k=0; k<xg->nvbins; k++)
    XtFree((XtPointer) xg->bincounts[k]);
  XtFree((XtPointer) xg->bincounts);

  XtFree((XtPointer) xg->senddata);
}

void
init_brush_vars(xgobidata *xg)
{
  static Boolean firsttime = True;
  int i;
  glyphv glyph;
  XColor exact;
  Colormap cmap = DefaultColormap(display, DefaultScreen(display));

  if (firsttime)
  {
    xg->is_brushing = False;
    xg->brush_on = True;
    xg->is_point_painting = True;
    xg->is_line_painting = False;
    xg->brush_mode = transient;
    xg->is_glyph_painting = xg->is_color_painting = False;

    find_glyph_type_and_size(appdata.defaultGlyph, &glyph);
    xg->glyph_0.type = xg->glyph_id.type = glyph.type;
    xg->glyph_0.size = xg->glyph_id.size = glyph.size;

    if (XParseColor(display, cmap, appdata.defaultColor, &exact)) {
      if (XAllocColor(display, cmap, &exact)) {
        xg->color_0 = exact.pixel;
      }
    } else {
      (void) fprintf(stderr, "finding %s failed\n", appdata.defaultColor);
       xg->color_0 = appdata.fg;
    }

    /*
     * Used in binning the plot window
    */
    xg->nhbins = NHBINS;
    xg->nvbins = NVBINS;
    /*
     * These are initialized so that the first merge_brushbins()
     * call will behave reasonably.
    */
    xg->bin0.x = xg->bin1.x = NHBINS;
    xg->bin0.y = xg->bin1.y = NVBINS;

    /*
     * This is necessary when reshape_brush is False.
    */
    brush_pos.x1 = brush_pos.y1 = 20;
    brush_pos.x2 = brush_pos.y2 = 40;

    firsttime = False;
  }
  xg->delete_erased_pts = False;

  /*
   * Initialize rows_in_plot to be all the data : note this, dfs; work
  */
  for (i=0; i<xg->nrows_in_plot; i++)
    xg->rows_in_plot[i] = i;
}

static int
point_in_which_bin(xgobidata *xg, int x, int y, int *ih, int *iv)
{
  int inwindow = 1;

  *ih = (int) ((float) xg->nhbins * (float) x / (xg->max.x+1.0));
  *iv = (int) ((float) xg->nvbins * (float) y / (xg->max.y+1.0));

  if (*ih < 0 || *ih > xg->nhbins - 1 || *iv < 0 || *iv > xg->nvbins - 1)
    inwindow = 0;

  return(inwindow);
}

Boolean
brush_once(xgobidata *xg, Boolean force)
{
/*
 * Determine which bins the brush is currently sitting in.
 * bin0 is the bin which contains of the upper left corner of the
 * brush; bin1 is the one containing of the lower right corner.
*/
  int ulx = MIN(brush_pos.x1, brush_pos.x2);
  int uly = MIN(brush_pos.y1, brush_pos.y2);
  int lrx = MAX(brush_pos.x1, brush_pos.x2);
  int lry = MAX(brush_pos.y1, brush_pos.y2);
  Boolean changed = False;

  if ( !point_in_which_bin(xg, ulx, uly, &xg->bin0.x, &xg->bin0.y) ) {
    xg->bin0.x = MAX(xg->bin0.x, 0);
    xg->bin0.x = MIN(xg->bin0.x, xg->nhbins - 1);
    xg->bin0.y = MAX(xg->bin0.y, 0);
    xg->bin0.y = MIN(xg->bin0.y, xg->nhbins - 1);
  }
  if ( !point_in_which_bin(xg, lrx, lry, &xg->bin1.x, &xg->bin1.y) ) {
    xg->bin1.x = MAX(xg->bin1.x, 0);
    xg->bin1.x = MIN(xg->bin1.x, xg->nhbins - 1);
    xg->bin1.y = MAX(xg->bin1.y, 0);
    xg->bin1.y = MIN(xg->bin1.y, xg->nhbins - 1);
  }

/*
 * Now paint.
*/
  if (xg->is_point_painting) {
    changed = active_paint_points(xg);
    if ((changed || force) && xg->sync_brush) {
      if (xg->link_glyph_brushing ||
          xg->link_color_brushing ||
          xg->link_erase_brushing)
      {
        announce_brush_data(xg);

#if defined RPC_USED || defined DCE_RPC_USED
        xfer_brushinfo(xg);
#endif
      }
    }
  }

  if (xg->is_line_painting) {
    active_paint_lines(xg);
    if (xg->sync_brush) {
      if (xg->link_lines_to_lines || xg->link_points_to_lines)
        announce_line_brush_data(xg);
    }
  }

/*
 * Now here's a troublesome one from the point of
 * view of modularity:
*/
  if (changed && xg->sync_brush)
    if (xg->is_cprof_plotting && xg->link_cprof_plotting)
      cprof_plot_once(xg);

/*
 * To accumulative the knowledge of whether any points
 * have changed or not ...
*/
  asyncchanged = asyncchanged || changed;
  return(changed);
}

void
reinit_transient_brushing(xgobidata *xg)
{
/*
 * If a new variable is selected or a variable is transformed
 * during transient brushing,
 * restore all points to the permanent value, and then
 * re-execute brush_once() to brush the points that are
 * now underneath the brush.  For now, don't make the
 * same change for persistent or undo brushing.
*/
  int j, k;

  for (j=0; j<xg->nrows_in_plot; j++)
  {
    k = xg->rows_in_plot[j];
    xg->color_now[k] = xg->color_ids[k] ;
    xg->glyph_now[k].type = xg->glyph_ids[k].type;
    xg->glyph_now[k].size = xg->glyph_ids[k].size;
  }
  copy_brushinfo_to_senddata(xg);
  (void) brush_once(xg, False);
}

/* ARGSUSED */
XtEventHandler
brush_motion(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  XMotionEvent *xmotion = (XMotionEvent *) evnt;
  Boolean changed = False;

/*
  XEvent ahead;
  while (XEventsQueued(display, QueuedAlready) > 0) {
    XPeekEvent(display, &ahead);
    if (ahead.type == MotionNotify) {
      xmotion = (XMotionEvent *) &ahead;
      XtAppNextEvent(app_con, &ahead);
    }
    else if (ahead.type == PropertyNotify || ahead.type == NoExpose)
      XtAppNextEvent(app_con, &ahead);
    else
      break;
  }
*/

  if (xg->is_point_painting || xg->is_line_painting) {
    if (button1down) {
      int xdist = brush_pos.x2 - brush_pos.x1 ;
      int ydist = brush_pos.y2 - brush_pos.y1 ;
      /*
       * If this scheme works, then what's happening is that (x2,y2)
       * is the corner that's moving.
      */
      brush_pos.x1 = xmotion->x - xdist ;
      brush_pos.x2 = xmotion->x ;
      brush_pos.y1 = xmotion->y - ydist ;
      brush_pos.y2 = xmotion->y ;
    }

    if (button2down) {
      brush_pos.x2 = xmotion->x ;
      brush_pos.y2 = xmotion->y ;
    }

    if (xg->brush_on) {
      changed = brush_once(xg, False);

      /* must redraw everything if connected lines are showing */
      if (xg->is_line_painting || xg->connect_the_points || xg->nrgroups > 0)
      {
        plot_once(xg);
      }
      else {
        if (changed)
          plot_bins(xg);
        else quickplot_once(xg);
      }
    }
    else {
      quickplot_once(xg);
    }
  }
}

/* ARGSUSED */
void
init_brush_size(xgobidata *xg)
{
  brush_pos.x1 = brush_pos.y1 = 20;
  brush_pos.x2 = brush_pos.y2 = 40;
}

/* ARGSUSED */
XtEventHandler
brush_button(Widget w, xgobidata *xg, XEvent *evnt)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;
  Boolean changed = False;

  if (xg->is_point_painting || xg->is_line_painting) {
    if (xbutton->button == 1 || xbutton->button == 2) {
      if (xbutton->type == ButtonPress) {
        /*
         * This covers a funny case:  two windows, both in
         * brushing.  Brush persistently in one for a while,
         * now brush transiently in the second.  Without this
         * line, the transient brushing isn't sending the
         * correct data.
        */
        copy_brushinfo_to_senddata(xg);

        /*
         * If jump_brush is True, redraw the brush where the
         * pointer is, with the lower right corner of the brush
         * at the pointer.
        */
        if (xg->jump_brush) {
          int xdist = brush_pos.x2 - brush_pos.x1 ;
          int ydist = brush_pos.y2 - brush_pos.y1 ;
          /*
           * If this scheme works, then what's happening is that (x2,y2)
           * is the corner that's moving.
          */
          brush_pos.x1 = xbutton->x - xdist ;
          brush_pos.x2 = xbutton->x ;
          brush_pos.y1 = xbutton->y - ydist ;
          brush_pos.y2 = xbutton->y ;
        }
        /*
         * If jump_brush is False, move the pointer to the current
         * lower right corner of the brush.
        */
        else {
          XWarpPointer(display, None, xg->plot_window, 0,0,0,0,
            brush_pos.x2, brush_pos.y2 );
        }

        if (xbutton->button == 1 && xbutton->state != 8) {
          button1down = 1;
        }
        else if (xbutton->button == 2 ||
                (xbutton->button == 1 && xbutton->state != 8))
        {
          button2down = 1;
        }

        if (xg->brush_on) {
          changed = brush_once(xg, True);

          if (xg->is_point_painting) {
            if (xg->link_glyph_brushing ||
                xg->link_color_brushing ||
                xg->link_erase_brushing)
            {
              XtOwnSelection( (Widget) xg->workspace,
                (Atom) XG_NEWPAINT,
                (Time) CurrentTime,
                (XtConvertSelectionProc) pack_brush_data,
                (XtLoseSelectionProc) pack_brush_lose ,
                (XtSelectionDoneProc) pack_brush_done );
            }
          }

          /* If line brushing, update linked lines */
          if (xg->is_line_painting &&
             (xg->link_lines_to_lines || xg->link_points_to_lines))
          {
            XtOwnSelection( (Widget) xg->workspace,
              (Atom) XG_NEWLINEPAINT,
              (Time) CurrentTime, /* doesn't work with XtLastTimeStamp...? */
              (XtConvertSelectionProc) pack_line_brush_data,
              (XtLoseSelectionProc) pack_line_brush_lose ,
              (XtSelectionDoneProc) pack_line_brush_done );
          }
        }

        if (xg->jump_brush || xg->is_line_painting || changed) {
          plot_once(xg);
        }
        else {  /* whether changed or not? */
          plot_bins(xg);
        }
      }
      else if (xbutton->type == ButtonRelease)
      {
/*
        int x1 = MIN(brush_pos.x1, brush_pos.x2);
        int y1 = MIN(brush_pos.y1, brush_pos.y2);
        int x2 = MAX(brush_pos.x1, brush_pos.x2);
        int y2 = MAX(brush_pos.y1, brush_pos.y2);
*/

        if (xbutton->button == 1)
          button1down = 0;
        else if (xbutton->button == 2)
          button2down = 0;
 
        /*
         * We're redrawing everything on buttonrelease; we have no
         * way of knowing at this point whether things changed or
         * not, since that information is not accumulated.
        */
        if (xg->brush_on && button1down == 0 && button2down == 0) {

          if (xg->jump_brush || xg->is_line_painting) {
            plot_once(xg);

            /*
             * Send brushing data:  this is needed if sync_brush is
             * false, but it doesn't hurt to do it in all cases.
             * ... but let's try only doing it for async brushing ...
            */
            if (!xg->sync_brush && asyncchanged) {
              if (xg->link_glyph_brushing ||
                  xg->link_color_brushing ||
                  xg->link_erase_brushing)
              {
                announce_brush_data(xg);

#if defined RPC_USED || defined DCE_RPC_USED
                xfer_brushinfo(xg);
#endif
              }
            }

            if (xg->link_lines_to_lines || xg->link_points_to_lines)
              announce_line_brush_data(xg);

            if (!xg->sync_brush && asyncchanged &&
                xg->is_cprof_plotting && xg->link_cprof_plotting)
            {
              cprof_plot_once(xg);
            }
          }
        }
        asyncchanged = False;
      }
    }
  }
}

void
draw_brush(xgobidata *xg)
{
/*
 * Use brush_pos to draw the brush.
*/
  int x1 = MIN( brush_pos.x1, brush_pos.x2 );
  int x2 = MAX( brush_pos.x1, brush_pos.x2 );
  int y1 = MIN( brush_pos.y1, brush_pos.y2 );
  int y2 = MAX( brush_pos.y1, brush_pos.y2 );

  if (!mono)
  {
    if (xg->is_color_painting && xg->color_id != plotcolors.bg)
      XSetForeground(display, copy_GC, xg->color_id);
    else
      XSetForeground(display, copy_GC, plotcolors.fg);
  }

  if (xg->is_point_painting)
  {
    XDrawRectangle(display, xg->plot_window, copy_GC,
      x1, y1, (unsigned) abs(x2-x1), (unsigned) abs(y2-y1));
    /* Mark the corner to which the cursor will be attached */
    XDrawRectangle(display, xg->plot_window, copy_GC,
      brush_pos.x2-1, brush_pos.y2-1, 2, 2);

    /*
     * highlight brush
    */
    if (xg->brush_on) {
      XDrawRectangle(display, xg->plot_window, copy_GC,
        x1-1, y1-1, (unsigned) abs(x2-x1+2), (unsigned) abs(y2-y1+2));

      /* Mark the corner to which the cursor will be attached */
      XDrawRectangle(display, xg->plot_window, copy_GC,
        brush_pos.x2-2, brush_pos.y2-2, 4, 4);
    }
  }

  if (xg->is_line_painting)
  {
    XDrawLine(display, xg->plot_window, copy_GC,
      x1 + (x2 - x1)/2, y1, x1 + (x2 - x1)/2, y2 );
    XDrawLine(display, xg->plot_window, copy_GC,
      x1, y1 + (y2 - y1)/2, x2, y1 + (y2 - y1)/2 );

    if (xg->brush_on)
    {
      XDrawLine(display, xg->plot_window, copy_GC,
        x1 + (x2 - x1)/2 + 1, y1, x1 + (x2 - x1)/2 + 1, y2 );
      XDrawLine(display, xg->plot_window, copy_GC,
        x1, y1 + (y2 - y1)/2 + 1, x2, y1 + (y2 - y1)/2 + 1 );
    }
  }
}

int
under_brush(xgobidata *xg, int k)
/*
 * Determine if point is under the brush.
*/
{
  int pt;
  int x1 = MIN( brush_pos.x1, brush_pos.x2 );
  int x2 = MAX( brush_pos.x1, brush_pos.x2 );
  int y1 = MIN( brush_pos.y1, brush_pos.y2 );
  int y2 = MAX( brush_pos.y1, brush_pos.y2 );

  pt = (xg->screen[k].x <= x2 && xg->screen[k].y <= y2 &&
       xg->screen[k].x >= x1 && xg->screen[k].y >= y1) ? 1 : 0;
  return(pt);
}


int
xed_by_brush(xgobidata *xg, int pta, int ptb)
/*
 * Determine if line intersects brush.
*/
{
  int denom;
  float m;              /* slope of the line (pta, ptb) */
  int x, y;             /* coordinates of the intersection point */
  int intersection = 0;

/* coordinates of the points, pta and ptb
 *  xg->screen[pta].x, xg->screen[pta].y
 *  xg->screen[ptb].x, xg->screen[ptb].y
*/

  m = ((denom = xg->screen[ptb].x - xg->screen[pta].x) != 0) ?
    (float) (xg->screen[ptb].y - xg->screen[pta].y) / (float) denom :
    0;

/*
 * First check for intersection with the vertical line of the
 * crosshair brush.
*/

  x = brush_pos.x1 + (brush_pos.x2 - brush_pos.x1)/2;
  if (BETWEEN(xg->screen[pta].x, xg->screen[ptb].x, x)) {
    if (BETWEEN(brush_pos.y1, brush_pos.y2,
     (int) ( xg->screen[pta].y + m * (float) (x - xg->screen[pta].x) )))
    {
      intersection = 1;
    }
  }

/*
 * If nothing's turned up yet, check for intersection with the
 * horizontal line of the crosshair brush.
*/
  if (!intersection) {

    y = brush_pos.y1 + (brush_pos.y2 - brush_pos.y1)/2;
    if (BETWEEN(xg->screen[pta].y, xg->screen[ptb].y, y)) {
      if (m != 0) {
        if (BETWEEN(brush_pos.x1, brush_pos.x2,
          (int) (xg->screen[pta].x + (float) (y - xg->screen[pta].y) / m )))
        {
          intersection = 1;
        }
      }
    }
  }

  /* What about vertical and horizontal lines? */
  if (!intersection && (xg->screen[pta].x == xg->screen[ptb].x))
    if (BETWEEN(brush_pos.x1, brush_pos.x2, xg->screen[pta].x))
      intersection = 1;
  if (!intersection && (xg->screen[pta].y == xg->screen[ptb].y))
    if (BETWEEN(brush_pos.y1, brush_pos.y2, xg->screen[pta].y))
      intersection = 1;

  return(intersection);
}

void
assign_points_to_bins(xgobidata *xg)
{
  int i, k, ih, iv;

  /*
   * Reset bin counts to zero.
  */
  for (ih=0; ih<xg->nhbins; ih++)
    for (iv=0; iv<xg->nvbins; iv++)
      xg->bincounts[ih][iv] = 0;

  for (k=0; k<xg->nrows_in_plot; k++)
  {
    i = xg->rows_in_plot[k];

/*
 * One option for ignoring unlinkable points.  DFS

    if (i >= xg->nlinkable)
      break;
*/

    if (xg->screen[i].x >=0 && xg->screen[i].x <= xg->max.x &&
        xg->screen[i].y >=0 && xg->screen[i].y <= xg->max.y)
    {
      if (point_in_which_bin(xg, xg->screen[i].x, xg->screen[i].y, &ih, &iv))
      {
        xg->binarray[ih][iv] = (Cardinal *)
          XtRealloc((XtPointer) xg->binarray[ih][iv],
            (Cardinal) (xg->bincounts[ih][iv]+1) * sizeof(Cardinal) );
        /*
         * Binarray contains the
         * index of rows_in_plot[] rather than the contents, so
         * here the assignment is k rather than i
        */
        xg->binarray[ih][iv][xg->bincounts[ih][iv]] = (Cardinal) k;
        (xg->bincounts[ih][iv])++;
      }
    }
  }
}

void
find_extended_brush_corners(icoords *bin0, icoords *bin1, xgobidata *xg)
{
  static brush_coords obrush;
  static int initd = 0;
  int x1 = MIN(brush_pos.x1, brush_pos.x2);
  int y1 = MIN(brush_pos.y1, brush_pos.y2);
  int x2 = MAX(brush_pos.x1, brush_pos.x2);
  int y2 = MAX(brush_pos.y1, brush_pos.y2);
  int ox1, oy1, ox2, oy2;

  if (!initd)
  {
    /* from initial values */
    obrush.x1 = obrush.y1 = 20;
    obrush.x2 = obrush.y2 = 40;
    initd = 1;
  }

  ox1 = MIN(obrush.x1, obrush.x2);
  oy1 = MIN(obrush.y1, obrush.y2);
  ox2 = MAX(obrush.x1, obrush.x2);
  oy2 = MAX(obrush.y1, obrush.y2);

/*
 * What bins contain the brush and the previous brush?  Allow
 * extension for safety, using BRUSH_MARGIN.
*/

  if (!point_in_which_bin(xg,
    MIN(x1, ox1) - 2*BRUSH_MARGIN,
    MIN(y1, oy1) - 2*BRUSH_MARGIN,
    &bin0->x, &bin0->y) )
  {
    bin0->x = MAX(bin0->x, 0);
    bin0->x = MIN(bin0->x, xg->nhbins - 1);
    bin0->y = MAX(bin0->y, 0);
    bin0->y = MIN(bin0->y, xg->nhbins - 1);
  }
  if (!point_in_which_bin(xg,
    MAX(x2, ox2) + 2*BRUSH_MARGIN,
    MAX(y2, oy2) + 2*BRUSH_MARGIN,
    &bin1->x, &bin1->y) )
  {
    bin1->x = MAX(bin1->x, 0);
    bin1->x = MIN(bin1->x, xg->nhbins - 1);
    bin1->y = MAX(bin1->y, 0);
    bin1->y = MIN(bin1->y, xg->nhbins - 1);
  }

  obrush.x1 = brush_pos.x1;
  obrush.y1 = brush_pos.y1;
  obrush.x2 = brush_pos.x2;
  obrush.y2 = brush_pos.y2;
}
