/* exclusion.c */
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

Widget collbl[4];
Widget epane, epopup = NULL, eform = NULL;
Widget *names, *symbols, *hide_tgl, *exclude_tgl;
Position popupx = -1, popupy = -1;
int nclust_prev = 0;
cluster *clusv_prev;

static void
draw_cluster_symbol(xgobidata *xg, int k) {
/*
 * Draw the current glyph in the current color in the
 * glyph_wksp.  If no glyph is chosen, draw a border.
*/
  Drawable gwin = XtWindow(symbols[k]);
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

static void
mark_cluster_symbol(xgobidata *xg, int k, int yes) {
/*
 * Draw the current glyph in the current color in the
 * glyph_wksp.  If no glyph is chosen, draw a border.
*/
  Drawable gwin = XtWindow(symbols[k]);
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

/* ARGSUSED */
static XtCallbackProc
symbol_expose_cb(Widget w, xgobidata *xg, XawDrawingAreaCallbackStruct *cdata)
{
  int k;
  for (k=0; k<xg->nclust; k++) {
    if (w == symbols[k]) {
      draw_cluster_symbol(xg, k);
      break;
    }
  }
}

/*
 * Before going down, save the group names
*/
void
save_group_names(xgobidata *xg) {
  int k;
  String gname;
  char refstr[32];
  if (xg->nclust > 1) {
    for (k=0; k<xg->nclust; k++) {
      sprintf(refstr, "Group %d", k+1);
      XtVaGetValues(names[k], XtNstring, &gname, NULL);
      if (strcmp(refstr, gname) != 0)
        strcpy(xg->clusv[k].name, gname);
      else
        sprintf(xg->clusv[k].name, "");
    }
  }
}


/* ARGSUSED */
static XtCallbackProc
close_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  save_group_names(xg);

  XtDestroyWidget(epopup);
  epopup = NULL;
}

/* ARGSUSED */
static XtCallbackProc
hide_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i, k;

  for (k=0; k<xg->nclust; k++) {
    if (w == hide_tgl[k]) {
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

/* ARGSUSED */
static XtCallbackProc
exclude_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i, k;

  for (k=0; k<xg->nclust; k++) {
    if (w == exclude_tgl[k]) {
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
symbol_reset_cb(Widget w, xgobidata *xg, XawDrawingAreaCallbackStruct *cdata)
{
  int i, k, gno;
  XEvent *event = cdata->event;

  if (event->type == ButtonRelease) {

    XButtonEvent *xbutton = (XButtonEvent *) cdata->event;
    int button = xbutton->button;
    int state = xbutton->state;

    for (k=0; k<xg->nclust; k++) {
      if (w == symbols[k]) {
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
        XtVaSetValues(names[gno], XtNforeground, xg->color_id, NULL);
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
        XtVaSetValues(names[gno], XtNforeground, xg->clusv[gno].color, NULL);
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
      draw_cluster_symbol(xg, gno);
    }
    if (button == 2) {
      mark_cluster_symbol(xg, gno, 1);
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
addGroup(int k, Widget parent, xgobidata *xg) {
  Widget vref = (k==0) ? collbl[0] : symbols[k-1];
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

  names[k] = XtVaCreateManagedWidget("HideOrExclude",
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
  if (mono) set_mono(names[k]);

  symbols[k] = XtVaCreateManagedWidget("HideOrExclude",
    drawingAreaWidgetClass, parent,
    XtNresizable, (Boolean) False,
    XtNfromHoriz, collbl[0],
    XtNfromVert, vref,
    XtNbackground, plotcolors.bg,
    XtNwidth, 22,
    XtNheight, 22,
    NULL);
  if (mono) set_mono(symbols[k]);
  XtAddCallback(symbols[k], XtNexposeCallback,
    (XtCallbackProc) symbol_expose_cb, (XtPointer) xg);
  XtAddCallback(symbols[k], XtNinputCallback,
    (XtCallbackProc) symbol_reset_cb, (XtPointer) xg);

  hide_tgl[k] = CreateToggle(xg, "", True,
    collbl[1], vref, (Widget) NULL,
    xg->clusv[k].hidden, ANY_OF_MANY, parent, "HideOrExclude");
  XtAddCallback(hide_tgl[k], XtNcallback,
    (XtCallbackProc) hide_cback, (XtPointer) xg);
    
  exclude_tgl[k] = CreateToggle(xg, "", True,
    /* (xg->nlinkable != xg->nrows) ? False : True, */
    collbl[2], vref, (Widget) NULL,
    xg->clusv[k].excluded, ANY_OF_MANY, parent, "HideOrExclude");
  XtAddCallback(exclude_tgl[k], XtNcallback,
    (XtCallbackProc) exclude_cback, (XtPointer) xg);
}

static void
alloc_clust(xgobidata *xg) {
  names = (Widget *)
    XtRealloc((XtPointer)names, xg->nclust * sizeof(Widget));
  symbols = (Widget *)
    XtRealloc((XtPointer)symbols, xg->nclust * sizeof(Widget));
  hide_tgl = (Widget *)
    XtRealloc((XtPointer)hide_tgl, xg->nclust * sizeof(Widget));
  exclude_tgl = (Widget *)
    XtRealloc((XtPointer)exclude_tgl, xg->nclust * sizeof(Widget));
}

static void
shift_clust_widgets(xgobidata *xg) {
  int i, k;
  Dimension colwidth[4];
  Dimension cboxwidth;

  if (xg->nclust > 1) {

    for (i=0; i<4; i++)
      XtVaGetValues(collbl[i], XtNwidth, &colwidth[i], NULL);

    XtVaGetValues(hide_tgl[0], XtNwidth, &cboxwidth, NULL);

    for (k=0; k<xg->nclust; k++) {
      if (colwidth[1] > 22)
        XtVaSetValues(symbols[k],
          XtNhorizDistance, colwidth[1]/2, NULL);
      if (colwidth[2] > cboxwidth)
        XtVaSetValues(hide_tgl[k],   
          XtNhorizDistance, (colwidth[2]-cboxwidth)/2, NULL);
      if (colwidth[3] > cboxwidth)
        XtVaSetValues(exclude_tgl[k],
          XtNhorizDistance, (colwidth[3]-cboxwidth)/2, NULL);
    }
  }
}

static void
create_clust_widgets(xgobidata *xg) {
  int i;

  save_brush_groups (xg);

  if (xg->nclust > 1) {
    alloc_clust(xg);

    for (i=0; i<xg->nclust; i++)
      addGroup(i, eform, xg);

    XtManageChildren(hide_tgl, xg->nclust);
    XtManageChildren(exclude_tgl, xg->nclust);
  } else {

    (void) XtVaCreateManagedWidget("HideOrExclude",
      labelWidgetClass, eform,
      XtNlabel, "*** There's just one group; all symbols are alike ***",
      XtNfromVert, collbl[0],
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      NULL);
  }
}

/* ARGSUSED */
XtCallbackProc
open_exclusion_popup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Widget reset_clusters_cmd, close, href;
  Widget form0;
  Dimension width, height;
  int i, k;
  static char *colname[] = {"Group name", "Symbol", "Hidden", "Excluded"};

  if (epopup == NULL) {

    if (popupx == -1 && popupy == -1) {
      XtVaGetValues(xg->workspace,
        XtNwidth, &width,
        XtNheight, &height, NULL);
      XtTranslateCoords(xg->workspace,
        (Position) width, (Position) (height/2), &popupx, &popupy);
    }

    /*
     * Create the popup to solicit subsetting arguments.
    */
    epopup = XtVaCreatePopupShell("Exclusion",
      topLevelShellWidgetClass, xg->shell,
      XtNx, (Position) popupx,
      XtNy, (Position) popupy,
      XtNinput, (Boolean) True,
      XtNtitle, (String) "Hide or exclude groups",
      XtNiconName, (String) "Exclusion",
      NULL);
    if (mono) set_mono(epopup);

    /*
     * Create a paned widget so the 'Click here ...'
     * can be all across the bottom.
    */
    epane = XtVaCreateManagedWidget("Form0",
      panedWidgetClass, epopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      XtNresizable, False,
      NULL);

    reset_clusters_cmd = XtVaCreateManagedWidget("Close",
      commandWidgetClass, epane,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Reset brushed groups",
      NULL);
    XtAddCallback(reset_clusters_cmd, XtNcallback,
      (XtCallbackProc) reset_clusters_cback, (XtPointer) xg);

    form0 = XtVaCreateManagedWidget("Form",
      formWidgetClass, epane,
      XtNresizable, False,
      NULL);
    if (mono) set_mono(form0);
    /*
     * Create the form widget.
    */
    eform = XtVaCreateManagedWidget("Form",
      formWidgetClass, form0,
      XtNmappedWhenManaged, False,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(eform);

    /*
     * Add row of column names
    */
    for (i=0; i<4; i++) {
      href = (i==0) ? NULL : collbl[i-1];
      collbl[i] = XtVaCreateManagedWidget("HideOrExclude",
        labelWidgetClass, eform,
        XtNborderWidth, 0,
        XtNlabel, colname[i],
        XtNfromHoriz, href,
        NULL);
    }
    XtManageChildren(collbl, 4);

    create_clust_widgets(xg);

    close = XtVaCreateManagedWidget("Close",
      commandWidgetClass, epane,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close);
    XtAddCallback(close, XtNcallback,
      (XtCallbackProc) close_cback, (XtPointer) xg);

  }

  XtPopup(epopup, (XtGrabKind) XtGrabNone);
  set_wm_protocols(epopup);
  XRaiseWindow(display, XtWindow(epopup));

  XtMapWidget(eform);
  shift_clust_widgets (xg);
  if (xg->nclust > 1)
    for (k=0; k<xg->nclust; k++)
      XDefineCursor (display, XtWindow(symbols[k]), default_cursor);
}

/* ARGSUSED */
XtCallbackProc
reset_clusters_cback (Widget w, xgobidata *xg, XtPointer callback_data)
{
  save_group_names (xg);

  XtVaGetValues(epopup,
    XtNx, &popupx,
    XtNy, &popupy,
    NULL);
  XtDestroyWidget(epopup);
  epopup = NULL;
  open_exclusion_popup_cback ((Widget) NULL, xg, callback_data);
}

void
save_brush_groups (xgobidata *xg) {
  int i, k, n, j, groupno;
  int new_color, new_glyph;
  glyphv glyphs_used[NGLYPHS];
  unsigned long colors_used[NCOLORS+2];
  int nglyphs_used, ncolors_used;
  int *cols, ncols;
  int ncols_used_prev = xg->ncols_used;
  int nclust = 1;  /* There can never be 0 clusters, right */
  /*int nr = (xg->nlinkable < xg->nrows) ? xg->nlinkable : xg->nrows;*/
  int nr = xg->nrows;

/*
 * Copy the existing groups into clusv_prev
*/
  nclust_prev = xg->nclust;
  clusv_prev = (cluster *) XtMalloc(nclust_prev * sizeof(cluster));
  if (nclust_prev > 1) {
    for (k=0; k<nclust_prev; k++) {
      clusv_prev[k].glyphtype = xg->clusv[k].glyphtype;
      clusv_prev[k].glyphsize = xg->clusv[k].glyphsize;
      clusv_prev[k].color = xg->clusv[k].color;
      strcpy(clusv_prev[k].name, xg->clusv[k].name);
    }

    XtFree ((XtPointer) xg->clusv);
  }

  xg->nclust = 1;

/*
 * Find all glyphs and colors used.
*/
  colors_used[0] = xg->color_ids[0];
  ncolors_used = 1;
  if (!mono) {
    for (i=0; i<nr; i++) {
      new_color = 1;
      for (k=0; k<ncolors_used; k++) {
        if (colors_used[k] == xg->color_ids[i]) {
          new_color = 0;
          break;
        }
      }
      if (new_color) {
        colors_used[ncolors_used] = xg->color_ids[i];
        ncolors_used++;
      }
    }
  }

  glyphs_used[0].type = xg->glyph_ids[0].type;
  glyphs_used[0].size = xg->glyph_ids[0].size;
  nglyphs_used = 1;
  for (i=0; i<nr; i++) {
    new_glyph = 1;
    for (k=0; k<nglyphs_used; k++) {
      if (glyphs_used[k].type == xg->glyph_ids[i].type &&
          glyphs_used[k].size == xg->glyph_ids[i].size)
      {
        new_glyph = 0;
        break;
      }
    }
    if (new_glyph) {
      glyphs_used[nglyphs_used].type = xg->glyph_ids[i].type;
      glyphs_used[nglyphs_used].size = xg->glyph_ids[i].size;
      nglyphs_used++;
    }
  }

/* 
 * Deal with the brushing groups
*/
  if (ncolors_used * nglyphs_used == 1)
  {
    /*
     * If there are no brushing groups, set each value = 1.
     * Forget about unmapping the variable, though; it's
     * too excruciating.
    */
    for (i=0; i<nr; i++)
      GROUPID_RAW(i) = GROUPID_TFORM1(i) = GROUPID_TFORM2(i) = 0.0;

  } else {

    xg->ncols_used = xg->ncols;

    xg->clusv = (cluster *)
      XtMalloc((Cardinal) (ncolors_used * nglyphs_used) * sizeof(cluster));

    /*
     * Loop over glyphs and colors to find out how many
     * clusters there are.
    */
    nclust = 0;
    for (k=0; k<nglyphs_used; k++) {

      for (n=0; n<ncolors_used; n++) {
        new_color = 0;
        /*
         * Loop over all points, looking at glyph and color ids.
         * new clusv group.
        */
        for (i=0; i<nr; i++) {
          /*
           * If we find a pair ...
          */
          if (xg->glyph_ids[i].type == glyphs_used[k].type &&
              xg->glyph_ids[i].size == glyphs_used[k].size &&
              xg->color_ids[i] == colors_used[n])
          {
            new_color = 1;
            /*
             * make sure it's not already a member of clusv[]
            */
            for (j=0; j<nclust; j++) {
              if (xg->clusv[j].glyphtype == glyphs_used[k].type &&
                xg->clusv[j].glyphsize == glyphs_used[k].size &&
                xg->clusv[j].color == colors_used[n])
                  new_color = 0;
            }
            break;
          }
        }

        if (new_color) {
          xg->clusv[nclust].glyphtype = xg->clusv[nclust].glyphtype_prev =
            glyphs_used[k].type;
          xg->clusv[nclust].glyphsize = xg->clusv[nclust].glyphsize_prev =
            glyphs_used[k].size;
          xg->clusv[nclust].color = xg->clusv[nclust].color_prev =
            colors_used[n];
          nclust++;
        }
      }
    }

/* 
 * If there are missing values, be sure to initialize
 * the added column of the missing values matrix.
*/
    if (xg->missing_values_present) init_missing_groupvar(xg);

    map_group_var(xg);

    /*
     * If there are clusters, set the data in the groups column
    */
    if (nclust > 1) {
      for (n=0; n<nclust; n++) {
        for (i=0; i<xg->nrows; i++) {
          if (xg->glyph_ids[i].type == xg->clusv[n].glyphtype &&
              xg->glyph_ids[i].size == xg->clusv[n].glyphsize &&
              xg->color_ids[i] == xg->clusv[n].color)
          {
            GROUPID_RAW(i) = GROUPID_TFORM1(i) = GROUPID_TFORM2(i) = (float) n;
          }
        }
      }
    }
    xg->nclust = nclust;

    /*
     * If necessary, reset the values of clusv[].hidden, clusv[].excluded.
    */
    if (nclust > 1) {
      for (k=0; k<nclust; k++) {
        xg->clusv[k].hidden = False;
        xg->clusv[k].excluded = False;
      }
    }
    for (i=0; i<nr; i++) {
      if (xg->erased[i])
        xg->clusv[(int)GROUPID(i)].hidden = True;
      if (xg->excluded[i])
        xg->clusv[(int)GROUPID(i)].excluded = True;
    }

/*
 * In case any groups are hidden or erased, and new points have been
 * added to those groups, we need to make one more pass through all
 * the rows,
*/
    xg->nrows_in_plot = 0;
    for (i=0; i<nr; i++) {
      xg->erased[i] = xg->clusv[(int)GROUPID(i)].hidden;
      if (!xg->excluded[i])
        xg->rows_in_plot[(xg->nrows_in_plot)++] = i;
    }

    update_nrgroups_in_plot(xg);
  }


/*
 * It is possible, though not likely, that the erased or hidden state
 * of some points was changed by the foregoing.    Just in case ...
*/
  copy_brushinfo_to_senddata (xg);
  if (xg->link_glyph_brushing ||
      xg->link_color_brushing ||
      xg->link_erase_brushing)
  {
    announce_brush_data(xg);
#if defined RPC_USED || defined DCE_RPC_USED
    xfer_brushinfo(xg);
#endif
  }


  reset_3d_cmds(xg);
  if (xg->ncols_used == 3) {
    alloc_tour(xg);
    init_tour(xg, 1);
  }
  if (xg->ncols_used == 3) {
    alloc_corr(xg);
    init_corr(xg);
  }
  /* tour_alloc() should have been executed in any case */

/*
 * Run the data through the pipeline.
*/
  /*
   * Figure out which group the group variable belongs to.
   * This isn't meaningful yet, but it will be if we implement
   * an interactive way to reset groups.  ?? why ??
  */
  ncols = 0;
  groupno = xg->vgroup_ids[xg->ncols-1];
  cols = (int *) XtMalloc((unsigned) xg->ncols_used * sizeof(int));
  for (j=0; j<xg->ncols_used; j++) {
    if (xg->vgroup_ids[j] == groupno)
      cols[ncols++] = j;
  }

  /*
   * dfs: changing 9 November 99, because update_vc gives me
   * purify errors if tour hasn't been run.
  */
  if (xg->plot_mode == GTOUR_MODE && xg->ncols_used > 2) {
    update_sphered(xg, cols, ncols);
    xg->v0[0][xg->ncols-1] = 0.0;
    xg->v0[1][xg->ncols-1] = 0.0;
    xg->uold[0][xg->ncols-1] = 0.0;
    xg->uold[1][xg->ncols-1] = 0.0;
  }

  update_lims(xg);
  update_world(xg);

  init_tickdelta(xg);
  if (xg->is_xyplotting)
    init_ticks(&xg->xy_vars, xg);
  else if (xg->is_plotting1d)
    init_ticks(&xg->plot1d_vars, xg);

  /*
   * This will add the extra variable to the case profile plot
   * if xgobi is case profile plotting.
  */
  reset_nvars_cprof_plot(xg);

  if (ncols_used_prev != xg->ncols_used)
    varlist_add_group_var(xg);

  /*
   * This is necessary in case any points have
   * been added to a group that's hidden or excluded in the
   * "Hide or exclude groups" panel.
  */
  if (xg->is_plotting1d) plot1d_texture_var(xg);
  world_to_plane(xg);
  plane_to_screen(xg);
  assign_points_to_bins(xg);
  plot_once(xg);

  XtFree((XtPointer) cols);

} /* save_brush_groups (xgobidata *xg) { */

