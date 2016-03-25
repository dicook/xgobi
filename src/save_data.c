/* save_data.c */
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

/*
 * The first section is for the panel that is used to
 * make a copy of the current data; it creates an entire
 * set of files.
*/

static Widget npopup = (Widget) NULL;
static Widget epopup = (Widget) NULL;
static Widget newfname_txt;
static Widget save_lines_tgl;
/*Widget save_linecolors_tgl;*/
static Boolean sv_lines = False;

/* Used in writing out color vectors */
char *namesv;
int nameslen;
#define NAMESIZE 32
#define NAMESV(j) (namesv + j*NAMESIZE)

/* should this be 2 or 3? */
#define NDATABTNS 3
int data_ind;
#define RAWDATA    0
#define TFORMDATA  1
#define JITTERDATA 2
static Widget data_menu_cmd, data_menu_btn[NDATABTNS] ;
static char *data_menu_btn_label[] = {
  "Save raw data",
  "Save transformed data",
  "Save jittered data"
};


#define NFORMATBTNS 2
int format_ind;
#define ASCIIDATA  0
#define BINARYDATA 1
Widget format_menu_cmd, format_menu_btn[NFORMATBTNS] ;
static char *format_menu_btn_label[] = {
  "Save data as ascii",
  "Save data as binary"
};

#define NROWBTNS 4
int row_ind;
#define NEWFNAMEROW 0
#define ERASE       1
#define LABELLED    2
#define ALLROWS     3
Widget row_menu_cmd, row_menu_btn[NROWBTNS] ;
static char *row_menu_btn_label[] = {
  "Use the file newfname.rowindx",
  "Save currently displayed points only",
  "Save currently labelled points only",
  "Save all points"
};

#define NCOLBTNS 3
int col_ind;
#define NEWFNAMECOL  0
#define ALLCOLS      1
#define SELECTEDVARS 2
Widget col_menu_cmd, col_menu_btn[NCOLBTNS] ;
static char *col_menu_btn_label[] = {
  "Use the file newfname.colindx",
  "Save all columns",
  "Save columns selected in the Variable List"
};

#define NMISSINGBTNS 2
int missing_ind;
#define WRITENA   0
#define WRITEIMP  1
Widget missing_menu_cmd, missing_menu_btn[NMISSINGBTNS] ;
static char *missing_menu_btn_label[] = {
  "Write out missings as 'na'",
  "Write out currently assigned values"
};

/* ARGSUSED */
static XtCallbackProc
save_toggle_cback(Widget w, xgobidata *xg, XtPointer cback_data)
{
  Boolean state;
  XtVaGetValues(w, XtNstate, &state, NULL);
  setToggleBitmap(w, state);
}

/* ARGSUSED */
static XtCallbackProc
set_data_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;
  for (btn=0; btn<NDATABTNS; btn++)
    if (data_menu_btn[btn] == w)
      break;

  XtVaSetValues(data_menu_cmd,
    XtNlabel, data_menu_btn_label[btn],
    NULL);
  data_ind = btn;
}

/* ARGSUSED */
static XtCallbackProc
set_format_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;
  for (btn=0; btn<NFORMATBTNS; btn++)
    if (format_menu_btn[btn] == w)
      break;

  XtVaSetValues(format_menu_cmd,
    XtNlabel, format_menu_btn_label[btn],
    NULL);
  format_ind = btn;
}

/* ARGSUSED */
static XtCallbackProc
set_row_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k, btn;
  for (k=0; k<NROWBTNS; k++)
    if (row_menu_btn[k] == w) {
      btn = k;
      break;
    }
  XtVaSetValues(row_menu_cmd,
    XtNlabel, row_menu_btn_label[btn],
    NULL);
  row_ind = btn;
}

/* ARGSUSED */
static XtCallbackProc
set_col_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k, btn;
  for (k=0; k<NCOLBTNS; k++)
    if (col_menu_btn[k] == w) {
      btn = k;
      break;
    }
  XtVaSetValues(col_menu_cmd,
    XtNlabel, col_menu_btn_label[btn],
    NULL);
  col_ind = btn;
}

/* ARGSUSED */
static XtCallbackProc
set_missing_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k, btn;
  for (k=0; k<NMISSINGBTNS; k++)
    if (missing_menu_btn[k] == w) {
      btn = k;
      break;
    }
  XtVaSetValues(missing_menu_cmd,
    XtNlabel, missing_menu_btn_label[btn],
    NULL);
  missing_ind = btn;
}

/* ARGSUSED */
static XtCallbackProc
close_npopup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtPopdown(npopup);
}

static int
set_rowv(int *rowv, char *rootname, xgobidata *xg)
{
  int i, j, k;
  int nrows = 0;
  char fname[164];
  char message[256];
  FILE *fp;

  switch (row_ind) {

    case NEWFNAMEROW:
    /*
     * If the user is supplying a file named rootname.rowindx, read it
     * in and verify that it's reasonable: that is, contains no dups.
     * ... Actually, that would be painfully slow.  Just verify that the
     * values are legal.
    */

      sprintf(fname, "%s.rowindx", rootname);
      if ( (fp = fopen(fname, "r")) == NULL) {
        sprintf(message,
          "The file '%s' can not be read\n", fname);
        show_message(message, xg);
        return(0);
      } else {
        i = 0;
        while (fscanf(fp, "%d", &rowv[i]) != EOF) {
          if (rowv[i] <= 0 || rowv[i] > xg->nrows) {
            sprintf(message,
              "Your rowindx file contains an impermissible value, %d,\n",
              rowv[i]);
            strcat(message,
              "which is either <=0 or > nrows\n");
            show_message(message, xg);
            fclose(fp);
            return(0);
          }
          (rowv[i])--;
          i++;
        }
        nrows = i;
      }
      fclose(fp);
      break;

    case ERASE:
    /*
     * Otherwise just copy the row numbers representing unerased
     * points into rowv, and return their count.
    */

      for (i=0, j=0; i<xg->nrows_in_plot; i++) {
        k = xg->rows_in_plot[i];
        if (!xg->erased[k])
          rowv[j++] = k;
      }
      nrows = j;
      break;

    case LABELLED:
    /*
     * Otherwise just copy the row numbers representing sticky
     * labels into rowv, and return their count.
    */

      for (i=0; i<xg->nsticky_ids; i++) {
        rowv[i] = xg->sticky_ids[i];
      }
      nrows = xg->nsticky_ids;
      break;

    case ALLROWS:
    /* 
     * Finally, let rowv be (0,1,2,,,,xg->nrows)
    */
      for (i=0; i<xg->nrows; i++)
        rowv[i] = i;
      nrows = xg->nrows;
      break;

    default:
      fprintf(stderr, "error in row_ind; impossible type %d\n", row_ind);
      break;
  }

  return(nrows);
}

static int
set_colv(int *colv, char *rootname, xgobidata *xg)
{
  int i;
  int ncols = 0;
  char fname[164];
  char message[256];
  FILE *fp;

  switch (col_ind) {

    case NEWFNAMECOL:
    /*
     * If the user is supplying a file named rootname.colindx, read it
     * in and verify that it's reasonable: that is, contains no dups.
     * ... Actually, that would be painfully slow.  Just verify that the
     * values are legal.
    */

      sprintf(fname, "%s.colindx", rootname);
      if ( (fp = fopen(fname, "r")) == NULL) {
        sprintf(message,
          "The file '%s' can not be read\n", fname);
        show_message(message, xg);
        return(0);
      } else {
        i = 0;
        while (fscanf(fp, "%d", &colv[i]) != EOF) {
          if (colv[i] <= 0 || colv[i] > xg->ncols_used) {
            sprintf(message,
              "Your colindx file contains an impermissible value, %d,\n",
              colv[i]);
            strcat(message, "which is either <=0 or >ncols\n");
            show_message(message, xg);
            fclose(fp);
            return(0);
          }
          (colv[i])--;
          i++;
        }
        ncols = i;
      }
      fclose(fp);
      break;

    case ALLCOLS:
    /* 
     * let colv be (0,1,2,,,,xg->ncols_used)
    */
      for (i=0; i<xg->ncols_used; i++)
        colv[i] = i;
      ncols = xg->ncols_used;
      break;

    case SELECTEDVARS:
    /* 
     * let colv be xg->selectedvars
    */
      ncols = 0;
      for (i=0; i<xg->ncols_used; i++)
        if (xg->selectedvars[i])
          colv[ncols++] = i;
      break;
    

    default:
      fprintf(stderr, "error in col_ind; impossible type %d\n", col_ind);
      break;
  }
  
  return(ncols);
}

