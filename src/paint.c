/* paint.c */
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

/*
 * October 21, 1999.  Added code that prevents the brushing
 * of the non-linkable points.  dfs, di
*/

static unsigned long npts_under_brush = 0;

void
add_under_brush_label(xgobidata *xg) {
  if (npts_under_brush > 0) {
    char str[32];
    sprintf(str, "%d", npts_under_brush);
    XDrawImageString(display, xg->plot_window, copy_GC,
      5,
      xg->plotsize.height - 5,
      str,
      strlen(str));
  }
}

static Boolean
update_glyph_arrays(int i, int k, Boolean changed, xgobidata *xg) {
  unsigned long ltype;
  Boolean new_changed = changed;
  int nels;

  if (xg->under_new_brush[i]) {
    if (xg->brush_mode == undo) {
      xg->glyph_now[i].type = xg->glyph_prev[i].type;
      xg->glyph_now[i].size = xg->glyph_prev[i].size;
    }
    else {
      xg->glyph_now[i].type = xg->glyph_id.type;
      xg->glyph_now[i].size = xg->glyph_id.size;
    }

    if (xg->brush_mode != transient) {
      xg->glyph_ids[i].type = xg->glyph_now[i].type;
      xg->glyph_ids[i].size = xg->glyph_now[i].size;
    }
  }
  else {
    xg->glyph_now[i].type = xg->glyph_ids[i].type;
    xg->glyph_now[i].size = xg->glyph_ids[i].size;
  }

  /*
   * Keep senddata up to date
  */

  /*
   * Watch for changed in a single place
  */
  nels = (xg->nrgroups) ? xg->nrgroups_in_plot : xg->nlinkable_in_plot;
  ltype = (unsigned long) glyph_color_pointtype(xg, i);
  if (!changed && xg->senddata[9 + nels + k] != ltype)
    new_changed = True;
  xg->senddata[9 + nels + k] = (unsigned long) ltype;

  return(new_changed);
}

static Boolean
build_glyph_vectors(xgobidata *xg)
{
  int ih, iv, m, j, k, gp, n, p;
  static icoords obin0 = {NHBINS/2, NVBINS/2};
  static icoords obin1 = {NHBINS/2, NVBINS/2};
  icoords imin, imax;
  Boolean changed = False;

  if (xg->brush_mode == transient) {
    imin.x = MIN(xg->bin0.x, obin0.x);
    imin.y = MIN(xg->bin0.y, obin0.y);
    imax.x = MAX(xg->bin1.x, obin1.x);
    imax.y = MAX(xg->bin1.y, obin1.y);
  }
  else {
    imin.x = xg->bin0.x;
    imin.y = xg->bin0.y;
    imax.x = xg->bin1.x;
    imax.y = xg->bin1.y;
  }

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<xg->bincounts[ih][iv]; m++) {
        /*
         * j is the row number; k is the index of rows_in_plot[]
        */
        j = xg->rows_in_plot[ k = xg->binarray[ih][iv][m] ] ;

        if (j < xg->nlinkable) {

          /* update the glyph arrays for every member of the row group */
          if (xg->nrgroups > 0) {
            gp = xg->rgroup_ids[k];
            for (n=0; n<xg->rgroups[gp].nels; n++) {
              p = xg->rgroups[gp].els[n];
/* doing unnecessary work */
              changed = update_glyph_arrays(p, gp, changed, xg);
            }

          } else
            /* update the glyph arrays only for this guy */
            changed = update_glyph_arrays(j, k, changed, xg);
        }
      }
    }
  }

  obin0.x = xg->bin0.x;
  obin0.y = xg->bin0.y;
  obin1.x = xg->bin1.x;
  obin1.y = xg->bin1.y;

  return(changed);
}

