/* natrl_hermite.c */
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

float **h0, **h1, **hp0, **hp1, **capH0, **capH1, **phipi;
float **acoefs, *bcoefs;

void
alloc_natural_hermite(int n, int maxlJ)
{
  int i;

  h0 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    h0[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  h1 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    h1[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  hp0 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    hp0[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  hp1 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    hp1[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  capH0 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    capH0[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  capH1 = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    capH1[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  phipi = (float **) XtMalloc(
    (unsigned int) 2*sizeof(float *));
  for (i=0; i<2; i++)
    phipi[i] = (float *) XtMalloc(
      (unsigned int) n*sizeof(float));
  acoefs = (float **) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
  for (i=0; i<maxlJ; i++)
    acoefs[i] = (float *) XtMalloc(
      (unsigned int) maxlJ*sizeof(float));
  bcoefs = (float *) XtMalloc(
    (unsigned int) maxlJ*sizeof(float *));
}

void
free_natural_hermite(int maxlJ)
{
  int i;

  for (i=0; i<maxlJ; i++)
    XtFree((char *) h0[i]);
  XtFree((char *) h0);
  for (i=0; i<maxlJ; i++)
    XtFree((char *) h1[i]);
  XtFree((char *) h1);
  for (i=0; i<maxlJ; i++)
    XtFree((char *) hp0[i]);
  XtFree((char *) hp0);
  for (i=0; i<maxlJ; i++)
    XtFree((char *) hp1[i]);
  XtFree((char *) hp1);
  for (i=0; i<maxlJ; i++)
    XtFree((char *) capH0[i]);
  XtFree((char *) capH0);
  for (i=0; i<maxlJ; i++)
    XtFree((char *) capH1[i]);
  XtFree((char *) capH1);
  for (i=0; i<2; i++)
    XtFree((char *) phipi[i]);
  XtFree((char *) phipi);
  for (i=0; i<maxlJ; i++)
    XtFree((char *) acoefs[i]);
  XtFree((char *) acoefs);
  XtFree((char *) bcoefs);

}

float
natural_hermite_index(float **proj_data, int n, int *rows_in_plot, int lJ)
{
  int i,j,m;
  float tmpf1, tmpf2;
  float fact, pow2, twicefact;
  float indx_val;

/* Calculate Hermite polynomials */
  for (i=0; i<n; i++)
  {
    m = rows_in_plot[i];
    capH0[0][m] = 1.;
    capH1[0][m] = 1.;
  }
  if (lJ > 1)
  {
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      capH0[1][m] = proj_data[0][m];
      capH1[1][m] = proj_data[1][m];
    }
    for (i=2; i<lJ; i++)
    {
      for (j=0; j<n; j++)
      {
        m = rows_in_plot[j];
        capH0[i][m] = (proj_data[0][m]
          * capH0[i-1][m] -
          (i-1.)*capH0[i-2][m]);
        capH1[i][m] = (proj_data[1][m]
          * capH1[i-1][m] -
          (i-1.)*capH1[i-2][m]);
      }
    }
  }

/* Calculate constant term */
  tmpf1 = (float)(sqrt((double) 2.)*(float)QUROOTPI);
  for (i=0; i<n; i++)
  {
    m = rows_in_plot[i];
    phipi[0][m] = exp(-proj_data[0][m]*proj_data[0][m]/2.) / 
      sqrt((double)(2.*M_PI)) ;
    phipi[1][m] = exp(-proj_data[1][m]*proj_data[1][m]/2.) / 
      sqrt((double)(2.*M_PI)) ;
  }

/* Calculate coefficients */
  tmpf1 = (float)sqrt((double) 2.);
  for (i=0; i<n; i++)
  {
    m = rows_in_plot[i];
    h0[0][m] = capH0[0][m];
    h1[0][m] = capH1[0][m];
  }
  if (lJ > 1)
  {
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      h0[1][m] = capH0[1][m];
      h1[1][m] = capH1[1][m];
    }
    fact = 1.;
    for (i=2; i<lJ; i++)
    {
      fact *= ((float)i);
      tmpf2 = (float)sqrt((double) (fact));
      for (j=0; j<n; j++)
      {
        m = rows_in_plot[j];
        h0[i][m] =
          capH0[i][m] / tmpf2;
        h1[i][m] =
          capH1[i][m] / tmpf2;
      }
    }
  }

/* Calculate index */
  for (i=0; i<lJ; i++)
    for (j=0; j<(lJ-i); j++)
      acoefs[i][j] = mean_fn4(h0[i],h1[j],phipi[0],phipi[1],n,
        rows_in_plot);/*section*/
  fact = 1.;
  twicefact = 1.;
  pow2 = 2.;
  for (i=0; i<lJ; i++)
  {
    if (i%2 == 0)
    {
      j = i/2;
      tmpf1 = (float)pow((double)-1.,(double)j);
      bcoefs[i] = tmpf1*sqrt((double) twicefact)/
        (fact*pow2*sqrt((double)M_PI));
      if (i == 0)
      {
        twicefact *= ((float) (2.*(j+1)));
      }
      else
      {
        fact *= ((float) (j+1));
        twicefact *= ((float) (2*(j+1)));
        twicefact *= ((float) (2*(j+1)-1));
      }
      pow2 *= (4.);
	}
    else
      bcoefs[i] = 0.;
  }
  indx_val = 0.;

  for (i=0; i<lJ; i++)
    for (j=0; j<(lJ-i); j++)
    {
    tmpf1 = acoefs[i][j]*acoefs[i][j];
    indx_val += tmpf1;
    }
  for (i=0; i<lJ; i++)
    for (j=0; j<(lJ-i); j++)
    indx_val -= (2.*acoefs[i][j]*bcoefs[i]*bcoefs[j]);
  indx_val += (1./(12.*(float)M_PI*(float)M_PI));

  return(indx_val);
}

void
natural_hermite_deriv(float **data, float **proj_data, float *alpha,
float *beta, float **derivs, int n, int *rows_in_plot, int p,
int *active_vars, int nactive, int lJ)
{
  int i, j, k, l, m;
  float tmpf1, tmpf2, tmpf3;

/* Calculate derivatives of Hermite functions, in this case not
 * derivatives of Hermite functions but derivatives of the
 * coefficients, acoefs.
*/
  for (i=0; i<n; i++) {
    m = rows_in_plot[i];
    hp0[0][m] = -proj_data[0][m] * phipi[0][m];
    hp1[0][m] = -proj_data[1][m] * phipi[1][m];
  }
  for (i=1; i<lJ; i++)
    for (j=0; j<n; j++) {
      m = rows_in_plot[j];
      hp0[i][m] = (sqrt((double) (i)) * h0[i-1][m] - proj_data[0][m] *
        h0[i][m]) * phipi[0][m];
      hp1[i][m] = (sqrt((double) (i)) * h1[i-1][m] - proj_data[1][m] *
        h1[i][m]) * phipi[1][m];
    }

/* alpha */
  for (k=0; k<p; k++)
    derivs[0][k] = 0.;


/* Calculate second term in derivative */
  for (l=0; l<lJ; l++) {
    for (j=0; j<lJ; j++) {
      for (k=0; k<nactive; k++) {
        tmpf1 = 0.;
        for (i=0; i<n; i++) {
          m = rows_in_plot[i];
          tmpf3 = data[m][active_vars[k]] -
            alpha[active_vars[k]]*proj_data[0][m] -
            beta[active_vars[k]]*proj_data[1][m];
          tmpf1 += (hp0[l][m] * h1[j][m] * phipi[1][m] *
            tmpf3);
        }/*section*/
        tmpf1 /= ((float)n);
        derivs[0][active_vars[k]] -= (2.*tmpf1*bcoefs[l]*bcoefs[j]);
      }
    }
  }
/* Calculate first term */
  for (l=0; l<lJ; l++) {
    for (j=0; j<(lJ-l); j++) {
      for (k=0; k<nactive; k++) {
        tmpf2 = 0.;
        for (i=0; i<n; i++) {
          m = rows_in_plot[i];
          tmpf3 = data[m][active_vars[k]] -
            alpha[active_vars[k]] * proj_data[0][m] -
            beta[active_vars[k]] * proj_data[1][m];
          tmpf2 += (hp0[l][m] * h1[j][m] *
            phipi[1][m] * tmpf3);
        }/*section*/
        tmpf2 /= ((float)n);
        derivs[0][active_vars[k]] += (2.0*acoefs[l][j]*tmpf2);
      }
    }
  }

/* beta */
  for (k=0; k<p; k++)
    derivs[1][k] = 0.;

/* Calculate second term in derivative */
  for (l=0; l<lJ; l++) {
    for (j=0; j<lJ; j++) {
      for (k=0; k<nactive; k++) {
        tmpf1 = 0.;
        for (i=0; i<n; i++) {
          m = rows_in_plot[i];
          tmpf3 = data[m][active_vars[k]] -
            alpha[active_vars[k]]*proj_data[0][m] -
            beta[active_vars[k]]*proj_data[1][m];
          tmpf1 += (h0[l][m] * phipi[0][m] * hp1[j][m] *
            tmpf3);
        }/*section*/
        tmpf1 /= ((float)n);
        derivs[1][active_vars[k]] -= (2.*tmpf1*bcoefs[l]*bcoefs[j]);
      }
    }
  }
/* Calculate first term */
  for (l=0; l<lJ; l++) {
    for (j=0; j<(lJ-l); j++) {
      for (k=0; k<nactive; k++) {
        tmpf2 = 0.;
        for (i=0; i<n; i++) {
          m = rows_in_plot[i];
          tmpf3 = data[m][active_vars[k]] -
            alpha[active_vars[k]] *
            proj_data[0][m] -
            beta[active_vars[k]] *
            proj_data[1][m];
          tmpf2 += (h0[l][m] *
            phipi[0][m] *
            hp1[j][m] *
            tmpf3);
        }/*section*/
        tmpf2 /= ((float)n);
        derivs[1][active_vars[k]] += (2.0*acoefs[l][j]*tmpf2);
      }
    }
  }
}
