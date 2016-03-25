/* identify.c */
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

#include <X11/keysym.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

static Widget identify_panel, rm_sticky_cmd, all_sticky_cmd;
Widget id_linkopt_menu_cmd, id_linkopt_menu, id_linkopt_menu_btn[1];
extern Widget caselist_popup;

void
init_identify_vars(xgobidata *xg)
{
  xg->is_identify = False;
  xg->nearest_point = -1;
  xg->nsticky_ids = 0;
}

/*
 * Code for cycling
*/

/* Not yet released */

/* End of cycling section */

/*
Add pack_ids_done and pack_ids_lose in wherever they belong ...
*/

/* ARGSUSED */
XtSelectionDoneProc
pack_ids_done(Widget w, Atom *selection, Atom *target)
{
/*
 * This routine does nothing; its only purpose in life is to
 * prevent the Intrinsics from freeing the selection value.
*/
}

/* ARGSUSED */
XtLoseSelectionProc
pack_ids_lose(Widget w, Atom *selection, XtPointer xgobi)
{
/*
 * This routine does nothing; its only purpose in life is to
 * prevent the Intrinsics from freeing the selection value.
*/
}

void
announce_ids(xgobidata *xg)
{
/*
 * Execute XChangeProperty(), sending no data, just to let
 * linked XGobis know that new ids are available.
*/
  XChangeProperty( display,
    RootWindowOfScreen(XtScreen(xg->shell)),
    XG_IDS_ANNC, XG_IDS_ANNC_TYPE,
    (int) 32, (int) PropModeReplace,
    (unsigned char *) NULL, 0);
}

/* ARGSUSED */
XtConvertSelectionProc
pack_ids(Widget w, Atom *selection, Atom *target,
Atom *type_ret, XtPointer *retdata, unsigned long *length_ret, int *format_ret)
/*
  w           owning widget, xg->workspace
  selection   XG_IDS
  target      XG_IDS_TYPE
  type_ret    XG_IDS_TYPE again
  retdata     xg->nrows + xg->sticky_ids
  length_ret  3 + xg->nsticky_ids
  format_ret  type of retdata
*/
{
/*
 * pack up the nearest_point and the sticky_ids to send
*/
  extern xgobidata xgobi;
  int i;
  unsigned long *rdata;

  if (*target == XG_IDS_TYPE)
  {
    rdata = (unsigned long *) XtMalloc((Cardinal)
      (3+xgobi.nsticky_ids) * sizeof(unsigned long) );

    rdata[0] = (unsigned long) xgobi.nlinkable;
    rdata[1] = (unsigned long) xgobi.nearest_point;
    rdata[2] = (unsigned long) xgobi.nsticky_ids;
    for (i=0; i<xgobi.nsticky_ids; i++)
      if (i < xgobi.nlinkable)
        rdata[i+3] = (unsigned long) xgobi.sticky_ids[i] ;

    *type_ret = XG_IDS_TYPE ;
    *retdata = (XtPointer) rdata ;
    *length_ret = (unsigned long) (3+xgobi.nsticky_ids) ;
    *format_ret = (int) 32 ;
  }
}

/* ARGSUSED */
XtSelectionCallbackProc
unpack_ids(Widget w, XtPointer xgobiptr, Atom *atom, Atom *atom_type,
XtPointer retdata, unsigned long *lendata, int *fmt)
/*
  atom         should be XG_IDS
  atom_type    should be XG_IDS_TYPE
  fmt          should be 32
*/
{
  xgobidata *xg = (xgobidata *) xgobiptr;
  int i, nids, nr;
  unsigned long *rdata;

  if (*atom == XG_IDS &&
      /**atom_type != NULL &&*/
      *atom_type &&
      *atom_type == XG_IDS_TYPE )
  {
    if (*lendata > 0)
    {
      rdata = (unsigned long *) retdata;

      nr = (int) *rdata++ ;
      {
        if (nr == xg->nlinkable)
        {
          xg->nearest_point = (int) *rdata++ ;
          /*
           * If the currently identified point has an index
           * higher than the number of linkable points in
           * this data, then we just won't show it.
           * The sticky ids were checked out when they were
           * packed.
          */
          if (xg->nearest_point >= xg->nlinkable)
            xg->nearest_point = -1;

          nids = (int) *rdata++ ;
          xg->nsticky_ids = nids ;
          xg->sticky_ids = (unsigned int *) XtRealloc(
            (XtPointer) xg->sticky_ids,
            (unsigned int) xg->nsticky_ids * sizeof(unsigned int) );

          for (i=0; i<xg->nsticky_ids; i++)
            xg->sticky_ids[i] = (int) *rdata++ ;

          plot_once(xg);
          passive_update_cprof_plot(xg);
        }
      }
/*
 * According to my reading of Asente and Swick, I can eliminate
 * this line if I don't specify a done procedure -- and purify
 * is complaining about the line, so that's what I'll do.  dfs 6/99
*/
      /*XtFree((XtPointer) retdata) ;*/
    }
  }
}