int
write_ascii_data(char *rootname, int *rowv, int nr, int *colv, int nc,
int data_ind, int missing_ind, xgobidata *xg)
{
  char fname[164];
  char message[256];
  FILE *fp;
  int i, j, ir, jc;
  float **fdatap;
  long **ldatap;

  if (data_ind == RAWDATA || data_ind == TFORMDATA)
    sprintf(fname, "%s.dat", rootname);
  else if (data_ind == JITTERDATA)
    sprintf(fname, "%s.jit", rootname);

  if ( (fp = fopen(fname, "w")) == NULL) {
    sprintf(message,
      "The file '%s' can not be created\n", fname);
    show_message(message, xg);
    return(0);
  } else {

    if (data_ind == RAWDATA)
      fdatap = xg->raw_data;
    else if (data_ind == TFORMDATA)
      fdatap = xg->tform2;
    else if (data_ind == JITTERDATA)
      ldatap = xg->jitter_data;

    for (i=0; i<nr; i++) {
      ir = rowv[i];
      for (j=0; j<nc; j++) {
        jc = colv[j];
        if (xg->missing_values_present && xg->is_missing[ir][jc] &&
            missing_ind == WRITENA)
        {
          fprintf(fp, "NA ");
        } 
        else {
          if (data_ind == RAWDATA || data_ind == TFORMDATA)
            fprintf(fp, "%g ", fdatap[ir][jc]);
          else if (data_ind == JITTERDATA)
            fprintf(fp, "%ld ", ldatap[ir][jc]);
        } 
      }
      fprintf(fp, "\n");
    }

    fclose(fp);
    return(1);
  }
}

void
strip_blanks(char *str)
{
  int src, dest;

  for (src=0, dest=0; src<(int)(strlen(str)+1); src++)
    if (str[src] != ' ')
      str[dest++] = str[src];
}

/* ARGSUSED */
XtCallbackProc
create_xgobi_file_set(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int nr, nc, nvgroups;
  int *rowv, *colv;
  char *rootname = (char *) NULL;
  char fname[164];
  char message[256];
  FILE *fp;
  int i, j, k;
  Boolean skipit;
  /* These are all defined in missing.c */
  extern int nimputations;
  extern char **imp_names;
  extern float **imp_values;

  /*
   * An inconsistency:  Can't save binary data and still
   * write out "na"
  */
  if (format_ind == BINARYDATA &&
      xg->missing_values_present && missing_ind == WRITENA) {

    sprintf(message,
      "Sorry, XGobi can't write 'NA' in binary format.");
    show_message(message, xg);
    return ((XtCallbackProc) 0);
  }

/* Step 1: get the rootname and verify that it's writable */
  if (rootname == (char *) NULL)
    rootname = (char *) XtMalloc(132 * sizeof(char));
  XtVaGetValues(newfname_txt, XtNstring, (String) &rootname, NULL);
  /* Having trouble with blanks ... */
  strip_blanks(rootname);
  if ( (fp = fopen(rootname, "w")) == NULL) {
    sprintf(message,
      "The file '%s' can not be opened for writing\n", rootname);
    show_message(message, xg);
    /*XtFree(rootname);*/
    return ((XtCallbackProc) 0);
  } else {
    unlink(rootname);
    fclose(fp);
  }

/* Save resources; they don't depend on nr or nc */
  if (!save_resources(rootname, xg)) {
    return ((XtCallbackProc) 0);
  }

/* Determine the rows to be saved */
  rowv = (int *) XtMalloc((Cardinal) xg->nrows * sizeof(int));
  nr = set_rowv(rowv, rootname, xg);
  if (nr == 0) {
    sprintf(message,
      "You have not successfully specified any rows; sorry");
    show_message(message, xg);
    /*XtFree(rootname);*/
    XtFree((char *) rowv);
    return ((XtCallbackProc) 0);
  }

/* Determine the columns to be saved */
  colv = (int *) XtMalloc((Cardinal) xg->ncols_used * sizeof(int));
  nc = set_colv(colv, rootname, xg);
  if (nc == 0) {
    sprintf(message,
      "You have not successfully specified any columns; sorry");
    show_message(message, xg);
    /*XtFree(rootname);*/
    XtFree((char *) rowv);
    XtFree((char *) colv);
    return ((XtCallbackProc) 0);
  }

  /*
   * Save .dat first:  ascii or binary, raw or tform, missings as
   * 'na' or as currently imputed values
  */
  if (format_ind == BINARYDATA) {  /* current missing values written out */
    if (write_binary_data(rootname, rowv, nr, colv, nc, data_ind, xg) == 0) {
      /*XtFree(rootname);*/
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    }
  } else {
    if (write_ascii_data(rootname, rowv, nr, colv, nc, data_ind,
        missing_ind, xg) == 0) {
      /*XtFree(rootname);*/
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    }
  }

/* Save column labels */
  if (!save_collabels(rootname, colv, nc, data_ind, xg)) {
    XtFree((char *) rowv);
    XtFree((char *) colv);
    return ((XtCallbackProc) 0);
  }

/* Save row labels */
  if (!save_rowlabels(rootname, rowv, nr, xg)) {
    XtFree((char *) rowv);
    XtFree((char *) colv);
    return ((XtCallbackProc) 0);
  }

/* Save colors */
  skipit = True;
  for (i=0; i<nr; i++) {
    if (xg->color_now[ rowv[i] ] != plotcolors.fg) {
      skipit = False;
      break;
    }
  }
  if (!skipit) {
    if (!brush_save_colors(rootname, rowv, nr, xg)) {
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    }
  }

/* Save glyphs */
  skipit = True;
  for (i=0; i<nr; i++) {
    if (xg->glyph_now[ rowv[i] ].type != xg->glyph_0.type ||
        xg->glyph_now[ rowv[i] ].size != xg->glyph_0.size)
    {
      skipit = False;
      break;
    }
  }
  if (!skipit) {
    if (!brush_save_glyphs(rootname, rowv, nr, xg)) {
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    }
  }

/* Save erase -- unless using erase vector to choose what to copy */
  if (row_ind != ERASE) {
    skipit = True;
    for (i=0; i<nr; i++) {
      if (xg->erased[ rowv[i] ] == 1) {
        skipit = False;
        break;
      }
    }
    if (!skipit) {
      if (!brush_save_erase(rootname, rowv, nr, xg)) {
        XtFree((char *) rowv);
        XtFree((char *) colv);
        return ((XtCallbackProc) 0);
      }
    }
  }

/* Save lines -- let the user specify this one; I can't guess */
  XtVaGetValues(save_lines_tgl, XtNstate, &sv_lines, NULL);
  if (sv_lines) {
    fprintf(stderr, ".. saving %s.lines ...\n", rootname);
    if (!save_lines(rootname, rowv, nr, xg)) {
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    }
  }

/* Save linecolors */
  skipit = True;
  for (k=0; k<xg->nlines; k++) {
    if (xg->line_color_now[k] != plotcolors.fg) {
      skipit = False;
      break;
    }
  }
  if (!skipit) {
    fprintf(stderr, ".. saving %s.linecolors ...\n", rootname);
    if (!save_line_colors(rootname, rowv, nr, xg)) {
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    }
  }


/* Save vgroups */
  nvgroups = numvargroups(xg);
  if (nvgroups != xg->ncols_used) {
    sprintf(fname, "%s.vgroups", rootname);
    if ( (fp = fopen(fname, "w")) == NULL) {
      sprintf(message,
        "The file '%s' can not be opened for writing\n", fname);
      show_message(message, xg);
      /*XtFree(rootname);*/
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    } else {
      for (j=0; j<nc; j++)
        fprintf(fp, "%d ", xg->vgroup_ids[colv[j]] + 1);
      fprintf(fp, "\n");
      fclose(fp);
    }
  }

/* Save rgroups */
  if (xg->nrgroups > 0) {
    sprintf(fname, "%s.rgroups", rootname);
    if ( (fp = fopen(fname, "w")) == NULL) {
      sprintf(message,
        "The file '%s' can not be opened for writing\n", fname);
      show_message(message, xg);
      /*XtFree(rootname);*/
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    } else {
      for (j=0; j<nr; j++)
        fprintf(fp, "%d ", xg->rgroup_ids[rowv[j]] + 1);
      fprintf(fp, "\n");
      fclose(fp);
    }
  }

/*
 * The way to do this is to cycle over all imputations,
 * populating raw_data (and tform2) with the imputed
 * values.  Grab and restore the current values ...
 *
 * All this elaborate process is only needed if nr != xg->nrows
 * or nc != xg->ncols_used.  (There aren't any missings in the
 * last column, so ncols_used, ncols; it's all the same.)
*/
  if (xg->missing_values_present && nimputations > 0) {
    int n;

    /* If saving imputed data, also save .missing */
    if (!save_missing(rootname, rowv, nr, colv, nc, xg)) {
      /*XtFree(rootname);*/
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    }
   
    /* Copy .imp_names .. */
    if (!copy_impnames(rootname, xg)) {
      XtFree((char *) rowv);
      XtFree((char *) colv);
      return ((XtCallbackProc) 0);
    }

    if (nr == xg->nrows && nc == xg->ncols_used) {

      /* Copy everything from .imp .. */
      if (!copy_imputations(rootname, xg)) {
        XtFree((char *) rowv);
        XtFree((char *) colv);
        return ((XtCallbackProc) 0);
      }

    } else {

      float *curmiss;
      float **impvtmp;
      int nmiss_copied;

      curmiss = (float *) XtMalloc((Cardinal) xg->nmissing * sizeof(float));
      /* Grab the current values to restore later */
      k = 0;
      for (i=0; i<xg->nrows; i++)
        for (j=0; j<xg->ncols_used; j++)
          if (xg->is_missing[i][j])
            curmiss[k++] = xg->raw_data[i][j];

      /* Allocate a temporary array to hold all the imputations needed */
      impvtmp = (float **) XtMalloc((Cardinal) xg->nmissing * sizeof(float *));
      for (n=0; n<xg->nmissing; n++)
        impvtmp[n] = (float *)
          XtMalloc((Cardinal) nimputations * sizeof(float));

      /* Store all imputed values to be saved */
      for (n=0; n<nimputations; n++) {

        /* Read in a set of missing values */
        /* They're stored columnwise, so loop over rows inside cols */
        k = 0;
        for (j=0; j<xg->ncols_used; j++)
          for (i=0; i<xg->nrows; i++)
            if (xg->is_missing[i][j])
              xg->raw_data[i][j] = imp_values[k++][n];

        /* Write the appropriate ones into impvalues */
        nmiss_copied = 0;
        for (j=0; j<nc; j++)
          for (i=0; i<nr; i++)
            if (xg->is_missing[rowv[i]][colv[j]])
              impvtmp[nmiss_copied++][n] = xg->raw_data[rowv[i]][colv[j]];
      }
            
      /* Write out impvalues */
      sprintf(fname, "%s.imp", rootname);
      if ( (fp = fopen(fname, "w")) == NULL) {
        sprintf(message,
          "The file '%s' can not be opened for writing\n", fname);
        show_message(message, xg);
        /*XtFree(rootname);*/
        XtFree((char *) rowv);
        XtFree((char *) colv);
        return ((XtCallbackProc) 0);
      } else {
        for (k=0; k<nmiss_copied; k++) {
          for (n=0; n<nimputations; n++)
            fprintf(fp, "%f ", impvtmp[k][n]);
          fprintf(fp, "\n");
        }
        fclose(fp);
      }

      /* Restore current values */
      k = 0;
      for (i=0; i<xg->nrows; i++)
        for (j=0; j<xg->ncols_used; j++)
          if (xg->is_missing[i][j])
            xg->raw_data[i][j] = curmiss[k++];

      /* Free allocated arrays */
      XtFree((char *) rowv);
      for (i=0; i<xg->nmissing; i++)
        XtFree((char *) impvtmp[i]);
      XtFree((char *) impvtmp);
    }
  }

/*
 * Continue saving files: .doc?
 * Don't bother with .missing, .nlinkable
*/

  XtFree((char *) rowv);
  XtFree((char *) colv);
}

