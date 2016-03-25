/* callbacks.c */
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
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


/* CORBA */
Boolean selectVariable(xgobidata *xg, int j, int button, int state);

/* ARGSUSED */
XtEventHandler
expose_cback(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
/*
 * If the plot window is fully or partially exposed, clear and redraw.
*/
{
  if (evnt->xexpose.count == 0)  /* Compress expose events */
    plot_once(xg);
}

/* ARGSUSED */
XtEventHandler
resize_cback(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
/*
 * If the window is resized, recalculate the size of the plot window, and
 * then, if points are plotted, clear and redraw.
*/
{
  XtVaGetValues(xg->workspace,
    XtNwidth, &xg->plotsize.width,
    XtNheight, &xg->plotsize.height, NULL);

  XFreePixmap(display, xg->pixmap0);
  xg->pixmap0 = XCreatePixmap(display, xg->plot_window,
    xg->plotsize.width,
    xg->plotsize.height,
    depth);
  XFillRectangle(display, xg->pixmap0, clear_GC,
    0, 0,
    xg->plotsize.width,
    xg->plotsize.height);

  xg->max.x = (int) xg->plotsize.width;
  xg->max.y = (int) xg->plotsize.height;
  xg->minxy = MIN(xg->max.x, xg->max.y);
  xg->mid.x = xg->max.x / 2;
  xg->mid.y = xg->max.y / 2;

  find_plot_center(xg);
  plane_to_screen(xg);

  if (xg->is_xyplotting)
  {
    /* set axes using ticks0 */
    convert_ticks(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);

    /*
     * If the user hasn't scaled up the data, then use plot_too_big()
     * to make sure the tick and axis labels will fit in the
     * plot window.
    */
    if (xg->scale.x <= xg->scale0.x && xg->scale.y <= xg->scale0.y)
    {
      while (plot_too_big(xg))
      {
        xg->scale0.x = .97 * xg->scale0.x;
        xg->scale.x = xg->scale0.x;
        xg->scale0.y = .97 * xg->scale0.y;
        xg->scale.y = xg->scale0.y;

        plane_to_screen(xg);
        convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
        convert_axes(&xg->ticks0, xg);
      }
    }

    /* set ticks using current axes */
    convert_ticks(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
    extend_axes(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
  }
  else if (xg->is_plotting1d)
  {
    /* set axes using ticks0 */
    convert_ticks(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
    /* set ticks using current axes */
    convert_ticks(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
    extend_axes(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
  }

  /*
   * This causes the var_panel widget to take on its maximum height whenever
   * there is a resize event.
  */
  XtVaGetValues(xg->workspace, XtNheight, &xg->plotsize.height, NULL);
  XtVaSetValues(xg->var_panel, XtNheight, xg->plotsize.height, NULL);

  if (xg->is_brushing)
    assign_points_to_bins(xg);

/*
 * At this point, the expose callback is executed.
*/
}

/* ARGSUSED */
XtCallbackProc
exit_panel_cback(Widget w, xgobidata *xg, XtPointer cd)
{
  xg->is_realized = False;
  XtDestroyWidget(xg->shell);
}

/* ARGSUSED */
XtCallbackProc
exit_solo_cback(Widget w, xgobidata *xg, XtPointer cd)
{
  xg->is_realized = False;
  XtDestroyApplicationContext(app_con);
  exit(0);
}

/* ARGSUSED */
XtEventHandler
varselect(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  int j;
  XButtonEvent *event = (XButtonEvent *) evnt;

  if (event->button == 1 || event->button == 2)
  {
    for (j=0; j<xg->ncols_used; j++)
      if (xg->vardraww[j] == w)
        break;

    selectVariable(xg, j, event->button, event->state);
  }
}

Boolean
selectVariable(xgobidata *xg, int j, int button, int state)
{
  Boolean newvar = false;

  if (xg->is_xyplotting)
    newvar = xy_varselect(j, button, state, xg);

  else if (xg->is_plotting1d)
  {
    newvar = plot1d_varselect(j, button, state, xg);
    plot1d_texture_var(xg);
  }
  else if (xg->is_spinning)
  {
    newvar = spin_varselect(j, button, state, xg);
    draw_last_touched(xg);
  }
  else if (xg->is_touring &&
      !xg->is_backtracking &&
      !xg->is_local_scan &&
      xg->tour_link_state != receive)
  {
    /* event used by interactive gt */
    newvar = tour_varselect(j, state, xg);
  }
  else if (xg->is_corr_touring && !(corr_backtracking(xg)))
    newvar = corr_varselect(j, button, state, xg);

  if (newvar) {
    world_to_plane(xg);
    plane_to_screen(xg);

    if (xg->is_xyplotting)
      init_ticks(&xg->xy_vars, xg);
    else if (xg->is_plotting1d)
      init_ticks(&xg->plot1d_vars, xg);

    /*
     * If brushing, screen coordinates just changed and
     * need to re-bin points before plotting.
    */
    if (xg->is_brushing) {
      assign_points_to_bins(xg);

      if (xg->brush_mode == transient)
        reinit_transient_brushing(xg);
    }

    plot_once(xg);
  }
  return (true);
}

/* ARGSUSED */
XtEventHandler
varexpose(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  int j;

  if (evnt->xexpose.count == 0) {  /* Compress expose events */
    for (j=0; j<xg->ncols_used; j++)
      if (xg->vardraww[j] == w)
        break;

    refresh_vbox(xg, j, 0);
  }
}

/*
 * This section was written by Juergen Symanzik <symanzik@iastate.edu>,
 * modified by dfs.
*/

void Clone_XGobi ()
{
  char command[200], origfname[120];
  char fullfname[120], rootfname[120];
  char newtitle[256];
  static int version = 0;
  char *xgobidir;
  struct stat buf;
  char xgobi_exec[164];
  extern xgobidata xgobi;
  xgobidata *xg = (xgobidata *) &xgobi;

  /* set up variables for resources */
  strcpy (origfname, xg->datafilename);

  xg->isCloned = True;
  xg->clone_PID = getpid ();
  xg->clone_Time = version;
  xg->clone_Name = xg->datarootname;
  xg->delete_clone_data = True;

  sprintf (rootfname, "xg_%d_%d", xg->clone_PID, xg->clone_Time);
  sprintf (fullfname, "/tmp/");
  strcat(fullfname, rootfname);
  strcpy (xg->datafilename, fullfname);
  version++;
  
  brush_save_colors((char *) NULL, (int *) NULL, xg->nrows, xg);
  brush_save_glyphs((char *) NULL, (int *) NULL, xg->nrows, xg);
  brush_save_erase((char *) NULL, (int *) NULL, xg->nrows, xg);
  copy_resources ((char *) NULL, xg);

  save_lines((char *) NULL, (int *) NULL, xg->nrows, xg);
  save_line_colors((char *) NULL, (int *) NULL, xg->nrows, xg);
  save_collabels ((char *) NULL, (int *) NULL, (int) NULL,
    2 /* data_ind = TFORMDATA */, xg);
  save_rowlabels ((char *) NULL, (int *) NULL, xg->nrows, xg);
  write_binary_data((char *) NULL, (int *) NULL, xg->nrows,
    (int *) NULL, xg->ncols_used, 1, xg);  /* 1= tform2 */

  if (xg->is_missing_values_xgobi) {
    extern int nimputations;
    extern char **imp_names;
    extern float **imp_values;

    save_missing((char *) NULL, (int *) NULL, xg->nrows,
      (int *) NULL, xg->ncols_used, xg);

    /* And what about imputations?  copy .imp and .impnames */
    read_imputation_data(xg);
    if (nimputations > 0) {
      read_imputation_names(xg);
      copy_impnames((char *) NULL, xg);
      copy_imputations((char *) NULL, xg);
    }
  }

  if (xg->progname == NULL || xg->progname == "xgobi") {
    xgobidir = getenv("XGOBID");
    if (xgobidir == NULL || strlen(xgobidir) == 0) {
      xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
      (void) strcpy(xgobidir, DEFAULTDIR);
    }
    sprintf(xgobi_exec, "%s/bin/xgobi", xgobidir);
    
    /* If no luck there, then just try 'xgobi' without a path name */
    if (stat(xgobi_exec, &buf) != 0)
      sprintf(xgobi_exec, "xgobi");
  } else {
    sprintf(xgobi_exec, xg->progname);
  }

  if (mono)
    strcat(xgobi_exec, " -mono");

  sprintf(newtitle, "%s[%d]", xg->title, version);

  if (xg->is_missing_values_xgobi)
    sprintf(command, "%s -vtitle \"'%s'\" %s.missing &",
      xgobi_exec, newtitle, xg->datafilename);
  else
    sprintf(command, "%s -vtitle \"'%s'\" %s &",
      xgobi_exec, newtitle, xg->datafilename);

  fprintf(stderr, "Executing:  %s\n", command);
  system (command);

/*
 * Now restore the present xgobi's name
*/
  strcpy (xg->datafilename, origfname);
}

/* ARGSUSED */
XtCallbackProc
clone_xgobi_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Clone_XGobi();
}

void Clone_XGobi_Scatmat ()
{
  char command[200], origfname[120];
  char fullfname[120], rootfname[120];
  char newtitle[256];
  static int version = 0;
  char *xgobidir;
  struct stat buf;
  char xgobi_exec[164];
  extern xgobidata xgobi;
  xgobidata *xg = (xgobidata *) &xgobi;

  /* set up variables for resources */
  strcpy (origfname, xg->datafilename);

  xg->isCloned = True;
  xg->clone_PID = getpid ();
  xg->clone_Time = version;
  xg->clone_Name = xg->datarootname;
  xg->delete_clone_data = True;

  sprintf (rootfname, "xg_%d_%d", xg->clone_PID, xg->clone_Time);
  sprintf (fullfname, "/tmp/");
  strcat(fullfname, rootfname);
  strcpy (xg->datafilename, fullfname);
  version++;
  
/*
 * has_data & has_cdf_data are variables within Juergen's program.
*/

  brush_save_colors((char *) NULL, (int *) NULL, xg->nrows, xg);
  brush_save_glyphs((char *) NULL, (int *) NULL, xg->nrows, xg);
  brush_save_erase((char *) NULL, (int *) NULL, xg->nrows, xg);
  copy_resources ((char *) NULL, xg);

  save_lines((char *) NULL, (int *) NULL, xg->nrows, xg);
  save_line_colors((char *) NULL, (int *) NULL, xg->nrows, xg);
  save_collabels ((char *) NULL, xg->tour_vars, xg->numvars_t,
    2 /* data_ind = TFORMDATA */, xg);
  save_rowlabels ((char *) NULL, (int *) NULL, xg->nrows, xg);
  write_binary_data((char *) NULL, (int *) NULL, xg->nrows,
    xg->tour_vars, xg->numvars_t, 1, xg);  /* 1= tform2 */

  if (xg->is_missing_values_xgobi) {
    extern int nimputations;
    extern char **imp_names;
    extern float **imp_values;

    save_missing((char *) NULL, (int *) NULL, xg->nrows,
      (int *) NULL, xg->ncols_used, xg);

    /* And what about imputations?  copy .imp and .impnames */
    read_imputation_data(xg);
    if (nimputations > 0) {
      read_imputation_names(xg);
      copy_impnames((char *) NULL, xg);
      copy_imputations((char *) NULL, xg);
    }
  }

  if (xg->progname == "xgobi") {
    xgobidir = getenv("XGOBID");
    if (xgobidir == NULL || strlen(xgobidir) == 0) {
      xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
      (void) strcpy(xgobidir, DEFAULTDIR);
    }
    sprintf(xgobi_exec, "%s/bin/xgobi", xgobidir);
    
    /* If no luck there, then just try 'xgobi' without a path name */
    if (stat(xgobi_exec, &buf) != 0)
      sprintf(xgobi_exec, "xgobi");
  } else {
    sprintf(xgobi_exec, xg->progname);
  }

  if (mono)
    strcat(xgobi_exec, " -mono");

  strcat(xgobi_exec, " -scatmat");

  sprintf(newtitle, "%s[%d]", xg->title, version);

  if (xg->is_missing_values_xgobi)
    sprintf(command, "%s -vtitle \"'%s'\" %s.missing &",
      xgobi_exec, newtitle, xg->datafilename);
  else
    sprintf(command, "%s -vtitle \"'%s'\" %s &",
      xgobi_exec, newtitle, xg->datafilename);

  fprintf(stderr, "Executing:  %s\n", command);
  system (command);

/*
 * Now restore the present xgobi's name
*/
  strcpy (xg->datafilename, origfname);
}

/* ARGSUSED */
XtCallbackProc
scatmat_xgobi_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Clone_XGobi_Scatmat();
}

/* ARGSUSED */
XtCallbackProc
start_xplore_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
#ifdef XPLORE
  startxplore (xg);
#endif
}

/* ARGSUSED */
XtCallbackProc
stop_xplore_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
#ifdef XPLORE
  stopxplore (xg);
#endif
}

#undef NAMESIZE
#undef NRES
#undef NAMESV
