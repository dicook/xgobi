/* pipeline.c */
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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#define FUDGE_FACTOR (xg->numvars_t+1)*2/7

/* ------------ Dynamic allocation, freeing section --------- */

void
alloc_pipeline_arrays(xgobidata *xg)
/*
 * Dynamically allocate arrays.
*/
{
  Cardinal nc = (Cardinal) xg->ncols, nr = (Cardinal) xg->nrows;
  register Cardinal i;

  xg->sphered_data = (float **) XtMalloc(nr*sizeof(float *));
  for (i=0; i<nr; i++)
    xg->sphered_data[i] = (float *)XtMalloc(nc * sizeof(float));

  xg->tform1 = (float **) XtMalloc(nr * sizeof(float *));
  for (i=0; i<nr; i++)
    xg->tform1[i] = (float *) XtMalloc(nc * sizeof(float));
  xg->tform2 = (float **) XtMalloc(nr * sizeof(float *));
  for (i=0; i<nr; i++)
    xg->tform2[i] = (float *) XtMalloc(nc * sizeof(float));

  xg->world_data = (long **) XtMalloc(nr * sizeof(long *));
  for (i=0; i<nr; i++)
    xg->world_data[i] = (long *) XtMalloc(nc * sizeof(long));

  /* XtCalloc initializes to zero */
  xg->jitter_data = (long **) XtMalloc(nr * sizeof(long *));
  for (i=0; i<nr; i++)
    xg->jitter_data[i] = (long *) XtCalloc(nc, sizeof(long));

  xg->planar = (lcoords *) XtMalloc(nr * sizeof(lcoords));
  xg->screen = (icoords *) XtMalloc(nr * sizeof(icoords));

  xg->rows_in_plot = (int *) XtMalloc(
    (Cardinal) xg->nrows * sizeof(int));

  xg->lim0 = (lims *) XtMalloc(nc * sizeof(lims));
  xg->lim = (lims *) XtMalloc(nc * sizeof(lims));
  xg->lim_tform = (lims *) XtMalloc(nc * sizeof(lims));
  xg->lim_raw = (lims *) XtMalloc(nc * sizeof(lims));
}

void
free_pipeline_arrays(xgobidata *xg)
/*
 * Dynamically free arrays used in data pipeline.
*/
{
  int i;

  for (i=0; i<xg->nrows; i++)
    XtFree((XtPointer) xg->sphered_data[i]);
  XtFree((XtPointer) xg->sphered_data);

  for (i=0; i<xg->nrows; i++)
    XtFree((XtPointer) xg->tform1[i]);
  XtFree((XtPointer) xg->tform1);
  for (i=0; i<xg->nrows; i++)
    XtFree((XtPointer) xg->tform2[i]);
  XtFree((XtPointer) xg->tform2);

  for (i=0; i<xg->nrows; i++)
    XtFree((XtPointer) xg->world_data[i]);
  XtFree((XtPointer) xg->world_data);

  for (i=0; i<xg->nrows; i++)
    XtFree((XtPointer) xg->jitter_data[i]);
  XtFree((XtPointer) xg->jitter_data);

  XtFree((XtPointer) xg->planar);
  XtFree((XtPointer) xg->screen);

  XtFree((XtPointer) xg->rows_in_plot);

  XtFree((XtPointer) xg->lim0);
  XtFree((XtPointer) xg->lim);
  XtFree((XtPointer) xg->lim_tform);
  XtFree((XtPointer) xg->lim_raw);
}

/* ------------ End of dynamic allocation section --------- */

/* ------------ Data pipeline section --------- */

void
copy_raw_to_tform(xgobidata *xg)
{
  int i, j;

  for (i=0; i<xg->nrows; i++)
    for (j=0; j<xg->ncols_used; j++)
      xg->tform1[i][j] = xg->tform2[i][j] = xg->raw_data[i][j];
}

void
copy_raw_to_tform1(xgobidata *xg)
{
  int i, j;

  for (i=0; i<xg->nrows; i++)
    for (j=0; j<xg->ncols_used; j++)
      xg->tform1[i][j] = xg->raw_data[i][j];
}

void
copy_tform1_to_tform2(xgobidata *xg)
{
  int i, j;

  for (j=0; j<xg->ncols_used; j++) {
    (void) strcpy(xg->collab_tform2[j], xg->collab_tform1[j]);
    for (i=0; i<xg->nrows; i++) {
      xg->tform2[i][j] = xg->tform1[i][j];
    }
  }
}

