/* missing.c */
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
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#include "xgobi_config.h"

static Boolean use_brush_groups = False;

static Widget ipopup = (Widget) NULL;
Widget specify_txt, pct_above_txt, pct_below_txt;

#define NVSPECBTNS 2
Widget varspec_menu, varspec_menu_cmd, varspec_menu_btn[NVSPECBTNS];
#define VSPEC_ALL 0
#define VSPEC_SEL_AND_VG 1
static int varspec_type = VSPEC_SEL_AND_VG;
/* these two depend on varspec_type */
static int vars_to_impute[NCOLS];
static int nvars_to_impute = 0;
/*
 * This is the cumulative number of missings per column,
 * which will allow me to index into impval in order to
 * read in the imputed values for selected columns only.
*/
static int *nmissing_indx = (int *) NULL;

Widget imputation_menu, imputation_menu_cmd, *imputation_menu_btn;
Widget impname_label;
static Widget rescale_cmd;
static Boolean rescale = True;
static int imputation_indx;

enum {pct_above, pct_below, specified} fixed_val;

void
init_missing_array(int nr, int nc, xgobidata *xg) {
  /* Allocate the missing values array */
  int k, p;

  xg->is_missing = (short **)
    XtMalloc((Cardinal) nr * sizeof(short *));
  for (k=0; k<nr; k++) {
    xg->is_missing[k] = (short *) XtMalloc((Cardinal) nc * sizeof(short));
    for (p=0; p<nc; p++)
      xg->is_missing[k][p] = 0;
  }
}

static int
set_vars_to_impute(xgobidata *xg)
/* 
 * Determine which variables are to be subject to imputation.
*/
{
  int j, nvgroups;

  switch (varspec_type) {

    case VSPEC_ALL:
      nvars_to_impute = xg->ncols_used;
      for (j=0; j<xg->ncols_used; j++)
        vars_to_impute[j] = j;
      break;

    case VSPEC_SEL_AND_VG:
      nvars_to_impute = find_selected_cols(xg, vars_to_impute);
      if ( (nvgroups = numvargroups(xg)) < xg->ncols_used ) 
        add_vgroups(xg, vars_to_impute, &nvars_to_impute);
      break;
  }
}

void
set_nmissing_indx(xgobidata *xg)
{
  int i, j, k;

  nmissing_indx = (int *) XtMalloc((Cardinal) xg->ncols * sizeof(int));

  k = 0;
  for (j=0; j<xg->ncols; j++) {
    nmissing_indx[j] = k;
    for (i=0; i<xg->nrows; i++) {
      if (xg->is_missing[i][j]) k++;
    }
  }
}

void
read_missing_values(xgobidata *xg)
{
  int i, j, ok, itmp, row, col;
  int nmissing = 0;
  FILE *fp;
  static char *suffixes[] = {".missing"};

  if (xg->file_read_type != read_all)
    return;

  if ( (fp = open_xgobi_file(xg->datafname, 1, suffixes, "r", true)) != NULL)
  {

    init_missing_array(xg->nrows, xg->ncols, xg);

    j = 0;
    i = 0;
    while ((ok = fscanf(fp, "%d", &itmp)) != EOF) {
      row = i;
      col = j;
      j++;
      if (j==xg->ncols_used) { j=0; i++; }
      if (i==xg->nrows && j>0) ok = False;

      if (!ok) {
        fprintf(stderr, "Problem reading %s.missing;\n", xg->datafname);
        fprintf(stderr, "row %d, column %d.\n", i, j);
        fprintf(stderr, "Make sure sizes of %s and %s.missing match\n",
          xg->datafilename, xg->datafname);
        fclose(fp);
        exit(1);
      }

      xg->is_missing[row][col] = itmp;
      if (itmp != 0) nmissing++;
    }

    if (xg->nmissing != 0 && xg->nmissing != nmissing) {
      fprintf(stderr, "I found %d missing values in your data file but\n",
        xg->nmissing);
      fprintf(stderr, "%d missing values in your .missing file.",
        nmissing);
      fprintf(stderr, "I'll use the .missing results.\n");
    }
    xg->nmissing = nmissing;

    fclose(fp);
    xg->missing_values_present = True;
  }
}

void 
init_missing_groupvar(xgobidata *xg)
{
/*
 * Initialize
 * the added column of the missing values matrix
 * when the group variable is generated.
*/
  int i;

  for (i=0; i<xg->nrows; i++)
  {
    xg->is_missing[i][xg->ncols-1] =
      xg->is_missing[i][xg->ncols-1] = (short) 0;
  }
}

