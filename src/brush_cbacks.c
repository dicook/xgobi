/* brush_cbacks.c */
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
 *  dfs@research.att.com        dicook@iastate.edu          *
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

#define NAMESIZE 15
#define NAMESV(j) (namesv + j*NAMESIZE)

/* ARGSUSED */
XtCallbackProc
brush_points_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  xg->is_point_painting = !xg->is_point_painting;
  quickplot_once(xg);

  setToggleBitmap(w, xg->is_point_painting);
}

/* ARGSUSED */
XtCallbackProc
brush_active_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  xg->brush_on = !xg->brush_on ;
  quickplot_once(xg);

  setToggleBitmap(w, xg->brush_on);
}

/* ARGSUSED */
XtCallbackProc
brush_lines_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  xg->is_line_painting = !xg->is_line_painting;
  quickplot_once(xg);

  setToggleBitmap(w, xg->is_line_painting);
}

/* ARGSUSED */
XtCallbackProc
br_perst_cback (Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Turn persistent brushing on or off.
*/
{
  if (xg->brush_mode != persistent)
  { 
    int j, k;

    xg->brush_mode = persistent;

    for (j=0; j<xg->nrows_in_plot; j++)
    {
      k = xg->rows_in_plot[j];
      xg->color_ids[k] =      xg->color_now[k] ;
      xg->glyph_ids[k].type = xg->glyph_now[k].type;
      xg->glyph_ids[k].size = xg->glyph_now[k].size;
    }
  }

  reset_br_types(xg);
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
br_trans_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  /*
   * If transient brushing is being turned off, also
   * restore colors and glyphs to persistent identities.
  */
/* No, don't */
  if (xg->brush_mode == transient)
  {
/*
    int j;
    for (j=0; j<xg->nrows_in_plot; j++)
    {
      xg->glyph_now[j].type = xg->glyph_ids[j].type;
      xg->glyph_now[j].size = xg->glyph_ids[j].size;
      xg->color_now[j] = xg->color_ids[j];
    }
*/
  }
  else {
    xg->brush_mode = transient;
  }

  reset_br_types(xg);
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
br_undo_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (xg->brush_mode != undo) {
    xg->brush_mode = undo;
  }

  reset_br_types(xg);
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
br_update_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (xg->plot_the_points) {
    if (xg->link_glyph_brushing ||
        xg->link_color_brushing ||
        xg->link_erase_brushing)
    {
      XtOwnSelection( (Widget) xg->workspace,
        (Atom) XG_ROWSINPLOT,
        (Time) CurrentTime,
        (XtConvertSelectionProc) pack_rowsinplot_data,
        (XtLoseSelectionProc) pack_brush_lose ,
        (XtSelectionDoneProc) pack_brush_done );
      announce_rows_in_plot(xg);

      XtOwnSelection( (Widget) xg->workspace,
        (Atom) XG_NEWPAINT,
        (Time) CurrentTime, /* doesn't work with XtLastTimeStamp...? */
        (XtConvertSelectionProc) pack_brush_data,
        (XtLoseSelectionProc) pack_brush_lose ,
        (XtSelectionDoneProc) pack_brush_done );
      announce_brush_data(xg);
    }
  }

  if ((xg->link_points_to_points || xg->link_points_to_lines) &&
    xg->connect_the_points)
  {
    XtOwnSelection( (Widget) xg->workspace,
      (Atom) XG_NEWLINEPAINT,
      (Time) CurrentTime, /* doesn't work with XtLastTimeStamp...? */
      (XtConvertSelectionProc) pack_line_brush_data,
      (XtLoseSelectionProc) pack_line_brush_lose ,
      (XtSelectionDoneProc) pack_line_brush_done );
    announce_line_brush_data(xg);
  }
}

/* ARGSUSED */
XtCallbackProc
reset_brush_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  init_brush_size(xg);
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
reset_point_colors_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int j;

  if (!mono)
  {
    for (j=0; j<xg->nrows; j++)
      xg->color_ids[j] = xg->color_now[j] = plotcolors.fg;
  }

  copy_brushinfo_to_senddata(xg);
  if (xg->link_color_brushing)
  {
    XtOwnSelection( (Widget) xg->workspace,
      (Atom) XG_NEWPAINT,
      (Time) CurrentTime, /* doesn't work with XtLastTimeStamp...? */
      /*(Time) XtLastTimestampProcessed(display),*/
      (XtConvertSelectionProc) pack_brush_data,
      (XtLoseSelectionProc) pack_brush_lose ,
      (XtSelectionDoneProc) pack_brush_done );
    announce_brush_data(xg);
  }
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
reset_line_colors_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (!mono)
    init_line_colors(xg);
  if (xg->link_points_to_points || xg->link_points_to_lines)
  {
    XtOwnSelection( (Widget) xg->workspace,
      (Atom) XG_NEWLINEPAINT,
      (Time) CurrentTime, /* doesn't work with XtLastTimeStamp...? */
      (XtConvertSelectionProc) pack_line_brush_data,
      (XtLoseSelectionProc) pack_line_brush_lose ,
      (XtSelectionDoneProc) pack_line_brush_done );
    announce_line_brush_data(xg);
  }
  plot_once(xg);
}

