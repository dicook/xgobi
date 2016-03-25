/* tour_util.c */
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
#include <limits.h>
#include <float.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

void
copy_basis(float **source, float **target, int n)
{
  int i, j;

  for (i=0; i<2; i++)
    for (j=0; j<n; j++)
      target[i][j] = source[i][j];
}

void
copy_vector(float *source, float *target, int n)
{
  int j;

  for (j=0; j<n; j++)
    target[j] = source[j];
}

void
norm(float *x, int n)
{
  int j;
  double xn = 0;

  for (j=0; j<n; j++)
    xn = xn + DOUBLE(x[j]*x[j]);
  xn = sqrt(xn);
  for (j=0; j<n; j++)
    x[j] = x[j]/FLOAT(xn);
}

float
calc_norm(float *x, int n)
{
  int j;
  double xn = 0;

  for (j=0; j<n; j++)
    xn = xn + DOUBLE(x[j]*x[j]);
  xn = sqrt(xn);

  return((float)xn);
}

float
calc_norm_sq(float *x, int n)
{
  int j;
  double xn = 0;

  for (j=0; j<n; j++)
    xn = xn + DOUBLE(x[j]*x[j]);

  return((float)xn);
}

float
inner_prod(float *x1, float *x2, int n)
{
  double xip;
  int j;

  xip = 0.;
  for (j=0; j<n; j++)
    xip = xip + (double)x1[j]*(double)x2[j];
  return((float)xip);
}

float
mean_fn(float *x1, int n, int *rows_in_plot)
{
  int i;
  float tmpf;
  float mean1;

  tmpf = 0.;
  for (i=0; i<n; i++)
    tmpf += (x1[rows_in_plot[i]]-x1[0]);
  mean1 = tmpf / (float)n;
  mean1 += x1[0];

  return(mean1);
}

float
mean_fn2(float *x1, float *x2, int n, int *rows_in_plot)
{
  int i;
  float tmean, tmpf1;
  float mean1, mean2;

  tmpf1 = 0.;
  for (i=0; i<n; i++)
    tmpf1 += x1[rows_in_plot[i]];
  mean1 = tmpf1 / (float)n;
  tmpf1 = 0.;
  for (i=0; i<n; i++)
    tmpf1 += x2[rows_in_plot[i]];
  mean2 = tmpf1 / (float)n;
  tmean = 0.;
  for (i=0; i<n; i++) {
    tmean += ((x1[rows_in_plot[i]]-mean1)*(x2[rows_in_plot[i]]-mean2));
  }
  tmean /= ((float)n);
  tmean += (mean1*mean2);

  return(tmean);
}

float
mean_fn3(float *x1, float *x2, float *wgts, int n, int *rows_in_plot)
{
  int i;
  float tmean, tmpf1;
  float mean1, mean2;

  tmpf1 = 0.;
  for (i=0; i<n; i++)
    tmpf1 += x1[rows_in_plot[i]];
  mean1 = tmpf1 / (float)n;
  tmpf1 = 0.;
  for (i=0; i<n; i++)
    tmpf1 += x2[rows_in_plot[i]];
  mean2 = tmpf1 / (float)n;

  tmean = 0.;
  for (i=0; i<n; i++) {
    tmean += ((x1[rows_in_plot[i]]-mean1)*(x2[rows_in_plot[i]]-mean2)*
      wgts[rows_in_plot[i]]);
  }
  tmean /= ((float)n);
  tmean += (mean1*mean2);

  return(tmean);
}

float
mean_fn4(float *x1, float *x2, float *x3, float *x4, int n, int *rows_in_plot)
{
  int i;
  float tmean;

  tmean = 0.;
  for (i=0; i<n; i++) {
    tmean += (x1[rows_in_plot[i]]*x2[rows_in_plot[i]]*
      x3[rows_in_plot[i]]*x4[rows_in_plot[i]]);
  }
  tmean /= ((float)n);

  return(tmean);
}

float
min(float *x1, int n)
{
  int i;
  float tmpf1;

  tmpf1 = 1000.;
  for (i=0; i<n; i++)
    if (x1[i] < tmpf1)
      tmpf1 = x1[i];

  return(tmpf1);
}

float
max(float *x1, int n)
{
  int i;
  float tmpf1;

  tmpf1 = -1000.;
  for (i=0; i<n; i++)
    if (x1[i] > tmpf1)
      tmpf1 = x1[i];

  return(tmpf1);
}