static Boolean
update_color_arrays(int i, int k, Boolean changed, xgobidata *xg) {
  unsigned long ltype;
  Boolean new_changed = changed;
  int nels;

  if (xg->under_new_brush[i]) {
    if (xg->brush_mode == undo)
      xg->color_now[i] = xg->color_prev[i];
    else {
      xg->color_now[i] = xg->color_id;
    }

    if (xg->brush_mode != transient)
      xg->color_ids[i] = xg->color_now[i];
  }
  else
    xg->color_now[i] = xg->color_ids[i];

  /*
   * Keep senddata up to date, and watch for changes as a result
   * of color brushing.
   *
   * If rgroups, then k is the rgroup id
  */
  nels = (xg->nrgroups) ? xg->nrgroups_in_plot : xg->nlinkable_in_plot;
  ltype = (unsigned long) glyph_color_pointtype(xg, i);
  if (!changed && xg->senddata[9 + nels + k] != ltype)
    new_changed = True;
  xg->senddata[9 + nels + k] = (unsigned long) ltype;

  return(new_changed);
}

static Boolean
build_color_vectors(xgobidata *xg)
{
  int ih, iv, m, j, k, gp, n, p;
  static icoords obin0 = {NHBINS/2, NVBINS/2};
  static icoords obin1 = {NHBINS/2, NVBINS/2};
  icoords imin, imax;
  Boolean changed = False;

  if (xg->brush_mode == transient) {
    imin.x = MIN(xg->bin0.x, obin0.x);
    imin.y = MIN(xg->bin0.y, obin0.y);
    imax.x = MAX(xg->bin1.x, obin1.x);
    imax.y = MAX(xg->bin1.y, obin1.y);
  }
  else {
    imin.x = xg->bin0.x;
    imin.y = xg->bin0.y;
    imax.x = xg->bin1.x;
    imax.y = xg->bin1.y;
  }

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<xg->bincounts[ih][iv]; m++) {
        j = xg->rows_in_plot[ k = xg->binarray[ih][iv][m] ] ;
        /*
         * k   raw index, based on nrows
         * j   index based on nrows_in_plot
        */
        if (j < xg->nlinkable) {

          if (xg->nrgroups > 0) {
            /* update the color arrays for every member of the row group */
            gp = xg->rgroup_ids[k];
            for (n=0; n<xg->rgroups[gp].nels; n++) {
              p = xg->rgroups[gp].els[n];

/* doing unnecessary work */
              changed = update_color_arrays(p, gp, changed, xg);

            }

          } else {
            /* update the color arrays only for this guy */
            changed = update_color_arrays(j, k, changed, xg);
          }
        }
      }
    }
    obin0.x = xg->bin0.x;
    obin0.y = xg->bin0.y;
    obin1.x = xg->bin1.x;
    obin1.y = xg->bin1.y;
  }

  return(changed);
}

void
init_glyph_ids(xgobidata *xg)
{
  int j;

  for (j=0; j<xg->nrows; j++) {
    xg->glyph_ids[j].type = xg->glyph_now[j].type =
      xg->glyph_prev[j].type = xg->glyph_0.type;
    xg->glyph_ids[j].size = xg->glyph_now[j].size =
      xg->glyph_prev[j].size = xg->glyph_0.size;
  }
}

void
init_color_ids(xgobidata *xg)
{
  int j;

  xg->color_id = xg->color_0;
  for (j=0; j<xg->nrows; j++) {
    xg->color_ids[j] = xg->color_now[j] =
      xg->color_prev[j] = xg->color_0;
  }
}

void
init_erase(xgobidata *xg)
{
  int j;

  for (j=0; j<xg->nrows; j++)
    xg->erased[j] = 0;
}