void
copy_tform_to_sphered(xgobidata *xg)
{
/*
 * Copy tform1 to sphered_data.
*/
  int i, j;

  for (i=0; i<xg->nrows; i++)
    for (j=0; j<xg->ncols_used; j++)
      xg->sphered_data[i][j] = xg->tform1[i][j];  /* ?? */
}

/*
 * This is called in three places:  when a transformation is done,
 * when the dummy variable is changed by brushing, and when points
 * are deleted.  In the transformation case, only the columns in
 * the current variable group are affected; in the brushing case,
 * only the last variable is affected.  When points are deleted,
 * all variables are affected, and everything needs to be done.
 * Let this be the kernel of a routine that can be called whenever
 * we need to push the data pipeline.
*/
void
update_sphered(xgobidata *xg, int *cols, int ncols)
{
  /*
   * Update variance-covariance matrix
  */
  int j;

  if (ncols == xg->ncols_used && xg->ncols_used > 2)
    for (j=0; j<xg->ncols_used; j++)
      recalc_vc(j, xg);
  else
    for (j=0; j<ncols; j++)
      recalc_vc(cols[j], xg);
  /*
   * Update sphered_data[]
  */
  if (xg->is_princ_comp)
  {
    if (update_vc_active_and_do_svd(xg, xg->numvars_t, xg->tour_vars))
      spherize_data(xg, xg->numvars_t, xg->numvars_t, xg->tour_vars);
    else
      copy_tform_to_sphered(xg);
  }
}

void
min_max(xgobidata *xg, float **data, int *cols, int ncols,
float *min, float *max)
/*
 * Find the minimum and maximum values of each column or variable
 * group using using the min-max scaling.
*/
{
  int i, j, k, n;
/*
 * Choose an initial value for *min and *max
*/
  *min = *max = data[xg->rows_in_plot[0]][cols[0]];

  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg->nrows_in_plot; i++) {
      k = xg->rows_in_plot[i];
      if (data[k][j] < *min)
        *min = data[k][j];
      else if (data[k][j] > *max)
        *max = data[k][j];
    }
  }
}

int
icompare(int *x1, int *x2)
{
  int val = 0;

  if (*x1 < *x2)
    val = -1;
  else if (*x1 > *x2)
    val = 1;

  return(val);
}

float
median_largest_dist(xgobidata *xg, float **data, int *cols, int ncols,
  float *min, float *max)
{
/*
 * Find the minimum and maximum values of each column or variable
 * group scaling by median and largest distance
*/
  int i, j, k, n, np;
  double dx, sumdist, lgdist = 0.0;
  float *x, fmedian;
  double dmedian = 0;
  extern int fcompare(const void *, const void *);

  np = ncols * xg->nrows_in_plot;
  x = (float *) XtMalloc((Cardinal) np * sizeof(float));
  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg->nrows_in_plot; i++) {
      k = xg->rows_in_plot[i];
      x[n*xg->nrows_in_plot + i] = data[k][j];
    }
  }

  qsort((void *) x, np, sizeof(float), fcompare);
  dmedian = ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
  */

  for (i=0; i<xg->nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<ncols; j++) {
      dx = (double) data[xg->rows_in_plot[i]][cols[j]] - dmedian;
      sumdist += (dx*dx);
    }
    if (sumdist > lgdist)
      lgdist = sumdist;
  }

  lgdist = sqrt(lgdist);

  XtFree((char *) x);
  
  fmedian = (float) dmedian;
  *min = fmedian - lgdist;
  *max = fmedian + lgdist;

  return fmedian;
}

/*
 * This code is also used in the "transpose plots", the logical
 * zooming plots done in identification.
*/
void
adjust_limits(float *min, float *max)
/*
 * This test should be cleverer.  It should test the ratios
 * lim[i].min/rdiff and lim[i].max/rdiff for overflow or
 * rdiff/lim[i].min and rdiff/lim[i].max for underflow.
 * It should probably do it inside a while loop, too.
 * See Advanced C, p 187.  Set up floation point exception
 * handler which alters the values of lim[i].min and lim[i].max
 * until no exceptions occur.
*/
{
  if (*max - *min == 0) {
    if (*min == 0.0) {
      *min = -1.0;
      *max = 1.0;
    } else {
      *min = .9 * *min;
      *max = 1.1 * *max;
    }
  }

  /* This is needed to account for the case that max == min < 0 */
  if (*max < *min) {
    float ftmp = *max;
    *max = *min;
    *min = ftmp;
  }
}

