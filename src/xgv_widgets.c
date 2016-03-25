/***************************************************************************

    This file defines all the widgets .

*************************************************************************** */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include <X11/keysym.h>
#include "xgvis.h"
#include "DrawingA.h"
#include "../bitmaps/leftarrow.xbm"
#include "../bitmaps/rightarrow.xbm"

extern XtCallbackProc PopUpDistMenu();
extern XtCallbackProc PopDownDistMenu();
extern XtCallbackProc choose_dist_cback();
extern XtCallbackProc PopUpThresholdPanel();
extern XtCallbackProc reset_cback();
extern XtCallbackProc scramble_cback();
extern XtCallbackProc center_cback();
extern XtCallbackProc xgvis_help_MDS_background_cback();
extern XtCallbackProc xgvis_help_controls_cback();
extern XtCallbackProc xgvis_help_kruskal_shepard_cback();
extern XtCallbackProc xgvis_help_torgerson_gower_cback();
extern XtCallbackProc xgvis_help_input_file_formats_cback();
extern XtCallbackProc run_cback();
extern XtCallbackProc Quit();

extern XtCallbackProc mds_lnorm_cback();
extern XtCallbackProc mds_power_cback();
extern XtCallbackProc mds_distpow_cback();
extern XtCallbackProc mds_weightpow_cback();
extern XtCallbackProc mds_within_between_cback();
extern XtCallbackProc mds_rand_select_cback();
extern XtCallbackProc mds_rand_select_new_cback();
extern XtCallbackProc mds_perturb_cback();
extern XtCallbackProc mds_perturb_new_cback();
extern XtCallbackProc mds_stepsize_cback();
extern XtCallbackProc mds_iterate_cback();
extern XtCallbackProc mds_dimsleft_cback();
extern XtCallbackProc mds_dimsright_cback();
extern XtCallbackProc save_distance_matrix();
extern XtCallbackProc mds_casewise_cback();
extern XtCallbackProc mds_launch_cback();
extern XtCallbackProc xgv_open_anchor_popup_cback();


extern void build_stress_plotwin(Widget, Widget, Widget);
extern void build_dissim_plotwin(Widget);
extern void distances2innerproducts();
extern void update_dissim_plot();

extern Widget collbl[4];

extern int point_midbutton;

Widget mdsPanel;
Widget mds_dims_label;
Widget mds_stepsize_label;
Widget mds_stepsize_sbar;
Widget mds_power_label;
Widget mds_power_sbar;
Widget mds_distpow_label;
Widget mds_distpow_sbar;
Widget mds_lnorm_label;
Widget mds_lnorm_sbar;
Widget mds_weightpow_label;
Widget mds_weightpow_sbar;
Widget mds_within_between_label;
Widget mds_within_between_sbar;
Widget mds_rand_select_label;
Widget mds_rand_select_label_new;
Widget mds_rand_select_sbar;
Widget mds_perturb_label;
Widget mds_perturb_label_new;
Widget mds_perturb_sbar;
Widget runPanel;

Widget mds_launch_ntxt, mds_launch_nlbl, mds_launch_nlblx;
Widget run_cmd[6];
#define RUN    run_cmd[0]
#define STEP   run_cmd[1]
#define RESET  run_cmd[2]
#define SCRAM  run_cmd[3]
#define CENTER run_cmd[4]
#define HELP   run_cmd[5]

/*
#define SYMM   run_cmd[5]
#define RAW    run_cmd[6]
*/


/* MDS with groups */
#define NGROUPBTNS 5
Widget group_menu_cmd, group_menu_btn[NGROUPBTNS] ;
Widget group_menu_box, group_menu_lab, group_menu, group_menu_popup;
char *group_menu_btn_label[] = {
  "scale all",
  "within groups",
  "between groups",
  "anchors scale",
  "anchors fixed"
};

/* MDS with frozen variables */
#define NFREEZEBTNS 3
Widget freeze_menu_cmd, freeze_menu_btn[NFREEZEBTNS] ;
Widget freeze_menu_box, freeze_menu_lab, freeze_menu, freeze_menu_popup;
static char *freeze_menu_btn_label[] = {
  "No vars frozen",
  "Var 1 frozen",
  "Vars 1,2 frozen"
};

/* help bitmaps */
#define NHELPBTNS 5
Widget help_menu_cmd, help_menu, help_menu_btn[NHELPBTNS];
static char *help_menu_btn_label[] = {
  "MDS Background",
  "MDS Controls of XGvis",
  "Formula for Kruskal-Shepard Distance Scaling",
  "Formula for Torgerson-Gower Dot-Product Scaling (classic MDS)",
  "Formats of XGvis Input Files"
};

Dimension runPanelWidth, mdsPanelWidth;


/* ARGSUSED */
XtCallbackProc
set_mds_group_cback(w, cldata, cdata)
  Widget w;
  XtPointer cldata, cdata;
{
  int k, btn;
  xgobidata *xg = (xgobidata *) &xgobi;

  for (k=0; k<NGROUPBTNS; k++)
    if (group_menu_btn[k] == w) {
      btn = k;
      break;
    }

  XtVaSetValues(group_menu_cmd,
    XtNlabel, group_menu_btn_label[btn],
    NULL);

  switch (btn) {
    case 0:
      {
	mds_group_ind = deflt;
	break;
      }
    case 1:
      {
	mds_group_ind = within;
	break;
      }
    case 2:
      {
	mds_group_ind = between;
	break;
      }
    case 3:
      {
	mds_group_ind = anchorscales;
	xgv_open_anchor_popup_cback(w, xg, cdata);
	break;
      }
    case 4:
      {
	mds_group_ind = anchorfixed;
	xgv_open_anchor_popup_cback(w, xg, cdata);
	break;
      }
    default:
      {
	mds_group_ind = deflt;
      }
  }
}