Boolean
active_paint_points(xgobidata *xg)
{
  int ih, iv, j, pt, k, gp;
  Boolean changed;
/*
 * Set under_new_brush[j] to 1 if point j is inside the rectangular brush.
*/

  /* Zero out under_new_brush[] before looping */
  npts_under_brush = 0;
  for (j=0; j<xg->nrows_in_plot; j++)
    xg->under_new_brush[xg->rows_in_plot[j]] = 0;

  /*
   * bincounts[][] and binarray[][][] only represent the the
   * rows in rows_in_plot[] so there's no need to test for that.
  */
  for (ih=xg->bin0.x; ih<=xg->bin1.x; ih++) {
    for (iv=xg->bin0.y; iv<=xg->bin1.y; iv++) {
      for (j=0; j<xg->bincounts[ih][iv]; j++) {
        pt = xg->rows_in_plot[xg->binarray[ih][iv][j]];

        if (pt < xg->nlinkable) {

          if (!xg->erased[pt] && under_brush(xg, pt)) {
            xg->under_new_brush[pt] = 1;
            npts_under_brush++ ;

            /* brush other members of this row group */
            if (xg->nrgroups > 0) {
              gp = xg->rgroup_ids[pt];
              if (gp < xg->nrgroups) {  /* exclude points without an rgroup */
                for (k=0; k<xg->rgroups[gp].nels; k++) {
                  if (!xg->erased[ xg->rgroups[gp].els[k] ])
                    xg->under_new_brush[ xg->rgroups[gp].els[k] ] = 1;
                }
              }
            }
            /* */

          }
        }
      }
    }
  }

  changed = False;

  /*
   * Now, using the under_new_brush[] vector that only contains un-erased
   * points, build the color and glyph vectors.
  */
  if (xg->is_color_painting)
    changed = build_color_vectors(xg) || changed;

  if (xg->is_glyph_painting)
    changed = build_glyph_vectors(xg) || changed;

  return(changed);
}

void
init_line_colors(xgobidata *xg)
{
  int j;

  for (j=0; j<xg->nlines; j++)
  {
    xg->line_color_ids[j] = xg->line_color_now[j] =
      xg->line_color_prev[j] = plotcolors.fg;
  }
}

static void
build_line_color_vectors(xgobidata *xg)
{
  int j;

  for (j=0; j<xg->nlines; j++) {
    if (xg->xed_by_new_brush[j]) {

      if (xg->brush_mode == undo)
        xg->line_color_now[j] = xg->line_color_prev[j];
      else
        xg->line_color_now[j] = xg->color_id;

      if (xg->brush_mode != transient)
        xg->line_color_ids[j] = xg->line_color_now[j];
    }
    else
      xg->line_color_now[j] = xg->line_color_ids[j];
  }
}

void
active_paint_lines(xgobidata *xg)
{
  int j, k, from, to, gp;
/*
 * Set xed_by_new_brush[j] to 1 if line j intersects the crosshair brush.
 * If we're also point painting and the rectangular brush is shown,
 * then also set xed_by_new_brush[j] to 1 if both a and b are contained
 * within the brush.
*/
  for (j=0; j<xg->nlines; j++)
    xg->xed_by_new_brush[j] = 0;

  for (j=0; j<xg->nlines; j++) {
    from = xg->connecting_lines[j].a - 1;
    to = xg->connecting_lines[j].b - 1;

    if (from < xg->nlinkable && to < xg->nlinkable &&
        !xg->erased[from] && !xg->erased[to])
    {
      if (xed_by_brush(xg, from, to))
        xg->xed_by_new_brush[j] = 1;
      else {
        if (xg->is_point_painting) {
          if (under_brush(xg, from) && under_brush(xg, to))
            xg->xed_by_new_brush[j] = 1;
        }
      }

      /* brush other members of this line group */
      if (xg->nlgroups) {
        if (xg->xed_by_new_brush[j]) {
          gp = xg->lgroup_ids[j];
          if (gp > -1 && xg->lgroups[gp].nels > 1) {
            for (k=0; k<xg->lgroups[gp].nels; k++) {
              xg->xed_by_new_brush[ xg->lgroups[gp].els[k] ] = 1;
            }
          }
        }
      }
      /* */

    }
  }

/*
  for (j=0;j<xg->nlines;j++)
  printf("%d ", xg->xed_by_new_brush[j]);
  printf("\n");
*/

  if (xg->is_color_painting)
    build_line_color_vectors(xg);
}