/* ARGSUSED */
static XtCallbackProc
rescale_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  rescale = !rescale;
}

/*************************************************************************/
/* Here's the part that pertains to the imputation files:
 *  Read in an m x k file that contains
 *   k imputations of
 *   m missing values, arranged column-wise
 *  The file's name is datafname.imp

 *  (Optional) read in a file of k imputation names
 *  The file's name is datafname.impnames
*/

#define MAX_NIMPUTATIONS 100
#define IMPNAMELEN 32
int nimputations;
char **imp_names;
float **imp_values;

void
read_imputation_data(xgobidata *xg)
{
  int k, i, ch;
  char fname[128];
  float row1[MAX_NIMPUTATIONS];
  FILE *fp;
  static Boolean initd = False;
  static char *suffixes[] = {".imp"};

  if (!initd) {

    nimputations = 0;

    sprintf(fname, "%s.imp", xg->datafname);

    if ( (fp = open_xgobi_file(xg->datafname, 1, suffixes, "r", true)) != NULL)
    {
      /* Read in the first row to get the number of imputations */

      k = 0;
      while ( (ch = getc(fp)) != '\n')
      {
        if (ch == ' ' || ch == '\t')
          continue;
        else
          ungetc(ch, fp);
        
        if (fscanf(fp, "%f", &row1[k++]) < 0)
        {
          fprintf(stderr,
            "error in reading first row of %s\n", fname);
          fclose(fp);
          exit(0);
        }
        else if (k >= MAX_NIMPUTATIONS)
        {
          fprintf(stderr,
            "increase MAX_NIMPUTATIONS and recompile\n");
          fclose(fp);
          exit(0);
        }
      }
      nimputations = k;

      imp_values = (float **) XtMalloc(
        (Cardinal) xg->nmissing * sizeof (float *));
      for (k=0; k<xg->nmissing; k++)
        imp_values[k] = (float *) XtMalloc(
          (Cardinal) nimputations * sizeof(float));

      /* Copy row1 into the data array */
      for (k=0; k<nimputations; k++)
        imp_values[0][k] = row1[k];

      /* Now read the rest of the data, one row at a time */
      for (i=1; i<xg->nmissing; i++)
        for (k=0; k<nimputations; k++)
          if (fscanf(fp, "%f", &imp_values[i][k]) < 0) 
          {
            fprintf(stderr,
              "Do you have enough imputed values?  I've got a problem\n");
            fprintf(stderr,
              "at row %d, column %d\n", i, k);
          }

      }
     initd = True;
  }
}

void
read_imputation_names(xgobidata *xg)
{
  int i, k, len;
  char fname[128];
  char str[2*IMPNAMELEN];
  FILE *fp;
  static Boolean initd = False;

  if (!initd) {

    sprintf(fname, "%s.impnames", xg->datafname);
    /* allocate the array of imputation names */
    imp_names = (char **) XtMalloc(
      (Cardinal) nimputations * sizeof (char *));
    for (k=0; k<nimputations; k++)
      imp_names[k] = (char *) XtMalloc(
        (Cardinal) IMPNAMELEN * sizeof(char));

    if ( (fp = fopen(fname,"r")) != NULL)
    {

      k = 0;
      while (fgets(str, 2*IMPNAMELEN-1, fp) != NULL)
      {
        len = MIN(INT(strlen(str)), IMPNAMELEN-1) ;
        while (str[len-1] == '\n' || str[len-1] == ' ')
          len-- ;
        str[len] = '\0' ;
        strcpy(imp_names[k], str) ;
        k++;
        if (k >= nimputations)
          break;
      }

      if (k != nimputations)
      {
        (void) fprintf(stderr,
          "The number of imputations in %s.imp file = %d,\n",
          xg->datafname, nimputations);
        (void) fprintf(stderr,
          "while the number of labels supplied is %d\n", k);
        
        for (i=k; i<nimputations; i++)
          (void) sprintf(imp_names[i], "Imp %d", i+1);
      }
      fclose(fp);
    }
    else  /* If there is no file of names, use default imputation names */
    {
      for (i=0; i<nimputations; i++)
        (void) sprintf(imp_names[i], "Imp %d", i+1);
    }
    initd = True;
  }
}