/* ARGSUSED */
XtCallbackProc
freeze_0_cback(Widget w, XtPointer cd, int *which) {
  mds_freeze_var = 0;
  XtVaSetValues(freeze_menu_cmd, XtNlabel, (String) freeze_menu_btn_label[0], NULL);  
}

/* ARGSUSED */
XtCallbackProc
freeze_1_cback(Widget w, XtPointer cd, int *which) {
  mds_freeze_var = 1;
  XtVaSetValues(freeze_menu_cmd, XtNlabel, (String) freeze_menu_btn_label[1], NULL);  
}

/* ARGSUSED */
XtCallbackProc
freeze_2_cback(Widget w, XtPointer cd, int *which) {
  mds_freeze_var = 2;
  XtVaSetValues(freeze_menu_cmd, XtNlabel, (String) freeze_menu_btn_label[2], NULL);  
}

static Widget metric_cmd[2], metricPanel;
/* ARGSUSED */
XtCallbackProc
setMetricCback(Widget w, XtPointer cd, int *which) {
  Arg args[1];
  char str[30];

  metric_nonmetric = METRIC;
  
  setToggleBitmap(w, True);
  setToggleBitmap(metric_cmd[1], False);
  if(KruskalShepard_classic == KRUSKALSHEPARD) {
    sprintf(str, "Data Power (D^p): %3.1f ",  mds_power);
  } else {
    sprintf(str, "Data Power (D^2p): %3.1f ",  mds_power);
  }

  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_power_label, args, 1);
  XawScrollbarSetThumb(mds_power_sbar, mds_power/6.0, -1.);

  mds_once(False);
  update_dissim_plot();
}

/* ARGSUSED */
XtCallbackProc
setNonmetricCback(Widget w, XtPointer cd, int *which) {
  Arg args[1];
  char str[30];

  metric_nonmetric = NONMETRIC;

  setToggleBitmap(w, True);
  setToggleBitmap(metric_cmd[0], False);

  sprintf(str, "Isotonic(D): %d%% ", (int) (mds_isotonic_mix*100));
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_power_label, args, 1);
  XawScrollbarSetThumb(mds_power_sbar, mds_isotonic_mix/1.04, -1.);

  mds_once(False);
  update_dissim_plot();

}

static Widget KruskalShepard_cmd[2], KruskalShepardPanel;
/* ARGSUSED */
XtCallbackProc
setKruskalShepardCback(Widget w, XtPointer cd, int *which) {
  Arg args[1];
  char str[30];
  extern Widget mds_power_label;

  KruskalShepard_classic = KRUSKALSHEPARD;
  
  setToggleBitmap(w, True);
  setToggleBitmap(KruskalShepard_cmd[1], False);

  XtVaSetValues(mds_distpow_sbar, XtNsensitive, True, NULL);
  XtVaSetValues(mds_lnorm_sbar, XtNsensitive, True, NULL);

  if(metric_nonmetric == METRIC) {
    sprintf(str, "Data Power (D^p): %3.1f ",  mds_power);
  } else {
    sprintf(str, "Isotonic(D): %d%% ", (int) (mds_isotonic_mix*100));
  }
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_power_label, args, 1);

  mds_once(False);
  update_dissim_plot();
}

/* ARGSUSED */
XtCallbackProc
setClassicCback(Widget w, XtPointer cd, int *which) {
  Arg args[1];
  char str[30];
  extern Widget mds_power_label;

  KruskalShepard_classic = CLASSIC;

  setToggleBitmap(w, True);
  setToggleBitmap(KruskalShepard_cmd[0], False);

  XtVaSetValues(mds_distpow_sbar, XtNsensitive, False, NULL);
  XtVaSetValues(mds_lnorm_sbar, XtNsensitive, False, NULL);

  if(metric_nonmetric == METRIC) {
    sprintf(str, "Data Power (D^2p): %3.1f ",  mds_power);
  } else {
    sprintf(str, "Isotonic(D): %d%% ", (int) (mds_isotonic_mix*100));
  }
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_power_label, args, 1);

  mds_once(False);
  update_dissim_plot();
}


/******************************************************************/
void
make_xgvis_widgets()
{
  char str[30];
  static Widget file_menu_cmd, file_menu, file_menu_btn[2];
  int j, k;
  static char *file_menu_names[] = {
    "Save distance matrix ...",
    "Exit"
  };
  Widget mds_launch_cmd;
  Dimension width;

  form0 = XtVaCreateManagedWidget("Form0",
    formWidgetClass, shell,
    /*XtNfont, panel_data.Font,*/
    XtNfont, appdata.font,
    NULL);
  if (mono) set_mono(form0);

/**************************************************************
 * Define Run Panel, to contain the reset and run buttons. 
 */

  runPanel = XtVaCreateManagedWidget("Panel",
    formWidgetClass, form0,
    XtNhorizDistance, 5,
    XtNvertDistance, 5,
    XtNorientation, (XtOrientation) XtorientVertical,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNbottom, (XtEdgeType) XtChainTop,
    NULL);
  if (mono) set_mono(runPanel);

/* File menu */

  sprintf(str, " Vars 1,2 frozen  ");
  runPanelWidth = XTextWidth(panel_data.Font, str, strlen(str)) + 12;

  file_menu_cmd = XtVaCreateManagedWidget("Command",
    menuButtonWidgetClass, runPanel,
    XtNlabel, (String) "File                 ",
    XtNmenuName, (String) "Menu",
    XtNwidth, (Dimension) runPanelWidth,
    NULL);
  if (mono) set_mono(file_menu_cmd);

  file_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, file_menu_cmd,
    NULL);
  if (mono) set_mono(file_menu);

  file_menu_btn[0] = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, file_menu,
    XtNlabel, (String) file_menu_names[0],
    NULL);
  if (mono) set_mono(file_menu_btn[0]);
  file_menu_btn[1] = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, file_menu,
    XtNlabel, (String) file_menu_names[1],
    NULL);
  if (mono) set_mono(file_menu_btn[1]);

  XtAddCallback(file_menu_btn[0], XtNcallback,
    (XtCallbackProc) save_distance_matrix, (XtPointer) NULL);
  XtAddCallback(file_menu_btn[1], XtNcallback,
    (XtCallbackProc) Quit, (XtPointer) NULL);


