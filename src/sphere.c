/* sphere.c */
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

static Widget spane, spopup = NULL;
static Position popupx = -1, popupy = -1;
static Boolean is_sphere_mapped = False;
static Widget select_npc, npc, totvar, condnum, do_sph;
static Widget scree_plot_wksp;
static WidgetSize scree_wksp;
static Window scree_plot_window;
static Pixmap scree_plot_pixmap;
static float *evals;
static int numpcs = 0;
static  char str[20];

/* ARGSUSED */
static XtCallbackProc
close_sphere_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtDestroyWidget(spopup);
  spopup = NULL;
  is_sphere_mapped = False;
}

/* ARGSUSED */
static XtCallbackProc
scree_expose_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XCopyArea(display, scree_plot_pixmap, scree_plot_window, copy_GC,
    0, 0, scree_wksp.width, scree_wksp.height, 0, 0);
}
   
/* ARGSUSED */
static XtCallbackProc
select_npc_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int j;
  float ftmp1=0.0, ftmp2=0.0;
  char message[MSGLENGTH];
  char *snumpcs;

  XtVaGetValues(npc, XtNstring, (String) &snumpcs, NULL);
  numpcs = atoi(snumpcs);

  if (numpcs<1)
  {
     sprintf(message, "Need to choose at least 1 PC.\n");
     show_message(message, xg);
  XtVaSetValues(do_sph,
                XtNsensitive, False,
                NULL);
  }
  else if (numpcs > xg->nsph_vars)
  {
     sprintf(message, "Need to choose at most %d PCs.\n",xg->nsph_vars);
     show_message(message, xg);

  XtVaSetValues(do_sph,
                XtNsensitive, False,
                NULL);
  } else {
     
  for (j=0; j<numpcs; j++)
    ftmp1 += evals[j];
  for (j=0; j<xg->nsph_vars; j++)
    ftmp2 += evals[j];

  sprintf(str, "Tot. Var.: %5.1f ", ftmp1/ftmp2);
  XtVaSetValues(totvar,
    XtNlabel, str,
    NULL);

  sprintf(str, "Cond. Num.: %.2e ", evals[0]/evals[numpcs-1]);
  XtVaSetValues(condnum,
    XtNlabel, str,
    NULL);

  XtVaSetValues(do_sph,
                XtNsensitive, True,
                NULL);
  
  }
  
}

