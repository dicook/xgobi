/* subset.c */
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

/*
 * November 5, 1999: Modifying to use nlinkable instead of
 * nrows.  The rows beyond nlinkable will not now be sampled;
 * however, they will all be included in each sample.
 *
 * Also, modifying to sample rgroup_ids when rgroups are
 * being used.
*/


/*************************************************************
 * Modified by James Brook, Oct 1994
 * British Telecommunications Labs, UK, jamesb@aom.bt.co.uk
 * Want to add another option to the subsetting panel, to
 * choose all rows with a particular identifier.
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

#include "../bitmaps/rightarrow20.xbm"
#include "../bitmaps/leftarrow20.xbm"

static Widget spopup;
static Widget from_text, to_text;
static char *from_str, *blocksize_str;
static Widget sample_text;
static char *sample_str;
static Widget everyn_text, everyn_init_text;
static char *everyn_str, *everyn_init_str;

/*************************************************************
 * Added by James Brook, Oct 1994
 ************************************************************/
static Widget id_label, id_text;
static char *id_str;
/************************************************************/

static Widget apply, restore, close;
static Boolean rescale = False;

#define NSTYPEBTNS 5
#define BLOCK  0
#define SAMPLE 1
#define EVERYN 2
#define STICKY 3
#define ROWID  4
static int subset_type = SAMPLE;
/*****/
static Widget stype_menu_cmd, stype_menu, stype_menu_btn[NSTYPEBTNS];
static Widget stype_menu_box, stype_menu_lab;
static char *stype_menu_str = "Subsetting method:";
static char *stype_menu_name[] = {
  "Consecutive block",
  "Random sample without replacement",
  "Every nth case",
  "Cases with row label == one of the current sticky labels",
  "Cases with the specified row label",
};
static char *stype_menu_nickname[] = {
  "Consecutive block",
  "Random sample",
  "Every nth case",
  "Sticky labels",
  "Specified cases",
};

static Widget stype_box[NSTYPEBTNS];

static void
clear_subset(xgobidata *xg) {
  int i, rgid;

  xg->nrows_in_plot = 0;

  for (i=0; i<xg->nlinkable; i++)
    xg->erased[i] = 1;

  if (xg->nrgroups > 0) {
    rgid = xg->rgroup_ids[i];
    xg->rgroups[rgid].excluded = True;
  }
}

static void
add_to_subset(int i, xgobidata *xg) {

  if (xg->nrgroups > 0) {
    int j, el;

    xg->rgroups[i].excluded = False;
    for (j=0; j<xg->rgroups[i].nels; j++) {
      el = xg->rgroups[i].els[j];
      xg->erased[el] = 0;
      xg->rows_in_plot[xg->nrows_in_plot++] = el;
    }

  } else {
    xg->erased[i] = 0;
    xg->rows_in_plot[xg->nrows_in_plot++] = i;
  }
}

/* ARGSUSED */
static XtCallbackProc
rescale_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  rescale = !rescale;
  setToggleBitmap(w, rescale);
}

/*
 * This algorithm taken from Knuth, Seminumerical Algorithms;
 * Vol 2 of his series.
*/
int
sample_xgobi(int n, xgobidata *xg) {
  int t, m;
  int doneit = 0;
  float rrand;

/*
 * dfs 5 November 99, replacing nrows with nlinkable,
 * enabling rgroups subsetting
*/

  int top = (xg->nrgroups > 0) ? xg->nrgroups : xg->nlinkable;

  if (n > 0 && n < top) {

    for (t=0, m=0; t<top && m<n; t++)
    {
      rrand = (float) randvalue();
      if ( ((top - t) * rrand) < (n - m) ) {
        add_to_subset(t, xg);
        m++;
      }
    }

    doneit = 1;
  }
  return(doneit);
}

