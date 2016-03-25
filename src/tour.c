/* tour.c */
/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  the xgobi code and associated documentation,  provided  *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *     Copyright (c) of code not pertaining to manual       *
 *     control is owned by Bellcore (1990,1991,1992,1993,   *
 *     1994, 1995).                                         *
 *     Copyright (c) of code pertaining to manual           *
 *     control of the grand tour is owned jointly by        *
 *     Dianne Cook and AT&T Bell Labs (1995).               *
 *  All Rights Reserved.                                    *
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

#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

/* external tour variables */
extern int got_new_basis;
/*
 * for backtrack control, to allow it to be de-sensitive at the
 * start and when reinit is pressed.
*/
static int bt_firsttime = 1;

/* frozen variables */
static float caa, cab, cba, cbb;
static int icaa, icab, icba, icbb;

/* variable to indicate whether fading is being done or not */
int variable_fading;   /* can be 0, 1 or 2 */
int variable_fading_finished;
/* variables to determine whether to stop tour_proc */
static int counting_to_stop;  /* can be 0, 1 or 2 */
static Boolean ready_to_stop_now;
static Boolean next_step;

/* variables for Householder and Givens interpolation */
#define GEODESIC 0
#define HOUSEHOLDER 1
#define GIVENS 2

float ip1, ip2, ip3, ip4, ip5, ip6, ip7, ip8;
static float **thh;
extern int tour_interp_btn;
static float angle_tol = 0.001;

/* optimization variables */
static float indx_prev = 0.;

/* tour axis labeling */
XSegment *tour_axes;

/* This should be moved into xgobitypes.h some time */
#define OWN_TOUR_SELECTION XtOwnSelection( (Widget) xg->workspace, \
  (Atom) XG_NEWTOUR, (Time) CurrentTime, \
  (XtConvertSelectionProc) pack_tour_data, \
  (XtLoseSelectionProc) pack_tour_lose , \
  (XtSelectionDoneProc) pack_tour_done ) 
/* Functions used in this file */
XtConvertSelectionProc pack_tour_data() ;
XtSelectionDoneProc pack_tour_done() ;
XtLoseSelectionProc pack_tour_lose() ;

/*
 * Methods used for grand tour are outlined in Andreas Buja, Daniel
 * Asimov, Catherine Hurley (1990) "Methods for Subspace Interpolation
 * in Dynamic Graphics" A technical note at Bellcore, Morristown.
 * Also many controls of the tour, such as backtracking and local scan
 * are as suggested by Catherine Hurley and Andreas Buja (1990)
 * "Analyzing High-Dimensional Data with Motion Graphics" Siam Journal
 * on Scientific and Statistical Computing.
*/

void
set_bt_firsttime()
{
  bt_firsttime = 1;
}

void
alloc_tour(xgobidata *xg)
{
  unsigned int nc = (unsigned int) xg->ncols;
  unsigned int nr = (unsigned int) xg->nrows;
  int i, j;

/* nrows x 2 */
  xg->xi0 = (long **) XtMalloc(nr * sizeof(long *));
  for (i=0; i<nr; i++)
    xg->xi0[i] = (long *) XtMalloc((unsigned int) 2 * sizeof(long));
  xg->xi1 = (long **) XtMalloc((unsigned) (nr * sizeof(long *)));
  for (i=0; i<nr; i++)
    xg->xi1[i] = (long *) XtMalloc((unsigned int) 2 * sizeof(long));
/* nrows x 2 frozen variables*/
  xg->xif = (long **) XtMalloc(nr * sizeof(long *));
  for (i=0; i<nr; i++)
    xg->xif[i] = (long *) XtMalloc((unsigned int) 2 * sizeof(long));
  
/* 2 x ncols */
  xg->v0 = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->v0[i] = (float *) XtMalloc(nc * sizeof(float));

  xg->v1 = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->v1[i] = (float *) XtMalloc(nc * sizeof(float));

  xg->tv = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->tv[i] = (float *) XtMalloc(nc * sizeof(float));

/* 2 x ncols */

  xg->u0 = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->u0[i] = (float *) XtMalloc(nc * sizeof(float));

  xg->u1 = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->u1[i] = (float *) XtMalloc(nc * sizeof(float));

  xg->u = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->u[i] = (float *) XtMalloc(nc * sizeof(float));

  xg->uold = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->uold[i] = (float *) XtMalloc(nc * sizeof(float));

  xg->tour_vars = (int *) XtMalloc(nc * sizeof(int));

  xg->ufrozen = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->ufrozen[i] = (float *) XtMalloc(nc * sizeof(float));
  xg->uwarm = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->uwarm[i] = (float *) XtMalloc(nc * sizeof(float));

  /* Axes and axis labels */
  tour_axes = (XSegment *) XtMalloc(nc * sizeof(XSegment));

  xg->tour_lab = (char **) XtMalloc(nc * sizeof (char *));
  for (j=0; j<nc; j++)
    xg->tour_lab[j] = (char *) XtMalloc(
      (unsigned int) (COLLABLEN+2*16) * sizeof(char));
      /* size allocated for collab_tform2[] in read_labels.c */

/*
 * Scratch arrays
*/
  xg->tnx = (float *) XtMalloc(nc * sizeof(float));
  thh = (float **) XtMalloc(nc * sizeof(float *));
  for (i=0; i<nc; i++)
    thh[i] = (float *) XtMalloc(nc * sizeof(float));

  alloc_std_vars(xg);

  /* frozen variables */
  xg->frozen_vars = (int *) XtMalloc(nc * sizeof(int));
}

void
free_tour(xgobidata *xg)
{
  int j,k;

  XtFree((XtPointer) xg->tour_vars);

  for (k=0; k<xg->nrows; k++)
    XtFree((XtPointer) xg->xi0[k]);
  XtFree((XtPointer) xg->xi0);
  for (k=0; k<xg->nrows; k++)
    XtFree((XtPointer) xg->xi1[k]);
  XtFree((XtPointer) xg->xi1);

  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->u[k]);
  XtFree((XtPointer) xg->u);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->u0[k]);
  XtFree((XtPointer) xg->u0);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->u1[k]);
  XtFree((XtPointer) xg->u1);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->uold[k]);
  XtFree((XtPointer) xg->uold);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->ufrozen[k]);
  XtFree((XtPointer) xg->ufrozen);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->uwarm[k]);
  XtFree((XtPointer) xg->uwarm);

  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->v0[k]);
  XtFree((XtPointer) xg->v0);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->v1[k]);
  XtFree((XtPointer) xg->v1);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->tv[k]);
  XtFree((XtPointer) xg->tv);

  XtFree((XtPointer) tour_axes);
  for (j=0; j<xg->ncols; j++)
    XtFree((XtPointer) xg->tour_lab[j]);
  XtFree((XtPointer) xg->tour_lab);

  XtFree((XtPointer) xg->tnx);
  for (j=0; j<xg->ncols; j++)
    XtFree((XtPointer) thh[j]);
  XtFree((XtPointer) thh);

  free_std_vars(xg);

  /* frozen variables */
  XtFree((XtPointer) xg->frozen_vars);
}

void
reinit_tour_hist(xgobidata *xg)
{
  hist_rec *histp;
/*
 * Free and then reinitialize the tour arrays.
*/
  if (xg->fl != NULL)
  {
    while (xg->fl->prev != NULL)
    {
      histp = xg->fl;
      xg->fl = xg->fl->prev;
      XtFree((XtPointer) histp->hist[0]);
      XtFree((XtPointer) histp->hist[1]);
      XtFree((XtPointer) histp);
    }
  }

  if (xg->is_backtracking)
  {
    xg->hfirst->prev = NULL;
    while (xg->hfirst->next != NULL)
    {
      histp = xg->hfirst;
      xg->hfirst = xg->hfirst->next;
      XtFree((XtPointer) histp->hist[0]);
      XtFree((XtPointer) histp->hist[1]);
      XtFree((XtPointer) histp);
    }
  }
  else
  {
    if (xg->curr != NULL)
    {
      xg->curr->next = NULL;
      xg->hfirst->prev = NULL;
      while (xg->curr->prev != NULL)
      {
        histp = xg->curr;
        xg->curr = xg->curr->prev;
        XtFree((XtPointer) histp->hist[0]);
        XtFree((XtPointer) histp->hist[1]);
        XtFree((XtPointer) histp);
      }
    }
  }
}

void
zero_tau(xgobidata *xg)
{
  int k;

  for (k=0; k<5; k++)   /* Givens needs five angles */
  {
    xg->tau[k]  = 0.0;
    xg->tinc[k] = 0.0;
  }
}

void
zero_tinc(xgobidata *xg)
{
  int k;

  for (k=0; k<5; k++)
    xg->tinc[0] = 0.0;
}

