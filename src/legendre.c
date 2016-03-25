/* legendre.c */
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
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#define SQRT2PI 2.5066282746310007

static float **P0, **P1, **Rp, **Pp0, **Pp1;
static float **acoefs;

/* This index is discussed in "Exploratory Projection Pursuit" by
 * Jerome H. Friedman, JASA 1987
*/

void
alloc_legendre(int n, int maxlJ)
{
  int i;

  P0 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    P0[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  P1 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    P1[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  Rp = (float **) XtMalloc(
    (unsigned int) 2*sizeof(float *));
  for (i=0; i<2; i++)
    Rp[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  Pp0 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    Pp0[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  Pp1 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    Pp1[i] = (float *) XtMalloc(
    (unsigned int) n*sizeof(float));
  acoefs = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    acoefs[i] = (float *) XtMalloc(
      (unsigned int) maxlJ*sizeof(float));
}

void
free_legendre(int maxlJ)
{
  int i;

  for (i=0; i<maxlJ; i++)
    XtFree((XtPointer) P0[i]);
  XtFree((XtPointer) P0);
  for (i=0; i<maxlJ; i++)
    XtFree((XtPointer) P1[i]);
  XtFree((XtPointer) P1);
  for (i=0; i<2; i++)
    XtFree((XtPointer) Rp[i]);
  XtFree((XtPointer) Rp);
  for (i=0; i<maxlJ; i++)
    XtFree((XtPointer) Pp0[i]);
  XtFree((XtPointer) Pp0);
  for (i=0; i<maxlJ; i++)
    XtFree((XtPointer) Pp1[i]);
  XtFree((XtPointer) Pp1);
  for (i=0; i<maxlJ; i++)
    XtFree((XtPointer) acoefs[i]);
  XtFree((XtPointer) acoefs);
}

float
legendre_index(float **proj_data, int n, int *rows_in_plot, int lJ)
{
  int i,j,k;
  float tmpf1, tmpf2;
  float indx_val;

/* calculate R */
  for (i=0; i<n; i++)
  {
    k = rows_in_plot[i];
    tmpf1 = proj_data[0][k];
    tmpf1 /= (sqrt((double) 2.));
    if (tmpf1 >= 0.)
      tmpf2 = (1. + erf(tmpf1)) / 2.;
    else
      tmpf2 = erfc(fabs(tmpf1)) / 2.;
    Rp[0][k] = tmpf2*2. - 1.;
    tmpf1 = proj_data[1][k];
    tmpf1 /= (sqrt((double) 2.));
    if (tmpf1 >= 0.)
      tmpf2 = (1. + erf(tmpf1)) / 2.;
    else
      tmpf2 = erfc(fabs(tmpf1)) / 2.;
    Rp[1][k] = tmpf2*2. - 1.;
  }

/* calculate P's */
  for (i=0; i<n; i++)
  {
    k = rows_in_plot[i];
    P0[0][k] = Rp[0][k];
    P1[0][k] = Rp[1][k];
  }
  if (lJ > 1)
  {
    for (i=0; i<n; i++)
    {
      k = rows_in_plot[i];
      P0[1][k] = (3. * Rp[0][k] * Rp[0][k] - 1.) / 2.;
      P1[1][k] = (3. * Rp[1][k] * Rp[1][k] - 1.) / 2.;
    }
    for (i=2; i<lJ; i++)
    {
      for (j=0; j<n; j++)
      {
        k = rows_in_plot[j];
        P0[i][k] = ((2.*(i+1)-1.)*Rp[0][k]*P0[i-1][k]
          -(i*P0[i-2][k]))/(i+1);
        P1[i][k] = ((2.*(i+1)-1.)*Rp[1][k]*P1[i-1][k]
          -(i*P1[i-2][k]))/(i+1);
      }
    }
  }

/* calculate PP index */
  indx_val = 0.;
  tmpf1 = 0.;
  tmpf2 = 0.;
  for (i=0; i<lJ; i++)
  {
    tmpf1 = mean_fn(P0[i], n, rows_in_plot);
    tmpf2 = mean_fn(P1[i], n, rows_in_plot);
    tmpf1 *= tmpf1;
    tmpf2 *= tmpf2;
    indx_val += (tmpf1 * (2.*(i+1)+1.) / 4.);
    indx_val += (tmpf2 * (2.*(i+1)+1.) / 4.);
  }

  tmpf1 = 0.;
  for (i=0; i<lJ; i++)
  {
    for (j=0; j<(lJ-i-1); j++)
    {
      acoefs[i][j] = mean_fn2(P0[i], P1[j],
        n, rows_in_plot);
      tmpf1 = (acoefs[i][j]*acoefs[i][j]*(2.*(i+1)+1.)*(2.*(j+1)+1.)/4.);
      indx_val += tmpf1;
    }
  }
  return(indx_val);
}

void
legendre_deriv(float **data, float **proj_data, float *alpha, float *beta,
float **derivs, int n, int *rows_in_plot, int p, int *active_vars,
int nactive, int lJ)
{
  int i, j, k, l, m;
  float tmpf1, tmpf2, tmpf3;

/* calculate Pp's */
  for (i=0; i<n; i++)
  {
    Pp0[0][rows_in_plot[i]] = 0;
    Pp0[1][rows_in_plot[i]] = 1;
  }
  if (lJ > 1)
  {
    for (i=0; i<n; i++)
    {
      Pp1[0][rows_in_plot[i]] = 0;
      Pp1[1][rows_in_plot[i]] = 1;
    }
    for (i=2; i<lJ; i++)
    {
      for (j=0; j<n; j++)
      {
        m = rows_in_plot[j];
        Pp0[i][m] = Rp[0][m]*Pp0[i-1][m] + (i+1)*P0[i-1][m];
        Pp1[i][m] = Rp[1][m]*Pp1[i-1][m] + (i+1)*P1[i-1][m];
      }
    }
  }
/* calculate derivatives */
/* alpha */
  for (k=0; k<p; k++)
    derivs[0][k] = 0.;

  for (k=0; k<nactive; k++)
  {
    for (i=0; i<lJ; i++)
    {
      tmpf2 = 0.;
      for (j=0; j<n; j++)
      {
        m = rows_in_plot[j];
        tmpf3 = data[m][active_vars[k]];
        tmpf2 += (Pp0[i][m]*exp(-proj_data[0][m] * proj_data[0][m]/2.)*
             (tmpf3 -
             alpha[active_vars[k]] * proj_data[0][m] -
             beta[active_vars[k]] * proj_data[1][m]));
      }
      tmpf1 = mean_fn(P0[i], n, rows_in_plot);
      tmpf2 /= ((float)n);
      derivs[0][active_vars[k]] += ((2.*(i+1)+1.)*tmpf1*tmpf2);
    }
    for (i=0; i<lJ; i++)
    {
      for (j=0; j<(lJ-i-1); j++)
      {
        tmpf2 =0.;
        for (l=0; l<n; l++)
        {
          m = rows_in_plot[l];
          tmpf3 = data[m][active_vars[k]];
          tmpf2 += (Pp0[i][m]*P1[j][m] *
            exp(-proj_data[0][m]*proj_data[0][m]/2.)*
            (tmpf3 -
            alpha[active_vars[k]]*proj_data[0][m] -
            beta[active_vars[k]]*proj_data[1][m]));
        }
        tmpf2 /= ((float)n);
        derivs[0][active_vars[k]] += ((2.*(i+1)+1)*
          (2.*(j+1)+1)*acoefs[i][j]*tmpf2);
      }
    }
    derivs[0][active_vars[k]] /= ((float)SQRT2PI);
  }
/* beta */
  for (k=0; k<p; k++)
    derivs[1][k] = 0.;

  for (k=0; k<nactive; k++) {
    for (i=0; i<lJ; i++) {
      tmpf2 = 0.;
      for (j=0; j<n; j++) {
        m = rows_in_plot[j];
        tmpf3 = data[m][active_vars[k]];
        tmpf2 += (Pp1[i][m]*exp(-proj_data[1][m]*proj_data[1][m]/2.)*
          (tmpf3 -
          alpha[active_vars[k]]*proj_data[0][m]
          - beta[active_vars[k]]*proj_data[1][m]));
      }
      tmpf1 = mean_fn(P1[i], n, rows_in_plot);
      tmpf2 /= ((float)n);
      derivs[1][active_vars[k]] += ((2.*(i+1)+1.)*tmpf1*tmpf2);
    }
    for (i=0; i<lJ; i++)
      for (j=0; j<(lJ-i-1); j++) {
        tmpf2 =0.;
        for (l=0; l<n; l++) {
          m = rows_in_plot[l];
          tmpf3 = data[m][active_vars[k]];
          tmpf2 += (P0[i][m]*Pp1[j][m] *
            exp(-proj_data[1][m]*proj_data[1][m]/2.)*
            (tmpf3 -
             alpha[active_vars[k]]*proj_data[0][m] -
             beta[active_vars[k]]*proj_data[1][m]));
        }
        tmpf2 /= ((float)n);
        derivs[1][active_vars[k]] += ((2.*(i+1)+1)*
          (2.*(j+1)+1)*acoefs[i][j]*tmpf2);
      }
    derivs[1][active_vars[k]] /= ((float)SQRT2PI);
  }
}

