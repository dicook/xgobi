/* read_array.c */
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
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#define BLOCKSIZE 1000

Boolean gotone = False;

/*
 * Some people's distributions of the X libraries don't contain these
 * two routines, so here they are:
*/

#if !defined(XlibSpecificationRelease) || XlibSpecificationRelease < 5
XrmDatabase
XrmGetDatabase(Display *display)
{
  return display->db;
}
void
XrmSetDatabase(Display *display, XrmDatabase database)
{
  display->db = database;
}
#endif

/*ARGSUSED*/
static void
stdin_empty(int arg)
{
  if (!gotone) {
    fprintf(stderr, "xgobi requires a filename or some data from stdin\n");
    exit(0);
  }
}

void
strip_suffixes(xgobidata *xg)
{
/*
 * Find the name of the data file excluding certain suffixes:
 * .bin, .missing, .dat
*/
  int i, nchars;
  static char *suffix[] = {
    ".bin",
    ".missing",
    ".dat"
  };

  sprintf(xg->datafname, "%s", xg->datafilename);
  for (i=0; i<3; i++)
  {
    nchars = strlen(xg->datafilename) - strlen(suffix[i]) ;
    if (strcmp(suffix[i],
               (char *) &xg->datafilename[nchars]) == 0)
    {
      strncpy((char *) xg->datafname, (char *) xg->datafilename, nchars);
      xg->datafname[nchars] = '\0';
      break;
    }
  }
}

void
read_binary(FILE *fp, xgobidata *xg)
{
  int i, j, nr, nc;
  int onesize = sizeof(float);
  int out;

  fread((char *) &nr, sizeof(nr), 1, fp);
  fread((char *) &nc, sizeof(nc), 1, fp);

  xg->ncols_used = nc;
  xg->ncols = nc + 1;

  if (xg->file_read_type == read_all)
    xg->nrows = nr;
  else
    xg->nrows = xg->file_sample_size;

  xg->file_rows_sampled = (long *)
    XtMalloc((Cardinal) xg->nrows * sizeof(long));

  xg->raw_data = (float **) XtMalloc((Cardinal) xg->nrows * sizeof(float *));
  for (i=0; i<xg->nrows; i++)
    xg->raw_data[i] = (float *) XtMalloc((Cardinal) xg->ncols * sizeof(float));

  if (xg->file_read_type == read_block) {
    fseek (fp,
      (long) (sizeof(long)*2 + onesize * (xg->file_start_row * xg->ncols_used)),
      SEEK_SET);
  }

  if (xg->file_read_type == read_all) {

    for (i=0; i<xg->nrows; i++) {
      for (j=0; j<xg->ncols_used; j++) {
        out = fread((char *) &xg->raw_data[i][j], onesize, 1, fp);
        if (out != 1) {
          fprintf(stderr, "problem in reading the binary data file\n");
          fclose(fp);
          exit(0);

        } else if (xg->raw_data[i][j] == FLT_MAX) {
          xg->raw_data[i][j] = 0.0;
          /* Allocate the missing values array */
          if (!xg->missing_values_present) {
            init_missing_array(xg->nrows, xg->ncols, xg);
            xg->missing_values_present = true;
          }

          xg->is_missing[i][j] = 1;
          xg->nmissing++;
        }
      }
    }

  } else if (xg->file_read_type == read_block) {
    int q;

    for (i=0, q=xg->file_start_row; i<xg->nrows; i++, q++) {
      xg->file_rows_sampled[i] = q;
      for (j=0; j<xg->ncols_used; j++) {
        out = fread((char *) &xg->raw_data[i][j], onesize, 1, fp);
        if (out != 1) {
          fprintf(stderr, "problem in reading the binary data file\n");
          fclose(fp);
          exit(0);

        } else if (xg->raw_data[i][j] == FLT_MAX) {
          xg->raw_data[i][j] = 0.0;
          /* Allocate the missing values array */
          if (!xg->missing_values_present) {
            init_missing_array(xg->nrows, xg->ncols, xg);
            xg->missing_values_present = true;
          }

          xg->is_missing[i][j] = 1;
          xg->nmissing++;
        }
      }
    }

  } else if (xg->file_read_type == draw_sample) {

    int t, m, n;
    float rrand;

    /* This is the number of rows we will sample */
    n = xg->nrows;
    if (n > 0 && n < xg->file_length) {

      for (t=0, m=0; t<xg->file_length && m<n; t++) {

        rrand = (float) randvalue();

        if ( ((float)(xg->file_length - t) * rrand) < (float)(n - m) )
        {
          /* seek forward in the file to row to be read */
          if (fseek(fp,
              (long)(sizeof(long)*2 + onesize * t * xg->ncols_used),
              SEEK_SET) == 0)
          {
            xg->file_rows_sampled[m] = t;

            for (j=0; j<xg->ncols_used; j++) {
              out = fread((char *) &xg->raw_data[i][j], onesize, 1, fp);
              if (out != 1) {
                fprintf(stderr, "problem in reading the binary data file\n");
                fclose(fp);
                exit(0);
              } else if (xg->raw_data[i][j] == FLT_MAX) {
                xg->raw_data[i][j] = 0.0;

                /* Allocate the missing values array */
                if (!xg->missing_values_present) {
                  init_missing_array(xg->nrows, xg->ncols, xg);
                  xg->missing_values_present = true;
                }
                xg->is_missing[i][j] = 1;
                xg->nmissing++;
              }
            }
            m++;
          }
        }
      }
    }
  }

  if (fclose(fp) == EOF)
    fprintf(stderr, "binary_read: error in fclose");
}

