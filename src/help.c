/* help.c */
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#define NMENUS NCOLS+40
#define NSBARS 32
#define NCOMMANDS 512

typedef struct {
   Widget w;
   char *helpFile;
} HelpTable;

HelpTable pb_table[NCOMMANDS];
HelpTable sbar_table[NSBARS];
HelpTable menupb_table[NMENUS];

/* ARGSUSED */
XtActionProc
HelpSelect(Widget w, XEvent *evnt, String *params, Cardinal nparams)
/*
 * Search the help tables looking for w, then call help()
*/
{
  int j = 0;
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;

  while (pb_table[j].w != NULL)
  {
    if (pb_table[j].w == w)
    {
      help(w, pb_table[j].helpFile, xg);
      break;
    }
    else
      j++;
  }

  j = 0;
  while (sbar_table[j].w != NULL)
  {
    if (sbar_table[j].w == w)
    {
      help(w, sbar_table[j].helpFile, xg);
      break;
    }
    else
      j++;
  }

  j = 0;
  while (menupb_table[j].w != NULL)
  {
    if (menupb_table[j].w == w)
    {
      help(w, menupb_table[j].helpFile, xg);
      break;
    }
    else
      j++;
  }
}

/* ARGSUSED */
XtCallbackProc
help_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  help(XtParent(XtParent(w)), "Help", xg);
}

/* ARGSUSED */
XtCallbackProc
about_xgobi_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  help(XtParent(XtParent(w)), "AboutXGobi", xg);
}

void
init_help(xgobidata *xg)
{
  xg->nhelpids.pb = xg->nhelpids.menupb = xg->nhelpids.sbar = 0;
}

/* ARGSUSED */
XtCallbackProc
help_done_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtDestroyWidget(XtParent(XtParent(w)));
}