void
init_tour(xgobidata *xg, int firsttime)
{
  int i, j, m;

  xg->is_touring = False;

  if (xg->ncols_used > 2)
  {
  /* Don't re-initialize these when reading new data */
    if (firsttime)
    {
      xg->step = TOURSTEP0;
      xg->is_stepping = xg->is_local_scan = False;
      xg->is_tour_paused = False;
      xg->local_scan_dir = IN;
      xg->tour_link_state = unlinked;
      xg->tour_senddata = (unsigned long *) NULL;
    }
    /* moved here from init_vars */
    xg->is_princ_comp = False;
    xg->numvars_t = 3;
    xg->tour_vars[0] = 0;
    xg->tour_vars[1] = 1;
    xg->tour_vars[2] = 2;
    xg->manip_var = 0;
    xg->tour_cont_fact = one;
    xg->fcont_fact = 1.;

    /* backtracking */
    xg->is_backtracking = False;
    xg->nhist_list = 1;
    xg->max_nhist_list = MAXHIST;
    xg->old_nhist_list = -1;
    xg->backtrack_dir = BACKWARD;
    xg->fl = NULL;
    xg->curr = NULL;
    xg->hfirst = xg->curr;
    xg->is_store_history = True;

    /* for projection pursuit */
    xg->pp_index_btn = 0;
    xg->recalc_max_min = True;
    xg->xaxis_indent = XTextWidth(appdata.plotFont,
      "0000e-00", strlen("0000e-00") + ASCII_TEXT_BORDER_WIDTH);
      /*"0000e-00", strlen("0000e-00") + 2*ASCII_TEXT_BORDER_WIDTH);*/

    /* declare starting base as first two chosen variables */
    for (i=0; i<2; i++)
      for (j=0; j<xg->ncols; j++)
        xg->u0[i][j] = xg->u1[i][j] = xg->u[i][j] = xg->uold[i][j] =
          xg->v0[i][j] = xg->v1[i][j] = 0.0;

    xg->u1[0][xg->tour_vars[0]] =
      xg->u0[0][xg->tour_vars[0]] = xg->u[0][xg->tour_vars[0]] =
      xg->v0[0][xg->tour_vars[0]] = xg->v1[0][xg->tour_vars[0]] = 1.0;
    xg->u1[1][xg->tour_vars[1]] = xg->u[1][xg->tour_vars[1]] =
      xg->u0[1][xg->tour_vars[1]] =
      xg->v0[1][xg->tour_vars[1]] = xg->v1[1][xg->tour_vars[1]] = 1.0;

    xg->wait_for_more_vars = False;

    for (m=0; m<xg->nrows_in_plot; m++)
    {
      i = xg->rows_in_plot[m];
      xg->xi0[i][0] = xg->world_data[i][0];
      xg->xi0[i][1] = xg->world_data[i][1];
    }
    xg->s[0] = 0;
    xg->s[1] = M_PI_2;
    xg->coss[0] = 1.0;
    xg->coss[1] = 0.0;
    xg->sins[1] = 1.0;
    xg->sins[0] = 0.0;
    xg->icoss[0] = PRECISION2;
    xg->icoss[1] = 0;
    xg->isins[1] = PRECISION2;
    xg->isins[0] = 0;

    zero_tau(xg);
    xg->delta = 0.0;
    xg->dv = 1.0;

    xg->run_tour_proc = False;
    xg->pp_replot_freq = 10;

    /* for controlling behaviour during variable fading */
    variable_fading = 0;
    variable_fading_finished = 0;

    /* for controlling turning off tour_proc */
    counting_to_stop = 0;
    ready_to_stop_now = False;
    next_step = False;

    /* frozen variables */
    xg->nfrozen_vars = 0;
    for (j=0; j<xg->ncols; j++)
      xg->ufrozen[0][j] = xg->ufrozen[1][j] = 0.;
  }
  else  /* if ncols <= 2, can't tour */
    ;
}

void
init_basis(xgobidata *xg)
{
/*
 * Set u0 (old first basis) to be u(t) (new first basis)
*/
  if (xg->nfrozen_vars > 0)
  {
    copy_basis(xg->uwarm, xg->u0, xg->ncols_used);
    norm(xg->u0[0], xg->ncols_used);
    norm(xg->u0[1], xg->ncols_used);
    gram_schmidt(xg->u0[0], xg->u0[1], xg->ncols_used);
  }
  else
    copy_basis(xg->u, xg->u0, xg->ncols_used);
}

void
copy_u0_to_pd0(xgobidata *xg)
{
  copy_basis(xg->u0, xg->v0, xg->ncols_used);
}

void
copy_u1_to_pd1(xgobidata *xg)
{
  copy_basis(xg->u1, xg->v1, xg->ncols_used);
}

void
change_first_projection(int xy_ind, int varno, xgobidata *xg)
{
  int k;
  int var = xg->tour_vars[varno];

  if (!xy_ind)
    for (k=0; k<xg->nrows_in_plot; k++)
      xg->xi0[ xg->rows_in_plot[k] ][0] =
        xg->world_data[ xg->rows_in_plot[k] ][var];
  else
    for (k=0; k<xg->nrows_in_plot; k++)
      xg->xi0[ xg->rows_in_plot[k] ][1] =
        xg->world_data[ xg->rows_in_plot[k] ][var];
}

void
gen_norm_variates(int n, int p, float *vars)
{
  int i, check=1;
  double frunif[2];
  double r, fac, frnorm[2];

  for (i=0; i<(n*p+1)/2; i++) {
    while (check) {

      rnorm2(&frunif[0], &frunif[1]);
      r = frunif[0] * frunif[0] + frunif[1] * frunif[1];

      if (r < 1)
      {
        check = 0;
        fac = sqrt(-2. * log(r) / r);
        frnorm[0] = frunif[0] * fac;
        frnorm[1] = frunif[1] * fac;
      }
    }
    check = 1;
    vars[2*i] = FLOAT(frnorm[0]);
    vars[2*i+1] = FLOAT(frnorm[1]);
  }
}

void
new_basis(xgobidata *xg)
/*
 * Generate two random p dimensional vectors to form new ending basis
*/
{
  int j, check = 1;
  double frunif[2];
  double r, fac, frnorm[2];

/*
 * Method suggested by Press, Flannery, Teukolsky, and Vetterling (1986)
 * "Numerical Recipes" p.202-3, for generating random normal variates .
*/

  /* Zero out u1 before filling; this might fix a bug we are
     encountering with returning from a receive tour.
  */
  for (j=0; j<xg->ncols_used; j++)
    xg->u1[0][j] = xg->u1[1][j] = 0.0 ;

  if (xg->numvars_t > 2) {
    for (j=0; j<xg->numvars_t; j++) {
      while (check) {

        rnorm2(&frunif[0], &frunif[1]);
        r = frunif[0] * frunif[0] + frunif[1] * frunif[1];
  
        if (r < 1)
        {
          check = 0;
          fac = sqrt(-2. * log(r) / r);
          frnorm[0] = frunif[0] * fac;
          frnorm[1] = frunif[1] * fac;
        }
      }
      check = 1;
      xg->u1[0][xg->tour_vars[j]] = FLOAT(frnorm[0]);
      xg->u1[1][xg->tour_vars[j]] = FLOAT(frnorm[1]);
    }
    norm(xg->u1[0], xg->ncols_used);
    norm(xg->u1[1], xg->ncols_used);
/*
 * Orthogonalize the second vector on the first using Gram-Schmidt
*/
    gram_schmidt(xg->u1[0], xg->u1[1], xg->ncols_used);
  }
  else
  {
    xg->u1[0][xg->tour_vars[0]] = 1.;
    xg->u1[1][xg->tour_vars[1]] = 1.;
  }
}

void
store_basis(xgobidata *xg, float **basis)
{
/*
 * This routine saves a basis into the history list.
*/
  int i, j;
  hist_rec *prev;
  int too_close;

  /*
   * Initial node doubly linked to itself, used when tour is started.
  */
  if (xg->curr == NULL)
  {
    AllocBox(xg);
    for (i=0; i<xg->ncols_used; i++)
      for (j=0; j<2; j++)
        xg->curr->hist[j][i] = basis[j][i];
    xg->curr->next = xg->curr;
    xg->curr->prev = xg->curr;
    xg->hfirst = xg->curr;
    xg->old_nhist_list = xg->nhist_list;
    xg->nhist_list++;
  }
  else
  /* every other case */
  {
    too_close = check_proximity(basis, xg->curr->hist, xg->ncols_used);
  
    if (!too_close)
    {
      prev = xg->curr;
      if ((xg->nhist_list <= MAXHIST) &&
          (abs(xg->nhist_list - xg->old_nhist_list) > 0))
      {
    /*
     * Add in new node and remake circular list
    */
        AllocBox(xg);
        for (i=0; i<xg->ncols_used; i++)
          for (j=0; j<2; j++)
            xg->curr->hist[j][i] = basis[j][i];
        xg->curr->prev = prev;
        xg->curr->next = xg->hfirst;
        prev->next = xg->curr;
        xg->hfirst->prev = xg->curr;
        xg->old_nhist_list = xg->nhist_list;
        if (xg->nhist_list != MAXHIST)
          xg->nhist_list++;
      }
      else
      {
        /*
         * Case where MAXHIST has been reached. Move pointer to
         * next node in list and overwrite base.
        */
        xg->curr = xg->curr->next;
        for (i=0; i<xg->ncols_used; i++)
          for (j=0; j<2; j++)
            xg->curr->hist[j][i] = basis[j][i];
        xg->hfirst = xg->curr->next;
      }
    }
    nback_update_label(xg);
  }
}

void
retrieve_basis(xgobidata *xg)
{
  int i, j;

  for (i=0; i<xg->ncols_used; i++)
    for (j=0; j<2; j++)
      xg->u1[j][i] = xg->curr->hist[j][i];
}

void
set_local_scan_dir_in(xgobidata *xg)
{
  xg->local_scan_dir = IN;
}