/********************************************************************
*          Reading ascii files                                      *
*********************************************************************/

void
alloc_block(int nblocks, xgobidata *xg) {
/*
 * Allocate space for nblocks*BLOCKSIZE rows
*/
  int i;

  xg->raw_data = (float **) XtRealloc( (char *) xg->raw_data,
    (Cardinal) (nblocks * BLOCKSIZE) * sizeof(float *));
  for (i=BLOCKSIZE*(nblocks-1); i<BLOCKSIZE*nblocks; i++)
    xg->raw_data[i] = (float *)
      XtMalloc((Cardinal) xg->ncols * sizeof(float));
}     

static void
alloc_missing_block(int nblocks, xgobidata *xg) {
/*
 * Allocate is_missing for nblocks*BLOCKSIZE rows
*/
  int i;

  xg->is_missing = (short **) XtRealloc((char *) xg->is_missing,
    (Cardinal) (nblocks * BLOCKSIZE) * sizeof(short *));
  for (i=BLOCKSIZE*(nblocks-1); i<BLOCKSIZE*nblocks; i++)
    xg->is_missing[i] = (short *)
      XtCalloc((Cardinal) xg->ncols, sizeof(short));
}

Boolean
find_data_start(FILE *fp)
{
  int ch;
  Boolean morelines = True;
  Boolean comment_line = True;

  while (comment_line)
  {
    /* skip white space */
    while (1)
    {
      ch = getc(fp);
      if (ch == '\t' || ch == ' ' || ch == '\n')
        ;
      else
        break;
    }

    /*
     * If we've crept up on an EOF, set morelines to False.
    */
    if (ch == EOF)
    {
      morelines = False;
      break;
    }

    /* Comment lines must begin with a punctuation character */
    else if (ispunct(ch) && ch != '-' && ch != '+' && ch != '.')
    {
      fprintf(stderr, "Skipping a comment line beginning with '%c'\n", ch);
      while ((ch = getc(fp)) != '\n')
        ;
    }
    else if (isalpha(ch) && ch != 'n' && ch != 'N')
    {
      fprintf(stderr, "Comment lines must begin with # or %%;\n");
      fprintf(stderr, "I found a line beginning with '%c'\n", ch);
      exit(1);
    }
    else
    {
      comment_line = False;
      ungetc(ch, fp);
    }
  }

  return(morelines);
}