/* ARGSUSED */
XtCallbackProc
launch_missing_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  char fname[256], command[512], title[128];
  char *xgobidir;
  char xgobi_exec[256];
  struct stat buf;
  Boolean success = True, foundit = False;
  FILE *fp;

  sprintf(fname, "%s.missing", xg->datafname);
  /*
   * If the .missing file hasn't been created, now's the
   * time to write it out.
   *
   * And if the "-only" argument has been used, better
   * write out a new .missing file whether one is present or not.
  */
  if ((fp = fopen(fname, "r")) == NULL)
    success =  save_missing((char *) NULL, (int *) NULL, xg->nrows,
      (int *) NULL, xg->ncols_used, xg);
  else if (xg->file_read_type != read_all) {
    fprintf(stderr, "Moving %s to %s.bak\n", fname, fname);
    sprintf(command, "mv %s %s.bak\n", fname, fname);
    system(command);
    success =  save_missing((char *) NULL, (int *) NULL, xg->nrows,
      (int *) NULL, xg->ncols_used, xg);
  }
  else
    fclose(fp);
  sprintf(title, "");
  if (success) {

    /* Get the title of the current xgobi */
    char *str_type[50];
    XrmValue value;

    if (XrmGetResource((XrmDatabase) XrmGetDatabase(display),
      "xgobi.title", "XGobi.Title", str_type, &value))
    {
      fprintf(stderr, "%s\n", value.addr);
    }

    sprintf(title, " -vtitle \"'%s-Missing'\"", xg->title);

    if (xg->progname == "xgobi") {
      xgobidir = getenv("XGOBID");
      if (xgobidir == NULL || strlen(xgobidir) == 0) {
        xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
        (void) strcpy(xgobidir, XGOBI_DEFAULTDIR);
      }

      sprintf(xgobi_exec, "%s/bin/xgobi", xgobidir);
      if (stat(xgobi_exec, &buf) == 0);
        foundit = True;

      /* If it's not in the bin directory, try the source directory */
      if (!foundit) {
        sprintf(xgobi_exec, "%s/xgobi", xgobidir);
        if (stat(xgobi_exec, &buf) == 0)
          foundit = True;
      }

      /* If still no luck, then just use 'xgobi' without a path name */
      if (!foundit)
        sprintf(xgobi_exec, "xgobi");

    } else
      sprintf(xgobi_exec, xg->progname);

    /* Add the title, if we've got one */
    if (strlen(title) > 0) {
      strcat(xgobi_exec, " ");
      strcat(xgobi_exec, title);
    }

    if (mono)
      strcat(xgobi_exec, " -mono");
    sprintf(command, "%s %s &", xgobi_exec, fname);

    fprintf(stderr, "%s\n", command);
    system (command);

    /* This doesn't work: presumably it's being called too early. */
    /* update_linked_brushing(xg); */
  }
}

void
rejitter_imputations(xgobidata *xg)
{
/*
 * It turns out to be convex_jittering that's messing me up
 * when cycling through imputations.  A perfectly good jitter
 * value for the center of the plot is not a good value out
 * at the edges.  So I have to rejitter everything that's
 * moved.  I hope this isn't way too slow.
*/

  extern float *jitfacv;
  extern Boolean convex_jittering;
  float rdiff, ftmp;
  float precis = PRECISION1;  /* 32768 */
  int i, j, k, m;

  if (jitfacv != NULL) {
    for (k=0; k<nvars_to_impute; k++) {
      j = vars_to_impute[k];
      if (jitfacv[j] > 0.) {
        for (i=0; i<xg->nrows_in_plot; i++) {
          m = xg->rows_in_plot[i];
	      if (xg->is_missing[m][j]) {
            if (convex_jittering) {
              /* update the value in world_data[][] */
              rdiff = xg->lim[j].max - xg->lim[j].min;
              ftmp = -1.0 + 2.0*(xg->tform2[m][j] - xg->lim[j].min)/rdiff;
              xg->world_data[m][j] = (long) (precis * ftmp);
            }
            jitter_one_value(m, j, xg);
          }
        }
      }
    }
  }
}

