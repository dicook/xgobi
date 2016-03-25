#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <X11/keysym.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include "xgvis.h"

#define IJ i*dist.ncols+j 
#define UNIFORM 0
#define NORMAL  1

extern void update_plot(xgobidata *);
extern void update_dissim_plot(void);
extern void mds_once(Boolean);
extern void set_vgroups(void);
extern void set_dist_matrix_from_edges(struct array *, struct array *, int);
extern void set_dist_matrix_from_pos(struct array *, struct array *, double);
extern void reinit_stress(void);
extern void scramble_pos(void);
extern void center_scale_pos(void);
extern double drandval(int);

static void
reset_dims_label(void) {
  char str[32];
  extern Widget mds_dims_label;

  sprintf(str, "Dim (k): %d", mds_dims);
  XtVaSetValues(mds_dims_label,
    XtNstring, (String) str,
    NULL);
}

/* ARGSUSED */
XtCallbackProc
mds_dimsleft_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  if (mds_dims > 1) {
    mds_dims--;
    reset_dims_label();

    set_vgroups();

    update_plot(&xgobi);
    plot_once(&xgobi);

    mds_once(False);
  }
}

/* ARGSUSED */
XtCallbackProc
mds_dimsright_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  if (mds_dims < xgobi.ncols_used) {
    mds_dims++;
    reset_dims_label();

    set_vgroups();

    update_plot(&xgobi);
    plot_once(&xgobi);

    mds_once(False);
  }
}