float
mean_largest_dist(xgobidata *xg, float **data, int *cols, int ncols,
  float *min, float *max)
{
/*
 * Find the minimum and maximum values of each column or variable
 * group scaling by mean and largest distance
*/
  int i, j;
  double dx, sumxi, mean, sumdist, lgdist = 0.0;

  /*
   * Find the overall mean for the columns
  */
  sumxi = 0.0;
  for (j=0; j<ncols; j++) {
    for (i=0; i<xg->nrows_in_plot; i++) {
      dx = (double) data[xg->rows_in_plot[i]][cols[j]];
      sumxi += dx;
    }
  }
  mean = sumxi / (double) xg->nrows_in_plot / (double) ncols;

  /*
   * Find the maximum of the sum of squared differences
   * from the mean over all rows
  */

  for (i=0; i<xg->nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<ncols; j++) {
      dx = (double) data[xg->rows_in_plot[i]][cols[j]] - mean;
      sumdist += (dx*dx);
    }
    if (sumdist > lgdist)
      lgdist = sumdist;
  }

  lgdist = sqrt(lgdist);
  
  *min = mean - lgdist;
  *max = mean + lgdist;

  return mean;
}

void
mean_lgdist(xgobidata *xg, float **data)
/*
 * Find the minimum and maximum values of each column or variable
 * group scaling by mean and largest distance.
*/
{
  int i, j, n;
  float min, max;
  double dx;
  double sumxi;
  double sumdist = 0.0;
  double *mean, lgdist = 0.0;

  /*
   * Find the mean for each column
  */
  mean = (double *) XtMalloc((Cardinal) xg->numvars_t * sizeof(double));
  for (j=0; j<xg->numvars_t; j++) {
    sumxi = 0.0;
    for (i=0; i<xg->nrows_in_plot; i++) {
      dx = (double) data[xg->rows_in_plot[i]][xg->tour_vars[j]];
      sumxi += dx;
    }
    mean[j] = sumxi / xg->nrows_in_plot;
  }

  /*
   * Find the maximum of the sum of squared differences
   * from the column mean over all rows
  */
  for (i=0; i<xg->nrows_in_plot; i++) {
    sumdist = 0.0;
    for (j=0; j<xg->numvars_t; j++) {
      dx = (data[xg->rows_in_plot[i]][xg->tour_vars[j]]-mean[j]);
      dx *= dx;
      sumdist += dx;
    }
    if (sumdist > lgdist)
      lgdist = sumdist;
  }

  lgdist = sqrt(lgdist);
  
  for (j=0; j<xg->numvars_t; j++) {
    min = mean[j] - lgdist;
    max = mean[j] + lgdist;
    adjust_limits(&min, &max);
    n = xg->tour_vars[j];
    xg->lim[n].min = xg->lim0[n].min = min;
    xg->lim[n].max = xg->lim0[n].max = max;
  }

  XtFree((XtPointer) mean);
}


/*
 * This only gives the correct result if the
 * vgroups vector is of the form {0,1,2,...,ngroups-1}
*/
int
numvargroups(xgobidata *xg)
{
  int j, nvgroups = 0;

  for (j=0; j<xg->ncols_used; j++)
     if (xg->vgroup_ids[j] > nvgroups)
       nvgroups = xg->vgroup_ids[j];
  nvgroups++;

  return(nvgroups);
}

void
init_lim0(xgobidata *xg)
{
  int j, *cols, ncols;
  float min, max;
  int k;
  int nvgroups = numvargroups(xg);

  cols = (int *) XtMalloc((Cardinal) xg->ncols * sizeof(int));
  for (k=0; k<nvgroups; k++) {
    ncols = 0;
    for (j=0; j<xg->ncols_used; j++) {
      if (xg->vgroup_ids[j] == k)
        cols[ncols++] = j;
    }

    min_max(xg, xg->tform2, cols, ncols, &min, &max);
    adjust_limits(&min, &max);
    for (j=0; j<ncols; j++) {
      xg->lim0[cols[j]].min = min;
      xg->lim0[cols[j]].max = max;
    }

/* All these limits need a thorough review */
    min_max(xg, xg->raw_data, cols, ncols, &min, &max);
    adjust_limits(&min, &max);
    for (j=0; j<ncols; j++) {
      xg->lim_raw[cols[j]].min = min;
      xg->lim_raw[cols[j]].max = max;
    }

  }

  XtFree((XtPointer) cols);
}