/* ARGSUSED */
XtCallbackProc
open_new_xgobi_popup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  static Boolean initd = False;
  Widget fname_lab;

  static char *data_menu_str = "Data stage:";
  Widget data_menu_box, data_menu_lab, data_menu;
  static char *format_menu_str = "Data format:";
  Widget format_menu_box, format_menu_lab, format_menu;
  static char *row_menu_str = "Row-wise data:";
  Widget row_menu_box, row_menu_lab, row_menu;
  static char *col_menu_str = "Column-wise data:";
  Widget col_menu_box, col_menu_lab, col_menu;
  static char *missing_menu_str = "Missing data:";
  Widget missing_menu_box, missing_menu_lab, missing_menu;
  Widget options_box;

  Dimension width, height;
  Position x, y;
  Widget oframe, panel, box;
  Widget close, doit;
  int k;
  char str[64];

  if (npopup == (Widget) NULL) {

    XtVaGetValues(w,
      XtNwidth, &width,
      XtNheight, &height, NULL);
    XtTranslateCoords(w,
      (Position) (width/2), (Position) (height/2), &x, &y);

    npopup = XtVaCreatePopupShell("CreateFileSet",
      /*transientShellWidgetClass, XtParent(w),*/
      topLevelShellWidgetClass, XtParent(w),
      XtNinput,            (Boolean) True,
      XtNallowShellResize, (Boolean) True,
      XtNtitle,            (String) "Create XGobi file set",
      XtNiconName,         (String) "CopyData",
      XtNx,                x,
      XtNy,                y,
      NULL);
    if (mono) set_mono(npopup);

    oframe = XtVaCreateManagedWidget("Form",
      panedWidgetClass, npopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      NULL);

    panel = XtVaCreateManagedWidget("Panel",
      formWidgetClass, oframe,
      /*boxWidgetClass, oframe,*/
      /*XtNorientation, (XtOrientation) XtorientVertical,*/
      NULL);
    if (mono) set_mono(panel);

    /* Label and text widget to capture the new file name */
    box = XtVaCreateManagedWidget("Panel",
      boxWidgetClass, panel,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNhSpace, 1,
      XtNvSpace, 1,
      NULL);
    if (mono) set_mono(box);
    fname_lab = (Widget) XtVaCreateManagedWidget("SaveXGobi",
      labelWidgetClass, box,
      XtNlabel, "New root file name: ",
      XtNresize, False,
      NULL);
    if (mono) set_mono(fname_lab);
    sprintf(str, "MMMMMMMMMMMMMMMMMM");
    width = XTextWidth(appdata.font, str, strlen(str)) +
      2*ASCII_TEXT_BORDER_WIDTH;
    newfname_txt = XtVaCreateManagedWidget("MissingText",
      asciiTextWidgetClass, box,
      XtNeditType, (int) XawtextEdit,
      XtNstring, (String) "   ",
      XtNwidth, width,
      XtNdisplayCaret, (Boolean) True,
      XtNresize, False,
      NULL);
    if (mono) set_mono(newfname_txt);

    data_ind = RAWDATA;
    build_labelled_menu(&data_menu_box, &data_menu_lab, data_menu_str,
      &data_menu_cmd, &data_menu, data_menu_btn,
      data_menu_btn_label, data_menu_btn_label,  /* no nicknames */
      NDATABTNS, data_ind, panel, box,
      XtorientHorizontal, appdata.font, "SaveXGobi", xg);
    for (k=0; k<NDATABTNS; k++)
      XtAddCallback(data_menu_btn[k],  XtNcallback,
        (XtCallbackProc) set_data_cback, (XtPointer) xg);

    format_ind = ASCIIDATA;
    build_labelled_menu(&format_menu_box, &format_menu_lab, format_menu_str,
      &format_menu_cmd, &format_menu, format_menu_btn,
      format_menu_btn_label, format_menu_btn_label, 
      NFORMATBTNS, format_ind, panel, data_menu_box,
      XtorientHorizontal, appdata.font, "SaveXGobi", xg);
    for (k=0; k<NFORMATBTNS; k++)
      XtAddCallback(format_menu_btn[k],  XtNcallback,
        (XtCallbackProc) set_format_cback, (XtPointer) xg);

    row_ind = ALLROWS;
    build_labelled_menu(&row_menu_box, &row_menu_lab, row_menu_str,
      &row_menu_cmd, &row_menu, row_menu_btn,
      row_menu_btn_label, row_menu_btn_label,
      NROWBTNS, row_ind, panel, format_menu_box,
      XtorientHorizontal, appdata.font, "SaveXGobi", xg);
    for (k=0; k<NROWBTNS; k++)
      XtAddCallback(row_menu_btn[k],  XtNcallback,
        (XtCallbackProc) set_row_cback, (XtPointer) xg);

    col_ind = ALLCOLS;
    build_labelled_menu(&col_menu_box, &col_menu_lab, col_menu_str,
      &col_menu_cmd, &col_menu, col_menu_btn,
      col_menu_btn_label, col_menu_btn_label,
      NCOLBTNS, col_ind, panel, row_menu_box, 
      XtorientHorizontal, appdata.font,"SaveXGobi", xg);
    for (k=0; k<NCOLBTNS; k++)
      XtAddCallback(col_menu_btn[k],  XtNcallback,
        (XtCallbackProc) set_col_cback, (XtPointer) xg);

    missing_ind = WRITENA;
    build_labelled_menu(&missing_menu_box, &missing_menu_lab, missing_menu_str,
      &missing_menu_cmd, &missing_menu, missing_menu_btn,
      missing_menu_btn_label, missing_menu_btn_label,
      NMISSINGBTNS, missing_ind, panel, col_menu_box, 
      XtorientHorizontal, appdata.font,"SaveXGobi", xg);
    for (k=0; k<NMISSINGBTNS; k++)
      XtAddCallback(missing_menu_btn[k],  XtNcallback,
        (XtCallbackProc) set_missing_cback, (XtPointer) xg);

    options_box = XtVaCreateManagedWidget("Panel",
      boxWidgetClass, panel,
      XtNfromVert, missing_menu_box,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNhSpace, 1,
      XtNvSpace, 1,
      NULL);
    save_lines_tgl = CreateToggle(xg, ".lines file", True,
      (Widget) NULL, (Widget) NULL, (Widget) NULL, False,
      ANY_OF_MANY, options_box, "SaveXGobi");
    XtManageChild(save_lines_tgl);
    XtAddCallback(save_lines_tgl,  XtNcallback,
      (XtCallbackProc) save_toggle_cback, (XtPointer) xg);

/*
    save_linecolors_tgl = CreateToggle(xg, ".linecolors file", True,
      (Widget) NULL, (Widget) NULL, (Widget) NULL, False,
      ANY_OF_MANY, options_box, "SaveXGobi");
    XtManageChild(save_linecolors_tgl);
*/

    doit = XtVaCreateManagedWidget("SaveXGobi",
      commandWidgetClass, panel,
      XtNfromVert, options_box,
      XtNlabel, (String) "Save files",
      NULL);
    if (mono) set_mono(doit);
    XtAddCallback(doit, XtNcallback,
      (XtCallbackProc) create_xgobi_file_set, (XtPointer) xg);

    close = XtVaCreateManagedWidget("SaveXGobi",
      commandWidgetClass, oframe,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close);
    XtAddCallback(close, XtNcallback,
      (XtCallbackProc) close_npopup_cback, (XtPointer) NULL);
  }

  XtPopup(npopup, (XtGrabKind) XtGrabNone);
  XRaiseWindow(display, XtWindow(npopup));

  if (!initd)
  {
    set_wm_protocols(npopup);
    initd = True;
  }

}