void
read_ids(xgobidata *xg)
{
  XtGetSelectionValue(
    xg->workspace,
    (Atom) XG_IDS,
    (Atom) XG_IDS_TYPE,
    (XtSelectionCallbackProc) unpack_ids,
    (XtPointer) xg,
    (Time) XtLastTimestampProcessed(display) );
    /*(Time) CurrentTime );*/
}

/*___End of linked identification section___*/

void
map_identify(xgobidata *xg, Boolean ident)
{
  if (ident)
  {
    XtMapWidget(xg->identify_mouse);
    XtMapWidget(identify_panel);
  }
  else
  {
    XtUnmapWidget(xg->identify_mouse);
    XtUnmapWidget(identify_panel);
  }
}

/* ARGSUSED */
XtCallbackProc
rm_sticky_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  /* see vc_lists.c */
  if (caselist_popup && ((ShellWidget) caselist_popup)->shell.popped_up) {
    int k;
    for (k=0; k<xg->nsticky_ids; k++)
      update_list_selection(xg, xg->sticky_ids[k], False);
  }

  xg->nsticky_ids = 0;

  plot_once(xg);
  if (xg->is_cprof_plotting && xg->link_cprof_plotting) {
    realloc_tform(xg);  /* Done when nsticky_ids changes. */
    update_cprof_plot(xg);
  }

  if (xg->link_identify)
    announce_ids(xg);

}

/* ARGSUSED */
XtCallbackProc
all_sticky_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i;

  xg->nsticky_ids = xg->nrows_in_plot;
  xg->sticky_ids = (Cardinal *) XtRealloc((XtPointer) xg->sticky_ids,
    (Cardinal) xg->nsticky_ids * sizeof(Cardinal));
  for (i=0; i<xg->nrows_in_plot; i++)
    xg->sticky_ids[i] = xg->rows_in_plot[i];

  /* see vc_lists.c */
  if (caselist_popup && ((ShellWidget) caselist_popup)->shell.popped_up) {
    int k;
    for (k=0; k<xg->nsticky_ids; k++)
      update_list_selection(xg, xg->sticky_ids[k], True);
  }

  plot_once(xg);
  if (xg->is_cprof_plotting && xg->link_cprof_plotting) {
    realloc_tform(xg);  /* Done when nsticky_ids changes. */
    update_cprof_plot(xg);
  }

  if (xg->link_identify)
    announce_ids(xg);

}

/* ARGSUSED */
XtActionProc
NullNearestPoint(Widget w, XEvent *evnt, String *params, Cardinal nparams)
{
  extern xgobidata xgobi;

  if (xgobi.is_identify)
  {
    xgobi.nearest_point = -1;
    /*
     * Plot once ... then there won't be an id in the plot
     * if the cursor is outside the plot window.
    */
    quickplot_once(&xgobi);
    if (xgobi.link_identify)
      announce_ids(&xgobi);
  }
}

int
find_nearest_point(icoords *cursor_pos, xgobidata *xg)
{
/*
 * Returns index of nearest un-erased point
*/
  int i, sqdist, near, xdist, ydist, npoint;

  npoint = -1;
  /*near = 1000*PRECISION1;*/

  near = 20*20;  /* If nothing is close, don't show any label */

  for (i=0; i<xg->nrows_in_plot; i++) {
    if (!xg->erased[ xg->rows_in_plot[i] ]) {
      xdist = xg->screen[ xg->rows_in_plot[i] ].x - cursor_pos->x;
      ydist = xg->screen[ xg->rows_in_plot[i] ].y - cursor_pos->y;
      sqdist = xdist*xdist + ydist*ydist;
      if (sqdist < near) {
        near = sqdist;
        npoint = xg->rows_in_plot[i];
      }
    }
  }
  return(npoint);
}