void
update_imputation(xgobidata *xg)
{
  extern void transform_all(xgobidata *);
  transform_all(xg);

/*
 * And now the pipeline ...
*/

  if (xg->is_princ_comp && xg->ncols_used >= 2)
  {
    if (update_vc_active_and_do_svd(xg, xg->numvars_t, xg->tour_vars))
      spherize_data(xg, xg->numvars_t, xg->numvars_t, xg->tour_vars);
    else
      copy_tform_to_sphered(xg);
  }

/* ?? di and dfs ?? */
  if (xg->is_plotting1d)
    plot1d_texture_var(xg);

/*
 * Could update the limits for the parallel coordinates
 * plot even if we aren't updating for the main plot.
 * update_cprof_lims(xg);
*/
  if (rescale)
    update_lims(xg);

  /* rejitter imputed values */
  rejitter_imputations(xg);

  update_world(xg);
  world_to_plane(xg);
  plane_to_screen(xg);

  if (rescale) {
    init_axes(xg, False);
    init_tickdelta(xg);
    if (xg->is_xyplotting)
      init_ticks(&xg->xy_vars, xg);
    else if (xg->is_plotting1d)
      init_ticks(&xg->plot1d_vars, xg);
  }

  if (xg->is_brushing)
    assign_points_to_bins(xg);

  plot_once(xg);

  /* Case profile */
  if (xg->is_cprof_plotting && xg->link_cprof_plotting)
  {
    update_cprof_plot(xg);
    cprof_plot_once(xg);
  }
}

static void
impute_single(int *missv, int nmissing, int *presv, int npresent, int col,
xgobidata *xg)
{
  int i, k;
  float rrand;

  /*
   * Then loop over the missing values, plugging in some value
   * drawn from the present values.
  */
  for (i=0; i<nmissing; i++) {
    for (k=0; k<npresent; k++) {
      rrand = (float) randvalue();

      if ( ((npresent - k) * rrand) < 1.0 ) {
        xg->raw_data[missv[i]][col] = xg->raw_data[presv[k]][col];
        /*
         * This is the default -- transformations will be applied
         * later to those that need it.
        */
        xg->tform1[missv[i]][col] = xg->tform1[presv[k]][col];
        xg->tform2[missv[i]][col] = xg->tform2[presv[k]][col];
        break;
      }
    }
  }
}

static void
reset_imputation_label(int indx)
/* Reset the name  of the label */
{
  char str[512];

  if (indx < 0)
    sprintf(str, "Selected: ");
  else
    sprintf(str, "Selected: %s", imp_names[indx]);

  XtVaSetValues(impname_label, XtNlabel, str, NULL);
}

/* ARGSUSED */
static XtCallbackProc
impute_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
/* Perform single imputation */

  int i, j, k, n, m, npresent, *presv, nmissing, *missv;

  presv = (int *) XtMalloc((Cardinal) xg->nrows * sizeof(int));
  missv = (int *) XtMalloc((Cardinal) xg->nrows * sizeof(int));

  set_vars_to_impute(xg);

  if (use_brush_groups && xg->nclust > 1) {

    /* Loop over the number of brushing groups */
    for (n=0; n<xg->nclust; n++) {

      /* Then loop over the number of columns */
      for (m=0; m<nvars_to_impute; m++) {
        npresent = nmissing = 0;
        j = vars_to_impute[m];

        /*
         * And finally over the rows, including only those rows
         * which belong to the current cluster
        */
        for (i=0; i<xg->nrows_in_plot; i++) {
          k = xg->rows_in_plot[i];
          if (xg->raw_data[k][xg->ncols_used-1] == (float) n) { 
            if (!xg->erased[k]) {   /* ignore erased values altogether */
              if (xg->is_missing[k][j])
                missv[nmissing++] = k;
              else
                presv[npresent++] = k;
            }
          }
        }
        impute_single(missv, nmissing, presv, npresent, j, xg);
      }
    }
  }

  else {
    for (m=0; m<nvars_to_impute; m++) {
      npresent = nmissing = 0;
      j = vars_to_impute[m];
      /*
       * Build the vector of indices of present values that can be used
       * to draw from.
      */
      for (i=0; i<xg->nrows_in_plot; i++) {
        k = xg->rows_in_plot[i];
        if (!xg->erased[k]) {   /* ignore erased values altogether */
          if (xg->is_missing[k][j])
            missv[nmissing++] = k;
          else
            presv[npresent++] = k;
        }
      }
      impute_single(missv, nmissing, presv, npresent, j, xg);
    }
  }

  /* Reset the imputation label to null */
  if (nimputations) 
    reset_imputation_label(-1);

  /* Handle transformations, run through pipeline, plot */
  update_imputation(xg);

  XtFree((char *) presv);
  XtFree((char *) missv);
}

/* ARGSUSED */
static XtCallbackProc
use_groups_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  use_brush_groups = !use_brush_groups;

  if (use_brush_groups)
    if (xg->ncols_used < xg->ncols)
      save_brush_groups(xg);
}