void
update_lims(xgobidata *xg)
{
  int j, k, n;
  float min, max;
  int *cols, ncols;
  int nvgroups = numvargroups(xg);

  /* 
   * First update the limits for the tform2.  Then 
   * override lim and lim0 if necessary.
  */
  init_lim0(xg);

  /*
   * Take tform2[][], one variable group at a time, and generate
   * the min and max for each variable group (and thus for each
   * column).
  */
  cols = (int *) XtMalloc((Cardinal) xg->ncols * sizeof(int));
  for (k=0; k<nvgroups; k++) {
    ncols = 0;
    for (j=0; j<xg->ncols_used; j++) {
      if (xg->vgroup_ids[j] == k)
        cols[ncols++] = j;
    }

    switch(xg->std_type)
    {
      case 0:
        min_max(xg, xg->tform2, cols, ncols, &min, &max);
        break;
      case 1:
        mean_largest_dist(xg, xg->tform2, cols, ncols, &min, &max);
        break;
      case 2:
        median_largest_dist(xg, xg->tform2, cols, ncols, &min, &max);
        break;
    }

    adjust_limits(&min, &max);

    for (n=0; n<ncols; n++) {
      xg->lim[cols[n]].min = min;
      xg->lim[cols[n]].max = max;
    }
  }
  XtFree((XtPointer) cols);

/*
 * Set the limits that are based exclusively on tform data;
 * they need to be kept separately for parallel coordinate
 * plots.
*/
  for (j=0; j<xg->ncols_used; j++) {
    xg->lim_tform[j].min = xg->lim[j].min;
    xg->lim_tform[j].max = xg->lim[j].max;
  }
}


void
tform_to_world(xgobidata *xg)
{
/*
 * Take tform2[][], one column at a time, and generate
 * world_data[]
*/
  int i, j, m;
  float rdiff, ftmp;
  float precis = PRECISION1;  /* 32768 */

  for (j=0; j<xg->ncols_used; j++) {
    rdiff = xg->lim[j].max - xg->lim[j].min;
    for (i=0; i<xg->nrows_in_plot; i++) {
      m = xg->rows_in_plot[i];
      ftmp = -1.0 + 2.0*(xg->tform2[m][j] - xg->lim[j].min)/rdiff;
      xg->world_data[m][j] = (long) (precis * ftmp);

      /* Add in the jitter values */
      xg->world_data[m][j] += xg->jitter_data[m][j];
    }
  }
}

static void
sphered_to_world(xgobidata *xg)
{
/*
 * Take sphered_data[], one column at a time, and generate
 * world_data[]
*/
  int i, j, k, m;
  float rdiff, ftmp;
  float precis = PRECISION1;

  for (m=0; m<xg->numvars_t; m++) {
    j = xg->tour_vars[m];
    rdiff = xg->lim[j].max - xg->lim[j].min;
    for (i=0; i<xg->nrows_in_plot; i++) {
      k = xg->rows_in_plot[i];
      ftmp = -1.0 + 2.0*(xg->sphered_data[k][j] - xg->lim[j].min)/rdiff;
      xg->world_data[k][j] = (long) (precis * ftmp);

      /* Add in the jitter values */
      xg->world_data[k][j] += xg->jitter_data[k][j];
    }
  }
}

void
update_world(xgobidata *xg)
{
/*
 * Keep world_data[] up to date.
*/
/*  if (xg->is_princ_comp && xg->is_touring)
    sphered_to_world(xg);
  else*//*sphere*/
    tform_to_world(xg);

  /*
   * span_planes() operates on world_data[], so it is called
   * when world_data[] changes.
  */
  if (xg->is_touring)
    span_planes(xg);
  if (xg->is_corr_touring)
    corr_span_planes(xg);
}

void
world_to_plane(xgobidata *xg)
/*
 * Using the current view, project the data from world_data[],
 * the data expressed in 'world coordinates,' to planar[], the
 * data expressed in 'projection coordinates.'
*/
{
  if (xg->is_touring)
    all_tour_reproject(xg);
  else if (xg->is_corr_touring)
    corr_reproject(xg);
  else if (xg->is_spinning)
  {
    if (xg->is_spin_type.yaxis || xg->is_spin_type.xaxis)
      ax_rot_reproject(xg);
    else if (xg->is_spin_type.oblique)
      ob_rot_reproject(xg);
  }
  else if (xg->is_xyplotting)
    xy_reproject(xg);

  else if (xg->is_plotting1d)
    plot1d_reproject(xg);
}

