/* utils.c */
/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  the xgobi code and associated documentation,  provided  *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *        Copyright (c) 1997 AT&T                           *
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
#include <sys/stat.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"

/*-- start using the same random number generator as ggobi --*/
extern void sgenrand (unsigned long);
extern double genrand (void);
extern void lsgenrand(unsigned long seed_array);

void
init_random_seed() {
  sgenrand ((long) time ((long *) 0));
}


double
randvalue (void) {
  return (genrand ());   
}

void
rnorm2 (double *drand, double *dsave) {
  *drand = 2.0 * genrand () - 1.0;
  *dsave = 2.0 * genrand () - 1.0;
}

int
find_selected_cols (xgobidata *xg, int *cols)
{
  int i, ncols = 0;

  if (xg->is_plotting1d) {
    if (xg->plot1d_vars.y != -1)
      cols[ncols++] = xg->plot1d_vars.y;
    else if (xg->plot1d_vars.x != -1)
      cols[ncols++] = xg->plot1d_vars.x;
  }
  else if (xg->is_xyplotting) {
    cols[ncols++] = xg->xy_vars.x;
    cols[ncols++] = xg->xy_vars.y;
  }
  else if (xg->is_spinning) {
    cols[ncols++] = xg->spin_vars.x;
    cols[ncols++] = xg->spin_vars.y;
    cols[ncols++] = xg->spin_vars.z;
  }
  else if (xg->is_touring) {
    for (i=0; i<xg->numvars_t; i++)
      cols[ncols++] = xg->tour_vars[i];
  }
  else if (xg->is_corr_touring) {
    for (i=0; i<xg->ncorr_xvars; i++)
      cols[ncols++] = xg->corr_xvars[i];
    for (i=0; i<xg->ncorr_yvars; i++)
      cols[ncols++] = xg->corr_yvars[i];
  }
  return(ncols);
}

void
add_vgroups(xgobidata *xg, int *cols, int *ncols)
/*
 * If one of the chosen columns is in a vgroup,
 * add its comrades (unless they're already present)
*/
{
  int nc = *ncols;
  int j, k, n;

  for (j=0; j<nc; j++) {
    int vg = xg->vgroup_ids[cols[j]];

    for (k=0; k<xg->ncols_used; k++) {
      if (xg->vgroup_ids[k] == vg && k != cols[j]) {
        /* Got one; if it isn't already in cols, add it */
        Boolean addit = True;
        for (n=0; n<nc; n++) {
          if (cols[n] == k) {
            addit = False;
            break;
          }
        }
        if (addit) cols[(*ncols)++] = k;
        if (*ncols >= xg->ncols_used)
          break;
      }
    }
  }
}

int
fcompare(const void *x1, const void *x2)
{
  int val = 0;
  float *f1 = (float *) x1;
  float *f2 = (float *) x2;

  if (*f1 < *f2)
    val = -1;
  else if (*f1 > *f2)
    val = 1;

  return(val);
}

void
resort_vgroup_ids(xgobidata *xg, int *group_ids) {
  int maxid, i, id, newid, j;
  Boolean found;

  /*
   * Find maximum vgroup id.
  */
  maxid = 0;

  /* Don't allow group variable to be vgrouped */
  for (i=1; i<xg->ncols-1; i++) {
    if (group_ids[i] > maxid)
      maxid = group_ids[i];
  }

  /*
   * Find minimum vgroup id, set it to 0.  Find next, set it to 1; etc.
  */
  id = 0;
  newid = -1;
  while (id <= maxid) {
    found = false;
    for (j=0; j<xg->ncols-1; j++) {
      if (group_ids[j] == id) {
        newid++;
        found = true;
        break;
      }
    }
    if (found) {
      for (j=0; j<xg->ncols-1; j++) {
        if (group_ids[j] == id) {
          group_ids[j] = newid;
        }
      }
    }
    id++;
  }
}

/* Not used anywhere yet ... */
void
fshuffle(float *x, int n) {
/*
 * Knuth, Seminumerical Algorithms, Vol2; Algorithm P.
*/
  int i, k;
  float f;

  for (i=0; i<n; i++) {
    k = (int) (randvalue() * (double) i);
    f = x[i];
    x[i] = x[k];
    x[k] = f;
  }
}

