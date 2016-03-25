/* plot_once.c */
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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

static XPoint *points;
static XSegment *segs;
static XRectangle *open_rectangles;
static XRectangle *filled_rectangles;
static XArc *open_circles;
static XArc *filled_circles;

void
alloc_plot_arrays(xgobidata *xg)
{
  points = (XPoint *) XtMalloc(
    (Cardinal) xg->nrows * sizeof(XPoint));
  segs = (XSegment *) XtMalloc(
    (Cardinal) (2 * xg->nrows) * sizeof(XSegment));
  open_rectangles = (XRectangle *) XtMalloc(
    (Cardinal) xg->nrows * sizeof(XRectangle));
  filled_rectangles = (XRectangle *) XtMalloc(
    (Cardinal) xg->nrows * sizeof(XRectangle));
  open_circles = (XArc *) XtMalloc(
    (Cardinal) xg->nrows * sizeof(XArc));
  filled_circles = (XArc *) XtMalloc(
    (Cardinal) xg->nrows * sizeof(XArc));
}

void
free_plot_arrays()
{
  XtFree((XtPointer) points);
  XtFree((XtPointer) segs);
  XtFree((XtPointer) open_rectangles);
  XtFree((XtPointer) filled_rectangles);
  XtFree((XtPointer) open_circles);
  XtFree((XtPointer) filled_circles);
}

static void
find_point_colors_used(xgobidata *xg, int *ncolors_used,
unsigned long *colors_used)
{
  Boolean new_color;
  int i, k, m;

  /*
   * Loop once through xg->color_now[], collecting the colors currently
   * in use into the colors_used[] vector.
  */
  if (xg->is_point_painting || xg->got_new_paint) {

    *ncolors_used = 1;
    for (i=0; i<xg->nrows_in_plot; i++) {
      m = xg->rows_in_plot[i];
      new_color = True;
      for (k=0; k<*ncolors_used; k++) {
        if (colors_used[k] == xg->color_now[m]) {
          new_color = False;
          break;
        }
      }
      if (new_color) {
        colors_used[*ncolors_used] = xg->color_now[m];
        (*ncolors_used)++;
      }
    }

    /*
     * Make sure that the current brushing color is
     * last in the list, so that it is drawn on top of
     * the pile of points.
    */
    for (k=0; k<(*ncolors_used-1); k++) {
      if (colors_used[k] == xg->color_id) {
        colors_used[k] = colors_used[*ncolors_used-1];
        colors_used[*ncolors_used-1] = xg->color_id;
        break;
      }
    }

    /*
     * Furthermore, make sure that the default color
     * (plotcolors.fg) is first, so that it is drawn
     * underneath the pile of points.  And this should
     * be done after checking for color_id, because
     * sometimes color_id = plotcolors.fg just by default.
    */
    for (k=0; k<(*ncolors_used); k++) {
      if (colors_used[k] == plotcolors.fg && k != 0) {
        colors_used[k] = colors_used[0];
        colors_used[0] = plotcolors.fg;
        break;
      }
    }
  }

}

void
build_plus(icoords *pos, int nrow, XSegment *segv, int nplus, short size)
{
  short x = (short) pos[nrow].x;

  switch (size)
  {
    case TINY:
    case SMALL:
      break;
    case MEDIUM:
    case LARGE:
      size++ ;
      break;
    case JUMBO:
      size = size + 2 ;
      break;
    default:
      fprintf(stderr, "error in build_plus; impossible size %d\n", size);
      size = (short) MEDIUM + 1 ;
      break;
  }
  segv[nplus].x1 = x - size;
  segv[nplus].x2 = x + size;
  segv[nplus].y1 = segv[nplus].y2 = (short) pos[nrow].y;
  nplus++;
  segv[nplus].x1 = segv[nplus].x2 = x;
  segv[nplus].y1 = (short) pos[nrow].y - size;
  segv[nplus].y2 = (short) pos[nrow].y + size;
}

