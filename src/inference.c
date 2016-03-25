/* inference.c */
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

static Widget *var_cbox;
static Widget tpane, tpopup = NULL;
static Position popupx = -1, popupy = -1;
Widget sort_cmd;
Widget perm_cmd, perm_anim, perm_mult, perm_nmult;
Widget norm_cmd, norm_anim, brush_tog, vgroup_tog, restore_cmd;
static Boolean inf_vgroup = False;
static Widget *varlabel;
static int ntform_cols, *tform_cols = NULL;
/*
int
which_cols(int *cols, int varno, xgobidata *xg) {

 // Figure out which columns to transform.

  int j, ncols = 0;
  int groupno = xg->vgroup_ids[varno];

  // allocated before this is called 
  for (j=0; j<(xg->ncols-1); j++) {
    if (xg->vgroup_ids[j] == groupno)
      cols[ncols++] = j;
  }
  return(ncols);
}
*/
static void
set_initial_variable(xgobidata *xg) {
  int j, jvar, gid;

  if (xg->is_plotting1d)
    jvar = (xg->plot1d_vars.y != -1) ? xg->plot1d_vars.y : xg->plot1d_vars.x;
  else if (xg->is_xyplotting)
    jvar = xg->xy_vars.x;
  else if (xg->is_spinning)
    jvar = xg->spin_vars.x;
  else if (xg->is_touring)
    jvar = xg->tour_vars[0];
  else if (xg->is_corr_touring)
    jvar = xg->corr_xvars[0];

  gid = xg->vgroup_ids[jvar];
  for (j=0; j<xg->ncols-1; j++)
    if (xg->vgroup_ids[j] == gid) {
      XtVaSetValues(var_cbox[j], XtNstate, True, NULL);
      setToggleBitmap(var_cbox[j], True);
    }
}

static void
mean_stddev(xgobidata *xg, int *cols, int ncols, float (*stage1)(float),
  float *mean, float *stddev)
/*
 * Find the minimum and maximum values of a column or variable
 * group scaling by mean and std_width standard deviations.
 * Use the function pointer to domain_adj.
*/
{
  int i, j, n;
  double sumxi = 0.0, sumxisq = 0.0;
  double dx, dmean, dvar, dstddev;
  double dn = (double) (ncols * xg->nrows);

  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg->nrows; i++) {
      dx = (double) (*stage1)(xg->raw_data[i][j]);
      sumxi = sumxi + dx;
      sumxisq = sumxisq + dx * dx;
    }
  }
  dmean = sumxi / dn;
  dvar = (sumxisq / dn) - (dmean * dmean);
  dstddev = sqrt(dvar);

  *mean = (float) dmean;
  *stddev = (float) dstddev;
}

/*
float
median(xgobidata *xg, float **data, int *cols, int ncols)
{

// Find the minimum and maximum values of each column or variable
// group scaling by median and largest distance

  int i, j, n, np;
  float *x;
  double dmedian = 0;
  extern int fcompare(const void *, const void *);

  np = ncols * xg->nrows;
  x = (float *) XtMalloc((Cardinal) np * sizeof(float));
  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg->nrows; i++) {
      x[n*xg->nrows_in_plot + i] = data[i][j];
    }
  }

  qsort((void *) x, np, sizeof(float), fcompare);
  dmedian = ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;

  return (float) dmedian;
}


void
reset_tform(xgobidata *xg) {
  int j;

}
*/

static int 
sort_compare (float *val1, float *val2)
{
  if (*val1 < *val2) 
    return (-1);
  else if (*val1 == *val2)
    return (0);
  else 
    return (1);
}

static void
tform_response(xgobidata *xg, int *cols, int ncols)
{
  int j, n;

/*
printf("%d %d %d %d\n",
xg->vgroup_ids[0], xg->vgroup_ids[1],xg->vgroup_ids[2],xg->vgroup_ids[3]);
*/

  if (xg->ncols_used > 2)
    update_sphered(xg, cols, ncols);
  update_lims(xg);
  update_world(xg);

  world_to_plane(xg);
  plane_to_screen(xg);

  /*
    This bit of init_axes() is needed.
  */
  for (n=0; n<ncols; n++) {
    j = cols[n];
    xg->nicelim[j].min = xg->lim0[j].min;
    xg->nicelim[j].max = xg->lim0[j].max;
    SetNiceRange(j, xg);
    xg->deci[j] = set_deci(xg->tickdelta[j]);
  }

  if (xg->is_xyplotting) {
    init_ticks(&xg->xy_vars, xg);
  }
  else if (xg->is_plotting1d)
    init_ticks(&xg->plot1d_vars, xg);

  if (xg->is_brushing) {
    assign_points_to_bins(xg);
    if (xg->brush_mode == transient)
      reinit_transient_brushing(xg);
  }

  plot_once(xg);

  if (xg->is_cprof_plotting)
    update_cprof_plot(xg);
}

