/* getfname.c */
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

#include <sys/types.h>
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include <X11/keysym.h>

char msg1[] =
  "Enter filename; press return:";

XtEventHandler
file_save_done(Widget w, xgobidata *xg, XEvent *event)
{
  XKeyPressedEvent *evnt = (XKeyPressedEvent *) event;
  KeySym key;

  key = XLookupKeysym(evnt, 0);
  if (key == XK_Return)
  {
    /*
     * w is ftext; XtParent(w) = fform
     * XtParent(XtParent(w)) = fpopup
     * XtParent(XtParent(XtParent(w))) = the parent we're interested in
    */

    XtVaSetValues(XtParent(XtParent(XtParent(w))),
      XtNstate, (Boolean) False,
      NULL);

    if (strcmp(xg->save_type, SAVE_SPIN_COEFS) == 0)
      spin_save_coefs(w, xg);
    if (strcmp(xg->save_type, SAVE_SPIN_RMAT) == 0)
      spin_save_rmat(w, xg);
    if (strcmp(xg->save_type, READ_SPIN_RMAT) == 0)
      spin_read_rmat(w, xg);
    else if (strcmp(xg->save_type, SAVE_TOUR_COEFS) == 0)
      tour_save_coefs(w, xg);
    else if (strcmp(xg->save_type, SAVE_TOUR_HIST) == 0)
      tour_save_hist(w, xg);
    else if (strcmp(xg->save_type, READ_TOUR_HIST) == 0)
      tour_read_hist(w, xg);
    else if (strcmp(xg->save_type, OPEN_BITMAP_FILE) == 0)
      read_bm_filename(w, xg);

    XtDestroyWidget(XtParent(XtParent(w)));
  }
}

/* ARGSUSED */
XtCallbackProc
fcancel_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * If the plot window is fully or partially exposed, clear and redraw.
*/
{
  XtDestroyWidget(XtParent(XtParent(w)));

  if (strcmp(xg->save_type, SAVE_SPIN_COEFS) == 0)
    start_spin_proc(xg);
  if (strcmp(xg->save_type, SAVE_SPIN_RMAT) == 0)
    start_spin_proc(xg);
  if (strcmp(xg->save_type, READ_SPIN_RMAT) == 0)
    start_spin_proc(xg);
  else if (strcmp(xg->save_type, SAVE_TOUR_COEFS) == 0)
    start_tour_proc(xg);
  else if (strcmp(xg->save_type, SAVE_TOUR_HIST) == 0)
    start_tour_proc(xg);
  else if (strcmp(xg->save_type, READ_TOUR_HIST) == 0)
    start_tour_proc(xg);
  else if (strcmp(xg->save_type, OPEN_BITMAP_FILE) == 0)
  {
    bm_cancel();
    start_tour_proc(xg);
  }
}

void
fname_popup(Widget popup_pop, xgobidata *xg)
{
  Widget fpopup, fform, flabel, ftext, fcancel;
  Dimension width, height;
  Position x, y;
  Cursor text_cursor = XCreateFontCursor(display, XC_xterm);
  String msg, initname;

  XtVaGetValues(popup_pop,
    XtNwidth, &width,
    XtNheight, &height,
    NULL);
  XtTranslateCoords(popup_pop,
    (Position) (width/2), (Position) (height/2), &x, &y);

/*
 * Create the popup itself.
*/
  fpopup = XtVaCreatePopupShell("FSavePopup",
    /*
     * If this is a topLevelShell, the user is asked to
     * place it; if it's transient, it pops up where we
     * tell it to.
    */
    /*topLevelShellWidgetClass, popup_pop,*/
    transientShellWidgetClass, popup_pop,
    XtNx, (Position) x,
    XtNy, (Position) y,
    XtNinput, (Boolean) True,
    XtNallowShellResize, (Boolean) True,
    XtNtitle, (String) "Solicit File Name",
    NULL);
  if (mono) set_mono(fpopup);
/*
 * Create the form widget.
*/
  fform = XtVaCreateManagedWidget("FSaveForm",
    formWidgetClass, fpopup,
    NULL);
  if (mono) set_mono(fform);
/*
 * Create the label.
*/
  msg = (String) XtMalloc((Cardinal) 100 * sizeof(char));
  initname = (String) XtMalloc((Cardinal) 120 * sizeof(char));
  strcpy(msg, "");
  strcpy(initname, "");
  if (strcmp(xg->save_type, SAVE_SPIN_COEFS) == 0)
    strcpy(msg, msg1);
  if (strcmp(xg->save_type, SAVE_SPIN_RMAT) == 0)
    strcpy(msg, msg1);
  if (strcmp(xg->save_type, READ_SPIN_RMAT) == 0)
    strcpy(msg, msg1);
  else if (strcmp(xg->save_type, SAVE_TOUR_COEFS) == 0)
    strcpy(msg, msg1);
  else if (strcmp(xg->save_type, SAVE_TOUR_HIST) == 0)
    strcpy(msg, msg1);
  else if (strcmp(xg->save_type, READ_TOUR_HIST) == 0)
    strcpy(msg, msg1);
  else if (strcmp(xg->save_type, OPEN_BITMAP_FILE) == 0)
    strcpy(msg, msg1);

  flabel = XtVaCreateManagedWidget("FSaveText",
    labelWidgetClass, fform,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNlabel, (String) msg,
    NULL);
  if (mono) set_mono(flabel);

/*
 * Create the text widget to solicit the filename.
*/
  ftext = XtVaCreateManagedWidget("FSaveName",
    asciiTextWidgetClass, fform,
    XtNfromVert, (Widget) flabel,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainRight,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNresizable, (Boolean) True,
    XtNeditType, (int) XawtextEdit,
    XtNresize, (XawTextResizeMode) XawtextResizeWidth,
    XtNstring, (String) initname,
    NULL);
  if (mono) set_mono(ftext);

  XtAddEventHandler(ftext, KeyPressMask, FALSE,
     (XtEventHandler) file_save_done, (XtPointer) xg);

/*
 * Add a cancel button
*/
  fcancel = (Widget) CreateCommand(xg, "Cancel",
    1, (Widget) NULL, (Widget) ftext,
    fform, "FSaveCancel");
  XtManageChild(fcancel);
  XtAddCallback(fcancel, XtNcallback,
    (XtCallbackProc) fcancel_cback, (XtPointer) xg);

  XtPopup(fpopup, XtGrabExclusive);
  XRaiseWindow(display, XtWindow(fpopup));

  XDefineCursor(display, XtWindow(ftext), text_cursor);
/*
 * Should do something more clever here -- get the size
 * of the window and place the cursor that way.
*/
  XWarpPointer(display, None, XtWindow(ftext), 0,0,0,0, 10,40);

  XtFree(msg);
  XtFree(initname);
}