void
build_x(icoords *pos, int nrow, XSegment *segv, int nx, short size)
{
  short x;
  x = (short) pos[nrow].x;
  segv[nx].x1 = x - size;
  segv[nx].x2 = x + size;
  segv[nx].y1 = (short) pos[nrow].y - size;
  segv[nx].y2 = (short) pos[nrow].y + size;
  nx++;
  segv[nx].x1 = x + size;
  segv[nx].x2 = x - size;
  segv[nx].y1 = pos[nrow].y - size;
  segv[nx].y2 = pos[nrow].y + size;
}

void
build_circle(icoords *pos, int nrow, XArc *circv, int ncirc, short size)
{
  size = size * 3;

  circv[ncirc].x = (unsigned short) (pos[nrow].x - size/2);
  circv[ncirc].y = (unsigned short) (pos[nrow].y - size/2);
  circv[ncirc].width = size;
  circv[ncirc].height = size;
  circv[ncirc].angle1 = (short) 0;
  circv[ncirc].angle2 = (short) 23040;
}

void
build_rect(icoords *pos, int nrow, XRectangle * rectv, int nrect, short size)
{
  size = size * 3 - 1;

  rectv[nrect].x = (unsigned short) (pos[nrow].x - (size/2 + 1));
  rectv[nrect].y = (unsigned short) (pos[nrow].y - (size/2 + 1));
  rectv[nrect].width = rectv[nrect].height = size;
}

void
build_point(icoords *pos, int nrow, XPoint * pointv, int npt)
{
  pointv[npt].x = (unsigned short) (pos[nrow].x);
  pointv[npt].y = (unsigned short) (pos[nrow].y);
}

void
build_glyph(xgobidata *xg, int jchar, icoords *xypos, int jpos,
  XPoint *pointv, int *np,
  XSegment *segv, int *ns,
  XRectangle *openrectv, int *nr_open,
  XRectangle *filledrectv, int *nr_filled,
  XArc *openarcv, int *nc_open,
  XArc *filledarcv, int *nc_filled)
  /*
   * Use jchar for indexing the glyph and color vectors;
   * use jpos for indexing xypos
  */
{
  short size, type;

  /*
   * If running the section tour, use the section type and size.
  */
  if (xg->is_tour_section) {
    type = xg->section_glyph_ids[jchar].type;
    size = xg->section_glyph_ids[jchar].size;
  } else {
    type = xg->glyph_now[jchar].type;
    size = xg->glyph_now[jchar].size;
  }

  if (type == PLUS_GLYPH) {
    build_plus(xypos, jpos, segv, *ns, size);
    *ns = *ns+2;
  } else if (type == X_GLYPH) {
    build_x(xypos, jpos, segv, *ns, size);
    *ns = *ns+2;
  }
  else if (type == OPEN_RECTANGLE_GLYPH) {
    build_rect(xypos, jpos, openrectv, *nr_open, size);
    (*nr_open)++;
  }
  else if (type == FILLED_RECTANGLE_GLYPH) {
    build_rect(xypos, jpos, filledrectv, *nr_filled, size);
    (*nr_filled)++;
  }
  else if (type == OPEN_CIRCLE_GLYPH) {
    build_circle(xypos, jpos, openarcv, *nc_open, size);
    (*nc_open)++;
  }
  else if (type == FILLED_CIRCLE_GLYPH) {
    build_circle(xypos, jpos, filledarcv, *nc_filled, size);
    (*nc_filled)++;
  }
  else if (type == POINT_GLYPH) {
    build_point(xypos, jpos, pointv, *np);
    (*np)++;
  }
  else {
    (void) fprintf(stderr, "build_glyph: impossible glyph type %d\n", type);
  }
}

static void
clear_glyphs(xgobidata *xg, int nr_open, int nc_open)
{
  if (nr_open)
    XFillRectangles(display, xg->pixmap0, clear_GC,
      open_rectangles, nr_open);
  /*
   * kludge to get around X bug:  Can't draw
   * more than 65535 circles according to the source code in
   * XDrArcs.c.  But 65535 fails, too, and 32768.
  */
  if (nc_open) {
    int k = 0;
    while (nc_open > MAXARCS) {
      XFillArcs(display, xg->pixmap0, clear_GC,
        &open_circles[k*MAXARCS], MAXARCS);
      nc_open -= MAXARCS;
      k++;
    }
    XFillArcs(display, xg->pixmap0, clear_GC,
      &open_circles[k*MAXARCS], nc_open);
  }
}