/* ARGSUSED */
static XtCallbackProc
close_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtDestroyWidget(tpopup);
  tpopup = NULL;
}

/* ARGSUSED */
XtCallbackProc
sort_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{
  int i, j;
  /*  create sort index  here - mallika */
  float *sort_data; /* mallika */
  /* Allocate array for sorted columns - mallika */
  sort_data = (float *) XtMalloc((Cardinal) xg->nrows * sizeof(float));

/*  for (n=0; n<ncols; n++) {*/
    j = 0;
    for (i=0; i<xg->nrows; i++)
      sort_data[i] = xg->tform1[i][j];

    qsort((char *) sort_data, xg->nrows, sizeof(float), sort_compare);
   
    for (i=0; i<xg->nrows; i++)
      xg->tform2[i][j] = sort_data[i]; 

    (void) sprintf(xg->collab_tform2[j], "sort(%s)", xg->collab_tform1[j]);
    XtVaSetValues(varlabel[j], XtNlabel, xg->collab_tform2[j], NULL);
/*  }*/
  XtFree((XtPointer) sort_data);
}

void
permute(xg)
  xgobidata *xg;
{
  int i, j, k, ii, kk;
  float f;
/*  for (n=0; n<ncols; n++) {*/
    j = 0;

    /* copy tform1[][j] into tform2[][j] */
    for (i=0; i<xg->nrows; i++)
      xg->tform2[i][j] = xg->tform1[i][j];

    /*
     * Shuffle tform2[][j]
     * Knuth, Seminumerical Algorithms, Vol2; Algorithm P.
    */
    for (i=0; i<xg->nrows_in_plot; i++) {
      k = (int) (randvalue() * (double) i);

      ii = xg->rows_in_plot[i];
      kk = xg->rows_in_plot[k];
 
      f = xg->tform2[ii][j];
      xg->tform2[ii][j] = xg->tform2[kk][j];
      xg->tform2[kk][j] = f;
    }

    (void) sprintf(xg->collab_tform2[j], "perm(%s)", xg->collab_tform1[j]);
        XtVaSetValues(varlabel[j], XtNlabel, xg->collab_tform2[j], NULL);
/*  }*/
}

/* ARGSUSED */
XtCallbackProc
perm_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{
  permute(xg);
}

/* ARGSUSED */
XtCallbackProc
perm_anim_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{

}

/* ARGSUSED */
XtCallbackProc
perm_mult_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{

}

/* ARGSUSED */
XtCallbackProc
perm_nmult_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{

}

/* ARGSUSED */
XtCallbackProc
norm_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{

}

/* ARGSUSED */
XtCallbackProc
norm_anim_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{

}

/* ARGSUSED */
XtCallbackProc
brush_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{

}

/* ARGSUSED */
XtCallbackProc
vgroup_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{

}

/* ARGSUSED */
XtCallbackProc
restor_cback(w, xg, callback_data)
  Widget w;
  xgobidata *xg;
  XtPointer callback_data;
{

}
/* ARGSUSED */
static XtCallbackProc
var_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Boolean state;
  XtVaGetValues(w, XtNstate, &state, NULL);
  setToggleBitmap(w, state);
}

