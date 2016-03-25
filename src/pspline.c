/* pspline.c */
/************************************************************
 *                                                          *
 *  This code was taken from STATLIB                        *
 *                                                          *
 *  http://lib.stat.cmu.edu/S/pspline                       *
 *    A shar archive containing S functions which implement *
 *    penalized spline smoothing. Contributed by Jim        *
 *    Ramsay [2/Aug/96] [13/Dec/96] (78k)                   *
 *                                                          *
 *  and translated from FORTRAN to C                        *        
 *                                                          *
 *  Pspline.f -- translated by f2c                          *
 *  (version of 23 April 1993  18:34:30).                   *
 *  You must link the resulting object file with the        *
 *  libraries:                                              *
 *	-lf2c -lm   (in that order)                         *
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

#include <math.h>
#include "f2c.h"

/* Table of constant values */

static integer c__2 = 2;
static integer c__1 = 1;

/*------------------------------------------------------------------------- */
/*  GAULEG  ...  computes Gauss-Legendre quadrature points and weights */
/*    for definite integral with unit kernel */
/*    Adapted from Numerical Recipes in Fortran, p. 145 */

/*  Arguments: */
/*  N     ...  number of points */
/*  A     ...  lower limit of integration */
/*  B     ...  upper limit of integration */
/*  QUADPT ...  quadrature points or abscissas */
/*  QUADWT ...  quadrature weights */

/* Subroutine */
int gaulegfn_(integer *n, doublereal *a, doublereal *b,
doublereal *quadpt, doublereal *quadwt)
{
    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1;

    /* Builtin functions */
    /*double cos();*/  /* include math.h should handle this */

    /* Local variables */
    static integer i, j, m;
    static doublereal z, p1, p2, p3, x1, z1, dj, dn, pp, xm;


    /* Parameter adjustments */
    --quadwt;
    --quadpt;

    /* Function Body */
    dn = (doublereal) (*n);
    m = (*n + 1) / 2;
    xm = (*a + *b) / 2.;
    x1 = (*b - *a) / 2.;
    i__1 = m;
    for (i = 1; i <= i__1; ++i) {
	z = cos(((doublereal) i - .25) * 3.141592654 / (dn + .5));
L10:
	p1 = 1.;
	p2 = 0.;
	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    dj = (doublereal) j;
	    p3 = p2;
	    p2 = p1;
	    p1 = ((dj * 2. - 1.) * z * p2 - (dj - 1.) * p3) / dj;
	}
	pp = dn * (z * p1 - p2) / (z * z - 1.);
	z1 = z;
	z = z1 - p1 / pp;
	if ((d__1 = z - z1, fabs(d__1)) > 3e-14) {
	    goto L10;
	}
	quadpt[i] = xm - x1 * z;
	quadpt[*n + 1 - i] = xm + x1 * z;
	quadwt[i] = x1 * 2. / ((1. - z * z) * pp * pp);
	quadwt[*n + 1 - i] = quadwt[i];
    }

    return 0;
} /* gaulegfn_ */

/* -------------------------------------------------------------------------- */
/* Subroutine */
int bsplvbfn_(doublereal *t, integer *norder, doublereal *x, integer *left,
doublereal *biatx)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static doublereal term;
    static integer i, j;
    static doublereal saved, deltal[20], deltar[20];
    static integer jp1;


    /* Parameter adjustments */
    --biatx;
    --t;

    /* Function Body */
    j = 1;
    biatx[1] = (float)1.;
    if (j >= *norder) {
	return 0;
    }

L10:
    jp1 = j + 1;
    deltar[j - 1] = t[*left + j] - *x;
    deltal[j - 1] = *x - t[*left + 1 - j];
    saved = (float)0.;
    i__1 = j;
    for (i = 1; i <= i__1; ++i) {
	term = biatx[i] / (deltar[i - 1] + deltal[jp1 - i - 1]);
	biatx[i] = saved + deltar[i - 1] * term;
	saved = deltal[jp1 - i - 1] * term;
    }
    biatx[jp1] = saved;
    j = jp1;
    if (j < *norder) {
	goto L10;
    }

} /* bsplvbfn_ */

/* --------------------------------------------------------------------------*/
/* SPLIP ...  Computes the inner product of order M bspline functions that*/
/*              are nonzero over an interval. */
/*              For example, if the lower bound is the */
/*              knot with index 0, and the order is 4, */
/*              the active spline functions are: */
/*              B_{i4}, B_{i-1,4}, B_{i-2,4}, and B_{i-3,4} */
/*              The inner products are turned in array OUTIP in the order */
/*              (B_{i4},   B_{i4}),      (B_{i4},   B_{i-1,4}), */
/*              (B_{i4},   B_{i-2,4}),   (B_{i4},   B_{i-3,4}), */
/*              (B_{i-1,4},B_{i-1,4}),   (B_{i-1,4},B_{i-2,4}), */
/*              (B_{i-1,4},B_{i-3,4}),   (B_{i-2,3},B_{i-2,3}), */
/*              (B_{i-2,4},B_{i-3,4}),   (B_{i-3,4},B_{i-3,4}) */

/*  N  ...  length of knot sequence */
/*  X  ...  strictly increasing sequence of N knot values */
/*  INDEX ... index of lower bound of interval, must be < N */
/*  NORDER ...  order of spline < 20 */
/* OUTIP ...  array of length NORDER*(NORDER+1)/2 for returning inner products
*/
/*  IER ... error return */

/* Subroutine */
int splipfn_(integer *n, doublereal *x, integer *index, integer *norder,
doublereal *outip, integer *ier)
{
    /* System generated locals */
    integer i__1, i__2, i__3;

    /* Local variables */
    static doublereal knot[40];
    static integer i, j, k, m;
    static doublereal biatx[20], wi, quadpt[20], quadwt[20];


    /* Parameter adjustments */
    --outip;
    --x;

    /* Function Body */
    *ier = 0;
    if (*index < 1 || *index >= *n) {
	*ier = 1;
	return 0;
    }

/*  generate quadrature points and weights for Gauss-Legendre quadrature 
*/

    gaulegfn_(norder, &x[*index], &x[*index + 1], quadpt, quadwt);

    i__1 = *norder * (*norder + 1) / 2;
    for (m = 1; m <= i__1; ++m) {
	outip[m] = (float)0.;
    }

/*  first compute local knot sequence from X */

    knot[*norder - 1] = x[*index];
    knot[*norder] = x[*index + 1];
    i__1 = *norder - 1;
    for (i = 1; i <= i__1; ++i) {
	if (*index - i >= 1) {
	    knot[*norder - i - 1] = x[*index - i];
	} else {
	    knot[*norder - i - 1] = x[1];
	}
	if (*index + i + 1 <= *n) {
	    knot[*norder + i] = x[*index + i + 1];
	} else {
	    knot[*norder + i] = x[*n];
	}
    }

/*  now compute the spline values at the quadrature points */

    i__1 = *norder;
    for (i = 1; i <= i__1; ++i) {
	bsplvbfn_(knot, norder, &quadpt[i - 1], norder, biatx);
	m = 0;
	wi = quadwt[i - 1];
	i__2 = *norder;
	for (j = 1; j <= i__2; ++j) {
	    i__3 = *norder;
	    for (k = j; k <= i__3; ++k) {
		++m;
		outip[m] += wi * biatx[*norder - j] * biatx[*norder - k];
	    }
	}
    }

    return 0;
} /* splipfn_ */

/*  ---------------------------------------------------------------------- */
/*  HMATFN ... computes matrix of inner products of Bspline functions */

/*  X      ...  strictly ascending sequence of values */
/*  N      ...  length of the sequence */
/*  NMO    ...  number of rows of band-structured matrix containing inner */
/*               products:  NMO = N - NORDER.  Number of columns = NORDER */
/*  NORDER ... order of differential operator in spline penalty term */
/*             permissible values:  3  ...  4 */
/* H      ...  band structured matrix with NORDER columns containing nonzero*/
/*             inner products */
/*  OUTIP  ...  scratch array of length NORDER*(NORDER+1)/2 */
/*  SPCWRD ...  logical flag indicating that X values are equally spaced */