static void
draw_glyphs(xgobidata *xg, int np, int ns, int nr_open, int nr_filled,
int nc_open, int nc_filled)
{
  if (np)
    XDrawPoints(display, xg->pixmap0, copy_GC,
      points, np, CoordModeOrigin);
  if (ns)
    XDrawSegments(display, xg->pixmap0, copy_GC,
      segs, ns);
  if (nr_open)
    XDrawRectangles(display, xg->pixmap0, copy_GC,
      open_rectangles, nr_open);
  if (nr_filled) {
    XDrawRectangles(display, xg->pixmap0, copy_GC,
      filled_rectangles, nr_filled);
    XFillRectangles(display, xg->pixmap0, copy_GC,
      filled_rectangles, nr_filled);
  }

  if (nc_open) {
    int k = 0;
    while (nc_open > MAXARCS) {
      XDrawArcs(display, xg->pixmap0, copy_GC,
        &open_circles[k*MAXARCS], MAXARCS);
      nc_open -= MAXARCS;
      k++;
    }
    XDrawArcs(display, xg->pixmap0, copy_GC,
      &open_circles[k*MAXARCS], nc_open);
  }
  if (nc_filled) {
    int k = 0;
    while (nc_filled > MAXARCS) {
      XDrawArcs(display, xg->pixmap0, copy_GC,
        &filled_circles[k*MAXARCS], MAXARCS);
      XFillArcs(display, xg->pixmap0, copy_GC,
        &filled_circles[k*MAXARCS], MAXARCS);
      nc_filled -= MAXARCS;
      k++;
    }
    XDrawArcs(display, xg->pixmap0, copy_GC,
      &filled_circles[k*MAXARCS], nc_filled);
    XFillArcs(display, xg->pixmap0, copy_GC,
      &filled_circles[k*MAXARCS], nc_filled);
  }
}


void
find_tick_label(xgobidata *xg, int var, int tickn, float *ticknos, char *str)
{
  int length;
  float ftmp;

/*
 * deci:  number of decimal places in each tickdelta; one per column
 * var:  index into deci array
 * ticknos: xticks or yticks, vector of ticklabel values
 * tickn: indec into ticknos array
*/
  if ((xg->deci[var] > 4 ||
       fabs((double) ticknos[tickn]) >= 10000.) &&
       ticknos[tickn] != 0.0)
  {
    number_length(ticknos[tickn], &length);
    ftmp = (float) ticknos[tickn]/pow((double)10, (double)(length-1));
    if (ftmp == (float)(int)ftmp)
      (void) sprintf (str, "%.0e", ticknos[tickn]);
    else
      (void) sprintf (str, "%.2e", ticknos[tickn]);
  }
  else {
    switch(xg->deci[var]) {
      case 0:
        (void) sprintf (str, "%3.0f", ticknos[tickn]);
        break;
      case 1:
        (void) sprintf (str, "%3.1f", ticknos[tickn]);
        break;
      case 2:
        (void) sprintf (str, "%3.2f", ticknos[tickn]);
        break;
      case 3:
        (void) sprintf (str, "%3.3f", ticknos[tickn]);
        break;
      case 4:
        (void) sprintf (str, "%3.4f", ticknos[tickn]);
        break;
    }
  }
}

void
draw_nearest_point(xgobidata *xg)
{
  int found = 0, i;

  for (i=0; i<xg->nsticky_ids; i++) {
    if (xg->sticky_ids[i] == xg->nearest_point) {
      found = 1 ;
      break ;
    }
  }

  if (!found) {
    draw_diamond_around_point(xg->nearest_point, xg->plot_window, xg);
    plot_nearest_id(xg, xg->plot_window);
  }
}

static void
plot_sticky_ids(xgobidata *xg)
{
  int i, k;
/*
 * Draw the ids of any cases that have been designated sticky.
*/
  for (i=0; i<xg->nsticky_ids; i++) {
   if ( !xg->erased[ k = xg->sticky_ids[i] ] ) {
     draw_diamond_around_point(k, xg->pixmap0, xg);
     XDrawImageString(display, xg->pixmap0, clear_GC,
       xg->screen[k].x + 3, xg->screen[k].y - 3,
       xg->rowlab[k],
       strlen(xg->rowlab[k]));
   }
  }
}