void
basis_dir_ang(xgobidata *xg)
{
  float x, y ;
  int k;
  int n = xg->ncols_used;

/* calculate values to minimize angle between two base pairs */
  x = sq_inner_prod(xg->u0[0], xg->u1[0], n) +
    sq_inner_prod(xg->u0[0], xg->u1[1], n) -
    inner_prod(xg->u0[1], xg->u1[0], n) *
    inner_prod(xg->u0[1], xg->u1[0], n) -
    inner_prod(xg->u0[1], xg->u1[1], n) *
    inner_prod(xg->u0[1], xg->u1[1], n);

  y = inner_prod(xg->u0[0], xg->u1[0], n) *
    inner_prod(xg->u0[1], xg->u1[0], n) +
    inner_prod(xg->u0[0], xg->u1[1], n) *
    inner_prod(xg->u0[1], xg->u1[1], n);

/* calculate angles of rotation from bases (u) to princ dirs (v) */
  if (fabs(x) < angle_tol)
  {
    xg->s[0] = 0.0;
    xg->s[1] = M_PI_2;
  }
  else
  {
    xg->s[0] = (float) atan2((double)(2.*y),(double)x)/2.;
    xg->s[1] = xg->s[0] + M_PI_2;
  }

/* calculate cosines and sines of s */
  for (k=0; k<2; k++)
  {
    xg->coss[k] = FLOAT(cos(DOUBLE(xg->s[k])));
    xg->sins[k] = FLOAT(sin(DOUBLE(xg->s[k])));
    xg->icoss[k] = INT(xg->coss[k] * PRECISION2);
    xg->isins[k] = INT(xg->sins[k] * PRECISION2);
  }
}

void
princ_dirs(xgobidata *xg)
{
  int j;
  int n = xg->ncols_used;
  
/* calculate first princ dirs */ /* if there are frozen vars u0 won't
                                     have norm 1, but since this is just
                                     rotation it should be ok */
  if (xg->nfrozen_vars > 0)
  {
    for (j=0; j<xg->ncols_used; j++)
    {
      xg->v0[0][j] = xg->coss[0]*xg->u0[0][j] + xg->sins[0]*xg->u0[1][j];
      xg->v0[1][j] = xg->coss[1]*xg->u0[0][j] + xg->sins[1]*xg->u0[1][j];
    }
    norm(xg->v0[0], n);
    norm(xg->v0[1], n);
  }
  else
  {
    for (j=0; j<xg->ncols_used; j++)
    {
      xg->v0[0][j] = xg->coss[0]*xg->u0[0][j] + xg->sins[0]*xg->u0[1][j];
      xg->v0[1][j] = xg->coss[1]*xg->u0[0][j] + xg->sins[1]*xg->u0[1][j];
    }
    norm(xg->v0[0], n);
    norm(xg->v0[1], n);
  }
  
/* calculate second princ dirs by projecting v0 onto v1 */
  for (j=0; j<xg->ncols_used; j++)
  {
    xg->v1[0][j] = inner_prod(xg->v0[0], xg->u1[0], n) * xg->u1[0][j] +
          inner_prod(xg->v0[0], xg->u1[1], n) * xg->u1[1][j] ;
    xg->v1[1][j] = inner_prod(xg->v0[1], xg->u1[0], n) * xg->u1[0][j] +
          inner_prod(xg->v0[1], xg->u1[1], n) * xg->u1[1][j] ;
  }

  if (xg->nfrozen_vars > 0)
  {
    /* compute coefficeients for getting back to the warm space */
    caa = inner_prod(xg->uwarm[0], xg->v0[0], xg->ncols_used);
    cab = inner_prod(xg->uwarm[0], xg->v0[1], xg->ncols_used);
    cba = inner_prod(xg->uwarm[1], xg->v0[0], xg->ncols_used);
    cbb = inner_prod(xg->uwarm[1], xg->v0[1], xg->ncols_used);
    icaa = INT(caa * PRECISION2);
    icab = INT(cab * PRECISION2);
    icba = INT(cba * PRECISION2);
    icbb = INT(cbb * PRECISION2);
  }
}

void
princ_angs(xgobidata *xg)
{
  int j, k;
  float tmpf1, tmpf2;
  float tol2 = 0.01;
/*
 * if the norms vanish need to regenerate another basis and new
 * princ dirs, otherwise no rotation occurs - to be put in code
 * where new basis is generated between consecutive tours
*/

  if (calc_norm(xg->v1[0], xg->ncols_used) < angle_tol)
    for (j=0; j<xg->ncols_used; j++)
      xg->v1[0][j] = xg->u1[0][j];
  if (calc_norm(xg->v1[1], xg->ncols_used) < angle_tol)
    for (j=0; j<xg->ncols_used; j++)
      xg->v1[1][j] = xg->u1[1][j];
  norm(xg->v1[0], xg->ncols_used);
  norm(xg->v1[1], xg->ncols_used);

/* calculate principle angles for movement, and calculate stepsize */
/* put in check for outside of cos domain, ie >1, <-1. */
  tmpf1 = inner_prod(xg->v0[0], xg->v1[0], xg->ncols_used);
  tmpf2 = inner_prod(xg->v0[1], xg->v1[1], xg->ncols_used);
  if (tmpf1 > 1.0)
    tmpf1 = 1.0;
  else if (tmpf1 < -1.0)
    tmpf1 = -1.0;
  if (tmpf2 > 1.0)
    tmpf2 = 1.0;
  else if (tmpf2 < -1.0)
    tmpf2 = -1.0;
  xg->tau[0] = (float) acos((double) tmpf1);
  xg->tau[1] = (float) acos((double) tmpf2);

  if ((xg->tau[0] < tol2) && (xg->tau[1] < tol2))
  {
    zero_tau(xg);
    k = 0;
    for (j=0; j<xg->numvars_t; j++)
    {
      xg->u1[0][j] = xg->u1[1][j] = 0.0;
      if (j == xg->tour_vars[k])
      {
        k++;
      }
      else
        xg->u0[0][j] = xg->u0[1 ][j] = 0.;
    }
    xg->wait_for_more_vars = True;

  }
  else
    xg->wait_for_more_vars = False;

  if (!xg->wait_for_more_vars)
  {
    if (xg->tau[0] < tol2)
      xg->tau[0] = 0.;
    if (xg->tau[1] < tol2)
      xg->tau[1] = 0.;
    zero_tinc(xg);
  
    xg->dv = sqrt(xg->tau[0]*xg->tau[0] + xg->tau[1]*xg->tau[1]);
    xg->delta = xg->step/xg->dv;
  
  /* orthogonalize v1 wrt v0 by Gram-Schmidt and normalize */
    if (xg->tau[0] > angle_tol)
      gram_schmidt(xg->v0[0], xg->v1[0], xg->ncols_used);
    if (xg->tau[1] > angle_tol)
      gram_schmidt(xg->v0[1], xg->v1[1], xg->ncols_used);
  }
}

void
span_planes(xgobidata *xg)
{
/*
 * This routine preprojects the data into the span of starting
 * and ending planes at the time of new basis generation
*/
  int i, j, k, m;

  for (m=0; m<xg->nrows_in_plot; m++)
  {
    i = xg->rows_in_plot[m];
    for (j=0; j<xg->ncols_used; j++)
      xg->tnx[j] = FLOAT(xg->world_data[i][j]);
    if (xg->numvars_t == 2 && (xg->is_pp_optimz || xg->is_princ_comp))
    {
/*
 * In this case, we're skipping the fade-out and jumping down
 * to two, so just project into that basis.
*/
      if (xg->nfrozen_vars > 0)
      {
        for (k=0; k<2; k++)
        {
          xg->xif[i][k] = (long)(inner_prod(xg->tnx,
            xg->ufrozen[k], xg->ncols_used));
        }
      }
      for (k=0; k<2; k++)
      {
        xg->xi0[i][k] = (long)(inner_prod(xg->tnx,
          xg->u0[k], xg->ncols_used));
      }
    } 
    else 
    {/* General case */
      if (xg->nfrozen_vars > 0)
      {
        for (k=0; k<2; k++)
        {
          xg->xif[i][k] = (long)(inner_prod(xg->tnx,
            xg->ufrozen[k], xg->ncols_used));
        }
      }
      for (k=0; k<2; k++)
      {
        xg->xi0[i][k] = (long)(inner_prod(xg->tnx, xg->v0[k],
          xg->ncols_used));
        xg->xi1[i][k] = (long)(inner_prod(xg->tnx, xg->v1[k],
          xg->ncols_used));
      }

    }
  }
}

