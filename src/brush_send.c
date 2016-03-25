/* brush_send.c */
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

/* Send vector for linked line brushing */
static unsigned long *lbsenddata;

int
int_compare(const void *val1, const void *val2)
{
  register int *left = (int *) val1;
  register int *right = (int *) val2;

  if (*left < *right) return (-1);
  else if (*left > *right) return (1);
  else return  (0);
}

void
copy_brushinfo_to_senddata(xgobidata *xg)
{
/*
 * Fill the vector that is used in linked brushing.  This
 * routine is called when brushing is turned on and will
 * need to be called whenever there is a change in
 * xg->rows_in_plot[].  This same vector is used to transfer
 * all the brushing data or just the rows_in_plot[] vector.
*/
  int i, j, k, nr;
  static int nrows = 0;

  nr = (xg->nrgroups) ? xg->nrgroups_in_plot : xg->nlinkable_in_plot;
  
  /*
   * Only reallocate senddata when nrows_in_plot has changed.
  */
  if (nrows != nr) {
    xg->senddata = (unsigned long *) XtRealloc((XtPointer) xg->senddata,
      (Cardinal) (9 + 2*nr) * sizeof(unsigned long) );
    nrows = nr;
  }

  xg->senddata[2] = (unsigned long) xg->nrgroups;  /* not used */
  xg->senddata[3] = (unsigned long) xg->link_points_to_points;
  xg->senddata[4] = (unsigned long) xg->link_points_to_lines;
  xg->senddata[5] = (unsigned long) xg->link_color_brushing;
  xg->senddata[6] = (unsigned long) xg->link_glyph_brushing;
  xg->senddata[7] = (unsigned long) xg->link_erase_brushing;
  xg->senddata[8] = (unsigned long) (xg->brush_mode ? True : False);

  if (xg->nrgroups > 0) {
    xg->senddata[0] = (unsigned long) xg->nrgroups;
    xg->senddata[1] = (unsigned long) xg->nrgroups_in_plot;

    for (i=0, j=9; i<xg->nrgroups && j<9+xg->nrgroups_in_plot; i++) {
      if (!xg->rgroups[i].excluded) {
        xg->senddata[j] = (unsigned long) i;
        xg->senddata[j + xg->nrgroups_in_plot] = (unsigned long)
          glyph_color_pointtype(xg, xg->rgroups[i].els[0]);
        j++;
      }
    }

  } else {

    xg->senddata[0] = (unsigned long) xg->nlinkable;
    xg->senddata[1] = (unsigned long) xg->nlinkable_in_plot;

    /* nr: xg->nlinkable_in_plot */
    for (i=0, j=9; i<nr; i++, j++) {
      k = xg->rows_in_plot[i];
      xg->senddata[j] = (unsigned long) k;
      xg->senddata[j + nr] = (unsigned long) glyph_color_pointtype(xg, k);
    }
  }
}

int
glyph_color_pointtype(xgobidata *xg, int rownum)
{
  int i;
  int pointtype = -1;
/*
 * fold the color and glyph information into a single
 * identifier.  in this way, 4 times as much brushing information
 * can be sent.
*/

  if (mono) {
    pointtype = 1000 * (xg->glyph_now[rownum].size - 1) +
                 100 * (xg->glyph_now[rownum].type - 1) +
             ncolors * xg->erased[rownum];
  }
  else {
    for (i=0; i<ncolors; i++) {
      if (xg->color_now[rownum] == color_nums[i]) {
        pointtype = 1000 * (xg->glyph_now[rownum].size - 1) +
                     100 * (xg->glyph_now[rownum].type - 1) +
                 ncolors * xg->erased[rownum] +
                           i ;
      }
    }
  }

  return(pointtype);
}

static void
senddata_to_linecolors (xgobidata *xg, long *pointtypes, int *rownums, int npts)
{
  int n, rownum, pointtype;
  int remainder;

  if (!mono) {
    for (n=0; n<npts; n++) {
      rownum = rownums[n];
      if (rownum >= xg->nlines)
        break;

      pointtype = (int) pointtypes[n];

      remainder = pointtype % 1000 ;
      remainder = remainder % 100 ;
      if (remainder >= ncolors)
        fprintf(stderr, "remainder = %d\n", remainder);
      xg->line_color_now[rownum] = color_nums[remainder] ;
    }
  }
}