static void
init_file_rows_sampled(xgobidata *xg) {
  int i, k, t, m, n;
  float rrand;

  switch (xg->file_read_type) {
    case read_all:
      xg->file_rows_sampled = NULL;
      break;

    case read_block:
      xg->nrows = xg->file_sample_size;
      xg->file_rows_sampled = (long *)
        XtMalloc((Cardinal) xg->nrows * sizeof(long));
      for (i=0, k=xg->file_start_row; i<xg->nrows; i++, k++)
        xg->file_rows_sampled[i] = k;
      break;

    case draw_sample:

      xg->nrows = xg->file_sample_size;
      xg->file_rows_sampled = (long *)
        XtMalloc((Cardinal) xg->nrows * sizeof(long));

      n = xg->nrows;
      if (n > 0 && n < xg->file_length) { 
        for (t=0, m=0; t<xg->file_length && m<n; t++) {

          rrand = (float) randvalue();
          if ( ((float)(xg->file_length - t) * rrand) < (float)(n - m) )
            xg->file_rows_sampled[m++] = t;
        }
      }
      break;

    default:
      fprintf(stderr,
        "Impossible value for xg->file_read_type: %d\n", xg->file_read_type);
  }
}

static Boolean
seek_to_file_row(int array_row, FILE *fp, xgobidata *xg) {
  int i, file_row, ch;  /* ch is used, no matter what the compiler says */
  static int prev_file_row = 0;
  Boolean ok = true;

  if (array_row >= xg->file_sample_size) {
    return false;
  }

  /* Identify the row number of the next file row we want. */
  file_row = xg->file_rows_sampled[array_row];


  for (i=prev_file_row; i<file_row; i++) {
    if (!find_data_start(fp)) {
      ok = False;
      break;
    } else {
      /* skip a line */
      while ((ch = getc(fp)) != '\n') {
        ;
      }
    }
  }

  /* add one to step over the one we're about to read in */
  prev_file_row = file_row+1; 

  return ok;
}