/* Run */
  RUN = XtVaCreateManagedWidget("Command",
    toggleWidgetClass, runPanel,
    XtNlabel,       "Run MDS",
    XtNstate,        False,
    XtNfromVert,     file_menu_cmd,
    XtNvertDistance, 10,
    NULL);
  if (mono) set_mono(RUN);
  setToggleBitmap(RUN, False);
  XtVaSetValues(RUN, XtNwidth, (Dimension) runPanelWidth, NULL);
  XtAddCallback(RUN, XtNcallback,
    (XtCallbackProc) run_cback, (XtPointer) NULL);

/* Choice of metric or non-metric MDS */
  metricPanel = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, runPanel,
    XtNhorizDistance, 5,
    XtNvertDistance, 5,
    XtNorientation, (XtOrientation) XtorientVertical,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNfromVert,   RUN,
    NULL);
  if (mono) set_mono(metricPanel);
  metric_cmd[0] = XtVaCreateWidget("XGVToggle",
    toggleWidgetClass, metricPanel,
    XtNstate, (Boolean) True,
    XtNlabel, (String) "Metric ",
    NULL);
  if (mono) set_mono(metric_cmd[0]);
  metric_cmd[1] = XtVaCreateWidget("XGVToggle",
    toggleWidgetClass, metricPanel,
    XtNlabel, (String) "NonMetr",
    XtNradioGroup, metric_cmd[0],
    NULL);
  if (mono) set_mono(metric_cmd[1]);
  XtManageChildren(metric_cmd, 2);

  setToggleBitmap(metric_cmd[0], True);
  setToggleBitmap(metric_cmd[1], False);
  XtAddCallback(metric_cmd[0], XtNcallback,
    (XtCallbackProc) setMetricCback, (XtPointer) NULL);
  XtAddCallback(metric_cmd[1], XtNcallback,
    (XtCallbackProc) setNonmetricCback, (XtPointer) NULL);

/* Choice of K-S (distance-based) or T-Y (classical, dotproduct-based) MDS */
  KruskalShepardPanel = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, runPanel,
    XtNhorizDistance, 5,
    XtNvertDistance, 5,
    XtNorientation, (XtOrientation) XtorientVertical,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNfromVert,   metricPanel,
    NULL);
  if (mono) set_mono(KruskalShepardPanel);
  KruskalShepard_cmd[0] = XtVaCreateWidget("XGVToggle",
    toggleWidgetClass, KruskalShepardPanel,
    XtNstate, (Boolean) True,
    XtNlabel, (String) "Krsk/Sh",
    NULL);
  if (mono) set_mono(KruskalShepard_cmd[0]);
  KruskalShepard_cmd[1] = XtVaCreateWidget("XGVToggle",
    toggleWidgetClass, KruskalShepardPanel,
    XtNlabel, (String) "Classic",
    XtNradioGroup, KruskalShepard_cmd[0],
    NULL);
  if (mono) set_mono(KruskalShepard_cmd[1]);
  XtManageChildren(KruskalShepard_cmd, 2);

  setToggleBitmap(KruskalShepard_cmd[0], True);
  setToggleBitmap(KruskalShepard_cmd[1], False);
  XtAddCallback(KruskalShepard_cmd[0], XtNcallback,
    (XtCallbackProc) setKruskalShepardCback, (XtPointer) NULL);
  XtAddCallback(KruskalShepard_cmd[1], XtNcallback,
    (XtCallbackProc) setClassicCback, (XtPointer) NULL);

/* Step */
  STEP = XtVaCreateManagedWidget("Command",
    commandWidgetClass, runPanel,
    XtNlabel, (String) "Step                        ",
    XtNfromVert,     KruskalShepardPanel,
    XtNvertDistance, 10,
    XtNwidth, (Dimension) runPanelWidth,
    NULL);
  if (mono) set_mono(STEP);
  XtAddCallback(STEP, XtNcallback,
    (XtCallbackProc) mds_iterate_cback, (XtPointer) NULL );

/* Reset */
  RESET = XtVaCreateManagedWidget("Command",
    commandWidgetClass, runPanel,
    XtNlabel,     "Re-init                           ",
    XtNsensitive,  pos_orig.nrows != 0,
    XtNfromVert,   STEP,
    XtNvertDistance, 8,
    XtNwidth, (Dimension) runPanelWidth,
    NULL);
  if (mono) set_mono(RESET);
  XtAddCallback(RESET, XtNcallback,
    (XtCallbackProc) reset_cback, (XtPointer) NULL);
  /* If there was no position matrix passed in, the Reset button should be insensitive. */
  if (pos_orig.nrows == 0)
    XtVaSetValues(RESET, XtNsensitive, False, NULL);