static void
senddata_to_glyphs_and_colors (xgobidata *xg, long *pointtypes, int *rownums,
int npts, int sending_colors, int sending_glyphs, int sending_erase)
{
  int n, rownum, pointtype;
  int remainder;

/*
 * If the input data is rgroup data, then the rownums here are
 * actually rgroup_ids rather than row numbers.
*/

  for (n=0; n<npts; n++) {  /* incoming nrows_in_plot or nrgroups_in_plot */
    rownum = rownums[n];
    if (xg->nrgroups)
      rownum = xg->rgroups[xg->rgroup_ids[rownum]].els[0];

    if (rownum >= xg->nrows_in_plot || rownum >= xg->nlinkable)
      break;

    pointtype = (int) pointtypes[n];

    if (sending_glyphs && xg->link_glyph_brushing)
    {
      xg->glyph_now[rownum].size = pointtype / 1000 + 1 ;
      if (xg->glyph_now[rownum].size < 1 ||
          xg->glyph_now[rownum].size > NGLYPHSIZES)
      {
        (void) fprintf(stderr, "impossible point size %d in row %d\n",
          xg->glyph_now[rownum].size, rownum);
      }
    }

    remainder = pointtype % 1000 ;

    if (sending_glyphs && xg->link_glyph_brushing)
    {
      xg->glyph_now[rownum].type = remainder / 100 + 1 ;
      if (xg->glyph_now[rownum].type < 1 ||
          xg->glyph_now[rownum].type > NGLYPHTYPES)
      {
        (void) fprintf(stderr, "impossible point type %d in row %d\n",
          xg->glyph_now[rownum].type, rownum);
      }
    }

    remainder = remainder % 100 ;
    if (sending_erase && xg->link_erase_brushing)
      xg->erased[rownum] = remainder / ncolors ;

    if (!mono && sending_colors && xg->link_color_brushing) {
      remainder = remainder % ncolors ;
      if (remainder >= ncolors)
        fprintf(stderr,
          "Trouble: not this many colors - %d\n", remainder);
      xg->color_now[rownum] = color_nums[remainder] ;
    }

/*
 * 12/2/98  Implement linked brushing between a
 * single xgobi and a corresponding scatterplot matrix as
 * designed and built by Di.
*/
    if (xg->nrgroups) {
      int k, j, el0;
      int gp = rownum;
      el0 = xg->rgroups[gp].els[0];
      for (k=1; k<xg->rgroups[gp].nels; k++) {
        j = xg->rgroups[gp].els[k];
        xg->erased[j] = xg->erased[el0];
        if (!xg->erased[j]) {
          xg->color_now[j] = xg->color_now[el0];
          xg->glyph_now[j].type = xg->glyph_now[el0].type;
          xg->glyph_now[j].size = xg->glyph_now[el0].size;
        }
      }
    }
  }
}

/******** For glyph and color linking *******/

/*
 * All the XtSelectionDoneProcs and XtLoseSelectionProcs
 * do nothing; their purpose is to
 * prevent the Intrinsics from freeing the selection value.
*/
/* ARGSUSED */
XtSelectionDoneProc
pack_brush_done(Widget w, Atom *selection, Atom *target) { }

/* ARGSUSED  */
XtLoseSelectionProc
pack_brush_lose(Widget w, Atom *selection, XtPointer xgobi) { }

void
announce_brush_data(xgobidata *xg)
{
/*
 * This sends an event to tell other XGobi windows to
 * execute XtGetSelection().  It sends no data.
*/
  XChangeProperty( display,
    RootWindowOfScreen(XtScreen(xg->shell)),
    XG_NEWPAINT_ANNC, XG_NEWPAINT_ANNC_TYPE,
    (int) 32, (int) PropModeReplace,
    (unsigned char *) NULL, 0);
}

/* ARGSUSED */
XtConvertSelectionProc
pack_brush_data(Widget w, Atom *selection, Atom *target,
Atom *type_ret, XtPointer *retdata, unsigned long *length_ret, int *format_ret)
/*
  w           owning widget, xg->workspace
  selection   XG_NEWPAINT
  target      XG_NEWPAINT_TYPE
  type_ret    XG_NEWPAINT_TYPE again
  retdata     brushing data itself
  length_ret  length of retdata
  format_ret  type of retdata
*/
{
/*
 * When another XGobi requests the brushing data from the server,
 * this function is executed by the active XGobi.  It provides
 * the brushing data to the requestor.
 * It is declared by the call to XtOwnSelection().
*/
  extern xgobidata xgobi;
  int nsend;

  if (*target == XG_NEWPAINT_TYPE)
  {
    *retdata = (XtPointer) xgobi.senddata ;

    nsend = (xgobi.nrgroups) ?
      xgobi.nrgroups_in_plot : xgobi.nlinkable_in_plot;

    *length_ret = (unsigned long) (2*nsend + 9) ;
    *type_ret = XG_NEWPAINT_TYPE ;
    *format_ret = (int) 32 ;
  }
}

