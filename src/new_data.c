/* new_data.c */
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
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

/*CORBA:  Added by Ross and Duncan. */
int read_data_from_file(char *filename, xgobidata *xg);
void initializeNewData(xgobidata *xg, char *name, int ncols_prev,
  int ncols_used_prev, int readFiles);
#define DEFAULT_COLOR color_nums[0]
#define DEFAULT_GLYPH

/*
XtCallbackProc
place_ndata_popup(Widget w, xgobidata *xg, XtPointer callback_data)
 * Determine where to place the popup window in which the user enters
 * a filename; invoked before ndata_read.
{
  turn_on_xyplotting(xg);
  map_xyplot(True);

  (void) strcpy(xg->save_type, READ_DATA );
  fname_popup(w, xg);
}
*/


void reinit_1 (xgobidata *xg, int ncols_prev, int ncols_used_prev)
{
  int k;
  extern Widget varlist_popup, caselist_popup;

  if (xg->is_pp)
    turn_off_pp(xg);
  if (xg->is_princ_comp)
    turn_off_pc(xg);

  /* reset xgobi's mode to xyplotting; this makes life a lot easier */
  turn_on_xyplotting(xg);

  free_plot_arrays();
  free_brush_arrays(xg);
  free_rotate_arrays();
  free_line_edit_arrays(xg);
  free_axis_arrays(xg);
  free_pipeline_arrays(xg);
  if (ncols_used_prev > 2)
  {
    free_tour(xg);
    free_corr(xg);
  }
  free_smooth_arrays();

  /* missing values 'shadow' matrix */
  if (xg->missing_values_present && xg->is_missing) {
    for (k=0; k<xg->nrows; k++)
      XtFree((XtPointer) xg->is_missing[k]);
    XtFree((char *) xg->is_missing);
    xg->is_missing = NULL;
  }

  for (k=0; k<xg->nrows; k++)
    XtFree((XtPointer) xg->raw_data[k]);
  XtFree((XtPointer) xg->raw_data);
  xg->raw_data = NULL;

  for (k=0; k<ncols_prev; k++)
    XtFree((XtPointer) xg->collab[k]);
  XtFree((XtPointer) xg->collab);
  for (k=0; k<ncols_prev; k++)
    XtFree((XtPointer) xg->collab_tform1[k]);
  XtFree((XtPointer) xg->collab_tform1);
  xg->collab_tform1 = NULL;
  for (k=0; k<ncols_prev; k++)
    XtFree((XtPointer) xg->collab_tform2[k]);
  XtFree((XtPointer) xg->collab_tform2);
  xg->collab_tform2 = NULL;

  for (k=0; k<xg->nrows; k++)
    XtFree((XtPointer) xg->rowlab[k]);
  XtFree((XtPointer) xg->rowlab);
  xg->rowlab = NULL;

  XtFree((XtPointer) xg->vgroup_ids);
  /* These are handled when the new arrays are read or generated */
  /*XtFree((XtPointer) xg->connecting_lines);*/
  /*XtFree((XtPointer) xg->erased);*/
  XtFree((XtPointer) xg->last_forward);

  free_rgroups(xg);
  free_lgroups(xg);

  if (varlist_popup != NULL) {
    XtDestroyWidget(varlist_popup);
    varlist_popup = NULL;
    free_varlist(xg);
  }
  if (caselist_popup != NULL) {
    XtDestroyWidget(caselist_popup);
    caselist_popup = NULL;
    free_caselist(xg);
  }
}

void reinit_3 (xgobidata *xg, char *filename, char *name,
int ncols_prev, int ncols_used_prev)
{
  (void) read_vgroups(filename, True, xg);
  (void) read_rgroups(filename, True, xg);  /* row groups */
  (void) read_connecting_lines(filename, True, xg);
  (void) set_lgroups(True, xg);  /* line groups */
  (void) read_erase(filename, True, xg);
  (void) read_nlinkable(filename, True, xg);

  initializeNewData(xg, name, ncols_prev, ncols_used_prev, 1);
}