/* ARGSUSED */
void
assign_fixed_value(xgobidata *xg)
{
  int i, j, k, m;
  float maxval, minval, range, val, fstr;
  String val_str;

  set_vars_to_impute(xg);

  if (fixed_val == pct_above || fixed_val == pct_below) {

    if (fixed_val == pct_above)
      XtVaGetValues(pct_above_txt, XtNstring, (String) &val_str, NULL);
    else if (fixed_val == pct_below)
      XtVaGetValues(pct_below_txt, XtNstring, (String) &val_str, NULL);

    if (strlen(val_str) == 0) {
      char message[MSGLENGTH];
      sprintf(message,
      "You selected '%% over or under max' but didn't specify a percentage.\n");

      show_message(message, xg);
      return;
    }

    fstr = (float) atof(val_str);
    if (fstr < 0 || fstr > 100) {
      char message[MSGLENGTH];
      sprintf(message,
        "You specified %f%%; please specify a percentage between 0 and 100.\n",
        fstr);
      show_message(message, xg);
      return;
    }

    for (k=0; k<nvars_to_impute; k++) {
       j = vars_to_impute[k];

      /* First find the maximum and minimum values of the non-missing data */
      maxval = xg->lim[j].min;
      minval = xg->lim[j].max;

      for (i=0; i<xg->nrows_in_plot; i++) {
        m = xg->rows_in_plot[i];
        if (!xg->is_missing[m][j]) {
          if (xg->raw_data[m][j] > maxval) maxval = xg->raw_data[m][j];
          if (xg->raw_data[m][j] < minval) minval = xg->raw_data[m][j];
        }
      }
      range = maxval - minval;

      /* Then fill it in */
      if (fixed_val == pct_above)
        val = maxval + (fstr/100.) * range;
      else if (fixed_val == pct_below)
        val = minval - (fstr/100.) * range;

      for (i=0; i<xg->nrows_in_plot; i++) {
        m = xg->rows_in_plot[i];
        if (xg->is_missing[m][j]) {
          xg->raw_data[m][j] = xg->tform1[m][j] = xg->tform2[m][j] =
            val;
        }
      }
    }
  }
  else if (fixed_val == specified) {

    XtVaGetValues(specify_txt, XtNstring, (String) &val_str, NULL);
    if (strlen(val_str) == 0) {
      char message[MSGLENGTH];
      sprintf(message,
        "You've selected 'Specify' but haven't specified a value.\n");
      show_message(message, xg);
      return;
    }
    else {
      val = (float) atof(val_str);
      for (i=0; i<xg->nrows_in_plot; i++) {
        m = xg->rows_in_plot[i];
        for (k=0; k<nvars_to_impute; k++) {
          j = vars_to_impute[k];
          if (xg->is_missing[m][j]) {
            xg->raw_data[m][j] = xg->tform1[m][j] = xg->tform2[m][j] =
              val;
          }
        }
      }
    }
  }

  /* Reset the imputation label to null */
  if (nimputations) 
    reset_imputation_label(-1);
  
  update_imputation(xg);
}

/* ARGSUSED */
static XtCallbackProc
pct_above_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  fixed_val = pct_above;
  assign_fixed_value(xg);
}

/* ARGSUSED */
static XtCallbackProc
pct_below_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  fixed_val = pct_below;
  assign_fixed_value(xg);
}

/* ARGSUSED */
static XtCallbackProc
specify_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  fixed_val = specified;
  assign_fixed_value(xg);
}

static void
reset_menu_mark(void)
{
  int j;
  Arg args[1];

/*
 * Turn off margin indicator on each menu item.
*/
  XtSetArg(args[0], XtNleftBitmap, (Pixmap) None);
  for (j=0; j<nimputations; j++)
    XtSetValues(imputation_menu_btn[j], args, 1);

  XtVaSetValues(imputation_menu_btn[imputation_indx],
    XtNleftBitmap, menu_mark,
    NULL);
}

static void
reset_imputation(xgobidata *xg)
{
/* imputation_indx, **imp_values, xg->nmissing */

  int i, j, k, m;
  int idx = imputation_indx;

  set_vars_to_impute(xg);

  for (m=0; m<nvars_to_impute; m++) {
    j = vars_to_impute[m];
    k = nmissing_indx[j];
    for (i=0; i<xg->nrows; i++) {
      if (xg->is_missing[i][j]) {
        xg->raw_data[i][j] = xg->tform1[i][j] = xg->tform2[i][j] =
          imp_values[k][idx];
        k++;
        if (k == xg->nmissing)
          break;
      }
    }
  }

  update_imputation(xg);
}