/* ARGSUSED */
XtCallbackProc
ahead_block_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int from_n, bsize;
  char str[32];
  int top = (xg->nrgroups > 0) ? xg->nrgroups : xg->nlinkable;

  XtVaGetValues(from_text, XtNstring, (String) &from_str, NULL);
  XtVaGetValues(to_text, XtNstring, (String) &blocksize_str, NULL);

  from_n = atoi(from_str);
  bsize = atoi(blocksize_str);

  from_n += bsize/2;
  from_n = MIN(from_n, top-2);

  sprintf(str, "%d", from_n);
  XtVaSetValues(from_text, XtNstring, str, NULL);
  sprintf(str, "%d", bsize);
  XtVaSetValues(to_text, XtNstring, str, NULL);

  XtCallCallbacks(apply, XtNcallback, (XtPointer) xg);
}
/* ARGSUSED */
XtCallbackProc
back_block_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int from_n, bsize;
  char str[32];

  XtVaGetValues(from_text, XtNstring, (String) &from_str, NULL);
  XtVaGetValues(to_text, XtNstring, (String) &blocksize_str, NULL);

  from_n = atoi(from_str);
  bsize = atoi(blocksize_str);
  
  from_n -= bsize/2;
  if (from_n < 1) from_n = 1;

  sprintf(str, "%d", from_n);
  XtVaSetValues(from_text, XtNstring, str, NULL);
  sprintf(str, "%d", bsize);
  XtVaSetValues(to_text, XtNstring, str, NULL);

  XtCallCallbacks(apply, XtNcallback, (XtPointer) xg);
}

/* ARGSUSED */
XtCallbackProc
subset_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i, j, k;
  int doneit = 0;
  int from_n, to_n;
  int sample_size;
  int everyn_n, everyn_init;

  /*int prev_nrows_in_plot = xg->nrows_in_plot;*/
  int top = (xg->nrgroups > 0) ? xg->nrgroups : xg->nlinkable;

  switch (subset_type) {

    case BLOCK:
      XtVaGetValues(from_text, XtNstring, (String) &from_str, NULL);
      XtVaGetValues(to_text, XtNstring, (String) &blocksize_str, NULL);

      from_n = atoi(from_str);
      to_n = from_n + atoi(blocksize_str) - 1;
      to_n = (to_n >= top) ? top-1 : to_n;

      if (to_n > 0 && to_n <= top &&
          from_n > 0 && from_n < top &&
          from_n < to_n)
      {
        clear_subset(xg);

        for (i=(from_n - 1), k=0; i<to_n; i++, k++)
          add_to_subset(i, xg);

        xg->delete_erased_pts = True;
        doneit = 1;
      }
      else {
        char message[MSGLENGTH];
        sprintf(message, "The limits aren't correctly specified.\n");
        show_message(message, xg);
      }
      break;

    case SAMPLE:

      XtVaGetValues(sample_text, XtNstring, (String) &sample_str, NULL);
      sample_size = atoi(sample_str);
      if (sample_size > 0) {
        clear_subset(xg);
        doneit = sample_xgobi(sample_size, xg);
      }
      break;

    case EVERYN:
      XtVaGetValues(everyn_text, XtNstring, (String) &everyn_str, NULL);
      everyn_n = atoi(everyn_str);

      XtVaGetValues(everyn_init_text,
        XtNstring, (String) &everyn_init_str, NULL);
      everyn_init = atoi(everyn_init_str);

      if (everyn_n > 0 && everyn_n < top &&
        everyn_init > 0 && everyn_init < (top-1) )
      {
        clear_subset(xg);

        for (i=(everyn_init-1), k=0; i<top; i=i+everyn_n, k++)
          add_to_subset(i, xg);

        doneit = 1;
      }
      else
      {
        char message[MSGLENGTH];
        sprintf(message, "Interval 'n' not correctly specified.\n");
        show_message(message, xg);
      }
      break;

    case STICKY:

      /*************************************************************
       * Added by James Brook, Oct 1994
       ************************************************************/

      clear_subset(xg);

      for (i=0; i<xg->nlinkable; i++) {
        for (j=0; j<xg->nsticky_ids; j++) {
          if (!strcmp(xg->rowlab[i], xg->rowlab[xg->sticky_ids[j]])) {

            if (xg->nrgroups > 0)
              add_to_subset(xg->rgroup_ids[i], xg);
            else
              add_to_subset(i, xg);

            break;
          }
        }
      }

      xg->delete_erased_pts = True;
      doneit = 1;
      break;

    case ROWID:

      /***************************************************************
       * Could use the same format as the others, i.e. loop over rows 
       * setting them erased then unerasing the ones we want, but it
       * seems quicker this way
       ***************************************************************/

      XtVaGetValues(id_text, XtNstring, (String) &id_str, NULL);

      clear_subset(xg);

      for (i=0; i<xg->nlinkable; i++) {
        if (!strcmp(xg->rowlab[i], id_str)) {
          if (xg->nrgroups > 0)
            add_to_subset(xg->rgroup_ids[i], xg);
          else
            add_to_subset(i, xg);
        }
      }

      xg->delete_erased_pts = True;
      doneit = 1;
  /************************************************************/
  }

  if (doneit)
  {
    /* add all the unlinkable guys to the sample */
    for (i=xg->nlinkable; i<xg->nrows; i++) {
      xg->rows_in_plot[xg->nrows_in_plot++] = i;
    }

    update_nrgroups_in_plot(xg);
    reset_rows_in_plot(xg, rescale);

    copy_brushinfo_to_senddata(xg);
    assign_points_to_bins(xg);
    plot_once(xg);

    if (xg->is_cprof_plotting && xg->link_cprof_plotting)
      passive_update_cprof_plot(xg);
  }
}

