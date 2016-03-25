/* central_mass.c */
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

#define EXPMINUS1 0.3678794411714423
#define ONEMINUSEXPMINUS1 0.63212056

static float *h0, *h1;
static float acoefs;

void
alloc_central_mass(int n)
{
  h0 = (float *) XtMalloc((unsigned int) n*sizeof(float *));
  h1 = (float *) XtMalloc((unsigned int) n*sizeof(float *));
}

void
free_central_mass()
{
  XtFree((XtPointer) h0);
  XtFree((XtPointer) h1);
}

float
central_mass_index(float **proj_data, int n, int *rows_in_plot)
{
  int i, m;
  float indx_val;

/* Calculate coefficients */
  for (i=0; i<n; i++)
  {
    m = rows_in_plot[i];
    h0[m] = exp(-proj_data[0][m]*proj_data[0][m]/2.) ;
    h1[m] = exp(-proj_data[1][m]*proj_data[1][m]/2.) ;
  }

/* Calculate index */
  acoefs = mean_fn2(h0,h1,n,rows_in_plot);
  indx_val = (acoefs - (float)EXPMINUS1)/(float)ONEMINUSEXPMINUS1 ;
  return(indx_val);
}

void
central_mass_deriv(float **data, float **proj_data, float *alpha, float *beta,
float **derivs, int n, int *rows_in_plot, int p, int nactive, int *active_vars)
{
  int i, k, m;
  float tmpf;

  for (i=0; i<2; i++)
    for (k=0; k<p; k++)
      derivs[i][k] = 0.;

/* alpha */
  for (k=0; k<nactive; k++)
  {
    tmpf = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf -= (proj_data[0][m]*h0[m]*h1[m]*
        (data[m][active_vars[k]] -
        alpha[active_vars[k]]*proj_data[0][m] -
        beta[active_vars[k]]*proj_data[1][m]));
    }
    tmpf /= ((float)n);
    derivs[0][active_vars[k]] = tmpf;
  }

/* beta */
  for (k=0; k<nactive; k++)
  {
    tmpf = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf -= (proj_data[1][m]*h0[m]*h1[m]*
        (data[m][active_vars[k]] -
        alpha[active_vars[k]]*proj_data[0][m] -
        beta[active_vars[k]]*proj_data[1][m]));
    }
    tmpf /= ((float)n);
    derivs[1][active_vars[k]] = tmpf;
  }

}

#undef EXPMINUS1
#undef ONEMINUSEXPMINUS1
