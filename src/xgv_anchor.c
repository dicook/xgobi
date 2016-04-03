/* xgv_anchor.c */
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
#include <limits.h>
#include <float.h> 
#include <math.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include "DrawingA.h"
#include "xgvis.h"

static void xgv_shift_clust_widgets(xgobidata *);
static void xgv_alloc_clust(xgobidata *);
static void xgv_addGroup(int, Widget, xgobidata *);
static void xgv_draw_cluster_symbol(xgobidata *, int);
void v_mark_cluster_symbol(xgobidata *, int, int);
static void xgv_create_clust_widgets(xgobidata *);
XtCallbackProc xgv_reset_clusters_cback (Widget, xgobidata *, XtPointer);
XtCallbackProc xgv_open_anchor_popup_cback(Widget, xgobidata *, XtPointer);
static XtCallbackProc xgv_symbol_reset_cb(Widget, xgobidata *, XawDrawingAreaCallbackStruct *);
static XtCallbackProc xgv_anchor_cback(Widget, xgobidata *, XtPointer);
static XtCallbackProc xgv_close_cback(Widget, xgobidata *, XtPointer);
static XtCallbackProc xgv_symbol_expose_cb(Widget, xgobidata *, XawDrawingAreaCallbackStruct *);
static XtCallbackProc xgv_exclude_cback(Widget, xgobidata *, XtPointer);
static XtCallbackProc xgv_hide_cback(Widget, xgobidata *, XtPointer);

extern void save_group_names(xgobidata *);
extern void save_brush_groups(xgobidata *);
extern XtCallbackProc set_mds_group_cback(Widget, XtPointer, XtPointer);

extern int nclust_prev;
extern cluster *clusv_prev;
extern Widget group_menu_btn[];
extern Widget epopup;

Widget xgv_collbl[5];
Widget xgv_epane, xgv_epopup = NULL, xgv_eform = NULL;
Widget *xgv_names, *xgv_symbols, *xgv_hide_tgl, *xgv_exclude_tgl, *xgv_anchor_tgl;
Position xgv_popupx = -1, xgv_popupy = -1;


/*
 * Before going down, save the group names
*/
static void
xgv_save_group_names(xgobidata *xg) {
  int k;
  String gname;
  char refstr[32];
  if (xg->nclust > 1) {
    for (k=0; k<xg->nclust; k++) {
      sprintf(refstr, "Group %d", k+1);
      XtVaGetValues(xgv_names[k], XtNstring, &gname, NULL);
      if (strcmp(refstr, gname) != 0)
        strcpy(xg->clusv[k].name, gname);
      else
        sprintf(xg->clusv[k].name, "");
    }
  }
}

static void
xgv_create_clust_widgets(xgobidata *xg) {
  int i;

  save_brush_groups (xg);

  if (xg->nclust > 1) {
    xgv_alloc_clust(xg);

    for (i=0; i<xg->nclust; i++)
      xgv_addGroup(i, xgv_eform, xg);

    XtManageChildren(xgv_hide_tgl,    xg->nclust);
    XtManageChildren(xgv_exclude_tgl, xg->nclust);
    XtManageChildren(xgv_anchor_tgl,  xg->nclust);
  } else {

    (void) XtVaCreateManagedWidget("HideOrExclude",
      labelWidgetClass, xgv_eform,
      XtNlabel, "*** There's just one group; all symbols are alike ***",
      XtNfromVert, xgv_collbl[0],
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      NULL);
  }
}

/* ARGSUSED */
static XtCallbackProc
xgv_hide_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i, k;

  for (k=0; k<xg->nclust; k++) {
    if (w == xgv_hide_tgl[k]) {
      xg->clusv[k].hidden = !xg->clusv[k].hidden;
      break;
    }
  }

  for (i=0; i<xg->nrows; i++)
    if ((int)GROUPID(i) == k)
      xg->erased[i] = xg->clusv[k].hidden;

  plot_once(xg);

  /* Update parallel coordinates plot? */
  if (xg->is_cprof_plotting && xg->link_cprof_plotting)
    cprof_plot_once(xg);

  setToggleBitmap(w, xg->clusv[k].hidden);

  copy_brushinfo_to_senddata(xg);
  XtOwnSelection( (Widget) xg->workspace,
    (Atom) XG_NEWPAINT,
    (Time) CurrentTime, /* doesn't work with XtLastTimeStamp...? */
    (XtConvertSelectionProc) pack_brush_data,
    (XtLoseSelectionProc) pack_brush_lose ,
    (XtSelectionDoneProc) pack_brush_done );
  announce_brush_data(xg);
}