static void
draw_axes(xgobidata *xg)
{
  int xstart, ystart;

  XSetForeground(display, copy_GC, xg->axisColor);

  if (xg->is_plotting1d) {

    if (xg->plot1d_vars.y != -1) { /* plotting vertically */
      ystart = check_y_axis(xg, xg->plot1d_vars.y, &xg->ticks);
      add_y_axis(xg, &xg->plot1d_vars, ystart, &xg->ticks);
      if (xg->add_gridlines) 
        add_y_gridlines(xg, xg->plot1d_vars.y, ystart, &xg->ticks);
    } else {
      xstart = check_x_axis(xg, xg->plot1d_vars.x, &xg->ticks);
      add_x_axis(xg, &xg->plot1d_vars, xstart, &xg->ticks);
      if (xg->add_gridlines)
        add_x_gridlines(xg, xg->plot1d_vars.x, xstart, &xg->ticks);
    }

  } else if (xg->is_xyplotting) {

    xstart = check_x_axis(xg, xg->xy_vars.x, &xg->ticks);
    add_x_axis(xg, &xg->xy_vars, xstart, &xg->ticks);
    ystart = check_y_axis(xg, xg->xy_vars.y, &xg->ticks);
    add_y_axis(xg, &xg->xy_vars, ystart, &xg->ticks);

    if (xg->add_gridlines) {
      add_x_gridlines(xg, xg->xy_vars.x, xstart, &xg->ticks);
      add_y_gridlines(xg, xg->xy_vars.y, ystart, &xg->ticks);
    }
   
  } else if (xg->is_spinning) {

    if (xg->is_spin_type.oblique)
      draw_ob_spin_axes(xg);
    else
      draw_ax_spin_axes(xg);
  }
  else if (xg->is_touring)
    draw_tour_axes(xg);
  else if (xg->is_corr_touring)
    draw_corr_axes(xg);

  XSetForeground(display, copy_GC, plotcolors.fg);
}

static void
add_decorations(xgobidata *xg)
{
  extern void draw_scaling_crosshair(xgobidata *);
/*
 * These get drawn to the window instead of the pixmap
*/
  if (xg->nearest_point != -1)
    draw_nearest_point(xg);

  if (xg->is_brushing) {
    draw_brush(xg);
    add_under_brush_label(xg);
  }

  else if (xg->is_line_editing)
    draw_editing_lines(xg);

  else if (xg->is_scaling)
    draw_scaling_crosshair(xg);
}

