/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  this  code and associated documentation,  provided      *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *    Copyright (c) 1997 Sigbert Klinke                     *
 *                       <sigbert@wiwi.hu-berlin.de>        *
 *  All Rights Reserved.                                    *
 *                                                          *
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#define malloc(x) XtMalloc((Cardinal) x)
#define free(x)   XtFree((char *) x)

double *xsort;
int *gsort;

int
indexsort (const void *left, const void *right)
{
  if (xsort[*((int *) left)]>xsort[*((int *) right)]) return (1);
  if (xsort[*((int *) left)]<xsort[*((int *) right)]) return (-1);
  return (0);
}

int
groupsort (const void *left, const void *right)
{
  if (gsort[*((int *)left)]>gsort[*((int *)right)]) return (1);
  if (gsort[*((int *)left)]<gsort[*((int *)right)]) return (-1);
  return (indexsort(left, right));
}

double
quartic (double x)
{
  double x2 = x*x;
  if (x2>1.0) return 0.0;
  return (0.9375*(1-x2)*(1-x2));
}

void
nadaraya_watson (int n, double *x, double *y, int *group, double bandwidth, double *yest, int *index)
{
  int i, j, k, maxgroup, start;
  double diff, kern, *yup, *ydn;

  yup   = (double *) malloc(n*sizeof(double));  
  ydn   = (double *) malloc(n*sizeof(double));  

  maxgroup = -1;
  for (i=0; i<n; i++)
  { index[i] = i;
    if (group[i]>maxgroup) maxgroup = group[i]; 
    ydn[i] = quartic(0);          /* K(0)     */
    yup[i] = y[i]*quartic(0);     /* Y_i K(0) */
  }

  xsort = x;
  qsort ((char *) index, n, sizeof(int), indexsort);

  for (k=0; k<=maxgroup; k++)
  { for (start=i=0; i<n; i++)
    { if (group[index[i]]==k)
      { for (j=start; j<i; j++)
        { if (group[index[j]]==k)
	  { diff = (x[index[i]]-x[index[j]])/bandwidth;
            kern = quartic(diff);
            if (kern==0.0) 
              start++;
            else
	    { yup[index[i]] += y[index[j]]*kern;
              yup[index[j]] += y[index[i]]*kern;
	      ydn[index[i]] += kern;
              ydn[index[j]] += kern;
            }
          }
        }
      }
    }
  }

  for (i=0; i<n; i++)
    yest[i] = yup[i]/ydn[i];

  free (yup);
  free (ydn);
}  


extern long *x_sm;
extern long *y_sm;
extern int *grp_id_sm;

void
nadaraya_watson_smoother(long *x, long *y, int n, long *sm_pars,
  int num_grp, unsigned long *grp_id, int *pnum_pts)

/* Di uses global variables x_sm and y_sm for smoothing !! */

{
  int i, *g, *index;
  double *xsk, *ysk, *ye;

  if (x_sm)
    free (x_sm);
  x_sm = (long *) malloc (n*sizeof(long));
  if (y_sm)
    free (y_sm);
  y_sm = (long *) malloc (n*sizeof(long));
  if (grp_id_sm)
    free (grp_id_sm);
  grp_id_sm = (int *) malloc (n*sizeof(int));

  xsk = (double *) malloc (n*sizeof(double));
  ysk = (double *) malloc (n*sizeof(double));

  for (i=0; i<n; i++)
  { xsk[i] = x[i];
    ysk[i] = y[i];
  } 
  
  ye = (double *) malloc (n*sizeof(double));
  g  = (int *) malloc (n*sizeof(int));
  for (i=0; i<n; i++)
   g[i] = (int) grp_id[i]-1;    
 
  index = (int *) malloc (n*sizeof(int));
  nadaraya_watson (n, xsk, ysk, g, (double) *sm_pars, ye, index);

  xsort = xsk;
  gsort = g;

  qsort ((char *) index, n, sizeof(int), groupsort);

  for (i=0; i<n; i++)
  { x_sm[i] = (long) xsk[index[i]];
    y_sm[i] = (long) ye[index[i]];
    grp_id_sm[i] = g[index[i]]+1; 
  }

  *pnum_pts = n;

  free (index);
  free (ye);
  free (g);
  free (xsk);
  free (ysk);
}