/* ARGSUSED */
XtCallbackProc
PopUpDistMenu(Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * Pop up the distance matrix menu.
*/
{
  Dimension width, height;
  Position x, y;
  static int initd = 0;

  if (!initd)
  {
    XtVaGetValues(w,
      XtNwidth, &width,
      XtNheight, &height, NULL);
    XtTranslateCoords(w,
      (Position) (width/2), (Position) (height/2), &x, &y);

    XtVaSetValues(dist_popup,
      XtNx, x, XtNy, y, NULL); 

    initd = 1;
  }

  XtPopup(dist_popup, XtGrabNone);
}

/* ARGSUSED */
XtCallbackProc
PopDownDistMenu(Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * Close the distance matrix menu.
*/
{
    XtPopdown(dist_popup);
}

/* ARGSUSED */
XtCallbackProc
choose_dist_cback(Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * Close the distance matrix menu.
*/
{
  int i;
  Arg args[1];
  Boolean selected = False;

  for (i=0; i<NDISTTYPES-1; i++)
  {
    XtSetArg(args[0], XtNstate, &selected);
    XtGetValues(dist_types[i], args, 1);
    if (selected)
    {
      dist_type = i ;
      break;
    }
  }

  if (dist_type < 0 || dist_type > NDISTTYPES-1)
  {
    fprintf(stderr, "Sorry, that's not a valid dist_type.\n");
  }
  else
  {
    /* dist_type has been set, let's call the right routine. */
    switch (dist_type) {
    case LINK:
      set_dist_matrix_from_edges(&dist, &edges, pos.nrows);
      break;
    case EUCLIDIAN:
      set_dist_matrix_from_pos(&dist, &pos, 2.0);
      break;
    case MANHATTAN:
      set_dist_matrix_from_pos(&dist, &pos, 1.0);
      break;
    case DOTPROD:
    case COSDIST:
      break;
    case USER_SUPPLIED:
    case ADJACENCY:
    case MAHALANOBIS:
      fprintf(stderr, "Not implemented yet.\n");
    }
  }
}

/* ARGSUSED */
XtCallbackProc
reset_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  int i, k;

  for(k = 0; k < MIN(pos_orig.ncols, MAXDIMS-1); k++)
    for(i = 0; i < pos.nrows; i++)
      pos.data[i][k] = pos_orig.data[i][k];

  center_scale_pos();

  update_plot(&xgobi);
  plot_once(&xgobi);

  reinit_stress();
}

/* ARGSUSED */
XtCallbackProc
scramble_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  scramble_pos();

  update_plot(&xgobi);
  plot_once(&xgobi);

  reinit_stress();
}

/* ARGSUSED */
XtCallbackProc
center_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{

  center_scale_pos();

  update_plot(&xgobi);
  plot_once(&xgobi);

  reinit_stress();
}

/* ARGSUSED */
XtCallbackProc
run_cback(Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * This callback defines the actions associated with the run button.
*/
{
  xgv_is_running = !xgv_is_running;

  if (xgv_is_running) {

    /*
     * In case points have been moved in xgobi, we'd better
     * copy in the current values in raw_data.
    */
    int i, j;
    for (i=0; i<xgobi.nrows; i++)
      for (j=0; j<xgobi.ncols_used; j++)
        pos.data[i][j] = xgobi.raw_data[i][j] ;

    (void) XtAppAddWorkProc(app_con, RunWorkProcs, NULL);
  }

  setToggleBitmap(w, xgv_is_running);
}

/* ARGSUSED */
static XtTimerCallbackProc
turn_off_scaling(xgobidata *xg, XtIntervalId id)
{
    is_rescale = 0;
}


/* ARGSUSED */
XtCallbackProc
Quit(Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * This callback defines the actions associated with the 'exit' button.
*/
{
    extern void do_exit(int);

    do_exit(0);
}

/* ARGSUSED */
XtCallbackProc
mds_lnorm_cback (Widget w, XtPointer client_data, XtPointer slideposp)
{
  Arg args[1];
  char str[30];
  extern Widget mds_lnorm_label, mds_lnorm_sbar;

  float slidepos = * (float *) slideposp;

  mds_lnorm = floor(((double) (5.0 * slidepos * 1.04) + 1.0)*10.) / 10. ;
  if(mds_lnorm > 6.) mds_lnorm = 6.;
  sprintf(str, "Minkowski n'rm (m): %3.1f ", mds_lnorm);
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_lnorm_label, args, 1);
  XawScrollbarSetThumb(mds_lnorm_sbar, (mds_lnorm-1.)/1.04/5.0 , -1.);

  mds_lnorm_over_distpow = mds_lnorm/mds_distpow;
  mds_distpow_over_lnorm = mds_distpow/mds_lnorm;

  set_vgroups();

  update_plot(&xgobi);
  plot_once(&xgobi);
}
/* ARGSUSED */
XtCallbackProc
mds_power_cback (Widget w, XtPointer client_data, XtPointer slideposp)
/*
 * Adjust the power to which we raise the dissim data matrix.
*/
{
  Arg args[1];
  char str[30];
  extern Widget mds_power_label, mds_power_sbar;

  float slidepos = * (float *) slideposp;

  if (metric_nonmetric == METRIC) {
    mds_power = floor(6. * slidepos * 1.04 * 10.) / 10. ;
    if(mds_power > 6.) mds_power = 6.;
    if(KruskalShepard_classic == KRUSKALSHEPARD) {
      sprintf(str, "Data Power (D^p): %3.1f ",  mds_power);
    } else {
      sprintf(str, "Data Power (D^2p): %3.1f ",  mds_power);
    }
    XtSetArg(args[0], XtNstring, str);
    XtSetValues(mds_power_label, args, 1);
    XawScrollbarSetThumb(mds_power_sbar, mds_power/1.04/6.0, -1.);
  } else { /* nonmetric */
    mds_isotonic_mix = floor(slidepos * 1.04 * 100.)/100. ;
    if(mds_isotonic_mix > 1.0) mds_isotonic_mix = 1.0;
    sprintf(str, "Isotonic(D): %d%% ", (int) (mds_isotonic_mix*100));
    XtSetArg(args[0], XtNstring, str);
    XtSetValues(mds_power_label, args, 1);
    XawScrollbarSetThumb(mds_power_sbar, mds_isotonic_mix/1.04, -1.);
  }

/*
 * The third column of the diagnostics matrix has to be
 * reset as well, and this may be the easiest way to do it
 * because the indices are not simple.
*/
  mds_once(False);
  update_dissim_plot();

}

/* ARGSUSED */
XtCallbackProc
mds_distpow_cback (Widget w, XtPointer client_data, XtPointer slideposp)
/*
 * Adjust the power to which we raise the distance matrix.
*/
{
  Arg args[1];
  char str[30];
  extern Widget mds_distpow_label, mds_distpow_sbar;

  float slidepos = * (float *) slideposp;

  mds_distpow = floor(6. * slidepos * 1.04 * 10.) / 10. ;
  if(mds_distpow > 6.) mds_distpow = 6.;
  sprintf(str, "%s: %3.1f ", "Dist Power (d^q)", mds_distpow);
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_distpow_label, args, 1);
  XawScrollbarSetThumb(mds_distpow_sbar, mds_distpow/1.04/6., -1.);

  mds_lnorm_over_distpow = mds_lnorm/mds_distpow;
  mds_distpow_over_lnorm = mds_distpow/mds_lnorm;

  mds_once(False);
  update_dissim_plot();
}

/* ARGSUSED */
XtCallbackProc
mds_weightpow_cback (Widget w, XtPointer client_data, XtPointer slideposp)
/*
 * Adjust the power to which we raise the dissimilarity matrix to obtain weights.
*/
{
  Arg args[1];
  char str[30];
  extern Widget mds_weightpow_label, mds_weightpow_sbar;

  float slidepos = * (float *) slideposp;

  mds_weightpow = floor((slidepos - 0.5) * 1.08 * 8.0 * 10.)/10.;
  if(mds_weightpow > 4.) mds_weightpow = 4.;
  if(mds_weightpow < -4.) mds_weightpow = -4.;
  sprintf(str, "%s: %4.1f ", "Wght pow (w=D^r)", mds_weightpow);
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_weightpow_label, args, 1);
  XawScrollbarSetThumb(mds_weightpow_sbar, mds_weightpow/1.08/8. + 0.5, -1.);

  mds_once(False);
}