void
id_proc(xgobidata *xg)
{
  int root_x, root_y;
  unsigned int kb;
  Window root, child;
  static int ocpos_x = 0, ocpos_y = 0;
  icoords cpos;
  static int inwindow = 1;
  int wasinwindow;
  int nearest = xg->nearest_point;

  wasinwindow = inwindow;
/*
 * Get the current pointer position.
*/
  if (XQueryPointer(display, xg->plot_window, &root, &child,
            &root_x, &root_y, &cpos.x, &cpos.y, &kb))
  {
    inwindow = (0 < cpos.x && cpos.x < xg->max.x &&
                0 < cpos.y && cpos.y < xg->max.y) ;
    /*
     * If the pointer is inside the plotting region ...
    */
    if (inwindow) {
      /*
       * If the pointer has moved ...
      */
      if ((cpos.x != ocpos_x) || (cpos.y != ocpos_y)) {
        ocpos_x = cpos.x;
        ocpos_y = cpos.y;
        /*
         * nearest_point ranges from 0 to nrows-1; 
         * it is -1, nothing is near enough to draw.
        */
        xg->nearest_point = find_nearest_point(&cpos, xg);
        if (xg->nearest_point != nearest) {
          quickplot_once(xg);
          if (xg->link_identify) {
            XtOwnSelection( (Widget) xg->workspace,
              (Atom) XG_IDS,
              (Time) CurrentTime,
              (XtConvertSelectionProc) pack_ids,
              (XtLoseSelectionProc) pack_ids_lose ,
              (XtSelectionDoneProc) NULL );
              /* (XtSelectionDoneProc) pack_ids_done ); */
            announce_ids(xg);
          }
          if (xg->is_cprof_plotting && xg->link_cprof_plotting)
            update_cprof_plot(xg);
        }
      }
    }
  }

  if (!inwindow && wasinwindow)
  {
    /*
     * Don't draw the current label or update the profile plot
     * if the pointer leaves the plot window.
    */
    xg->nearest_point = -1;
    quickplot_once(xg);  /* dfs 4.27.94 */
    if (xg->is_cprof_plotting && xg->link_cprof_plotting)
      update_cprof_plot(xg);
  }
}

void
plot_nearest_id(xgobidata *xg, Drawable win)
/*
 * Draw the id of the nearest point.
*/
{
  if (xg->nearest_point > -1)  {
    XDrawString(display, win, copy_GC,
      xg->screen[ xg->nearest_point ].x + 3,
      xg->screen[ xg->nearest_point ].y - 3,
      xg->rowlab[ xg->nearest_point ],
      strlen(xg->rowlab[ xg->nearest_point ]));

    /* Also display the label at the lower left corner of the plot */
    XDrawString(display, win, copy_GC,
      5, 
      xg->plotsize.height - 5,
      xg->rowlab[ xg->nearest_point ],
      strlen(xg->rowlab[ xg->nearest_point ]));
  }
}

void
update_sticky_ids(xgobidata *xg)
/*
 * Called from the case selection procedures -- done when
 * new variables are selected using the case list.
*/
{
  int *sticky_guys;
  int nsticky_guys = 0;
  int i, k, initval = 20, numinits = 1;

  sticky_guys = (int *) XtMalloc((Cardinal) initval * sizeof(int));

  for (i=0; i<xg->nrows_in_plot; i++)
  {
    k = xg->rows_in_plot[i] ;
    if (xg->selectedcases[k]) {
      sticky_guys[nsticky_guys++] = k ;
      if (nsticky_guys >= initval*numinits) {
        numinits++ ;
        sticky_guys = (int *) XtRealloc((XtPointer) sticky_guys,
          (Cardinal) (numinits * initval) * sizeof(Cardinal));
      }
    }
  }

  xg->nsticky_ids = nsticky_guys;
  xg->sticky_ids = (Cardinal *) XtRealloc((XtPointer) xg->sticky_ids,
    (Cardinal) xg->nsticky_ids * sizeof(Cardinal));

  for (i=0; i<xg->nsticky_ids; i++)
    xg->sticky_ids[i] = sticky_guys[i];

  XtFree((XtPointer) sticky_guys);
}

