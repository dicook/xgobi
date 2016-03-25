/* de_indices.c */
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

/*float window_width_const = 3.12;*/

float
calc_kernel2(float Y[2][2], int n, float bandwidth)
{
  float return_val;
  float tmpf;
/*   float window_width2; */

/*  window_width2 = window_width_const /
    (float)pow((double)n, (double)window_width_exponent);*/

  return_val = 0.;

  Y[0][0] -= Y[1][0];
  Y[0][1] -= Y[1][1];

  Y[0][0] /= bandwidth;
  Y[0][1] /= bandwidth;

  tmpf = Y[0][0]*Y[0][0] + Y[0][1]*Y[0][1];

  if (tmpf < 1.0) {
    tmpf = 1.0 - tmpf;
    return_val = 3.0 * tmpf * tmpf / (float)M_PI;
  }

  return(return_val);

}

float
calc_kernel3(float Y[2][2], int n, float bandwidth)
{
  float return_val;
  float tmpf;
/*   float window_width3; */

/*  window_width3 = window_width_const /
    (float)pow((double)n, (double)window_width_exponent);*/

  return_val = 0.;

  Y[0][0] -= Y[1][0];
  Y[0][1] -= Y[1][1];

  Y[0][0] /= bandwidth;
  Y[0][1] /= bandwidth;

  tmpf = Y[0][0]*Y[0][0] + Y[0][1]*Y[0][1];

  if (tmpf < 1.0)
  {
    tmpf = 1.0 - tmpf;
    return_val = 4.0 * tmpf * tmpf * tmpf/ (float)M_PI;
  }

  return(return_val);

}

float
fts_index(float **proj_data, int n, int *rows_in_plot, float bandwidth)
{
  int i, j;
/*   float window_width3 = window_width_const; */
  float Y[2][2];
  float indx_val;

/*  tmpf = (float) pow((double)n, (double)window_width_exponent);
  window_width3 /= tmpf;*/

  indx_val = 0.;
  for (i=0; i<n; i++)
  {
    for (j=0; j<n; j++)
    {
      Y[0][0] = proj_data[0][rows_in_plot[i]];
      Y[0][1] = proj_data[1][rows_in_plot[i]];
      Y[1][0] = proj_data[0][rows_in_plot[j]];
      Y[1][1] = proj_data[1][rows_in_plot[j]];
      indx_val += calc_kernel3(Y, n, bandwidth);
    }
  }
  indx_val /= ((float)(n*n));
  indx_val /= (bandwidth*bandwidth);
  return(indx_val);
}

float
entropy_index(float **proj_data, int n, int *rows_in_plot, float bandwidth)
{
  int i, j;
/*   float  bandwidth = window_width_const; */
  float Y[2][2], tmpf;
  float indx_val;

/*  tmpf =  (float) pow((double)n, (double)window_width_exponent);
  bandwidth /= tmpf;*/

  indx_val = 0.;
  for (i=0; i<n; i++)
  {
    tmpf = 0.;
    for (j=0; j<n; j++)
    {
      Y[0][0] = proj_data[0][rows_in_plot[i]];
      Y[0][1] = proj_data[1][rows_in_plot[i]];
      Y[1][0] = proj_data[0][rows_in_plot[j]];
      Y[1][1] = proj_data[1][rows_in_plot[j]];
      tmpf += calc_kernel3(Y,n,bandwidth);
  }
    tmpf /= (bandwidth*bandwidth*n);
    indx_val += ((float)log((double)tmpf));
  }
  indx_val /= ((float)(n));
  return(indx_val);
}