void
read_ascii(FILE *fp, xgobidata *xg)
{
  register int ch;
  int i, j, k, jrows, nrows, jcols, fs;
  int nitems;
  float row1[NCOLS];
  short row1_missing[NCOLS];
  int nblocks;
  char word[64];

  /* Initialize these before starting */
  for (k=0; k<NCOLS; k++) {
    row1_missing[k] = 0;
    row1[k] = 0.0;
  }
  xg->ncols_used = 0;

  init_file_rows_sampled(xg);

/*
 * Find the index of the first row of data that we're interested in.
*/

  nrows = 0;
  if (xg->file_read_type == read_all) {
    if (find_data_start(fp) == False)
      return;

  } else {  /* if -only was used on the command line */
    if (!seek_to_file_row(nrows, fp, xg))
      return;
  }

/*
 * Read in the first row of the data file and calculate ncols.
*/

  gotone = True;

/*
 * I've left behind some checking that's done in bak/read_array.c --
 * test xgobi on a text file and see what happens.
*/

  while ( (ch = getc(fp)) != '\n') {

    if (ch == '\t' || ch == ' ')
      ;

    else if ( ungetc(ch, fp) == EOF || fscanf(fp, "%s", word) < 0 ) {
      fprintf(stderr,
        "read_array: error in reading first row of data\n");
      fclose(fp);
      exit(0);

    } else {

      if ( strcasecmp(word, "na") == 0 || strcmp(word, ".") == 0) {
        xg->missing_values_present = True;
        xg->nmissing++;
        row1_missing[xg->ncols_used] = 1;

      } else {
        row1[xg->ncols_used] = (float) atof(word);
      }
      xg->ncols_used++ ;

      if (xg->ncols_used >= NCOLS) {
        fprintf(stderr,
         "This file has more than %d columns.  In order to read it in,\n",
          NCOLS);
        fprintf(stderr,
         "increase NCOLS in xgobitypes.h and recompile.\n");
        exit(0);
      }
    }
  }

  xg->ncols = xg->ncols_used + 1;

/*
 * If we're reading everything, allocate the first block.
 * If -only has been used, allocate the whole shebang.
*/
  if (xg->file_read_type == read_all) {

    xg->nrows = 0;
    alloc_block(1, xg);
    if (xg->missing_values_present)
      alloc_missing_block(1, xg);

  } else {  /* -only has been used */

    xg->nrows = xg->file_sample_size;
    xg->raw_data = (float **) XtMalloc(
      (Cardinal) xg->nrows * sizeof(float *));
    for (i=0; i<xg->nrows; i++)
      xg->raw_data[i] = (float *)
        XtMalloc((Cardinal) xg->ncols * sizeof(float));

    if (xg->missing_values_present)
      init_missing_array(xg->nrows, xg->ncols, xg);
  }

/*
 * Fill in the first row
*/
  for (j=0; j<xg->ncols_used; j++)
    xg->raw_data[0][j] = row1[j];
  if (xg->missing_values_present) {
    for (j=0; j<xg->ncols_used; j++)
      xg->is_missing[0][j] = row1_missing[j];
  }
  nrows++;

/*
 * Read data, reallocating as needed.  Determine nrows for the read_all case.
*/
  nblocks = 1;
  nitems = xg->ncols_used;
  jrows = 1;
  jcols = 0;
  while (1)
  {

    if (jcols == 0) {
      if (xg->file_read_type == read_all) {
        if (!find_data_start(fp))
          break;

      } else {  /* if -only was used on the command line */
        if (!seek_to_file_row(nrows, fp, xg))
          break;
      }
    }

    fs = fscanf(fp, "%s", word);

    if (fs == EOF)
      break;
    else if (fs < 0)
    {
      fprintf(stderr, "Problem with input data\n");
      fclose(fp);
      exit(0);
    }
    else
    {
      nitems++;

      if ( strcasecmp(word, "na") == 0 || strcmp(word, ".") == 0 ) {

        if (!xg->missing_values_present) {
          xg->missing_values_present = True;
          /*
           * Only when the first "na" or "." has been encountered
           * is it necessary to allocate space to contain the
           * missing values matrix.  Initialize all previous values
           * to 0.
          */
          if (xg->file_read_type == read_all) {
            alloc_missing_block(nblocks, xg);
            for (i=BLOCKSIZE*(nblocks-1); i<BLOCKSIZE*nblocks; i++) {
              for (k=0; k<xg->ncols_used; k++)
                xg->is_missing[i][k] = 0;
            }
          } else {
            init_missing_array(xg->nrows, xg->ncols, xg);
          }
        }

        xg->nmissing++;
        xg->is_missing[nrows][jcols] = 1;
        xg->raw_data[nrows][jcols] = 0.0;
      }
      else
        xg->raw_data[nrows][jcols] = (float) atof(word);

      jcols++;
      if (jcols == xg->ncols_used)
      {
        jcols = 0;
        nrows++;
        jrows++;
      }

      if (xg->file_read_type == read_all) {
        if (jrows == BLOCKSIZE) {
          jrows = 0;
          nblocks++;
          if (nblocks%20 == 0)
            fprintf(stderr, "reallocating; n > %d\n", nblocks*BLOCKSIZE);

          alloc_block(nblocks, xg);

          if (xg->missing_values_present)
            alloc_missing_block(nblocks, xg);
        }

      } else {  /* -only was used */
        if (nrows >= xg->nrows)
          break;
      }
    }
  }

/*
 * Close the data file
*/
  if (fclose(fp) == EOF)
    fprintf(stderr, "read_array: error in fclose");

  if (xg->file_read_type == read_all)
    xg->nrows = nrows;

  fprintf(stderr, "size of data: %d x %d\n", xg->nrows, xg->ncols);

  if ( nitems != xg->nrows * xg->ncols_used )
  {
    (void) fprintf(stderr, "read_array: nrows*ncols != nitems read\n");
    (void) fprintf(stderr, "(nrows %d, ncols %d, nitems read %d)\n",
      xg->nrows, xg->ncols_used, nitems);
    exit(0);
  }
  else if (nitems == 0)
  {
    (void) fprintf(stderr, "No data read\n");
    exit(0);
  }
  else
  {
    /*
     * If we haven't yet encountered a missing value, free up
     * the whole matrix.
    */
    if (!xg->missing_values_present)
      xg->is_missing = (short **) NULL;

    if (xg->file_read_type == read_all) {
      /*
       * One last XtFree and XtRealloc to make raw_data take up exactly
       * the amount of space it needs.
      */
      for (i=xg->nrows; i<BLOCKSIZE*nblocks; i++)
        XtFree((XtPointer) xg->raw_data[i]);
      xg->raw_data = (float **) XtRealloc((XtPointer) xg->raw_data,
        (Cardinal) xg->nrows * sizeof(float *));

      if (xg->missing_values_present) {
        for (i=xg->nrows; i<BLOCKSIZE*nblocks; i++)
          XtFree((XtPointer) xg->is_missing[i]);
        xg->is_missing = (short **) XtRealloc((XtPointer) xg->is_missing,
          (Cardinal) xg->nrows * sizeof(short *));
      }
    }

    /*
     * If the data contains only one column, add a second,
     * the numbers 1:nrows -- and let the added column be
     * the first column?
    */
    xg->single_column = False;
    if (xg->ncols_used == 1)
    {
      xg->single_column = True;
      xg->ncols_used = 2;
      xg->ncols = 3;
      for (i=0; i<xg->nrows; i++)
      {
        xg->raw_data[i] = (float *) XtRealloc(
          (XtPointer) xg->raw_data[i],
          (Cardinal) 3 * sizeof(float));
        xg->raw_data[i][1] = xg->raw_data[i][0] ;
        xg->raw_data[i][0] = (float) (i+1) ;

        /* And populate a column of missing values with 0s, if needed */
        if (xg->missing_values_present)
        {
          xg->is_missing[i] = (short *) XtRealloc(
            (XtPointer) xg->is_missing[i],
            (Cardinal) 3 * sizeof(short));
          xg->is_missing[i][1] = 0 ;
        }
      }
    }
  }
}