/* ARGSUSED */
XtCallbackProc
restore_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i;

  for (i=0; i<xg->nrows; i++)
  {
    xg->erased[i] = 0;
    xg->rows_in_plot[i] = i;
  }
  xg->nrows_in_plot = xg->nrows;

  update_nrgroups_in_plot(xg);
  reset_rows_in_plot(xg, rescale);

  copy_brushinfo_to_senddata(xg);
  assign_points_to_bins(xg);
  plot_once(xg);

  if (xg->is_cprof_plotting && xg->link_cprof_plotting)
    passive_update_cprof_plot(xg);
}

/* ARGSUSED */
static XtCallbackProc
close_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtPopdown(spopup);
}

/* ARGSUSED */     
static XtCallbackProc
stype_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;      

  if (subset_type != STICKY)
    XtUnmapWidget(stype_box[subset_type]);

  for (btn=0; btn<NSTYPEBTNS; btn++)
    if (stype_menu_btn[btn] == w)
      break;

  XtVaSetValues(stype_menu_cmd,
    XtNlabel, stype_menu_nickname[btn],
    NULL);
  subset_type = btn;

  if (subset_type != STICKY)
    XtMapWidget(stype_box[subset_type]);
}


/* ARGSUSED */
XtCallbackProc
subset_panel_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Widget sframe, sform, paramform;
  Widget sample_label;
  Widget rescale_cmd;
  Widget from_label, to_label;
  Widget aheadw, backw;
  Widget everyn_label;
  Widget everyn_init_label;
  Dimension width, height;
  Position x, y;
  static int initd = 0;
  int k;

  if (!initd)
  {
    Pixmap rightarr20, leftarr20;
    rightarr20 = XCreatePixmapFromBitmapData(display,
      DefaultRootWindow(display),
      (char *) rightarrow20_bits, rightarrow20_width, rightarrow20_height,
      appdata.fg, appdata.bg, depth);
    leftarr20 = XCreatePixmapFromBitmapData(display,
      DefaultRootWindow(display),
      (char *) leftarrow20_bits, leftarrow20_width, leftarrow20_height,
      appdata.fg, appdata.bg, depth);

    from_str = (char *) XtMalloc((Cardinal) 20 * sizeof(char));
    blocksize_str = (char *) XtMalloc((Cardinal) 20 * sizeof(char));
    everyn_str = (char *) XtMalloc((Cardinal) 20 * sizeof(char));
    everyn_init_str = (char *) XtMalloc((Cardinal) 20 * sizeof(char));
    sample_str = (char *) XtMalloc((Cardinal) 20 * sizeof(char));
    /*************************************************************
     * Added by James Brook, Oct 1994
     ************************************************************/
    id_str = (char *) XtMalloc((Cardinal) 32 * sizeof(char));
    /*****************************************************************/
    
    (void) sprintf(from_str, "1");
    if (xg->nrgroups) {
      (void) sprintf(blocksize_str, "%d", (int) (xg->nrgroups/5));
      (void) sprintf(sample_str, "%d", xg->nrgroups);
    } else {
      (void) sprintf(blocksize_str, "%d", (int) (xg->nrows/10));
      (void) sprintf(sample_str, "%d", xg->nrows_in_plot);
    }
    (void) sprintf(everyn_str, "1");
    (void) sprintf(everyn_init_str, "1");
    /*************************************************************
     * Added by James Brook, Oct 1994
     ************************************************************/
    (void) sprintf(id_str, xg->rowlab[0]);
    /*****************************************************************/

    XtVaGetValues(xg->shell,
      XtNwidth, &width,
      XtNheight, &height, NULL);
    XtTranslateCoords(xg->shell,
      (Position) (width/2), (Position) (height/2), &x, &y);
    /*
     * Create the popup to solicit subsetting arguments.
    */
    spopup = XtVaCreatePopupShell("Form0",
      /*transientShellWidgetClass, w,*/
      topLevelShellWidgetClass, xg->shell,
      XtNx, (Position) x,
      XtNy, (Position) y,
      XtNinput, (Boolean) True,
      /*XtNallowShellResize, (Boolean) False,*/
      XtNtitle, (String) "Subset data",
      XtNiconName, (String) "Subset",
      NULL);
    if (mono) set_mono(spopup);

    /*
     * Create a paned widget so the 'Click here ...'
     * can be all across the bottom.
    */
    sframe = XtVaCreateManagedWidget("Form",
      panedWidgetClass, spopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      NULL);

    /*
     * Create the form widget.
    */
    sform = XtVaCreateManagedWidget("Form",
      formWidgetClass, sframe,
      NULL);
    if (mono) set_mono(sform);

    paramform = XtVaCreateManagedWidget("Param",
      formWidgetClass, sform,
      NULL);
    if (mono) set_mono(paramform);

    build_labelled_menu(&stype_menu_box, &stype_menu_lab, stype_menu_str,
      &stype_menu_cmd, &stype_menu, stype_menu_btn,
      stype_menu_name, stype_menu_nickname,
      NSTYPEBTNS, subset_type, paramform, NULL,
      XtorientHorizontal, appdata.font, "Subset", xg);
    for (k=0; k<NSTYPEBTNS; k++)
      XtAddCallback(stype_menu_btn[k],  XtNcallback,
        (XtCallbackProc) stype_menu_cback, (XtPointer) xg);

    for (k=0; k<NSTYPEBTNS; k++) {
      stype_box[k] = XtVaCreateManagedWidget("Subset",
        boxWidgetClass, paramform,
        XtNfromVert, stype_menu_box,
        XtNmappedWhenManaged, (subset_type == k) ? True : False,
        XtNorientation, (XtOrientation) XtorientHorizontal,
        XtNleft, (XtEdgeType) XtChainLeft,
        XtNright, (XtEdgeType) XtChainLeft,
        XtNtop, (XtEdgeType) XtChainTop,
        XtNbottom, (XtEdgeType) XtChainTop,
        XtNhSpace, 1,
        XtNvSpace, 1,
        NULL);
      if (mono) set_mono(stype_box[k]);
    }

/* parameters for consecutive block subsetting */

    backw = XtVaCreateManagedWidget("Icon",
      commandWidgetClass, stype_box[BLOCK],
      XtNinternalHeight, (Dimension) 0,
      XtNinternalWidth, (Dimension) 0,
      XtNborderColor, (Pixel) appdata.fg,
      XtNbitmap, (Pixmap) leftarr20,
      NULL);
    XtAddCallback(backw, XtNcallback,
      (XtCallbackProc) back_block_cback, (XtPointer) xg);

    from_label = XtVaCreateManagedWidget("Subset",
      labelWidgetClass, stype_box[BLOCK],
      XtNfromHoriz, (Widget) backw,
      XtNlabel, (String) "Initial row ",
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(from_label);

    from_text = XtVaCreateManagedWidget("Subset",
      asciiTextWidgetClass, stype_box[BLOCK],
      XtNfromHoriz, (Widget) from_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) from_str,
      NULL);
    if (mono) set_mono(from_text);

    to_label = XtVaCreateManagedWidget("Subset",
      labelWidgetClass, stype_box[BLOCK],
      XtNfromHoriz, (Widget) from_text,
      XtNlabel, (String) "Block size",
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(to_label);

    to_text = XtVaCreateManagedWidget("Subset",
      asciiTextWidgetClass, stype_box[BLOCK],
      XtNfromHoriz, (Widget) to_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) blocksize_str,
      NULL);
    if (mono) set_mono(to_text);

    aheadw = XtVaCreateManagedWidget("Command",
      commandWidgetClass, stype_box[BLOCK],
      XtNfromHoriz, (Widget) to_text,
      XtNinternalHeight, (Dimension) 0,
      XtNinternalWidth, (Dimension) 0,
/*
      XtNborderColor, (Pixel) appdata.fg,
*/
      XtNbitmap, (Pixmap) rightarr20,
      NULL);
    XtAddCallback(aheadw, XtNcallback,
      (XtCallbackProc) ahead_block_cback, (XtPointer) xg);

/* parameters for random sampling */

    sample_label = XtVaCreateManagedWidget("Subset",
      labelWidgetClass, stype_box[SAMPLE],
      XtNlabel, (String) "Sample of size",
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(sample_label);

    sample_text = XtVaCreateManagedWidget("Subset",
      asciiTextWidgetClass, stype_box[SAMPLE],
      XtNfromHoriz, (Widget) sample_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) sample_str,
      NULL);
    if (mono) set_mono(sample_text);

/* parameters for "every nth row" subsetting */

    everyn_label = XtVaCreateManagedWidget("Subset",
      labelWidgetClass, stype_box[EVERYN],
      XtNlabel, (String) " n ",
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(everyn_label);

    everyn_text = XtVaCreateManagedWidget("Subset",
      asciiTextWidgetClass, stype_box[EVERYN],
      XtNfromHoriz, (Widget) everyn_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) everyn_str,
      NULL);
    if (mono) set_mono(everyn_text);

    everyn_init_label = XtVaCreateManagedWidget("Subset",
      labelWidgetClass, stype_box[EVERYN],
      XtNfromHoriz, (Widget) everyn_text,
      XtNlabel, (String) "starting with",
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(everyn_init_label);

    everyn_init_text = XtVaCreateManagedWidget("Subset",
      asciiTextWidgetClass, stype_box[EVERYN],
      XtNfromHoriz, (Widget) everyn_init_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) everyn_init_str,
      NULL);
    if (mono) set_mono(everyn_init_text);

/* parameters for "use row id" subsetting */

    id_label = XtVaCreateManagedWidget("Subset",
      labelWidgetClass, stype_box[ROWID],
      XtNlabel, (String) "Row id ",
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(id_label);

    id_text = XtVaCreateManagedWidget("Subset",
      asciiTextWidgetClass, stype_box[ROWID],
      XtNfromHoriz, (Widget) id_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) id_str,
      NULL);
    if (mono) set_mono(id_text);

    apply = (Widget) CreateCommand(xg, "Subset the data",
      1, (Widget) NULL, (Widget) paramform,
      sform, "Subset");
    XtManageChild(apply);
    XtAddCallback(apply, XtNcallback,
      (XtCallbackProc) subset_cback, (XtPointer) xg);

    rescale_cmd = (Widget) CreateToggle(xg, "Rescale", True,
      apply, paramform, (Widget) NULL, rescale, ANY_OF_MANY,
      sform, "SubsetRescale");
    XtManageChild(rescale_cmd);
    XtAddCallback(rescale_cmd, XtNcallback,
      (XtCallbackProc) rescale_cback, (XtPointer) xg);

    restore = (Widget) CreateCommand(xg, "Include all data",
      True, (Widget) rescale_cmd, (Widget) paramform,
      sform, "Subset");
    XtManageChild(restore);
    XtAddCallback(restore, XtNcallback,
      (XtCallbackProc) restore_cback, (XtPointer) xg);

    close = XtVaCreateManagedWidget("Close",
      commandWidgetClass, sframe,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close);
    XtAddCallback(close, XtNcallback,
      (XtCallbackProc) close_cback, (XtPointer) xg);
  }

  XtPopup(spopup, (XtGrabKind) XtGrabNone);
  XRaiseWindow(display, XtWindow(spopup));

  if (!initd)
  {
    set_wm_protocols(spopup);
    initd = 1;
  }
}