/* Subroutine */
int hmatfn_(integer *n, integer *nmo, integer *norder, doublereal *x,
doublereal *h, doublereal *outip, logical *spcwrd)
{
    /* System generated locals */
    integer h_dim1, h_offset, i__1, i__2, i__3;

    /* Local variables */
    static integer nmnorder, imjp1, kmjp1, i, j, k, m;
    static doublereal delta, hi1, hi2;
    static integer nmnorderpj, ier;


/*  clear array */

    /* Parameter adjustments */
    --outip;
    h_dim1 = *nmo;
    h_offset = h_dim1 + 1;
    h -= h_offset;
    --x;

    /* Function Body */
    i__1 = *nmo;
    for (i = 1; i <= i__1; ++i) {
	i__2 = *norder;
	for (j = 1; j <= i__2; ++j) {
	    h[i + j * h_dim1] = (float)0.;
	}
    }

/* ----------------------------  order 1  -------------------------------
-*/

    if (*norder == 1) {
	if (*spcwrd) {
	    delta = x[2] - x[1];
	    i__1 = *n - 1;
	    for (i = 1; i <= i__1; ++i) {
		h[i + h_dim1] = delta;
	    }
	} else {
	    i__1 = *n - 1;
	    for (i = 1; i <= i__1; ++i) {
		h[i + h_dim1] = x[i + 1] - x[i];
	    }
	}
	return 0;
    }

/* ----------------------------  order 2  -------------------------------
-*/

    if (*norder == 2) {
	if (*spcwrd) {
	    hi1 = (x[3] - x[1]) / 3.;
	    hi2 = (x[2] - x[1]) / 6.;
	    i__1 = *n - 2;
	    for (i = 1; i <= i__1; ++i) {
		h[i + h_dim1] = hi1;
		if (i == 1) {
		    h[i + (h_dim1 << 1)] = 0.;
		} else {
		    h[i + (h_dim1 << 1)] = hi2;
		}
	    }
	} else {
	    i__1 = *n - 2;
	    for (i = 1; i <= i__1; ++i) {
		h[i + h_dim1] = (x[i + 2] - x[i]) / 3.;
		if (i == 1) {
		    h[i + (h_dim1 << 1)] = 0.;
		} else {
		    h[i + (h_dim1 << 1)] = (x[i + 1] - x[i]) / 6.;
		}
	    }
	}
	return 0;
    }

/* ----------------------  order 3 or up  ---------------------------*/

    if (*norder > 2) {
	nmnorder = *n - *norder;
	if (*spcwrd) {
	    i__1 = *norder + 1;
	    splipfn_(n, &x[1], &i__1, norder, &outip[1], &ier);
	    if (ier != 0) {
		ier += 10;
		return 0;
	    }
	    i__1 = *n - 1;
	    for (i = 1; i <= i__1; ++i) {
		m = 0;
		i__2 = *norder;
		for (j = 1; j <= i__2; ++j) {
		    imjp1 = i - j + 1;
		    nmnorderpj = nmnorder + j;
		    i__3 = *norder;
		    for (k = j; k <= i__3; ++k) {
			++m;
			kmjp1 = k - j + 1;
			if (i > k - 1 && i < nmnorderpj) {
			    h[imjp1 + kmjp1 * h_dim1] += outip[m];
			}
		    }
		}
	    }
	} else {
	    i__1 = *n - 1;
	    for (i = 1; i <= i__1; ++i) {
		splipfn_(n, &x[1], &i, norder, &outip[1], &ier);
		if (ier != 0) {
		    ier += 10;
		    return 0;
		}
		m = 0;
		i__2 = *norder;
		for (j = 1; j <= i__2; ++j) {
		    imjp1 = i - j + 1;
		    nmnorderpj = nmnorder + j;
		    i__3 = *norder;
		    for (k = j; k <= i__3; ++k) {
			++m;
			kmjp1 = k - j + 1;
			if (i > k - 1 && i < nmnorderpj) {
			    h[imjp1 + kmjp1 * h_dim1] += outip[m];
			}
		    }
		}
	    }
	}
    }

    return 0;
} /* hmatfn_ */

/* -------------------------------------------------------------------------*/
/*  DIVDIFFFN  ...  computes divided difference coefficients up to order N */
/*          for argument value array of length N, N >= 2 */

/* Subroutine */
int divdifffn_(integer *n, doublereal *x, doublereal *c, doublereal *wk)
{
    /* System generated locals */
    integer wk_dim1, wk_offset, i__1, i__2, i__3;

    /* Local variables */
    static integer i, j, k;
    static doublereal dj1;
    static integer nm1;
    static doublereal djk;


    /* Parameter adjustments */
    wk_dim1 = *n;
    wk_offset = wk_dim1 + 1;
    wk -= wk_offset;
    --c;
    --x;

    /* Function Body */
    if (*n == 1) {
	c[1] = (float)1.;
    }

/*  set up coefficients for order 2 */

    nm1 = *n - 1;
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	i__2 = nm1;
	for (j = 1; j <= i__2; ++j) {
	    wk[i + j * wk_dim1] = (float)0.;
	}
    }
    i__1 = nm1;
    for (j = 1; j <= i__1; ++j) {
	dj1 = x[j + 1] - x[j];
	wk[j + j * wk_dim1] = (float)-1. / dj1;
	wk[j + 1 + j * wk_dim1] = (float)1. / dj1;
    }

/*  recurse up to order n - 2 */

    i__1 = *n - 2;
    for (k = 1; k <= i__1; ++k) {
	i__2 = nm1 - k;
	for (j = 1; j <= i__2; ++j) {
	    djk = x[j + k + 1] - x[j];
	    i__3 = j + k + 1;
	    for (i = j; i <= i__3; ++i) {
		wk[i + j * wk_dim1] = (wk[i + (j + 1) * wk_dim1] - wk[i + j * 
			wk_dim1]) / djk;
	    }
	}
    }

/*  return divided difference coefficients times final difference */

    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	c[i] = wk[i + wk_dim1] * djk;
    }

    return 0;
} /* divdifffn_ */

/*  ---------------------------------------------------------------------- */
/*  GTWGFN ... computes cross-product of divided difference coefficients */
/*               with respect to x and weights w */
/*              That is, computes G'WG where G is N by N-NORDER differencing*/
/*               matrix and W is a diagonal weight matrix */

/*  N  ...  length of the sequence */
/*  NORDER ... order of differential operator in spline penalty term */
/*             permissible values:  1  ...  4 */
/*  M ...   N - NORDER */
/*  X  ...  strictly ascending sequence of N values */
/*  W  ...  N positive weights */
/* GTWG  ...  resulting N - NORDER by NORDER + 1 matrix in band structured mod
e*/
/*  WK    ...  working array of length NORDER */
/*  C     ...  working array of length NORDER*NORDER */
/*  SPCWRD ..  logical flag indicating equal spacing of the X values */

/* Subroutine */
int gtwgfn_(integer *n, integer *norder, doublereal *x, doublereal *w,
doublereal *work, doublereal *wk, doublereal *c, logical *spcwrd)
{
    /* System generated locals */
    integer i__1, i__2, i__3;

    /* Local variables */
    static integer iboffset, igoffset, iqoffset, nmnorder, i, j, l;
    static integer nsize, nordp1, mj;
    static doublereal sum;


    /* Parameter adjustments */
    c -= 21;
    --wk;
    --work;
    --w;
    --x;

    /* Function Body */
    nordp1 = *norder + 1;
    nmnorder = *n - *norder;
    nsize = nmnorder * nordp1;
    igoffset = nmnorder * *norder;
    iboffset = igoffset + nsize;
    iqoffset = iboffset + nsize;

    if (*spcwrd) {
	divdifffn_(&nordp1, &x[1], &c[21], &wk[1]);
	i__1 = nmnorder;
	for (i = 1; i <= i__1; ++i) {
	    mj = i;
	    i__2 = nordp1;
	    for (j = 1; j <= i__2; ++j) {
		work[iqoffset + mj] = c[j + 20];
		mj += nmnorder;
	    }
	    mj = i;
	    i__2 = min(i,nordp1);
	    for (j = 1; j <= i__2; ++j) {
		sum = 0.;
		i__3 = *norder + 2 - j;
		for (l = 1; l <= i__3; ++l) {
		    sum += c[l + 20] * c[l + j + 19] * w[i + l - 1];
		}
		work[igoffset + mj] = sum;
		mj += nmnorder;
	    }
	}
    } else {
	i__1 = nmnorder;
	for (i = 1; i <= i__1; ++i) {
	    divdifffn_(&nordp1, &x[i], &c[21], &wk[1]);
	    mj = i;
	    i__2 = nordp1;
	    for (j = 1; j <= i__2; ++j) {
		work[iqoffset + mj] = c[j + 20];
		mj += nmnorder;
	    }
	    mj = i;
	    i__2 = min(i,nordp1);
	    for (j = 1; j <= i__2; ++j) {
		sum = 0.;
		i__3 = *norder + 2 - j;
		for (l = 1; l <= i__3; ++l) {
		    sum += c[l + 20] * c[l + j - 1 + j * 20] * w[i + l - 1];
		}
		work[igoffset + mj] = sum;
		mj += nmnorder;
	    }
	    i__2 = nordp1;
	    for (l = 1; l <= i__2; ++l) {
		i__3 = *norder;
		for (j = 1; j <= i__3; ++j) {
		    c[l + (*norder + 2 - j) * 20] = c[l + (nordp1 - j) * 20];
		}
	    }
	}
    }

/*  clear upper right triangle */

    mj = nmnorder;
    i__1 = *norder;
    for (j = 1; j <= i__1; ++j) {
	i__2 = j;
	for (i = 1; i <= i__2; ++i) {
	    work[mj + i + igoffset] = (float)0.;
	}
	mj += nmnorder;
    }

/*     write (*,'(a)') ' H:' */
/*     do i=1,nmnorder */
/*       write (*,'(i3,5f12.4)') i, */
/*    1        (work((j-1)*nmnorder+i),j=1,norder) */
/*     end do */
/*     write (*,'(a)') ' GtG:' */
/*     do i=1,nmnorder */
/*       write (*,'(i3,5e12.4)') i, */
/*    1        (work((j-1)*nmnorder+i+igoffset),j=1,nordp1) */
/*     end do */
/*     write (*,'(a)') ' Q:' */
/*     do i=1,nmnorder */
/*       write (*,'(i3,5f12.4)') i, */
/*    1        (work((j-1)*nmnorder+i+iqoffset),j=1,nordp1) */
/*     end do */

    return 0;
} /* gtwgfn_ */