static void
xgv_mark_cluster_symbol(xgobidata *xg, int k, int yes) {
/*
 * Draw the current glyph in the current color in the
 * glyph_wksp.  If no glyph is chosen, draw a border.
*/
  Drawable gwin = XtWindow(xgv_symbols[k]);
  int type = xg->clusv[k].glyphtype;
  int size = xg->clusv[k].glyphsize;
  icoords xypos[1];

  /* Clear it */
  XFillRectangle(display, gwin, clear_GC, 0, 0, 40, 40);

/*
 * The center is at 11, 11 ...   (How that can be?)
*/
  xypos[0].x = xypos[0].y = 11;

  /* Set the foreground color */
  if (!mono) {
    XSetForeground(display, copy_GC,
      (xg->clusv[k].color == plotcolors.bg) ? plotcolors.fg :
        xg->clusv[k].color);
  }

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

  if (!mono) {
    if (xg->clusv[k].color == plotcolors.bg)
      XDrawLine(display, gwin, copy_GC, 0, 0, 22, 22);
  }

  /* this is the marking part */
  if(yes) {
    XDrawLine(display, gwin, copy_GC, 0, 0, 0, 20);
    XDrawLine(display, gwin, copy_GC, 0,20,20, 20);
    XDrawLine(display, gwin, copy_GC,20,20,20,  0);
    XDrawLine(display, gwin, copy_GC,20, 0, 0,  0);
  }

  XFlush(display);
  XSync(display, False);
}

static void
xgv_draw_cluster_symbol(xgobidata *xg, int k) {
/*
 * Draw the current glyph in the current color in the
 * glyph_wksp.  If no glyph is chosen, draw a border.
*/
  Drawable gwin = XtWindow(xgv_symbols[k]);
  int type = xg->clusv[k].glyphtype;
  int size = xg->clusv[k].glyphsize;
  icoords xypos[1];

  /* Clear it */
  XFillRectangle(display, gwin, clear_GC, 0, 0, 40, 40);

/*
 * The center is at 11, 11 ...   (How that can be?)
*/
  xypos[0].x = xypos[0].y = 11;

  /* Set the foreground color */
  if (!mono) {
    XSetForeground(display, copy_GC,
      (xg->clusv[k].color == plotcolors.bg) ? plotcolors.fg :
        xg->clusv[k].color);
  }

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

  if (!mono) {
    if (xg->clusv[k].color == plotcolors.bg)
      XDrawLine(display, gwin, copy_GC, 0, 0, 22, 22);
  }


  XFlush(display);
  XSync(display, False);
}

/* ARGSUSED */
static XtCallbackProc
xgv_exclude_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i, k;
  Boolean excluded;

  for (k=0; k<xg->nclust; k++) {
    if (w == xgv_exclude_tgl[k]) {
      xg->clusv[k].excluded = !xg->clusv[k].excluded;
      setToggleBitmap(w, xg->clusv[k].excluded);
      break;
    }
  }

  xg->nrows_in_plot = 0;
  for (i=0; i<xg->nrows; i++) {
    xg->excluded[i] = xg->clusv[(int)GROUPID(i) ].excluded;
    if (!xg->excluded[i])
      xg->rows_in_plot[(xg->nrows_in_plot)++] = i;
  }

  update_nrgroups_in_plot(xg);
  reset_rows_in_plot(xg, True);

  assign_points_to_bins(xg);
  plot_once(xg);

  /* Update parallel coordinates plot? */
  if (xg->is_cprof_plotting && xg->link_cprof_plotting)
    cprof_plot_once(xg);

  copy_brushinfo_to_senddata(xg);
  XtOwnSelection( (Widget) xg->workspace,
    (Atom) XG_ROWSINPLOT,
    (Time) CurrentTime,
    (XtConvertSelectionProc) pack_rowsinplot_data,
    (XtLoseSelectionProc) pack_brush_lose,
    (XtSelectionDoneProc) pack_brush_done );
  announce_rows_in_plot(xg);

  XtOwnSelection( (Widget) xg->workspace,
    (Atom) XG_NEWPAINT,
    (Time) CurrentTime,
    (XtConvertSelectionProc) pack_brush_data,
    (XtLoseSelectionProc) pack_brush_lose ,
    (XtSelectionDoneProc) pack_brush_done );
  announce_brush_data(xg);
}

/* ARGSUSED */
static XtCallbackProc
xgv_symbol_expose_cb(Widget w, xgobidata *xg, XawDrawingAreaCallbackStruct *cdata)
{
  int k;
  for (k=0; k<xg->nclust; k++) {
    if (w == xgv_symbols[k]) {
      xgv_draw_cluster_symbol(xg, k);
      break;
    }
  }
}