void
tour_reproject(xgobidata *xg)
/*
 * This routine uses the data projected into the span of
 * the starting basis and ending basis, and then rotates in this
 * space.
*/
{
  long xit[2];
  int i, m;
  float costf[2], sintf[2];
  int costi[2], sinti[2];

  for (i=0; i<2; i++)
  {
    costf[i] = FLOAT(cos(DOUBLE(xg->tinc[i])));
    sintf[i] = FLOAT(sin(DOUBLE(xg->tinc[i])));
    costi[i] = INT(costf[i] * PRECISION2);
    sinti[i] = INT(sintf[i] * PRECISION2);
  }

/* See span_planes for comments */
  if (xg->numvars_t == 2 && (xg->is_pp_optimz || xg->is_princ_comp))
    ;

  else
  {
  /* basically do calculations ready for use in drawing
   * segments in variable circles */
  /* do these in integer to speed calculations? */
    if (xg->nfrozen_vars > 0)
      {
      for (i=0; i<xg->ncols_used; i++)
      {
        xg->tv[0][i] = costf[0]*xg->v0[0][i] + sintf[0]*xg->v1[0][i];
        xg->tv[1][i] = costf[1]*xg->v0[1][i] + sintf[1]*xg->v1[1][i];
      }

      for (i=0; i<xg->ncols_used; i++)
      {
        xg->uwarm[0][i] = caa * xg->tv[0][i] + cab * xg->tv[1][i];
          xg->u[0][i] = xg->uwarm[0][i] + xg->ufrozen[0][i];
          xg->uwarm[1][i] = cba * xg->tv[0][i] + cbb * xg->tv[1][i];
          xg->u[1][i] = xg->uwarm[1][i] + xg->ufrozen[1][i];
      }
    }
    else
    {
      for (i=0; i<xg->ncols_used; i++)
      {
        xg->tv[0][i] = costf[0]*xg->v0[0][i] + sintf[0]*xg->v1[0][i];
        xg->tv[1][i] = costf[1]*xg->v0[1][i] + sintf[1]*xg->v1[1][i];
      }

      for (i=0; i<xg->ncols_used; i++)
      {
        xg->u[0][i] = xg->coss[0] * xg->tv[0][i] - xg->sins[0]*xg->tv[1][i];
        xg->u[1][i] = -xg->coss[1] * xg->tv[0][i] + xg->sins[1]*xg->tv[1][i];
      }
    }
  }

  if (xg->nfrozen_vars > 0)
  {
    for (m=0; m<xg->nrows_in_plot; m++)
    {
      i = xg->rows_in_plot[m];
      xit[0] = costi[0] * xg->xi0[i][0] + sinti[0] * xg->xi1[i][0];
      xit[1] = costi[1] * xg->xi0[i][1] + sinti[1] * xg->xi1[i][1];
      xit[0] = xit[0] / PRECISION2;
      xit[1] = xit[1] / PRECISION2;

      xg->planar[i].x = icaa * xit[0] + icab * xit[1];
      xg->planar[i].y = icba * xit[0] + icbb * xit[1];
      xg->planar[i].x = xg->planar[i].x / PRECISION2;
      xg->planar[i].y = xg->planar[i].y / PRECISION2;
      xg->planar[i].x = xg->planar[i].x + xg->xif[i][0];
      xg->planar[i].y = xg->planar[i].y + xg->xif[i][1];
    }
  }
  else
  {
    for (m=0; m<xg->nrows_in_plot; m++)
    {
      i = xg->rows_in_plot[m];
  
      xit[0] = costi[0] * xg->xi0[i][0] + sinti[0] * xg->xi1[i][0];
      xit[1] = costi[1] * xg->xi0[i][1] + sinti[1] * xg->xi1[i][1];
      xit[0] = xit[0] / PRECISION2;
      xit[1] = xit[1] / PRECISION2;
      xg->planar[i].x = xg->icoss[0] * xit[0] - xg->isins[0] * xit[1];
      xg->planar[i].y = (-xg->icoss[1] * xit[0] + xg->isins[1] * xit[1]);
      xg->planar[i].x = xg->planar[i].x / PRECISION2;
      xg->planar[i].y = xg->planar[i].y / PRECISION2;
    }
  }

  if (xg->is_tour_section)
    tour_section_calcs(xg, 1);

}

/*
 * This routine is not used. It projects the data from p-space
 * to 2-space at every incremental movement. So the procedure is
 * more intuitively apparent but lacks the speed of the preprojection
 * method.
*/
/*
void
tour_reproject2(xgobidata *xg)
{
  int i, j;

  for (j=0; j<xg->ncols_used; j++)
  {
    xg->tv[0][j] = FLOAT(
      cos(DOUBLE(tinc[0])) * xg->v0[0][j] +
      sin(DOUBLE(tinc[0])) * xg->v1[0][j]);
    xg->tv[1][j] = FLOAT(
      cos(DOUBLE(tinc[1])) * xg->v0[1][j] +
      sin(DOUBLE(tinc[1])) * xg->v1[1][j]);
  }
  for (j=0; j<xg->ncols_used; j++)
  {
    xg->u[0][j] = FLOAT(cos(DOUBLE(s[0])) * xg->v[0][j] -
      sin(DOUBLE(s[0])) * xg->v[1][j]);
    xg->u[1][j] = FLOAT(-cos(DOUBLE(s[1])) * xg->v[0][j] +
      sin(DOUBLE(s[1])) * xg->v[1][j]);
  }

  for (i=0; i<xg->nrows; i++)
  {
    for (j=0; j<xg->ncols_used; j++)
      xg->tnx[j] = FLOAT(xg->world_data[i][j]);

    xg->planar[i].x = (long) inner_prod(xg->tnx, xg->u[0], xg->ncols_used);
    xg->planar[i].y = (long) inner_prod(xg->tnx, xg->u[1], xg->ncols_used);
  }
}
*/

/**********************************************
This is a new section for doing rotation between starting and ending
planes by using Householder reflections resulting in rotations.
***********************************************/
void
HH_tour_path(xgobidata *xg)
{
  float tmpf;
  int k, j;
  float tol = 0.00001;
  int tmpi = 0;

/* Calculate reflection vectors which will form rotation plane */
  /* Calculate r0 */
  for (j=0; j<xg->ncols_used; j++) {
    xg->tv[0][j] = xg->u0[0][j] - xg->u1[0][j];
  }
  norm(xg->tv[0], xg->ncols_used);

  /* Calculate r1 */
  tmpf = inner_prod(xg->u0[1], xg->tv[0], xg->ncols_used);
  for (j=0; j<xg->ncols_used; j++) {
    xg->tv[1][j] = xg->u0[1][j] - 2.*tmpf*xg->tv[0][j] - xg->u1[1][j];
  }
  norm(xg->tv[1], xg->ncols_used);

/* Calculate angle between r0,r1 - double it to form angle of rotation */
  tmpf = inner_prod(xg->tv[0],xg->tv[1], xg->ncols_used);
  if (tmpf > 1.0) tmpf = 1.0;
  if (tmpf < -1.0) tmpf = -1.0;
  xg->tau[0] = 2.0 * FLOAT(acos(DOUBLE(tmpf)));
  xg->dv = xg->tau[0];
  xg->delta = xg->step / xg->dv;

/* Form 4D orthonormal basis around r0, r1, u0 and store in V0, V1 */
  for (j=0; j<xg->ncols_used; j++)
    for (k=0; k<2; k++)
      if ((fabs(xg->u0[k][j]) < tol) && (fabs(xg->u1[k][j]) < tol))
        tmpi++;
  tmpi = xg->ncols_used - tmpi/2;
  copy_basis(xg->tv,xg->v0, xg->ncols_used);
  copy_basis(xg->u0,xg->v1, xg->ncols_used);
  gram_schmidt(xg->v0[0],xg->v0[1], xg->ncols_used);
  gram_schmidt(xg->v0[0],xg->v1[0], xg->ncols_used);
  gram_schmidt(xg->v0[1],xg->v1[0], xg->ncols_used);
/* altered to accommodate reading in history with tour space higher
 * than 3
*/
  if (tmpi > 3)
  {
    gram_schmidt(xg->v0[0],xg->v1[1], xg->ncols_used);
    gram_schmidt(xg->v0[1],xg->v1[1], xg->ncols_used);
    gram_schmidt(xg->v1[0],xg->v1[1], xg->ncols_used);
  }
  else {
    for (j=0; j<xg->ncols_used; j++)
      xg->v1[1][j] = 0.0;
  }

/* Calculate inner_prods for various later calculations */
  ip1 = inner_prod(xg->u0[0],xg->v0[0], xg->ncols_used);
  ip2 = inner_prod(xg->u0[0],xg->v0[1], xg->ncols_used);
  ip3 = inner_prod(xg->u0[0],xg->v1[0], xg->ncols_used);
  ip4 = inner_prod(xg->u0[0],xg->v1[1], xg->ncols_used);
  ip5 = inner_prod(xg->u0[1],xg->v0[0], xg->ncols_used);
  ip6 = inner_prod(xg->u0[1],xg->v0[1], xg->ncols_used);
  ip7 = inner_prod(xg->u0[1],xg->v1[0], xg->ncols_used);
  ip8 = inner_prod(xg->u0[1],xg->v1[1], xg->ncols_used);

}

void
HH_tour_reproject(xgobidata *xg)
{
  float costf, sintf;
  float tmpf;
  int i, j, m;

  costf = (float)cos((double)(xg->tinc[0]));
  sintf = (float)sin((double)(xg->tinc[0]));

/* form HHu but store in U */
  for (j=0; j<xg->ncols_used; j++)
  {
    xg->u[0][j] = (costf*ip1-sintf*ip2)*xg->v0[0][j]
      + (sintf*ip1+costf*ip2)*xg->v0[1][j]
      + ip3*xg->v1[0][j] + ip4*xg->v1[1][j];
    xg->u[1][j] = (costf*ip5-sintf*ip6)*xg->v0[0][j]
      + (sintf*ip5+costf*ip6)*xg->v0[1][j]
      + ip7*xg->v1[0][j] + ip8*xg->v1[1][j];
  }

/* calculate data projections */
  for (i=0; i<xg->nrows_in_plot; i++)
  {
    m = xg->rows_in_plot[i];
    tmpf = (costf*ip1-sintf*ip2) * FLOAT(xg->xi0[m][0]) +
      (sintf*ip1+costf*ip2) * FLOAT(xg->xi0[m][1]) +
      ip3*FLOAT(xg->xi1[m][0]) + ip4*FLOAT(xg->xi1[m][1]);
    xg->planar[m].x = (long)tmpf;
    tmpf = (costf*ip5-sintf*ip6) * FLOAT(xg->xi0[m][0]) +
      (sintf*ip5+costf*ip6) * FLOAT(xg->xi0[m][1]) +
      ip7*FLOAT(xg->xi1[m][0]) + ip8*FLOAT(xg->xi1[m][1]);
    xg->planar[m].y = (long)tmpf;
  }
}