/*  ------------------------------------------------------------------- */
/*  LDLTBD ...  computes rational Choleski factorization */
/*            A = LDL' for a banded matrix A */

/*  N  ... order of matrix */
/*  K  ... number of off-diagonal bands + 1 */
/*  ABAND  ... N by K matrix ... diagonal in 1st column, and */
/*         column j contains the N - j + 1 nonzero elements */
/*         of off-diagonal band j starting in row j */
/*         On exit, the first column contains the values of D, and */
/*         remaining columns contain the off-diagonal values of L */
/*  IER  ...  error return:  0 means no error, */
/*                           1 means N < 1 */
/*                           2 means K < 1 */
/*                           3 means K > N */
/*                         -J means zero or negative element for D found on*/
/*                             loop J */

/* Subroutine */
int ldltbdspl_(integer *n, integer *k, doublereal *aband, integer *ier)
{
    /* System generated locals */
    integer aband_dim1, aband_offset, i__1, i__2, i__3, i__4;

    /* Local variables */
    static integer iend, i, j, l;
    static doublereal vj;
    static integer jmi, ist, lst;
    static doublereal sum;


    /* Parameter adjustments */
    aband_dim1 = *n;
    aband_offset = aband_dim1 + 1;
    aband -= aband_offset;

    /* Function Body */
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
/* Computing MAX */
	i__2 = 1, i__3 = j - *k + 1;
	ist = max(i__2,i__3);
	i__2 = j - 1;
	for (i = ist; i <= i__2; ++i) {
	    jmi = j - i;
	    aband[jmi + *k * aband_dim1] = aband[j + (jmi + 1) * aband_dim1] *
		     aband[i + aband_dim1];
	}
	sum = aband[j + aband_dim1];
	i__2 = j - 1;
	for (i = ist; i <= i__2; ++i) {
	    jmi = j - i;
	    sum -= aband[j + (jmi + 1) * aband_dim1] * aband[jmi + *k * 
		    aband_dim1];
	}
	vj = sum;
	if (vj <= 0.) {
	    *ier = -j;
	    return 0;
	}
	aband[j + aband_dim1] = sum;
/* Computing MIN */
	i__2 = *n, i__3 = j + *k - 1;
	iend = min(i__2,i__3);
	i__2 = iend;
	for (i = j + 1; i <= i__2; ++i) {
	    sum = aband[i + (i - j + 1) * aband_dim1];
/* Computing MAX */
	    i__3 = 1, i__4 = i - *k + 1;
	    lst = max(i__3,i__4);
	    i__3 = j - 1;
	    for (l = lst; l <= i__3; ++l) {
		sum -= aband[i + (i - l + 1) * aband_dim1] * aband[j - l + *k 
			* aband_dim1];
	    }
	    aband[i + (i - j + 1) * aband_dim1] = sum / vj;
	}
    }

/*  clean up working storage */

    i__1 = *k - 1;
    for (i = 1; i <= i__1; ++i) {
	aband[i + *k * aband_dim1] = 0.;
    }

    return 0;
} /* ldltbdspl_ */

/*  ---------------------------------------------------------------------- */
/*  GDIFFFN ... computes differences for a vector y with respect to x */
/*              That is, computes G'y where G is N by N-NORDER differencing*/
/*               matrix. */

/*  N  ...  length of the sequence */
/*  NORDER ... order of differential operator in spline penalty term */
/*             permissible values:  3  ...  4 */
/*  X  ...  strictly ascending sequence of values */
/*  Y  ...  sequence to be differenced */

/* Subroutine */
int gdifffn_(integer *n, integer *norder, doublereal *x, doublereal *y,
doublereal *wk, doublereal *c)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer i, j;
    static integer nordp1;
    static doublereal sum;


    /* Parameter adjustments */
    --c;
    --wk;
    --y;
    --x;

    /* Function Body */
    nordp1 = *norder + 1;
    i__1 = *n - *norder;
    for (i = 1; i <= i__1; ++i) {
	divdifffn_(&nordp1, &x[i], &c[1], &wk[1]);
	sum = 0.;
	i__2 = nordp1;
	for (j = 1; j <= i__2; ++j) {
	    sum += y[i + j - 1] * c[j];
	}
	y[i] = sum;
    }

    return 0;
} /* gdifffn_ */

/*  ------------------------------------------------------------------- */
/*  SOLVBD ...  computes the solution to the equation */
/*                Ax = y for a symmetric banded matrix A */
/*            given the rational Choleski factorization  A = LDL' */

/*  N  ... order of matrix */
/*  K  ... number of off-diagonal bands + 1 */
/*  M  ... number of columns of right side array Y */
/*  LBAND  ... N by K matrix ... D in 1st column, and */
/*        column j contains the N - j + 1 nonzero elements of lower triangular
*/
/*         Choleski factor L */
/*         off-diagonal band j starting in row j */
/*  Y  ... N by M array containing right side.  On return it contains */
/*         the solutions */
/*  IER  ...  error return:  0 means no error, */
/*                           1 means N < 1 */
/*                           2 means K < 1 */
/*                           3 means K > N */
/*                          J + 10 means zero or negative element for D found 
on*/
/*                             loop J */

/* Subroutine */
int solvbdspl_(integer *n, integer *k, integer *m, doublereal *lband,
doublereal *y, integer *ier)
{
    /* System generated locals */
    integer lband_dim1, lband_offset, y_dim1, y_offset, i__1, i__2, i__3;

    /* Local variables */
    static integer i, j, icomp, jcomp, jrs, ist;
    static doublereal sum;


/*  check arguments */

    /* Parameter adjustments */
    y_dim1 = *n;
    y_offset = y_dim1 + 1;
    y -= y_offset;
    lband_dim1 = *n;
    lband_offset = lband_dim1 + 1;
    lband -= lband_offset;

    /* Function Body */
    if (*n < 1) {
	*ier = 1;
	return 0;
    }
    if (*k < 1) {
	*ier = 2;
	return 0;
    }
    if (*k > *n) {
	*ier = 3;
	return 0;
    }
    if (*m < 1) {
	*ier = 4;
	return 0;
    }
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
	if (lband[j + lband_dim1] <= 0.) {
	    *ier = j + 10;
	    return 0;
	}
    }

/*  Solve  Lu = y */

    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
/* Computing MAX */
	i__2 = 1, i__3 = j - *k + 1;
	ist = max(i__2,i__3);
	i__2 = *m;
	for (jrs = 1; jrs <= i__2; ++jrs) {
	    sum = y[j + jrs * y_dim1];
	    i__3 = j - 1;
	    for (i = ist; i <= i__3; ++i) {
		sum -= lband[j + (j - i + 1) * lband_dim1] * y[i + jrs * 
			y_dim1];
	    }
	    y[j + jrs * y_dim1] = sum;
	}
    }

/*  Solve Dv = u */

    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
	i__2 = *m;
	for (jrs = 1; jrs <= i__2; ++jrs) {
	    y[j + jrs * y_dim1] /= lband[j + lband_dim1];
	}
    }

/*  Solve  L'x = v */

    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
	jcomp = *n - j + 1;
/* Computing MAX */
	i__2 = 1, i__3 = j - *k + 1;
	ist = max(i__2,i__3);
	i__2 = *m;
	for (jrs = 1; jrs <= i__2; ++jrs) {
	    sum = y[jcomp + jrs * y_dim1];
	    i__3 = j - 1;
	    for (i = ist; i <= i__3; ++i) {
		icomp = *n - i + 1;
		sum -= lband[icomp + (j - i + 1) * lband_dim1] * y[icomp + 
			jrs * y_dim1];
	    }
	    y[jcomp + jrs * y_dim1] = sum;
	}
    }

    return 0;
} /* solvbdspl_ */

/*  ---------------------------------------------------------------------- */
/*  GCFN ... computes  GC where G is N by N-NORDER differencing */
/*               matrix and C is a vector of length N-NORDER */

/*  N      ...  length of the sequence */
/*  NORDER ...  order of differential operator in spline penalty term */
/*              permissible values:  1  ...  4 */
/*  X      ...  strictly ascending sequence of N values */
/*  W      ...  sequence of N positive weights */
/*  CVEC   ...  N - NORDER vector */
/*  y      ...  vector of length N containing values to be smoothed: */
/*          resulting N vector is Y - lambda*G W C */