float
sq_inner_prod(float *x1, float *x2, int n)
{
  float xip;
  int j;

  xip = 0;
  for (j=0; j<n; j++)
    xip = xip + x1[j]*x2[j];
  return(xip*xip);
}

void
gram_schmidt(float *x1, float *x2, int n)
{
  int j;
  float ip;

  ip = inner_prod(x1, x2, n);
  for (j=0; j<n; j++)
    x2[j] = x2[j] - ip*x1[j];

  norm(x2, n);
}

int
check_proximity(float **base1, float **base2, int n)
/*
 * This routine checks if two bases are close to identical
*/
{
  float diff, ssq1 = 0.0, ssq2 = 0.0;
  int i, indic = 0;
  float tol = 0.001;

  for (i=0; i<n; i++)
  {
    diff = base1[0][i] - base2[0][i];
    ssq1 = ssq1 + (diff * diff);

    diff = base1[1][i] - base2[1][i];
    ssq2 = ssq2 + (diff * diff);
  }

  if ((ssq1 < tol) && (ssq2 < tol))
    indic = 1;

  return(indic);
}

void
AllocBox(xgobidata *xg)
/*
 * This routine allocates space for a new node if there are no unused nodes
 * on the free list, fl.
*/
{
  if (xg->fl == NULL)
  {
    xg->curr = (hist_rec *) XtMalloc(
      (unsigned int) sizeof(hist_rec));
    xg->curr->hist[0] = (float *) XtMalloc(
      (unsigned int) xg->ncols * sizeof(float));
    xg->curr->hist[1] = (float *) XtMalloc(
      (unsigned int) xg->ncols * sizeof(float));
  }
  else
  {
    xg->curr = xg->fl;
    xg->fl = xg->fl->prev;
  }
}

void
check_deselection(int varno, xgobidata *xg)
{
  int i, done;

  if (xg->u0[0][varno] == 1.0)
  {
    done = 0;
    i = 0;
    while (!done && (i <= xg->numvars_t))
    {
      if ((xg->u0[1][xg->tour_vars[i]] != 1.0) &&
        (xg->tour_vars[i] != varno))
        {
          xg->u0[0][varno] = 0.0;
          xg->u0[0][xg->tour_vars[i]] = 1.0;
          copy_u0_to_pd0(xg);
          change_first_projection(0, i, xg);
          done = 1;
      }
      i++;
    }
  }
  else if (xg->u0[1][varno] == 1.0)
  {
    done = 0;
    i = 0;
    while (!done && (i <= xg->numvars_t))
    {
      if ((xg->u0[0][xg->tour_vars[i]] != 1.0) &&
        (xg->tour_vars[i] != varno))
        {
          xg->u0[1][varno] = 0.0;
          xg->u0[1][xg->tour_vars[i]] = 1.0;
          copy_u0_to_pd0(xg);
          change_first_projection(1, i, xg);
          done = 1;
      }
      i++;
    }
  }
  copy_basis(xg->u0, xg->u, xg->ncols_used);
  copy_basis(xg->u0, xg->u1, xg->ncols_used);
  copy_u1_to_pd1(xg);
  world_to_plane(xg);
  plane_to_screen(xg);
  plot_once(xg);
  tour_var_lines(xg);

}

void
copy_matrix(float **M1, float **M2, int n, int p)
{
  int i, j;

  for (i=0; i<n; i++)
    for (j=0; j<p; j++)
      M2[i][j] = M1[i][j];

}

void
matrix_mult(float **M1, float **M2, float **M3, int n, int p, int m)
{
  int i, j, k;
  float tmpf;

  for (i=0; i<n; i++)
    for (j=0; j<m; j++)
    {
      tmpf = 0.;
      for (k=0; k<p; k++)
        tmpf += (M1[i][k]*M2[k][j]);
      M3[i][j] = tmpf;
    }
}

void
gen_unif_variates(int n, int p, float *vars, float radius)
{
  int i, check=1;
  double frunif[2];
  double r, frnorm[2];

  for (i=0; i<(n*p+1)/2; i++) {
    while (check) {

      rnorm2(&frunif[0], &frunif[1]);
      r = frunif[0] * frunif[0] + frunif[1] * frunif[1];

      if (r < 1)
      {
        check = 0;
        frnorm[0] = frunif[0] * radius;
        frnorm[1] = frunif[1] * radius;
      }
    }
    check = 1;
    vars[2*i] = FLOAT(frnorm[0]);
    vars[2*i+1] = FLOAT(frnorm[1]);
  }
}