/* ARGSUSED */
static XtCallbackProc
choose_imputation_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int j, imp_btn;

  for (j=0; j<nimputations; j++) {
    if (imputation_menu_btn[j] == w) {
      imp_btn = j;
      break;
    }
  }

  imputation_indx = imp_btn;

  /* Reset the menu */
  reset_menu_mark();

  /* Reset the menu */
  reset_imputation_label(imputation_indx);

  /* Reset the data */
  reset_imputation(xg);
}

/* ARGSUSED */
static XtEventHandler
imp_scroll_cback(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2) {
    if (xbutton->button == 1) {
      imputation_indx++;
      if (imputation_indx > nimputations-1)
        imputation_indx = 0;
    }
    else if (xbutton->button == 2) {
      imputation_indx--;
      if (imputation_indx < 0)
        imputation_indx = nimputations-1;
    }
    reset_menu_mark();
    reset_imputation_label(imputation_indx);
    reset_imputation(xg);
  }
}

/* ARGSUSED */
static XtCallbackProc
close_imp_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtPopdown(ipopup);
}

void
make_imputation_menu(xgobidata *xg)
{
  if (nimputations > 0) {
    int j;

    imputation_menu = XtVaCreatePopupShell("Menu",
      simpleMenuWidgetClass, imputation_menu_cmd,
      NULL);

    imputation_menu_btn = (Widget *) XtMalloc(
      (Cardinal) nimputations * sizeof(Widget));

    for (j=0; j<nimputations; j++) {
      imputation_menu_btn[j] = XtVaCreateWidget("Command",
        smeBSBObjectClass, imputation_menu,
        XtNlabel, (String) imp_names[j],
        XtNleftMargin, (Dimension) 24,
        NULL);
    }
    XtManageChildren(imputation_menu_btn, (Cardinal) nimputations);

    for (j=0; j<nimputations; j++) {
      XtAddCallback(imputation_menu_btn[j], XtNcallback,
        (XtCallbackProc) choose_imputation_cback, (XtPointer) xg);
    }
  }
}

static void
set_varspec_menu_marks(void)
{
  int k;
  for (k=0; k<NVSPECBTNS; k++)
    XtVaSetValues(varspec_menu_btn[k],
      XtNleftBitmap, (Pixmap) None,
      NULL);

  XtVaSetValues(varspec_menu_btn[varspec_type],
    XtNleftBitmap, (Pixmap) menu_mark,
    NULL);
}

/* ARGSUSED */
static XtCallbackProc
set_varspec_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<NVSPECBTNS; btn++)
    if (varspec_menu_btn[btn] == w)
      break;

  varspec_type = btn;
  set_varspec_menu_marks();
}

void
make_varspec_menu(xgobidata *xg, Widget parent, Widget vref)
/*
 * Build a menu to contain options for variable specification.
*/
{
  int j;

  static char *varspec_names[] = {
    "All variables",
    "Currently selected variables (and vgroups)"
  };

  varspec_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Variables to impute",
    XtNmenuName, (String) "Menu",
    XtNfromVert, vref,
    NULL);
  if (mono) set_mono(varspec_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    varspec_menu_cmd, "MissingData");

  varspec_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, varspec_menu_cmd,
    NULL);
  if (mono) set_mono(varspec_menu);

  for (j=0; j<NVSPECBTNS; j++)
  {
    varspec_menu_btn[j] = XtVaCreateWidget("Command",
      smeBSBObjectClass, varspec_menu,
      XtNleftMargin, (Dimension) 24,
      XtNlabel, (String) varspec_names[j],
      NULL);
    if (mono) set_mono(varspec_menu_btn[j]);
    XtAddCallback(varspec_menu_btn[j], XtNcallback,
      (XtCallbackProc) set_varspec_cback, (XtPointer) xg);
  }
  XtManageChildren(varspec_menu_btn, NVSPECBTNS);
}

/* ARGSUSED */
XtCallbackProc
open_imputation_popup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  static Boolean initd = False;
  Boolean sens;
  Widget mframe, mform;
  Widget close_cmd, impute_cmd, use_groups_cmd;
  Widget assign_label, pct_above_cmd, pct_below_cmd, specify_cmd;
  Widget imp_arrow_cmd;
  char str[512];
  int impname_len, max_impname_len;
  char *longest_impname = (char *) NULL;

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
     * Create the imputation popup 
    */
    ipopup = XtVaCreatePopupShell("MissingData",
      /*transientShellWidgetClass, XtParent(w),*/
      topLevelShellWidgetClass, XtParent(w),
      XtNinput,            (Boolean) True,
      XtNallowShellResize, (Boolean) True,
      XtNtitle,            (String) "Imputation",
      XtNiconName,         (String) "Imputation",
      XtNx,                x,
      XtNy,                y,
      NULL);
    if (mono) set_mono(ipopup);

    /*
     * Create a paned widget so the 'Click here ...'
     * can be all across the bottom.
    */
    mframe = XtVaCreateManagedWidget("Form",
      panedWidgetClass, ipopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      NULL);
    /*
     * Create the form widget.
    */
    mform = XtVaCreateManagedWidget("MissingData",
      formWidgetClass, mframe,
      NULL);
    if (mono) set_mono(mform);