void
plot_once(xgobidata *xg)
{
  int j, k, m;
  unsigned long current_color;
  static int npoint_colors_used = 1;
  static unsigned long point_colors_used[NCOLORS+2];
  int npt, nseg, nr_open, nr_filled, nc_open, nc_filled;

/*
 * First clear the background pixmap
*/
  XFillRectangle(display, xg->pixmap0, clear_GC,
    0, 0, xg->plotsize.width, xg->plotsize.height );

/*
 * Draw the axes first so they'll be behind everything.
*/
  if (xg->is_axes)
    draw_axes(xg);

/*
 * Draw the lines first so that lines will be "behind" open glyphs.
*/
  if ((xg->connect_the_points || xg->plot_the_arrows) && xg->nlines > 0) {
    draw_connecting_lines(xg);
    XSync(display, False);
  }

  if (xg->plot_the_points) {
    if (!mono) {
      find_point_colors_used(xg, &npoint_colors_used, point_colors_used);

      /*
       * Now look through point_colors_used[], plotting the points of each
       * color in a group.
      */
      for (k=0; k<npoint_colors_used; k++)
      {
        current_color = point_colors_used[k];
        npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;
        for (m=0; m<xg->nrows_in_plot; m++)
        {
          j = xg->rows_in_plot[m];
          if (!xg->erased[j])
            if (xg->color_now[j] == current_color)
              build_glyph(xg, j, xg->screen, j,
                points, &npt,
                segs, &nseg,
                open_rectangles, &nr_open,
                filled_rectangles, &nr_filled,
                open_circles, &nc_open,
                filled_circles, &nc_filled);
        }
        XSetForeground(display, copy_GC, current_color);
        draw_glyphs(xg, npt, nseg, nr_open, nr_filled, nc_open, nc_filled);
      }
    }
    else  /* if (is_mono) */
    {
      npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;
      for (m=0; m<xg->nrows_in_plot; m++)
      {
        j = xg->rows_in_plot[m];
        if (!xg->erased[j])
          build_glyph(xg, j, xg->screen, j,
            points, &npt,
            segs, &nseg,
            open_rectangles, &nr_open,
            filled_rectangles, &nr_filled,
            open_circles, &nc_open,
            filled_circles, &nc_filled);
      }
      /*
       * Fill in the backgrounds with the background color to wipe
       * out the lines.
      */
      if (xg->connect_the_points && xg->nlines > 0)
        clear_glyphs(xg, nr_open, nc_open);

      draw_glyphs(xg, npt, nseg, nr_open, nr_filled, nc_open, nc_filled);
    }
  }

/*
 *if (xg->connect_the_points && xg->nlines > 0)
 *  draw_connecting_lines(xg);
*/

  plot_sticky_ids(xg);

  if (xg->is_smoothing)
    smooth_data(xg);

  XCopyArea(display, xg->pixmap0, xg->plot_window, copy_GC,
    0, 0,
    xg->plotsize.width, xg->plotsize.height, 0, 0 );

/*
 * The rest of these get drawn to the window instead of the pixmap
*/
  add_decorations(xg);

  xg->got_new_paint = 0;

#if defined RPC_USED || defined DCE_RPC_USED
  xfer_brushinfo (xg);
#endif

  return;
}