void
fts_deriv(float **data, float **proj_data, float **derivs,
int n, int *rows_in_plot, int p,
float *alpha, float *beta, float bandwidth,
int nactive, int *active_vars)
{
  int i, j, k;
  float tmpf1, tmpf2, tmpf3, tmpf4;
/*   float window_width3 = window_width_const; */
  float Y[2][2];

/*  tmpf1 =  (float)pow((double)n, (double)window_width_exponent);
  window_width3 /= tmpf1;*/

  for (i=0; i<2; i++)
    for (j=0; j<p; j++)
      derivs[i][j] = 0.;

/* alpha and beta */
    for (i=0; i<n; i++)
      for (j=0; j<n; j++) {
        if (i != j) {
          Y[0][0] = proj_data[0][rows_in_plot[i]];
          Y[0][1] = proj_data[1][rows_in_plot[i]];
          Y[1][0] = proj_data[0][rows_in_plot[j]];
          Y[1][1] = proj_data[1][rows_in_plot[j]];
          Y[0][0] -= Y[1][0];
          Y[0][1] -= Y[1][1];
          Y[0][0] /= bandwidth;
          Y[0][1] /= bandwidth;
          tmpf1 = Y[0][0]*Y[0][0] + Y[0][1]*Y[0][1];
          if (tmpf1 < 1) {
            tmpf1 = 1 - tmpf1;
            tmpf1 *= tmpf1;
            tmpf3 = tmpf1 * Y[0][0];
            tmpf1 *= Y[0][1];
            for (k=0; k<nactive; k++) {
              tmpf4 = (data[rows_in_plot[i]][active_vars[k]] -
                data[rows_in_plot[j]][active_vars[k]])
                / bandwidth;
              tmpf2 = (tmpf4 -
                alpha[active_vars[k]] * Y[0][0] -
                beta[active_vars[k]] * Y[0][1]);
              derivs[0][active_vars[k]] += (tmpf3*tmpf2);
              derivs[1][active_vars[k]] += (tmpf1*tmpf2);
            }
          }
        }
      }

  for (j=0; j<2; j++)
    for (k=0; k<nactive; k++) {
      derivs[j][active_vars[k]] /= ((float)(n*n));
      derivs[j][active_vars[k]] *= (-24.0);
      derivs[j][active_vars[k]] /= ((float)bandwidth*bandwidth);
      derivs[j][active_vars[k]] /= ((float)M_PI);
    }

}

void
entropy_deriv(float **data, float **proj_data, float **derivs,
int n, int *rows_in_plot, int p, float *alpha, float *beta,
float bandwidth, int nactive, int *active_vars)
{
  int i, j, k;
  float tmpf1, tmpf2, tmpf3, tmpf4, tmpf5, tmpf6;
/*   float window_width3 = window_width_const; */
  float Y[2][2];

/*  tmpf1 =  (float)pow((double)n, (double)bandwidth);
  window_width3 /= tmpf1;*/

  for (i=0; i<2; i++)
    for (j=0; j<p; j++)
      derivs[i][j] = 0.;

/* alpha and beta */
  for (i=0; i<n; i++) {
    for (k=0; k<nactive; k++) {
      tmpf5 = 0.;
      tmpf6 = 0.;
      for (j=0; j<n; j++) {
        if (i != j) {
          Y[0][0] = proj_data[0][rows_in_plot[i]];
          Y[0][1] = proj_data[1][rows_in_plot[i]];
          Y[1][0] = proj_data[0][rows_in_plot[j]];
          Y[1][1] = proj_data[1][rows_in_plot[j]];
          Y[0][0] -= Y[1][0];
          Y[0][1] -= Y[1][1];
          Y[0][0] /= bandwidth;
          Y[0][1] /= bandwidth;
          tmpf1 = Y[0][0]*Y[0][0] + Y[0][1]*Y[0][1];
          if (tmpf1 < 1) {
            tmpf1 = 1 - tmpf1;
            tmpf1 *= tmpf1;
            tmpf3 = tmpf1 * Y[0][0];
            tmpf1 *= Y[0][1];
            tmpf4 = (data[rows_in_plot[i]][active_vars[k]] -
              data[rows_in_plot[j]][active_vars[k]])
              / bandwidth;
            tmpf2 = (tmpf4 -
              alpha[active_vars[k]] * Y[0][0] -
              beta[active_vars[k]] * Y[0][1]);
            tmpf5 += (tmpf3*tmpf2);
            tmpf6 += (tmpf1*tmpf2);
          }
        }
      }
      tmpf1 = 0.;
      for (j=0; j<n; j++)
      {
        Y[0][0] = proj_data[0][rows_in_plot[i]];
        Y[0][1] = proj_data[1][rows_in_plot[i]];
        Y[1][0] = proj_data[0][rows_in_plot[j]];
        Y[1][1] = proj_data[1][rows_in_plot[j]];
        tmpf1 += calc_kernel3(Y,n,bandwidth);
      }
      tmpf1 /= (bandwidth*bandwidth*n);
      derivs[0][active_vars[k]] += (tmpf5/tmpf1);
      derivs[1][active_vars[k]] += (tmpf6/tmpf1);
    }
  }

  for (j=0; j<2; j++)
    for (k=0; k<nactive; k++) {
      derivs[j][active_vars[k]] /= ((float)(n*n));
      derivs[j][active_vars[k]] *= (-24.0);
      derivs[j][active_vars[k]] /= ((float)bandwidth*bandwidth);
      derivs[j][active_vars[k]] /= ((float)M_PI);
    }

}
