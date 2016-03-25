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
#include <string.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

/* ARGSUSED */
XtCallbackProc
dismiss_message_cback(w, popup, cback_data)
  Widget w;
  XtPointer *popup;
  XtPointer cback_data;
{
  Widget parent = (Widget) popup;
  XtDestroyWidget(parent);
}

void
show_message(String message, xgobidata *xg)
{
  Widget popup, frame, text, done;
  Dimension width, height;
  Dimension wksp_width, wksp_height;
  Position x, y;
  int nreturns;
  int i, j, nchars, start;
  Widget shell = xg->shell;

  /*
   * Figure out the necessary width and height of the window
   * to contain the message.
  */
  for (i=0, j=0, nchars=0, nreturns=0, start=0; i<(int)strlen(message); i++)
  {
    j++ ;
    if (message[i] == '\n')
    {
      if (j > nchars)
      {
        nchars = j ;
        start = i - j + 1;
      }
      nreturns++ ;
      j = 0;
    }
  }

  width = 20 + XTextWidth(appdata.helpFont, &message[start], nchars) ;
  height = (nreturns + 1) * FONTHEIGHT(appdata.helpFont);

  /*
   * Create the popup itself.
  */
  popup = XtVaCreatePopupShell("Message",
    transientShellWidgetClass,
    shell,       /* child of the shell itself? */
    XtNtitle, (String) "XGobi Message Window",
    XtNiconName, (String) "XGobi Message Window",
    NULL);
  if (mono) set_mono(popup);
  /*
   * Create the paned widget.
  */
  frame = XtVaCreateManagedWidget("Form",
    panedWidgetClass, popup,
    XtNorientation, (XtOrientation) XtorientVertical,
    NULL);
  if (mono) set_mono(frame);

  /*
   * Create the text widget.
  */
  text = XtVaCreateManagedWidget("Text",
    asciiTextWidgetClass, frame,
    XtNallowResize, (Boolean) True,
    XtNshowGrip, (Boolean) False,
    XtNtype, (XawAsciiType) XawAsciiString,
    XtNstring, (String) message,
    XtNwidth, (Dimension) width,
    XtNheight, (Dimension) height,
    XtNscrollVertical, (XawTextScrollMode) XawtextScrollWhenNeeded,
    XtNdisplayCaret, (Boolean) False,
    XtNfont, (XFontStruct *) appdata.helpFont,  /* for now */
    NULL);
  if (mono) set_mono(text);

  /*
   * Create the Done button.
  */
  done = XtVaCreateManagedWidget("Done",
    commandWidgetClass, frame,
    XtNshowGrip, (Boolean) False,
    XtNskipAdjust, (Boolean) True,
    XtNlabel, (String) "Click here to dismiss",
    NULL);
  if (mono) set_mono(done);

  XtAddCallback(done, XtNcallback,
    (XtCallbackProc) dismiss_message_cback, (XtPointer) popup);

  /*
   * Place the message in the middle of the XGobi window
  */
  XtVaGetValues(shell,
    XtNwidth, &wksp_width,
    XtNheight, &wksp_height, NULL);
  XtTranslateCoords(shell,
    (Position) (wksp_width/2), (Position) (wksp_height/2), &x, &y);

  XtVaSetValues(popup,
    XtNx, (Position) (x - width/2),
    XtNy, (Position) (y - height/2),
    NULL);

  XtPopup(popup, XtGrabNone);
  XRaiseWindow(display, XtWindow(popup));

  set_wm_protocols(popup);
}