/* ARGSUSED */
XtCallbackProc
mds_within_between_cback (Widget w, XtPointer client_data, XtPointer slideposp)
/*
 * Adjust the degree to which we use within and between grp dissimilarities
*/
{
  Arg args[1];
  char str[30];
  extern Widget mds_within_between_label, mds_within_between_sbar;

  float slidepos = * (float *) slideposp;

  mds_within_between = floor(slidepos * 1.04 * 2.0 * 50.) / 50.;
  if(mds_within_between > 2.0) mds_within_between = 2.0;
  sprintf(str, "Withn=%3.2f Betwn=%3.2f", (2. - mds_within_between), mds_within_between);
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_within_between_label, args, 1);
  XawScrollbarSetThumb(mds_within_between_sbar, mds_within_between/1.04/2., -1.);

  mds_once(False);
}

/* ARGSUSED */
XtCallbackProc
mds_rand_select_cback (Widget w, XtPointer client_data, XtPointer slideposp)
/*
 * Adjust the probability of random selection of a dist/diss
*/
{
  Arg args[1];
  char str[30];
  int i;
  extern Widget mds_rand_select_label, mds_rand_select_sbar;

  float slidepos = * (float *) slideposp;

  mds_rand_select_val = floor(slidepos*1.04 * 100.) / 100.;
  if(mds_rand_select_val > 1.0) mds_rand_select_val = 1.0;
  sprintf(str, "Select'n prob: %d%%", (int) (mds_rand_select_val*100));
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_rand_select_label, args, 1);
  XawScrollbarSetThumb(mds_rand_select_sbar, mds_rand_select_val/1.04, -1.);

  mds_once(False);
}
/* ARGSUSED */
XtCallbackProc
mds_rand_select_new_cback (Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * Call for new random selection vector
*/
{
  mds_rand_select_new = TRUE;

  mds_once(False);
}

/* ARGSUSED */
XtCallbackProc
mds_perturb_cback (Widget w, XtPointer client_data, XtPointer slideposp)
/*
 * Adjust the degreee of perturbation of configuration
*/
{
  Arg args[1];
  char str[30];
  extern Widget mds_perturb_label, mds_perturb_sbar;

  float slidepos = * (float *) slideposp;

  mds_perturb_val = floor(slidepos * 1.04 * 100.) / 100.;
  if(mds_perturb_val > 1.0) mds_perturb_val = 1.0;
  sprintf(str, "Perturb: %d%%", (int) (mds_perturb_val*100));
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_perturb_label, args, 1);
  XawScrollbarSetThumb(mds_perturb_sbar, mds_perturb_val/1.04, -1.);

  mds_once(False);
}