/* ARGSUSED */
XtSelectionCallbackProc
unpack_brush_data(Widget w, XtPointer xgobiptr, Atom *atom, Atom *atom_type,
XtPointer retdata, unsigned long *lendata, int *fmt)
/*
  atom         should be XG_NEWPAINT
  atom_type    should be XG_NEWPAINT_TYPE
  fmt          should be 32
*/
{
  xgobidata *xg = (xgobidata *) xgobiptr;
  long i;
  int j, npts, trans, nr;
  int *brushed;
  unsigned long *rdata;
  int link_p2p, nrgroups, link_p2l;
  int sending_colors, sending_glyphs, sending_erase;
  Boolean doit = False;

  if (*atom == XG_NEWPAINT &&
      *atom_type &&
      *atom_type != XT_CONVERT_FAIL)
  {
    if (*lendata > 0) {
      rdata = (unsigned long *) retdata;

      nr = (int) *rdata++ ;  /* incoming nlinkable or nrgroups */
      npts = (int) *rdata++ ;  /* nlinkable_in_plot or nrgroups_in_plot */
      nrgroups = (int) *rdata++ ; /* don't care about this now */
      link_p2p = (int) *rdata++ ;
      link_p2l = (int) *rdata++ ;

      sending_colors = (int) *rdata++ ;
      sending_glyphs = (int) *rdata++ ;
      sending_erase = (int) *rdata++ ;
      trans = (int) *rdata++ ;

      /*
       * We already know that the receiver hasn't disabled linked
       * brushing for points (either xg->link_points_to_points or
       * xg->link_points_to_lines is true), otherwise read_paint
       * wouldn't have been called.
       * Let both the receiving value and the sending value together
       * (link_p2p or link_p2l)
       * determine whether to assign the colors to points or lines.
      */

      if (npts > 0) {
          if (xg->link_points_to_points && link_p2p) {
            if (nr == xg->nlinkable || nr == xg->nrgroups)
              doit = True;
          } else if (xg->link_points_to_lines && link_p2l && nr == xg->nlines)
            doit = True;
      }

      if (doit) {
        brushed = (int *) XtMalloc((Cardinal) npts * sizeof(int));

        /*
         * If linkp2p, use brushed to reset the values of
         *   xg->rows_in_plot
         *   xg->nrows_in_plot
         *   xg->excluded
         *
         *   rgroups_in_plot
         *   rgroups[].excluded
        */
        for (i=0; i<npts; i++) {
          brushed[i] = (int) *rdata++;  /* rows_in_plot or ngroups_in_plot */
        }

        if (link_p2p) {
          senddata_to_glyphs_and_colors(xg,
            (long *) rdata, brushed, npts,
            sending_colors, sending_glyphs, sending_erase);
        }
        else if (link_p2l) {
          senddata_to_linecolors(xg, (long *) rdata, brushed, npts);
        }

        /*
         * If the painting mode of the sending window is
         * persistent or undo, then make these changes permanent.
         * The changes aren't fully permanent unless they're in
         * senddata[], but I'll do that when a ButtonPress is
         * received, so it should be ok.
        */
        if (!trans) {
          if (xg->rgroups) {
            int k;
            rg_struct rg;
            for (i=0; i<npts; i++) {
              rg = xg->rgroups[ brushed[i] ];
              for (k=0; k<rg.nels; k++) {
                j = rg.els[k];
                if (link_p2p) {
                  if (sending_colors && xg->link_color_brushing)
                    xg->color_ids[j] = xg->color_now[j];
                  if (sending_glyphs && xg->link_color_brushing) {
                    xg->glyph_ids[j].type = xg->glyph_now[j].type;
                    xg->glyph_ids[j].size = xg->glyph_now[j].size;
                  }
                } else {
                  xg->line_color_ids[j] = xg->line_color_now[j];
                }
              }
            }

          } else {  /* no rgroups in use */

            for (i=0; i<npts; i++) {
              j = brushed[i];
              if (link_p2p) {
                if (sending_colors && xg->link_color_brushing)
                  xg->color_ids[j] = xg->color_now[j];
                if (sending_glyphs && xg->link_color_brushing) {
                  xg->glyph_ids[j].type = xg->glyph_now[j].type;
                  xg->glyph_ids[j].size = xg->glyph_now[j].size;
                }
              } else {
                xg->line_color_ids[j] = xg->line_color_now[j];
              }
            }
          }
        }
        XtFree((XtPointer) brushed);
        xg->got_new_paint = 1;
        plot_once(xg);

        if (xg->is_cprof_plotting)
          cprof_plot_once(xg);
      }
    }
  }
  XtFree((XtPointer) retdata);

  plot_once(xg);

#if defined RPC_USED || defined DCE_RPC_USED
  xfer_brushinfo(xg);
#endif
}