/* center and scale */
  CENTER = XtVaCreateManagedWidget("Command",
    commandWidgetClass, runPanel,
    XtNlabel, (String) "Center/Scale                        ",
    XtNfromVert,     RESET,
    XtNvertDistance, 2,
    XtNwidth, (Dimension) runPanelWidth,
    NULL);
  if (mono) set_mono(CENTER);
  XtAddCallback(CENTER, XtNcallback,
    (XtCallbackProc) center_cback, (XtPointer) NULL);


/* freeze variables */
   freeze_menu_cmd = XtVaCreateManagedWidget("MenuButton",
      menuButtonWidgetClass, runPanel,
      XtNlabel, (String) freeze_menu_btn_label[0],
      XtNmenuName, (String) "Menu",
      XtNwidth, (Dimension) runPanelWidth,
      XtNfromVert, CENTER,
      XtNvertDistance, 8,
      XtNresize, False,
      NULL);
   if (mono) set_mono(freeze_menu_cmd);

   freeze_menu = XtVaCreatePopupShell("Menu",
      simpleMenuWidgetClass, freeze_menu_cmd,
      NULL);
   if (mono) set_mono(freeze_menu);

   for(k=0; k<NFREEZEBTNS; k++) {
      freeze_menu_btn[k] = XtVaCreateWidget("Command",
        smeBSBObjectClass, freeze_menu,
        XtNlabel, freeze_menu_btn_label[k],
        NULL);
      if (mono) set_mono(freeze_menu_btn[k]);
   }
   XtManageChildren(freeze_menu_btn, (Cardinal) NFREEZEBTNS);

   XtAddCallback(freeze_menu_btn[0],  XtNcallback,
      (XtCallbackProc) freeze_0_cback, (XtPointer) NULL);
   XtAddCallback(freeze_menu_btn[1],  XtNcallback,
      (XtCallbackProc) freeze_1_cback, (XtPointer) NULL);
   XtAddCallback(freeze_menu_btn[2],  XtNcallback,
      (XtCallbackProc) freeze_2_cback, (XtPointer) NULL);
    

    /* Shepard diagram: launch xgobi to contain diagnostic data ----------------------------- */
    mds_launch_cmd = XtVaCreateManagedWidget("Command",
      commandWidgetClass, runPanel,
      XtNlabel,        "Shepard Plot                     ",
      XtNfromVert,     freeze_menu_cmd,
      XtNvertDistance, 8,
      XtNwidth, (Dimension) runPanelWidth,
      NULL);
    XtAddCallback(mds_launch_cmd, XtNcallback,
      (XtCallbackProc) mds_launch_cback,
      (XtPointer) NULL);
    if (mono) set_mono(mds_launch_cmd);

    sprintf(str, "%d dists", ndistances);
    mds_launch_nlbl = XtVaCreateManagedWidget("Label",
      labelWidgetClass, runPanel,
      XtNlabel,     str,
      XtNfromVert, mds_launch_cmd,
      XtNvertDistance, 0,
      XtNwidth, (Dimension) runPanelWidth,
      NULL);
    if (mono) set_mono(mds_launch_nlbl);

/* Help */
  help_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, runPanel,
    XtNlabel, (String) "Help...                 ",
    XtNmenuName, (String) "Menu",
    XtNfromVert, mds_launch_nlbl,
    XtNvertDistance, 8,
    XtNwidth, (Dimension) runPanelWidth,
    NULL);
  if (mono) set_mono(help_menu_cmd);

  help_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, help_menu_cmd,
    NULL);
  if (mono) set_mono(help_menu);
  for (j=0; j<NHELPBTNS; j++)
  {
    help_menu_btn[j] = XtVaCreateWidget("Command",
      smeBSBObjectClass, help_menu,
      XtNlabel, (String) help_menu_btn_label[j],
      NULL);
    if (mono) set_mono(help_menu_btn[j]);
  }
  XtManageChildren(help_menu_btn, NHELPBTNS);

  XtAddCallback(help_menu_btn[0], XtNcallback,
    (XtCallbackProc) xgvis_help_MDS_background_cback, (XtPointer) NULL);
  XtAddCallback(help_menu_btn[1], XtNcallback,
    (XtCallbackProc) xgvis_help_controls_cback, (XtPointer) NULL);
  XtAddCallback(help_menu_btn[2], XtNcallback,
    (XtCallbackProc) xgvis_help_kruskal_shepard_cback, (XtPointer) NULL);
  XtAddCallback(help_menu_btn[3], XtNcallback,
    (XtCallbackProc) xgvis_help_torgerson_gower_cback, (XtPointer) NULL);
  XtAddCallback(help_menu_btn[4], XtNcallback,
    (XtCallbackProc) xgvis_help_input_file_formats_cback, (XtPointer) NULL);

