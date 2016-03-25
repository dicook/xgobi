/* kernel.c */
/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  this  code and associated documentation,  provided      *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *    Copyright (c) 1994 Sigbert Klinke                     *
 *                       <sigbert@wiwi.hu-berlin.de>        *
 *  All Rights Reserved.                                    *
 *                                                          *
 ************************************************************/

#include <math.h>
#include <stdlib.h>

#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

static int *bin, *sort;

/*static float window_width_const = 3.12;*/

void
alloc_bin_fts(int n)
{
  bin  = (int *) XtMalloc((Cardinal) (2 * n * sizeof(int)));
  sort = (int *) XtMalloc((Cardinal) (n * sizeof(int)));
}

void
free_bin_fts()
{
  XtFree((XtPointer) bin);
  XtFree((XtPointer) sort);
}

#define BINWIDTH   0.01

int
compare1(const void *val1, const void *val2)
{
  register int *left = (int *) val1;
  register int *right = (int *) val2;

  if (*(bin+2* *left)<*(bin+2* *right)) return (-1);
  if (*(bin+2* *left)>*(bin+2* *right)) return (1);
  return  (0);
}

float
bin_fts_index(float **proj_data, int n, int *rows_in_plot, float bandwidth)
/* Calculation of

   \sum_{i=1}^{n} \sum{j=1}^{i-1} K_h((x_i - x_j)/h, (y_i-y_j)/h)
   
*/

{
  float indx_val, tmpf, hf;
  int i, j, m, start, ki, kj;
  long dist, tmpl, h;
/*   float window_width3; */

/* Binning */
  for (i=0; i<n; i++)
  { m = rows_in_plot[i];
    *(bin+2*i)   = floor(proj_data[0][m] / BINWIDTH);
    *(bin+2*i+1) = floor(proj_data[1][m] / BINWIDTH);
    *(sort+i)    = i;
  }

/* Sorting */
  qsort ((char *) sort, n, sizeof(int), compare1);

/* Calculate index function */
/*  window_width3 = window_width_const*pow((double) n, -(double) bandwidth);*/
  h = (long) ceil (bandwidth/BINWIDTH);
  h *= h;
  hf = 1.0/ ((float) h);   
  start = 0;
  indx_val = 0;
  for (i=0; i<n; i++)
  { for (j=start; j<i; j++)
    { ki = 2*sort[i];
      kj = 2*sort[j];
      tmpl = *(bin+ki) - *(bin+kj);
      dist = tmpl * tmpl;
      if (dist>h)
        start++;
      else
      { ki++;
        kj++;
        tmpl = *(bin+ki) - *(bin+kj);
        dist += tmpl*tmpl;
        if (dist < h)
        { tmpf = 1-hf*(float)dist;
          indx_val += tmpf*tmpf*tmpf;
        } 
      }
    }
  }
  indx_val = n + 2.0*indx_val;
  indx_val *= (4.0 / (M_PI*n*n*bandwidth*bandwidth));
  return (indx_val);
}

void bin_fts_deriv (float **data, float **proj_data,
                    float *alpha, float *beta, 
                    float **derivs,
                    int n, int *rows_in_plot, 
                    int p, int nactive, int *active_vars,
                    float bandwidth)
/* Calculation of

   \sum_{i=1}^{n} \sum{j=1}^{i-1} 
      dK_h((x_i - x_j)/h, (y_i-y_j)/h) (xo_{i,m} - xo_{j,m})^2

   replace alpha_m by beta_m for derivation of beta
*/