/* ARGSUSED */
XtCallbackProc
reset_glyphs_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  init_glyph_ids(xg);

  copy_brushinfo_to_senddata(xg);
  if (xg->link_glyph_brushing)
  {
    XtOwnSelection( (Widget) xg->workspace,
      (Atom) XG_NEWPAINT,
      (Time) CurrentTime, /* doesn't work with XtLastTimeStamp...? */
      /*(Time) XtLastTimestampProcessed(display),*/
      (XtConvertSelectionProc) pack_brush_data,
      (XtLoseSelectionProc) pack_brush_lose ,
      (XtSelectionDoneProc) pack_brush_done );
    announce_brush_data(xg);
  }
  plot_once(xg);
}

int
find_gid(glyphv *glyph)
{
  int gid = 0;

  gid = NGLYPHSIZES*(glyph->type-1) + glyph->size ;
  return(gid);
}

void
update_nrgroups_in_plot(xgobidata *xg) {
  /*
   * Whenever rows_in_plot is reset, reset nrgroups_in_plot as well.
   * Make sure the values of excluded are set.
  */
  if (xg->nrgroups > 0) {
    int i;
    xg->nrgroups_in_plot = 0;
    for (i=0; i<xg->nrgroups; i++) {
      if (xg->excluded[ xg->rgroups[i].els[0] ])
        xg->rgroups[i].excluded = true;
      else {
        xg->rgroups[i].excluded = false;
        xg->nrgroups_in_plot++;
      }
    }
  }
}

void
reset_rows_in_plot(xgobidata *xg, Boolean reset_lims)
{
  int i, j;

  /*
   * Make sure the value of nlinkable_in_plot corresponds to 
   * nrows_in_plot
  */
  xg->nlinkable_in_plot = 0;
  for (i=0; i<xg->nlinkable; i++)
    if (!xg->excluded[i])
      xg->nlinkable_in_plot++;
  

  if (xg->is_plotting1d)
    plot1d_texture_var(xg);

  /*
   * dfs: changing 9 November 99, because update_vc gives me
   * purify errors if tour hasn't been run.
  */
  if (xg->plot_mode == GTOUR_MODE && xg->ncols_used > 2)
    update_sphered(xg, (int *) NULL, xg->ncols_used);
  if (reset_lims)
    update_lims(xg);
  update_world(xg);
  world_to_plane(xg);
  plane_to_screen(xg);

  if (reset_lims) {
    if (xg->is_xyplotting) {
      for (j=0; j<xg->ncols_used; j++) {
        xg->nicelim[j].min = xg->lim0[j].min;
        xg->nicelim[j].max = xg->lim0[j].max;
        SetNiceRange(j, xg);
        xg->deci[j] = set_deci(xg->tickdelta[j]);
      }
      init_ticks(&xg->xy_vars, xg);
    }
    else if (xg->is_plotting1d)
      init_ticks(&xg->plot1d_vars, xg);
  }

  if (xg->is_pp) {
    xg->recalc_max_min = True;
    reset_pp_plot();
    pp_index(xg,0,1);
  }
}