/* ARGSUSED */
XtEventHandler
set_sticky(Widget w, xgobidata *xg, XEvent *evnt)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    int i, j, found = 0;

    int root_x, root_y;
    unsigned int kb;
    Window root, child;
    icoords cpos;

    /*
     * I see no reason why this should be needed, but it
     * seems to be; otherwise, nearest_point keeps turning into
     * -1 as soon as the button is clicked -- this happens to
     * Di's students in Iowa and it happens on the SCA machine.
   */
    if (XQueryPointer(display, xg->plot_window, &root, &child,
      &root_x, &root_y, &cpos.x, &cpos.y, &kb))
        if ( (xg->nearest_point = find_nearest_point(&cpos, xg)) == -1 )
          return ((XtEventHandler) 0);

    for (i=0; i<xg->nsticky_ids; i++) {
       if (xg->sticky_ids[i] == xg->nearest_point) {
         found = 1;
         xg->nsticky_ids-- ;
         for (j=i; j<xg->nsticky_ids; j++) {
           xg->sticky_ids[j] = xg->sticky_ids[j+1] ;
         }
         xg->sticky_ids = (unsigned int *) XtRealloc( (XtPointer)
           xg->sticky_ids,
           (unsigned) xg->nsticky_ids * sizeof(unsigned int) );

         break;
       }
    }

    if (!found) {

      xg->nsticky_ids++ ;
      xg->sticky_ids = (unsigned int *) XtRealloc( (XtPointer)
        xg->sticky_ids,
        (unsigned) xg->nsticky_ids * sizeof(unsigned int) );
      xg->sticky_ids[xg->nsticky_ids-1] = xg->nearest_point;

      /* see vc_lists.c */
      if (caselist_popup && ((ShellWidget) caselist_popup)->shell.popped_up)
        update_list_selection(xg, xg->nearest_point, True);
    }

    plot_once(xg);
    if (xg->is_cprof_plotting && xg->link_cprof_plotting) {
      realloc_tform(xg);  /* Done when nsticky_ids changes. */
      update_cprof_plot(xg);
    }
    if (xg->link_identify)
      announce_ids(xg);
  }
}

void
set_id_linkopt_menu_marks(xgobidata *xg)
{
  if (xg->link_identify)
    XtVaSetValues(id_linkopt_menu_btn[0],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
  else
    XtVaSetValues(id_linkopt_menu_btn[0],
      XtNleftBitmap, (Pixmap) None,
      NULL);
}

/* ARGSUSED */
XtCallbackProc
id_linkopt_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<1; btn++)
    if (id_linkopt_menu_btn[btn] == w)
      break;

  switch (btn) {
    case 0 :
      xg->link_identify = !xg->link_identify;
      break;
  }
  set_id_linkopt_menu_marks(xg);
}


void
make_id_linkopt_menu(xgobidata *xg, Widget parent, Widget href, Widget vref)
{
  int k;

  static char *id_linkopt_menu_name[] = {
    "Link identification",
  };

  id_linkopt_menu_cmd = XtVaCreateManagedWidget("LinkBrushButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Link",
    XtNmenuName, (String) "Menu",
    XtNfromHoriz, href,
    XtNfromVert, vref,
    NULL);
  if (mono) set_mono(id_linkopt_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    id_linkopt_menu_cmd, "IdentifyLink");

  id_linkopt_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, id_linkopt_menu_cmd,
    XtNinput, True,
    NULL);
  if (mono) set_mono(id_linkopt_menu);

  for (k=0; k<1; k++)
  {
    id_linkopt_menu_btn[k] = XtVaCreateWidget("Command",
      smeBSBObjectClass, id_linkopt_menu,
      XtNleftMargin, (Dimension) 24,
      XtNleftBitmap, menu_mark,
      XtNlabel, (String) id_linkopt_menu_name[k],
      NULL);
    if (mono) set_mono(id_linkopt_menu_btn[k]);

    XtAddCallback(id_linkopt_menu_btn[k], XtNcallback,
      (XtCallbackProc) id_linkopt_menu_cback, (XtPointer) xg);
  }

  XtManageChildren(id_linkopt_menu_btn, 1);
}


void
make_identify(xgobidata *xg)
{
/*
  char str[64];
  Dimension max_width;
 *
 * Widest button label used in this panel; used to set width of sbar.
 *
  (void) sprintf(str, "Change Direction");
  max_width = XTextWidth(appdata.font, str, strlen(str));
*/

/*
 * IdentifyPanel
*/
  identify_panel = XtVaCreateManagedWidget("IdentifyPanel",
    boxWidgetClass, xg->box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
    NULL);
  if (mono) set_mono(identify_panel);

  /* Cycling */

/*
 * Reset identification labels button
*/
  rm_sticky_cmd = CreateCommand(xg, "Remove Labels",
    True, (Widget) NULL, (Widget) NULL,
    (Widget) identify_panel, "Id_RmLabels");
  XtManageChild(rm_sticky_cmd);

  XtAddCallback(rm_sticky_cmd, XtNcallback,
    (XtCallbackProc) rm_sticky_cback, (XtPointer) xg);

/*
 * Make them all sticky
*/
  all_sticky_cmd = CreateCommand(xg, "Make all sticky",
    True, (Widget) NULL, (Widget) NULL,
    (Widget) identify_panel, "Id_AllLabels");
  XtManageChild(all_sticky_cmd);

  XtAddCallback(all_sticky_cmd, XtNcallback,
    (XtCallbackProc) all_sticky_cback, (XtPointer) xg);

  make_id_linkopt_menu(xg, identify_panel, (Widget) NULL, all_sticky_cmd);

/*
 * Initialize a couple of variables needed for linking case plotting
 * to the variable list.
 *cprof_selectedvars = (int *) XtMalloc((Cardinal)
 *  xg->ncols * sizeof(int));
*/
}