/* Subroutine */
int gcfn_(integer *n, integer *norder, doublereal *x, doublereal *w,
doublereal *cvec, doublereal *y, doublereal *lambda, doublereal *wk,
doublereal *c)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer iplm1, i, l;
    static doublereal factr;
    static integer nordp1;


    /* Parameter adjustments */
    --c;
    --wk;
    --y;
    --cvec;
    --w;
    --x;

    /* Function Body */
    nordp1 = *norder + 1;
    i__1 = *n - *norder;
    for (i = 1; i <= i__1; ++i) {
	factr = *lambda * cvec[i];
	divdifffn_(&nordp1, &x[i], &c[1], &wk[1]);
	i__2 = nordp1;
	for (l = 1; l <= i__2; ++l) {
	    iplm1 = i + l - 1;
	    y[iplm1] -= factr * c[l] * w[iplm1];
	}
    }

    return 0;
} /* gcfn_ */

/* f */
/*  --------------------------------------------------------------------- */
/*  BDINV  ...  invert a band-structured matrix of order N and bandwidth */
/*              M that has been replaced by its rational Choleksi decomp. */
/*              On completion the inverse overwrites the input matrix X */

/*  N  ...  order of the matrix */
/*  M  ... number of off-diagonal bands ... M+1 = number of cols of X */
/*  X  ...  band-structured matrix containing Choleski factors */
/*  IER  ...  error return */

/* Subroutine */
int bdinvspl_(integer *n, integer *m, doublereal *x, integer *ier)
{
    /* System generated locals */
    integer x_dim1, x_offset, i__1, i__2;

    /* Local variables */
    static integer ilim, i, k, l, kp1, lp1, mp1, ipk, ipl;
    static doublereal sum;


/*  check for zero diagonal entries */

    /* Parameter adjustments */
    x_dim1 = *n;
    x_offset = x_dim1 + 1;
    x -= x_offset;

    /* Function Body */
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	if (x[i + x_dim1] <= 0.) {
	    *ier = i + 10;
	    return 0;
	}
    }

    mp1 = *m + 1;
    ilim = 1;
    x[*n + x_dim1] = 1. / x[*n + x_dim1];
    for (i = *n - 1; i >= 1; --i) {
	i__1 = ilim;
	for (l = 1; l <= i__1; ++l) {
	    sum = 0.;
	    lp1 = l + 1;
	    ipl = i + l;
	    i__2 = ilim;
	    for (k = 1; k <= i__2; ++k) {
		kp1 = k + 1;
		ipk = i + k;
		if (k == l) {
		    sum -= x[ipk + kp1 * x_dim1] * x[ipl + x_dim1];
		} else if (k > l) {
		    sum -= x[ipk + kp1 * x_dim1] * x[ipk + (k - l + 1) * 
			    x_dim1];
		} else {
		    sum -= x[ipk + kp1 * x_dim1] * x[ipl + (l - k + 1) * 
			    x_dim1];
		}
	    }
	    x[l + mp1 * x_dim1] = sum;
	}
	sum = 1. / x[i + x_dim1];
	i__1 = ilim;
	for (l = 1; l <= i__1; ++l) {
	    sum -= x[i + l + (l + 1) * x_dim1] * x[l + mp1 * x_dim1];
	}
	x[i + x_dim1] = sum;
	i__1 = ilim;
	for (l = 1; l <= i__1; ++l) {
	    x[i + l + (l + 1) * x_dim1] = x[l + mp1 * x_dim1];
	}
	if (ilim < *m) {
	    ++ilim;
	}
    }

/*  clear upper triangle */

    i__1 = *m;
    for (l = 1; l <= i__1; ++l) {
	x[l + mp1 * x_dim1] = 0.;
    }

    return 0;
} /* bdinvspl_ */

/*  ------------------------------------------------------------------------*/
/*  SPLCAL ...  an O(n) spline smoother with penalty on D^m */
/*         called by a driver routine that has already set up the two */
/*         band-structured matrices required in array WORK */

/*  N       ...  number of data points */
/*  NVAR    ...  number of sets of values to be smoothed */
/*  NORDER  ...  order of derivative to be penalized (max. value = 19) */
/*  X       ...  array of strictly increasing values of length N */
/*  W       ...  array of positive weights of length N */
/*  Y       ...  N by NVAR matrix of values to be smoothed */
/*  YHAT    ...  N by NVAR matrix of values of smooth */
/*  LEV     ...  array of N leverage values */
/*  GCV     ...  output value of the GCV criterion */
/*  CV      ...  output value of the CV criterion */
/*  DF      ...  output value of the DF criterion */
/*  LAMBDA  ...  penalty parameter */
/*  WORK    ...  working storage array of length at least */
/*                  3*N*(NORDER+1) */

/*            NB:  if the data are to be re-smoothed with a different */
/*            value of LAMBDA, the returned contents of WORK must be */
/*            left untouched between the first and subsequent calls */

/*  IER     ...  error return: */
/*            0 ... no error */
/*            1 ... N < 2 */
/*            2 ... NORDER out of permissible range */
/*            3 ... X not strictly increasing */
/*            4 ... LAMBDA negative */
/*           -j ... failure in the rational Choleski decomposition routine*/
/*                  LDLTBD because of nonpositive value of diagonal element*/
/*                   at index J in the main loop */

/* Subroutine */
int splcal_(integer *n, integer *nvar, integer *norder, doublereal *x,
doublereal *w, doublereal *y, doublereal *yhat, doublereal *lev,
doublereal *gcv, doublereal *cv, doublereal *df,
doublereal *lambda, doublereal *work, integer *ier)
{
    /* System generated locals */
    integer y_dim1, y_offset, yhat_dim1, yhat_offset, i__1, i__2, i__3;
    doublereal d__1;

    /* Local variables */
    static integer ivar, iboffset, igoffset, iqoffset;
    static integer nmnorder, iyoffset, nworksiz, i, j, l, m, nband;
    static doublereal trace;
    static integer nsize, l1, l2;
    static integer ml;
    static doublereal xn;
    static integer ml1, ml2;
    static doublereal wk1[400], wk2[400], fac;
    static integer ldn;
    static doublereal res, sse;
    static integer lup;
    static doublereal sum;


/*  set up offset values for storing information in array WORK */

    /* Parameter adjustments */
    --work;
    --lev;
    yhat_dim1 = *n;
    yhat_offset = yhat_dim1 + 1;
    yhat -= yhat_offset;
    y_dim1 = *n;
    y_offset = y_dim1 + 1;
    y -= y_offset;
    --w;
    --x;

    /* Function Body */
    nmnorder = *n - *norder;
    nband = *norder + 1;
    nsize = nmnorder * nband;
    igoffset = nmnorder * *norder;
    iboffset = igoffset + nsize;
    iqoffset = iboffset + nsize;
    iyoffset = iqoffset + nsize;
    nworksiz = nmnorder * ((*norder << 2) + 3) + *n;

/*    The next result is a band-structured matrix of order N - NORDER */
/*    and bandwidth  NORDER + 1 of the form  B = H + LAMDA * G^t Wd G */
/*    where Wd = diag(W) and G contains the divided difference coeffs. */

    m = 0;
    i__1 = *norder;
    for (j = 1; j <= i__1; ++j) {
	i__2 = nmnorder;
	for (i = 1; i <= i__2; ++i) {
	    ++m;
	    work[iboffset + m] = work[m] + *lambda * work[igoffset + m];
	}
    }
    i__1 = nmnorder;
    for (i = 1; i <= i__1; ++i) {
	++m;
	work[iboffset + m] = *lambda * work[igoffset + m];
    }

/*  ********************  call LBANDMAT  ************************** */
/*    This step computes the rational Choleski decomposition of */
/*    the above band-structured matrix B */

    ldltbdspl_(&nmnorder, &nband, &work[iboffset + 1], ier);
    if (*ier != 0) {
	  return 0;
    }

/*  ++++++++++++++  loop through values to be smoothed  +++++++++++ */

    i__1 = *nvar;
    for (ivar = 1; ivar <= i__1; ++ivar) {

/*  ********************  call GDIFFFUN  ************************** */
/*    This step computes the divided difference values GtY of Y */

	i__2 = *n;
	for (i = 1; i <= i__2; ++i) {
	    work[iyoffset + i] = y[i + ivar * y_dim1];
	}
	gdifffn_(n, norder, &x[1], &work[iyoffset + 1], wk1, wk2);

/*  ********************  call SOLVBD  ************************** */
/*    This step solves the equation  BC = GtY.  C replaces GtY */

	solvbdspl_(&nmnorder, &nband, &c__1, &work[iboffset + 1], &work[
		iyoffset + 1], ier);
	if (*ier != 0) {
	    return 0;
	}

/*  ********************  call GCVEC  ************************** */
/*    This step updates original vector Y to Y - LAMBDA * G X */

	i__2 = *n;
	for (i = 1; i <= i__2; ++i) {
	    yhat[i + ivar * yhat_dim1] = y[i + ivar * y_dim1];
	}
	gcfn_(n, norder, &x[1], &w[1], &work[iyoffset + 1], &yhat[ivar * 
		yhat_dim1 + 1], lambda, wk1, wk2);
    }

/* +++++++++++++++++  end of loop through functions to be smoothed  +++++
+*/

/*  Compute band-structured portion of inverse of matrix B */
/*  This replaces B */

    bdinvspl_(&nmnorder, norder, &work[iboffset + 1], ier);
/*     write (*,'(a)') ' B:' */
/*     do i=1,nmnorder */
/*       write (*,'(i3,5e12.4)') i, */
/*    1        (work((j-1)*nmnorder+i+iboffset),j=1,nband) */
/*     end do */

/*  compute trace of hat matrix, SSE, CV, and GCV criteria */

    xn = (doublereal) (*n);
    trace = (float)0.;
    sse = (float)0.;
    *cv = (float)0.;
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	sum = (float)0.;
/* Computing MAX */
	i__2 = 0, i__3 = i - nmnorder;
	ldn = max(i__2,i__3);
/* Computing MIN */
	i__2 = *norder, i__3 = i - 1;
	lup = min(i__2,i__3);
	ml = nmnorder * ldn;
	i__2 = lup;
	for (l = ldn; l <= i__2; ++l) {
/* Computing 2nd power */
	    d__1 = work[ml + i - l + iqoffset];
	    sum += d__1 * d__1 * work[i - l + iboffset];
	    ml += nmnorder;
	}
	ml1 = nmnorder * ldn;
	i__2 = lup - 1;
	for (l1 = ldn; l1 <= i__2; ++l1) {
	    fac = work[ml1 + i - l1 + iqoffset];
	    ml2 = nmnorder * (l1 + 1);
	    i__3 = lup;
	    for (l2 = l1 + 1; l2 <= i__3; ++l2) {
		sum += fac * (float)2. * work[ml2 + i - l2 + iqoffset] * work[
			(l2 - l1) * nmnorder + i - l1 + iboffset];
		ml2 += nmnorder;
	    }
	    ml1 += nmnorder;
	}
	sum = sum * *lambda * w[i];
	lev[i] = (float)1. - sum;
	trace += sum;
	i__2 = *nvar;
	for (ivar = 1; ivar <= i__2; ++ivar) {
	    res = (y[i + ivar * y_dim1] - yhat[i + ivar * yhat_dim1]) / w[i];
	    sse += res * res;
/* Computing 2nd power */
	    d__1 = res / sum;
	    *cv += d__1 * d__1;
	}
    }