/* ARGSUSED */
XtCallbackProc
delete_erased_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i;
  Boolean nerased = False;

  /*
   * First make sure that some points are erased.
  */
  for (i=0; i<xg->nrows; i++)
    if (!xg->erased[i]) {
      nerased = True;
      break;
    }

  if (nerased) {

    xg->nrows_in_plot = 0;
    for (i=0; i<xg->nrows; i++) {
      xg->excluded[i] = xg->erased[i];

      if (!xg->erased[i]) {
        xg->rows_in_plot[(xg->nrows_in_plot)++] = i;
        xg->excluded[i] = 0;
      }
    }

    update_nrgroups_in_plot(xg);

    xg->delete_erased_pts = True;
    reset_rows_in_plot(xg, True);

    assign_points_to_bins(xg);
    plot_once(xg);
  }
}

void
brush_on(xgobidata *xg)
{
/*
 *  If this mode is currently selected, turn it off.
*/
  if (xg->prev_plot_mode == BRUSH_MODE && xg->plot_mode != BRUSH_MODE)
  {
    XtDisownSelection( (Widget) xg->workspace,
      (Atom) XG_NEWPAINT,
      (Time) XtLastTimestampProcessed(display));
      /*(XtConvertSelectionProc) pack_brush_data );*/

    xg->is_brushing = False;
    /* Remove event handler for the workspace widget. */
    XtRemoveEventHandler(xg->workspace, XtAllEvents,
      TRUE, (XtEventHandler) brush_button, (XtPointer) xg);
    XtRemoveEventHandler(xg->workspace, XtAllEvents,
      TRUE, (XtEventHandler) brush_motion, (XtPointer) xg);

    /*
     * If we've been using transient brushing, restore
     * all points colors and glyphs to their persistent state.
    if (xg->brush_mode == transient) {
      int j;
      for (j=0; j<xg->nrows_in_plot; j++) {
        xg->glyph_now[j].type = xg->glyph_ids[j].type;
        xg->glyph_now[j].size = xg->glyph_ids[j].size;
        xg->color_now[j] = xg->color_ids[j];
      }
    }
    */  /* No, don't. */

    map_brush(xg, False);
    plot_once(xg);
  }
  else if (xg->prev_plot_mode != BRUSH_MODE &&
           xg->plot_mode == BRUSH_MODE)
  {
    /* Add event handlers for the workspace widget.  */
    XtAddEventHandler(xg->workspace,
      ButtonPressMask | ButtonReleaseMask,
      FALSE, (XtEventHandler) brush_button, (XtPointer) xg);
    XtAddEventHandler(xg->workspace,
      Button1MotionMask | Button2MotionMask ,
      FALSE, (XtEventHandler) brush_motion, (XtPointer) xg);

    xg->is_brushing = True;
    map_brush(xg, True);

    if (xg->reshape_brush)
      init_brush_size(xg);
    draw_brush(xg);

    assign_points_to_bins(xg);

    /*
     * The current transient brushing behavior is as follows:
     * the selected points remain selected when leaving brushing.
     * When re-entering brushing (or when selecting a new
     * variable; see varselect() for very similar code), the
     * brushing operations are performed so that the initial
     * state matches what's under the brush.  This is necessary
     * here to cover two cases:  a change of projection outside
     * of brushing (either through change of mode or variable,
     * or a transformation), and if reshape_brush is true.
    */ 
    if (xg->brush_mode == transient) {
      int j, k;
      for (j=0; j<xg->nrows_in_plot; j++) {
        k = xg->rows_in_plot[j];
        xg->color_now[k] = xg->color_ids[k] ;
        xg->glyph_now[k].type = xg->glyph_ids[k].type;
        xg->glyph_now[k].size = xg->glyph_ids[k].size;
      }
      brush_once(xg, False);
      plot_once(xg);
    }
  /* */

    copy_brushinfo_to_senddata(xg);

    if (xg->is_tour_section)
      turn_off_section_tour(xg);
  }
}

#undef NAMESIZE
#undef NAMESV
#undef SMALL
#undef MEDIUM
#undef LARGE