{
  float tmpf, dm, hf;/* window_width3;*/
  int i, j, m, start, k, ki, kj;
  long dist, tmplx, h, tmply;

/* Initialize derivatives */
  for (i=0; i<2; i++)
    for (k=0; k<p; k++)
      derivs[i][k] = 0.;

/* Binning */
  for (i=0; i<n; i++)
  { m = rows_in_plot[i];
    *(bin+2*i) = floor(proj_data[0][m] / BINWIDTH);
    *(bin+2*i+1) = floor(proj_data[1][m] / BINWIDTH);
    *(sort+i) = i;
  }

/* Sorting */
  qsort ((char *) sort, n, sizeof(int), compare1);

/* common term for derivatives */
  tmpf = 0.931 * pow ((double) n, 1.0/24.0);
/*  window_width3 = tmpf*window_width_const*pow((double) n, -(double) bandwidth);*/
  h  = ceil (bandwidth/BINWIDTH);
  h *= h;  
  hf = 1.0/((float) h);
  start = 0;
  for (i=0; i<n; i++)
  { for (j=start; j<i; j++)
    { ki = 2*sort[i];
      kj = 2*sort[j];
      tmplx = *(bin+ki) - *(bin+kj);
      dist  = tmplx * tmplx;
      if (dist>h)
        start++;
      else
      { ki++;
        kj++;
        tmply = *(bin+ki) - *(bin+kj);
        dist += tmply*tmply;
        if (dist < h)
        { tmpf = 1-hf*dist;
          ki = rows_in_plot[sort[i]];
          kj = rows_in_plot[sort[j]];
          for (k=0; k<nactive; k++)
          { dm = (data[ki][active_vars[k]] - data[kj][active_vars[k]])-
                 BINWIDTH*(alpha[active_vars[k]]*tmplx-beta[active_vars[k]]*tmply);
            derivs[0][active_vars[k]] += (tmpf*tmpf*(float)tmplx*dm);
            derivs[1][active_vars[k]] += (tmpf*tmpf*(float)tmply*dm); 
          }
        } 
      }
    }
  }

  tmpf = -24.0/pow(bandwidth, 3.0);
  tmpf /= n;
  tmpf /= h;
  tmpf /= ((float) M_PI);
/*   tmpf = -24.0/(M_PI*n*h*pow(bandwidth, 3)); */
  for (i=0; i<2; i++)
    for (k=0; k<p; k++)
      derivs[i][k] *= tmpf;
}

static float *dens, *deno;

void alloc_bin_entropy (int n)
{
  bin  = (int *) XtMalloc((Cardinal) (2 * n * sizeof(int)));
  sort = (int *) XtMalloc((Cardinal) (n * sizeof(int)));
  dens = (float *) XtMalloc((Cardinal) (n * sizeof(float)));
  deno = (float *) XtMalloc((Cardinal) (n * sizeof(float)));
}

void free_bin_entropy()
{
  XtFree((XtPointer) bin);
  XtFree((XtPointer) sort);
  XtFree((XtPointer) dens);
  XtFree((XtPointer) deno);
}

float bin_entropy_index (float **proj_data, int n, int *rows_in_plot,
float bandwidth)
/* Calculation of

\sum_{i=1}^{n} log ( 1.0 + \sum_{j=1, i\neq j}^n K_h ((x_i-x_j)/h, (y_i - y_j)/h)
*/
{ float indx_val, tmpf, hf;/*, window_width3;*/
  int i, j, m, start, ki, kj;
  long dist, tmpl, h;

/* Binning */
  for (i=0; i<n; i++)
  { m = rows_in_plot[i];
    *(bin+2*i)   = floor(proj_data[0][m] / BINWIDTH);
    *(bin+2*i+1) = floor(proj_data[1][m] / BINWIDTH);
     *(sort+i)    = i;
    *(dens+i)    = 0.0;
  }

/* Sorting */
  qsort ((char *) sort, n, sizeof(int), compare1);

/* Calculate index function */
/*   window_width3 = window_width_const*pow((double) n, -(double) bandwidth); */
  h  = ceil (bandwidth/BINWIDTH);
  h *= h;  
  hf = 1.0/((float) h);
  start = 0;
  for (i=0; i<n; i++)
  { for (j=start; j<i; j++)
    { ki = 2*sort[i];
      kj = 2*sort[j];
      tmpl = *(bin+ki) - *(bin+kj);
      dist = tmpl * tmpl;
      if (dist>h)
        start++;
      else
      { ki++;
        kj++;
        tmpl = *(bin+ki) - *(bin+kj);
        dist += tmpl*tmpl;
        if (dist < h)
        { tmpf = 1-hf*dist;
          tmpf = tmpf*tmpf*tmpf;
          *(dens+i) += tmpf;
          *(dens+j) += tmpf;
        } 
      }
    }
  }
  indx_val = 0;
  for (i=0; i<n; i++)
    indx_val += log (1.0 + *(dens+i));

  indx_val = indx_val / n + log (4.0 / (n*bandwidth*bandwidth*M_PI)); 
  return (indx_val);
}