/*******************************************************************
 * Define MDS Panel, to contain the controls for MDS.
*/

  mdsPanel = XtVaCreateManagedWidget("Panel",
    formWidgetClass, form0,
    XtNfromHoriz, (Widget) runPanel,
    XtNhorizDistance, 5,
    XtNvertDistance, 5,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNbottom, (XtEdgeType) XtChainTop,
    NULL);
  if (mono) set_mono(mdsPanel);

  /* Number of dimensions -------------------------------- */

  dims_left = XtVaCreateManagedWidget("Icon",
      labelWidgetClass, mdsPanel,
      XtNhorizDistance, 5,
      XtNvertDistance, 5,
      XtNinternalHeight, (Dimension) 0,
      XtNinternalWidth, (Dimension) 0,
      XtNborderColor, (Pixel) appdata.fg,
      XtNbitmap, (Pixmap) leftarr,
      NULL);
    if (mono) set_mono(dims_left);
    XtAddEventHandler(dims_left, ButtonPressMask, FALSE,
      (XtEventHandler) mds_dimsleft_cback, (XtPointer) NULL);

    sprintf(str, "Dim (k): 999");
    mdsPanelWidth = XTextWidth(panel_data.Font, str, strlen(str));

    sprintf(str, "Dim (k): %d", mds_dims);
    mds_dims_label = XtVaCreateManagedWidget("Label",
        asciiTextWidgetClass, mdsPanel,
        XtNhorizDistance, 0,
        XtNvertDistance, 5,
        XtNstring, (String) str,
        XtNdisplayCaret, (Boolean) False,
        XtNwidth, (Dimension) mdsPanelWidth,
        /*XtNheight, (Dimension) 16,*/
        XtNfromHoriz, dims_left,
        NULL);
    if (mono) set_mono(mds_dims_label);

    dims_right = XtVaCreateManagedWidget("Icon",
      labelWidgetClass, mdsPanel,
      XtNfromHoriz, mds_dims_label,
      XtNhorizDistance, 0,
      XtNvertDistance, 5,
      XtNinternalHeight, (Dimension) 0,
      XtNinternalWidth, (Dimension) 0,
      XtNborderColor, (Pixel) appdata.fg,
      XtNbitmap, (Pixmap) rightarr,
      NULL);
    if (mono) set_mono(dims_right);
    XtAddEventHandler(dims_right, ButtonPressMask, FALSE,
      (XtEventHandler) mds_dimsright_cback, (XtPointer) NULL);

    /* first define width of label fields: this is the widest */
    sprintf(str, "Select'n prob: 100X    new");
    mdsPanelWidth = XTextWidth(panel_data.Font, str, strlen(str));

    /* Stepsize label and scrollbar for mds method ------------------------ */
    sprintf(str, "Stepsize: %3.4f", mds_stepsize);

    mds_stepsize_label = XtVaCreateManagedWidget("Label",
        asciiTextWidgetClass, mdsPanel,
        XtNstring, (String) str,
        XtNdisplayCaret, (Boolean) False,
        XtNfromVert, (Widget) mds_dims_label,
        XtNwidth, (Dimension) mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_stepsize_label);

    mds_stepsize_sbar = XtVaCreateManagedWidget("Scrollbar",
        scrollbarWidgetClass, mdsPanel,
        XtNorientation, (XtOrientation) XtorientHorizontal,
        XtNfromVert, (Widget) mds_stepsize_label,
        XtNvertDistance, 0,
        XtNwidth, (Dimension) mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_stepsize_sbar);
    XawScrollbarSetThumb(mds_stepsize_sbar,
			 (float) pow((double)(mds_stepsize / 0.2), .5), 
			 -1.); /* inverse formula for slider -> step */
    XtAddCallback(mds_stepsize_sbar, XtNjumpProc,
        (XtCallbackProc) mds_stepsize_cback, (XtPointer) NULL);

    /* Exponent of dissimilarity matrix ---------------------------------- */
    sprintf(str, "Data Power (D^p): %3.1f", mds_power);

    mds_power_label = XtVaCreateManagedWidget("Label",
        asciiTextWidgetClass, mdsPanel,
        XtNstring, (String) str,
        XtNdisplayCaret, (Boolean) False,
        XtNfromVert, (Widget) mds_stepsize_sbar,
        XtNwidth, mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_power_label);

    mds_power_sbar = XtVaCreateManagedWidget("Scrollbar",
        scrollbarWidgetClass, mdsPanel,
        XtNorientation, (XtOrientation) XtorientHorizontal,
        XtNfromVert, (Widget) mds_power_label,
        XtNvertDistance, 0,
        XtNwidth, (Dimension) mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_power_sbar);
    XawScrollbarSetThumb(mds_power_sbar, mds_power/6.0, -1.);
    XtAddCallback(mds_power_sbar, XtNjumpProc,
        (XtCallbackProc) mds_power_cback, (XtPointer) NULL);

    /* Exponent of distance matrix ----------------------------------- */
    sprintf(str, "Dist Power (d^q): %3.1f", mds_distpow);

    mds_distpow_label = XtVaCreateManagedWidget("Label",
        asciiTextWidgetClass, mdsPanel,
        XtNstring, (String) str,
        XtNdisplayCaret, (Boolean) False,
        XtNfromVert, (Widget) mds_power_sbar,
        XtNwidth, mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_distpow_label);

    mds_distpow_sbar = XtVaCreateManagedWidget("Scrollbar",
        scrollbarWidgetClass, mdsPanel,
        XtNorientation, (XtOrientation) XtorientHorizontal,
        XtNfromVert, (Widget) mds_distpow_label,
        XtNvertDistance, 0,
        XtNwidth, (Dimension) mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_distpow_sbar);
    XawScrollbarSetThumb(mds_distpow_sbar, mds_distpow/6.0, -1.);
    XtAddCallback(mds_distpow_sbar, XtNjumpProc,
        (XtCallbackProc) mds_distpow_cback, (XtPointer) NULL);

    /* Minkowski norm label and scrollbar for mds method ------------------------ */
    sprintf(str, "Minkowski n'rm (m): %3.1f", mds_lnorm);

    mds_lnorm_label = XtVaCreateManagedWidget("Label",
        asciiTextWidgetClass, mdsPanel,
        XtNstring, (String) str,
        XtNdisplayCaret, (Boolean) False,
        XtNfromVert, (Widget) mds_distpow_sbar,
        XtNwidth, mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_lnorm_label);

    mds_lnorm_sbar = XtVaCreateManagedWidget("Scrollbar",
        scrollbarWidgetClass, mdsPanel,
        XtNorientation, (XtOrientation) XtorientHorizontal,
        XtNfromVert, (Widget) mds_lnorm_label,
        XtNvertDistance, 0,
        XtNwidth, (Dimension) mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_lnorm_sbar);

    XawScrollbarSetThumb(mds_lnorm_sbar, 
			 (mds_lnorm-1.0)/5.0, 
			 -1.); /* Range: 1:6 */
    XtAddCallback(mds_lnorm_sbar, XtNjumpProc,
        (XtCallbackProc) mds_lnorm_cback, (XtPointer) NULL);

    /* Exponent of weights wij=(Dij^p)^r for mds method -------------------- */
    sprintf(str, "Wght pow (w=D^r): %3.1f", mds_weightpow);

    mds_weightpow_label = XtVaCreateManagedWidget("Label",
        asciiTextWidgetClass, mdsPanel,
        XtNstring, (String) str,
        XtNdisplayCaret, (Boolean) False,
        XtNfromVert, (Widget) mds_lnorm_sbar,
        XtNwidth, mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_weightpow_label);

    mds_weightpow_sbar = XtVaCreateManagedWidget("Scrollbar",
      scrollbarWidgetClass, mdsPanel,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNfromVert, (Widget) mds_weightpow_label,
      XtNvertDistance, 0,
      XtNwidth, (Dimension) mdsPanelWidth,
      NULL);
    if (mono) set_mono(mds_weightpow_sbar);
    /* range should be -4 to +4 */
    XawScrollbarSetThumb(mds_weightpow_sbar, mds_weightpow/8.0+0.5, -1.);
    XtAddCallback(mds_weightpow_sbar, XtNjumpProc,
      (XtCallbackProc) mds_weightpow_cback, (XtPointer) NULL);

    /* Interpolation within-between groups -------------------------- */
    sprintf(str, "Withn:%3.2f  Betwn:%3.2f", (2. - mds_within_between), mds_within_between);

    mds_within_between_label = XtVaCreateManagedWidget("Label",
        asciiTextWidgetClass, mdsPanel,
        XtNstring, (String) str,
        XtNdisplayCaret, (Boolean) False,
        XtNfromVert, (Widget) mds_weightpow_sbar,
        XtNwidth, mdsPanelWidth,
        NULL);
    if (mono) set_mono(mds_within_between_label);

    mds_within_between_sbar = XtVaCreateManagedWidget("Scrollbar",
      scrollbarWidgetClass, mdsPanel,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNfromVert, (Widget) mds_within_between_label,
      XtNvertDistance, 0,
      XtNwidth, (Dimension) mdsPanelWidth,
      NULL);
    if (mono) set_mono(mds_within_between_sbar);
    /* range should be 0 to 2, default 1 */
    XawScrollbarSetThumb(mds_within_between_sbar, mds_within_between/2.0, -1.);
    XtAddCallback(mds_within_between_sbar, XtNjumpProc,
      (XtCallbackProc) mds_within_between_cback, (XtPointer) NULL);

    /* Use of groups: all dists, within grps, between grps -------- */

    group_menu_lab = XtVaCreateManagedWidget("Label",
      asciiTextWidgetClass, mdsPanel,
      XtNdisplayCaret, (Boolean) False,
      XtNfromVert, (Widget) mds_within_between_sbar,
      XtNwidth, (Dimension) mdsPanelWidth-72,
      XtNstring, (String) "MDS with",
      NULL);
    if (mono) set_mono(group_menu_lab);

    group_menu_popup = XtVaCreateManagedWidget("Command",
      commandWidgetClass, mdsPanel,
      XtNfromHoriz, group_menu_lab,
      XtNhorizDistance, 0,
      XtNlabel, (String) "Groups",
      XtNfromVert, (Widget)  mds_within_between_sbar,
      XtNwidth, (Dimension) 70,
      NULL);
   if (mono) set_mono(group_menu_popup);
   XtAddCallback(group_menu_popup, XtNcallback,
     (XtCallbackProc) xgv_open_anchor_popup_cback, (XtPointer) &xgobi);

   mds_group_ind = deflt;
   group_menu_cmd = XtVaCreateManagedWidget("MenuButton",
      menuButtonWidgetClass, mdsPanel,
      XtNlabel, (String) group_menu_btn_label[0],
      XtNmenuName, (String) "Menu",
      XtNwidth, (Dimension) mdsPanelWidth,
      XtNfromVert, group_menu_lab,
      XtNvertDistance, 0,
      XtNresize, False,
      NULL);
    if (mono) set_mono(group_menu_cmd);

    group_menu = XtVaCreatePopupShell("Menu",
      simpleMenuWidgetClass, group_menu_cmd,
      NULL);
    if (mono) set_mono(group_menu);

    for(k=0; k<NGROUPBTNS; k++) {
      group_menu_btn[k] = XtVaCreateWidget("Command",
        smeBSBObjectClass, group_menu,
        XtNlabel, group_menu_btn_label[k],
        NULL);
      if (mono) set_mono(group_menu_btn[k]);
    }
    XtManageChildren(group_menu_btn, (Cardinal) NGROUPBTNS);

    for (k=0; k<NGROUPBTNS; k++)
      XtAddCallback(group_menu_btn[k],  XtNcallback,
        (XtCallbackProc) set_mds_group_cback, (XtPointer) NULL);

    /* Random selection of distances -------------------------- */
    sprintf(str, "new ");
    width = XTextWidth(panel_data.Font, str, strlen(str))+2;
    sprintf(str, "Select'n prob: %d%%", (int) (mds_rand_select_val*100));
    mds_rand_select_label = XtVaCreateManagedWidget("Label",
       asciiTextWidgetClass, mdsPanel,
       XtNstring, (String) str,
       XtNdisplayCaret, (Boolean) False,
       XtNfromVert, (Widget) group_menu_cmd,
       XtNwidth, mdsPanelWidth-width-2,
       NULL);
       if (mono) set_mono(mds_rand_select_label);

    mds_rand_select_label_new = XtVaCreateManagedWidget("Command",
        commandWidgetClass, mdsPanel,
        XtNfromHoriz, mds_rand_select_label,
        XtNhorizDistance, 0,
        XtNlabel, (String) "new",
	XtNfromVert, (Widget)  group_menu_cmd,
        XtNwidth, (Dimension) width,
        NULL);
    if (mono) set_mono(mds_rand_select_label_new);
    XtAddCallback(mds_rand_select_label_new, XtNcallback,
      (XtCallbackProc) mds_rand_select_new_cback, (XtPointer) NULL );

    mds_rand_select_sbar = XtVaCreateManagedWidget("Scrollbar",
      scrollbarWidgetClass, mdsPanel,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNfromVert, (Widget) mds_rand_select_label,
      XtNvertDistance, 0,
      XtNwidth, (Dimension) mdsPanelWidth,
      NULL);
    if (mono) set_mono(mds_rand_select_sbar);
    /* range should be 0 to 1, default 1 */
    XawScrollbarSetThumb(mds_rand_select_sbar, mds_rand_select_val/1.04, -1.);
    XtAddCallback(mds_rand_select_sbar, XtNjumpProc,
      (XtCallbackProc) mds_rand_select_cback, (XtPointer) NULL);

    /* Random perturbation of configuration -------------------------- */
    sprintf(str, "new ");
    width = XTextWidth(panel_data.Font, str, strlen(str))+2;
    sprintf(str, "Perturb: %d%%", (int) (mds_perturb_val*100));
    mds_perturb_label = XtVaCreateManagedWidget("Label",
       asciiTextWidgetClass, mdsPanel,
       XtNstring, (String) str,
       XtNdisplayCaret, (Boolean) False,
       XtNfromVert, (Widget) mds_rand_select_sbar,
       XtNwidth, mdsPanelWidth-width-2,
       NULL);
       if (mono) set_mono(mds_perturb_label);

    sprintf(str, "new ");
    mds_perturb_label_new = XtVaCreateManagedWidget("Command",
        commandWidgetClass, mdsPanel,
        XtNfromHoriz, mds_perturb_label,
        XtNhorizDistance, 0,
        XtNlabel, (String) "new",
        XtNfromVert, (Widget) mds_rand_select_sbar,
        XtNwidth, (Dimension) width,
        NULL);
    if (mono) set_mono(mds_perturb_label_new);
    XtAddCallback(mds_perturb_label_new, XtNcallback,
      (XtCallbackProc) mds_perturb_new_cback, (XtPointer) NULL );

    mds_perturb_sbar = XtVaCreateManagedWidget("Scrollbar",
      scrollbarWidgetClass, mdsPanel,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNfromVert, (Widget) mds_perturb_label,
      XtNvertDistance, 0,
      XtNwidth, (Dimension) mdsPanelWidth,
      NULL);
    if (mono) set_mono(mds_perturb_sbar);
    /* range should be 0 to 1, default 1 */
    XawScrollbarSetThumb(mds_perturb_sbar, mds_perturb_val/1.04, -1.);
    XtAddCallback(mds_perturb_sbar, XtNjumpProc,
      (XtCallbackProc) mds_perturb_cback, (XtPointer) NULL);