/* Computing 2nd power */
    d__1 = (real) (*nvar) * trace / xn;
    *gcv = sse / xn / (d__1 * d__1);
    *cv /= xn;
    *df = xn - trace;

    return 0;
} /* splcal_ */

/* -----------------------------------------------------------------------*/
/* Subroutine */
int fmm_(integer *n, integer *nvar, integer *norder, doublereal *xvec,
doublereal *wvec, doublereal *yvec, doublereal *yhat, doublereal *lev,
doublereal *gcv, doublereal *cv, doublereal *df, doublereal *lambda,
integer *method, doublereal *work, doublereal *tol, integer *ier)
{
    /* System generated locals */
    integer yvec_dim1, yvec_offset, yhat_dim1, yhat_offset, i__1;
    doublereal d__1;

    /* Builtin functions */
    /*double sqrt(), log(), exp();*/
    /*double cos();*/  /* include math.h should handle these */

    /* Local variables */
    static integer igoffset, nmnorder;
    static doublereal a, b, d, e;
    static integer i;
    static doublereal p, q, r, u, v, w, x, ratio, t1, t2, fu, fv, fw, fx, xm, 
	    targdf;
    static doublereal eps, tol1, tol2;


    /* Parameter adjustments */
    --work;
    --lev;
    yhat_dim1 = *n;
    yhat_offset = yhat_dim1 + 1;
    yhat -= yhat_offset;
    yvec_dim1 = *n;
    yvec_offset = yvec_dim1 + 1;
    yvec -= yvec_offset;
    --wvec;
    --xvec;

    /* Function Body */
    targdf = *df;
    nmnorder = *n - *norder;
    igoffset = nmnorder * *norder;
    t1 = (float)0.;
    t2 = (float)0.;
    i__1 = nmnorder;
    for (i = 1; i <= i__1; ++i) {
	t1 += work[i];
	t2 += work[igoffset + i];
    }
    ratio = t1 / t2;

    eps = 1.;
L10:
    eps /= 2.;
    tol1 = eps + 1.;
    if (tol1 > 1.) {
	goto L10;
    }
    eps = sqrt(eps);

/*  ----------------  initialization of lambda  ------------------------- 
*/

    a = (float)1e-10;
    b = (float)3.;
/*     v = a + 0.382*(b - a) */
/*     write (*, '(a,e10.3)') ' LAMBDA =', lambda */
    if (*lambda <= 0.) {
	v = (float).75;
    } else {
	v = (log(*lambda / ratio) / (float)2.772589 + (float)2.) / (float)6.;
    }
    w = v;
    x = v;
    e = (float)0.;
    *lambda = ratio * exp((x * (float)6. - (float)2.) * (float)2.772589);
/*     write (*, '(a,e10.3)') ' LAMBDA =', lambda */

/*  Call 1 to SPLCAL */

    splcal_(n, nvar, norder, &xvec[1], &wvec[1], &yvec[yvec_offset], &yhat[
	    yhat_offset], &lev[1], gcv, cv, df, lambda, &work[1], ier);
    if (*ier != 0) {
	return 0;
    }
/*     write (*,'(a, f10.5,e10.3,4f12.4)') */
/*    1                   ' Call 1:', x, lambda, df, gcv, cv, fu */

    if (*method == 2) {
/* Computing 2nd power */
	d__1 = targdf - *df;
	fx = d__1 * d__1;
    }
    if (*method == 3) {
	fx = *gcv;
    }
    if (*method == 4) {
	fx = *cv;
    }
    fv = fx;
    fw = fx;

/*  --------------------  main loop starts here ------------------------- 
*/

L20:
    xm = (a + b) * (float).5;
    tol1 = eps * fabs(x) + *tol / 3.;
    tol2 = tol1 * 2.;

/*  check stopping criterion */

    if ((d__1 = x - xm, fabs(d__1)) <= tol2 - (b - a) * (float).5) {
	goto L90;
    }

/* is golden-section necessary? */

    if (fabs(e) <= tol1) {
	goto L40;
    }

/*  fit parabola */

    r = (x - w) * (fx - fv);
    q = (x - v) * (fx - fw);
    p = (x - v) * q - (x - w) * r;
    q = (q - r) * (float)2.;
    if (q > 0.) {
	p = -p;
    }
    q = fabs(q);
    r = e;
    e = d;

/*  is parabola acceptable? */

/* L30: */
    if (fabs(p) >= (d__1 = q * (float).5 * r, fabs(d__1))) {
	goto L40;
    }
    if (p <= q * (a - x)) {
	goto L40;
    }
    if (p >= q * (b - x)) {
	goto L40;
    }

/*  a parabolic interpolation step */

    d = p / q;
    u = x + d;

/*  f must not be evaluated too close to a or b */

    if (u - a < tol2 || b - u < tol2) {
	if (xm - x >= 0.) {
	    d = tol1;
	} else {
	    d = -tol1;
	}
    }
    goto L50;

/*  a golden-section step */

L40:
    if (x >= xm) {
	e = a - x;
    }
    if (x < xm) {
	e = b - x;
    }
    d = e * (float).382;

/*  f must not be evaluated too close to x */

L50:
    if (fabs(d) >= tol1) {
	u = x + d;
    } else {
	if (d >= 0.) {
	    u = x + tol1;
	} else {
	    u = x - tol1;
	}
    }
    *lambda = ratio * exp((u * (float)6. - (float)2.) * (float)2.772589);

/*  Call 2 to SPLCAL */

    splcal_(n, nvar, norder, &xvec[1], &wvec[1], &yvec[yvec_offset], &yhat[
	    yhat_offset], &lev[1], gcv, cv, df, lambda, &work[1], ier);
    if (*ier != 0) {
	return 0;
    }
    if (*method == 2) {
/* Computing 2nd power */
	d__1 = targdf - *df;
	fu = d__1 * d__1;
    }
    if (*method == 3) {
	fu = *gcv;
    }
    if (*method == 4) {
	fu = *cv;
    }
/*     write (*,'(a, f10.5,e10.3,4f12.4)') */
/*    1                   ' Call 2:', u, lambda, df, gcv, cv, fu */

/*  update  a, b, v, w, and x */

    if (fu > fx) {
	goto L60;
    }
    if (u >= x) {
	a = x;
    }
    if (u < x) {
	b = x;
    }
    v = w;
    fv = fw;
    w = x;
    fw = fx;
    x = u;
    fx = fu;
    goto L20;

L60:
    if (u < x) {
	a = u;
    }
    if (u >= x) {
	b = u;
    }
    if (fu <= fw) {
	goto L70;
    }
    if (w == x) {
	goto L70;
    }
    if (fu <= fv) {
	goto L80;
    }
    if (v == x) {
	goto L80;
    }
    if (v == w) {
	goto L80;
    }
    goto L20;

L70:
    v = w;
    fv = fw;
    w = u;
    fw = fu;
    goto L20;

L80:
    v = u;
    fv = fu;
    goto L20;

/*  -------------------  end of main loop  ------------------------------ 
*/

L90:

    return 0;
} /* fmm_ */