void
initializeNewData(xgobidata *xg, char *name, int ncols_prev,
  int ncols_used_prev, int readFiles)
{

  /*
   * Activate deletion if .linkable file is not used;
   * deactivate if nlinkable != nrows.
  */
  /*set_deletion(xg->nlinkable == xg->nrows);*/

  xg->last_forward = (int *) XtMalloc ((Cardinal) xg->nrows * sizeof (int));

  destroy_varsel_widgets(ncols_prev, xg);

  set_title_and_icon(name, xg);
  alloc_pipeline_arrays(xg);
  alloc_plot_arrays(xg);
  alloc_brush_arrays(xg);  /* has to be done before read_brush_indices() */
  alloc_rotate_arrays(xg);
  alloc_line_edit_arrays(xg);
  alloc_axis_arrays(xg);

  alloc_smooth_arrays(xg);

  if (readFiles &&
      read_point_colors(xg->datafname, True, True, xg) &&
      read_point_glyphs(xg->datafname, True, True, xg))
  {
    init_line_colors(xg);
  } else {
    init_glyph_ids(xg);
    init_color_ids(xg);
  }

  xg->nrows_in_plot = xg->nrows;

  build_varlist(xg);
  build_caselist(xg);

  init_plot1d_vars(xg);
  init_xyplot_vars(xg);
  init_rotate_vars(xg);
  init_brush_vars(xg);
  init_scale_vars(xg);
  init_identify_vars(xg);
  init_line_edit_vars(xg);

  if (xg->ncols_used > 2)
  {
    alloc_tour(xg);
    reinit_tour_hist(xg);
    if (ncols_used_prev > 2)
      init_tour(xg, 0);
    else
      init_tour(xg, 1);
  }
  copy_raw_to_tform(xg);
  if (xg->ncols_used > 2)
  {
    alloc_corr(xg);
    init_corr(xg);
  }

/*
 * See whether rotate and tour should be sensitive or insensitive.
*/
  reset_3d_cmds(xg);
/*
 * Backtracking has to be off, and the forward/backward button insensitive.
*/
  reset_backtrack_cmd(true, false, true, false);
  reset_cycleback_cmd(true, false, "F");

/*
 * If case profile plot is up, close it
*/
  if (xg->is_cprof_plotting)
    turn_off_cprof_plotting(xg);

  if (xg->ncols_used > 2)
    compute_vc_matrix(xg);
  update_lims(xg);
  update_world(xg);

/*
 * Reinitialize variable selection panel.
*/
  set_varsel_label(xg);
  XSync(display, False);
  reset_var_panel(xg);

/*
 * Reinitialize the brush menus
 *init_brush_menus();
*/

/*
 * Reinit the missing menu entries.
*/
  set_sens_missing_menu_btns(xg->missing_values_present);

  world_to_plane(xg);
  plane_to_screen(xg);
/*
 * Reinitialize axes and ticks.
*/
  init_axes(xg, False);
  init_ticks(&xg->xy_vars, xg);
}

void
refreshXGobiDisplay(xgobidata *xg) {
  /* take from reinit_3 */
  update_required = true;

  if (xg->ncols_used > 2)
    compute_vc_matrix(xg);
  update_lims(xg);
  update_world(xg);

/*
 * Reinitialize variable selection panel.
*/
  set_varsel_label(xg);
  XSync(display, False);
  reset_var_panel(xg);

/*
 * Reinitialize the brush menus
 *init_brush_menus();
*/

/*
 * Reinit the missing menu entries.
*/
  set_sens_missing_menu_btns(xg->missing_values_present);

  world_to_plane(xg);
  plane_to_screen(xg);
/*
 * Reinitialize axes and ticks.
*/
  init_axes(xg, False);
  init_ticks(&xg->xy_vars, xg);
}

void
read_new_data(Widget w, xgobidata *xg)
{
  char *filename = XtMalloc(132 * sizeof(char));
  XtVaGetValues(w, XtNstring, (String) &filename, NULL);

  read_data_from_file(filename, xg);
}

/*
 This routine comes directly from the routine above
 but removes the reliance on the widget.
 This is allows us to call it with the name of a new file
 and have XGobi load the contents of that file and auxilary 
 files.
 */