/* ---------------------------------------------------------------------*/
/* The routines below have been added for the R/S connection */

int glyphIDfromName(char *glyphName) {
  int id = -1;

  if (strcasecmp(glyphName, "plus") == 0)
    id = PLUS_GLYPH;
  else if (strcasecmp(glyphName, "x") == 0)
    id = X_GLYPH;
  else if (strcasecmp(glyphName, "point") == 0)
    id = POINT_GLYPH;
  else if ((strcasecmp(glyphName, "open rectangle") == 0) ||
           (strcasecmp(glyphName, "open_rectangle") == 0) ||
           (strcasecmp(glyphName, "openrectangle") == 0))
    id = OPEN_RECTANGLE_GLYPH;
  else if ((strcasecmp(glyphName, "filled rectangle") == 0) ||
           (strcasecmp(glyphName, "filled_rectangle") == 0) ||
           (strcasecmp(glyphName, "filledrectangle") == 0))
    id = FILLED_RECTANGLE_GLYPH;
  else if ((strcasecmp(glyphName, "open circle") == 0) ||
           (strcasecmp(glyphName, "open_circle") == 0) ||
           (strcasecmp(glyphName, "opencircle") == 0))
    id = OPEN_CIRCLE_GLYPH;
  else if ((strcasecmp(glyphName, "filled circle") == 0) ||
           (strcasecmp(glyphName, "filled_circle") == 0) ||
           (strcasecmp(glyphName, "filledcircle") == 0))
    id = FILLED_CIRCLE_GLYPH;

  return id;
}

int glyphNames(char **names) {
  int i;
  static char* glyphNames[] =
    {"plus", "x", "openrectangle", "filledrectangle", "opencircle",
    "filledcircle", "point"};
  for (i=0; i<7; i++) names[i] = glyphNames[i];
  return (7);
}

int varno_from_name(xgobidata *xg, char *name) {
  int i, varno = -1;

  for(i = 0; i < xg->ncols_used; i++) {
    if(strcmp(name, xg->collab[i]) == 0) {
      varno = i;
      break;
    }
  }
  return varno;

}

/***************** opening files ************************/
FILE *open_file(char *f, char *suffix, char *rw_ind)
{
  FILE *fp = NULL;
  char fname[128];
  struct stat buf;
  Boolean found = false;

  sprintf(fname, "%s%s", f, suffix);

  found = (stat(fname, &buf) == 0);
  if (found) {
    if (S_ISDIR(buf.st_mode)) {
      ;
    } else {
      fp = fopen(fname, rw_ind);
    }
  }

  if (fp != NULL) {
    /*
     * Make sure it isn't an empty file -- get a single character
    */
    int ch = getc(fp);
    if (ch == EOF) {
      fprintf(stderr, "%s is an empty file!\n", fname);
      fclose(fp);
      fp = NULL;
    } else ungetc(ch, fp);
  }

  return fp;
}

FILE *
open_xgobi_file(char *fname, int nsuffixes, char **suffixes,
char *rw, Boolean optional)
{
  FILE *fp = NULL;
  int n;

  if (nsuffixes == 0)
    fp = open_file(fname, "", rw);

  else {
    for (n=0; n<nsuffixes; n++) {
      fp = open_file(fname, (char *) suffixes[n], rw);
      if (fp != NULL) {
        break;
      }
    }
  }

  if (fp == NULL && !optional) {
    char errmsg[512], stmp[16];
    if (nsuffixes > 0) {
      sprintf(errmsg, "Unable to open ");
      sprintf(stmp, " or ");
      for (n=0; n<nsuffixes; n++) {
        if (n == nsuffixes-1)
          sprintf(stmp, ".\n");
        sprintf(errmsg, "%s%s%s%s", errmsg, fname, suffixes[n], stmp);
      }

    } else {
      sprintf(errmsg, "Unable to open %s\n", fname);
    }
    fprintf(stderr, "%s", errmsg);
  }

  return fp;
}