/********************************************************************/
/* End of first section */
/********************************************************************/
/* Beginning of second section */
/********************************************************************/

/* 
 * The second section is for the panel that is used to
 * save selected files.  The default is to use the
 * current root file name, but I guess I'll let the user
 * override that -- Suppose they want to create a set of
 * indicator vector files somehow ...
 *
 * So what can be saved?
 *   fname.bin
 *   fname.glyphs
 *   fname.colors
 *   fname.erase
 *   fname.lines
 *   fname.linecolors
 *   fname.resources
*/

Widget save_choice[8];
#define BINARY_TGL    save_choice[0]
#define PTCOLOR_TGL   save_choice[1]
#define PTGLYPH_TGL   save_choice[2]
#define PTERASE_TGL   save_choice[3]
#define LINE_TGL      save_choice[4]
#define LINECOLOR_TGL save_choice[5]
#define RESOURCE_TGL  save_choice[6]
#define JITTER_TGL    save_choice[7]

/* ARGSUSED */
static XtCallbackProc
close_epopup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtPopdown(epopup);
}

/* ARGSUSED */
XtCallbackProc
write_selected_xgobi_files(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int j;
  Boolean set;
  char *rootname = (char *) NULL;

  int nr = (int) xg->nrows_in_plot;
  int *rowv = (int *) xg->rows_in_plot;
  int nc = xg->ncols_used;
  int *colv;
  colv = (int *) XtMalloc((Cardinal) nc * sizeof(int));
  for (j=0; j<nc; j++)
    colv[j] = j;

/* Step 1: get the rootname */
  if (rootname == (char *) NULL)
    rootname = (char *) XtMalloc(132 * sizeof(char));
  XtVaGetValues(newfname_txt, XtNstring, (String) &rootname, NULL);
  /* Having trouble with blanks ... */
  strip_blanks(rootname);

  XtVaGetValues(BINARY_TGL, XtNstate, &set, NULL);
  if (set)
    if (write_binary_data(rootname, rowv, nr, colv, nc, TFORMDATA, xg) == 0)
      return ((XtCallbackProc) 0);

  XtVaGetValues(PTCOLOR_TGL, XtNstate, &set, NULL);
  if (set)
    if (brush_save_colors(rootname, rowv, nr, xg) == 0)
      return ((XtCallbackProc) 0);
  XtVaGetValues(PTGLYPH_TGL, XtNstate, &set, NULL);
  if (set)
    if (brush_save_glyphs(rootname, rowv, nr, xg) == 0)
      return ((XtCallbackProc) 0);
  XtVaGetValues(PTERASE_TGL, XtNstate, &set, NULL);
  if (set)
    if (brush_save_erase(rootname, rowv, nr, xg) == 0)
      return ((XtCallbackProc) 0);

  XtVaGetValues(LINE_TGL, XtNstate, &set, NULL);
  if (set)
    if (save_lines(rootname, rowv, nr, xg) == 0)
      return ((XtCallbackProc) 0);
  XtVaGetValues(LINECOLOR_TGL, XtNstate, &set, NULL);
  if (set)
    if (save_line_colors(rootname, rowv, nr, xg) == 0)
      return ((XtCallbackProc) 0);

  XtVaGetValues(RESOURCE_TGL, XtNstate, &set, NULL);
  if (set)
    if (save_resources(rootname, xg) == 0)
      return ((XtCallbackProc) 0);

  XtVaGetValues(JITTER_TGL, XtNstate, &set, NULL);
  if (set)
    if (write_ascii_data(rootname, rowv, nr, colv, nc, JITTERDATA,
      WRITEIMP, xg) == 0)
        return ((XtCallbackProc) 0);
}