void
read_paint(xgobidata *xg)
{
    XtGetSelectionValue(
      xg->workspace,
      (Atom) XG_NEWPAINT,
      (Atom) XG_NEWPAINT_TYPE,
      (XtSelectionCallbackProc) unpack_brush_data,
      (XtPointer) xg,
      (Time) XtLastTimestampProcessed(display) );
      /*(Time) CurrentTime );*/
}

void
announce_rows_in_plot(xgobidata *xg)
{
/*
 * This sends an event to tell other XGobi windows to
 * execute XtGetSelection().  It sends no data.
*/
  XChangeProperty( display,
    RootWindowOfScreen(XtScreen(xg->shell)),
    XG_ROWSINPLOT_ANNC, XG_ROWSINPLOT_ANNC_TYPE,
    (int) 32, (int) PropModeReplace,
    (unsigned char *) NULL, 0);
}

/* ARGSUSED */
XtConvertSelectionProc
pack_rowsinplot_data(Widget w, Atom *selection, Atom *target,
Atom *type_ret, XtPointer *retdata, unsigned long *length_ret, int *format_ret)
/*
  w            owning widget, xg->workspace
  selection    XG_ROWSINPLOT
  target       XG_ROWSINPLOT_TYPE
  type_ret     XG_ROWSINPLOT_TYPE again
  retdata      brushing data itself
  length_ret   length of retdata
  format_ret   type of retdata
*/
{
/*
 * When another XGobi requests the brushing data from the server,
 * this function is executed by the active XGobi.  It provides
 * the rows_in_plot[] data to the requestor.
 * It is declared by the call to XtOwnSelection().
*/
  extern xgobidata xgobi;
  int nsend;

  if (*target == XG_ROWSINPLOT_TYPE) {
    /*
     * Send only the first part of senddata
    */
    *retdata = (XtPointer) xgobi.senddata ;
    nsend = (xgobi.nrgroups > 0) ?
      xgobi.nrgroups_in_plot : xgobi.nlinkable_in_plot;
    *type_ret = XG_ROWSINPLOT_TYPE ;
    *length_ret = (unsigned long) (nsend + 9) ;
    *format_ret = (int) 32 ;
  }
}