void
plot_bins(xgobidata *xg)
{
  icoords bin0, bin1;
  icoords loc0, loc1;
  icoords loc_clear0, loc_clear1;
  int ih, iv, j, m, k, new_color;
  unsigned long current_color;
  static int npoint_colors_used;
  static unsigned long point_colors_used[NCOLORS+2];
  int npt, nseg, nr_open, nr_filled, nc_open, nc_filled;

  find_extended_brush_corners(&bin0, &bin1, xg);

/*
 * Determine locations of bin corners: upper left edge of loc0;
 * lower right edge of loc1.
*/
  loc0.x = (int) ((float) bin0.x / (float) xg->nhbins * (xg->max.x+1.0));
  loc0.y = (int) ((float) bin0.y / (float) xg->nvbins * (xg->max.y+1.0));
  loc1.x = (int) ((float) (bin1.x+1) / (float) xg->nhbins * (xg->max.x+1.0));
  loc1.y = (int) ((float) (bin1.y+1) / (float) xg->nvbins * (xg->max.y+1.0));

/*
 * Clear an area a few pixels inside that region.  Watch out
 * for border effects.
*/
  loc_clear0.x = (bin0.x == 0) ? 0 : loc0.x + BRUSH_MARGIN;
  loc_clear0.y = (bin0.y == 0) ? 0 : loc0.y + BRUSH_MARGIN;
  loc_clear1.x = (bin1.x == xg->nhbins-1) ? xg->max.x : loc1.x - BRUSH_MARGIN;
  loc_clear1.y = (bin1.y == xg->nvbins-1) ? xg->max.y : loc1.y - BRUSH_MARGIN;

  XFillRectangle(display, xg->pixmap0, clear_GC,
    loc_clear0.x, loc_clear0.y,
    (Dimension) (1 + loc_clear1.x - loc_clear0.x) ,
    (Dimension) (1 + loc_clear1.y - loc_clear0.y) );

/*
 * Draw the axes first so they'll be behind everything.
*/
  if (xg->is_axes)
    draw_axes(xg);

/*
 * Next the connecting lines, using similar reasoning.
*/
  if ((xg->connect_the_points || xg->plot_the_arrows) && xg->nlines > 0)
    draw_connecting_lines(xg);

/*
 * Plot the points that live inside the bins defined by the extended brush.
 * This is an edited version of plot_once().
*/
  if (xg->plot_the_points) {
    if (!mono) {
      /*
       * Need this line here to initialize -- especially to cover
       * the odd case that you're using xgobi in mono mode on a
       * color display.
      */
      point_colors_used[0] = xg->color_now[0];
      /*
       * Loop once through color_now[], collecting the colors currently
       * in use into the point_colors_used[] vector.
       *
       * Since this only loops over the points in the current
       * bins, I should do it whenever brushing?
      */
      if (xg->is_point_painting) {
        npoint_colors_used = 1;
        for (ih=bin0.x; ih<=bin1.x; ih++) {
          for (iv=bin0.y; iv<=bin1.y; iv++) {
            for (m=0; m<xg->bincounts[ih][iv]; m++) {
              j = xg->rows_in_plot[xg->binarray[ih][iv][m]];

              new_color = 1;
              for (k=0; k<npoint_colors_used; k++) {
                if (point_colors_used[k] == xg->color_now[j]) {
                  new_color = 0;
                  break;
                }
              }
              if (new_color) {
                point_colors_used[npoint_colors_used] = xg->color_now[j];
                npoint_colors_used++;
              }
            }
          }
        }
      }

      /*
       * Now look through point_colors_used[], plotting the points of each
       * color in a group.
      */
      for (k=0; k<npoint_colors_used; k++)
      {
        current_color = point_colors_used[k];
        npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;
        for (ih=bin0.x; ih<=bin1.x; ih++) {
          for (iv=bin0.y; iv<=bin1.y; iv++) {
            for (m=0; m<xg->bincounts[ih][iv]; m++) {
              j = xg->rows_in_plot[xg->binarray[ih][iv][m]];

              if (!xg->erased[j])
                if (xg->color_now[j] == current_color)
                  build_glyph(xg, j, xg->screen, j,
                    points, &npt,
                    segs, &nseg,
                    open_rectangles, &nr_open,
                    filled_rectangles, &nr_filled,
                    open_circles, &nc_open,
                    filled_circles, &nc_filled);
            }
          }
        }
        XSetForeground(display, copy_GC, current_color);
        draw_glyphs(xg, npt, nseg, nr_open, nr_filled, nc_open, nc_filled);
      }
    }
    else  /* if (is_mono) */
    {
      npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;
      for (ih=bin0.x; ih<=bin1.x; ih++) {
        for (iv=bin0.y; iv<=bin1.y; iv++) {
          for (m=0; m<xg->bincounts[ih][iv]; m++) {

            j = xg->rows_in_plot[xg->binarray[ih][iv][m]];

            if (!xg->erased[j])
              build_glyph(xg, j, xg->screen, j,
                points, &npt,
                segs, &nseg,
                open_rectangles, &nr_open,
                filled_rectangles, &nr_filled,
                open_circles, &nc_open,
                filled_circles, &nc_filled);
          }
        }
      }
      draw_glyphs(xg, npt, nseg, nr_open, nr_filled, nc_open, nc_filled);
    }
  }

  plot_sticky_ids(xg);

/*
 * Copy from pixmap back to the plot window.
*/
  XCopyArea(display, xg->pixmap0, xg->plot_window, copy_GC,
    loc0.x, loc0.y,
    (Dimension) (loc1.x - loc0.x), (Dimension) (loc1.y - loc0.y),
    loc0.x, loc0.y );
/*
 * Draw these straight to the window
*/
  add_decorations(xg);

  xg->got_new_paint = 0;

#if defined RPC_USED || defined DCE_RPC_USED
  xfer_brushinfo(xg);
#endif

  return;
}

void
quickplot_once(xgobidata *xg)
{
/*
 * Copy from pixmap back to the plot window and then add
 * decorations:  for replotting when only the decorations have
 * changed, the points haven't changed position or characteristic.
 * Used in identification, for instance ...
*/
  XCopyArea(display, xg->pixmap0, xg->plot_window, copy_GC,
    0, 0,
    xg->plotsize.width, xg->plotsize.height, 0, 0 );

/*
 * Draw these straight to the window
*/
  add_decorations(xg);
  return;
}