void
plane_to_screen(xgobidata *xg)
/*
 * Use the data in 'projection coordinates' and rescale it to the
 * dimensions of the current plotting window, writing it into screen.
 * At the same time, update segs.
*/
{
  int j, k;
  long nx, ny;
  float scale_x = xg->scale.x / 2;
  float scale_y = xg->scale.y / 2;

  /*
   * Calculate is, a scale factor.  Either force the plot to be
   * square or scale so as to use the entire plot window.  (Or
   * as much of the plot window as scale.x and scale.y permit.)
  */
  xg->is.x = (xg->plot_square) ?
    (long) (xg->minxy * scale_x) :
    (long) (xg->max.x * scale_x);
  xg->is.y = (xg->plot_square) ?
    (long) (-1 * xg->minxy * scale_y) :
    (long) (-1 * xg->max.y * scale_y);

  /*
   * Calculate new coordinates.
  */
  for (k=0; k<xg->nrows_in_plot; k++) {
    j = xg->rows_in_plot[k];

    /*
     * shift in world coords
    */
    nx = xg->planar[j].x + xg->shift_wrld.x;
    ny = xg->planar[j].y - xg->shift_wrld.y;

    /*
     * scale from world to plot window and expand-contract as desired
    */
    xg->screen[j].x = (int) ((nx * xg->is.x) >> EXP1);
    xg->screen[j].y = (int) ((ny * xg->is.y) >> EXP1);

    /*
     * shift into middle of plot window 
    */
    xg->screen[j].x += xg->mid.x;
    xg->screen[j].y += xg->mid.y;
  }
}

/*********************** Reverse pipeline ***********************/

void
screen_to_plane(xgobidata *xg, int pt, icoords *eps,
  Boolean horiz, Boolean vert)
{
  icoords prev_planar;
  find_plot_center(xg);

  /* fixes the hor/vert movement problem; AB July 18 2001
   * but still doesn't work right in corr tour 
   * when the same variable is used for both vertical and horizontal manip */
  eps->x = 0;  eps->y = 0;  

  if (horiz) {
    xg->screen[pt].x -= xg->cntr.x;
    prev_planar.x = xg->planar[pt].x;
    xg->planar[pt].x = xg->screen[pt].x * PRECISION1 / xg->is.x ;
    eps->x = xg->planar[pt].x - prev_planar.x;
  }

  if (vert) {
    xg->screen[pt].y -= xg->cntr.y;
    prev_planar.y = xg->planar[pt].y;
    xg->planar[pt].y = xg->screen[pt].y * PRECISION1 / xg->is.y ;
    eps->y = xg->planar[pt].y - prev_planar.y;
  }
}