/********************************************************************
*          End of section on reading ascii files                    *
*********************************************************************/

void 
make_scatmat(xgobidata *xg)
{
  int i, i2, j, k, *cols, ncells;
  float *min, *max, **scatmat, cpi5, spi5;
  double pi5;

  ncells = xg->nrows*xg->ncols_used*(xg->ncols_used-1)/2;
  min = (float *) XtMalloc((Cardinal) xg->ncols * sizeof(float));
  max = (float *) XtMalloc((Cardinal) xg->ncols * sizeof(float));
  cols = (int *) XtMalloc((Cardinal) xg->ncols * sizeof(int));
  scatmat = (float **) XtMalloc((Cardinal) ncells * sizeof(float *));
  for (i=0; i<ncells; i++)
    scatmat[i] = (float *) XtMalloc((Cardinal) 4 * sizeof(float));

  for (j=0; j<xg->ncols_used; j++)
  {
    min[j]=xg->raw_data[0][j];
    max[j]=xg->raw_data[0][j];
    for (i=0; i<xg->nrows; i++)
    {
      if (min[j] > xg->raw_data[i][j])
        min[j] = xg->raw_data[i][j];
      if (max[j] < xg->raw_data[i][j])
        max[j] = xg->raw_data[i][j];
    }
  }

  for (j=0; j<xg->ncols_used; j++)
    for (i=0; i<xg->nrows; i++)
    {
      if (max[j]-min[j] > 0)
        xg->raw_data[i][j] = (xg->raw_data[i][j]-min[j])/(max[j]-min[j]);
      else
        xg->raw_data[i][j] = 0.5;
    }

  /* Horizontal layout variable */
  k=0;
  for (j=2; j<=xg->ncols_used; j++)
  {
    for (i=0; i<xg->nrows*(xg->ncols_used-j+1); i++)
    {
      scatmat[k][0] = j-1;
      k++;
    }
  }

  /* Vertical layout variable */
  k=0;
  for (j=2; j<=xg->ncols_used; j++)
  {
    for (i=xg->ncols_used-j+1; i>0; i--)
      for (i2=0; i2<xg->nrows; i2++)
      {
        scatmat[k][1] = i;
        k++;
      }
  }

  /* Horizontal variable values */
  k=0;
  for (j=2; j<=xg->ncols_used; j++)
  {
    for (i=0; i<(xg->ncols_used-j+1); i++)
      for (i2=0; i2<xg->nrows; i2++)
      {
        scatmat[k][2] = xg->raw_data[i2][j-2];
        k++;
      }
  }

  /* Vertical variable values */
  k=0;
  for (j=2; j<=xg->ncols_used; j++)
  {
    for (i=j; i<=xg->ncols_used; i++)
      for (i2=0; i2<xg->nrows; i2++)
      {
        scatmat[k][3] = xg->raw_data[i2][i-1];
        k++;
      }
  }

  for (i=0; i<xg->nrows; i++)
      XtFree((XtPointer) xg->raw_data[i]);
  XtFree((char *) xg->raw_data);

  xg->sm_nrows = xg->nrows;
  xg->sm_ncols = xg->ncols_used;
  xg->nrows = ncells+xg->sm_ncols;/* additional piece to do var labels */
  xg->ncols = 3;
  xg->ncols_used = 2;
  xg->raw_data = (float **) XtMalloc((Cardinal) xg->nrows * sizeof(float *));
  for (i=0; i<xg->nrows ; i++)
    xg->raw_data[i] = (float *) XtMalloc((Cardinal) xg->ncols * sizeof(float));

  pi5 = (double) M_PI/5.;
  cpi5 = (float) cos(pi5);
  spi5 = (float) sin(pi5);
  /*  make dummy points for labels */
  for (i=0; i<xg->nrows-xg->sm_ncols; i++)
  {
    xg->raw_data[i][0] = cpi5*scatmat[i][0]+spi5*scatmat[i][2];
    xg->raw_data[i][1] = cpi5*scatmat[i][1]+spi5*scatmat[i][3];
  }
  for (j=0; j<xg->sm_ncols; j++)
  {
    xg->raw_data[xg->nrows-xg->sm_ncols+j][0] = cpi5*(j+1)+spi5*0.5;
    xg->raw_data[xg->nrows-xg->sm_ncols+j][1] = cpi5*(xg->sm_ncols-j)+spi5*0.5;
  }

  for (i=0; i<xg->nrows-xg->sm_ncols; i++)
      XtFree((XtPointer) scatmat[i]);
  XtFree((XtPointer) scatmat);
  XtFree((XtPointer) min);
  XtFree((XtPointer) max);
  XtFree((XtPointer) cols);
}