/**********************************************
This is a new section for doing rotation between starting and ending
planes by using Givens rotations.
***********************************************/

void
Givens_tour_path(xgobidata *xg)
{
  float c1, s1, tmpf;
  float tmpu[2][4];
  int j, k, k1, k2;
  float tol = 0.00001;
  int tmpi = 0;

/* generate orthonormal basis on u0, u1 and store in v0, v1 */
  copy_basis(xg->u0, xg->v0, xg->ncols_used);
  copy_basis(xg->u1, xg->v1, xg->ncols_used);

/* calculate the number of active variables - necessary after
 * reading in history file */
  for (k=0; k<2; k++)
    for (j=0; j<xg->ncols_used; j++)
    if ((fabs(xg->u0[k][j]) < tol) && (fabs(xg->u1[k][j]) < tol))
      tmpi++;
  tmpi = xg->ncols_used - tmpi/2;

  gram_schmidt(xg->v0[0],xg->v0[1], xg->ncols_used);
  gram_schmidt(xg->v0[0],xg->v1[0], xg->ncols_used);
  gram_schmidt(xg->v0[1],xg->v1[0], xg->ncols_used);
  if (tmpi > 3)
  {
    gram_schmidt(xg->v0[0],xg->v1[1], xg->ncols_used);
    gram_schmidt(xg->v0[1],xg->v1[1], xg->ncols_used);
    gram_schmidt(xg->v1[0],xg->v1[1], xg->ncols_used);
  }
  else
  {
    for (j=0; j<xg->ncols_used; j++)
      xg->v1[1][j] = 0.0;
  }

/* Project u1 into v0,v1 and calculate rotation matrices */
  for (k1=0; k1<2; k1++)
    for (k2=0; k2<2; k2++)
    {
      tmpu[k1][k2] = inner_prod(xg->u1[k1], xg->v0[k2], xg->ncols_used);
      tmpu[k1][k2+2] = inner_prod(xg->u1[k1], xg->v1[k2], xg->ncols_used);
    }

/* algorithm from Golub and Van Loan, to generate Givens rotation
 * matrices
*/
  for (k1=0; k1<2; k1++)
  {
    for (k2=(k1+1); k2<4; k2++)
    {
      if (tmpu[k1][k2] == 0)
      {
        c1 = 1.;
        s1 = 0.;
      }
      else if (tmpu[k1][k2] >= tmpu[k1][k1])
      {
        tmpf = tmpu[k1][k1] / tmpu[k1][k2];
        s1 = 1./hypot(1.,tmpf);
        c1 = s1 * tmpf;
      }
      else
      {
        tmpf = tmpu[k1][k2] / tmpu[k1][k1];
        c1 = 1./hypot(1.,tmpf);
        s1 = c1 * tmpf;
      }
      tmpu[k1][k1] = c1 * tmpu[k1][k1] + s1 * tmpu[k1][k2];
      tmpu[k1][k2] = 0.;
      if (k1 == 0)
      {
        tmpf = c1 * tmpu[1][k1] + s1 * tmpu[1][k2];
        tmpu[1][k2] = -s1 * tmpu[1][k1] + c1 * tmpu[1][k2];
        tmpu[1][k1] = tmpf;
      }
      xg->tau[k1*3+(k2-k1-1)] = -(float)atan((double)s1/(double)c1);
      tmpf = (float)asin((double)s1);
    }
  }

  tmpf = 0. ;
  for (k=0; k<5; k++)
    tmpf += (xg->tau[k] * xg->tau[k]);

  xg->dv = sqrt(tmpf);
  xg->delta = xg->step / xg->dv;

}

void
Givens_tour_reproject(xgobidata *xg)
{
  float costf[5], sintf[5];
  float **R1, **R2, **R3;
  int i, j, k, k1, k2, m;

/* malloc space for scratch variable */
  R1 = (float **)XtMalloc(4 * sizeof(float *));
  for (k=0; k<4; k++)
    R1[k] = (float *) XtMalloc(4 * sizeof(float));
  R2 = (float **)XtMalloc(4 * sizeof(float *));
  for (k=0; k<4; k++)
    R2[k] = (float *) XtMalloc(4 * sizeof(float));
  R3 = (float **)XtMalloc(4 * sizeof(float *));
  for (k=0; k<4; k++)
    R3[k] = (float *) XtMalloc(4 * sizeof(float));


  for (k=0; k<5; k++)
  {
    costf[k] = cos(DOUBLE(xg->tinc[k]));
    sintf[k] = sin(DOUBLE(xg->tinc[k]));
  }

/* Calculate the incremental rotation matrix */
  for (k=0; k<4; k++)
  {
    for (j=0; j<4; j++)
      R1[k][j] = 0.;
    R1[k][k] = 1.;
  }
  copy_matrix(R1,R2,4,4);
  for (k1=4; k1>-1; k1--)
  {
    for (k2=0; k2<4; k2++)
    {
      for (j=0; j<4; j++)
        R1[k2][j] = 0.;
      R1[k2][k2] = 1.;
    }
    R1[(k1/3)][(k1/3)] = costf[k1];
    R1[(k1/3)][k1%3+k1/3+1] = -sintf[k1];
    R1[(k1%3+k1/3+1)][k1/3] = sintf[k1];
    R1[(k1%3+k1/3+1)][k1%3+k1/3+1] = costf[k1];
    matrix_mult(R1,R2,R3,4,4,4);
    copy_matrix(R3,R2,4,4);
  }

/* Calculate current projections of axes */
  for (j=0; j<xg->ncols_used; j++)
    for (k=0; k<2; k++)
      xg->u[k][j] = R2[k][0]*xg->v0[0][j] + R2[k][1]*xg->v0[1][j] +
        R2[k][2]*xg->v1[0][j] + R2[k][3]*xg->v1[1][j];

/* Calculate data projections */
  for (i=0; i<xg->nrows_in_plot; i++) {
    m = xg->rows_in_plot[i];
    xg->planar[m].x = (long)(R2[0][0]*(float)xg->xi0[m][0] +
      R2[0][1]*(float)xg->xi0[m][1] + R2[0][2]*(float)xg->xi1[m][0] +
      R2[0][3]*(float)xg->xi1[m][1]);
    xg->planar[m].y = (long)(R2[1][0]*(float)xg->xi0[m][0] +
      R2[1][1]*(float)xg->xi0[m][1] + R2[1][2]*(float)xg->xi1[m][0] +
      R2[1][3]*(float)xg->xi1[m][1]);
  }

  for (k=0; k<4; k++)
    XtFree((char *) R1[k]);
  XtFree((char *) R1);
  for (k=0; k<4; k++)
    XtFree((char *) R2[k]);
  XtFree((char *) R2);
  for (k=0; k<4; k++)
    XtFree((char *) R3[k]);
  XtFree((char *) R3);
}

void
set_fading_var(int logical)
{
  variable_fading = logical;
}

void
set_counting_to_stop(int logical)
{
  counting_to_stop = logical;
}

void
set_ready_to_stop_now(Boolean logical)
{
  ready_to_stop_now = logical;
}

void
increment_tour(xgobidata *xg)
{
  static int count = 0;

  next_step = False;
  world_to_plane(xg);
  if (xg->is_pp)
  {
    count++;
    if (count >= xg->pp_replot_freq)
    {
      pp_index(xg,0,0);
      count = 0;
    }
  }
  plane_to_screen(xg);
  plot_once(xg);

  /* broken for princ_comp; eigenval[] === 0 in invert_proj_coords() */
  tour_var_lines(xg);

  if (xg->tour_link_state == send_state)
  {
    xg->new_basis_ind = False;
    OWN_TOUR_SELECTION ;
    announce_tour_coefs(xg);
  }
}

