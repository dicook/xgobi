/* dummy.c */
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

/* #defines

static float *var_decs0, *var_decs1;

void
alloc_dummy(int n)
{
  var_decs0 = (float *) XtMalloc(
    (unsigned int) n*sizeof(float *));
  var_decs1 = (float *) XtMalloc(
    (unsigned int) n*sizeof(float *));
}

void
free_dummy()
{
  XtFree((XtPointer) var_decs0);
  XtFree((XtPointer) var_decs1);
}

float
dummy_index(float **proj_data, int n, int *rows_in_plot, float param)
{
  int i, m;
  float indx_val;

* Calculations of index *
  for (i=0; i<n; i++)
  {
    m = rows_in_plot[i];
    var_decs0[m] = something;
    var_decs1[m] = something;
  }

  indx_val = something ;
  return(indx_val);
}

void
dummy_deriv(float **data, float **proj_data, float **derivs,
int n, int *rows_in_plot, int p, float *alpha, float *beta, float param,
int nactive, int *active_vars)
{
  int i, k, m;
  float tmpf;

  for (i=0; i<2; i++)
    for (k=0; k<p; k++)
      derivs[i][k] = 0.;

* calculations for derivatives in direction alpha *
  for (k=0; k<nactive; k++)
  {
    tmpf = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf += (proj_data[0][m]*var_decs0[m]*something
***** multiplying by this last quantity ensures constraints of
    unit variance and orthogonality between alpha and beta
    are built in, found by Lagrange multipliers in our case,
    may be different in your case *****
        (data[m][active_vars[k]] -
         alpha[active_vars[k]]*proj_data[0][m] -
         beta[active_vars[k]]*proj_data[1][m]));
    }
    tmpf /= ((float)n);
    derivs[0][active_vars[k]] = tmpf;
 }

* calculations for derivatives in direction beta *
  for (k=0; k<nactive; k++)
  {
    tmpf = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf += something
***** ensure constraints are built in *****
     }
     tmpf /= ((float)n);
     derivs[1][active_vars[k]] = tmpf;
  }

}

#undefs
*/