/*  ------------------------------------------------------------------------*/
/*  PSPLINE ...  an O(n) spline smoother with penalty on D^m */
/*     This version can save intermediate results for reruns with new */
/*     values of smoothing parameter LAMBDA and can compute the */
/*     GCV, CV, and DF criteria */

/*  This program sets up the necessary two band-structured matrices, */
/*    and then either proceeds to a smooth calling SPLCAL */
/*    if the criterion value is fixed, */
/*   or calls FMM to optimize the smoothing criterion.  In the latter case*/

/*  N        ...  number of data points */
/*  NVAR     ...  number of sets of function values to be smoothed */
/*  NORDER   ...  order of derivative to be penalized (max. value = 19) */
/*  X        ...  array of strictly increasing values of length N */
/*  W        ...  array of positive weights of length N */
/*  Y        ...  matrix of values to be smoothed of dimension N by NVAR */
/*  YHAT     ...  matrix of values of    smooths  of dimension N by NVAR */
/*  LEV      ...  array of N leverage values */
/*  GCV      ...  output value of the GCV criterion */
/*  CV       ...  output value of the CV  criterion */
/*  DF       ...  output value of the  DF criterion */
/*  LAMBDA   ...  penalty parameter */
/*  DFMAX    ...  largest tolerated degrees of freedom */
/*  WORK     ...  working storage array of length at least */
/*                  (N-NORDER)*(3*NORDER+2)+N */

/*            NB:  if the data are to be re-smoothed with a different */
/*            value of LAMBDA, the returned contents of WORK must be */
/*            left untouched between the first and subsequent calls */

/*  METHOD  ... method for computing the smoothing parameter: */
/*            1  ...  fixed value of LAMBDA */
/*            2  ...  fixed value of DF */
/*            3  ...  LAMBDA optimizes GCV criterion */
/*            4  ...  LAMBDA optimizes  CV criterion */
/*  IRERUN  ... if nonzero, assume that a previous call has already */
/*             computed arrays H and GtWG */
/*  IER     ...  error return: */
/*            0 ... no error */
/*            1 ... N < 2*NORDER + 1 */
/*            2 ... NORDER out of permissible range: [1,10] */
/*            3 ... NVAR < 1 */
/*            4 ... LAMBDA negative */
/*            5 ... X not strictly increasing */
/*            6 ... W contains nonpositive elements */
/*           -j ... failure in the rational Choleski decomposition routine*/
/*                  LDLTBD because of nonpositive value of diagonal element*/
/*                   at index J in the main loop */

/*  See also subroutine SPLIFIT below that evaluates a spline smoothing */
/*  function or one of its derivatives at user-specified argument values. */

/* Subroutine */
int pspline_(integer *n, integer *nvar, integer *norder,
doublereal *x, doublereal *w, doublereal *y, doublereal *yhat,
doublereal *lev, doublereal *gcv, doublereal *cv, 
doublereal *df, doublereal *lambda, doublereal *dfmax,
doublereal *work, integer *method, integer *irerun, integer *ier)
{
    /* Initialized data */

    static doublereal eps = 1e-7;
    static doublereal tol = .001;

    /* System generated locals */
    integer y_dim1, y_offset, yhat_dim1, yhat_offset, i__1;
    doublereal d__1;

    /* Local variables */
    static integer nmnorder, i;
    static doublereal delta, range, xi;
    static doublereal critrn;
    static logical spcwrd;
    static doublereal wk1[121], wk2[121];
    static doublereal xim1;

    /* Parameter adjustments */
    --work;
    --lev;
    yhat_dim1 = *n;
    yhat_offset = yhat_dim1 + 1;
    yhat -= yhat_offset;
    y_dim1 = *n;
    y_offset = y_dim1 + 1;
    y -= y_offset;
    --w;
    --x;

    /* Function Body */

/*  check arguments */

    if (*n <= (*norder << 1) + 1) {
	*ier = 1;
	return 0;
    }
    if (*norder <= 1 || *norder > 10) {
	*ier = 2;
	return 0;
    }
    if (*nvar < 1) {
	*ier = 3;
	return 0;
    }
    if (*lambda < 0.) {
	*ier = 4;
	return 0;
    }

/*  Check for x strictly increasing, and also for x being equally spaced. 
*/
/*  It might save time if this were done prior to calling SPLGCV. */

    range = x[*n] - x[1];
    delta = range / (real) (*n - 1);
    spcwrd = TRUE_;
    critrn = range * eps;
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	if (w[i] <= 0.) {
	    *ier = 6;
	}
	xi = x[i];
	if (spcwrd && i > 1 && (d__1 = xi - xim1 - delta, fabs(d__1)) > critrn)
		 {
	    spcwrd = FALSE_;
	}
	if (i >= *norder && xi <= x[i - *norder + 1]) {
	    *ier = 5;
	}
	xim1 = xi;
    }
    if (*ier != 0) {
	return 0;
    }

    nmnorder = *n - *norder;

/*  if this is a re-run with a new lambda value, skip computation of H and
 */
/*   GtWG */

    if (*irerun != 0) {
	goto L10;
    }

/*  ********************  call HMAT  ************************** */
/*    This step computes band-structured matrix of inner products of */
/*    B-spline functions of order NORDER */

    hmatfn_(n, &nmnorder, norder, &x[1], &work[1], wk1, &spcwrd);

/*  ********************  call GTWGMAT  ************************** */
/*    This step computes the cross-product */
/*    of the N by N-NORDER matrix containing the divided difference */
/*    coefficients, the cross-product having array W as the metric */

    gtwgfn_(n, norder, &x[1], &w[1], &work[1], wk1, wk2, &spcwrd);

/*  take action depending on METHOD */

L10:
    if (*method == 1) {

/*  ********************  call SPLCAL  ************************** */

	splcal_(n, nvar, norder, &x[1], &w[1], &y[y_offset], &yhat[
		yhat_offset], &lev[1], gcv, cv, df, lambda, &work[1], ier);
    } else {

/*  ********************  call FMM  ************************** */

	fmm_(n, nvar, norder, &x[1], &w[1], &y[y_offset], &yhat[yhat_offset], 
		&lev[1], gcv, cv, df, lambda, method, &work[1], &tol, ier);
	if (*ier != 0) {
	    return 0;
	}
	if (*method > 2 && *df > *dfmax) {
	    *df = *dfmax;
	    fmm_(n, nvar, norder, &x[1], &w[1], &y[y_offset], &yhat[
		    yhat_offset], &lev[1], gcv, cv, df, lambda, &c__2, &work[
		    1], &tol, ier);
	}
    }

    return 0;
} /* pspline_ */

/* -------------------------------------------------------------------------*/
/* Subroutine */
int dpbsplvb_(doublereal *t, integer *jhigh, integer *index, doublereal *x,
integer *left, doublereal *biatx)
{
    /* Initialized data */

    static integer j = 1;

    /* System generated locals */
    integer i__1;

    /* Local variables */
    static doublereal term;
    static integer i;
    static doublereal saved, deltal[20], deltar[20];
    static integer jp1;



    /* Parameter adjustments */
    --biatx;
    --t;

    /* Function Body */

    switch ((int)*index) {
	case 1:  goto L10;
	case 2:  goto L20;
    }
L10:
    j = 1;
    biatx[1] = (float)1.;
    if (j >= *jhigh) {
	goto L99;
    }

L20:
    jp1 = j + 1;
    deltar[j - 1] = t[*left + j] - *x;
    deltal[j - 1] = *x - t[*left + 1 - j];
    saved = (float)0.;
    i__1 = j;
    for (i = 1; i <= i__1; ++i) {
	term = biatx[i] / (deltar[i - 1] + deltal[jp1 - i - 1]);
	biatx[i] = saved + deltar[i - 1] * term;
	saved = deltal[jp1 - i - 1] * term;
    }
    biatx[jp1] = saved;
    j = jp1;
    if (j < *jhigh) {
	goto L20;
    }

L99:
    return 0;
} /* dpbsplvb_ */