/* ARGSUSED */
XtCallbackProc
open_extend_xgobi_popup_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  static Boolean initd = False;
  Widget fname_lab;
  Dimension width, height;
  Position x, y;
  Widget oframe, panel, choice_panel, box;
  Widget close, doit;
  char str[64], rootfname[512];

  if (epopup == (Widget) NULL) {

    XtVaGetValues(w,
      XtNwidth, &width,
      XtNheight, &height, NULL);
    XtTranslateCoords(w,
      (Position) (width/2), (Position) (height/2), &x, &y);

    epopup = XtVaCreatePopupShell("ExtendFileSet",
      /*transientShellWidgetClass, XtParent(w),*/
      topLevelShellWidgetClass, XtParent(w),
      XtNinput,            (Boolean) True,
      XtNallowShellResize, (Boolean) True,
      XtNtitle,            (String) "Extend XGobi file set",
      XtNiconName,         (String) "ExtendFileSet",
      XtNx,                x,
      XtNy,                y,
      NULL);
    if (mono) set_mono(epopup);

    oframe = XtVaCreateManagedWidget("Form",
      panedWidgetClass, epopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      NULL);

    panel = XtVaCreateManagedWidget("Panel",
      formWidgetClass, oframe,
      NULL);
    if (mono) set_mono(panel);

    /* Label and text widget to capture the new file name */
    box = XtVaCreateManagedWidget("Panel",
      boxWidgetClass, panel,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      XtNhSpace, 1,
      XtNvSpace, 1,
      NULL);
    if (mono) set_mono(box);
    fname_lab = (Widget) XtVaCreateManagedWidget("SaveXGobi",
      labelWidgetClass, box,
      XtNlabel, "Root file name: ",
      XtNresize, False,
      NULL);
    if (mono) set_mono(fname_lab);
    sprintf(str, "MMMMMMMMMMMMMMMMMM");
    width = XTextWidth(appdata.font, str, strlen(str)) +
      2*ASCII_TEXT_BORDER_WIDTH;

    /*
     * If xgobi was started from within S, we don't know the
     * names of any of the data files.  We only know the S
     * directory, which will be added later.
    */
    if (xg->data_mode == Sprocess)
      sprintf(rootfname, "");
    else
      sprintf(rootfname, xg->datafilename);
    newfname_txt = XtVaCreateManagedWidget("MissingText",
      asciiTextWidgetClass, box,
      XtNeditType, (int) XawtextEdit,
      XtNstring, (String) rootfname,
      XtNwidth, width,
      XtNdisplayCaret, (Boolean) True,
      XtNresize, False,
      NULL);
    if (mono) set_mono(newfname_txt);

    choice_panel = XtVaCreateManagedWidget("Panel",
      formWidgetClass, panel,
      XtNfromVert, (Widget) box,
      NULL);
    if (mono) set_mono(choice_panel);

    BINARY_TGL = CreateToggle(xg, ".bin file", True,
      (Widget) NULL, (Widget) NULL, (Widget) NULL, False,
      ANY_OF_MANY, choice_panel, "ExtendFileSet");
    if (xg->data_mode == Sprocess)
      XtSetSensitive(BINARY_TGL, False);
    XtAddCallback(BINARY_TGL,  XtNcallback,
      (XtCallbackProc) save_toggle_cback, (XtPointer) xg);

    PTCOLOR_TGL = CreateToggle(xg, ".colors file", True,
      (Widget) NULL, BINARY_TGL, (Widget) NULL, False,
      ANY_OF_MANY, choice_panel, "ExtendFileSet");
    XtAddCallback(PTCOLOR_TGL ,  XtNcallback,
      (XtCallbackProc) save_toggle_cback, (XtPointer) xg);

    PTGLYPH_TGL = CreateToggle(xg, ".glyphs file", True,
      (Widget) NULL, PTCOLOR_TGL, (Widget) NULL, False,
      ANY_OF_MANY, choice_panel, "ExtendFileSet");
    XtAddCallback(PTGLYPH_TGL ,  XtNcallback,
      (XtCallbackProc) save_toggle_cback, (XtPointer) xg);

    PTERASE_TGL = CreateToggle(xg, ".erase file", True,
      (Widget) NULL, PTGLYPH_TGL, (Widget) NULL, False,
      ANY_OF_MANY, choice_panel, "ExtendFileSet");
    XtAddCallback(PTERASE_TGL ,  XtNcallback,
      (XtCallbackProc) save_toggle_cback, (XtPointer) xg);

    LINE_TGL = CreateToggle(xg, ".lines file", True,
      (Widget) NULL, PTERASE_TGL, (Widget) NULL, False,
      ANY_OF_MANY, choice_panel, "ExtendFileSet");
    XtAddCallback(LINE_TGL ,  XtNcallback,
      (XtCallbackProc) save_toggle_cback, (XtPointer) xg);

    LINECOLOR_TGL = CreateToggle(xg, ".linecolors file", True,
      (Widget) NULL, LINE_TGL, (Widget) NULL, False,
      ANY_OF_MANY, choice_panel, "ExtendFileSet");
    XtAddCallback(LINECOLOR_TGL ,  XtNcallback,
      (XtCallbackProc) save_toggle_cback, (XtPointer) xg);

    RESOURCE_TGL = CreateToggle(xg, ".resources file", True,
      (Widget) NULL, LINECOLOR_TGL, (Widget) NULL, False,
      ANY_OF_MANY, choice_panel, "ExtendFileSet");
    XtAddCallback(RESOURCE_TGL ,  XtNcallback,
      (XtCallbackProc) save_toggle_cback, (XtPointer) xg);

    JITTER_TGL = CreateToggle(xg, ".jit file (ascii)", True,
      (Widget) NULL, RESOURCE_TGL, (Widget) NULL, False,
      ANY_OF_MANY, choice_panel, "ExtendFileSet");
    XtAddCallback(JITTER_TGL ,  XtNcallback,
      (XtCallbackProc) save_toggle_cback, (XtPointer) xg);

    XtManageChildren(save_choice, 8);

    doit = XtVaCreateManagedWidget("ExtendFileSet",
      commandWidgetClass, panel,
      XtNlabel, (String) "Save files",
      XtNfromVert, (Widget) choice_panel,
      NULL);
    if (mono) set_mono(doit);
    XtAddCallback(doit, XtNcallback,
      (XtCallbackProc) write_selected_xgobi_files, (XtPointer) xg);

    close = XtVaCreateManagedWidget("ExtendFileSet",
      commandWidgetClass, oframe,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close);
    XtAddCallback(close, XtNcallback,
      (XtCallbackProc) close_epopup_cback, (XtPointer) NULL);
  }

  XtPopup(epopup, (XtGrabKind) XtGrabNone);
  XRaiseWindow(display, XtWindow(epopup));

  if (!initd)
  {
    set_wm_protocols(epopup);
    initd = True;
  }

}

/********************************************************************/
/* End of second section */
/********************************************************************/
/* Beginning of third section */
/********************************************************************/

/*
 * Third section: routines that will be used by both the
 * first and the second sections to save individual files.
*/

int
write_binary_data(char *rootname, int *rowv, int nr, int *colv, int nc,
int data_ind, xgobidata *xg)
{
  char *fname;
  char fnameplus[164];
  char message[256];
  FILE *fp;
  int i, j, ir, jc;
  float xfoo;
  float **datap;

  if (rowv == (int *) NULL) {
    rowv = (int *) XtMalloc((Cardinal) nr * sizeof(int));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  if (xg->data_mode == Sprocess)
    return(1);

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);
  sprintf(fnameplus, "%s.bin", fname);

  if ( (fp = fopen(fnameplus, "wb")) == NULL) {
    sprintf(message,
      "The file '%s' can not be created\n", fnameplus);
    show_message(message, xg);
    return(0);
  } else {

    /* First the number of rows and columns */
    fwrite((char *) &nr, sizeof(nr), 1, fp);
    fwrite((char *) &nc, sizeof(nc), 1, fp);

    if (data_ind == TFORMDATA)
      datap = xg->tform2;
    else
      datap = xg->raw_data;

    for (i=0; i<nr; i++) {
      ir = rowv[i];
      for (j=0; j<nc; j++)
      {
        if (colv == (int *) NULL)  /* Write all columns, in default order */
          jc = j;
        else
          jc = colv[j];  /* Write the columns as specified */
        if (xg->missing_values_present && xg->is_missing[i][j])
          xfoo = FLT_MAX;
        else
          xfoo = datap[ir][jc];
        fwrite((char *) &xfoo, sizeof(xfoo), 1, fp);
      }
    }

    fclose(fp);
    return(1);
  }
}

int
save_collabels(char *rootname, int *colv, int nc, int data_ind, xgobidata *xg)
{
  int j;
  FILE *fp;
  char fnameplus[164];
  char message[MSGLENGTH];
  char *fname;

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);

  sprintf(fnameplus, "%s.col", fname);
  XtFree(fname);

  if (xg->data_mode == Sprocess)
    return(0);
  else {
    fp = fopen(fnameplus, "w");
    if (fp == NULL) {
      sprintf(message, "Failed to open %s for writing.\n", fnameplus);
      show_message(message, xg);
      return(0);
    }
    else {
      if (nc == (int) NULL) {
        if (data_ind == RAWDATA) {
          for (j=0; j<xg->ncols_used; j++)
            (void) fprintf(fp, "%s\n",  xg->collab[j]);
        } else {
          for (j=0; j<xg->ncols_used; j++)
            (void) fprintf(fp, "%s\n",  xg->collab_tform2[j]);
        }
      } else {
        if (data_ind == RAWDATA) {
          for (j=0; j<nc; j++)
            (void) fprintf(fp, "%s\n",  xg->collab[ colv[j] ]);
        } else {
          for (j=0; j<nc; j++)
            (void) fprintf(fp, "%s\n",  xg->collab_tform2[ colv[j] ]);
        }
      }
    }

    fclose(fp);
  }
  return(1);
}

int
save_rowlabels(char *rootname, int *rowv, int nr, xgobidata *xg)
{
  int i;
  FILE *fp;
  char fnameplus[164];
  char message[MSGLENGTH];
  char *fname;

  if (rowv == (int *) NULL) {
    rowv = (int *) XtMalloc((Cardinal) nr * sizeof(int));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);

  sprintf(fnameplus, "%s.row", fname);
  XtFree(fname);

  if (xg->data_mode == Sprocess)
    return(0);
  else {
    fp = fopen(fnameplus, "w");
    if (fp == NULL) {
      sprintf(message, "Failed to open %s for writing.\n", fnameplus);
      show_message(message, xg);
      return(0);
    }
    else
    {
      for (i=0; i<nr; i++)
        (void) fprintf(fp, "%s\n",  xg->rowlab[ rowv[i] ]);
    }

    fclose(fp);
  }
  return(1);
}