void
read_array(xgobidata *xg)
{
  char fname[128];
  FILE *fp;
  static char *suffixes[] = {".dat", ""};

/*
 * Check file exists and open it - for stdin no open needs to be done
 * only assigning fp to be stdin.
*/
  if (strcmp((char *) xg->datafname, "stdin") == 0) {
    fp = stdin;

    /*
     * If reading from stdin, set an alarm.  If after 5 seconds,
     * no data has been read, print an error message and exit.
    */
    if (fp == stdin)
    {
      alarm((unsigned int) 5);
      signal(SIGALRM, stdin_empty);
    }
    read_ascii(fp, xg);
  }
  else {
    /* 
     * Are we reading the missing data into xg->raw_data ?
    */
    if (strcmp(
         ".missing",
         &xg->datafilename[strlen(xg->datafilename) - strlen(".missing")]
       ) == 0)
    {
      if ((fp = fopen(xg->datafilename, "r")) != NULL) {
        char *title, fulltitle[256];
        xg->is_missing_values_xgobi = True;
        xg->missing_values_present = True;
        read_ascii(fp, xg);

        /*
         * extend the title
        */
        title = (char *) XtMalloc(256 * sizeof(char));
        XtVaGetValues(xg->shell,
          XtNtitle, (String) &title,
          NULL);
        sprintf(fulltitle, "%s", title);

        /* -vtitle has been used */
        if (strcmp(xg->vtitle, "") != 0) {
           strcpy(fulltitle, xg->vtitle);
           sprintf(xg->vtitle, "");
        }

        XtVaSetValues(xg->shell,
          XtNtitle, (String) fulltitle,
          XtNiconName, (String) fulltitle,
          NULL);
      }
      else
      {
        (void) fprintf(stderr,
          "The file %s can't be opened for reading.\n", xg->datafilename);
        exit(0);
      }
    }
    else
    {
      /*
       * Try fname.bin before fname, to see whether there is a binary
       * data file available.  If there is, call read_binary().
      */
      strcpy(fname, (char *) xg->datafname);
      strcat(fname, ".bin");

      if ((fp = fopen(fname, "rb")) != NULL)
        read_binary(fp, xg);

      /*
       * If not, look for an ASCII file
      */
      else
      {
        fp = open_xgobi_file((char *) xg->datafname, 2, suffixes, "r", false);

        if (fp == NULL)
          exit(0);

        read_ascii(fp, xg);
      }
    }
  }

  if (xg->is_scatmat)
    make_scatmat(xg);
}