/*  --------------------------------------------------------------------- */
/* Subroutine */
int banfac_(doublereal *w, integer *nroww, integer *nrow, integer *nbandl,
integer *nbandu, integer *iflag)
{
    /* System generated locals */
    integer w_dim1, w_offset, i__1, i__2, i__3;

    /* Local variables */
    static integer jmax, kmax, i, j, k, midmk;
    static doublereal pivot;
    static integer nrowm1, middle;
    static doublereal factor;
    static integer ipk;



    /* Parameter adjustments */
    w_dim1 = *nroww;
    w_offset = w_dim1 + 1;
    w -= w_offset;

    /* Function Body */
    *iflag = 1;
    middle = *nbandu + 1;
/*                         w(middle,.) contains the main diagonal of  a . 
*/
    nrowm1 = *nrow - 1;
    if (nrowm1 < 0) {
	goto L999;
    } else if (nrowm1 == 0) {
	goto L900;
    } else {
	goto L1;
    }
L1:
    if (*nbandl > 0) {
	goto L10;
    }
/*                a is upper triangular. check that diagonal is nonzero . 
*/
    i__1 = nrowm1;
    for (i = 1; i <= i__1; ++i) {
	if (w[middle + i * w_dim1] == 0.) {
	    goto L999;
	}
    }
    goto L900;
L10:
    if (*nbandu <= 0) {
/*              a is lower triangular. check that diagonal is nonzero 
and */
/*                 divide each column by its diagonal . */
	i__1 = nrowm1;
	for (i = 1; i <= i__1; ++i) {
	    pivot = w[middle + i * w_dim1];
	    if (pivot == 0.) {
		goto L999;
	    }
/* Computing MIN */
	    i__2 = *nbandl, i__3 = *nrow - i;
	    jmax = min(i__2,i__3);
	    i__2 = jmax;
	    for (j = 1; j <= i__2; ++j) {
		w[middle + j + i * w_dim1] /= pivot;
	    }
	}
	return 0;
    }

/*        a  is not just a triangular matrix. construct lu factorization 
*/
    i__1 = nrowm1;
    for (i = 1; i <= i__1; ++i) {
/*                                  w(middle,i)  is pivot for i-th ste
p . */
	pivot = w[middle + i * w_dim1];
	if (pivot == 0.) {
	    goto L999;
	}
/*                 jmax  is the number of (nonzero) entries in column 
 i */
/*                     below the diagonal . */
/* Computing MIN */
	i__2 = *nbandl, i__3 = *nrow - i;
	jmax = min(i__2,i__3);
/*              divide each entry in column  i  below diagonal by pivo
t . */
	i__2 = jmax;
	for (j = 1; j <= i__2; ++j) {
	    w[middle + j + i * w_dim1] /= pivot;
	}
/*                 kmax  is the number of (nonzero) entries in row  i to */
/*                     the right of the diagonal . */
/* Computing MIN */
	i__2 = *nbandu, i__3 = *nrow - i;
	kmax = min(i__2,i__3);
/*                  subtract  a(i,i+k)*(i-th column) from (i+k)-th column */
/*                  (below row  i ) . */
	i__2 = kmax;
	for (k = 1; k <= i__2; ++k) {
	    ipk = i + k;
	    midmk = middle - k;
	    factor = w[midmk + ipk * w_dim1];
	    i__3 = jmax;
	    for (j = 1; j <= i__3; ++j) {
		w[midmk + j + ipk * w_dim1] -= w[middle + j + i * w_dim1] * 
			factor;
	    }
	}
    }
/*                                       check the last diagonal entry . 
*/
L900:
    if (w[middle + *nrow * w_dim1] != 0.) {
	return 0;
    }
L999:
    *iflag = 2;
    return 0;
} /* banfac_ */

/*  --------------------------------------------------------------------- */
/* Subroutine */
int banslv_(doublereal *w, integer *nroww, integer *nrow, integer *nbandl,
integer *nbandu, doublereal *b)
{
    /* System generated locals */
    integer w_dim1, w_offset, i__1, i__2, i__3;

    /* Local variables */
    static integer jmax, i, j, nrowm1, middle;



    /* Parameter adjustments */
    --b;
    w_dim1 = *nroww;
    w_offset = w_dim1 + 1;
    w -= w_offset;

    /* Function Body */
    middle = *nbandu + 1;
    if (*nrow == 1) {
	goto L20;
    }
    nrowm1 = *nrow - 1;
    if (*nbandl != 0) {
/*                                 forward pass */
/*            for i=1,2,...,nrow-1, subtract  right side(i)*(i-th colu
mn */
/*            of  l )  from right side  (below i-th row) . */
	i__1 = nrowm1;
	for (i = 1; i <= i__1; ++i) {
/* Computing MIN */
	    i__2 = *nbandl, i__3 = *nrow - i;
	    jmax = min(i__2,i__3);
	    i__2 = jmax;
	    for (j = 1; j <= i__2; ++j) {
		b[i + j] -= b[i] * w[middle + j + i * w_dim1];
	    }
	}
    }
/*                                 backward pass */
/*            for i=nrow,nrow-1,...,1, divide right side(i) by i-th diag- 
*/
/*            onal entry of  u, then subtract  right side(i)*(i-th column 
*/
/*            of  u)  from right side  (above i-th row). */
    if (*nbandu <= 0) {
/*                                a  is lower triangular . */
	i__1 = *nrow;
	for (i = 1; i <= i__1; ++i) {
	    b[i] /= w[i * w_dim1 + 1];
	}
	return 0;
    }
    i = *nrow;
L10:
    b[i] /= w[middle + i * w_dim1];
/* Computing MIN */
    i__1 = *nbandu, i__2 = i - 1;
    jmax = min(i__1,i__2);
    i__1 = jmax;
    for (j = 1; j <= i__1; ++j) {
	b[i - j] -= b[i] * w[middle - j + i * w_dim1];
    }
    --i;
    if (i > 1) {
	goto L10;
    }
L20:
    b[1] /= w[middle + w_dim1];

    return 0;
} /* banslv_ */

/* --------------------------------------------------------------------------*/
/* Subroutine */
int splint_(doublereal *x, doublereal *y, doublereal *t, integer *n,
integer *k, doublereal *q, doublereal *bcoef, integer *iflag)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer left, lenq;
    static integer kpkm1, kpkm2, i, j, ilp1mx;
    static integer jj;
    static doublereal xi;
    static integer km1, np1;

    /* Parameter adjustments */
    --bcoef;
    --q;
    --t;
    --y;
    --x;

    /* Function Body */
    np1 = *n + 1;
    km1 = *k - 1;
    kpkm1 = (*k << 1) - 1;
    kpkm2 = km1 << 1;
    left = *k;
    lenq = *n * (*k + km1);
    i__1 = lenq;
    for (i = 1; i <= i__1; ++i) {
	q[i] = (float)0.;
    }

/*  ***   loop over i to construct the  n  interpolation equations */

    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	xi = x[i];
/* Computing MIN */
	i__2 = i + *k;
	ilp1mx = min(i__2,np1);
/*        *** find  left  in the closed interval (i,i+k-1) such that 
*/
/*                t(left) .le. x(i) .lt. t(left+1) */
/*        matrix is singular if this is not possible */
	left = max(left,i);
	if (xi < t[left]) {
	    *iflag = 2;
	    return 0;
	}
L10:
	if (xi < t[left + 1]) {
	    goto L20;
	}
	++left;
	if (left < ilp1mx) {
	    goto L10;
	}
	--left;
	if (xi > t[left + 1]) {
	    *iflag = 2;
	    return 0;
	}

L20:
	dpbsplvb_(&t[1], k, &c__1, &xi, &left, &bcoef[1]);

	jj = i - left + 1 + (left - *k) * (*k + km1);
	i__2 = *k;
	for (j = 1; j <= i__2; ++j) {
	    jj += kpkm2;
	    q[jj] = bcoef[j];
	}
    }

/*     ***obtain factorization of  a  , stored again in  q. */

    banfac_(&q[1], &kpkm1, n, &km1, &km1, iflag);

    if (*iflag != 1) {
	*iflag = 3;
	return 0;
    }

/*     *** solve  a*bcoef = y  by backsubstitution */

    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	bcoef[i] = y[i];
    }

    banslv_(&q[1], &kpkm1, n, &km1, &km1, &bcoef[1]);

    return 0;
} /* splint_ */

/* -------------------------------------------------------------------------- */
/* Subroutine */
int dpinterv_(doublereal *xt, integer *lxt, doublereal *x,
integer *left, integer *mflag)
{
    /* Initialized data */

    static integer ilo = 1;

    static integer istep, middle, ihi;



    /* Parameter adjustments */
    --xt;

    /* Function Body */

    ihi = ilo + 1;
    if (ihi < *lxt) {
	goto L20;
    }
    if (*x >= xt[*lxt]) {
	goto L110;
    }
    if (*lxt <= 1) {
	goto L90;
    }
    ilo = *lxt - 1;
    ihi = *lxt;

L20:
    if (*x >= xt[ihi]) {
	goto L40;
    }
    if (*x >= xt[ilo]) {
	goto L100;
    }

/*              **** now x .lt. xt(ilo) . decrease  ilo  to capture  x . 
*/
    istep = 1;
L31:
    ihi = ilo;
    ilo = ihi - istep;
    if (ilo <= 1) {
	goto L35;
    }
    if (*x >= xt[ilo]) {
	goto L50;
    }
    istep <<= 1;
    goto L31;
L35:
    ilo = 1;
    if (*x < xt[1]) {
	goto L90;
    }
    goto L50;
/*              **** now x .ge. xt(ihi) . increase  ihi  to capture  x . 
*/
L40:
    istep = 1;
L41:
    ilo = ihi;
    ihi = ilo + istep;
    if (ihi >= *lxt) {
	goto L45;
    }
    if (*x < xt[ihi]) {
	goto L50;
    }
    istep <<= 1;
    goto L41;
L45:
    if (*x >= xt[*lxt]) {
	goto L110;
    }
    ihi = *lxt;

/*           **** now xt(ilo) .le. x .lt. xt(ihi) . narrow the dpinterval.
 */
L50:
    middle = (ilo + ihi) / 2;
    if (middle == ilo) {
	goto L100;
    }
/*     note. it is assumed that middle = ilo in case ihi = ilo+1 . */
    if (*x < xt[middle]) {
	goto L53;
    }
    ilo = middle;
    goto L50;
L53:
    ihi = middle;
    goto L50;
/* **** set output and return. */
L90:
    *mflag = -1;
    *left = 1;
    return 0;
L100:
    *mflag = 0;
    *left = ilo;
    return 0;
L110:
    *mflag = 1;
    if (*x == xt[*lxt]) {
	*mflag = 0;
    }
    *left = *lxt;
L111:
    if (*left == 1) {
	return 0;
    }
    --(*left);
    if (xt[*left] < xt[*lxt]) {
	return 0;
    }
    goto L111;
} /* dpinterv_ */

