#include <stdlib.h>
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include "xgvis.h"

#include "xgobi_config.h"

/* bitmaps: */
#include "../bitmaps/stress_kruskal_shepard.xbm"
#include "../bitmaps/strain_torgerson_gower.xbm"


/* ARGSUSED */
XtCallbackProc
xgvis_help_done_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  XtDestroyWidget(XtParent(XtParent(w)));
}

/* ARGSUSED */
XtCallbackProc
xgvis_help_MDS_background_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  char fname[100];
  char message[MSGLENGTH];
  char *xgobidir;
  FILE *fp;
  extern xgobidata xgobi;  /* defined in xgvis.c */
  Dimension width, height;

  xgobidir = getenv("XGOBID");
  if (xgobidir == NULL || strlen(xgobidir) == 0)
  {
    xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
    (void) strcpy(xgobidir, XGOBI_DEFAULTDIR);
    if (xgobidir == NULL || strlen(xgobidir) == 0)
    {
      sprintf(message,
       "XGOBID is not defined in your environment, and\n");
      strcat(message,
       "XGOBI_DEFAULTDIR is not defined in the XGobi Makefile;\n");
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
  (void) strcat(fname, "/help/xgvis_MDS_background");
  if ((fp = fopen(fname, "r")) == NULL)
  {
    sprintf(message,
      "Unable to open %s.\n", fname);
    strcat(message,
      "Is the shell variable XGOBID the name of the directory\n");
    strcat(message,
      "which contains the help subdirectory? \n");
    show_message(message, &xgobi);
    return((XtCallbackProc) 0);
  }
  else {
    Widget hpopup, hframe, htext, hdone;

    /* 80 columns x 20 rows, I hope */
    width = 45 * XTextWidth(appdata.helpFont, "M", 1) ;
    height = 30 * FONTHEIGHT(appdata.helpFont);

  /*
   * Create the popup itself.
  */
    hpopup = XtVaCreatePopupShell("Help",
      topLevelShellWidgetClass, shell,
      XtNtitle, (String) "XGvis Help Window",
      XtNiconName, (String) "XGvis Help Window",
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
      XtNscrollVertical, (XawTextScrollMode) XawtextScrollAlways,
      XtNallowResize, (Boolean) True,
      XtNshowGrip, (Boolean) False,
      XtNtype, (XawAsciiType) XawAsciiFile,
      XtNstring, (String) fname,
      XtNdisplayCaret, (Boolean) False,
      XtNfont, (XFontStruct *) appdata.helpFont,
      XtNheight, (Dimension) height,
      XtNwidth, (Dimension) width,
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
      (XtCallbackProc) xgvis_help_done_cback, (XtPointer) NULL);

    XtPopup(hpopup, XtGrabNone);
    XRaiseWindow(display, XtWindow(hpopup));

    set_wm_protocols(hpopup);

    fclose(fp);
  }
}


/* ARGSUSED */
XtCallbackProc
xgvis_help_controls_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  char fname[100];
  char message[MSGLENGTH];
  char *xgobidir;
  FILE *fp;
  extern xgobidata xgobi;  /* defined in xgvis.c */
  Dimension width, height;

  xgobidir = getenv("XGOBID");
  if (xgobidir == NULL || strlen(xgobidir) == 0)
  {
    xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
    (void) strcpy(xgobidir, XGOBI_DEFAULTDIR);
    if (xgobidir == NULL || strlen(xgobidir) == 0)
    {
      sprintf(message,
       "XGOBID is not defined in your environment, and\n");
      strcat(message,
       "XGOBI_DEFAULTDIR is not defined in the XGobi Makefile;\n");
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
  (void) strcat(fname, "/help/xgvis_controls");
  if ((fp = fopen(fname, "r")) == NULL)
  {
    sprintf(message,
      "Unable to open %s.\n", fname);
    strcat(message,
      "Is the shell variable XGOBID the name of the directory\n");
    strcat(message,
      "which contains the help subdirectory? \n");
    show_message(message, &xgobi);
    return((XtCallbackProc) 0);
  }
  else {
    Widget hpopup, hframe, hform, hfunc, htext, hdone;

    /* 80 columns x 20 rows, I hope */
    width = 45 * XTextWidth(appdata.helpFont, "M", 1) ;
    height = 30 * FONTHEIGHT(appdata.helpFont);

  /*
   * Create the popup itself.
  */
    hpopup = XtVaCreatePopupShell("Help",
      topLevelShellWidgetClass, shell,
      XtNtitle, (String) "XGvis Help Window",
      XtNiconName, (String) "XGvis Help Window",
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
      XtNscrollVertical, (XawTextScrollMode) XawtextScrollAlways,
      XtNallowResize, (Boolean) True,
      XtNshowGrip, (Boolean) False,
      XtNtype, (XawAsciiType) XawAsciiFile,
      XtNstring, (String) fname,
      XtNdisplayCaret, (Boolean) False,
      XtNfont, (XFontStruct *) appdata.helpFont,
      XtNheight, (Dimension) height,
      XtNwidth, (Dimension) width,
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
      (XtCallbackProc) xgvis_help_done_cback, (XtPointer) NULL);

    XtPopup(hpopup, XtGrabNone);
    XRaiseWindow(display, XtWindow(hpopup));

    set_wm_protocols(hpopup);

    fclose(fp);
  }
}


/* ARGSUSED */
XtCallbackProc
xgvis_help_torgerson_gower_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  char fname[100];
  char message[MSGLENGTH];
  char *xgobidir;
  FILE *fp;
  extern xgobidata xgobi;  /* defined in xgvis.c */
/*
  Dimension height;
*/

  xgobidir = getenv("XGOBID");
  if (xgobidir == NULL || strlen(xgobidir) == 0)
  {
    xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
    (void) strcpy(xgobidir, XGOBI_DEFAULTDIR);
    if (xgobidir == NULL || strlen(xgobidir) == 0)
    {
      sprintf(message,
       "XGOBID is not defined in your environment, and\n");
      strcat(message,
       "XGOBI_DEFAULTDIR is not defined in the XGobi Makefile;\n");
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
  (void) strcat(fname, "/bitmaps/strain_torgerson_gower.xbm");
  if ((fp = fopen(fname, "r")) == NULL)
  {
    sprintf(message,
      "Unable to open %s.\n", fname);
    strcat(message,
      "Is the shell variable XGOBID the name of the directory\n");
    strcat(message,
      "which contains the help subdirectory? \n");
    show_message(message, &xgobi);
    return((XtCallbackProc) 0);
  }
  else {
    Widget hpopup, hframe, hform, hfunc, hdone;
    Screen *scrn;
    Pixmap func_pix;
    Pixel funcfg, funcbg;

    /*
     * Read in the bitmap of the function.
    */
    scrn = XtScreen(shell);
    funcbg = WhitePixelOfScreen(scrn);
    funcfg = BlackPixelOfScreen(scrn);

    func_pix = XCreatePixmapFromBitmapData(display,
      RootWindowOfScreen(scrn),
      strain_torgerson_gower_bits, 
      strain_torgerson_gower_width, 
      strain_torgerson_gower_height,
      funcfg, funcbg,
      depth);

    /* 80 columns x 20 rows, I hope */
    /* width = 85 * XTextWidth(appdata.helpFont, "M", 1) ; */
/*
    height = 20 * FONTHEIGHT(appdata.helpFont);
*/

  /*
   * Create the popup itself.
  */
    hpopup = XtVaCreatePopupShell("Help",
      topLevelShellWidgetClass, shell,
      XtNtitle, (String) "XGvis Help Window",
      XtNiconName, (String) "XGvis Help Window",
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
   * Create a form widget to hold the label and text.
  */
    hform = XtVaCreateManagedWidget("Help",
      formWidgetClass, hframe,
      XtNbackground, funcbg,
      NULL);

    hfunc = XtVaCreateManagedWidget("Help",
      labelWidgetClass, hform,
      XtNinternalHeight, (Dimension) 0,
      XtNinternalWidth, (Dimension) 0,
      /* --------- bitmap goes here ---------------- */
      XtNbitmap, (Pixmap) func_pix,
      XtNborderWidth, 0,
      NULL);

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
      (XtCallbackProc) xgvis_help_done_cback, (XtPointer) NULL);

    XtPopup(hpopup, XtGrabNone);
    XRaiseWindow(display, XtWindow(hpopup));

    set_wm_protocols(hpopup);

    fclose(fp);
  }
}


/* ARGSUSED */
XtCallbackProc
xgvis_help_kruskal_shepard_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  char fname[100];
  char message[MSGLENGTH];
  char *xgobidir;
  FILE *fp;
  extern xgobidata xgobi;  /* defined in xgvis.c */
/*
  Dimension height;
*/

  xgobidir = getenv("XGOBID");
  if (xgobidir == NULL || strlen(xgobidir) == 0)
  {
    xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
    (void) strcpy(xgobidir, XGOBI_DEFAULTDIR);
    if (xgobidir == NULL || strlen(xgobidir) == 0)
    {
      sprintf(message,
       "XGOBID is not defined in your environment, and\n");
      strcat(message,
       "XGOBI_DEFAULTDIR is not defined in the XGobi Makefile;\n");
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
  (void) strcat(fname, "/bitmaps/stress_kruskal_shepard.xbm");
  if ((fp = fopen(fname, "r")) == NULL)
  {
    sprintf(message,
      "Unable to open %s.\n", fname);
    strcat(message,
      "Is the shell variable XGOBID the name of the directory\n");
    strcat(message,
      "which contains the help subdirectory? \n");
    show_message(message, &xgobi);
    return((XtCallbackProc) 0);
  }
  else {
    Widget hpopup, hframe, hform, hfunc, hdone;
    Screen *scrn;
    Pixmap func_pix;
    Pixel funcfg, funcbg;

    /*
     * Read in the bitmap of the function.
    */
    scrn = XtScreen(shell);
    funcbg = WhitePixelOfScreen(scrn);
    funcfg = BlackPixelOfScreen(scrn);

    func_pix = XCreatePixmapFromBitmapData(display,
      RootWindowOfScreen(scrn),
      stress_kruskal_shepard_bits, 
      stress_kruskal_shepard_width, 
      stress_kruskal_shepard_height,
      funcfg, funcbg,
      depth);

    /* 80 columns x 20 rows, I hope */
    /* width = 85 * XTextWidth(appdata.helpFont, "M", 1) ; */
/*
    height = 20 * FONTHEIGHT(appdata.helpFont);
*/

  /*
   * Create the popup itself.
  */
    hpopup = XtVaCreatePopupShell("Help",
      topLevelShellWidgetClass, shell,
      XtNtitle, (String) "XGvis Help Window",
      XtNiconName, (String) "XGvis Help Window",
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
   * Create a form widget to hold the label and text.
  */
    hform = XtVaCreateManagedWidget("Help",
      formWidgetClass, hframe,
      XtNbackground, funcbg,
      NULL);

    hfunc = XtVaCreateManagedWidget("Help",
      labelWidgetClass, hform,
      XtNinternalHeight, (Dimension) 0,
      XtNinternalWidth, (Dimension) 0,
      /* --------- bitmap goes here ---------------- */
      XtNbitmap, (Pixmap) func_pix,
      XtNborderWidth, 0,
      NULL);

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
      (XtCallbackProc) xgvis_help_done_cback, (XtPointer) NULL);

    XtPopup(hpopup, XtGrabNone);
    XRaiseWindow(display, XtWindow(hpopup));

    set_wm_protocols(hpopup);

    fclose(fp);
  }
}


/* ARGSUSED */
XtCallbackProc
xgvis_help_input_file_formats_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  char fname[100];
  char message[MSGLENGTH];
  char *xgobidir;
  FILE *fp;
  extern xgobidata xgobi;  /* defined in xgvis.c */
  Dimension width, height;

  xgobidir = getenv("XGOBID");
  if (xgobidir == NULL || strlen(xgobidir) == 0)
  {
    xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
    (void) strcpy(xgobidir, XGOBI_DEFAULTDIR);
    if (xgobidir == NULL || strlen(xgobidir) == 0)
    {
      sprintf(message,
       "XGOBID is not defined in your environment, and\n");
      strcat(message,
       "XGOBI_DEFAULTDIR is not defined in the XGobi Makefile;\n");
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
  (void) strcat(fname, "/help/xgvis_input_file_formats");
  if ((fp = fopen(fname, "r")) == NULL)
  {
    sprintf(message,
      "Unable to open %s.\n", fname);
    strcat(message,
      "Is the shell variable XGOBID the name of the directory\n");
    strcat(message,
      "which contains the help subdirectory? \n");
    show_message(message, &xgobi);
    return((XtCallbackProc) 0);
  }
  else {
    Widget hpopup, hframe, htext, hdone;

    /* 80 columns x 20 rows, I hope */
    width = 45 * XTextWidth(appdata.helpFont, "M", 1) ;
    height = 30 * FONTHEIGHT(appdata.helpFont);

  /*
   * Create the popup itself.
  */
    hpopup = XtVaCreatePopupShell("Help",
      topLevelShellWidgetClass, shell,
      XtNtitle, (String) "XGvis Help Window",
      XtNiconName, (String) "XGvis Help Window",
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
      XtNscrollVertical, (XawTextScrollMode) XawtextScrollAlways,
      XtNallowResize, (Boolean) True,
      XtNshowGrip, (Boolean) False,
      XtNtype, (XawAsciiType) XawAsciiFile,
      XtNstring, (String) fname,
      XtNdisplayCaret, (Boolean) False,
      XtNfont, (XFontStruct *) appdata.helpFont,
      XtNheight, (Dimension) height,
      XtNwidth, (Dimension) width,
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
      (XtCallbackProc) xgvis_help_done_cback, (XtPointer) NULL);

    XtPopup(hpopup, XtGrabNone);
    XRaiseWindow(display, XtWindow(hpopup));

    set_wm_protocols(hpopup);

    fclose(fp);
  }
}


