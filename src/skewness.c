/* skewness.c */
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

#define QUROOTPI 1.3313353638003898
#define SQROOTPI 1.7724538509055159
#define ONEON4PI 0.079577471545947673

static float *h0, *h1, *norm0, *norm1;
static float acoefs[2];

void
alloc_skewness(int n)
{
  h0 = (float *) XtMalloc(
    (unsigned int) n*sizeof(float));
  h1 = (float *) XtMalloc(
    (unsigned int) n*sizeof(float));
  norm0 = (float *) XtMalloc(
    (unsigned int) n*sizeof(float));
  norm1 = (float *) XtMalloc(
    (unsigned int) n*sizeof(float));
}

void
free_skewness()
{
  XtFree((XtPointer) h0);
  XtFree((XtPointer) h1);
  XtFree((XtPointer) norm0);
  XtFree((XtPointer) norm1);
}

float
skewness_index(float **proj_data, int n, int *rows_in_plot)
{
  int i, m;
  float indx_val;

/* Calculate Hermite polynomials */
  for (i=0; i<n; i++)
  {
    m = rows_in_plot[i];
    norm0[m] = exp(-proj_data[0][m]*proj_data[0][m]/2.);
    norm1[m] = exp(-proj_data[1][m]*proj_data[1][m]/2.);
    h0[m] = proj_data[0][m]*norm0[m];
    h1[m] = proj_data[1][m]*norm1[m];
  }

/* Calculate index */
  acoefs[0] = mean_fn2(norm0,h1,n,rows_in_plot);
  acoefs[1] = mean_fn2(h0,norm1,n,rows_in_plot);
  indx_val = acoefs[0]*acoefs[0]+acoefs[1]*acoefs[1];
  return(indx_val);
}

void
skewness_deriv(float **data, float **proj_data, float *alpha, float *beta,
float **derivs, int n, int *rows_in_plot, int p, int nactive, int *active_vars)
{
  /*int i, j, k, l, m;*/
  int i, k, m;
  float tmpf1, tmpf2, tmpf3;

  for (i=0; i<2; i++)
    for (k=0; k<p; k++)
      derivs[i][k] = 0.;

/* alpha */
  for (k=0; k<nactive; k++)
  {
    tmpf1 = 0.;
    tmpf2 = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf3 = data[m][active_vars[k]] -
        alpha[active_vars[k]]*proj_data[0][m] -
        beta[active_vars[k]]*proj_data[1][m];
      tmpf1 -= ((h0[m]*h1[m])*tmpf3);
      tmpf2 += ((norm1[m]*norm0[m])*
        (1.-proj_data[0][m]*proj_data[0][m])*tmpf3);
    }
    tmpf1 /= ((float)n);
    tmpf2 /= ((float)n);
    derivs[0][active_vars[k]] = 2.*(acoefs[0]*tmpf1+acoefs[1]*tmpf2) ;
  }

/* beta */
  for (k=0; k<nactive; k++)
  {
    tmpf1 = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf2 += ((norm0[m]*norm1[m])*(1.-proj_data[1][m]*proj_data[1][m])*
        (data[m][active_vars[k]] -
        alpha[active_vars[k]]*proj_data[0][m] -
        beta[active_vars[k]]*proj_data[1][m]));
    }
    tmpf2 /= ((float)n);
    derivs[1][active_vars[k]] = 2.*(acoefs[0]*tmpf2+acoefs[1]*tmpf1);
  }
}