/* ARGSUSED */
void
help(Widget popup_pop, char *helpfile, xgobidata *xg)
{
  char fname[100];
  char message[MSGLENGTH];
  char *xgobidir;
  FILE *fp;
  extern xgobidata xgobi;

  xgobidir = getenv("XGOBID");
  if (xgobidir == NULL || strlen(xgobidir) == 0)
  {
    xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
    (void) strcpy(xgobidir, DEFAULTDIR);
    if (xgobidir == NULL || strlen(xgobidir) == 0)
    {
      sprintf(message,
       "XGOBID is not defined in your environment, and\n");
      strcat(message,
       "DEFAULTDIR is not defined in the XGobi Makefile;\n");
      strcat(message,
        "see the person who installed XGobi for help.\n");
      show_message(message, &xgobi);
      return;
    }
    else
    {
      (void) strcpy(fname, xgobidir);
      XtFree((XtPointer) xgobidir);
    }
  }
  else
  {
    (void) strcpy(fname, xgobidir);
  }

/*
 * Check that the file is good.
*/
  (void) strcat(fname, "/help/");
  (void) strcat(fname, helpfile);
  if ((fp = fopen(fname, "r")) == NULL)
  {
    sprintf(message,
      "Unable to open %s.\n", fname);
    strcat(message,
      "Is the shell variable XGOBID the name of the directory\n");
    strcat(message,
      "which contains the help subdirectory?\n");
    show_message(message, &xgobi);
    return;
  }
  else
  {
    Widget hpopup, hframe, htext, hdone;
  /*
   * Create the popup itself.
  */
    hpopup = XtVaCreatePopupShell("Help",
      topLevelShellWidgetClass, popup_pop,
      XtNtitle, (String) "XGobi Help Window",
      XtNiconName, (String) "XGobi Help Window",
      NULL);
    if (mono) set_mono(hpopup);
  /*
   * Create the paned widget.
  */
    hframe = XtVaCreateManagedWidget("Form",
      panedWidgetClass, hpopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      NULL);
    if (mono) set_mono(hframe);

  /*
   * Create the text widget.
  */
    htext = XtVaCreateManagedWidget("Text",
      asciiTextWidgetClass, hframe,
      XtNallowResize, (Boolean) True,
      XtNshowGrip, (Boolean) False,
      XtNtype, (XawAsciiType) XawAsciiFile,
      XtNstring, (String) fname,
      /* AB
      XtNscrollVertical, (XawTextScrollMode) XawtextScrollWhenNeeded,
      */
      XtNscrollVertical, (XawTextScrollMode) XawtextScrollAlways,
      XtNdisplayCaret, (Boolean) False,
      XtNfont, (XFontStruct *) appdata.helpFont,
      /* Moving these here from the fallback resources for xgvis */
      XtNheight, 250,
      XtNwidth, 600,
      NULL);
    if (mono) set_mono(htext);

  /*
   * Create the Done button.
  */
    hdone = XtVaCreateManagedWidget("Done",
      commandWidgetClass, hframe,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(hdone);

    XtAddCallback(hdone, XtNcallback,
      (XtCallbackProc) help_done_cback, (XtPointer) NULL);

    XtPopup(hpopup, XtGrabNone);
    XRaiseWindow(display, XtWindow(hpopup));

    set_wm_protocols(hpopup);

    fclose(fp);
  }
  return;
}

void
add_pb_help(int *n, Widget w, char *fname)
/*
 * Add a widget to the helptable for pushbutton widgets.
*/
{
  if (*n>0 && pb_table[*n-1].w == NULL)
    (*n)--;

  pb_table[*n].w = w;
  pb_table[*n].helpFile = fname;
  (*n)++;

  if (*n == NCOMMANDS)
    printf("Edit help.c and increase NCOMMANDS\n");
}

void
add_menupb_help(int *n, Widget w, char *fname)
/*
 * Add a widget to the helptable for menus.
*/
{
  /*if (*n>0 && pb_table[*n-1].w == NULL)*/
  if (*n>0 && menupb_table[*n-1].w == NULL)
    (*n)--;

  menupb_table[*n].w = w;
  menupb_table[*n].helpFile = fname;
  (*n)++;

  if (*n == NMENUS)
    printf("Edit help.c and increase NMENUS\n");
}

void
add_sbar_help(int *n, Widget w, char *fname)
/*
 * Add a widget to the helptable for scrollbar widgets.
*/
{
  sbar_table[*n].w = w;
  sbar_table[*n].helpFile = fname;
  (*n)++;

  if (*n == NSBARS)
    printf("Edit help.c and increase NSBARS\n");
}

/****************************/

#include "../bitmaps/splash.xbm"
#include "../bitmaps/Bellcore.xbm"
Widget splashxgobi;

/*ARGSUSED*/
XtCallbackProc
kill_splash(Widget w, XtPointer client_data, XtPointer callback_data)
{
  XtDestroyWidget(XtParent(XtParent(w)));
  splashxgobi = (Widget) NULL;
}

void
show_splash_screen(xgobidata *xg)
{
  Screen *scrn;
  Boolean ok;
  Pixmap splashbcr_pix, splashxgobi_pix;
  Widget splashshell, splashform, splashbcr;
  Pixel splashfg, splashbg;
  XColor exact;
  Colormap cmap;
  XEvent event;

/* show the splash screen */
  scrn = XtScreen(xg->shell);
  splashbg = WhitePixelOfScreen(scrn);
  if (!mono)
  {
    cmap = DefaultColormap(display,  DefaultScreen(display));
    ok = False;
/* Bellcore blue: R=0, G=96/24576, B=141/36096 */
    if (XParseColor(display, cmap, "DeepSkyBlue4", &exact)) {
      if (XAllocColor(display, cmap, &exact)) {
        splashfg = exact.pixel;
        ok = True;
      }
    }
  }
  if (mono || !ok)
    splashfg = BlackPixelOfScreen(scrn);

  splashbcr_pix = XCreatePixmapFromBitmapData(display,
    RootWindowOfScreen(XtScreen(xg->shell)),
    Bellcore_bits, Bellcore_width, Bellcore_height,
    splashfg, splashbg,
    depth);
  splashxgobi_pix = XCreatePixmapFromBitmapData(display,
    RootWindowOfScreen(XtScreen(xg->shell)),
    splash_bits, splash_width, splash_height,
    splashfg, splashbg,
    depth);
  splashshell = XtVaCreatePopupShell("Splash",
    transientShellWidgetClass, xg->shell,
    XtNinput, True,
    XtNtitle, "XGobi",
    XtNbackground, splashbg,
    XtNx, 100,
    XtNy, 100,
    NULL);
  splashform = XtVaCreateManagedWidget("Splash",
    formWidgetClass, splashshell,
    XtNbackground, splashbg,
    NULL);
  splashbcr = XtVaCreateManagedWidget("Splash",
    labelWidgetClass, splashform,
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNbitmap, (Pixmap) splashbcr_pix,
    XtNborderWidth, 0,
    NULL);
  splashxgobi = XtVaCreateManagedWidget("Splash",
    commandWidgetClass, splashform,
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNbitmap, (Pixmap) splashxgobi_pix,
    XtNfromVert, splashbcr,
    XtNvertDistance, 0,
    XtNborderWidth, 0,
    NULL);

  XtAddCallback(splashxgobi, XtNcallback,
    (XtCallbackProc) kill_splash, (XtPointer) NULL);
  XtPopup(splashshell, XtGrabNone);
  XRaiseWindow(display, XtWindow(splashshell));

/*
printf("splashshell %d\n", XtWindow(splashshell));
printf("splashform %d\n", XtWindow(splashform));
printf("splashbcr %d\n", XtWindow(splashbcr));
printf("splashxgobi %d\n", XtWindow(splashxgobi));
*/

  while (1) {
    XtAppNextEvent(app_con, &event);
    /*printf("event = %d, window = %d\n", event.type, event.xany.window);*/
    XtDispatchEvent(&event);
    /* Once the last widget gets an expose event, bail out */
    if (event.type == 12 && event.xany.window == XtWindow(splashxgobi))
      break;
  }
}