/* ARGSUSED */
XtSelectionCallbackProc
unpack_rowsinplot_data(Widget w, XtPointer xgobiptr, Atom *atom,
Atom *atom_type, XtPointer retdata, unsigned long *lendata, int *fmt)
/*
  atom         should be XG_ROWSINPLOT
  atom_type    should be XG_ROWSINPLOT_TYPE
  fmt          should be 32
*/
{
  xgobidata *xg = (xgobidata *) xgobiptr;
  int i, npts, nr;
  unsigned long *rdata;
  int *brushed;

  if (*atom == XG_ROWSINPLOT &&
      *atom_type &&
      *atom_type == XG_ROWSINPLOT_TYPE )
  {
    if (*lendata > 0) {
      rdata = (unsigned long *) retdata;

      nr = (int) *rdata++ ;    /* incoming nlinkable or nrgroups */
      npts = (int) *rdata++ ;  /* nlinkable_in_plot or nrgroups_in_plot */
      *rdata++ ;    /* skip over nrgroups */
      *rdata++ ;    /* skip over link_p2l */
      *rdata++ ;    /* skip over link_l2l */
      *rdata++ ;    /* skip over sending_colors */
      *rdata++ ;    /* skip over sending_glyphs */
      *rdata++ ;    /* skip over sending_erase */
      *rdata++ ;    /* skip over is_transient */

      if (npts > 0 && (nr == xg->nlinkable || nr == xg->nrgroups))
      {
        int m;
        brushed = (int *) XtMalloc((Cardinal) npts * sizeof(int));
        for (i=0; i<npts; i++)
          brushed[i] = (int) *rdata++;  /* rows_in_plot or groups_in_plot */
        
        xg->nrows_in_plot = 0;

        if (xg->rgroups) {
          int k, el;
          rg_struct rg;
          int *rtmp = (int *) XtMalloc(xg->nlinkable * sizeof(int));

          for (m=0, i=0; m<npts; m++) {     /* i is the rgroup index */
            while (i < brushed[m]) {
              xg->rgroups[i].excluded = true;
              if (++i >= xg->nrgroups) break;
            }
            rg = xg->rgroups[i];
            for (k=0; k<rg.nels; k++) {
              xg->excluded[ el = rg.els[k] ] = false;
              rtmp[xg->nrows_in_plot++] = el;
            }
          }

          /*
           * It seems to be necessary to sort rows_in_plot; I don't
           * feel like figuring out why
          */
          qsort((char *) rtmp, xg->nrows_in_plot, sizeof(int), int_compare);
          for (i=0; i<xg->nrows_in_plot; i++)
            xg->rows_in_plot[i] = rtmp[i];

          XtFree((char *) rtmp);
        }
        else
        {
          for (m=0, i=0; m<npts; m++) {     /* i is the row index */
            while (i < brushed[m]) {
              xg->excluded[i] = true;
              if (++i >= xg->nlinkable) break;
            }
            xg->excluded[i] = false;
            xg->rows_in_plot[xg->nrows_in_plot++] = i;
            if (++i >= xg->nlinkable) break;
          }

          for (i=brushed[npts-1]; i<xg->nlinkable; i++)
            xg->excluded[i++] = true;
        }
          
        for (i=xg->nlinkable; i<xg->nrows; i++)
          if (!xg->excluded[i]) xg->rows_in_plot[xg->nrows_in_plot++] = i;

        update_nrgroups_in_plot(xg);

        /*
         * This could become a parameter that is passed from
         * the linked xgobi
        */
        reset_rows_in_plot(xg, True);
        /*reset_rows_in_plot(xg, False);*/
        plot_once(xg);

        /* I'm not sure if this belongs here, but I bet it does */
        passive_update_cprof_plot(xg);
      }
    }
  }
  XtFree((XtPointer) retdata);
}

void
read_rows_in_plot(xgobidata *xg)
{
    XtGetSelectionValue(
      xg->workspace,
      (Atom) XG_ROWSINPLOT,
      (Atom) XG_ROWSINPLOT_TYPE,
      (XtSelectionCallbackProc) unpack_rowsinplot_data,
      (XtPointer) xg,
      (Time) XtLastTimestampProcessed(display) );
      /*(Time) CurrentTime );*/
}

/************* Section for linked line brushing *******************/

/* ARGSUSED */
XtSelectionDoneProc
pack_line_brush_done(Widget w, Atom *selection, Atom *target) { }

/* ARGSUSED  */
XtLoseSelectionProc
pack_line_brush_lose(Widget w, Atom *selection, XtPointer xgobi) { }

void
announce_line_brush_data(xgobidata *xg)
{
/*
 * This sends an event to tell other XGobi windows to
 * execute XtGetSelection().  It sends no data.
*/
  XChangeProperty( display,
    RootWindowOfScreen(XtScreen(xg->shell)),
    XG_NEWLINEPAINT_ANNC, XG_NEWLINEPAINT_ANNC_TYPE,
    (int) 32, (int) PropModeReplace,
    (unsigned char *) NULL, 0);
}