/* ARGSUSED */
static XtCallbackProc
do_sph_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int j, n;
  char message[MSGLENGTH];

  if ((numpcs > 0) && (numpcs <= xg->nsph_vars))
  {
    if (evals[numpcs-1] == 0.0 || evals[0]/evals[numpcs-1] > 10000.0) {
      sprintf(message, "Need to choose less PCs. Var-cov close to singular.\n");
      show_message(message, xg);
    }
    else {
       
      spherize_data(xg, numpcs, xg->nsph_vars, xg->sph_vars);

/*      for (j=0; j<numpcs; j++)
        recalc_vc(j, xg);
*/

      set_sph_labs(xg, numpcs);

      xg->is_princ_comp = True;
      XtVaSetValues(xg->princ_comp_cmd, XtNstate, True, NULL);
      setToggleBitmap(xg->princ_comp_cmd, True);

      set_sens_pc_axes(True, xg);

      update_lims(xg);
      update_world(xg);

      world_to_plane(xg);
      plane_to_screen(xg);

      /*
      This bit of init_axes() is needed.
      */
      for (n=0; n<numpcs; n++) {
        j = xg->sph_vars[n];
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
  }
  
}

/* ARGSUSED */
XtCallbackProc
open_sphere_popup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Widget close;
  Widget form0;
  Dimension width, height;
  register int j;
  Widget box[2];
  Boolean do_svd = false;
  char npc_lab[5], tickmk[5];
  int xstrt, ystrt, xpos, ypos;
  char message[MSGLENGTH];
  
  if (!is_sphere_mapped) {
     
    if (spopup == NULL) {

      if (popupx == -1 && popupy == -1) {
        XtVaGetValues(xg->workspace,
          XtNwidth, &width,
          XtNheight, &height, NULL);
        XtTranslateCoords(xg->workspace,
          (Position) width, (Position) (height/2), &popupx, &popupy);
      }

      spopup = XtVaCreatePopupShell("Sphere Selected Variables",
        topLevelShellWidgetClass, xg->shell,
        XtNx,        popupx,
        XtNy,        popupy,
        XtNinput,    True,
        XtNtitle,    "Sphere variables",
        XtNiconName, "Sphere",
        NULL);
      if (mono) set_mono(spopup);

      /*
       * Create a paned widget so the 'Click here ...'
       * can be all across the bottom.
      */
      spane = XtVaCreateManagedWidget("Form",
        panedWidgetClass, spopup,
        XtNorientation, (XtOrientation) XtorientVertical,
        XtNresizable, False,
        NULL);

      form0 = XtVaCreateManagedWidget("Form",
        formWidgetClass, spane,
        XtNresizable, False,
        NULL);
      if (mono) set_mono(form0);

     /* Controls panel */
      box[0] = XtVaCreateManagedWidget("Close",
        boxWidgetClass, form0,
        XtNorientation, (XtOrientation) XtorientVertical,
        XtNleft, (XtEdgeType) XtChainLeft,
        XtNright, (XtEdgeType) XtChainLeft,
        XtNtop, (XtEdgeType) XtChainTop,
        XtNbottom, (XtEdgeType) XtChainTop,
        NULL);

      select_npc = (Widget) CreateCommand(xg, "Select Num PCs",
        True, NULL, NULL, box[0], "Sphere");
      XtVaSetValues(select_npc,
        XtNleft, (XtEdgeType) XtChainLeft,
        XtNright, (XtEdgeType) XtChainLeft,
        XtNtop, (XtEdgeType) XtChainTop,
        XtNbottom, (XtEdgeType) XtChainTop,
        NULL);
      XtManageChild(select_npc);
      XtAddCallback(select_npc, XtNcallback,
        (XtCallbackProc) select_npc_cback, (XtPointer) xg);

      sprintf(npc_lab,"%d", xg->nsph_vars);
      npc = XtVaCreateManagedWidget("Inference",
        asciiTextWidgetClass, box[0],
        XtNfromHoriz, (Widget) select_npc,
        XtNleft, (XtEdgeType) XtChainLeft,
        XtNright, (XtEdgeType) XtChainRight,
        XtNtop, (XtEdgeType) XtChainTop,
        XtNbottom, (XtEdgeType) XtChainTop,
        XtNresizable, (Boolean) True,
        XtNeditType, (int) XawtextEdit,
        XtNresize, (XawTextResizeMode) XawtextResizeWidth,
        XtNstring, (String) npc_lab,
        NULL);
      if (mono) set_mono(npc);

      (void) sprintf(str, "Tot. Var: %5.1f ", 100.0);
      width = XTextWidth(appdata.font, str,
        strlen(str) + 2*ASCII_TEXT_BORDER_WIDTH);
      (void) sprintf(str, "Tot. Var.: %5.1f ", 0.0);
      totvar = XtVaCreateManagedWidget("Sphere",
      labelWidgetClass,  box[0],
      XtNlabel, (String) str,
      XtNwidth, width,
      NULL);
      if (mono) set_mono(totvar);

      (void) sprintf(str, "Cond. Num: %.2e  ", 10000.1234);
      width = XTextWidth(appdata.font, str,
        strlen(str) + 2*ASCII_TEXT_BORDER_WIDTH);
      (void) sprintf(str, "Cond. Num.: %.2e  ", 0.0);
      condnum = XtVaCreateManagedWidget("Sphere",
      labelWidgetClass,  box[0],
      XtNlabel, (String) str,
      XtNwidth, width,
      NULL);
      if (mono) set_mono(condnum);

      do_sph = (Widget) CreateCommand(xg, "Do Sphering",
        False, NULL, NULL, box[0], "Sphere");
      XtVaSetValues(do_sph,
        XtNleft, (XtEdgeType) XtChainLeft,
        XtNright, (XtEdgeType) XtChainLeft,
        XtNtop, (XtEdgeType) XtChainTop,
        XtNbottom, (XtEdgeType) XtChainTop,
        NULL);
      XtManageChild(do_sph);
      XtAddCallback(do_sph, XtNcallback,
        (XtCallbackProc) do_sph_cback, (XtPointer) xg);

      /* Scree plot */
      box[1] = XtVaCreateManagedWidget("Close",
        boxWidgetClass, form0,
        XtNorientation, (XtOrientation) XtorientVertical,
        XtNleft, (XtEdgeType) XtChainLeft,
        XtNright, (XtEdgeType) XtChainLeft,
        XtNtop, (XtEdgeType) XtChainTop,
        XtNbottom, (XtEdgeType) XtChainTop,
        XtNfromHoriz, box[0],
        NULL);

      scree_plot_wksp = XtVaCreateManagedWidget("Inference",
        labelWidgetClass, box[1],
        XtNresizable, (Boolean) True,
        XtNleft, (XtEdgeType) XtChainLeft,
        XtNtop, (XtEdgeType) XtChainTop,
        XtNright, (XtEdgeType) XtRubber,
        XtNbottom, (XtEdgeType) XtRubber,
        XtNwidth, 200,
        XtNheight, 100,
        XtNlabel, (String) "",
        NULL);
      if (mono) set_mono(scree_plot_wksp);
 
      XtVaGetValues(scree_plot_wksp,
        XtNwidth, &scree_wksp.width,
        XtNheight, &scree_wksp.height, NULL);
      XtAddEventHandler(scree_plot_wksp,
        ExposureMask,
        FALSE, (XtEventHandler) scree_expose_cback, (XtPointer) NULL);

      close = XtVaCreateManagedWidget("Close",
        commandWidgetClass, spane,
        XtNshowGrip, (Boolean) False,
        XtNskipAdjust, (Boolean) True,
        XtNlabel, (String) "Click here to dismiss",
        NULL);
      if (mono) set_mono(close);
      XtAddCallback(close, XtNcallback,
        (XtCallbackProc) close_sphere_cback, (XtPointer) xg);
      
    }

    XtPopup(spopup, (XtGrabKind) XtGrabNone);
    set_wm_protocols(spopup);
    XRaiseWindow(display, XtWindow(spopup));

/*  if (doit) {
    Dimension hgt;
    XtVaGetValues(box[0], XtNheight, &hgt, NULL);
    XtVaSetValues(vport, XtNheight, hgt, NULL);
    XtMapWidget(vport);
  }*/

    is_sphere_mapped = True;

    scree_plot_window = XtWindow(scree_plot_wksp);
    scree_plot_pixmap = XCreatePixmap(display, scree_plot_window,
      scree_wksp.width, scree_wksp.height, depth);
    XFillRectangle(display, scree_plot_pixmap, clear_GC,
      0, 0, scree_wksp.width, scree_wksp.height);

  }

  /* Now calculate the svd and display results */
  restore_sph_labs(xg);
  get_sph_vars(xg);
  printf("num vars to be sph'd = %d: ",xg->nsph_vars);
  for (j=0; j<xg->nsph_vars; j++)
     printf("%d ",xg->sph_vars[j]);
  printf("\n");
  set_sph_tform_tp(xg);

  compute_vc_matrix(xg); 
   /* If xg->nsph_vars > 1 use svd routine, otherwise just standardize */
  if (xg->nsph_vars > 1) {
    do_svd = update_vc_active_and_do_svd(xg, xg->nsph_vars, xg->sph_vars);
    if (!do_svd)
    {
       sprintf(message,"Variance-covariance is identity already!\n");
       show_message(message,xg);
    } else {
      evals = (float *) XtMalloc((Cardinal) (xg->nsph_vars) * sizeof(float));
      get_evals(xg->nsph_vars, evals);
      for (j=0; j<xg->nsph_vars; j++)
      {
         /*evals[j] = sqrt((double)evals[j]);*/  /*on di's authority*/
         printf("%f ",evals[j]);
      }
      printf("\n");
      XFillRectangle(display, scree_plot_pixmap, clear_GC,
        0, 0, scree_wksp.width, scree_wksp.height);
      XDrawLine(display, scree_plot_pixmap, copy_GC, 10, 90, 190, 90);
      XDrawLine(display, scree_plot_pixmap, copy_GC, 10, 90, 10, 10);
      for (j=0; j<xg->nsph_vars; j++) {
        sprintf(tickmk,"%d", j+1);
        xpos = (int) (180./(float)(xg->nsph_vars-1)*j+10);
        ypos = (int) (90.-evals[j]/evals[0]*80.);
        XDrawString(display, scree_plot_pixmap, copy_GC,
          xpos, 95, tickmk, strlen(tickmk));
        if (j>0) 
          XDrawLine(display, scree_plot_pixmap, copy_GC, xstrt, ystrt,
            xpos, ypos);
        xstrt = xpos;
        ystrt = ypos;
      }
      XCopyArea(display, scree_plot_pixmap, scree_plot_window, copy_GC,
        0, 0, scree_wksp.width, scree_wksp.height, 0, 0);
/*      XFillRectangle(display, scree_plot_pixmap, clear_GC,
                     0, 0, scree_wksp.width, scree_wksp.height);*/
    }
    
  } else {
/*     scale to variance=1*/
  }
  

}