/* ARGSUSED */
static XtCallbackProc
xgv_close_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  xgv_save_group_names(xg);

  XtDestroyWidget(xgv_epopup);
  xgv_epopup = NULL;
}

/* ARGSUSED */
static XtCallbackProc
xgv_anchor_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k;

  for (k=0; k<xg->nclust; k++) {
    if (w == xgv_anchor_tgl[k]) {
      anchor_group[k] = !anchor_group[k];
      setToggleBitmap(w, anchor_group[k]);
      if(mds_group_ind != anchorscales && mds_group_ind != anchorfixed)
	set_mds_group_cback(group_menu_btn[3], NULL, NULL);
      break;
    }
  }
}


/* ARGSUSED */
static XtCallbackProc
xgv_symbol_reset_cb(Widget w, xgobidata *xg, XawDrawingAreaCallbackStruct *cdata)
{
  int i, k, gno;
  XEvent *event = cdata->event;

  if (event->type == ButtonRelease) {

    XButtonEvent *xbutton = (XButtonEvent *) cdata->event;
    int button = xbutton->button;
    int state = xbutton->state;

    for (k=0; k<xg->nclust; k++) {
      if (w == xgv_symbols[k]) {
        gno = k;
        break;
      }
    }

/*
 * On my machine, state == 8 is not a meaningful test.  This
 * is what would work:                                   dfs
*/
    printf("%d %d %d %d\n", button, state, Button1Mask|Mod1Mask, gno);

    if (button == 1 && state != 8) {

      if (xg->is_color_painting) {
        xg->clusv[gno].color_prev = xg->clusv[gno].color;
        xg->clusv[gno].color = xg->color_id;
        XtVaSetValues(xgv_names[gno], XtNforeground, xg->color_id, NULL);
      }
      if (xg->is_glyph_painting) {
        xg->clusv[gno].glyphtype_prev = xg->clusv[gno].glyphtype;
        xg->clusv[gno].glyphtype = xg->glyph_id.type;
        xg->clusv[gno].glyphsize_prev = xg->clusv[gno].glyphsize;
        xg->clusv[gno].glyphsize = xg->glyph_id.size;
      }

      for (i=0; i<xg->nrows; i++) {
        if ((int)GROUPID(i) == gno) {
          if (xg->is_color_painting)
            xg->color_ids[i] = xg->color_now[i] = xg->color_id;
          if (xg->is_glyph_painting) {
            xg->glyph_ids[i].type = xg->glyph_now[i].type = xg->glyph_id.type;
            xg->glyph_ids[i].size = xg->glyph_now[i].size = xg->glyph_id.size;
          }
        }
      }
    } else if (button == 2 || (button == 1 && state == 8)) {  

      /* undo */
      /*
      if (xg->is_color_painting) {
        xg->clusv[gno].color = xg->clusv[gno].color_prev;
        XtVaSetValues(xgv_names[gno], XtNforeground, xg->clusv[gno].color, NULL);
      }
      if (xg->is_glyph_painting) {
        xg->clusv[gno].glyphtype = xg->clusv[gno].glyphtype_prev;
        xg->clusv[gno].glyphsize = xg->clusv[gno].glyphsize_prev;
      }

      for (i=0; i<xg->nrows; i++) {
        if ((int)GROUPID(i) == gno) {
          if (xg->is_color_painting) {
            xg->color_ids[i] = xg->color_now[i] = xg->clusv[gno].color;
          }
          if (xg->is_glyph_painting) {
            xg->glyph_ids[i].type = xg->glyph_now[i].type =
              xg->clusv[gno].glyphtype;
            xg->glyph_ids[i].size = xg->glyph_now[i].size =
              xg->clusv[gno].glyphsize;
          }
        }
      }
      */
    }


    if (button == 1) {
      xgv_draw_cluster_symbol(xg, gno);
    }
    if (button == 2) {
      xgv_mark_cluster_symbol(xg, gno, 1);
    }

    plot_once(xg);

    /* Update parallel coordinates plot? */
    if (xg->is_cprof_plotting && xg->link_cprof_plotting)
      cprof_plot_once(xg);

    copy_brushinfo_to_senddata(xg);

    if (xg->is_point_painting) {
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
}

static void
xgv_addGroup(int k, Widget parent, xgobidata *xg) {
  Widget vref = (k==0) ? xgv_collbl[0] : xgv_symbols[k-1];
  int i;
  Boolean preserve_name = False;

  if (nclust_prev > 1) {
    for (i=0; i<nclust_prev; i++) {
      if (xg->clusv[k].glyphtype == clusv_prev[i].glyphtype &&
          xg->clusv[k].glyphsize == clusv_prev[i].glyphsize &&
          xg->clusv[k].color == clusv_prev[i].color &&
          strcmp(clusv_prev[i].name, "") != 0)
      {
        strcpy(xg->clusv[k].name, clusv_prev[i].name);
        preserve_name = True;
        break;
      }
    }
  }
  if (!preserve_name) sprintf(xg->clusv[k].name, "Group %d", k+1);

  xgv_names[k] = XtVaCreateManagedWidget("HideOrExclude",
    asciiTextWidgetClass, parent,
    XtNfromVert, vref,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNresizable, (Boolean) False,
    XtNeditType, (int) XawtextEdit,
    XtNstring, (String) xg->clusv[k].name,
    XtNforeground,
    (xg->clusv[k].color == plotcolors.bg) ? plotcolors.fg : xg->clusv[k].color,
    XtNbackground, plotcolors.bg,
    NULL);
  if (mono) set_mono(xgv_names[k]);

  xgv_symbols[k] = XtVaCreateManagedWidget("HideOrExclude",
    drawingAreaWidgetClass, parent,
    XtNresizable, (Boolean) False,
    XtNfromHoriz, xgv_collbl[0],
    XtNfromVert, vref,
    XtNbackground, plotcolors.bg,
    XtNwidth, 22,
    XtNheight, 22,
    NULL);
  if (mono) set_mono(xgv_symbols[k]);
  XtAddCallback(xgv_symbols[k], XtNexposeCallback,
    (XtCallbackProc) xgv_symbol_expose_cb, (XtPointer) xg);
  XtAddCallback(xgv_symbols[k], XtNinputCallback,
    (XtCallbackProc) xgv_symbol_reset_cb, (XtPointer) xg);

  xgv_hide_tgl[k] = CreateToggle(xg, "", True,
    xgv_collbl[1], vref, (Widget) NULL,
    xg->clusv[k].hidden, ANY_OF_MANY, parent, "HideOrExclude");
  XtAddCallback(xgv_hide_tgl[k], XtNcallback,
    (XtCallbackProc) xgv_hide_cback, (XtPointer) xg);
    
  xgv_exclude_tgl[k] = CreateToggle(xg, "", True,
    xgv_collbl[2], vref, (Widget) NULL,
    xg->clusv[k].excluded, ANY_OF_MANY, parent, "HideOrExclude");
  XtAddCallback(xgv_exclude_tgl[k], XtNcallback,
    (XtCallbackProc) xgv_exclude_cback, (XtPointer) xg);

  xgv_anchor_tgl[k] = CreateToggle(xg, "", True,
    xgv_collbl[3], vref, (Widget) NULL,
    anchor_group[k], ANY_OF_MANY, parent, "HideOrExclude");
  XtAddCallback(xgv_anchor_tgl[k], XtNcallback,
    (XtCallbackProc) xgv_anchor_cback, (XtPointer) xg);
}

static void
xgv_alloc_clust(xgobidata *xg) {
  xgv_names = (Widget *)
    XtRealloc((XtPointer)xgv_names, xg->nclust * sizeof(Widget));
  xgv_symbols = (Widget *)
    XtRealloc((XtPointer)xgv_symbols, xg->nclust * sizeof(Widget));
  xgv_hide_tgl = (Widget *)
    XtRealloc((XtPointer)xgv_hide_tgl, xg->nclust * sizeof(Widget));
  xgv_exclude_tgl = (Widget *)
    XtRealloc((XtPointer)xgv_exclude_tgl, xg->nclust * sizeof(Widget));
  xgv_anchor_tgl = (Widget *)
    XtRealloc((XtPointer)xgv_anchor_tgl, xg->nclust * sizeof(Widget));
}

static void
xgv_shift_clust_widgets(xgobidata *xg) {
  int i, k;
  Dimension colwidth[5];
  Dimension cboxwidth;

  if (xg->nclust > 1) {

    for (i=0; i<5; i++)
      XtVaGetValues(xgv_collbl[i], XtNwidth, &colwidth[i], NULL);

    XtVaGetValues(xgv_hide_tgl[0], XtNwidth, &cboxwidth, NULL);

    for (k=0; k<xg->nclust; k++) {
      if (colwidth[1] > 22)
        XtVaSetValues(xgv_symbols[k],
          XtNhorizDistance, colwidth[1]/2, NULL);
      if (colwidth[2] > cboxwidth)
        XtVaSetValues(xgv_hide_tgl[k],   
          XtNhorizDistance, (colwidth[2]-cboxwidth)/2, NULL);
      if (colwidth[3] > cboxwidth)
        XtVaSetValues(xgv_exclude_tgl[k],
          XtNhorizDistance, (colwidth[3]-cboxwidth)/2, NULL);
      if (colwidth[4] > cboxwidth)
        XtVaSetValues(xgv_anchor_tgl[k],
          XtNhorizDistance, (colwidth[4]-cboxwidth)/2, NULL);
    }
  }
}

/* ARGSUSED */
XtCallbackProc
xgv_open_anchor_popup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Widget reset_clusters_cmd, close, href;
  Widget form0;
  Dimension width, height;
  int i, k;
  static char *colname[] = {"Group name", "Symbol", "Hidden", "Excluded", "Anchor"};

  /* close the other popup from the tools menu if it exists */
  if(epopup != NULL) {
    save_group_names(xg);
    XtDestroyWidget(epopup);
    epopup = NULL;
  }

  /* this is dirty: assume there will never be more than 100 groups */
  if(anchor_group == NULL) {
    anchor_group = (Boolean *) XtMalloc(100 * sizeof(Boolean));
    for(i=0; i<100; i++) anchor_group[i] = FALSE;
  }

  if (xgv_epopup == NULL) {

    if (xgv_popupx == -1 && xgv_popupy == -1) {
      XtVaGetValues(xg->workspace,
        XtNwidth, &width,
        XtNheight, &height, NULL);
      XtTranslateCoords(xg->workspace,
        (Position) width, (Position) (height/2), &xgv_popupx, &xgv_popupy);
    }

    /*
     * Create the popup to solicit subsetting arguments.
    */
    xgv_epopup = XtVaCreatePopupShell("Exclusion",
      topLevelShellWidgetClass, xg->shell,
      XtNx, (Position) xgv_popupx,
      XtNy, (Position) xgv_popupy,
      XtNinput, (Boolean) True,
      XtNtitle, (String) "Hide, exclude, anchor groups",
      XtNiconName, (String) "Groups",
      NULL);
    if (mono) set_mono(xgv_epopup);

    /*
     * Create a paned widget so the 'Click here ...'
     * can be all across the bottom.
    */
    xgv_epane = XtVaCreateManagedWidget("Form0",
      panedWidgetClass, xgv_epopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      XtNresizable, False,
      NULL);

    reset_clusters_cmd = XtVaCreateManagedWidget("Close",
      commandWidgetClass, xgv_epane,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Reset brushed groups",
      NULL);
    XtAddCallback(reset_clusters_cmd, XtNcallback,
      (XtCallbackProc) xgv_reset_clusters_cback, (XtPointer) xg);

    form0 = XtVaCreateManagedWidget("Form",
      formWidgetClass, xgv_epane,
      XtNresizable, False,
      NULL);
    if (mono) set_mono(form0);
    /*
     * Create the form widget.
    */
    xgv_eform = XtVaCreateManagedWidget("Form",
      formWidgetClass, form0,
      XtNmappedWhenManaged, False,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(xgv_eform);

    /*
     * Add row of column names
    */
    for (i=0; i<5; i++) {
      href = (i==0) ? NULL : xgv_collbl[i-1];
      xgv_collbl[i] = XtVaCreateManagedWidget("HideOrExclude",
        labelWidgetClass, xgv_eform,
        XtNborderWidth, 0,
        XtNlabel, colname[i],
        XtNfromHoriz, href,
        NULL);
    }
    XtManageChildren(xgv_collbl, 5);

    xgv_create_clust_widgets(xg);

    close = XtVaCreateManagedWidget("Close",
      commandWidgetClass, xgv_epane,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close);
    XtAddCallback(close, XtNcallback,
      (XtCallbackProc) xgv_close_cback, (XtPointer) xg);

  }

  XtPopup(xgv_epopup, (XtGrabKind) XtGrabNone);
  set_wm_protocols(xgv_epopup);
  XRaiseWindow(display, XtWindow(xgv_epopup));

  XtMapWidget(xgv_eform);
  xgv_shift_clust_widgets (xg);
  if (xg->nclust > 1)
    for (k=0; k<xg->nclust; k++)
      XDefineCursor (display, XtWindow(xgv_symbols[k]), default_cursor);
}

/* ARGSUSED */
XtCallbackProc
xgv_reset_clusters_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  xgv_save_group_names (xg);

  XtVaGetValues(xgv_epopup,
    XtNx, &xgv_popupx,
    XtNy, &xgv_popupy,
    NULL);
  XtDestroyWidget(xgv_epopup);
  xgv_epopup = NULL;
  xgv_open_anchor_popup_cback ((Widget) NULL, xg, callback_data);
}