/* AB: add "labeled anchors" to mds_group_ind menu below */
/* Turn on and off the use of labels */
/*
    mds_casewise_label = XtVaCreateManagedWidget("Label",
      labelWidgetClass, mdsPanel,
      XtNlabel, (String) "MDS casewise:",
      XtNfromVert, (Widget) mds_group_cmd,
      NULL);
    if (mono) set_mono(mds_casewise_label);
    mds_casewise_cmd = XtVaCreateManagedWidget("Command",
      toggleWidgetClass, mdsPanel,
      XtNfromVert, mds_casewise_label,
      XtNvertDistance, 0,
      XtNstate, (Boolean) mds_casewise,
      XtNlabel, (String) "use current glyph&color",
      NULL);
    if (mono) set_mono(mds_casewise_cmd);
    XtAddCallback(mds_casewise_cmd, XtNcallback,
      (XtCallbackProc) mds_casewise_cback,
      (XtPointer) NULL );
*/

/* Button to control menu of distance matrix types */
/*
    dist_cmd = XtVaCreateManagedWidget("Command",
      commandWidgetClass, mdsPanel,
      XtNlabel, (String) "distance metric ...",
      XtNfromVert, (Widget) mds_group_cmd,
      XtNhorizDistance, 5,
      XtNvertDistance, 10,
      NULL);
    if (mono) set_mono(dist_cmd);
    XtAddCallback(dist_cmd, XtNcallback,
     (XtCallbackProc) PopUpDistMenu, (XtPointer) NULL);

    dist_popup = XtVaCreatePopupShell("DistMenu",
      transientShellWidgetClass, dist_cmd,
      XtNinput, True,
      XtNtitle, "Dist metrics",
      NULL);
    if (mono) set_mono(dist_popup);

    dist_mgr = XtVaCreateManagedWidget("Panel",
      formWidgetClass, dist_popup,
      NULL);
    if (mono) set_mono(dist_mgr);

    dist_types[USER_SUPPLIED] = XtVaCreateWidget("Command",
        toggleWidgetClass, dist_mgr,
        XtNstate, (Boolean) True,
        XtNlabel, (String) "D is user-supplied",
        NULL);
    if (mono) set_mono(dist_types[USER_SUPPLIED]);

    dist_types[LINK] = XtVaCreateWidget("Command",
        toggleWidgetClass, dist_mgr,
        XtNstate, (Boolean) False,
        XtNlabel, (String) "Link distances",
        XtNradioGroup, (Widget) dist_types[USER_SUPPLIED],
        XtNfromVert, (Widget) dist_types[USER_SUPPLIED],
        NULL);
    if (mono) set_mono(dist_types[LINK]);

    dist_types[ADJACENCY] = XtVaCreateWidget("Command",
        toggleWidgetClass, dist_mgr,
        XtNradioGroup, (Widget) dist_types[USER_SUPPLIED],
        XtNstate, (Boolean) False,
        XtNlabel, (String) "Adjacency matrix",
        XtNfromVert, (Widget) dist_types[LINK],
        NULL);
    if (mono) set_mono(dist_types[ADJACENCY]);

    dist_types[EUCLIDIAN] = XtVaCreateWidget("Command",
        toggleWidgetClass, dist_mgr,
        XtNradioGroup, (Widget) dist_types[USER_SUPPLIED],
        XtNstate, (Boolean) False,
        XtNlabel, (String) "Euclidian",
        XtNfromVert, (Widget) dist_types[ADJACENCY],
        NULL);
    if (mono) set_mono(dist_types[EUCLIDIAN]);

    dist_types[MANHATTAN] = XtVaCreateWidget("Command",
        toggleWidgetClass, dist_mgr,
        XtNradioGroup, (Widget) dist_types[USER_SUPPLIED],
        XtNstate, (Boolean) False,
        XtNlabel, (String) "Manhattan",
        XtNfromVert, (Widget) dist_types[EUCLIDIAN],
        NULL);
    if (mono) set_mono(dist_types[MANHATTAN]);

    dist_types[MAHALANOBIS] = XtVaCreateWidget("Command",
        toggleWidgetClass, dist_mgr,
        XtNradioGroup, (Widget) dist_types[USER_SUPPLIED],
        XtNstate, (Boolean) False,
        XtNlabel, (String) "Mahalanobis",
        XtNfromVert, (Widget) dist_types[MANHATTAN],
        NULL);
    if (mono) set_mono( dist_types[MAHALANOBIS]);

    dist_types[DOTPROD] = XtVaCreateWidget("Command",
        toggleWidgetClass, dist_mgr,
        XtNradioGroup, (Widget) dist_types[USER_SUPPLIED],
        XtNstate, (Boolean) False,
        XtNlabel, (String) "Dot Product",
        XtNfromVert, (Widget) dist_types[MAHALANOBIS],
        NULL);
    if (mono) set_mono(dist_types[DOTPROD]);

    dist_types[COSDIST] = XtVaCreateWidget("Command",
        toggleWidgetClass, dist_mgr,
        XtNradioGroup, (Widget) dist_types[USER_SUPPLIED],
        XtNstate, (Boolean) False,
        XtNlabel, (String) "Cosine",
        XtNfromVert, (Widget) dist_types[DOTPROD],
        NULL);
    if (mono) set_mono( dist_types[COSDIST]);

    XtManageChildren(dist_types, NDISTTYPES-1);

    apply_dist_cmd = XtVaCreateManagedWidget("Command",
        commandWidgetClass, dist_mgr,
        XtNlabel, (String) "Apply",
        XtNfromVert, (Widget) dist_types[COSDIST],
        XtNvertDistance, 15,
        NULL);
    if (mono) set_mono(apply_dist_cmd);
    XtAddCallback(apply_dist_cmd, XtNcallback,
        (XtCallbackProc) choose_dist_cback, (XtPointer) NULL );

    dist_types[CLOSE] = XtVaCreateManagedWidget("Command",
        commandWidgetClass, dist_mgr,
        XtNlabel, (String) "Close",
        XtNfromVert, (Widget) dist_types[COSDIST],
        XtNvertDistance, 15,
        XtNfromHoriz, (Widget) apply_dist_cmd,
        NULL);
    if (mono) set_mono(dist_types[CLOSE]);
    XtAddCallback(dist_types[CLOSE], XtNcallback,
        (XtCallbackProc) PopDownDistMenu, (XtPointer) NULL );
*/

    /* parent, href, vref */
    build_stress_plotwin(form0, NULL, mdsPanel);  
    /*
    build_stress_plotwin(form0, NULL, runPanel);  
    */
    build_dissim_plotwin(form0);  /* parent */
} /* end of make_xgvis_widgets() */

void
update_shepard_labels(int maxn) {
  char strtmp[64];

  /*AB  sprintf(strtmp, "max: %d", maxn); */
  sprintf(strtmp, "%d dists", maxn);
  XtVaSetValues(mds_launch_nlbl, XtNlabel, strtmp, NULL);
}