/*******************************************************************/
/*  Active if this is <not> the missing values xgobi but missing   */
/*                values are present                               */
/*******************************************************************/

    sens = !xg->is_missing_values_xgobi && xg->missing_values_present;

    rescale_cmd = (Widget) CreateToggle(xg, "Rescale when imputing",
      True, (Widget) NULL, (Widget) NULL, (Widget) NULL,
      rescale, ANY_OF_MANY, mform, "MissingData");
    XtManageChild(rescale_cmd);
    XtAddCallback(rescale_cmd, XtNcallback,
      (XtCallbackProc) rescale_cback, (XtPointer) xg);

    make_varspec_menu(xg, mform, rescale_cmd);
    set_varspec_menu_marks();

    impute_cmd = (Widget) CreateCommand(xg, "Perform random imputation",
      sens, (Widget) NULL, (Widget) varspec_menu_cmd,
      mform, "MissingData");
    XtManageChild(impute_cmd);
    XtAddCallback(impute_cmd, XtNcallback,
      (XtCallbackProc) impute_cback, (XtPointer) xg);

    use_groups_cmd = CreateToggle(xg, "Conditional on 'group' var",
      sens, (Widget) NULL, (Widget) impute_cmd, (Widget) NULL, use_brush_groups,
      ANY_OF_MANY, mform, "MissingData");
    XtVaSetValues(use_groups_cmd,
      XtNvertDistance, 0,
      XtNhorizDistance, 10,
      NULL);
    XtManageChild(use_groups_cmd);
    XtAddCallback(use_groups_cmd, XtNcallback,
      (XtCallbackProc) use_groups_cback, (XtPointer) xg);

    assign_label = XtVaCreateManagedWidget("Label",
      labelWidgetClass, mform,
      XtNfromVert, use_groups_cmd,
      XtNlabel, "Assign fixed value:",
      NULL);

    sprintf(str, "-999");
    width = XTextWidth(appdata.font, str, strlen(str)) +
      2*ASCII_TEXT_BORDER_WIDTH;
    pct_above_txt = XtVaCreateManagedWidget("MissingText",
      asciiTextWidgetClass, mform,
      XtNeditType, (int) XawtextEdit,
      XtNstring, (String) "10",
      XtNdisplayCaret, (Boolean) True,
      XtNwidth, width,
      XtNfromVert, (Widget) assign_label,
      XtNvertDistance, (Dimension) 0 ,
      XtNhorizDistance, (Dimension) 10 ,
      NULL);
    if (mono) set_mono(pct_above_txt);

    pct_above_cmd = (Widget) CreateCommand(xg, "% above max",
      sens, (Widget) pct_above_txt, (Widget) assign_label,
      /*(Widget) NULL, True, ONE_OF_MANY,*/
      mform, "MissingData");
    XtVaSetValues(pct_above_cmd,
      XtNvertDistance, 0, 
      XtNhorizDistance, 0, 
      NULL);
    XtManageChild(pct_above_cmd);
    XtAddCallback(pct_above_cmd, XtNcallback,
      (XtCallbackProc) pct_above_cback, (XtPointer) xg);

    sprintf(str, "-999");
    width = XTextWidth(appdata.font, str, strlen(str)) +
      2*ASCII_TEXT_BORDER_WIDTH;
    pct_below_txt = XtVaCreateManagedWidget("MissingText",
      asciiTextWidgetClass, mform,
      XtNeditType, (int) XawtextEdit,
      XtNstring, (String) "10",
      XtNdisplayCaret, (Boolean) True,
      XtNwidth, width,
      XtNfromVert, (Widget) pct_above_cmd,
      XtNvertDistance, (Dimension) 0 ,
      XtNhorizDistance, (Dimension) 10 ,
      NULL);
    if (mono) set_mono(pct_below_txt);

    pct_below_cmd = (Widget) CreateCommand(xg, "% below min",
      sens, (Widget) pct_below_txt, (Widget) pct_above_cmd,
      mform, "MissingData");
    XtVaSetValues(pct_below_cmd,
      XtNvertDistance, 0, 
      XtNhorizDistance, 0, 
      NULL);
    XtManageChild(pct_below_cmd);
    XtAddCallback(pct_below_cmd, XtNcallback,
      (XtCallbackProc) pct_below_cback, (XtPointer) xg);

    specify_cmd = (Widget) CreateCommand(xg, "Specify:", sens,
      (Widget) NULL, (Widget) pct_below_cmd,
      mform, "MissingData");
    XtVaSetValues(specify_cmd,
      XtNvertDistance, 0, 
      XtNhorizDistance, 10, 
      NULL);
    XtManageChild(specify_cmd);
    XtAddCallback(specify_cmd, XtNcallback,
      (XtCallbackProc) specify_cback, (XtPointer) xg);

    sprintf(str, "-99999");
    width = XTextWidth(appdata.font, str, strlen(str)) +
      2*ASCII_TEXT_BORDER_WIDTH;
    specify_txt = XtVaCreateManagedWidget("MissingText",
      asciiTextWidgetClass, mform,
      XtNeditType, (int) XawtextEdit,
      XtNstring, (String) "0",
      XtNdisplayCaret, (Boolean) True,
      XtNwidth, width,
      XtNfromVert, (Widget) pct_below_cmd,
      XtNfromHoriz, (Widget) specify_cmd,
      XtNvertDistance, (Dimension) 0 ,
      XtNhorizDistance, (Dimension) 0 ,
      NULL);
    if (mono) set_mono(specify_txt);