void
spline_smoother(long *x, long *y, int n, long *sm_pars, int num_grp,
  unsigned long *grp_id, int *pnum_pts)

/* Di uses still (!) global variables x_sm and y_sm for smoothing !! */

{
  int i, j, k, l, m, *index; 
  double *xsk, *ysk, *ye, *w, *lev, gcv, cv, df, lambda, dfmax, *work, *xw, *yw, *ew;
  long ng, nvar, norder, method, ierun, ier;
  extern int pspline_();

  if (x_sm)
    free (x_sm);
  x_sm = (long *) malloc (n*sizeof(long));
  if (y_sm)
    free (y_sm);
  y_sm = (long *) malloc (n*sizeof(long));
  if (grp_id_sm)
    free (grp_id_sm);
  grp_id_sm = (int *) malloc (n*sizeof(int));

  xsk = (double *) malloc (n*sizeof(double));
  ysk = (double *) malloc (n*sizeof(double));
  xw  = (double *) malloc (n*sizeof(double));
  yw  = (double *) malloc (n*sizeof(double));
  w   = (double *) malloc (n*sizeof(double));
  ye  = (double *) malloc (n*sizeof(double));
  lev = (double *) malloc (n*sizeof(double));
  ew  = (double *) malloc (n*sizeof(double));
  work = (double *) malloc (((n-2)*11+n)*sizeof(double));
  index = (int *) malloc (n*sizeof(int));

  for (i=0; i<n; i++)
  { xsk[i] = x[i];
    ysk[i] = y[i];
    w[i]   = 1.0;
    index[i] = i;
  } 

/* the range lambda might be adjusted to other datasets
   the actual range works well for the flea data
   SK
*/
  lambda    = exp(7+*sm_pars/150);

  nvar      = 1;
  norder    = 2;
  df        = norder+2;
  method    = 1;
  j         = 0;
  *pnum_pts = 0;

  for (k=1; k<=num_grp; k++)
  { for (ng=i=0; i<n; i++)
    { if (grp_id[i]==k)
      { xsk[ng] = x[i];
        ysk[ng] = y[i];
        index[ng] = ng;
        ng++;
      }
    }
    xsort = xsk;
    qsort ((char *) index, ng, sizeof(int), indexsort);
    for (i=0;i<ng;i++)
    { xw[i] = xsk[index[i]];
      yw[i] = ysk[index[i]];
    }
    for (i=0;i<ng;i++)
    { if (i && (xw[i]==xw[i-1]))
      { for (l=i; xw[l]==xw[i]; l++);
        for (m=i; m<l; m++)
          xw[m] += ((double) (m-i+1))/((double) (l-i+1));
      }        
    }
    dfmax = ng;
    ier   = ierun = 0;
    pspline_(&ng, &nvar, &norder, xw, w, yw, ew, lev, 
             &gcv, &cv, &df, &lambda, &dfmax, work, &method,
             &ierun, &ier);
    if (ier)
    { switch (ier)
      { case 1 : printf ("spline: n<2*norder+1\n"); break;
        case 2 : printf ("spline: norder out of range\n"); break;
        case 3 : printf ("spline: nvar<1\n"); break;
        case 4 : printf ("spline: lambda<0\n"); break;
        case 5 : printf ("spline: x not strictly monotone\n"); break;
        case 6 : printf ("spline: some w are negative\n"); break;
        default: printf ("spline: Choleski decomposition failed\n"); break;
      } 
    }
    else
    { for (i=0; i<ng; i++, j++)
      { x_sm[j] = (long) xw[i];
        y_sm[j] = (long) ew[i];
        grp_id_sm[j] = k;
      } 
    }
  }
  
  *pnum_pts = j;

  free (index);
  free (ye);
  free (xsk);
  free (ysk);
  free (w);
  free (lev);
  free (ew);
  free (work);
  free (xw);
  free (yw);
}
