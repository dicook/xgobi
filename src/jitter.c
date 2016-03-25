/* jitter.c */
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

#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#define JITFAC .2
static float jitfac = JITFAC;
float *jitfacv = (float *) NULL;

#define UNIFORM 0
#define NORMAL  1

static Widget jpopup = (Widget) NULL;
static Widget jittype[2];
static Boolean jitter_type = UNIFORM;
static Boolean jitter_vgroup = True;
Widget rejitter_cmd;

Boolean convex_jittering = True;

void
init_jitfac(xgobidata *xg)
{
  int j;

  if (xg->is_missing_values_xgobi)
    jitfac = JITFAC;
  else
    jitfac = 0.;

/*
 * If there's going to be a different jitfac for each column ...
 *  The tricky part is that when a new variable is selected, if
 *  the jitter window is open, it should reset the scrollbar for
 *  the appropriate jitfac.  But it isn't quite possible, really,
 *  given that the two or more variables in the window might have
 *  very different jitfacs.
*/
  jitfacv = (float *) XtMalloc((unsigned) xg->ncols * sizeof(float));
  for (j=0; j<xg->ncols; j++)
    jitfacv[j] = jitfac;
}

float
randval(int type)
{
/*
 * generate a random value.
*/
  double drand;
  static double dsave;
  static Boolean isave = false;

  if (type == UNIFORM) {
    drand = randvalue();
    /*
     * Center and scale to [-1, 1]
    */
    drand = (drand - .5) * 2;

  } else if (type == NORMAL) {

    Boolean check = true;
    double d, dfac;

    if (isave) {
      isave = false;
      /* prepare to return the previously saved value */
      drand = dsave;
    } else {
      isave = true;
      while (check) {

        rnorm2(&drand, &dsave);
        d = drand*drand + dsave*dsave;

        if (d < 1.0) {
          check = false;
          dfac = sqrt(-2. * log(d)/d);
          drand = drand * dfac;
          dsave = dsave * dfac;
        }
      } /* end while */
    } /* end else */

    /*
     * Already centered; scale to approximately [-1, 1]
    */
    drand = (drand / 3.0);
  }
  return((float)drand);
}

void
jitter_data(xgobidata *xg)
{
  int *cols;
  int i, j, k, m, ncols = 0;
  int nvgroups = numvargroups(xg);

/*
 * First determine the variables to be jittered:
 * this depends first on plotting mode and second on
 * vgroups (if jitter_vgroup is True)
*/

  cols = (int *) XtMalloc((Cardinal) xg->ncols * sizeof(int));
  ncols = find_selected_cols(xg, cols);

/*
 * Then vgroups ...  and the missing values xgobi should have
 * already been set up with vgroups = (0, ..., 0)
*/
  if (jitter_vgroup && nvgroups < xg->ncols_used)
    add_vgroups(xg, cols, &ncols);

  for (i=0; i<xg->nrows_in_plot; i++) {
    if ((m = xg->rows_in_plot[i]) >= xg->nlinkable)
      break;
    for (j=0; j<ncols; j++) {
      k = cols[j];
      jitfacv[k] = jitfac;

      jitter_one_value(m, k, xg);
    }
  }

  XtFree((char *) cols);
}

void
jitter_one_value(int m, int k, xgobidata *xg)
  /* m is the row index, k is the column index */
{
  long lrand, worldx;
  float rjit;

  lrand = (long) (randval(jitter_type) * (float) PRECISION1);

  /*
   * Confound: the world_data used here is already jittered:
   * subtract out the previous jittered value ...
  */
  if (convex_jittering) {
    worldx = xg->world_data[m][k] - xg->jitter_data[m][k];
    rjit = jitfacv[k] * (float) (lrand - worldx);
  }
  else
    rjit = jitfacv[k] * (float) lrand;

  xg->jitter_data[m][k] = (long) rjit;
}

static void
setToggles(void) {
  setToggleBitmap(jittype[0], jitter_type == UNIFORM);
  setToggleBitmap(jittype[1], jitter_type == NORMAL);
}

/* ARGSUSED */
XtCallbackProc
normal_jitter_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  Boolean set;

  XtVaGetValues(w, XtNstate, &set, NULL);
  if (set) {
    jitter_type = NORMAL;
    XtCallCallbacks(rejitter_cmd, XtNcallback, (XtPointer) xg);
  }
  setToggles();
}

/* ARGSUSED */
XtCallbackProc
uniform_jitter_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  Boolean set;

  XtVaGetValues(w, XtNstate, &set, NULL);
  if (set) {
    jitter_type = UNIFORM;
    XtCallCallbacks(rejitter_cmd, XtNcallback, (XtPointer) xg);
  }
  setToggles();
}

void
jitter_again(xgobidata *xg) {
  XtCallCallbacks(rejitter_cmd, XtNcallback, (XtPointer) xg);
}

/* ARGSUSED */
XtCallbackProc
jitfac_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  float slidepos = * (float *) slideposp;

  /* slidepos is on [0,1]; jitfac is also on [0,1] for now */
  jitfac = (float) slidepos * (float) slidepos;
  XtCallCallbacks(rejitter_cmd, XtNcallback, (XtPointer) xg);
}