/*
 * This event handler allows the label of the current point to
 * be snarfed into the system cut buffer, and then copied into
 * any other xterm or browser text field etc
 *
 * It got messy once I realized that I was capturing the key strokes
 * that would otherwise be used as accelerators to change the view
 * mode.  I couldn't just send the event out again, because the same
 * event handler would pick it up, so I had to call SetPlotMode
 * directly.
*/

XtEventHandler
copy_label(Widget w, xgobidata *xg, XEvent *event)
{
  XKeyPressedEvent *evnt = (XKeyPressedEvent *) event;
  KeySym key;
  static char *params[1];
  extern char *view_menu_accel[];  /* see widgets.c */
  char *buf, sbuf[8];
  int i;
  XComposeStatus status_in_out;

  buf = XtMalloc(8 * sizeof(char));
  XLookupString(evnt, buf, 8, &key, &status_in_out);

  sbuf[0] = buf[0];
  sbuf[1] = '\0';

  /* key = XLookupKeysym(evnt, 0); */
  if (key == XK_w && xg->nearest_point >= 0) {
    XStoreBytes(display, xg->rowlab[xg->nearest_point],
      strlen(xg->rowlab[xg->nearest_point]));
  } else {

    for (i=0; i<NVIEWMODES; i++) {
      if (strcmp(sbuf, view_menu_accel[i]) == 0) {
        params[0] = XtMalloc(2 * sizeof(char));
        sprintf(params[0], "%d", i);
        SetPlotMode(w, event, params, 1);
        break;
      }
    }
  }
}

void
identify_on(xgobidata *xg)
{
/*
 *  If this mode is currently selected, turn it off.
*/
  if (xg->prev_plot_mode == IDENTIFY_MODE && xg->plot_mode != IDENTIFY_MODE)
  {
    xg->nearest_point = -1;
    announce_ids(xg);

    XtRemoveEventHandler(xg->workspace,
      XtAllEvents, TRUE,
      (XtEventHandler) set_sticky, (XtPointer) xg);
    XtRemoveEventHandler(xg->workspace,
      XtAllEvents, TRUE,
      (XtEventHandler) copy_label, (XtPointer) xg);

    XDefineCursor(display, XtWindow(xg->workspace), default_cursor);
    xg->is_identify = False;
    xg->nearest_point = -1;
    map_identify(xg, False);
    plot_once(xg);

/*
 * I'm trying to work out how any linked xgobi's can be aware
 * that this xgobi isn't in identify mode any more -- sync'ing
 * and flushing don't do the trick.
 * I guess can just refuse to ever disown the selection;
 * I'm not sure what the downside is to that.  If it causes
 * memory leaks, I'll have to re-activate these lines.
*/
/*
    XtDisownSelection( (Widget) xg->workspace,
      (Atom) XG_IDS,
      (Time) XtLastTimestampProcessed(display));
*/
  }
  /* Else turn it on */
  else if (xg->plot_mode == IDENTIFY_MODE &&
           xg->prev_plot_mode != IDENTIFY_MODE)
  {
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
    XtAddEventHandler(xg->workspace, ButtonPressMask,
      FALSE, (XtEventHandler) set_sticky, (XtPointer) xg);
    XtAddEventHandler(xg->workspace, KeyPressMask, FALSE,
     (XtEventHandler) copy_label, (XtPointer) xg);

    XDefineCursor(display, XtWindow(xg->workspace), crosshair_cursor);
    xg->is_identify = True;
    map_identify(xg, True);

/*
 printf("init: %x %x %x %x\n",
 XG_IDS_ANNC,XG_IDS_ANNC_TYPE,XG_IDS,XG_IDS_TYPE);
*/

    XtOwnSelection( (Widget) xg->workspace,
      (Atom) XG_IDS,
      (Time) CurrentTime,
      /*(Time) XtLastTimestampProcessed(display),*/
      (XtConvertSelectionProc) pack_ids,
      (XtLoseSelectionProc) pack_ids_lose ,
      (XtSelectionDoneProc) NULL );
      /*(XtSelectionDoneProc) pack_ids_done );*/
  }
}

#undef LBMARGIN
#undef RTMARGIN