int
brush_save_colors(char *rootname, int *rowv, int nr, xgobidata *xg)
{
  char *fname;
  char fnameplus[164], *cid;
  int i, ir, k;
  long foo;
  FILE *fp;
  char message[MSGLENGTH];
  Boolean color_found;

  if (rowv == (int *) NULL) {
    rowv = (int *) XtMalloc((Cardinal) nr * sizeof(int));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);

  if (xg->data_mode == Sprocess)
  {
    /*
     * Now, the color ids.
    */
    if (!mono)
    {
      (void) strcpy(fnameplus, Spath0);
      (void) strcat(fnameplus, fname);
      (void) strcat(fnameplus, ".colors");
      if ( (fp = fopen(fnameplus, "w")) == NULL)
      {
        sprintf(message,
          "The file '%s' can't be opened for writing\n", fnameplus);
        show_message(message, xg);
        XtFree(fname);
        return(0);
      }
      else
      {
        nameslen = 0;
        namesv = (char *) XtMalloc(
          (Cardinal) nr * NAMESIZE * sizeof(char));
        /* "1" indicates an S data structure of one element. */
        (void) fprintf(fp, "%cS data%c", (char) 0 , (char) 1);
        /* "5" indicates type character. */
        foo = (long) 5;
        if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
          (void) fprintf(stderr, "error 1 in writing color vector\n");
        /* "nrows" indicates the number of elements. */
        foo = (long) nr;
        if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
          (void) fprintf(stderr, "error 2 in writing color vector\n");

        for (i=0; i<nr; i++) {
          color_found = False;
          ir = rowv[i];
          for (k=0; k<ncolors; k++) {
            if (color_nums[k] == xg->color_now[ir]) {
              (void) strcpy(NAMESV(ir), color_names[k]);
              color_found = True;
              break;
            }
          }
          if (!color_found) {  /*-- color not found, use Default --*/
            (void) strcpy(NAMESV(ir), "Default");
          }
          nameslen = nameslen + strlen(NAMESV(ir)) + 1;
        }

        foo = (long) nameslen;
        if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
          (void) fprintf(stderr, "error 3 in writing color vector\n");

        for (i=0; i<nr; i++) {
          ir = rowv[i];
          (void) fprintf(fp, "%s%c",  NAMESV(ir), (char) 0);
        }

        if (fclose(fp) == EOF)
          (void) fprintf(stderr, "error 4 in writing color vector\n");
        XtFree((XtPointer) namesv);
      }
    }
  }
  else if (xg->data_mode == ascii || xg->data_mode == binary) /* if not S */
  {
    if (!mono)
    {
      (void) strcpy(fnameplus, fname);
      (void) strcat(fnameplus, ".colors");
      if ( (fp = fopen(fnameplus, "w")) == NULL)
      {
        sprintf(message,
          "The file '%s' can't be opened for writing\n", fnameplus);
        show_message(message, xg);
        XtFree(fname);
        return(0);
      }
      else
      {
        for (i=0; i<nr; i++) {
          ir = rowv[i];
          for (k=0; k<ncolors; k++) {
            if (xg->color_now[ir] == color_nums[k]) {
              cid = color_names[k];
              break;
            }
          }
          (void) fprintf(fp, "%s\n", cid);
        }
      }
      if (fclose(fp) == EOF)
        (void) fprintf(stderr, "error in writing color vector\n");
    }
  }
  XtFree(fname);
  return(1);
}

int
brush_save_glyphs(char *rootname, int *rowv, int nr, xgobidata *xg)
{
  char *fname;
  char fnameplus[164];
  int i, gid;
  long foo;
  FILE *fp;
  char message[MSGLENGTH];

  if (rowv == (int *) NULL) {
    rowv = (int *) XtMalloc((Cardinal) nr * sizeof(int));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);

  if (xg->data_mode == Sprocess)
  {
/*
 * Second, the glyph ids.
*/
    (void) strcpy(fnameplus, Spath0);
    (void) strcat(fnameplus, fname);
    (void) strcat(fnameplus, ".glyphs");
    if ( (fp = fopen(fnameplus, "w")) == NULL)
    {
      sprintf(message,
        "The file '%s' can't be opened for writing\n", fnameplus);
      show_message(message, xg);
      XtFree(fname);
      return(0);
    }
    else
    {
      /* "1" indicates an S data structure of one element. */
      (void) fprintf(fp, "%cS data%c", (char) 0 , (char) 1);
      /* "2" indicates type integer. */
      foo = (long) 2;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        (void) fprintf(stderr, "error 1 in writing glyph vector\n");
      /* "nrows" indicates the number of elements. */
      foo = (long) nr;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        (void) fprintf(stderr, "error 2 in writing glyph vector\n");

      for (i=0; i<nr; i++)
      {
        foo = (long) find_gid(&xg->glyph_now[ rowv[i] ]);
        if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
          (void) fprintf(stderr, "error 3 in writing glyph vector\n");

      }
      if (fclose(fp) == EOF)
        (void) fprintf(stderr, "error 4 in writing glyph vector\n");
    }
  }
  else if (xg->data_mode == ascii || xg->data_mode == binary) /* if not S */
  {
    /*
     * Next, the glyphs
    */
    (void) strcpy(fnameplus, fname);
    (void) strcat(fnameplus, ".glyphs");
    if ( (fp = fopen(fnameplus, "w")) == NULL)
    {
      sprintf(message,
        "The file '%s' can't be opened for writing\n", fnameplus);
      show_message(message, xg);
      XtFree(fname);
      return(0);
    }
    for (i=0; i<nr; i++)
    {
      gid = find_gid(&xg->glyph_now[ rowv[i] ]);
      (void) fprintf(fp, "%d\n", gid);
    }
    fclose(fp);
  }
  return(1);
}

int
brush_save_erase(char *rootname, int *rowv, int nr, xgobidata *xg)
{
  char *fname;
  char fnameplus[164];
  int i;
  long foo;
  FILE *fp;
  char message[MSGLENGTH];

  if (rowv == (int *) NULL) {
    rowv = (int *) XtMalloc((Cardinal) nr * sizeof(int));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);

  if (xg->data_mode == Sprocess)
  {
/*
 * First, the erase vector.
*/
    (void) strcpy(fnameplus, Spath0);
    (void) strcat(fnameplus, fname);
    (void) strcat(fnameplus, ".erase");

    if ( (fp = fopen(fnameplus, "w")) == NULL)
    {
      sprintf(message,
        "The file '%s' can't be opened for writing\n", fnameplus);
      show_message(message, xg);
      XtFree(fname);
      return(0);
    }
    else
    {
      /* "1" indicates an S data structure of one element. */
      (void) fprintf(fp, "%cS data%c", (char) 0 , (char) 1);
      /* "2" indicates type integer. */
      foo = (long) 2;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        (void) fprintf(stderr, "error 1 in writing erase vector\n");
      /* "nrows" indicates the number of elements. */
      foo = (long) nr;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "error 2 in writing erase vector\n");
      for (i=0; i<nr; i++)
      {
        foo = (long) xg->erased[ rowv[i] ] ;
        if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
          (void) fprintf(stderr, "error 3 in writing erase vector\n");

      }
      if (fclose(fp) == EOF)
        (void) fprintf(stderr, "error 4 in writing erase vector\n");
    }
  }
  else if (xg->data_mode == ascii || xg->data_mode == binary) /* if not S */
  {
  /*
   * For a monochrome display, just write out the glyph ids.
   * For a color display, write both glyph id and color name.
   * First, the erase vector.
  */
    (void) strcpy(fnameplus, fname);
    (void) strcat(fnameplus, ".erase");
    if ( (fp = fopen(fnameplus, "w")) == NULL)
    {
      sprintf(message,
        "The file '%s' can't be opened for writing\n", fnameplus);
      show_message(message, xg);
      XtFree(fname);
      return(0);
    }

    for (i=0; i<nr; i++)
      (void) fprintf(fp, "%ld\n", (long) xg->erased[ rowv[i] ]);

    fclose(fp);
  }
  return(1);
}