/* ARGSUSED */
XtCallbackProc
rejitter_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  jitter_data(xg);

  /* jitter is added to world_data in tform_to_world and sphered_to_world */
  update_world(xg);

  world_to_plane(xg);
  plane_to_screen(xg);

  if (xg->is_brushing) {
    assign_points_to_bins(xg);
    if (xg->brush_mode == transient)
      reinit_transient_brushing(xg);
  }

  plot_once(xg);
  if (xg->is_cprof_plotting && xg->link_cprof_plotting)
    update_cprof_plot(xg);
}

/* ARGSUSED */
static XtCallbackProc
close_jit_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtPopdown(jpopup);
}

/* ARGSUSED */
static XtCallbackProc
use_vgroups_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  jitter_vgroup = !jitter_vgroup;

  setToggleBitmap(w, jitter_vgroup);
}

/* ARGSUSED */
XtCallbackProc
open_jitter_popup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  static Boolean initd = False;
  Widget mframe, mform;
  Widget use_vgroups_cmd, close_cmd;
  Widget jitfac_label, jitfac_sbar;
  Widget jittype_box;
  char str[512];

  if (!initd)
  {
    Dimension width, height;
    Position x, y;

    XtVaGetValues(w,
      XtNwidth, &width,
      XtNheight, &height, NULL);
    XtTranslateCoords(w,
      (Position) (width/2), (Position) (height/2), &x, &y);

    /*
     * Create the missing data popup
    */
    jpopup = XtVaCreatePopupShell("Jitter",
      /*transientShellWidgetClass, XtParent(w),*/
      topLevelShellWidgetClass, XtParent(w),
      XtNinput,            (Boolean) True,
      XtNallowShellResize, (Boolean) True,
      XtNtitle,            (String) "Jitter data",
      XtNiconName,         (String) "Jitter",
      XtNx,                x,
      XtNy,                y,
      NULL);
    if (mono) set_mono(jpopup);

    /*
     * Create a paned widget so the 'Click here ...'
     * can be all across the bottom.
    */
    mframe = XtVaCreateManagedWidget("Form",
      panedWidgetClass, jpopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      NULL);
    /*
     * Create the form widget.
    */
    mform = XtVaCreateManagedWidget("Jitter",
      formWidgetClass, mframe,
      NULL);
    if (mono) set_mono(mform);

    init_jitfac(xg);

    sprintf(str, "Degree of jitter");
    width = XTextWidth(appdata.font, str, strlen(str)) +
      2*ASCII_TEXT_BORDER_WIDTH;
    jitfac_label = XtVaCreateManagedWidget("Jitter",
      labelWidgetClass, mform,
      XtNlabel, str,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(jitfac_label);

    jitfac_sbar = XtVaCreateManagedWidget("Scrollbar",
      scrollbarWidgetClass, mform,
      XtNfromVert, (Widget) jitfac_label,
      XtNvertDistance, (int) 0,
      XtNwidth, width,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    if (mono) set_mono(jitfac_sbar);
    add_sbar_help(&xg->nhelpids.sbar,
      jitfac_sbar, "Jitter");
    XawScrollbarSetThumb(jitfac_sbar, jitfac, -1.);
    XtAddCallback(jitfac_sbar, XtNjumpProc,
      (XtCallbackProc) jitfac_cback, (XtPointer) xg);

    rejitter_cmd = (Widget) CreateCommand(xg, "Rejitter",
      True, (Widget) NULL, (Widget) jitfac_sbar,
      mform, "Jitter");
    XtVaSetValues(rejitter_cmd,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(rejitter_cmd);
    XtAddCallback(rejitter_cmd, XtNcallback,
      (XtCallbackProc) rejitter_cback, (XtPointer) xg);
 
    /* Uniform or normal variates? */

    jittype_box = XtVaCreateManagedWidget("Panel",
      boxWidgetClass, mform,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNfromVert, rejitter_cmd,
      NULL);
    if (mono) set_mono(jittype_box);

    jittype[UNIFORM] = (Widget) CreateToggle(xg, "Uniform",
      True, (Widget) NULL, (Widget) NULL, (Widget) NULL,
      True, ONE_OF_MANY, jittype_box, "Jitter");
    jittype[NORMAL] = (Widget) CreateToggle(xg, "Normal",
      True, (Widget) NULL, (Widget) NULL, (Widget) jittype[UNIFORM],
      False, ONE_OF_MANY, jittype_box, "Jitter");
    XtManageChildren(jittype, 2);

    XtAddCallback(jittype[UNIFORM], XtNcallback,
      (XtCallbackProc) uniform_jitter_cback, (XtPointer) xg);
    XtAddCallback(jittype[NORMAL], XtNcallback,
      (XtCallbackProc) normal_jitter_cback, (XtPointer) xg);

    use_vgroups_cmd = (Widget) CreateToggle(xg, "Jitter vgroup",
      True, (Widget) NULL, (Widget) jittype_box, (Widget) NULL,
      jitter_vgroup, ANY_OF_MANY, mform, "Jitter");
    XtManageChild(use_vgroups_cmd);
    XtAddCallback(use_vgroups_cmd, XtNcallback,
      (XtCallbackProc) use_vgroups_cback, (XtPointer) xg);

    close_cmd = XtVaCreateManagedWidget("Close",
      commandWidgetClass, mframe,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close_cmd);
    XtAddCallback(close_cmd, XtNcallback,
      (XtCallbackProc) close_jit_cback, (XtPointer) xg);
  }

  XtPopup(jpopup, (XtGrabKind) XtGrabNone);
  XRaiseWindow(display, XtWindow(jpopup));

  if (!initd)
  {
    set_wm_protocols(jpopup);
    initd = True;
  }
}