/* ARGSUSED */
XtCallbackProc
mds_perturb_new_cback (Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * Call for new perturbation with normal random numbers:
*/
{
  int i, k;

  for (i = 0; i < pos_orig.nrows; i++)
    for (k = mds_freeze_var; k < mds_dims; k++) {
      pos.data[i][k] = (1.0-mds_perturb_val)*pos.data[i][k] + (mds_perturb_val)*drandval(NORMAL);  /* standard normal */
    }

  center_scale_pos();

  update_plot(&xgobi);
  plot_once(&xgobi);

  reinit_stress();
}

/* ARGSUSED */
XtCallbackProc
mds_stepsize_cback (Widget w, XtPointer client_data, XtPointer slideposp)
/*
 * Adjust the stepsize (range: currently, 0.000:1.000).
*/
{
  Arg args[1];
  char str[30];
  extern Widget mds_stepsize_label;

  float slidepos = * (float *) slideposp;  /* 0:1 */

  mds_stepsize = floor(0.2 * slidepos * slidepos * 10000.) / 10000.;
  sprintf(str, "%s: %3.4f ", "Stepsize", mds_stepsize);
  XtSetArg(args[0], XtNstring, str);
  XtSetValues(mds_stepsize_label, args, 1);
}

/* ARGSUSED */
XtCallbackProc
mds_iterate_cback (Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * Step: one step through the mds loop.
*/
{
  mds_once(True);
  update_plot(&xgobi);
  RunWorkProc((xgobidata *) &xgobi);
  plot_once(&xgobi);
}

/* ARGSUSED */
static XtCallbackProc
fcancel_cback(Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * If the plot window is fully or partially exposed, clear and redraw.
*/
{
  XtDestroyWidget(XtParent(XtParent(w)));
  printf("fcancel_cback: If the plot window is fully or partially exposed, clear and redraw. \n");
}

/* ARGSUSED */
XtEventHandler
save_distance_matrix_go (Widget w, XtPointer cldata, XEvent *event)
{
  XKeyPressedEvent *evnt = (XKeyPressedEvent *) event;
  KeySym key;
  char *fname;
  FILE *fp;

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

    XtVaGetValues(w, XtNstring, &fname, NULL);
    if ( (fp = fopen(fname, "w")) == NULL)
    {
      char message[MSGLENGTH];
      sprintf(message, "Failed to open the file '%s' for writing.\n", fname);
      show_message(message, &xgobi);
    }
    else
    {
      int i, j;
      for (i = 0; i < dist.nrows; i++) {
        for (j = 0; j < dist.ncols; j++) {
          fprintf(fp, "%2.4f ", dist.data[i][j]);
        }
        fprintf(fp, "\n");
      }
      fflush(fp);
    }

    XtDestroyWidget(XtParent(XtParent(w)));
  }
}

/* ARGSUSED */
XtCallbackProc
save_distance_matrix (Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * Write out the distance matrix.
*/
{
/*
 * Create a popup window to get the name; then call
 * the save routine.
*/
  Widget fpopup, fform, flabel, ftext, fcancel;
  Dimension width, height;
  Position x, y;
  Cursor text_cursor = XCreateFontCursor(display, XC_xterm);

  XtVaGetValues(shell,
    XtNwidth, &width,
    XtNheight, &height,
    NULL);
  XtTranslateCoords(w,
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
    /*topLevelShellWidgetClass, w,*/
    transientShellWidgetClass, shell,
    XtNx, (Position) x,
    XtNy, (Position) y,
    XtNinput, (Boolean) True,
    XtNallowShellResize, (Boolean) True,
    XtNtitle, (String) "Solicit File Name",
    NULL);

/*
 * Create the form widget.
*/
  fform = XtVaCreateManagedWidget("FSaveForm",
    formWidgetClass, fpopup,
    NULL);

  flabel = XtVaCreateManagedWidget("FSaveText",
    labelWidgetClass, fform,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNlabel, (String) "Enter file name for distance matrix:",
    NULL);

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
    NULL);

  XtAddEventHandler(ftext, KeyPressMask, FALSE,
     (XtEventHandler) save_distance_matrix_go, (XtPointer) NULL);
/*
 * Add a cancel button
*/
  fcancel = XtVaCreateManagedWidget("Command",
    commandWidgetClass, fform,
    XtNfromVert, ftext,
    XtNlabel, "Cancel",
    NULL);
  XtAddCallback(fcancel, XtNcallback,
    (XtCallbackProc) fcancel_cback, (XtPointer) NULL);

  XtPopup(fpopup, XtGrabExclusive);
  XRaiseWindow(display, XtWindow(fpopup));

  XDefineCursor(display, XtWindow(ftext), text_cursor);
/*
 * Should do something more clever here -- get the size
 * of the window and place the cursor that way.
*/
  XWarpPointer(display, None, XtWindow(ftext), 0,0,0,0, 10,40);
}


/* ARGSUSED */
XtCallbackProc
mds_launch_cback (Widget w, XtPointer client_data, XtPointer callback_data)
/*
 * Launch the xgobi child containing diagnostic data.
*/
{
  extern xgobidata xgobi, xgobi_diag;
  extern Widget mds_launch_ntxt;
  int i, j;
  char **col_name;
  int nc = 7;
  static char *clab[] = {"d_ij", "f(D_ij)", "D_ij", "Res_ij", "Wgt_ij", "i", "j"};
  static char *blab[] = {"b_ij", "f(D_ij)", "D_ij", "Res_ij", "Wgt_ij", "i", "j"};
  char fname[512], config_basename[512];
  FILE *fp, *fpdat, *fprow, *fpvgrp;
  static int iter = 0;
  char message[MSGLENGTH];
  char command[512];
  char xgobi_exec[512];
  char *xgobidir;
  struct stat buf;
  int subset_size=0;

  subset_size = num_active_dist;

  col_name = (char **) XtMalloc(
    (Cardinal) nc * sizeof (char *));
  if(KruskalShepard_classic == KRUSKALSHEPARD) {
    for (j=0; j<nc; j++) col_name[j] = clab[j];
  } else {
    for (j=0; j<nc; j++) col_name[j] = blab[j];
  }

  sprintf(config_basename, "Shepard_Plot_%d", iter);
  
  /* Write out the data */
  sprintf(fname, "%s.dat", config_basename);
  if ( (fpdat = fopen(fname, "w")) == NULL) {
    sprintf(message,
	    "The file '%s' can not be created\n", fname);
    show_message(message, &xgobi);
    return(0);
  } else {
    sprintf(fname, "%s.row", config_basename);
    if ( (fprow = fopen(fname, "w")) == NULL) {
      sprintf(message,
	      "The file '%s' can not be created\n", fname);
      show_message(message, &xgobi);
      fclose(fprow);
      return(0);
    } else {
      sprintf(fname, "%s.vgroups", config_basename);
      if ( (fpvgrp = fopen(fname, "w")) == NULL) {
	sprintf(message,
		"The file '%s' can not be created\n", fname);
	show_message(message, &xgobi);
	fclose(fpvgrp);
	return(0);
      }
    }
  }

  /*
   * This takes care of writing out the data and the row labels
  */
  {double dist_trans, dist_config, dist_data, weight, resid;
  xgobidata *xg = (xgobidata *) &xgobi;
  mds_once(FALSE);
  for (i = 0; i < dist.nrows; i++) {
    for (j = 0; j < dist.ncols; j++) {
      dist_trans  = trans_dist[IJ];             	if (dist_trans  ==  DBL_MAX) continue;
      dist_config = config_dist[IJ];
      dist_data   = dist.data[i][j];
      resid = (dist_trans - dist_config);
      if(mds_weightpow == 0. && mds_within_between == 1.) { weight = 1.0; } else { weight = weights[IJ]; }
      fprintf(fpdat, "%5.5g %5.5g %5.5g %5.5g %5.5g %d %d\n",
	      dist_config, dist_trans, dist_data, resid, weight, i, j);
      fprintf(fprow, "%s|%s\n", xg->rowlab[ i ], xg->rowlab[ j ]);
    }
  }
  }

  /* variable groups file: keep variables "i" and "j" on same scale */
  fprintf(fpvgrp, "1\n2\n3\n4\n5\n6\n6\n");

  fclose(fpdat);
  fclose(fprow);
  fclose(fpvgrp);

  /* Write out the column labels */
  sprintf(fname, "%s.col", config_basename);
  if ( (fp = fopen(fname, "w")) == NULL) {
    sprintf(message,
      "The file '%s' can not be created\n", fname);
    show_message(message, &xgobi);
    return(0);
  } else {
    for (i=0; i<nc; i++) {
      fprintf(fp, "%s\n", col_name[i]);
    }
    fclose(fp);
  }
  
  XtFree((char *) col_name);

  xgobidir = getenv("XGOBID");
  if(xgobidir && strlen(xgobidir) > 0) { 
    sprintf(xgobi_exec, "%s/bin/xgobi", xgobidir);

    /* If no luck there, then just try 'xgobi' without a path name */
    if (stat(xgobi_exec, &buf) != 0)
      sprintf(xgobi_exec, "xgobi");
  } else
    sprintf(xgobi_exec, "xgobi");

  if (mono)
    strcat(xgobi_exec, " -mono");
  sprintf(command,
    "%s -subset %d %s &", xgobi_exec, subset_size, config_basename);
  fprintf(stderr, "%s\n", command);

  system (command);

  iter++;

} /* end mds_launch_cback(... */