/*************************************************************************/

    if (sens) {
      int i;

      read_imputation_data(xg);
      if (nimputations > 0)
        read_imputation_names(xg);
      if (nimputations == 0)
        sens = False;
      else
        set_nmissing_indx(xg);

      imputation_menu_cmd = XtVaCreateManagedWidget("MenuButton",
        menuButtonWidgetClass, mform,
        XtNlabel, (String) "Imputations",
        XtNmenuName, (String) "Menu",
        XtNfromVert, specify_cmd,
        XtNsensitive, sens,
        NULL);
      if (mono) set_mono(imputation_menu_cmd);
      add_menupb_help(&xg->nhelpids.menupb,
        imputation_menu_cmd, "MissingData");

      if (sens) {

        make_imputation_menu(xg);
        /* find the length of the longest imputation name */

        longest_impname = (char *) NULL;
        max_impname_len = 0;
        for (i=0; i<nimputations; i++) {
          if ((impname_len = strlen(imp_names[i])) > max_impname_len) {
            max_impname_len = impname_len;
            longest_impname = imp_names[i];
          }
        }

        imp_arrow_cmd = XtVaCreateManagedWidget("Icon",
          labelWidgetClass, mform,
          XtNinternalHeight, (Dimension) 0,
          XtNinternalWidth, (Dimension) 0,
          XtNborderColor, (Pixel) appdata.fg,
          XtNbitmap, (Pixmap) lrarr,
          XtNfromVert, specify_cmd,
          XtNfromHoriz, imputation_menu_cmd,
          XtNsensitive, sens,
          NULL);
        add_pb_help(&xg->nhelpids.pb,
          imp_arrow_cmd, "MissingData");
        XtAddEventHandler(imp_arrow_cmd, ButtonPressMask, FALSE,
          (XtEventHandler) imp_scroll_cback, (XtPointer) xg);

        sprintf(str, "Selected: %s", longest_impname);
        width = XTextWidth(appdata.font, str,
          strlen(str)) + 2*ASCII_TEXT_BORDER_WIDTH;

        impname_label = XtVaCreateManagedWidget("MissingData",
          labelWidgetClass, mform,
          XtNfromVert, imputation_menu_cmd,
          XtNvertDistance, 0, 
          XtNhorizDistance, 10, 
          XtNwidth, width,
          XtNresize, False,
          XtNlabel, "Selected: ",
          XtNjustify, XtJustifyLeft,
          NULL);
        if (mono) set_mono(impname_label);
      }
    }


/*************************************************************************/

    close_cmd = XtVaCreateManagedWidget("Close",
      commandWidgetClass, mframe,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close_cmd);
    XtAddCallback(close_cmd, XtNcallback,
      (XtCallbackProc) close_imp_cback, (XtPointer) xg);
  }

  XtPopup(ipopup, (XtGrabKind) XtGrabNone);
  XRaiseWindow(display, XtWindow(ipopup));

  if (!initd)
  {
    set_wm_protocols(ipopup);
    initd = True;
  }
}