void bin_entropy_deriv (float **data, float **proj_data,
                    float *alpha, float *beta, 
                    float **derivs,
                    int n, int *rows_in_plot, 
                    int p, int nactive, int *active_vars,
                    float bandwidth)
/* Calculation of

  - alpha_m \sum_{i=1}^{n} 
     \frac{ \sum_{j=1,i\neq j}^{n} dK_h((x_i-x_j)/h, (y_i-y_j)/h) (xo_{i,m}-xo{j,m})^2)}
          { 1.0 + \sum_{j=1,i\neq j}^{n} K_h((x_i-x_j)/h, (y_i-y_j)/h) }

   replace alpha_m by beta_m for derivation of beta
*/        

{ float tmpf, dm, hf;/*, window_width3;*/
  int i, j, m, start, k, ki, kj;
  long dist, tmpl, h, tmplx, tmply;

/* Initialize derivatives */
  for (i=0; i<2; i++)
    for (k=0; k<p; k++)
      derivs[i][k] = 0.;

/* Binning */
  for (i=0; i<n; i++)
  { m = rows_in_plot[i];
    *(bin+2*i) = floor(proj_data[0][m] / BINWIDTH);
    *(bin+2*i+1) = floor(proj_data[1][m] / BINWIDTH);
    *(sort+i) = i;
    *(deno+i) = 0.0;
  }

/* Sorting */
  qsort ((char *) sort, n, sizeof(int), compare1);

  tmpf = 0.931 * pow ((double) n, 1.0/24.0); 
/*   window_width3 = tmpf*window_width_const*pow((double) n, -(double) bandwidth);  */
  h = (long) ceil (bandwidth/BINWIDTH);
  h *= h;  
  hf = 1.0/((float) h);
  start = 0;

/* calculate denominator for every i  */
  for (i=0; i<n; i++)
  { for (j=start; j<i; j++)
    { ki = 2*sort[i];
      kj = 2*sort[j];
      tmpl = *(bin+ki) - *(bin+kj);
      dist = tmpl * tmpl;
      if (dist>h)
        start++;
      else
      { ki++;
        kj++;
        tmpl = *(bin+ki) - *(bin+kj);
        dist += tmpl*tmpl;
        if (dist < h)
        { tmpf = 1-hf*dist;
          tmpf = tmpf*tmpf*tmpf;
          *(deno+i) += tmpf;
          *(deno+j) += tmpf;
        } 
      }
    }
  }
/* calculate true denominator */
  for (i=0; i<n; i++)
    *(deno+i) = 1.0/(1.0+ *(deno+i));

/* sum up now nominators */
  start = 0;

  for (i=0; i<n; i++)
  { for (j=start; j<i; j++)   
    { ki = 2*sort[i];
      kj = 2*sort[j];
      tmplx = *(bin+ki) - *(bin+kj);
      dist  = tmplx * tmplx;
      if (dist>h)
        start++;
      else
      { ki++;
        kj++;
        tmply = *(bin+ki) - *(bin+kj);
        dist += tmply*tmply;
        if (dist < h)
        { tmpf = 1-hf*dist;
          ki = rows_in_plot[sort[i]];
          kj = rows_in_plot[sort[j]];
          for (k=0; k<nactive; k++)
          { dm = (data[ki][active_vars[k]] - data[kj][active_vars[k]])-
                 BINWIDTH*(alpha[active_vars[k]]*tmplx-beta[active_vars[k]]*tmply);
            derivs[0][active_vars[k]] += (tmpf*tmpf*tmplx*dm*(*(deno+i) + *(deno+j)));
            derivs[1][active_vars[k]] += (tmpf*tmpf*tmply*dm*(*(deno+i) + *(deno+j))); 
          }
        } 
      }
    }
  }

/* Correct derivatives */
  tmpf = -6.0 / ((float)n*(float)h*bandwidth);
  for (i=0; i<2; i++)
    for (k=0; k<p; k++)
      derivs[i][k] *= tmpf;
}

 