int
find_saved_links(connect_lines *tlinks, int *rowv, int nr,
Boolean find_colors, xgobidata *xg)
{
/*
 * For each end of the link, determine whether the
 * point is included or not.  This could be darn slow.
 * Rely on a < b.
*/
  int nl = 0;
  int i, k, n;
  int a, b, start_a, start_b;

  nameslen = 0;

  for (k=0; k<xg->nlines; k++) {
    start_a = start_b = -1;
    a = xg->connecting_lines[k].a - 1;
    b = xg->connecting_lines[k].b - 1;
    for (i=0; i<nr; i++) {
      if (rowv[i] == a) {
        start_a = i;
        break;
      }
    }
    if (start_a != -1) {
      for (i=start_a; i<nr; i++) {
        if (rowv[i] == b) {
          start_b = i;
          break;
        }
      }
    }
    if (start_a != -1 && start_b != -1) {  /* Both ends included */
      tlinks[nl].a = start_a + 1;
      tlinks[nl].b = start_b + 1;
      /*
       * If the namesv argument is present, then find the
       * color name as well.
      */
      if (find_colors) {
        for (n=0; n<ncolors; n++) {
          if (xg->line_color_now[k] == color_nums[n]) {
            strcpy(NAMESV(nl), color_names[n]);
            nameslen = nameslen + strlen(NAMESV(nl)) + 1;
            break;
          }
        }
      }
      nl++;
    }
  }
  return(nl);
}

int
save_lines(char *rootname, int *rowv, int nr, xgobidata *xg)
{
  char *fname;
  char fnameplus[164];
  int i, k;
  long foo;
  FILE *fp;
  char message[MSGLENGTH];
  int nl;
  connect_lines *tlinks;

  if (rowv == (int *) NULL) {
    rowv = (int *) XtMalloc((Cardinal) nr * sizeof(int));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);

  if (nr == xg->nrows) {
    nl = xg->nlines;
    tlinks = xg->connecting_lines;
  } else {
    /*
     *
     * Determine the number of links to be saved -- may as
     * well build a temporary links structure to use, actually.
    */
    tlinks = (connect_lines *)
      XtMalloc((Cardinal) xg->nlines * sizeof(connect_lines));
    nl = find_saved_links(tlinks, rowv, nr, False, xg);
  }

  if (xg->data_mode == Sprocess) {

    (void) strcpy(fnameplus, Spath0);
    (void) strcat(fnameplus, rootname);
    (void) strcat(fnameplus, ".lines");
    if ( (fp = fopen(fnameplus, "w")) == NULL) {
      char message[MSGLENGTH];
      sprintf(message,
        "Failed to open the file '%s' for writing.\n", fnameplus);
      show_message(message, xg);
    }
    else
    {

      /*
       * "1" indicates that the following is an
       * S data structure of one element.
      */
      (void) fprintf(fp, "%cS data%c", (char) 0 , (char) 1);
      /*
       * "2" indicates that the following is of type
       * integer; "4" would imply numeric, or double.
      */
      foo = (long) 2;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "xgobi: error in writing connecting lines.\n");

      /*
       * "2*nl" says the following has "2*nl" elements."
      */
      foo = (long) (2*nl) ;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "xgobi: error in writing connecting lines.\n");

      for (k=0; k<nl; k++)
      {
        foo = (long) tlinks[k].a ;
        if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
          fprintf(stderr, "xgobi: error in writing connecting lines.\n");
        foo = (long) tlinks[k].b ;
        if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
          fprintf(stderr, "xgobi: error in writing connecting lines.\n");
      }

      if (fclose(fp) == EOF)
        fprintf(stderr, "xgobi: error in writing connecting lines.\n");

    }
  }

  else {  /* If not S */

    sprintf(fnameplus, "%s.lines", fname);
    if ( (fp = fopen(fnameplus, "w")) == NULL) {
      sprintf(message,
        "The file '%s' can not be opened for writing\n", fnameplus);
      show_message(message, xg);
      XtFree (fname);
      return(0);
    } else {
      for (k=0; k<nl; k++)
        fprintf(fp, "%d %d\n", tlinks[k].a, tlinks[k].b);
    }
    fclose(fp);
  }

  if (nr != xg->nrows)
    XtFree((char *) tlinks);

  XtFree (fname);
  return(1);
}

int
save_line_colors(char *rootname, int *rowv, int nr, xgobidata *xg)
{
  char *fname;
  char fnameplus[164];
  int i, k;
  long foo;
  FILE *fp;
  char message[MSGLENGTH];
  int nl;
  connect_lines *tlinks;
  Boolean color_found;

  if (rowv == (int *) NULL) {
    rowv = (int *) XtMalloc((Cardinal) nr * sizeof(int));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);

  if (mono)
    return(1);

  namesv = (char *) XtMalloc((Cardinal) xg->nlines*NAMESIZE * sizeof(char));
  nameslen = 0;
  if (nr == xg->nrows) {
    nl = xg->nlines;
    tlinks = xg->connecting_lines;
    for (k=0; k<nl; k++) {
      color_found = False;
      for (i=0; i<ncolors; i++) {
        if (color_nums[i] == xg->line_color_now[k]) {
          (void) strcpy(NAMESV(k), color_names[i]);
          color_found = True;
          break;
        }
      }
      if (!color_found) {  /*-- color not found, use Default --*/
        (void) strcpy(NAMESV(k), "Default");
      }
      nameslen = nameslen + strlen(NAMESV(k)) + 1;
    }

  } else {
    /*
     * Determine the number of links to be saved -- may as
     * well build a temporary links structure to use, actually.
     * nameslen, a global variable, is supposed to be set in
     * find_saved_links()
    */
    tlinks = (connect_lines *)
      XtMalloc((Cardinal) xg->nlines * sizeof(connect_lines));
    nl = find_saved_links(tlinks, rowv, nr, True, xg);
  }

  if (xg->data_mode == Sprocess)
  {
    (void) strcpy(fnameplus, Spath0);
    (void) strcat(fnameplus, fname);
    (void) strcat(fnameplus, ".linecolors");
    if ( (fp = fopen(fnameplus, "w")) == NULL)
    {
      sprintf(message,
        "The file '%s' can't be opened for writing\n", fnameplus);
      show_message(message, xg);
      XtFree(fname);
      XtFree(namesv);
      if (nr != xg->nrows)
        XtFree((char *) tlinks);
      return(0);
    }
    else
    {
      /* "1" indicates an S data structure of one element. */
      (void) fprintf(fp, "%cS data%c", (char) 0 , (char) 1);
      /* "5" indicates type character. */
      foo = (long) 5;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        (void) fprintf(stderr, "error 1 in writing line color vector\n");

      /* "xg->nlines" indicates the number of elements. */
      foo = (long) nl;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        (void) fprintf(stderr, "error 2 in writing line color vector\n");

      foo = (long) nameslen;  /* is this it? */
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        (void) fprintf(stderr, "error 3 in writing line color vector\n");
      for (k=0; k<nl; k++)
        (void) fprintf(fp, "%s%c",  NAMESV(k), (char) 0);

      if (fclose(fp) == EOF)
        (void) fprintf(stderr, "error 4 in writing line color vector\n");

    }
  }

  else if (xg->data_mode == ascii || xg->data_mode == binary) /* if not S */
  {
    (void) strcpy(fnameplus, fname);
    (void) strcat(fnameplus, ".linecolors");
    if ( (fp = fopen(fnameplus, "w")) == NULL)
    {
      sprintf(message,
        "The file '%s' can't be opened for writing\n", fnameplus);
      show_message(message, xg);
      XtFree(fname);
      XtFree(namesv);
      if (nr != xg->nrows)
        XtFree((char *) tlinks);
      return(0);
    }
    else {
      for (k=0; k<nl; k++)
        (void) fprintf(fp, "%s\n",  NAMESV(k));
    }

    if (fclose(fp) == EOF)
      (void) fprintf(stderr, "error in writing line color vector\n");
  }

  XtFree(namesv);
  XtFree(fname);
  if (nr != xg->nrows)
    XtFree((char *) tlinks);

  return(1);
}