int
Sread_array(xgobidata *xg)
{
  int i, j, nitems = 0;
  int ok = 0;
  FILE *fp;
  char word[64];

  if ((fp = fopen((char *) xg->datafname, "r")) == NULL) {
    (void) fprintf(stderr,
      "Sread_array: data file %s does not exist\n", xg->datafname);
    exit(0);

  }

  else
  {
    xg->raw_data = (float **) XtMalloc(
			(Cardinal) xg->nrows * sizeof(float *));
    for (i=0; i<xg->nrows; i++)
      xg->raw_data[i] = (float *) XtMalloc(
				(Cardinal) xg->ncols * sizeof(float));

    /*
     * Allocate space for missing data, and free it if
     * it isn't used.
    */
    xg->is_missing = (short **) XtMalloc(
			(Cardinal) xg->nrows * sizeof(short *));
    for (i=0; i<xg->nrows; i++)
      xg->is_missing[i] = (short *) XtMalloc(
				(Cardinal) xg->ncols * sizeof(short));

    for (i=0; i<xg->nrows; i++) {
      for (j=0; j<xg->ncols_used; j++) {
        (void) fscanf(fp, "%s", word);
        if (strcasecmp(word, "na") == 0 || strcmp(word, ".") == 0) {
          if (!xg->missing_values_present)
            xg->missing_values_present = True;
          xg->is_missing[i][j] = 1;
          xg->raw_data[i][j] = 0.0;
        } else {
          xg->is_missing[i][j] = 0;
          xg->raw_data[i][j] = (float) atof(word);
        }
        nitems++;
      }
    }

    /*
     * Test the number of items read against the dimension
     * of the array.
    */
    if (nitems == xg->nrows * xg->ncols_used)
    {
      ok = 1;
      if (unlink((char *) xg->datafname) != 0)
        fprintf(stderr, "Sread_array: error in unlink\n");
    }
    else
    {
      ok = 0;
      (void) fprintf(stderr, "Sread_array: nrows*ncols != nitems\n");
      exit(0);
    }

    /*
     * Now fill some data into the extra column, which will be
     * used if groups are defined by brushing.
    */
    for (i=0; i<xg->nrows; i++) {
      xg->raw_data[i][xg->ncols_used] = 1.0;
      xg->is_missing[i][xg->ncols_used] = 0;
    }

    /*
     * If we haven't yet encountered a missing value, free up
     * the whole matrix.
    */
    if (!xg->missing_values_present)
    {
      for (i=0; i<xg->nrows; i++)
        XtFree((XtPointer) xg->is_missing[i]);
      XtFree((char *) xg->is_missing);
      xg->is_missing = (short **) NULL;
    }
  }

  return(ok);
}


void
fill_extra_column(xgobidata *xg)
{
  int i;

  for (i=0; i<xg->nrows; i++)
    xg->raw_data[i][xg->ncols_used] = 1.0 ;
}

void
find_root_name_of_data(char *fname, char *title)
/*
 * Strip off preceding directory names and find the final filename
 * to use in resetting the title and iconName.
*/
{
  char *pf;
  int j = 0;

  pf = fname;
  while (fname[j] != '\0')
  {
    /* Adding \\ for Ripley's Windows version */
    if (fname[j] == '/' || fname[j] == '\\')
      pf = &fname[j+1];
    j++;
  }

  (void) strcpy(title, pf);
}