void
do_last_increment(xgobidata *xg)
{
  int k, need_to_do_last_increment=0;

  if (!xg->is_pp_optimz)
  {
    if (tour_interp_btn == GEODESIC)
    {
      if (xg->is_local_scan && xg->local_scan_dir == IN)
      {
        if ((xg->tinc[0] != xg->tau[0]) ||
          (xg->tinc[1] != xg->tau[1]))
        {
          xg->tinc[0] = xg->tau[0];
          xg->tinc[1] = xg->tau[1];
          need_to_do_last_increment = 1;
        }
      }
      else
      {
        if ((xg->tinc[0] != xg->fcont_fact*xg->tau[0]) ||
          (xg->tinc[1] != xg->fcont_fact*xg->tau[1]))
        {
          xg->tinc[0] = xg->fcont_fact*xg->tau[0];
          xg->tinc[1] = xg->fcont_fact*xg->tau[1];
          need_to_do_last_increment = 1;
        }
      }
    }
    else if (tour_interp_btn == HOUSEHOLDER)
    {
      if (xg->is_local_scan && xg->local_scan_dir == IN)
      {
        if (xg->tinc[0] != xg->tau[0])
        {
          xg->tinc[0] = xg->tau[0];
          need_to_do_last_increment = 1;
        }
      }
      else
      {
        if (xg->tinc[0] != xg->fcont_fact*xg->tau[0])
        {
          xg->tinc[0] = xg->fcont_fact*xg->tau[0];
          need_to_do_last_increment = 1;
        }
      }
    }
    else if (tour_interp_btn == GIVENS)
    {
      if (xg->is_local_scan && xg->local_scan_dir == IN)
      {
        if ((xg->tinc[0] != xg->tau[0]) || 
          (xg->tinc[1] != xg->tau[1]) ||
          (xg->tinc[2] != xg->tau[2]) || 
          (xg->tinc[3] != xg->tau[3]) ||
          (xg->tinc[4] != xg->tau[4]))
        {
          for (k=0; k<5; k++)
            xg->tinc[k] = xg->tau[k];
          need_to_do_last_increment = 1;
        }
      }
      else
      {
        if ((xg->tinc[0] != xg->fcont_fact*xg->tau[0]) || 
          (xg->tinc[1] != xg->fcont_fact*xg->tau[1]) ||
          (xg->tinc[2] != xg->fcont_fact*xg->tau[2]) || 
          (xg->tinc[3] != xg->fcont_fact*xg->tau[3]) ||
          (xg->tinc[4] != xg->fcont_fact*xg->tau[4]))
        {
          for (k=0; k<5; k++)
            xg->tinc[k] = xg->fcont_fact*xg->tau[k];
          need_to_do_last_increment = 1;
        }
      }
    }

    if (need_to_do_last_increment)
    {
      world_to_plane(xg);
      plane_to_screen(xg);
      plot_once(xg);
      tour_var_lines(xg);
      if (xg->tour_link_state == send_state)
      {
        xg->new_basis_ind = True;
        OWN_TOUR_SELECTION ;
        announce_tour_coefs(xg);
      }

    }
  }
}

void
geodesic_tour_path(xgobidata *xg)
{
  basis_dir_ang(xg);
  princ_dirs(xg);
  princ_angs(xg);
}

void
determine_endbasis_and_path(xgobidata *xg)
{
/*
 * if haven't got U1 updated do so now according to corrct mode
 * of operation.
*/
  if (!got_new_basis)
  {
    if (xg->is_local_scan)
    {
    /*
     * Control for local scan
    */
      if (xg->local_scan_dir == IN)
      {
        /* If at anchor basis generate a second basis randomly */
        init_basis(xg);/* store current plane in u0 */
        if (xg->is_store_history)
          store_basis(xg, xg->u0);
        new_basis(xg);/* generate random target */
        xg->local_scan_dir = OUT;
      }
      else if (xg->local_scan_dir == OUT)
      {
        /* otherwise switch U0 and U1 to go back to anchor */
        copy_basis(xg->u0, xg->u1, xg->ncols_used);/* set u0 to be target */
        init_basis(xg);/* store current plane in u0 */
        if (xg->is_store_history)
          store_basis(xg, xg->u0);
        xg->local_scan_dir = IN;
      }
    }
    else if (xg->is_backtracking)
    {
    /*
     * Backtracking control.
    */
      if (xg->backtrack_dir == FORWARD)
      {
        init_basis(xg);
        if (xg->curr->next == NULL)
        {
          /*
           * Reverse direction and sleep for a second
           * when reaching the end of the history list.
          */
          xg->backtrack_dir = BACKWARD;
          xg->curr = xg->curr->prev;
          reset_cycleback_cmd(false, false, "B");
          if (!xg->is_stepping)
          sleep(1);
        }
        else
          xg->curr = xg->curr->next;

        retrieve_basis(xg);
        xg->nhist_list += xg->backtrack_dir;
        nback_update_label(xg);
      }
      else
      {
        init_basis(xg);
        if (xg->curr->prev == NULL)
        {
          /*
           * Reverse direction and sleep for a second
           * when reaching the end of the history list.
          */
          xg->backtrack_dir = FORWARD;
          xg->curr = xg->curr->next;
          reset_cycleback_cmd(false, false, "F");
          if (!xg->is_stepping)
          sleep(1);
        }
        else
          xg->curr = xg->curr->prev;

        retrieve_basis(xg);
        xg->nhist_list += xg->backtrack_dir;
        nback_update_label(xg);
      }
    }
    else
    {
    /* general scan tour */
      if (!check_proximity(xg->u, xg->u0, xg->ncols_used) ||
        xg->is_stepping)
      {
        init_basis(xg);
        if (xg->is_store_history)
          store_basis(xg, xg->u0);
      }
      if (xg->numvars_t >= 2)
      {
        if (xg->is_pp_optimz)
          pp_dir(xg);
        else
          new_basis(xg);
      }
    }
  }
/*
 * Calculate path.
*/
  nback_update_label(xg);
  zero_tau(xg);
/*
 * In these cases, we're skipping the fade-out and jumping down
 * to two, so skip the next three function calls.
*/
  if (xg->numvars_t == 2 && (xg->is_pp_optimz || xg->is_princ_comp))
  ;
  else
  {
    if (tour_interp_btn == GEODESIC)
      geodesic_tour_path(xg);
    else if (tour_interp_btn == HOUSEHOLDER)
      HH_tour_path(xg);
    else if (tour_interp_btn == GIVENS)
      Givens_tour_path(xg);
  }
  if (!xg->wait_for_more_vars)
    span_planes(xg);

  xg->new_direction_flag = False;
  got_new_basis = 0;
  if (variable_fading == 2)
    variable_fading--;
  else if (variable_fading == 1)
  {
    variable_fading--;
    variable_fading_finished = 1;
  }
  if (variable_fading_finished)
  {
    variable_fading_finished = 0;
    set_sens_localscan(true);
    reset_backtrack_cmd(false, false, true, false);
  }
}

void
all_tour_reproject(xgobidata *xg)
{
  if (tour_interp_btn == GEODESIC)
    tour_reproject(xg);
  else if (tour_interp_btn == HOUSEHOLDER)
    HH_tour_reproject(xg);
  else if (tour_interp_btn == GIVENS)
    Givens_tour_reproject(xg);
}

void
zero_indx_prev(void)
{
  indx_prev = 0.;
}

void
find_max_indx_in_interval(xgobidata *xg, float indx_now)
{
  int j, k, niter = -1;
  float indx_interm, tinc_start[5], test_val;
  float inc_factor = 0.625, inc_prev, inc_now, inc_interm;

  for (k=0; k<5; k++)
  tinc_start[k] = xg->tinc[k];

  inc_prev = -1.;
  inc_now = 0.;
  inc_interm = -inc_factor;
  test_val = fabs((double)(inc_factor/niter))*xg->step;

  while (test_val > angle_tol)
  {
  if (tour_interp_btn == GEODESIC)
    {
      xg->tinc[0] = tinc_start[0] + inc_interm*xg->delta*xg->tau[0];
      xg->tinc[1] = tinc_start[1] + inc_interm*xg->delta*xg->tau[1];
    }
    else if (tour_interp_btn == HOUSEHOLDER)
    {
      xg->tinc[0] = tinc_start[0] + inc_interm*xg->delta*xg->tau[0];
    }
    else if (tour_interp_btn == GIVENS)
    {
      for (j=0; j<5; j++)
        xg->tinc[j] = tinc_start[j] + inc_interm*xg->delta*xg->tau[j];
    }
    all_tour_reproject(xg);
    indx_interm = pp_index_retval(xg);
    if (indx_interm > indx_now)
    {
      indx_now = indx_prev = indx_interm;
      inc_now = inc_prev = inc_interm;
      inc_interm += (inc_factor/niter);
      test_val = fabs((double)(inc_factor/niter))*xg->step;
    }
    else if (indx_interm < indx_now)
    {
      niter *= (-2);
      indx_prev = indx_now;
      inc_prev = inc_now;
      inc_interm += (inc_factor/niter);
      test_val = fabs((double)(inc_factor/niter))*xg->step;
    }
    else
      break;
  }
  if (tour_interp_btn == GEODESIC)
  {
    xg->tinc[0] = tinc_start[0] + inc_prev*xg->delta*xg->tau[0];
    xg->tinc[1] = tinc_start[1] + inc_prev*xg->delta*xg->tau[1];
  }
  else if (tour_interp_btn == HOUSEHOLDER)
  {
    xg->tinc[0] = tinc_start[0] + inc_prev*xg->delta*xg->tau[0];
  }
  else if (tour_interp_btn == GIVENS)
  {
    for (j=0; j<5; j++)
      xg->tinc[j] = tinc_start[j] + inc_prev*xg->delta*xg->tau[j];
  }
  all_tour_reproject(xg);
}