#define RESNAMESIZE 64
#define NRES 22    /* This has to be exact */
int
save_resources(char *rootname, xgobidata *xg)
{
  char *fname;
  char resfile[110];
  int j;
  Dimension pwwidth, pwheight;
  Dimension vpwidth;
  FILE *fp;

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);

  (void) strcpy(resfile, fname);
  (void) strcat(resfile, ".resources");
  XtFree(fname);

  XtVaGetValues(xg->var_panel, XtNwidth, &vpwidth, NULL);
  XtVaGetValues(xg->workspace,
    XtNwidth, &pwwidth,
    XtNheight, &pwheight, NULL);

  if (xg->data_mode == Sprocess)
  {
    char Spath[50];
    long foo;
    namesv = (char *) XtMalloc((Cardinal) (NRES*RESNAMESIZE) * sizeof(char));

    (void) strcpy(Spath, Spath0);
    (void) strcat(Spath, resfile);
    if ( (fp = fopen(Spath, "w")) == NULL)
    {
      char message[MSGLENGTH];
      sprintf(message, "Failed to open the file '%s' for writing.\n", Spath);
      show_message(message, xg);
      return(0);
    }
    else
    {
      /*
       * "1" indicates that the following is an
       * S data structure of one element.
      */
      (void) fprintf(fp, "%cS data%c", (char) 0 , (char) 1);
      /*
       * "5" indicates that the following is of type
       * character.
      */
      foo = (long) 5;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "xgobi: error in writing resources\n");

      /*
       * NRES says the following has NRES elements."
      */
      foo = (long) NRES;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "xgobi: error in writing resources\n");

      nameslen = 0;
      j = 0;
      sprintf(NAMESV(j), "*showAxes: %d", xg->is_axes);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*showPoints: %d", xg->plot_the_points);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*showLines: %d", xg->connect_the_points);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
/*
 * This section could be made to reflect all the different kinds
 * of linking now available, but now now ...
*/
      sprintf(NAMESV(j), "*linkGlyphBrush: %d", xg->link_glyph_brushing);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*linkColorBrush: %d", xg->link_color_brushing);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*linkEraseBrush: %d", xg->link_erase_brushing);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*linkLineBrush: %d", xg->link_lines_to_lines);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*linkIdentify: %d", xg->link_identify);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*carryVars: %d", xg->carry_vars);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*jumpBrush: %d", xg->jump_brush);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*reshapeBrush: %d", xg->reshape_brush);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*syncBrush: %d", xg->sync_brush);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*plotSquare: %d", xg->plot_square);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*XGobi*PlotWindow.height: %d", pwheight);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*XGobi*PlotWindow.width: %d", pwwidth);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*XGobi*VarPanel.width: %d", vpwidth);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      /* Cloning resources */
      j++;
      sprintf(NAMESV(j), "*isCloned: %d", xg->isCloned);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*clonePID: %d", xg->clone_PID);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*cloneTime: %d", xg->clone_Time);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*cloneType: %d", (int) xg->clone_Type);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*cloneName: %s", xg->clone_Name);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;
      sprintf(NAMESV(j), "*deleteCloneData: %d", xg->delete_clone_data);
      nameslen = nameslen + strlen(NAMESV(j)) + 1;
      j++;

      if (j != NRES)
        fprintf(stderr,
          "NRES must be set to exactly the number of resources saved.\n");

      foo = (long) nameslen;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "error 3 in writing resources\n");

      for (j=0; j<NRES; j++)
        (void) fprintf(fp, "%s%c",  NAMESV(j), (char) 0);
      if (fclose(fp) == EOF)
        fprintf(stderr, "error 4 in writing resources\n");
      XtFree((XtPointer) namesv);
    }

  }
  else /*  if (xg->data_mode == ascii || xg->data_mode == binary) */
  {
    if ((fp = fopen(resfile, "w")) == NULL)
    {
      char message[MSGLENGTH];
      sprintf(message, "Failed to open the file '%s' for writing.\n", resfile);
      show_message(message, xg);
      return(0);
    }
    else
    {
      fprintf(fp, "*showAxes: %d\n", xg->is_axes);
      fprintf(fp, "*showPoints: %d\n", xg->plot_the_points);
      fprintf(fp, "*showLines: %d\n", xg->connect_the_points);
      fprintf(fp, "*linkIdentify: %d\n", xg->link_identify);
      fprintf(fp, "*linkGlyphBrush: %d\n", xg->link_glyph_brushing);
      fprintf(fp, "*linkColorBrush: %d\n", xg->link_color_brushing);
      fprintf(fp, "*linkEraseBrush: %d\n", xg->link_erase_brushing);
      fprintf(fp, "*linkLineBrush: %d\n", xg->link_lines_to_lines);
      fprintf(fp, "*carryVars: %d\n", xg->carry_vars);
      fprintf(fp, "*jumpBrush: %d\n", xg->jump_brush);
      fprintf(fp, "*reshapeBrush: %d\n", xg->reshape_brush);
      fprintf(fp, "*syncBrush: %d\n", xg->sync_brush);
      fprintf(fp, "*XGobi*PlotWindow.height: %d\n", pwheight);
      fprintf(fp, "*XGobi*PlotWindow.width: %d\n", pwwidth);
      fprintf(fp, "*XGobi*VarPanel.width: %d\n", vpwidth);
      /* Cloning Resources */
      fprintf(fp, "*isCloned: %d\n", xg->isCloned);
      fprintf(fp, "*clonePID: %d\n", xg->clone_PID);
      fprintf(fp, "*cloneTime: %d\n", xg->clone_Time);
      fprintf(fp, "*cloneType: %d\n", (int) xg->clone_Type);
      fprintf(fp, "*cloneName: %s\n", xg->clone_Name);
      fprintf(fp, "*deleteCloneData: %d\n", (int) xg->delete_clone_data);
      fclose(fp);
    }
  }
  return(1);
}

void
copy_resources(char *rootname, xgobidata *xg)
{
/*
 * Write out the entire resource database, display->db.
*/
  char *fname;
  char fnameplus[512];
  XrmDatabase db = XrmGetDatabase(display);

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);

  sprintf(fnameplus, "%s.resources", fname);

  XrmPutFileDatabase(db, fnameplus);
}

Boolean
save_missing(char *rootname, int *rowv, int nr, int *colv, int nc,
xgobidata *xg)
{
  int i;
  char *fname;
  Boolean success = True;
  FILE *fp;

  if (rowv == (int *) NULL) {
    rowv = (int *) XtMalloc((Cardinal) nr * sizeof(int));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);
  strcat(fname, ".missing");

  if ((fp = fopen(fname, "w")) == NULL) {
    fprintf(stderr, "Problem writing out the missing file, %s\n", fname);
    success = False;
  }
  else
  {
    int i, j, jc, m;
    for (i=0; i<nr; i++)
    {
      m = rowv[i];

      for (j=0; j<nc; j++) {
        if (colv == (int *) NULL)  /* Write all columns, in default order */
          jc = j;
        else
          jc = colv[j];  /* Write the columns as specified */

         if (xg->is_missing_values_xgobi)
            /* xgobi itself is missing_value_xgobi - we have
               to write the data, not the missings */
           (void) fprintf(fp, "%d ", (int) xg->raw_data[m][jc]);
         else
           (void) fprintf(fp, "%d ", xg->is_missing[m][jc]);
      }
      (void) fprintf(fp, "\n");
    }
    fclose(fp);
  }

  return(success);
}

Boolean
copy_impnames(char *rootname, xgobidata *xg)
{
  int n;
  char *fname;
  Boolean success = True;
  FILE *fp;
  char message[256];
  extern int nimputations;
  extern char **imp_names;

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);
  strcat(fname, ".impnames");
   
  if ( (fp = fopen(fname, "w")) == NULL) {
    sprintf(message,
      "The file '%s' can not be opened for writing\n", fname);
    show_message(message, xg);
    success = False;
  } else {
    for (n=0; n<nimputations; n++)
      fprintf(fp, "%s\n", imp_names[n]);
    fclose(fp);
  }
  return(success);
}

/* Copy all the imputation data: used when all data are being written */
Boolean
copy_imputations(char *rootname, xgobidata *xg)
{
  int k, n;
  char *fname;
  Boolean success = True;
  FILE *fp;
  char message[256];
  extern int nimputations;
  extern char **imp_names;
  extern float **imp_values;

  fname = XtMalloc(132 * sizeof(char));
  if (rootname == (char *) NULL)
    strcpy(fname, xg->datafilename);
  else
    strcpy(fname, rootname);
  strcat(fname, ".imp");
   
  if ( (fp = fopen(fname, "w")) == NULL) {
    sprintf(message,
      "The file '%s' can not be opened for writing\n", fname);
    show_message(message, xg);
    success = False;
  } else {
    for (k=0; k<xg->nmissing; k++) {
      for (n=0; n<nimputations; n++)
        fprintf(fp, "%f ", imp_values[k][n]);
      fprintf(fp, "\n");
    }
    fclose(fp);
  }
  return(success);
}