void
plane_to_world(xgobidata *xg, int pt, lcoords *eps)
{
  int j, var; 

  if (xg->is_xyplotting) {

    xg->world_data[pt][xg->xy_vars.x] = xg->planar[pt].x;
    xg->world_data[pt][xg->xy_vars.y] = xg->planar[pt].y;

  } else if (xg->is_spinning) {

    long nx, ny, nz;
    long lx, ly, lz;
    int ix = xg->spin_vars.x;
    int iy = xg->spin_vars.y;
    int iz = xg->spin_vars.z;

    nx = xg->planar[pt].x;
    ny = xg->planar[pt].y;

    if (xg->is_spin_type.oblique) {
      extern long lRmat[3][3];

      /*
       * Use the new values of x and y, but reconstruct
       * what the old value of nz would have been, had we
       * calculated it in ob_rot_reproject.
      */
      nz =
        lRmat[2][0] * xg->world_data[pt][ix] +
        lRmat[2][1] * xg->world_data[pt][iy] +
        lRmat[2][2] * xg->world_data[pt][iz];
      nz /= PRECISION2;

      /*
       * Calculate the inner product of t(lRmat) and (nx, ny, nz)
       * (This works because the transpose of lRmat is its inverse.)
      */ 
      lx = lRmat[0][0] * nx + lRmat[1][0] * ny + lRmat[2][0] * nz;
      ly = lRmat[0][1] * nx + lRmat[1][1] * ny + lRmat[2][1] * nz;
      lz = lRmat[0][2] * nx + lRmat[1][2] * ny + lRmat[2][2] * nz;

      xg->world_data[pt][ix] = lx / PRECISION2;
      xg->world_data[pt][iy] = ly / PRECISION2;
      xg->world_data[pt][iz] = lz / PRECISION2;
    }
    else if (xg->is_spin_type.yaxis) {

      /* nz isn't normally used or calculated; do it here */
      nz = -xg->isint.y * xg->world_data[pt][ix] +
        xg->icost.y * xg->world_data[pt][iz];
      nz /= PRECISION2;

      /*
       * Calculate the inner product of the transpose of the
       * rotation matrix and (nx, ny, nz)
      */ 
      lx = xg->icost.y * nx + -xg->isint.y * nz;
      ly = ny;
      lz = xg->isint.y * nx + xg->icost.y * nz;

      xg->world_data[pt][ix] = lx / PRECISION2;
      xg->world_data[pt][iy] = ly;
      xg->world_data[pt][iz] = lz / PRECISION2;
    }
    else if (xg->is_spin_type.xaxis) {

      /* nz isn't normally used or calculated; do it here */
      nz = -xg->isint.x * xg->world_data[pt][iy] +
        xg->icost.x * xg->world_data[pt][iz];
      nz /= PRECISION2;

      /*
       * Calculate the inner product of the transpose of the
       * rotation matrix and (nx, ny, nz)
      */ 
      lx = nx;
      ly = xg->icost.x * ny - xg->isint.x * nz;
      lz = xg->isint.x * ny + xg->icost.x * nz;

      xg->world_data[pt][ix] = lx;
      xg->world_data[pt][iy] = ly / PRECISION2;
      xg->world_data[pt][iz] = lz / PRECISION2;
    }
  } 
  else if (xg->is_touring && !xg->is_pp) {
    for (j=0; j<xg->numvars_t; j++) {
      var = xg->tour_vars[j];
      xg->world_data[pt][var] += 
       ((float)eps->x * xg->u[0][var] + (float)eps->y * xg->u[1][var]);
    }
  }
  else if (xg->is_corr_touring && !xg->is_corr_pursuit) {
    for (j=0; j<xg->ncorr_xvars; j++) {
      var = xg->corr_xvars[j];
      xg->world_data[pt][var] += 
       ((float)eps->x * xg->cu[0][var] + (float)eps->y * xg->cu[1][var]);
    }
    for (j=0; j<xg->ncorr_yvars; j++) {
      var = xg->corr_yvars[j];
      xg->world_data[pt][var] += 
       ((float)eps->x * xg->cu[0][var] + (float)eps->y * xg->cu[1][var]);
    }
  }
}

/*
 * world_data includes jitter_data
*/

void
world_to_raw_by_var(xgobidata *xg, int pt, int var)
{
  float precis = PRECISION1;
  float ftmp, rdiff;

  rdiff = xg->lim[var].max - xg->lim[var].min;

  /*
   * The new world_data value is taken to include the
   * value of jitter_data
  */
  ftmp = (float)(xg->world_data[pt][var] - xg->jitter_data[pt][var]) / precis;

  xg->tform2[pt][var] = (ftmp + 1.0) * .5 * rdiff;
  xg->tform2[pt][var] += xg->lim[var].min;

/*
 * Do a reverse transform of the tform data point and send
 * that back to the raw data array.
*/
  xg->raw_data[pt][var] = inv_transform(pt, var, xg);
}

void
world_to_raw(xgobidata *xg, int pt)
{
  int i;

  if (xg->is_xyplotting) {
    world_to_raw_by_var(xg, pt, xg->xy_vars.x);
    world_to_raw_by_var(xg, pt, xg->xy_vars.y);
  } else if (xg->is_spinning) {
    world_to_raw_by_var(xg, pt, xg->spin_vars.x);
    world_to_raw_by_var(xg, pt, xg->spin_vars.y);
    world_to_raw_by_var(xg, pt, xg->spin_vars.z);
  } else if (xg->is_touring) {
    for (i=0; i<xg->numvars_t; i++)
      world_to_raw_by_var(xg, pt, xg->tour_vars[i]);
  } else if (xg->is_corr_touring) {
    /* something like this, I guess */
    for (i=0; i<xg->ncorr_xvars; i++)
      world_to_raw_by_var(xg, pt, xg->corr_xvars[i]);
    for (i=0; i<xg->ncorr_yvars; i++)
      world_to_raw_by_var(xg, pt, xg->corr_yvars[i]);
  }
}

/******************* End of reverse pipeline *******************/