void
set_title_and_icon(char *fname, xgobidata *xg)
/*
 * If the user hasn't specified a title in a general resource
 * file, a data-specific resource file, or on the command
 * line, and if the data is being read in from a file, then
 * use the name of the file to compose the title and iconName
 * resources.
*/
{
  char fulltitle[150], usertitle[120], rootfname[100];
  char *str_type[50];
  XrmValue value;

  usertitle[0] = '\0' ;
  if (XrmGetResource((XrmDatabase) XrmGetDatabase(display),
    "xgobi.title", "XGobi.Title", str_type, &value))
  {
    (void) strncpy(usertitle, value.addr, (int) value.size);
  }

  if (strcmp(fname, "stdin") == 0) { /* If input is from stdin */
    /*
     * This section of code is almost, but not exactly the same
     * as the section for file input ...
    */

    /* usertitle is by default XGobi, the class name */
    if (usertitle && ((int)strlen(usertitle) > 0)) {
      /* 
       * If the title is exactly XGobi (or xgobi), use it
      */ 
      if (strcasecmp(usertitle, "XGobi") == 0)
        sprintf(fulltitle, usertitle);

      /* If the title contains XGobi (or xgobi), just use it */
      else if (strncasecmp("XGobi", usertitle, 5) == 0)
        sprintf(fulltitle, usertitle);
      
      /* else add the supplied title to fulltitle */
      else
        sprintf(fulltitle, "XGobi: %s", usertitle);
    }

    /* Hmm.  This will have to be tested in Clone_XGobi() */
    xg->datarootname = (char *) NULL;
  }
  else  /* If reading from a file */
  {
    /* Used in Clone_XGobi() as well as in one option here */
    find_root_name_of_data(fname, rootfname);
    xg->datarootname = XtMalloc((unsigned)
      (strlen(rootfname) + 1) * sizeof(char));
    strcpy(xg->datarootname, rootfname);

    strcpy(fulltitle, "XGobi: ");

    /* usertitle is by default XGobi, the class name */
    if (usertitle && ((int)strlen(usertitle) > 0))
    {
      /*
       * If the title is exactly XGobi, add the data name
       * to fulltitle
      */
      if (strcmp(usertitle, "XGobi") == 0)
        strcat(fulltitle, rootfname);

      /* If the title contains XGobi, just use it */
      else if (strncmp("XGobi", usertitle, 5) == 0)
        strcpy(fulltitle, usertitle);

      /* else add it to fulltitle */
      else
        strcat(fulltitle, usertitle);
    }
    else  /* If usertitle is NULL */
    {
      find_root_name_of_data(fname, rootfname);
      strcat(fulltitle, rootfname);
    }
  }

  /*
   * If vtitle has been used, override all other considerations.
   * This is helpful for cloning and calling xgobi from other
   * software.
  */
  if (strcmp(xg->vtitle, "") != 0) {
     strcpy(fulltitle, xg->vtitle);
     sprintf(xg->vtitle, "");
  }

  sprintf(xg->title, fulltitle);
  XtVaSetValues(xg->shell,
    XtNtitle, (String) fulltitle,
    XtNiconName, (String) fulltitle,
    NULL);
}

int
read_extra_resources(char *data_in)
/*
 * Read in the data-specific resource file.
*/
{
  char fname[128];
  int found = 0;
  int ok = 1;
  FILE *fp;
  static char *suffix = ".resources";

  if (data_in != "" && strcmp(data_in, "stdin") != 0) {
    (void) strcpy(fname, data_in);
    (void) strcat(fname, suffix);
    if ( (fp = fopen(fname,"r")) != NULL)
      found = 1;
  }

  if (found) {
    XrmDatabase newdb;

    if ( (newdb = (XrmDatabase) XrmGetFileDatabase(fname)) != NULL ) {
      XrmDatabase dispdb;

      dispdb = XrmGetDatabase(display);
      XrmMergeDatabases(newdb, &dispdb);
      XrmSetDatabase(display, dispdb);
    }
    else {
      ok = 0;
      fprintf(stderr,
        "read_extra_resources: problem reading data-specific resource file\n");
      exit(0);
    }

  /*
   * Close the data file
  */
    if (fclose(fp) == EOF)
      fprintf(stderr, "read_extra_resources: error in fclose");
  }
  return(ok);
}