int
check_tour(xgobidata *xg)
{
  int i, j, k;
  int return_val = 1;
  float **basis, indx_now;
  lcoords *planar_coords;

  XFlush(display);
  XSync(display, False);

  if (tour_interp_btn == GEODESIC)
  {
    xg->tinc[0] += (xg->delta * xg->tau[0]);
    xg->tinc[1] += (xg->delta * xg->tau[1]);
  }
  else if (tour_interp_btn == HOUSEHOLDER)
  {
    xg->tinc[0] += (xg->delta * xg->tau[0]);
  }
  else if (tour_interp_btn == GIVENS)
  {
    for (k=0; k<5; k++)
      xg->tinc[k] += (xg->delta * xg->tau[k]);
  }

  if (xg->is_pp_optimz)
  {           /* jumpsz controlled by scrollbar for pp */
    if (!xg->new_direction_flag)
    {
      return_val = 0;
      basis = (float **) XtMalloc(
        (unsigned int) 2*sizeof(float *));
      for (i=0; i<2; i++)
        basis[i] = (float *) XtMalloc(
          (unsigned int) xg->ncols_used*sizeof(float));

      planar_coords = (lcoords *) XtMalloc(
        (unsigned int) xg->nrows*sizeof(lcoords));

      for (i=0; i<xg->ncols_used; i++)
        for (j=0; j<2; j++)
          basis[j][i] = xg->u[j][i];
      for (i=0; i<xg->nrows; i++)
      {
        planar_coords[i].x = xg->planar[i].x;
        planar_coords[i].y = xg->planar[i].y;
      }
      all_tour_reproject(xg);
      indx_now = pp_index_retval(xg);
      if (indx_now > indx_prev)
      {
        return_val = 1;
        indx_prev = indx_now;
        for (i=0; i<xg->ncols_used; i++)
          for (j=0; j<2; j++)
            xg->u[j][i] = basis[j][i];
        for (i=0; i<xg->nrows; i++)
        {
          xg->planar[i].x = planar_coords[i].x;
          xg->planar[i].y = planar_coords[i].y;
        }
      }
      else
      {
        find_max_indx_in_interval(xg, indx_now);
        increment_tour(xg);
        zero_indx_prev();
      }
      for (k=0; k<2; k++)
        XtFree((XtPointer)basis[k]);
      XtFree((XtPointer)basis);
      XtFree((XtPointer)planar_coords);
    }
    else
      return_val = 0;
  }
  else  /* traditional tour */
  {
    if (tour_interp_btn == GEODESIC)
    {
      if (xg->tour_cont_fact == infinite)
      {
        if (xg->new_direction_flag)
	{
          return_val = 0;
          xg->new_direction_flag = False;
	}
      }
      else if (xg->is_local_scan && xg->local_scan_dir == IN)
      {
        if ((xg->tinc[0] > xg->tau[0]) || (xg->tinc[1] > xg->tau[1]) ||
          ((xg->tinc[0] == xg->tau[0]) && (xg->tinc[1] == xg->tau[1])))
            return_val = 0;
      }
      else
      {
        if ((xg->tinc[0] > xg->fcont_fact*xg->tau[0]) || 
          (xg->tinc[1] > xg->fcont_fact*xg->tau[1]) ||
          ((xg->tinc[0] == xg->fcont_fact*xg->tau[0]) && 
          (xg->tinc[1] == xg->fcont_fact*xg->tau[1])))
            return_val = 0;
      }
    }
    else if (tour_interp_btn == HOUSEHOLDER)
    {
      if (xg->tour_cont_fact == infinite)
      {
        if (xg->new_direction_flag)
	{
          return_val = 0;
          xg->new_direction_flag = False;
	}
      }
      else if (xg->is_local_scan && xg->local_scan_dir == IN)
      {
        if (xg->tinc[0] >= xg->tau[0])
          return_val = 0;
      }
      else
      {
        if (xg->tinc[0] >= xg->fcont_fact*xg->tau[0])
          return_val = 0;
      }
    }
    else if (tour_interp_btn == GIVENS)
    {
      if (xg->tour_cont_fact == infinite)
      {
        if (xg->new_direction_flag)
	{
          return_val = 0;
          xg->new_direction_flag = False;
	}
      }
      else if (xg->is_local_scan && xg->local_scan_dir == IN)
      {
        if ((fabs(xg->tinc[0]) > fabs(xg->tau[0])) ||
          (fabs(xg->tinc[1]) > fabs(xg->tau[1])) ||
          (fabs(xg->tinc[2]) > fabs(xg->tau[2])) ||
          (fabs(xg->tinc[3]) > fabs(xg->tau[3])) ||
          (fabs(xg->tinc[4]) > fabs(xg->tau[4])) ||
          (fabs(xg->tinc[0]) == fabs(xg->tau[0]) && 
           fabs(xg->tinc[1]) == fabs(xg->tau[1]) &&
           fabs(xg->tinc[2]) == fabs(xg->tau[2]) &&
           fabs(xg->tinc[3]) == fabs(xg->tau[3]) &&
           fabs(xg->tinc[4]) == fabs(xg->tau[4])))
          return_val = 0;
      }
      else
      {
        if ((fabs(xg->tinc[0]) > fabs(xg->fcont_fact*xg->tau[0])) ||
          (fabs(xg->tinc[1]) > fabs(xg->fcont_fact*xg->tau[1])) ||
          (fabs(xg->tinc[2]) > fabs(xg->fcont_fact*xg->tau[2])) ||
          (fabs(xg->tinc[3]) > fabs(xg->fcont_fact*xg->tau[3])) ||
          (fabs(xg->tinc[4]) > fabs(xg->fcont_fact*xg->tau[4])) ||
          (fabs(xg->tinc[0]) == fabs(xg->fcont_fact*xg->tau[0]) && 
           fabs(xg->tinc[1]) == fabs(xg->fcont_fact*xg->tau[1]) &&
           fabs(xg->tinc[2]) == fabs(xg->fcont_fact*xg->tau[2]) &&
           fabs(xg->tinc[3]) == fabs(xg->fcont_fact*xg->tau[3]) &&
           fabs(xg->tinc[4]) == fabs(xg->fcont_fact*xg->tau[4])))
          return_val = 0;
      }
    }
  }
  return(return_val);
}

void
tour_proc(xgobidata *xg)
{
/*
 * This is the work proc which takes care of motion during touring.
*/

/*
 * Continue incremental movement between bases.
*/
  if (check_tour(xg))
  {
    increment_tour(xg);
  }
/*
 * Calculation of new path for various different modes.
*/
  else
  {
/*
 * When reached the target basis turn off the tour proc if is_stepping is on
 * or if reduced number of variables to 2.
*/
    if ((xg->is_stepping && !next_step) || ready_to_stop_now)
    {
      if (xg->is_pp_optimz || xg->is_princ_comp)
      {/* skip the fade-out; jump down to two variables */
        if (xg->is_pp && !xg->is_pp_optimz)
          pp_index(xg,0,1);
        determine_endbasis_and_path(xg);
        world_to_plane(xg);
        plane_to_screen(xg);
        plot_once(xg);
        tour_var_lines(xg);
        if (xg->tour_link_state == send_state)
        {
          xg->new_basis_ind = True;
          OWN_TOUR_SELECTION ;
          announce_tour_coefs(xg);
        }
      }
      next_step = True;
      xg->run_tour_proc = False;
      ready_to_stop_now = False;
      determine_endbasis_and_path(xg);

/* turn on pause button to indicate that tour is paused for pause due 
to stopping at a bitmap view but not when fading variables to 2. */
      if (!xg->is_stepping && xg->numvars_t > 2)
        reset_tour_pause_cmd(xg);
    }
    else
    {
/*
 * Do a final projection into the ending plane if just finished a tour
*/
      do_last_increment(xg);

/* calculate and plot projection pursuit index */
      if (xg->is_pp && !xg->is_pp_optimz)
        pp_index(xg,0,1);

      determine_endbasis_and_path(xg);

      if (counting_to_stop == 2)
        counting_to_stop--;
      else if (counting_to_stop == 1)
      {
        counting_to_stop--;
        ready_to_stop_now = True;
      }
    }
  }
/*
 * Don't allow backtracking to be turned on before there are
 * two bases in the history record.  This takes care of problems
 * that occur if backtracking is turned on before there is
 * anything to backtrack toward.
*/
  if (bt_firsttime && xg->nhist_list >= 3 && !xg->is_pp_optimz)
  {
    reset_backtrack_cmd(false, false, true, false);
    if (xg->data_mode != Sprocess)
      reset_tourhist_cmds(xg, 1);
      bt_firsttime = 0;
  }
}