/* ARGSUSED */
XtConvertSelectionProc
pack_line_brush_data(Widget w, Atom *selection, Atom *target,
Atom *type_ret, XtPointer *retdata, unsigned long *length_ret, int *format_ret)
/*
  w           owning widget, xg->workspace
  selection   XG_NEWLINEPAINT
  target      XG_NEWLINEPAINT_TYPE
  type_ret    XG_NEWLINEPAINT_TYPE again
  retdata     brushing data itself
  length_ret  length of retdata
  format_ret  type of retdata
*/
{
/*
 * When another XGobi requests the brushing data from the server,
 * this function is executed by the active XGobi.  It provides
 * the brushing data to the requestor.
 * It is declared by the call to XtOwnSelection().
*/
  extern xgobidata xgobi;
  static Boolean initd = False;
  static int nlinkable = 0;
  static int nlines = 0;
  int k;

  if (!initd || nlinkable != xgobi.nlinkable || nlines != xgobi.nlines)
  {
    nlinkable = xgobi.nlinkable;
    nlines = xgobi.nlines;
    lbsenddata = (unsigned long *) XtRealloc((char *) lbsenddata, 
      (Cardinal) (5 + nlines) * sizeof(unsigned long));
    initd = True;
  }

  if (*target == XG_NEWLINEPAINT_TYPE)
  {
    lbsenddata[0] = (unsigned long) xgobi.nlinkable;
    lbsenddata[1] = (unsigned long) xgobi.nlines;
    lbsenddata[2] = (unsigned long) xgobi.link_lines_to_lines;
    lbsenddata[3] = (unsigned long) xgobi.link_points_to_lines;
    if (xgobi.brush_mode == transient)
      lbsenddata[4] = (unsigned long) True;
    else
      lbsenddata[4] = (unsigned long) False;

    /*
     * and now pack up the line colors themselves.
    */
    for (k=0; k<xgobi.nlines; k++)
      lbsenddata[5+k] = xgobi.line_color_now[k];

    *type_ret = XG_NEWLINEPAINT_TYPE ;
    *retdata = (XtPointer) lbsenddata ;
    *length_ret = (unsigned long) (xgobi.nlines + 5) ;
    *format_ret = (int) 32 ;
  }
}

/* ARGSUSED */
XtSelectionCallbackProc
unpack_line_brush_data(Widget w, XtPointer xgobiptr, Atom *atom,
Atom *atom_type, XtPointer retdata, unsigned long *lendata, int *fmt)
/*
  atom         should be XG_NEWLINEPAINT
  atom_type    should be XG_NEWLINEPAINT_TYPE
  fmt          should be 32
*/
{
  xgobidata *xg = (xgobidata *) xgobiptr;
  long i;
  int trans;
  int link_l2l, link_p2l;
  int nlines, nlinkable;
  unsigned long *rdata;

  if (*atom == XG_NEWLINEPAINT &&
      *atom_type &&
      *atom_type != XT_CONVERT_FAIL)
  {
    if (*lendata > 0)
    {
      rdata = (unsigned long *) retdata;

      nlinkable = (int) *rdata++ ;
      nlines = (int) *rdata++ ;
      link_l2l = (int) *rdata++ ;
      link_p2l = (int) *rdata++ ;
      trans = (int) *rdata++ ;

      /*
       * We already know that the receiver hasn't disabled linked
       * brushing for lines (either xg->link_points_to_points or
       * xg->link_lines_to_lines is true), otherwise read_line_paint
       * wouldn't have been called.
       * Let both the receiving value and the sending value together
       * (link_l2l or link_p2l)
       * determine whether to assign the colors to points or lines.
      */

      if (xg->link_lines_to_lines && link_l2l &&
          nlines > 0 && nlines == xg->nlines)
      {
        /*
         * If the painting mode of the sending window is
         * persistent or undo, then make these changes permanent.
        */

        if (trans)
          for (i=0; i<nlines; i++)
            xg->line_color_now[i] = (unsigned long) *rdata++ ;
        else
          for (i=0; i<nlines; i++)
            xg->line_color_ids[i] = xg->line_color_now[i] =
              (unsigned long) *rdata++ ;
      } 
      else if (xg->link_points_to_lines && link_p2l &&
               nlines > 0 && nlines == xg->nrows)
      {
        if (trans)
          for (i=0; i<nlines; i++)
            xg->color_now[i] = (unsigned long) *rdata++ ;
        else
          for (i=0; i<nlines; i++)
            xg->color_ids[i] = xg->color_now[i] =
              (unsigned long) *rdata++ ;
      }

      xg->got_new_paint = 1;
      plot_once(xg);
    }
  }
  if (retdata != NULL)
    XtFree((XtPointer) retdata);
}

void
read_line_paint(xgobidata *xg)
{
    XtGetSelectionValue(
      xg->workspace,
      (Atom) XG_NEWLINEPAINT,
      (Atom) XG_NEWLINEPAINT_TYPE,
      (XtSelectionCallbackProc) unpack_line_brush_data,
      (XtPointer) xg,
      (Time) XtLastTimestampProcessed(display) );
      /*(Time) CurrentTime );*/
}