/* -------------------------------------------------------------------------- */
/* Subroutine */
int dpbvalue_(doublereal *t, doublereal *bcoef, integer *n, integer *k,
doublereal *x, integer *jderiv, doublereal *fofx)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static doublereal fkmj;
    static integer i, j, mflag, jcmin, jcmax, jdrvp1;
    static doublereal aj[20];
    static integer jc;
    static doublereal dl[20];
    static integer jj;
    static doublereal dr[20];
    static integer km1, ip1, imk, kmj, nmi, ilo;

    /* Parameter adjustments */
    --bcoef;
    --t;

    /* Function Body */
    *fofx = (float)0.;
    if (*jderiv >= *k) {
	return 0;
    }

    i__1 = *n + *k;
    dpinterv_(&t[1], &i__1, x, &i, &mflag);
/*     if (mflag .ne. 0) return */
/*  *** if k = 1 (and jderiv = 0), fofx = bcoef(i). */
    km1 = *k - 1;
    if (km1 == 0) {
	*fofx = bcoef[i];
	return 0;
    }

/*  *** store the k b-spline coefficients relevant for the knot dpinterval
 */
/*     (t(i),t(i+1)) in aj(1),...,aj(k) and compute dl(j) = x - t(i+1-j), 
*/
/*     dr(j) = t(i+j) - x, j=1,...,k-1 . set any of the aj not obtainable 
*/
/*     from input to zero. set any t.s not obtainable equal to t(1) or */
/*     to t(n+k) appropriately. */
    jcmin = 1;
    imk = i - *k;
    ip1 = i + 1;
    if (imk < 0) {
	jcmin = 1 - imk;
	i__1 = i;
	for (j = 1; j <= i__1; ++j) {
	    dl[j - 1] = *x - t[ip1 - j];
	}
	i__1 = km1;
	for (j = i; j <= i__1; ++j) {
	    aj[*k - j - 1] = (float)0.;
	    dl[j - 1] = dl[i - 1];
	}
    } else {
	i__1 = km1;
	for (j = 1; j <= i__1; ++j) {
	    dl[j - 1] = *x - t[ip1 - j];
	}
    }

    jcmax = *k;
    nmi = *n - i;
    if (nmi < 0) {
	jcmax = *k + nmi;
	i__1 = jcmax;
	for (j = 1; j <= i__1; ++j) {
	    dr[j - 1] = t[i + j] - *x;
	}
	i__1 = km1;
	for (j = jcmax; j <= i__1; ++j) {
	    aj[j] = (float)0.;
	    dr[j - 1] = dr[jcmax - 1];
	}
    } else {
	i__1 = km1;
	for (j = 1; j <= i__1; ++j) {
	    dr[j - 1] = t[i + j] - *x;
	}
    }

    i__1 = jcmax;
    for (jc = jcmin; jc <= i__1; ++jc) {
	aj[jc - 1] = bcoef[imk + jc];
    }

/*               *** difference the coefficients  jderiv  times. */
    if (*jderiv > 0) {
	i__1 = *jderiv;
	for (j = 1; j <= i__1; ++j) {
	    kmj = *k - j;
	    fkmj = (real) kmj;
	    ilo = kmj;
	    i__2 = kmj;
	    for (jj = 1; jj <= i__2; ++jj) {
		aj[jj - 1] = (aj[jj] - aj[jj - 1]) / (dl[ilo - 1] + dr[jj - 1]
			) * fkmj;
		--ilo;
	    }
	}
    }

/*  *** compute value at  x  in (t(i),t(i+1)) of jderiv-th derivative, */
/*     given its relevant b-spline coeffs in aj(1),...,aj(k-jderiv). */
    if (*jderiv < km1) {
	jdrvp1 = *jderiv + 1;
	i__1 = km1;
	for (j = jdrvp1; j <= i__1; ++j) {
	    kmj = *k - j;
	    ilo = kmj;
	    i__2 = kmj;
	    for (jj = 1; jj <= i__2; ++jj) {
		aj[jj - 1] = (aj[jj] * dl[ilo - 1] + aj[jj - 1] * dr[jj - 1]) 
			/ (dl[ilo - 1] + dr[jj - 1]);
		--ilo;
	    }
	}
    }

    *fofx = aj[0];

    return 0;
} /* dpbvalue_ */





/* -------------------------------------------------------------------------*/
/*  SPLIFIT ...  this subroutine inputs a strictly increasing sequence of */
/*       arguments X, along with NVAR sets of function values Y, and */
/*      computes the values of NDERIV derivative values at the NARG argument*/
/*      values in XARG.  The points are interpolated by bsplines of degree  ND
EG*/
/*       using the "not-a-knot" condition of de Boor's book, page 55 */

/*  Arguments: */

/*  N      ... length of arrays TAU and GTAU */
/* NARG   ... number of argument values at which the derivatives are required
*/
/*  NVAR   ... number of sets of function values */
/*  NORDER ... order of B-spline (degree + 1) (minimum value 1) */
/*  NDERIV ... order of derivative (0 min, NORDER - 2 max) */
/*  X      ... array of strictly increasing argument values */
/* Y      ... matrix of degree N by NVAR of function values at argument values
 X*/
/* XARG   ... array length NARG of argument values at which the derivative*/
/*             values are to be computed */
/*  DY     ... matrix of degree NARG by NVAR of returned derivative values */
/*  WORK   ... working storage of length (2*NORDER-1)*N + NORDER */
/*  IER    ...  error return: */
/*               0 ... no error */
/*               1 ... inappropriate value of N */
/*               2 ... problem with knots detected in SPLINT */
/*              3 ... singularity of coefficient matrix detected in BANFAC*/
/*               4 ... inappropriate value of NDERIV */
/*               5 ... inappropriate value of NDEG */
/*               6 ... X not strictly increasing */

/* Subroutine */
int splifit_(integer *n, integer *narg, integer *nvar, integer *norder,
integer *nderiv, doublereal *x, doublereal *y, doublereal *xarg,
doublereal *dy, doublereal *work, integer *ier)
{
    /* System generated locals */
    integer y_dim1, y_offset, dy_dim1, dy_offset, i__1, i__2;

    /* Local variables */
    static integer iarg, ivar;
    static integer i, k, iflag, iboff, nhalf, iqoff, itoff, n2;


    /* Parameter adjustments */
    --work;
    dy_dim1 = *narg;
    dy_offset = dy_dim1 + 1;
    dy -= dy_offset;
    --xarg;
    y_dim1 = *n;
    y_offset = y_dim1 + 1;
    y -= y_offset;
    --x;

    /* Function Body */
    *ier = 0;

/*  check arguments */

    if (*n <= *norder) {
	*ier = 1;
	return 0;
    }
    if (*nderiv < 0 || *nderiv >= *norder) {
	*ier = 4;
	return 0;
    }
    if (*norder < 1) {
	*ier = 5;
	return 0;
    }
    i__1 = *n;
    for (i = 2; i <= i__1; ++i) {
	if (x[i] <= x[i - 1]) {
	    *ier = 6;
	    return 0;
	}
    }

/*  compute offsets for arrays */

    n2 = *n << 1;
    iboff = 1;
    itoff = *n + 1;
    iqoff = n2 + *norder + 1;

/*  construct knot sequence */

    i__1 = *norder;
    for (k = 1; k <= i__1; ++k) {
	work[*n + k] = x[1];
	work[n2 + k] = x[*n];
    }
    nhalf = *norder / 2;
    i__1 = *n;
    for (k = *norder + 1; k <= i__1; ++k) {
	work[*n + k] = x[k - nhalf];
    }

/*  -----------  loop through sets of function values */

    i__1 = *nvar;
    for (ivar = 1; ivar <= i__1; ++ivar) {

/*  call SPLINT to get the coefficients for the interpolating bspline 
*/

	splint_(&x[1], &y[ivar * y_dim1 + 1], &work[itoff], n, norder, &work[
		iqoff], &work[iboff], &iflag);
	*ier = iflag - 1;
	if (*ier != 0) {
	    return 0;
	}

/*  go through data computing value of derivative nderiv */

	i__2 = *narg;
	for (iarg = 1; iarg <= i__2; ++iarg) {
	    dpbvalue_(&work[itoff], &work[iboff], n, norder, &xarg[iarg], 
		    nderiv, &dy[iarg + ivar * dy_dim1]);
	}
    }

    return 0;
} /* splifit_ */