void
draw_tour_axes(xgobidata *xg)
{
  int j, k;
  int naxes;
  int raw_axis_len = MIN(xg->mid.x, xg->mid.y);
  float tol = .01;
  fcoords axs;
  char str[COLLABLEN];
  icoords cntr;

  /* Use the width of the plot window to normalize the axes */
  axs.x = raw_axis_len / 2.;
  axs.y = raw_axis_len / 2.;

  naxes = 0;

  if (xg->is_axes_centered) {
    cntr.x = xg->cntr.x;
    cntr.y = xg->cntr.y;
  } else {
    axs.x /= 2.0;
    axs.y /= 2.0;
    cntr.x = 1.1 * axs.x;
    cntr.y = xg->plotsize.height - (1.1 * axs.y);
  }

/*
 * modifications to always plot the axes based on the raw data,
 * regardless of principal components.
*/
  if (xg->is_princ_comp)
  { 
    if (!xg->is_pc_axes)
    {
      invert_proj_coords(xg);
      for (j=0; j<xg->ncols_used; j++)
      {
        if ((xg->tv[0][j]*xg->tv[0][j] + xg->tv[1][j]*xg->tv[1][j]) > tol)
        {
          tour_axes[naxes].x1 = cntr.x;
          tour_axes[naxes].x2 = cntr.x +
            (int) (axs.x * xg->tv[0][j]);
          tour_axes[naxes].y1 = cntr.y;
          tour_axes[naxes].y2 = cntr.y -
  	        (int) (axs.y * xg->tv[1][j]);
          strcpy(xg->tour_lab[naxes], xg->collab_tform2[j]);
          naxes++;
        }
      }
    }
    else if (xg->is_pc_axes)
    {
      /*      set_sph_labs(xg, xg->nsph_vars); this sets the tform panel labels
            should be separate from axis labels */
      for (j=0; j<xg->ncols_used; j++)
      {
        if ((xg->u[0][j]*xg->u[0][j] + xg->u[1][j]*xg->u[1][j]) > tol)
        {
          tour_axes[naxes].x1 = cntr.x;
          tour_axes[naxes].x2 = cntr.x +
            (int) (axs.x * xg->u[0][j]);
          tour_axes[naxes].y1 = cntr.y;
          tour_axes[naxes].y2 = cntr.y -
            (int) (axs.y * xg->u[1][j]);
          sprintf(str, "PC %d", naxes+1);
          strcpy(xg->tour_lab[naxes], str);
          naxes++;
        }
      }
    }
  }
  else
  {
    for (j=0; j<xg->ncols_used; j++)
    {
      if ((xg->u[0][j]*xg->u[0][j] + xg->u[1][j]*xg->u[1][j]) > tol)
      {
        tour_axes[naxes].x1 = cntr.x;
        tour_axes[naxes].x2 = cntr.x +
          (int) (axs.x * xg->u[0][j]);
        tour_axes[naxes].y1 = cntr.y;
        tour_axes[naxes].y2 = cntr.y -
          (int) (axs.y * xg->u[1][j]);
        strcpy(xg->tour_lab[naxes], xg->collab_tform2[j]);
        naxes++;
      }
    }
  }

/*
 * Draw axes of rotation.
*/
  XSetForeground(display, copy_GC, plotcolors.fg);
  XDrawSegments(display, xg->pixmap0, copy_GC, tour_axes, naxes);
/*
 * Add axis labels.
*/
  for (k=0; k<naxes; k++)
  {
    XDrawString(display, xg->pixmap0, copy_GC,
       tour_axes[k].x2 + 6, tour_axes[k].y2 - 5,
       xg->tour_lab[k], strlen(xg->tour_lab[k]));
  }
}

void
init_V(xgobidata *xg)
{
  int i, j;

  for (i=0; i<2; i++)
    for (j=0; j<xg->ncols_used; j++)
      xg->v0[i][j] = xg->v1[i][j] = 0.0;

  xg->v0[0][xg->tour_vars[0]] = xg->v1[0][xg->tour_vars[0]] = 1.0;
  xg->v0[1][xg->tour_vars[1]] = xg->v1[1][xg->tour_vars[1]] = 1.0;
}

void
zero_princ_angles(xgobidata *xg)
{
  xg->coss[0] = xg->sins[1] = 1.;
  xg->sins[0] = xg->coss[1] = 0.;
  xg->icoss[0] = xg->isins[1] = PRECISION2;
  xg->isins[0] = xg->icoss[1] = 0;
}

void
set_tourvar(xgobidata *xg, int varno)
{
  int i, j;
  int selected = 0;
  int found, itmp;

    xg->run_tour_proc = False;
    for (i=0; i<xg->numvars_t; i++)
      if (varno == xg->tour_vars[i])
        selected = 1;
    /*
     * deselect variable if numvars_t will not be reduced to 1
    */
    if (selected)
    {
      if (xg->numvars_t > 2)
      {
        xg->varchosen[varno] = False;
        i = 0;
        found = 0;
        while (!found)
        {
          if (varno == xg->tour_vars[i])
          {
            itmp = i;
            /*
             * Make current position the new starting plane
             * and make the new ending plane the current position
             * with 0 in indx'th place of tour_vars.
            */
            init_basis(xg);
  /*
   * This is necessary to prevent problems when deselecting either variable
   * X or Y in the first projection. Now if either of the two variables
   * is deselected the first variable in the tour_vars that is not
   * contributing to the current projection becomes the new X or Y variable
  */
            if (xg->nhist_list == 1 && xg->step == 0.0)
            {
              check_deselection(varno, xg);
            }
  /*
   * all other cases of deselection
  */
            else
            {
  /*
   * generate new ending basis with deselected variable zeroed out, only
   * necessary in the case of not being princ_comp of optimz because these
   * cases are dealt with later.
  */
              if (!(xg->is_pp_optimz || xg->is_princ_comp))
              {
                for (i=0; i<2; i++)
                  xg->u1[i][varno] = 0.0;
              }
            }
            /*
             * now refresh vboxes and labels and tour_vars
            */
            xg->numvars_t -= 1;
            for (j=itmp; j<xg->numvars_t; j++)
              xg->tour_vars[j] = xg->tour_vars[j+1];

            /* dfs, testing */
             if (xg->is_princ_comp)
               refresh_all_var_labels(xg);
            /*              */

            found = 1;
            refresh_vbox(xg, varno, 1);
          }
          i++;
        } /* end while */
  
  /*
   * keep_sphered data updated to active variables
  */
        if (xg->is_princ_comp && xg->ncols_used >= 2)
        {
          if (update_vc_active_and_do_svd(xg, xg->numvars_t, xg->tour_vars))
            spherize_data(xg, xg->numvars_t, xg->numvars_t, xg->tour_vars);
	  /*          else
            copy_tform_to_sphered(xg);*/
        } 
  /*
   * Cater for the case of principal components or optimize is on, when
   * the behaviour should be to completely blank out the variable
   * immediately, rather than fade it out.
  */
	  /*        if (xg->is_pp_optimz)*/
        if (xg->is_princ_comp || xg->is_pp_optimz)
        {
          xg->u[0][varno] = 0.0;
          norm(xg->u[0], xg->ncols_used);
          xg->u[1][varno] = 0.0;
          norm(xg->u[1], xg->ncols_used);
          gram_schmidt(xg->u[0], xg->u[1], xg->ncols_used);
          init_basis(xg);
          copy_basis(xg->u0, xg->u1, xg->ncols_used);
          copy_u0_to_pd0(xg);
          copy_basis(xg->u0, xg->v1, xg->ncols_used);
          update_lims(xg);
          update_world(xg);
          zero_tau(xg);
          zero_princ_angles(xg);
          world_to_plane(xg);
          plane_to_screen(xg);
          plot_once(xg);
          reset_var_labels(xg, !xg->is_princ_comp);
          reset_one_label(xg, varno, 1);/* bug fix: sphering transformation */
          tour_var_lines(xg);
          xg->new_direction_flag = True;
        }
  
  /* recalculate section variables */
        if (xg->is_tour_section)
        {
          set_tour_section_eps(0., xg, 0);
          tour_section_calcs(xg, 0);
        }
  /*
   * reset pp_plot if in projection pursuit, because the scale changes
   * depending on variables included.
  */
        if (xg->is_pp)
        {
          reset_pp_plot();
          xg->recalc_max_min = True;
        }
        /*
         * zero tau to force new tour from current position
        */
        zero_tau(xg);
        if (!xg->is_princ_comp)
        {
          set_fading_var(2);
          set_sens_localscan(false);
          reset_backtrack_cmd(false, false, false, false);
          if (xg->numvars_t == 2)
          {
            set_ready_to_stop_now(0);
            set_counting_to_stop(1);
          }
        }
      }
      if (xg->tour_cont_fact == infinite)
        xg->new_direction_flag = True;
    }
  
    else  /* if (!selected) */
    { /* add variable */
      if (!var_frozen(xg, varno)) /* don't add if var is frozen */
      {
        found = add_variable(xg, varno);
  
        if (xg->is_princ_comp && xg->ncols_used >= 2)
        {
          if (update_vc_active_and_do_svd(xg, xg->numvars_t, xg->tour_vars))
            spherize_data(xg, xg->numvars_t, xg->numvars_t, xg->tour_vars);
	  /*          else
            copy_tform_to_sphered(xg);*/
        }
        if (xg->is_princ_comp && check_singular_vc())
        {
          i = 0;
          found = 0;
          /* take the variable out of the sphered group - 
           * warning message is generated. awful programming
           * but this is the way that it is set up currently.
          */
          while (!found)
          {
            if (varno == xg->tour_vars[i])
            {
              itmp = i;
              xg->numvars_t -= 1;
              for (j=itmp; j<xg->numvars_t; j++)
                xg->tour_vars[j] = xg->tour_vars[j+1];
              found = 1;
            }
            i++;
          } /* end while */
          if (update_vc_active_and_do_svd(xg, xg->numvars_t, xg->tour_vars))
            spherize_data(xg, xg->numvars_t, xg->numvars_t, xg->tour_vars);
	  /*          else
            copy_tform_to_sphered(xg);*/
        }
        else
        {
          if (xg->is_princ_comp)
          {
            init_basis(xg);
            update_lims(xg);
            update_world(xg);
            world_to_plane(xg);
            plane_to_screen(xg);
            plot_once(xg);
            tour_var_lines(xg);
            reset_var_labels(xg, !xg->is_princ_comp);
          }
          refresh_vbox(xg, varno, 1);
          
          /* recalculate section variables */
          if (xg->is_tour_section)
          {
            set_tour_section_eps(0., xg, 0);
            tour_section_calcs(xg, 0);
          }
    
          if (xg->is_pp)
          {
            reset_pp_plot();
            xg->recalc_max_min = 1;
          }
    
          /*
           * zero tau to force new tour from current position
          */
          zero_tau(xg);
    
          if (xg->is_pp_optimz)
            xg->new_direction_flag = True;
    
          if (!found)
            (void) fprintf(stderr, "error: didn't find indexed variable \n");
    
          if (xg->tour_cont_fact == infinite)
            xg->new_direction_flag = True;
        }
      }
    }
    if (tour_on(xg) == True)
      start_tour_proc(xg);
}