int
read_data_from_file(char *filename, xgobidata *xg)
{
  char fname[100];
  float ftmp;
  FILE *fp;
  int ncols_prev = xg->ncols;
  int ncols_used_prev = xg->ncols_used;

/*
 * All very interesting, but not useful so far.  Perhaps if I
 * begin looking for "name filename" in the resource file, it
 * will turn out to be useful after all.
 *
 *  XrmDatabase *newdb = (XrmDatabase *) NULL;
 *  XrmPutStringResource(&newdb,
 *    (String) "*title", (String) "NewData");
 *  XrmPutStringResource(&newdb,
 *    (String) "*iconName", (String) "NewData");
 *
 *  XrmMergeDatabases(newdb, &display->db);
 *  XrmPutFileDatabase(display->db, "/u/dfs/xgobi/DBold");
*/
  /* For now, only read ascii or binary data files */
  /*  if (xg->data_mode == Sprocess )
  {
    XtFree(filename);
    return;
  }
  else */
  if ((fp = fopen(filename, "r")) == NULL)
  {
    strcpy(fname, filename);
    strcat(fname, ".dat");
    if ((fp = fopen(fname, "r")) == NULL)
    {
      char message[MSGLENGTH];
      sprintf(message, "Failed to open either %s or %s for reading\n",
        filename, fname);
      show_message(message, xg);
      XtFree(filename);
      return (-1);
    }
  }

/* Check that this is truly a readable file, not a directory */
  if (fscanf(fp, "%f", &ftmp) < 0)
  {
    char message[MSGLENGTH];
    sprintf(message, "Failed to open either %s or %s for reading\n",
      filename, fname);
    show_message(message, xg);
    XtFree(filename);
    return (-1);
  }
/* Check that the file closes normally */
  if (fclose(fp) == EOF)
  {
    char message[MSGLENGTH];
    sprintf(message, "Failed to close %s normally.\n",
      filename);
    show_message(message, xg);
    XtFree(filename);
    return (-1);
  }

/* Since the file is okay, do a couple of filename assignments */
  strcpy(xg->datafilename, filename);
  (void) strip_suffixes(xg);

/*
 * Now that we know this is a reasonable file, free the old
 * arrays before determining a new xg->nrows and xg->ncols.
*/
  reinit_1(xg, ncols_prev, ncols_used_prev);

  (void) read_array(xg);
  fill_extra_column(xg);
  (void) read_extra_resources(filename);
  (void) read_collabels(filename, True, xg);
  (void) read_rowlabels(filename, True, xg);

  reinit_3 (xg, filename, filename, ncols_prev, ncols_used_prev);

  update_required = True;
}

void
re_read_data(Widget w, xgobidata *xg)
{
  int i,j;
  char *filename;
  char fname[100];
  FILE *fp;

  filename = XtMalloc(132 * sizeof(char));
  XtVaGetValues(w, XtNstring, (String) &filename, NULL);

  if ((fp = fopen(filename, "r")) == NULL)
  {
    strcpy(fname, filename);
    strcat(fname, ".dat");
    if ((fp = fopen(fname, "r")) == NULL)
    {
      char message[MSGLENGTH];
      sprintf(message, "Failed to open either %s or %s for reading\n",
        filename, fname);
      show_message(message, xg);
      XtFree(filename);
      return;
    }
     i=0; j=0; 
     while (fscanf(fp, "%f", &xg->raw_data[i][j]) != EOF)
     {
       j++;
       if (j == xg->ncols-1)
       { 
         i++; 
         j = 0;
       }
     }
     if (i < xg->nrows)
     {
       char message[MSGLENGTH];
       sprintf(message, "Fewer rows read than in the original data set\n");
       show_message(message, xg);
     }
  }

/* Check that the file closes normally */
  if (fclose(fp) == EOF)
  {
    char message[MSGLENGTH];
    sprintf(message, "Failed to close %s normally.\n",
      filename);
    show_message(message, xg);
    XtFree(filename);
    return;
  }

  copy_raw_to_tform(xg);
  if (xg->ncols_used > 2)
    compute_vc_matrix(xg);
  update_lims(xg);

  update_world(xg);

  world_to_plane(xg);
  plane_to_screen(xg);
/*
 * Reinitialize axes and ticks.
*/
  init_axes(xg, False);
  init_ticks(&xg->xy_vars, xg);

  plot_once(xg);

  XtFree(filename);
}