#include <X11/Xaw/Viewport.h>
/* ARGSUSED */
XtCallbackProc
open_infer_popup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Widget close;
  Widget form0;
  Dimension width, height;
  register int j;
  Widget box_variables, box_varlabels, box_tforms;
  char str[64];
  Widget vport, vport_child;
  Widget box_stage[4],/*sbox,*/pbox,normbox;
  Boolean doit = false;

  Dimension maxwidth = 0;

  if (tpopup == NULL) {
    tform_cols = (int *) XtMalloc((Cardinal) (xg->ncols-1) * sizeof(int));
    var_cbox = (Widget *) XtMalloc((Cardinal) (xg->ncols-1) * sizeof(Widget));

    if (popupx == -1 && popupy == -1) {
      XtVaGetValues(xg->workspace,
        XtNwidth, &width,
        XtNheight, &height, NULL);
      XtTranslateCoords(xg->workspace,
        (Position) width, (Position) (height/2), &popupx, &popupy);
    }

    tpopup = XtVaCreatePopupShell("Inference Tools",
      topLevelShellWidgetClass, xg->shell,
      XtNx,        popupx,
      XtNy,        popupy,
      XtNinput,    True,
      XtNtitle,    "Inference Tools",
      XtNiconName, "Infer",
      NULL);
    if (mono) set_mono(tpopup);

    /*
     * Create a paned widget so the 'Click here ...'
     * can be all across the bottom.
    */
    tpane = XtVaCreateManagedWidget("Form",
      panedWidgetClass, tpopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      XtNresizable, False,
      NULL);

    form0 = XtVaCreateManagedWidget("Form",
      formWidgetClass, tpane,
      XtNresizable, False,
      NULL);
    if (mono) set_mono(form0);

    box_tforms = XtVaCreateManagedWidget("Close",
      boxWidgetClass, form0,
      XtNorientation, (XtOrientation) XtorientVertical,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);

    /* Stage 0: Tools */
    box_stage[0] = XtVaCreateManagedWidget("Close",
      boxWidgetClass, box_tforms,
/*      XtNorientation, (XtOrientation) XtorientHorizontal,*/
      NULL);

    /* Sort */
/*
      sbox = XtVaCreateManagedWidget("Close",
        boxWidgetClass, box_stage[0],
        XtNorientation, (XtOrientation) XtorientHorizontal,
//      XtNfromHoriz, (Widget) box_stage[0],
        NULL);
*/

    sort_cmd = (Widget) CreateCommand(xg, "Sort",
      True, (Widget) NULL, (Widget) NULL,
      box_stage[0], "Inference");
    XtVaSetValues(sort_cmd,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(sort_cmd);
    XtAddCallback(sort_cmd, XtNcallback,
      (XtCallbackProc) sort_cback, (XtPointer) xg);

    /* Permute */
    pbox = XtVaCreateManagedWidget("Close",
      boxWidgetClass, box_stage[0],
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNfromVert, (Widget) sort_cmd /*sbox*/,
      NULL);

    perm_cmd = (Widget) CreateCommand(xg, "Permute",
      True, (Widget) NULL, (Widget) NULL,
      pbox, "Inference");
    XtVaSetValues(perm_cmd,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(perm_cmd);
    XtAddCallback(perm_cmd, XtNcallback,
      (XtCallbackProc) perm_cback, (XtPointer) xg);

    perm_anim = (Widget) CreateToggle(xg, "Animate",
      True, (Widget) perm_cmd, (Widget) NULL,
      (Widget) NULL, False, ANY_OF_MANY, pbox, "Inference");
    XtVaSetValues(perm_anim,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(perm_anim);
    XtAddCallback(perm_anim, XtNcallback,
      (XtCallbackProc) perm_anim_cback, (XtPointer) xg);

    perm_mult = (Widget) CreateToggle(xg, "Multiples",
      True, (Widget) perm_anim, (Widget) NULL,
      (Widget) NULL, False, ANY_OF_MANY, pbox, "Inference");
    XtVaSetValues(perm_mult,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(perm_mult);
    XtAddCallback(perm_mult, XtNcallback,
      (XtCallbackProc) perm_mult_cback, (XtPointer) xg);

    perm_nmult = XtVaCreateManagedWidget("Inference",
      asciiTextWidgetClass, pbox,
      XtNfromHoriz, (Widget) perm_mult,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) "16",
      NULL);
    if (mono) set_mono(perm_mult);

    /*normal sample*/
    normbox = XtVaCreateManagedWidget("Close",
      boxWidgetClass, box_stage[0],
      XtNorientation, (XtOrientation) XtorientHorizontal, NULL);

    norm_cmd = (Widget) CreateCommand(xg, "Normal Sample",
      True, (Widget) NULL, (Widget) NULL,
      normbox, "Inference");
    XtVaSetValues(norm_cmd,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(norm_cmd);
    XtAddCallback(norm_cmd, XtNcallback,
      (XtCallbackProc) norm_cback, (XtPointer) xg);

    norm_anim = (Widget) CreateToggle(xg, "Animate",
      True, (Widget) norm_cmd, (Widget) NULL,
      (Widget) NULL, False, ANY_OF_MANY, normbox, "Inference");
    XtVaSetValues(norm_anim,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(norm_anim);
    XtAddCallback(norm_anim, XtNcallback,
      (XtCallbackProc) norm_anim_cback, (XtPointer) xg);
  
    /* Stage 1: Tools */
/*
      box_stage[1] = XtVaCreateManagedWidget("Close",
        boxWidgetClass, box_tforms, NULL);
*/
    
    /*Condition on brush variables*/
    brush_tog = (Widget) CreateToggle(xg, "Conditional on brush group variable",
      True, (Widget) NULL, (Widget) NULL, (Widget) NULL, False, ANY_OF_MANY,
      box_tforms, "Inference");
    XtVaSetValues(brush_tog,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(brush_tog);
    XtAddCallback(brush_tog, XtNcallback,
      (XtCallbackProc) brush_cback, (XtPointer) xg);

/*Extend to vgroup members*/
    vgroup_tog = (Widget) CreateToggle(xg, "Extend to vgroup members",
      True, (Widget) NULL, (Widget) NULL, (Widget) NULL, inf_vgroup, ANY_OF_MANY,
      box_tforms, "Inference");
    XtVaSetValues(vgroup_tog,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(vgroup_tog);
    XtAddCallback(vgroup_tog, XtNcallback,
      (XtCallbackProc) vgroup_cback, (XtPointer) xg);

/*Restore*/
    restore_cmd = (Widget) CreateCommand(xg, "Restore",
      True, (Widget) box_stage[0], (Widget) NULL, box_tforms, 
       "Inference");
    XtVaSetValues(restore_cmd,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    XtManageChild(restore_cmd);
    XtAddCallback(restore_cmd, XtNcallback,
      (XtCallbackProc) restor_cback, (XtPointer) xg);




/*    infer_vgroups_cmd = (Widget) CreateToggle(xg, "Use vgroups",
      True, (Widget) NULL, (Widget) NULL, (Widget) NULL,
      infer_vgroup, ANY_OF_MANY, mform, "Inference");
    XtManageChild(infer_vgroups_cmd);
    XtAddCallback(infer_vgroups_cmd, XtNcallback,
      (XtCallbackProc) infer_vgroups_cback, (XtPointer) xg);

    infer_bgroups_cmd = (Widget) CreateToggle(xg, "Use brush groups",
      False, (Widget) NULL, (Widget) NULL, (Widget) NULL,
      infer_bgroup, ANY_OF_MANY, mform, "Inference");
    XtManageChild(infer_bgroups_cmd);
    XtAddCallback(infer_bgroups_cmd, XtNcallback,
      (XtCallbackProc) infer_bgroups_cback, (XtPointer) xg);

*/

    vport = XtVaCreateManagedWidget("ViewPort",
      viewportWidgetClass, form0,
      XtNallowHoriz, False,
      XtNallowVert,  True,
      XtNfromHoriz, box_tforms,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNmappedWhenManaged, False,
      NULL);
    vport_child = XtVaCreateManagedWidget("Box",
      boxWidgetClass, vport,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      NULL);

    box_variables = XtVaCreateManagedWidget("Box",
      formWidgetClass, vport_child,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      NULL);
    for (j=0; j<xg->ncols-1; j++) {
      var_cbox[j] = CreateToggle(xg, xg->collab[j], True, 
        NULL, NULL, NULL, False, ANY_OF_MANY, box_variables, "Transformations");
      if (j>0)
        XtVaSetValues(var_cbox[j], XtNfromVert, var_cbox[j-1], NULL);
      XtAddCallback(var_cbox[j], XtNcallback,
       (XtCallbackProc) var_cback, (XtPointer) xg);
    }
    XtManageChildren(var_cbox, xg->ncols-1);

    box_varlabels = XtVaCreateManagedWidget("Form",
      formWidgetClass, vport_child,
      XtNorientation, (XtOrientation) XtorientVertical,
      XtNfromHoriz, box_variables,
      NULL);

    for (j=0; j<xg->ncols-1; j++) {
      sprintf(str, "%s(%s(%s)))", "Stdize", "Perm", xg->collab[j]);
      width = XTextWidth(appdata.font, str,
        strlen(str)) + 2*ASCII_TEXT_BORDER_WIDTH;
      if (width > maxwidth) maxwidth = width;
    }
    width = XTextWidth(appdata.font, "normsc()", strlen("normsc()")) +
      2*ASCII_TEXT_BORDER_WIDTH;
    maxwidth += width;

    varlabel = (Widget *) XtMalloc((Cardinal) (xg->ncols-1) * sizeof(Widget));
    for (j=0; j<xg->ncols-1; j++) {
      varlabel[j] = XtVaCreateWidget("Label",
        labelWidgetClass, box_varlabels,
        XtNlabel, (String) xg->collab_tform2[j],
        XtNfromVert, (j>0) ? varlabel[j-1] : NULL,
        XtNwidth, maxwidth,
        NULL);
    }
    XtManageChildren(varlabel, xg->ncols-1);

    close = XtVaCreateManagedWidget("Close",
      commandWidgetClass, tpane,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close);
    XtAddCallback(close, XtNcallback,
      (XtCallbackProc) close_cback, (XtPointer) xg);

    set_initial_variable(xg);

    doit = true;
  }

  XtPopup(tpopup, (XtGrabKind) XtGrabNone);
  set_wm_protocols(tpopup);
  XRaiseWindow(display, XtWindow(tpopup));

  if (doit) {
    Dimension hgt;
    XtVaGetValues(box_tforms, XtNheight, &hgt, NULL);
    XtVaSetValues(vport, XtNheight, hgt, NULL);
    XtMapWidget(vport);
  }
}


