/* corr.c */
/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  this  code and associated documentation,  provided      *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *     Copyright (c) of code not pertaining to manual       *
 *     control is owned by Dianne Cook (1994, 1995).        *
 *     Copyright (c) of code pertaining to manual           *
 *     control of the correlation tour is owned jointly by  *
 *     Dianne Cook and AT&T Bell Labs (1995).               *
 *  All Rights Reserved.                                    *
 *                                                          *
 *  We welcome your questions and comments, and request     *
 *  that you share any modifications with us.               *
 *                                                          *
 *      Dianne Cook                Andreas Buja             *
 *    dicook@iastate.edu     andreas@research.att.com       *
 * www.public.iastate.edu/~dicook/                          *
 *                         www.research.att.com/~andreas/   *
 *                                                          *
 ************************************************************/

#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

Widget corr_panel, corr_cmd[8], corr_label[3], corr_speed_sbar;

long **cxi0, **cxi1, **cxif;
static float tau_x, tau_y;
static float tinc_x, tinc_y;
static float corr_dv, corr_delta, corr_step;
/*
static Boolean xvar_to_fade = False;
static int xvar_fading;
static Boolean yvar_to_fade = False;
static int yvar_fading;
*/
/* variables to determine whether to stop corr_proc */
/*static int counting_to_stop;*/
static Boolean ready_to_stop_now;

static XSegment *corr_axes;
static char **corr_lab;
static hist_rec *cfl, *ccurr, *chfirst, *chend;
static int cnhist_list, old_cnhist_list, max_cnhist_list, cbt_dir;
static float *didx, *didy;
static Boolean cwait_for_more_vars;
static float indx_prev = 0.;

#define PAUSE corr_cmd[0]
#define REINIT corr_cmd[1]
#define CBACKTRACK corr_cmd[2]
#define NBASES_HISTORY corr_label[0]
#define CDIRECTION corr_cmd[3]
#define SYNC_AXES corr_cmd[4]
#define SPHERE corr_cmd[5]
#define CPURSUIT corr_cmd[6]

/* For setting the type of manipulation */
Widget cmanip_type_menu_label, cmanip_type_menu_cmd, cmanip_type_menu, 
  cmanip_type_menu_btn[4];
char *cmanip_type_menu_name[] = {"Vert", "Horiz", "Combined", "EqulComb"};
#define VERTICAL_BTN  cmanip_type_menu_btn[0]
#define HORIZONTAL_BTN cmanip_type_menu_btn[1]
#define COMB_BTN  cmanip_type_menu_btn[2]
#define EQ_COMB_BTN  cmanip_type_menu_btn[3]
#define VERTICAL_NAME  cmanip_type_menu_name[0]
#define HORIZONTAL_NAME cmanip_type_menu_name[1]
#define COMB_NAME  cmanip_type_menu_name[2]
#define EQ_COMB_NAME  cmanip_type_menu_name[3]

static float **cmanip_xvar_basis, **cmanip_yvar_basis;
static float xphi, yphi, xcosphi, xsinphi, ycosphi, ysinphi;
static int actual_nxvars, actual_nyvars;

/* For setting continuation factor */
Widget ccont_fact_menu_label, ccont_fact_menu_cmd, ccont_fact_menu, 
  ccont_fact_menu_btn[9];
char *ccont_fact_menu_name[] = {"1/10", "1/5", "1/4", "1/3", "1/2", "1", 
				  "2", "10", "Infinite"};

#define TENTH_BTN ccont_fact_menu_btn[0]
#define FIFTH_BTN ccont_fact_menu_btn[1]
#define QUARTER_BTN ccont_fact_menu_btn[2]
#define THIRD_BTN ccont_fact_menu_btn[3]
#define HALF_BTN ccont_fact_menu_btn[4]
#define ONE_BTN ccont_fact_menu_btn[5]
#define TWO_BTN ccont_fact_menu_btn[6]
#define TEN_BTN ccont_fact_menu_btn[7]
#define INFINITE_BTN ccont_fact_menu_btn[8]

#define TENTH_NAME ccont_fact_menu_name[0]
#define FIFTH_NAME ccont_fact_menu_name[1]
#define QUARTER_NAME ccont_fact_menu_name[2]
#define THIRD_NAME ccont_fact_menu_name[3]
#define HALF_NAME ccont_fact_menu_name[4]
#define ONE_NAME ccont_fact_menu_name[5]
#define TWO_NAME ccont_fact_menu_name[6]
#define TEN_NAME ccont_fact_menu_name[7]
#define INFINITE_NAME ccont_fact_menu_name[8]

void
alloc_corr(xgobidata *xg)
{
  unsigned int nc = (unsigned int) xg->ncols;
  unsigned int nc1 = (unsigned int) (xg->ncols + 1);
  unsigned int nc2 = (unsigned int) (2*xg->ncols);
  unsigned int nr = (unsigned int) xg->nrows;
  unsigned int nx = (unsigned int) xg->ncorr_xvars;
  unsigned int ny = (unsigned int) xg->ncorr_yvars;
  int i, j, k;

  xg->corr_xvars = (int *) XtMalloc(nc * sizeof(int));
  xg->corr_yvars = (int *) XtMalloc(nc * sizeof(int));

  xg->cu0 = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (k=0; k<2; k++)
    xg->cu0[k] = (float *) XtMalloc(nc * sizeof(float));

  xg->cu1 = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (k=0; k<2; k++)
    xg->cu1[k] = (float *) XtMalloc(nc * sizeof(float));

  xg->cu = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (k=0; k<2; k++)
    xg->cu[k] = (float *) XtMalloc(nc * sizeof(float));

  xg->cuold = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (k=0; k<2; k++)
    xg->cuold[k] = (float *) XtMalloc(nc * sizeof(float));

  cxi0 = (long **) XtMalloc(nr * sizeof(long *));
  for (k=0; k<nr; k++)
    cxi0[k] = (long *) XtMalloc(2 * sizeof(long));

  cxi1 = (long **) XtMalloc(nr * sizeof(long *));
  for (k=0; k<nr; k++)
    cxi1[k] = (long *) XtMalloc(2 * sizeof(long));

/* nrows x 2 frozen variables*/
  cxif = (long **) XtMalloc(nr * sizeof(long *));
  for (i=0; i<nr; i++)
    cxif[i] = (long *) XtMalloc((unsigned int) 2 * sizeof(long));

/* Axes and axis labels */
  corr_axes = (XSegment *) XtMalloc(nc2 * sizeof(XSegment));
  corr_lab = (char **) XtMalloc(nc1 * sizeof (char *));
  for (j=0; j<nc1; j++)
    corr_lab[j] = (char *) XtMalloc(
      (unsigned int) COLLABLEN * sizeof(char));

/* Correlation pursuit index and derivatives */

  /* Create arrays of size ??? to hold index and derivatives here */

  didx = (float *) XtMalloc(nx * sizeof(float));
  didy = (float *) XtMalloc(ny * sizeof(float));

/* corr controls */
  cmanip_xvar_basis = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (k=0; k<2; k++)
    cmanip_xvar_basis[k] = (float *) XtMalloc(nc * sizeof(float));
  cmanip_yvar_basis = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (k=0; k<2; k++)
    cmanip_yvar_basis[k] = (float *) XtMalloc(nc * sizeof(float));

/* frozen variables */
  xg->corr_xfrozen_vars = (int *) XtMalloc(nc * sizeof(int));
  xg->corr_yfrozen_vars = (int *) XtMalloc(nc * sizeof(int));

  xg->cufrozen = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->cufrozen[i] = (float *) XtMalloc(nc * sizeof(float));
  xg->cuwarm = (float **) XtMalloc((unsigned int) 2 * sizeof(float *));
  for (i=0; i<2; i++)
    xg->cuwarm[i] = (float *) XtMalloc(nc * sizeof(float));
}

void
free_corr(xgobidata *xg)
{
  int j, k;
  unsigned int nr = (unsigned int) xg->nrows;

  XtFree((XtPointer) xg->corr_xvars);
  XtFree((XtPointer) xg->corr_yvars);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->cu[k]);
  XtFree((XtPointer) xg->cu);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->cu0[k]);
  XtFree((XtPointer) xg->cu0);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->cu1[k]);
  XtFree((XtPointer) xg->cu1);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->cuold[k]);
  XtFree((XtPointer) xg->cuold);

  for (k=0; k<nr; k++)
    XtFree((XtPointer) cxi0[k]);
  XtFree((XtPointer) cxi0);
  for (k=0; k<nr; k++)
    XtFree((XtPointer) cxi1[k]);
  XtFree((XtPointer) cxi1);

  XtFree((XtPointer) corr_axes);
  for (j=0; j<xg->ncols; j++)
    XtFree((XtPointer) corr_lab[j]);
  XtFree((XtPointer) corr_lab);

  for (k=0; k<2; k++)
    XtFree((XtPointer) cmanip_xvar_basis[k]);
  XtFree((XtPointer) cmanip_xvar_basis);
  for (k=0; k<2; k++)
    XtFree((XtPointer) cmanip_yvar_basis[k]);
  XtFree((XtPointer) cmanip_yvar_basis);

  XtFree((XtPointer) didx);
  XtFree((XtPointer) didy);

  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->cufrozen[k]);
  XtFree((XtPointer) xg->cufrozen);
  for (k=0; k<2; k++)
    XtFree((XtPointer) xg->cuwarm[k]);
  XtFree((XtPointer) xg->cuwarm);
  XtFree((XtPointer) xg->corr_xfrozen_vars);
  XtFree((XtPointer) xg->corr_yfrozen_vars);
}

void
AllocCorrBox(xgobidata *xg)
/*
 * This routine allocates space for a new node if there are no unused nodes
 * on the free list, fl.
*/
{
  if (cfl == NULL)
  {
    ccurr = (hist_rec *) XtMalloc(
      (unsigned int) sizeof(hist_rec));
    ccurr->hist[0] = (float *) XtMalloc(
      (unsigned int) xg->ncols * sizeof(float));
    ccurr->hist[1] = (float *) XtMalloc(
      (unsigned int) xg->ncols * sizeof(float));
  }
  else
  {
    ccurr = cfl;
    cfl = cfl->prev;
  }
}

void
store_corr_basis(xgobidata *xg)
{
/*
 * This routine saves a basis, u0, into the history list.
*/
  int i, j;
  hist_rec *prev;
  int too_close;

/*
 * Initial node doubly linked to itself, used when tour is started.
*/
  if (ccurr == NULL)
  {
    AllocCorrBox(xg);
    for (i=0; i<xg->ncols_used; i++)
      for (j=0; j<2; j++)
        ccurr->hist[j][i] = xg->cu0[j][i];
    ccurr->next = ccurr;
    ccurr->prev = ccurr;
    chfirst = ccurr;
    old_cnhist_list = cnhist_list;
    cnhist_list++;
  }
  else
  /* every other case */
  {
    too_close = check_proximity(xg->cu0, ccurr->hist, xg->ncols_used);
  
    if (!too_close)
    {
      prev = ccurr;
      if ((cnhist_list <= MAXHIST))
      {
  /*
   * Add in new node and remake circular list
  */
        AllocCorrBox(xg);
        for (i=0; i<xg->ncols_used; i++)
          for (j=0; j<2; j++)
            ccurr->hist[j][i] = xg->cu0[j][i];
        ccurr->prev = prev;
        ccurr->next = chfirst;
        prev->next = ccurr;
        chfirst->prev = ccurr;
        old_cnhist_list = cnhist_list;
        if (cnhist_list != MAXHIST)
          cnhist_list++;
      }
      else
      {
        /*
         * Case where MAXHIST has been reached. Move pointer to
         * next node in list and overwrite base.
        */
        ccurr = ccurr->next;
        for (i=0; i<xg->ncols_used; i++)
          for (j=0; j<2; j++)
            ccurr->hist[j][i] = xg->cu0[j][i];
        chfirst = ccurr->next;
      }
    }
  }
}

void
start_corr_proc(xgobidata *xg)
{
  if (!xg->is_corr_paused)
  {
    xg->run_corr_proc = True;
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, NULL);
  }
}

void
stop_corr_proc(xgobidata *xg)
{
  xg->run_corr_proc = False;
}

void
corr_var_lines(xgobidata *xg)
{
  int j;
  int x, y;
  int r = xg->radius;
  float fr = (float) xg->radius;
  Window vwin;
  XGCValues *gcv, gcv_inst;

  XGetGCValues(display, varpanel_xor_GC, GCCapStyle|GCJoinStyle, &gcv_inst);
  gcv = &gcv_inst;

  if (xg->is_corr_touring)
  {
    for (j=0; j<xg->ncols_used; j++)
    {
      if (xg->varchosen[j])
        XSetLineAttributes(display, varpanel_xor_GC, 2, LineSolid,
          gcv->cap_style, gcv->join_style);

      vwin = XtWindow(xg->vardraww[j]);
      x = INT(fr * xg->cuold[0][j]);
      y = INT(fr * xg->cuold[1][j]);
      ChooseVar(vwin, r, x, y);
      x = INT(fr * xg->cu[0][j]);
      y = INT(fr * xg->cu[1][j]);
      ChooseVar(vwin, r, x, y);
      xg->cuold[0][j] = xg->cu[0][j];
      xg->cuold[1][j] = xg->cu[1][j];

      if (xg->varchosen[j])
        XSetLineAttributes(display, varpanel_xor_GC, 0, LineSolid,
          gcv->cap_style, gcv->join_style);
    }
  }
}

void
init_corr_basis(xgobidata *xg)
{

  if (xg->ncxfrozen_vars > 0 && xg->ncyfrozen_vars > 0)
  {
    copy_basis(xg->cuwarm, xg->cu0, xg->ncols_used);
    norm(xg->cu0[0], xg->ncols_used);
    norm(xg->cu0[1], xg->ncols_used);
  }
  else if (xg->ncxfrozen_vars > 0 && xg->ncyfrozen_vars == 0)
  {
    copy_vector(xg->cuwarm[0], xg->cu0[0], xg->ncols_used);
    norm(xg->cu0[0], xg->ncols_used);
    copy_vector(xg->cu[1], xg->cu0[1], xg->ncols_used);
  }
  else if (xg->ncxfrozen_vars == 0 && xg->ncyfrozen_vars > 0)
  {
    copy_vector(xg->cuwarm[1], xg->cu0[1], xg->ncols_used);
    norm(xg->cu0[1], xg->ncols_used);
    copy_vector(xg->cu[0], xg->cu0[0], xg->ncols_used);
  }
  else
    copy_basis(xg->cu, xg->cu0, xg->ncols_used);
}

void
zero_corr_tincs(void)
{
  tinc_x = tinc_y = 0.;
}

void
zero_corr_taus()
{
  tau_x = tau_y = 0.;
}

void
ci_span_planes(xgobidata *xg)
{
  int i, j, m;

  for (m=0; m<xg->nrows_in_plot; m++)
  {
    i = xg->rows_in_plot[m];
    for (j=0; j<xg->ncols_used; j++)
      xg->tnx[j] = FLOAT(xg->world_data[i][j]);
    cxi0[i][0] = (long)(inner_prod(xg->tnx, cmanip_xvar_basis[0], 
      xg->ncols_used));
    cxi0[i][1] = (long)(inner_prod(xg->tnx, cmanip_yvar_basis[0], 
      xg->ncols_used));
    cxi1[i][0] = (long)(inner_prod(xg->tnx, cmanip_xvar_basis[1], 
      xg->ncols_used));
    cxi1[i][1] = (long)(inner_prod(xg->tnx, cmanip_yvar_basis[1], 
      xg->ncols_used));
  }
}

void 
ci_reproject(xgobidata *xg)
{
  int i, j, m;

  if (actual_nxvars > 1) 
  {
    for (j=0; j<xg->ncols_used; j++)
      xg->cu[0][j] = xcosphi * cmanip_xvar_basis[0][j] + 
       xsinphi * cmanip_xvar_basis[1][j];
  }

  if (actual_nyvars > 1)
  {
    for (j=0; j<xg->ncols_used; j++)
      xg->cu[1][j] = ycosphi * cmanip_yvar_basis[0][j] + 
       ysinphi * cmanip_yvar_basis[1][j];
  }

  if (actual_nxvars > 1) 
  {
    for (m=0; m<xg->nrows_in_plot; m++)
    {
      i = xg->rows_in_plot[m];
      xg->planar[i].x = (long) (xcosphi * (float) cxi0[i][0] + 
        xsinphi * (float) cxi1[i][0]);
    }
  }

  if (actual_nyvars > 1)
  {
    for (m=0; m<xg->nrows_in_plot; m++)
    {
      i = xg->rows_in_plot[m];
      xg->planar[i].y = (long)
        (ycosphi * (float) cxi0[i][1] + 
         ysinphi * (float) cxi1[i][1]);
      if (xg->ncyfrozen_vars > 0)
      {
        xg->planar[i].y = xg->planar[i].y / PRECISION2;
        xg->planar[i].y += cxif[i][1];
      }
    }
  }
}

void
make_corr_coord_basis(xgobidata *xg)
{
  int j;
  float tol = 0.0001, ftmp;
  
  for (j=0; j<xg->ncols_used; j++)
  {
    cmanip_xvar_basis[0][j] = xg->cu0[0][j];
    cmanip_yvar_basis[0][j] = xg->cu0[1][j];
    cmanip_xvar_basis[1][j] = 0.;
    cmanip_yvar_basis[1][j] = 0.;
  }
  cmanip_xvar_basis[1][xg->corr_xmanip_var] = 1.;
  cmanip_yvar_basis[1][xg->corr_ymanip_var] = 1.;
  if (actual_nxvars > 1)
  {
    gram_schmidt(cmanip_xvar_basis[0],  cmanip_xvar_basis[1],
      xg->ncols_used);
    ftmp = calc_norm(cmanip_xvar_basis[1], xg->ncols_used);
    while (ftmp < tol)
    {
      gen_norm_variates(1, xg->ncorr_xvars, xg->tv[0]);
      for (j=0; j<xg->ncols_used; j++)
        cmanip_xvar_basis[1][j] = 0.;
      for (j=0; j<xg->ncorr_xvars; j++)
        cmanip_xvar_basis[1][xg->corr_xvars[j]] = xg->tv[0][j];
      norm(cmanip_xvar_basis[1], xg->ncols_used);
      gram_schmidt(cmanip_xvar_basis[0],  cmanip_xvar_basis[1],
        xg->ncols_used);
      ftmp = calc_norm(cmanip_xvar_basis[1], xg->ncols_used);
    }
  }
  if (actual_nyvars > 1)
  {
    gram_schmidt(cmanip_yvar_basis[0],  cmanip_yvar_basis[1],
      xg->ncols_used);
    ftmp = calc_norm(cmanip_yvar_basis[1], xg->ncols_used);
    while (ftmp < tol)
    {
      gen_norm_variates(1, xg->ncorr_yvars, xg->tv[0]);
      for (j=0; j<xg->ncols_used; j++)
        cmanip_yvar_basis[1][j] = 0.;
      for (j=0; j<xg->ncorr_yvars; j++)
        cmanip_yvar_basis[1][xg->corr_yvars[j]] = xg->tv[0][j];
      norm(cmanip_yvar_basis[1], xg->ncols_used);
      gram_schmidt(cmanip_yvar_basis[0],  cmanip_yvar_basis[1],
        xg->ncols_used);
      ftmp = calc_norm(cmanip_yvar_basis[1], xg->ncols_used);
    }
  } 
  for (j=0; j<xg->ncols_used; j++)
  {
    xg->cuold[0][j] = xg->cu[0][j] = cmanip_xvar_basis[0][j];
    xg->cuold[1][j] = xg->cu[1][j] = cmanip_yvar_basis[0][j];
  }

  xphi = 0.;
  yphi = 0.;
  xcosphi = 1.;
  xsinphi = 0.;
  ycosphi = 1.;
  ysinphi = 0.;
}

/* ARGSUSED */
XtEventHandler
interact_corr(Widget w, xgobidata *xg, XEvent *evnt)
{
  int i;
  static icoords pos, prev_pos;
  float distx, disty;
  float denom = (float) MIN(xg->mid.x, xg->mid.y);
  Boolean no_overlap = False;

  if (evnt->type == ButtonPress)
  {
    XButtonEvent *xbutton = (XButtonEvent *) evnt;
    if (evnt->xbutton.button == 1 || evnt->xbutton.button == 2)
    {
      stop_corr_proc(xg);
      /* This turns all frozen variables off, but there should be no need
       * to turn the vertical off if only horizontal manipulation is done,
       * for example. */
      while (xg->ncxfrozen_vars > 0)
        set_cxfrozen_var(xg, xg->corr_xfrozen_vars[0]);
      while (xg->ncyfrozen_vars > 0)
        set_cyfrozen_var(xg, xg->corr_yfrozen_vars[0]);
 
      pos.x = prev_pos.x = xbutton->x;
      pos.y = prev_pos.y = xbutton->y;

      actual_nxvars = xg->ncorr_xvars;
      no_overlap = False;
      for (i=0; i<xg->ncorr_xvars; i++)
        if (xg->corr_xmanip_var == xg->corr_xvars[i])
          no_overlap = True;
      if (!no_overlap)
        actual_nxvars++;
      actual_nyvars = xg->ncorr_yvars;
      no_overlap = False;
      for (i=0; i<xg->ncorr_yvars; i++)
        if (xg->corr_ymanip_var == xg->corr_yvars[i])
          no_overlap = True;
      if (!no_overlap)
        actual_nyvars++;

      if (actual_nxvars > 1 || actual_nyvars > 1)
      {
        init_corr_basis(xg);
        store_corr_basis(xg); 
        make_corr_coord_basis(xg);

        ci_span_planes(xg);
        ci_reproject(xg);
        plane_to_screen(xg);
        plot_once(xg);
        corr_var_lines(xg);
      }
    }
  }
  else if (evnt->type == MotionNotify)
  {
    XMotionEvent *xmotion = (XMotionEvent *) evnt;
    prev_pos.x = pos.x;
    prev_pos.y = pos.y;
    pos.x = xmotion->x;
    pos.y = xmotion->y;

    if (actual_nxvars > 1 || actual_nyvars > 1)
    {
      if (xg->corr_manip_type == vertical)
      {
        distx = 0.;
        if (actual_nyvars > 1)
          disty = prev_pos.y - pos.y;
      }
      else if (xg->corr_manip_type == horizontal)
      {
        if (actual_nxvars > 1)
          distx = pos.x - prev_pos.x;
        disty = 0.;
      }
      else if (xg->corr_manip_type == combined)
      {
        if (actual_nxvars > 1)
          distx = pos.x - prev_pos.x;
        if (actual_nyvars > 1)
          disty = prev_pos.y - pos.y;
      }
      else if (xg->corr_manip_type == eqcomb)
      {
        if (actual_nxvars > 1)
          distx = pos.x - prev_pos.x;
        if (actual_nyvars > 1)
          disty = prev_pos.y - pos.y;
        if (fabs(distx) != fabs(disty))
	{
          distx = (distx+disty)/1.414214;
          disty = distx;
	}
      }
    
      xphi = xphi + distx / denom;
      yphi = yphi + disty / denom;
  
      xcosphi = (float) cos((double) xphi);
      xsinphi = (float) sin((double) xphi);
      if (xcosphi > 1.0)
      {
        xcosphi = 1.0;
        xsinphi = 0.0;
      }
      else if (xcosphi < -1.0)
      {
        xcosphi = -1.0;
        xsinphi = 0.0;
      }
      ycosphi = (float) cos((double) yphi);
      ysinphi = (float) sin((double) yphi);
      if (ycosphi > 1.0)
      {
        ycosphi = 1.0;
        ysinphi = 0.0;
      }
      else if (ycosphi < -1.0)
      {
        ycosphi = -1.0;
        ysinphi = 0.0;
      }
  
      ci_reproject(xg);
      plane_to_screen(xg);
      plot_once(xg);
      corr_var_lines(xg);
      if (xg->is_corr_pursuit)
        cp_index(xg,0,0);
    }
  }
  else if (evnt->type == ButtonRelease)
  {
    /* need to get current projection and store it */
    if (actual_nxvars > 1 || actual_nyvars > 1)
    {
      init_corr_basis(xg);
      store_corr_basis(xg);
      zero_corr_taus();/* in preparation for restarting */
      zero_corr_tincs();
      if (xg->corr_cont_fact == infinite)
        xg->new_corr_dir_flag = True;
    }
    actual_nxvars = 0;
    actual_nyvars = 0;
    start_corr_proc(xg);
  }
}

void
corr_event_handlers(xgobidata *xg, Boolean add)
{
  if (add)
  {
    XtAddEventHandler(xg->workspace,
      ButtonPressMask | ButtonReleaseMask |
      Button1MotionMask | Button2MotionMask,
      FALSE, (XtEventHandler) interact_corr, (XtPointer) xg);
    XDefineCursor(display, XtWindow(xg->workspace), spin_cursor);
  }
  else
  {
    XtRemoveEventHandler(xg->workspace, XtAllEvents,
      TRUE, (XtEventHandler) interact_corr, (XtPointer) xg);
    XDefineCursor(display, XtWindow(xg->workspace), default_cursor);
  }
}

void
reinit_corr(xgobidata *xg)
{
  XtCallCallbacks(REINIT, XtNcallback, (XtPointer) xg);
}

void
nback_update_clabel(void)
{
  char str[10];

  (void) sprintf(str, "%d", cnhist_list);
  XtVaSetValues(NBASES_HISTORY,
    XtNstring, (String) str,
    NULL);
}

void
reinit_corr_hist(void)
{
  hist_rec *histp;
/*
 * Free and then reinitialize the tour arrays.
*/
  if (cfl != NULL)
  {
    while (cfl->prev != NULL)
    {
      histp = cfl;
      cfl = cfl->prev;
      XtFree((XtPointer) histp->hist[0]);
      XtFree((XtPointer) histp->hist[1]);
      XtFree((XtPointer) histp);
    }
  }

  if (ccurr != NULL)
  {
    ccurr->next = NULL;
    chfirst->prev = NULL;
    while (ccurr->prev != NULL)
    {
      histp = ccurr;
      ccurr = ccurr->prev;
      XtFree((XtPointer) histp->hist[0]);
      XtFree((XtPointer) histp->hist[1]);
      XtFree((XtPointer) histp);
    }
  }
  chfirst = ccurr; 
}


void
retrieve_corr_basis(xgobidata *xg)
{
  int i, j;

  for (i=0; i<xg->ncols_used; i++)
    for (j=0; j<2; j++)
      xg->cu1[j][i] = ccurr->hist[j][i];
}

void
init_corr(xgobidata *xg)
{
  int j;

/* initialize first variable as dependent variable, and
 * second two as independent variables.
*/
  xg->ncorr_xvars = 2;
  xg->ncorr_yvars = 1;

  for (j=0; j<xg->ncols; j++)
    xg->corr_xvars[j] = xg->corr_yvars[j] = 0;
  xg->corr_xvars[0] = 1;
  xg->corr_xvars[1] = 2;
  xg->corr_yvars[0] = 0;

  xg->corr_xmanip_var = 1;
  xg->corr_ymanip_var = 0;
  xg->corr_manip_type = combined;

  xg->corr_cont_fact = one;
  xg->fccont_fact = 1;

  for (j=0; j<xg->ncols; j++)
    xg->cu[0][j] = xg->cu[1][j] = xg->cu0[0][j] = xg->cu0[1][j] = 
      xg->cu1[0][j] = xg->cu1[1][j] = xg->cuold[0][j] = xg->cuold[1][j] = 0.;

  xg->cu[0][xg->corr_xvars[0]] = xg->cu0[0][xg->corr_xvars[0]] = 
    xg->cu1[0][xg->corr_xvars[0]] = xg->cuold[0][xg->corr_xvars[0]] = 1.;
  xg->cu[1][xg->corr_yvars[0]] = xg->cu0[1][xg->corr_yvars[0]] = 
    xg->cu1[1][xg->corr_yvars[0]] = xg->cuold[1][xg->corr_yvars[0]] = 1.;

  tau_x = tau_y = 0.;
  tinc_x = tinc_y = 0.;
  corr_delta = 0.0;
  corr_dv = 1.0;
  corr_step = TOURSTEP0;

  xg->is_corr_touring = False;
  xg->run_corr_proc = False;
  xg->is_corr_bt = False;
  cnhist_list = 1;
  max_cnhist_list = MAXHIST;
  cbt_dir = BACKWARD;
  cfl = NULL;
  ccurr = NULL;
  chfirst = ccurr;

  xg->is_corr_syncd_axes = False;
  xg->new_corr_dir_flag = False;

/* For correlation pursuit.
   Currently using pp structures.
*/

  xg->cp_recalc_max_min = True;
  xg->xaxis_indent = XTextWidth(appdata.plotFont,
    "0000e-00", strlen("0000e-00") + ASCII_TEXT_BORDER_WIDTH);
  xg->cp_replot_freq = 10;

  xg->ncxfrozen_vars = 0;
  xg->ncyfrozen_vars = 0;
  for (j=0; j<xg->ncorr_xvars; j++)
    xg->corr_xfrozen_vars[j] = 0;
  for (j=0; j<xg->ncorr_yvars; j++)
    xg->corr_yfrozen_vars[j] = 0;
  
}

void
map_corr_panel(xgobidata *xg, Boolean is_corr_on)
{
  if (is_corr_on)
  {
    XtMapWidget(corr_panel);
    XtMapWidget(xg->corr_mouse);
  }
  else
  {
    XtUnmapWidget(corr_panel);
    XtUnmapWidget(xg->corr_mouse);
  }
}

void
increment_corr(xgobidata *xg)
{
  static int count = 0;

  world_to_plane(xg);
  if (xg->is_corr_pursuit)
  {
    count++;
    if (count >= xg->cp_replot_freq)
    {
      cp_index(xg,0,0);
      count = 0;
    }
  }
  plane_to_screen(xg);
  plot_once(xg);
  corr_var_lines(xg);
}

void
zero_cindx_prev(void)
{
  indx_prev = 0.;
}

void
find_max_cindx_in_interval(xgobidata *xg, float indx_now)
{
  int niter = -1;
  float indx_interm, tinc_x_start, tinc_y_start, test_val;
  float inc_factor = 0.625, inc_prev, inc_now, inc_interm;
  float tol = 0.001;

  tinc_x_start = tinc_x;
  tinc_y_start = tinc_y;

  inc_prev = -1.;
  inc_now = 0.;
  inc_interm = -inc_factor;
  test_val = fabs((double)(inc_factor/niter))*corr_step;

  while (test_val > tol)
  {
    tinc_x = tinc_x_start + inc_interm*corr_delta*tau_x;
    tinc_y = tinc_y_start + inc_interm*corr_delta*tau_y;
    corr_reproject(xg);
    get_corr_index(xg, &indx_interm, didx, didy);
    if (indx_interm > indx_now)
    {
      indx_now = indx_prev = indx_interm;
      inc_now = inc_prev = inc_interm;
      inc_interm += (inc_factor/niter);
      test_val = fabs((double)(inc_factor/niter))*corr_step;
    }
    else if (indx_interm < indx_now)
    {
      niter *= (-2);
      indx_prev = indx_now;
      inc_prev = inc_now;
      inc_interm += (inc_factor/niter);
      test_val = fabs((double)(inc_factor/niter))*corr_step;
    }
    else
      break;
  }
  tinc_x = tinc_x_start + inc_prev*corr_delta*tau_x;
  tinc_y = tinc_y_start + inc_prev*corr_delta*tau_y;
  corr_reproject(xg);
}

int
more_interp_togo(xgobidata *xg)
{
  int i, j, k, ret_val = 0;
  float tol = 0.001;
  float **basis, indx_now;
  lcoords *planar_coords;

  if (tau_x > tol)
    tinc_x += (corr_delta * tau_x);
  if (tau_y > tol)
    tinc_y += (corr_delta * tau_y);

  if (xg->is_corr_optimz)
  {
    if (!xg->new_direction_flag)
    {
      ret_val = 0;
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
      corr_reproject(xg);
      get_corr_index(xg, &indx_now, didx, didy);
      if (indx_now > indx_prev)
      {
        ret_val = 1;
        indx_prev = indx_now;
        for (i=0; i<xg->ncols_used; i++)
          for (j=0; j<2; j++)
            xg->cu[j][i] = basis[j][i];
        for (i=0; i<xg->nrows; i++)
        {
          xg->planar[i].x = planar_coords[i].x;
          xg->planar[i].y = planar_coords[i].y;
        }
      }
      else
      {
        find_max_cindx_in_interval(xg, indx_now);
        increment_corr(xg);
        zero_cindx_prev();
      }
      for (k=0; k<2; k++)
        XtFree((XtPointer)basis[k]);
      XtFree((XtPointer)basis);
      XtFree((XtPointer)planar_coords);
    }
    else
      ret_val = 0;
  }
  else
  {
    if (!xg->new_corr_dir_flag)
    {
      if (xg->corr_cont_fact == infinite || 
        tinc_x < xg->fccont_fact*tau_x || tinc_y < xg->fccont_fact*tau_y)
        ret_val = 1;
    }
    else
      xg->new_corr_dir_flag = False;
  }

  return(ret_val);
}

/*
void
set_counting_to_stop_corr(logical)
  int logical;
{
  counting_to_stop = logical;
}
*/

/*
void
set_ready_to_stop_corr_now(logical)
  Boolean logical;
{
  ready_to_stop_now = logical;
}
*/

void
do_last_corr_increment(xgobidata *xg)
{
  if (!xg->is_corr_optimz)
  {
    if (xg->corr_cont_fact != infinite)
    {/* shouldn't need to make this check */
      if (tinc_x != xg->fccont_fact*tau_x || tinc_y != xg->fccont_fact*tau_y)
      {
        tinc_x = xg->fccont_fact*tau_x;
        tinc_y = xg->fccont_fact*tau_y;
        world_to_plane(xg);
        plane_to_screen(xg);
        plot_once(xg);
        corr_var_lines(xg);
      }
    }
  }
}

void
new_corr_basis(xgobidata *xg)
{
  int j;

  for (j=0; j<xg->ncols_used; j++)
    xg->cu1[0][j] = xg->cu1[1][j] = xg->tv[0][j] = 0.;

  gen_norm_variates(1, xg->ncorr_xvars, xg->tv[0]);
  for (j=0; j<xg->ncorr_xvars; j++)
    xg->cu1[0][xg->corr_xvars[j]] = xg->tv[0][j];
  norm(xg->cu1[0], xg->ncols_used);

  if (!xg->is_corr_syncd_axes)
  {
    for (j=0; j<xg->ncols_used; j++)
      xg->tv[0][j] = 0.;
    gen_norm_variates(1, xg->ncorr_yvars, xg->tv[0]);
    for (j=0; j<xg->ncorr_yvars; j++)
      xg->cu1[1][xg->corr_yvars[j]] = xg->tv[0][j];
    norm(xg->cu1[1], xg->ncols_used);
  }
  else
  {
    for (j=0; j<xg->ncorr_yvars; j++)
      xg->cu1[1][xg->corr_yvars[j]] = xg->cu1[0][xg->corr_xvars[j]];
  }

}

void
corr_span_planes(xgobidata *xg)
{
  int i, j, m;

  for (m=0; m<xg->nrows_in_plot; m++)
  {
    i = xg->rows_in_plot[m];
    for (j=0; j<xg->ncols_used; j++)
      xg->tnx[j] = FLOAT(xg->world_data[i][j]);

    if (xg->ncxfrozen_vars > 0)
      cxif[i][0] = (long)(inner_prod(xg->tnx,
        xg->cufrozen[0], xg->ncols_used));

    if (xg->ncyfrozen_vars > 0)
      cxif[i][1] = (long)(inner_prod(xg->tnx,
        xg->cufrozen[1], xg->ncols_used));

    cxi0[i][0] = (long)(inner_prod(xg->tnx, xg->cu0[0], xg->ncols_used));
    cxi1[i][0] = (long)(inner_prod(xg->tnx, xg->cu1[0], xg->ncols_used));
    cxi0[i][1] = (long)(inner_prod(xg->tnx, xg->cu0[1], xg->ncols_used));
    cxi1[i][1] = (long)(inner_prod(xg->tnx, xg->cu1[1], xg->ncols_used));
  }
}

void
corr_reproject(xgobidata *xg)
/*
 * This routine uses the data projected into the span of
 * the starting basis and ending basis, and then rotates in this
 * space.
*/
{
  int i, j, m;
  float ctx, stx, cty, sty, tmpf1, tmpf2;
  int ictx, istx, icty, isty, tmpi1, tmpi2;
  long x_min, x_max, y_min, y_max;

  ctx = cos((double) tinc_x);
  stx = sin((double) tinc_x);
  ictx = (int)(ctx * PRECISION2);
  istx = (int)(stx * PRECISION2);

  if (xg->ncxfrozen_vars > 0)
  {
    tmpf1 = sqrt(1. - calc_norm_sq(xg->cufrozen[0], xg->ncols_used));
    tmpi1 = INT(tmpf1 * PRECISION2);
    for (j=0; j<xg->ncols_used; j++)
    {
      xg->cuwarm[0][j] = ctx * xg->cu0[0][j] + stx * xg->cu1[0][j];
      xg->cuwarm[0][j] *= tmpf1;
      xg->cu[0][j] = xg->cuwarm[0][j] + xg->cufrozen[0][j];
    }
  }
  else
  {
    for (j=0; j<xg->ncols_used; j++)
      xg->cu[0][j] = ctx * xg->cu0[0][j] + stx * xg->cu1[0][j];
  }
  
  cty = cos((double) tinc_y);
  sty = sin((double) tinc_y);
  icty = (int)(cty * PRECISION2);
  isty = (int)(sty * PRECISION2);

  if (xg->ncyfrozen_vars > 0)
  {
    tmpf2 = sqrt(1. - calc_norm_sq(xg->cufrozen[1], xg->ncols_used));
    tmpi2 = INT(tmpf2 * PRECISION2);
    for (j=0; j<xg->ncols_used; j++)
    {
      xg->cuwarm[1][j] = cty * xg->cu0[1][j] + sty * xg->cu1[1][j];
      xg->cuwarm[1][j] *= tmpf2;
      xg->cu[1][j] = xg->cuwarm[1][j] + xg->cufrozen[1][j];
    }
  }
  else
  {
    for (j=0; j<xg->ncols_used; j++)
      xg->cu[1][j] = cty * xg->cu0[1][j] + sty * xg->cu1[1][j];
  }

  if (xg->is_corr_sphered)
  {
    for (m=0; m<xg->nrows_in_plot; m++)
    {
      i = xg->rows_in_plot[m];
      xg->planar[i].x = ictx * cxi0[i][0] + istx * cxi1[i][0];
      xg->planar[i].y = icty * cxi0[i][1] + isty * cxi1[i][1];
    }
    get_planar_range(xg, &x_min, &x_max, &y_min, &y_max);
    for (m=0; m<xg->nrows_in_plot; m++)
    {
      i = xg->rows_in_plot[m];
      xg->planar[i].x = 2*(long)((2*(double)(xg->planar[i].x-x_min)/
        (x_max - x_min) - 1)*PRECISION2);
      xg->planar[i].y = 2*(long)((2*(double)(xg->planar[i].y-y_min)/
        (y_max - y_min) - 1)*PRECISION2);
    }
  }
  else
  {
    for (m=0; m<xg->nrows_in_plot; m++)
    {
      i = xg->rows_in_plot[m];
      xg->planar[i].x = ictx * cxi0[i][0] + istx * cxi1[i][0];
      xg->planar[i].y = icty * cxi0[i][1] + isty * cxi1[i][1];
      xg->planar[i].x /= PRECISION2;
      xg->planar[i].y /= PRECISION2;
      if (xg->ncxfrozen_vars > 0)
      {
        xg->planar[i].x *= tmpi1;
        xg->planar[i].x /= PRECISION2;
        xg->planar[i].x += cxif[i][0];
      }
      if (xg->ncyfrozen_vars > 0)
      {
        xg->planar[i].y *= tmpi2;
        xg->planar[i].y /= PRECISION2;
        xg->planar[i].y += cxif[i][1];
      }
    }
  }
}


void
calc_corr_angs(xgobidata *xg)
{
  float tmpf;
  int j;
  float tol1 = 0.001, tol2 = 0.1;

  tmpf = inner_prod(xg->cu0[0], xg->cu1[0], xg->ncols_used);
  if (tmpf > 1.0)
    tmpf = 1.0;
  else if (tmpf < -1.0)
    tmpf = -1.0;
  tau_x = (float) acos((double) tmpf);
  if (tau_x > M_PI_2) /* check for smallest angle */
  {
    tau_x = (M_PI - tau_x);
    for (j=0; j<xg->ncols_used; j++)
      xg->cu1[0][j] *= (-1);
  }
  if (tau_x < tol1) /* check for round-off error */
    tau_x = 0.;
  tmpf = inner_prod(xg->cu0[1], xg->cu1[1], xg->ncols_used);
  if (tmpf > 1.0)
    tmpf = 1.0;
  else if (tmpf < -1.0)
    tmpf = -1.0;
  tau_y = (float) acos((double) tmpf);
  if (tau_y > M_PI_2) /* check for smallest angle */
  {
    tau_y = (M_PI - tau_y);
    for (j=0; j<xg->ncols_used; j++)
      xg->cu1[1][j] *= (-1);
  }
  if (tau_y < tol1) /* check for round-off error */
    tau_y = 0.;

/* if no new target plane keep cycling until get one */
  if (tau_x < tol2 && tau_y < tol2)
  {
    zero_corr_taus();
    zero_corr_tincs();
    cwait_for_more_vars = True;

  }

  if (!cwait_for_more_vars)
  {
    corr_dv = sqrt(tau_x*tau_x + tau_y*tau_y);
    corr_delta = corr_step/corr_dv;
  
    if (tau_x > tol1)
      gram_schmidt(xg->cu0[0], xg->cu1[0], xg->ncols_used);
    if (tau_y > tol1)
      gram_schmidt(xg->cu0[1], xg->cu1[1], xg->ncols_used);
  }
}

void
corr_path(xgobidata *xg)
{
  calc_corr_angs(xg);
}

void
reset_corr_pause_cmd(xgobidata *xg)
{
  XtCallCallbacks(PAUSE, XtNcallback, (XtPointer) xg);
  XtVaSetValues(PAUSE, XtNstate, xg->is_corr_paused, NULL);
  setToggleBitmap(PAUSE, xg->is_corr_paused);
}

void
reset_corr_sphere_cmd(xgobidata *xg, Boolean state)
{
  XtVaSetValues(SPHERE, XtNstate, (Boolean) state, NULL);
  setToggleBitmap(SPHERE, state);
}

void
reset_corr_pursuit_cmd(xgobidata *xg, Boolean state)
{
  XtVaSetValues(CPURSUIT, XtNstate, (Boolean) state, NULL);
  setToggleBitmap(CPURSUIT, state);
}

void
set_sens_corr_reinit_cmd(xgobidata *xg, Boolean sens)
{
  XtVaSetValues(REINIT, XtNsensitive, (Boolean) sens, NULL);
}

void
set_sens_corr_sync_axes_cmd(xgobidata *xg, Boolean sens)
{
  XtVaSetValues(SYNC_AXES, XtNsensitive, (Boolean) sens, NULL);
}

void
set_sens_corr_backtrack_cmd(xgobidata *xg, Boolean sens)
{
  XtVaSetValues(CBACKTRACK, XtNsensitive, (Boolean) sens, NULL);
}

void
set_state_corr_backtrack_cmd(xgobidata *xg, Boolean state)
{
  XtVaSetValues(CBACKTRACK, XtNstate, (Boolean) state, NULL);
  setToggleBitmap(CBACKTRACK, state);
}

void
reset_corr_sync_axes_cmd(xgobidata *xg, Boolean call, Boolean state)
{
  if (call)
    XtCallCallbacks(SYNC_AXES, XtNcallback, (XtPointer) xg);
  XtVaSetValues(SYNC_AXES, XtNstate, (Boolean) state, NULL);
  setToggleBitmap(SYNC_AXES, state);
}

void
reset_corr_cycleback_cmd(xgobidata *xg, int set, int sens, char *label)
{
  if (set)
    XtVaSetValues(CDIRECTION,
      XtNsensitive, (Boolean) sens,
      XtNlabel, (String) label,
      NULL);
  else
    XtVaSetValues(CDIRECTION,
      XtNlabel, (String) label,
      NULL);
}

void
determine_new_corr_endbasis_and_path(xgobidata *xg)
{
  if (xg->is_corr_bt)
  {
    /*
     * Backtracking control.
    */
    if (cbt_dir == FORWARD)
    {
      init_corr_basis(xg);
      if (ccurr->next == NULL)
      {
        /*
         * Reverse direction and sleep for a second
         * when reaching the end of the history list.
        */
        cbt_dir = BACKWARD;
        ccurr = ccurr->prev;
        reset_corr_cycleback_cmd(xg, 0, 0, "B");
        sleep(1);
      }
      else
        ccurr = ccurr->next;

      retrieve_corr_basis(xg);
      cnhist_list += cbt_dir;
      nback_update_clabel();
    }
    else
    {
      init_corr_basis(xg);
      if (ccurr->prev == NULL)
      {
        /*
         * Reverse direction and sleep for a second
         * when reaching the end of the history list.
        */
        cbt_dir = FORWARD;
        ccurr = ccurr->next;
        reset_corr_cycleback_cmd(xg, 0, 0, "F");
        sleep(1);
      }
      else
        ccurr = ccurr->prev;

      retrieve_corr_basis(xg);
      cnhist_list += cbt_dir;
      nback_update_clabel();
    }
  }
  else
  {
    init_corr_basis(xg);
    if (xg->is_corr_optimz)
      cp_dir(xg);
    else
      new_corr_basis(xg);
    store_corr_basis(xg);
    nback_update_clabel();
  }
  zero_corr_tincs();
  corr_path(xg);
  if (!cwait_for_more_vars)
    corr_span_planes(xg);
}

void
corr_proc(xgobidata *xg)
{
  if (more_interp_togo(xg))
  {
    increment_corr(xg);
  }
  else
  {
    if (ready_to_stop_now)
    {
      ready_to_stop_now = False;
    }
    else
    {
      do_last_corr_increment(xg);
      determine_new_corr_endbasis_and_path(xg);
      cwait_for_more_vars = False;
    }
  }
}

void
set_corr_speed(float slidepos, xgobidata *xg)
{
/*
 * If the slider is near the start of its range, set step = 0.
 * and stop the touring.
*/
  if (slidepos < .05)
  {
    stop_corr_proc(xg);
    corr_step = 0.0;
  }
  else
  {
    start_corr_proc(xg);

    if (slidepos < 0.8)
      corr_step = (slidepos - .05) / 20. ;
    else if ((slidepos >= 0.8) && (slidepos < 0.9))
      corr_step = pow((double)(slidepos-0.8),(double)0.90) + 0.0375;
    else 
      corr_step = sqrt((double)(slidepos-0.8)) + 0.0375;
  }
  corr_delta = corr_step/corr_dv;
}

/* ARGSUSED */
XtCallbackProc
corr_speed_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  float slidepos = * (float *) slideposp;

  set_corr_speed(slidepos, xg);
}

/* ARGSUSED */
XtCallbackProc
corr_pause_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (xg->is_corr_paused)
  {
      xg->is_corr_paused = False;
      start_corr_proc(xg);
  }
  else
  {
    xg->is_corr_paused = True;
    stop_corr_proc(xg);
  }
  setToggleBitmap(w, xg->is_corr_paused);
}

/* ARGSUSED */
XtCallbackProc
corr_reinit_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i, j;

/* set u0 to be the first two variables, and zero taus */
  for (i=0; i<2; i++)
    for (j=0; j<xg->ncols; j++)
      xg->cu0[i][j] = 0.;
  xg->cu0[1][xg->corr_yvars[0]] = 1.0;
  xg->cu0[0][xg->corr_xvars[0]] = 1.0;
  for (i=0; i<2; i++)
    for (j=0; j<xg->ncols; j++)
      xg->cu[i][j] = xg->cu0[i][j];
  for (i=0; i<2; i++)
    for (j=0; j<xg->ncols; j++)
      xg->cu1[i][j] = xg->cu0[i][j];

  zero_corr_tincs();
  zero_corr_taus();
  if (xg->corr_cont_fact == infinite)
    xg->new_corr_dir_flag = True;

  reinit_corr_hist();
  cnhist_list = 0;
  store_corr_basis(xg);
  nback_update_clabel();
  update_world(xg);
  world_to_plane(xg);
  plane_to_screen(xg);
  reset_cp_plot();
  plot_once(xg);
  corr_var_lines(xg);
}

/* ARGSUSED */
XtCallbackProc
set_corr_manip_type_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k;
  int cmanip_type_menu_id;

  for (k=0; k<4; k++)
  {
    if (w == cmanip_type_menu_btn[k])
    {
      cmanip_type_menu_id = k;
      break;
    }
  }
  
  XtVaSetValues(cmanip_type_menu_cmd,
    XtNlabel, cmanip_type_menu_name[cmanip_type_menu_id],
    NULL);

  if (cmanip_type_menu_id == 0)
    xg->corr_manip_type = vertical;
  else if (cmanip_type_menu_id == 1)
    xg->corr_manip_type = horizontal;
  else if (cmanip_type_menu_id == 2)
    xg->corr_manip_type = combined;
  else if (cmanip_type_menu_id == 3)
    xg->corr_manip_type = eqcomb;
}

void
draw_cmanip_var(xgobidata *xg)
{
  XDrawArc(display, XtWindow(xg->vardraww[xg->corr_xmanip_var]),
    varpanel_copy_GC,
    (int) 5, (int) 5,
    (unsigned int) (2*xg->radius - 10), (unsigned int) (2*xg->radius-10),
    150*64, 60*64);
  XDrawArc(display, XtWindow(xg->vardraww[xg->corr_xmanip_var]),
    varpanel_copy_GC,
    (int) 5, (int) 5,
    (unsigned int) (2*xg->radius - 10), (unsigned int) (2*xg->radius-10),
    330*64, 60*64);
  XDrawArc(display, XtWindow(xg->vardraww[xg->corr_ymanip_var]),
    varpanel_copy_GC,
    (int) 5, (int) 5,
    (unsigned int) (2*xg->radius - 10), (unsigned int) (2*xg->radius-10),
    60*64, 60*64);
  XDrawArc(display, XtWindow(xg->vardraww[xg->corr_ymanip_var]),
    varpanel_copy_GC,
    (int) 5, (int) 5,
    (unsigned int) (2*xg->radius - 10), (unsigned int) (2*xg->radius-10),
    240*64, 60*64);
}

void
draw_cfrozen_var(xgobidata *xg)
{
  int k;

  XGCValues *gcv, gcv_inst;

  XGetGCValues(display, varpanel_xor_GC, GCCapStyle|GCJoinStyle, &gcv_inst);
  gcv = &gcv_inst;

  XSetLineAttributes(display, varpanel_copy_GC, 0, LineOnOffDash,
    gcv->cap_style, gcv->join_style);
  for (k=0; k<xg->ncxfrozen_vars; k++)
  {
     XDrawArc(display, XtWindow(xg->vardraww[xg->corr_xfrozen_vars[k]]),
      varpanel_copy_GC,
      (int) 10, (int) 10,
      (unsigned int) (2*xg->radius - 20), 
      (unsigned int) (2*xg->radius - 20),
      150*64, 60*64);
    XDrawArc(display, XtWindow(xg->vardraww[xg->corr_xfrozen_vars[k]]),
      varpanel_copy_GC,
      (int) 10, (int) 10,
      (unsigned int) (2*xg->radius - 20), 
      (unsigned int) (2*xg->radius - 20),
      330*64, 60*64);
  }
  for (k=0; k<xg->ncyfrozen_vars; k++)
  {
    XDrawArc(display, XtWindow(xg->vardraww[xg->corr_yfrozen_vars[k]]),
      varpanel_copy_GC,
      (int) 10, (int) 10,
      (unsigned int) (2*xg->radius - 20), (unsigned int) (2*xg->radius-20),
      60*64, 60*64);
    XDrawArc(display, XtWindow(xg->vardraww[xg->corr_yfrozen_vars[k]]),
      varpanel_copy_GC,
      (int) 10, (int) 10,
      (unsigned int) (2*xg->radius - 20), (unsigned int) (2*xg->radius-20),
      240*64, 60*64);
  }
  XSetLineAttributes(display, varpanel_copy_GC, 0, LineSolid,
    gcv->cap_style, gcv->join_style);
}

void
set_cmanip_var(xgobidata *xg, unsigned int btn, int varno)
{
  if (btn == 1)
    xg->corr_xmanip_var = varno;
  else if (btn == 2)
    xg->corr_ymanip_var = varno;
  refresh_vboxes(xg);
}

void
add_xvariable(xgobidata *xg, int varno)
{
  int found = 0;
  int i, j;

  xg->varchosen[varno] = True;
  found = 0;
  if (varno < xg->corr_xvars[0])
  {
    for (j=xg->ncorr_xvars; j>0; j--) 
      xg->corr_xvars[j] = xg->corr_xvars[j-1];
    xg->corr_xvars[0] = varno;
    xg->ncorr_xvars += 1;
    found = 1;
  }
  i = 0;
  while (!found && i < xg->ncorr_xvars-1 )
  {
    if ( varno > xg->corr_xvars[i]  &&
      varno < xg->corr_xvars[i+1] )
    {
      for (j=xg->ncorr_xvars; j>i; j--) 
      xg->corr_xvars[j] = xg->corr_xvars[j-1];
      xg->corr_xvars[i+1] = varno;
      xg->ncorr_xvars += 1;
      found = 1;
    }
    i++;
  }
  if (!found && (varno > xg->corr_xvars[xg->ncorr_xvars-1]))
  {
    xg->corr_xvars[xg->ncorr_xvars] = varno;
    xg->ncorr_xvars += 1;
    found = 1;
  }
}

void
add_yvariable(xgobidata *xg, int varno)
{
  int found = 0;
  int i, j;

  xg->varchosen[varno] = True;
  found = 0;
  if (varno < xg->corr_yvars[0])
  {
    for (j=xg->ncorr_yvars; j>0; j--) 
      xg->corr_yvars[j] = xg->corr_yvars[j-1];
    xg->corr_yvars[0] = varno;
    xg->ncorr_yvars += 1;
    found = 1;
  }
  i = 0;
  while (!found && i < xg->ncorr_yvars-1 )
  {
    if ( varno > xg->corr_yvars[i]  &&
      varno < xg->corr_yvars[i+1] )
    {
      for (j=xg->ncorr_yvars; j>i; j--) 
      xg->corr_yvars[j] = xg->corr_yvars[j-1];
      xg->corr_yvars[i+1] = varno;
      xg->ncorr_yvars += 1;
      found = 1;
    }
    i++;
  }
  if (!found && (varno > xg->corr_yvars[xg->ncorr_yvars-1]))
  {
    xg->corr_yvars[xg->ncorr_yvars] = varno;
    xg->ncorr_yvars += 1;
    found = 1;
  }
}

void
remove_xvariable(xgobidata *xg, int cvarno, int varno)
{
  char message[MSGLENGTH];
  int j;

  if (xg->ncorr_xvars > 1)
  {
    xg->varchosen[varno] = False;
    xg->ncorr_xvars--;
    for (j=cvarno; j<xg->ncorr_xvars; j++) 
      xg->corr_xvars[j] = xg->corr_xvars[j+1];
    refresh_vbox(xg, varno, 1);
    zero_corr_tincs();
    zero_corr_taus();
    if (xg->corr_cont_fact == infinite)
      xg->new_corr_dir_flag = True;
  }
  else
  {
    sprintf(message,
     "Too few X variables to remove it.\n");
    show_message(message, xg);
  }
}

void
remove_yvariable(xgobidata *xg, int cvarno, int varno)
{
  char message[MSGLENGTH];
  int j;

  if (xg->ncorr_yvars > 1)
  {
    xg->varchosen[varno] = False;
    xg->ncorr_yvars--;
    for (j=cvarno; j<xg->ncorr_yvars; j++) 
      xg->corr_yvars[j] = xg->corr_yvars[j+1];
    refresh_vbox(xg, varno, 1);
    zero_corr_tincs();
    zero_corr_taus();
    if (xg->corr_cont_fact == infinite)
      xg->new_corr_dir_flag = True;
  }
  else
  {
    sprintf(message,
      "Too few Y variables to remove it.\n");
       show_message(message, xg);
  }
}

void
set_cxfrozen_var(xgobidata *xg, int varno)
{
  int j, k, chosen_id;
  Boolean chosen, actv;
  int cvarno;

  chosen = False;
  for (k=0; k<xg->ncxfrozen_vars; k++)
  {
    if (xg->corr_xfrozen_vars[k] == varno)
    {
      chosen = True;
      chosen_id = k;
    }
  }

  if (chosen)
  {
    for (k=chosen_id+1; k<xg->ncxfrozen_vars; k++)
      xg->corr_xfrozen_vars[k-1] = xg->corr_xfrozen_vars[k];
    xg->ncxfrozen_vars--;
    xg->cuwarm[0][varno] = xg->cufrozen[0][varno];
    xg->cufrozen[0][varno] = 0.;
    zero_corr_tincs();
    zero_corr_taus();
    if (xg->corr_cont_fact == infinite)
      xg->new_corr_dir_flag = True;
    add_xvariable(xg, varno);
  }
  else
  {
    actv = False;
    for (j=0; j<xg->ncorr_xvars; j++)
    {
      if (xg->corr_xvars[j] == varno)
      {
        actv = True;
        cvarno = j;
        break;
      }
    }
    if (actv) 
    {
      xg->corr_xfrozen_vars[xg->ncxfrozen_vars] = varno;
      xg->cufrozen[0][varno] = xg->cu[0][varno];
      xg->cufrozen[1][varno] = xg->cu[1][varno];
      if (xg->ncxfrozen_vars == 0)
      {
        init_corr_basis(xg);
        xg->cu0[0][varno] = 0.;
        xg->cu0[1][varno] = 0.;
        copy_basis(xg->cu0, xg->cuwarm, xg->ncols_used);
      }
      else
      {
        xg->cuwarm[0][varno] = 0.;
        xg->cuwarm[1][varno] = 0.;
      }
      xg->ncxfrozen_vars++;
      remove_xvariable(xg, cvarno, varno);
    }
  }
  draw_cfrozen_var(xg);
  refresh_vboxes(xg);
}

void
set_cyfrozen_var(xgobidata *xg, int varno)
{
  int j, k, chosen_id;
  Boolean chosen, actv;
  int cvarno;

  chosen = False;
  for (k=0; k<xg->ncyfrozen_vars; k++)
  {
    if (xg->corr_yfrozen_vars[k] == varno)
    {
      chosen = True;
      chosen_id = k;
    }
  }

  if (chosen)
  {
    for (k=chosen_id+1; k<xg->ncyfrozen_vars; k++)
      xg->corr_yfrozen_vars[k-1] = xg->corr_yfrozen_vars[k];
    xg->ncyfrozen_vars--;
    xg->cuwarm[1][varno] = xg->cufrozen[1][varno];
    xg->cufrozen[1][varno] = 0.;
    zero_corr_tincs();
    zero_corr_taus();
    if (xg->corr_cont_fact == infinite)
      xg->new_corr_dir_flag = True;
    add_yvariable(xg, varno);
  }
  else
  {
    actv = False;
    for (j=0; j<xg->ncorr_yvars; j++)
    {
      if (xg->corr_yvars[j] == varno)
      {
        actv = True;
        cvarno = j;
        break;
      }
    }
    if (actv) 
    {
      xg->corr_yfrozen_vars[xg->ncyfrozen_vars] = varno;
      xg->cufrozen[1][varno] = xg->cu[1][varno];
      if (xg->ncyfrozen_vars == 0)
      {
        init_corr_basis(xg);
        xg->cu0[1][varno] = 0.;
        copy_basis(xg->cu0, xg->cuwarm, xg->ncols_used);
      }
      else
      {
        xg->cuwarm[1][varno] = 0.;
      }
      xg->ncyfrozen_vars++;
      remove_yvariable(xg, cvarno, varno);
    }
  }
  draw_cfrozen_var(xg);
  refresh_vboxes(xg);
}

Boolean
corr_backtracking(xgobidata *xg)
{
  return(xg->is_corr_bt);
}

/* ARGSUSED */
XtCallbackProc
corr_bt_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (!xg->is_corr_bt)
  {
    if (xg->corr_cont_fact != infinite && ((cnhist_list > 2) || 
      (!check_proximity(xg->cu, xg->cu0, xg->ncols_used) && 
      cnhist_list == 2)))
    {
      cbt_dir = BACKWARD;
      reset_corr_cycleback_cmd(xg, 1, 1, "B");
      /*
       * Store current position if not arbitrarily close to
       * last entry in history list.
      */
      if (!check_proximity(xg->cu, xg->cu0, xg->ncols_used))
      {
        init_corr_basis(xg);
        store_corr_basis(xg);
        nback_update_clabel();
      }
      /* Have just added 1 to nhist_list with store_basis, when there was 
       * already one more than actually in the list, so decrement nhist_list
       * by one, if there are less than the MAXHIST bases in the list. Also
       * allowing for the situation when there are MAXHIST-1 in the list but
       * heading towards MAXHIST.
      */
      if ((cnhist_list < MAXHIST) || 
        ((cnhist_list == MAXHIST) && 
        (old_cnhist_list == (MAXHIST-1)))) 
      {
        cnhist_list--;
      }
      max_cnhist_list = cnhist_list;
      /*
       * Break circular list and make end pointers NULL
      */
      chend = ccurr;
      ccurr->next = NULL;
      chfirst->prev = NULL;

      zero_corr_tincs();
      zero_corr_taus();
      xg->is_corr_bt = True;
      set_varsel_label(xg);

      /* de/re-sensitize widgets */
      set_sens_corr_sync_axes_cmd(xg, 0);
      set_sens_corr_reinit_cmd(xg, 0);
    }
    else
    {
      set_state_corr_backtrack_cmd(xg, 0);
    }
  }
  else
  {
    if (cbt_dir == FORWARD)
    {
      /*
       * If not at end of curr list save previous base,
       * break links and load remaining bases onto the free list.
      */
      if (ccurr->prev != NULL)
      {
        chend = ccurr->prev;
        cfl = ccurr;
        cfl->prev = NULL;
        while (cfl->next != NULL)
          cfl = cfl->next;
      }
    }
    else
    {
      /*
       * If not at end of curr list save current base,
       * break links and load remaining bases onto the free list.
      */
      if (ccurr->next != NULL)
      {
        chend = ccurr;
        nback_update_clabel();
        ccurr = ccurr->next;
        cfl = ccurr;
        cfl->prev = NULL;
        while (cfl->next != NULL)
          cfl = cfl->next;
      }
      cnhist_list++;
    }
    /* ensure fl ends have NULL pointers */
    if (cfl != NULL)
      cfl->next = NULL;
    ccurr = chend;
    /*
     * re-form circular links
    */
    ccurr->next = chfirst;
    chfirst->prev = ccurr;
    old_cnhist_list = cnhist_list - 1;
    max_cnhist_list = MAXHIST;
    zero_corr_tincs();
    zero_corr_taus();
    reset_corr_cycleback_cmd(xg, 1, 0, "F");
    xg->is_corr_bt = False;
    set_varsel_label(xg);

    /* de/re-sensitize widgets */
    set_sens_corr_sync_axes_cmd(xg, 1);
    set_sens_corr_reinit_cmd(xg, 1);
  }

  setToggleBitmap(w, xg->is_corr_bt);
}

/* ARGSUSED */
XtCallbackProc
corr_cycleback_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int do_it = 1;
  char message[MSGLENGTH];

  if (xg->is_corr_bt)
  {
    if (cbt_dir == BACKWARD)   /* backwards */
    {
      /* 
       * If at the end of the list, ignore user request.
      */
      if (!ccurr->next)
      {
        do_it = 0;
        sprintf(message, "Can\'t go forward; at end of list.\n");
        show_message(message, xg);
      }
      else
      {
        reset_corr_cycleback_cmd(xg, 0, 0, "F");
        cbt_dir = FORWARD;
        ccurr = ccurr->next;
      }
    }
    else if (cbt_dir == FORWARD)  /* forwards */
    {
      /* 
       * If at the end of the list, ignore user request.
      */
      if (!ccurr->prev)
      {
        do_it = 0;
        sprintf(message, "Can\'t go backward; at end of list.\n");
        show_message(message, xg);
      }
      else
      {
        reset_corr_cycleback_cmd(xg, 0, 0, "B");
        cbt_dir = BACKWARD;
        ccurr = ccurr->prev;
      }
    }
    if (do_it)
    {
      init_corr_basis(xg);
      cnhist_list += cbt_dir;
      retrieve_corr_basis(xg);
      zero_corr_tincs();
      zero_corr_taus();
    }
  }
}

/* ARGSUSED */
XtCallbackProc
corr_sync_axes_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int j;
  char message[MSGLENGTH];

  if (xg->is_corr_syncd_axes)
  {
    xg->is_corr_syncd_axes = False;
  }
  else
  {
    if (xg->ncorr_xvars == xg->ncorr_yvars)
    {
      xg->is_corr_syncd_axes = True;
      init_corr_basis(xg);
      zero_corr_tincs();
      zero_corr_taus();
      for (j=0; j<xg->ncorr_yvars; j++)
        xg->cu0[1][xg->corr_yvars[j]] = xg->cu0[0][xg->corr_xvars[j]];
      update_world(xg);
      world_to_plane(xg);
      plane_to_screen(xg);
      plot_once(xg);
      corr_var_lines(xg);
      if (xg->corr_cont_fact == infinite)
        reinit_corr(xg);
    }
    else
    {
        sprintf(message,
         "The number of variables in each group is not equal. \n");
        show_message(message, xg);
        reset_corr_sync_axes_cmd(xg, 0, 0);
     }
  }
  setToggleBitmap(w, xg->is_corr_syncd_axes);
}

/* ARGSUSED */
XtCallbackProc
corr_sphere_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (!xg->is_corr_pursuit)
  {
    if (xg->is_corr_sphered)
      if (xg->ncorr_xvars > 1 || xg->ncorr_yvars > 1)
	xg->is_corr_sphered = False;
      else
	reset_corr_sphere_cmd(xg,1);
    else
      xg->is_corr_sphered = True;
  }
  else
  {
    xg->is_corr_sphered = False;
  }

  setToggleBitmap(w, xg->is_corr_sphered);
}

/*
void
reset_corr_manip_type_menu(xg)
  xgobidata *xg;
{
  char *label;

  if (xg->corr_manip_type == vertical)
    label = cmanip_type_menu_name[0];
  else if (xg->corr_manip_type == horizontal)
    label = cmanip_type_menu_name[1];
  else if (xg->corr_manip_type == combined)
    label = cmanip_type_menu_name[2];
  else if (xg->corr_manip_type == eqcomb)
    label = cmanip_type_menu_name[3];

  XtVaSetValues(cmanip_type_menu_cmd, XtNlabel, label, NULL);
}
*/

void
make_corr_manip_type_menu(xgobidata *xg, Widget parent, Widget vref)
{
  Dimension width = 0, maxwidth = 0;
  int longest = 0;
  int k;

  cmanip_type_menu_label = XtVaCreateManagedWidget("Label",
    labelWidgetClass, parent,
    XtNfromVert, vref,
    XtNlabel, "Manip:",
    NULL);
  if (mono) set_mono(cmanip_type_menu_label);

  for (k=0; k<4; k++)
  {
    width =
      XTextWidth(appdata.font, cmanip_type_menu_name[k], 
        strlen(cmanip_type_menu_name[k]) + 2*ASCII_TEXT_BORDER_WIDTH);
    if (width > maxwidth)
    {
      maxwidth = width;
      longest = k;
    }
  }

  /*
   * Initialize this widget to use the longest name in order
   * to ensure that it is wide enough.
  */
  cmanip_type_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) cmanip_type_menu_name[longest],
    XtNmenuName, (String) "Menu",
    XtNfromVert, vref,
    XtNfromHoriz, cmanip_type_menu_label,
    XtNresize, False,
    NULL);
  if (mono) set_mono(cmanip_type_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    cmanip_type_menu_cmd, "Corr_Manip");

  cmanip_type_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, cmanip_type_menu_cmd,
    NULL);
  if (mono) set_mono(cmanip_type_menu_cmd);

  VERTICAL_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cmanip_type_menu,
    XtNlabel, VERTICAL_NAME,
    NULL);
  if (mono) set_mono(VERTICAL_BTN);
  XtAddCallback(VERTICAL_BTN, XtNcallback,
    (XtCallbackProc) set_corr_manip_type_cback, (XtPointer) xg);

  HORIZONTAL_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cmanip_type_menu,
    XtNlabel, HORIZONTAL_NAME,
    NULL);
  if (mono) set_mono(HORIZONTAL_BTN);
  XtAddCallback(HORIZONTAL_BTN, XtNcallback,
    (XtCallbackProc) set_corr_manip_type_cback, (XtPointer) xg);

  COMB_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cmanip_type_menu,
    XtNlabel, COMB_NAME,
    NULL);
  if (mono) set_mono(COMB_BTN);
  XtAddCallback(COMB_BTN, XtNcallback,
    (XtCallbackProc) set_corr_manip_type_cback, (XtPointer) xg);

  EQ_COMB_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cmanip_type_menu,
    XtNlabel, EQ_COMB_NAME,
    NULL);
  if (mono) set_mono(EQ_COMB_BTN);
  XtAddCallback(EQ_COMB_BTN, XtNcallback,
    (XtCallbackProc) set_corr_manip_type_cback, (XtPointer) xg);

  XtVaSetValues(cmanip_type_menu_cmd,
    XtNlabel, cmanip_type_menu_name[2],
    NULL);

}

/*
void
reset_corr_cont_fact_menu(xg)
  xgobidata *xg;
{
  char *label;

  if (xg->corr_cont_fact == tenth)
    label = ccont_fact_menu_name[0];
  else if (xg->corr_cont_fact == fifth)
    label = ccont_fact_menu_name[1];
  else if (xg->corr_cont_fact == quarter)
    label = ccont_fact_menu_name[2];
  else if (xg->corr_cont_fact == third)
    label = ccont_fact_menu_name[3];
  else if (xg->corr_cont_fact == half)
    label = ccont_fact_menu_name[4];
  else if (xg->corr_cont_fact == one)
    label = ccont_fact_menu_name[5];
  else if (xg->corr_cont_fact == two)
    label = ccont_fact_menu_name[6];
  else if (xg->corr_cont_fact == ten)
    label = ccont_fact_menu_name[7];
  else if (xg->corr_cont_fact == infinite)
    label = ccont_fact_menu_name[8];

  XtVaSetValues(ccont_fact_menu_cmd, XtNlabel, label, NULL);
}
*/

/* ARGSUSED */
XtCallbackProc
set_corr_cont_fact_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k;
  int ccont_fact_menu_id;

  for (k=0; k<9; k++)
  {
    if (w == ccont_fact_menu_btn[k])
    {
      ccont_fact_menu_id = k;
      break;
    }
  }

  XtVaSetValues(ccont_fact_menu_cmd,
    XtNlabel, ccont_fact_menu_name[ccont_fact_menu_id],
    NULL);

  if (ccont_fact_menu_id == 0)
  {
    xg->corr_cont_fact = tenth;
    xg->fccont_fact = 0.1;
    if (cnhist_list > 2)
      set_sens_corr_backtrack_cmd(xg, 1);
  }
  else if (ccont_fact_menu_id == 1)
  {
    xg->corr_cont_fact = fifth;
    xg->fccont_fact = 0.2;
    if (cnhist_list > 2)
      set_sens_corr_backtrack_cmd(xg, 1);
  }
  else if (ccont_fact_menu_id == 2)
  {
    xg->corr_cont_fact = quarter;
    xg->fccont_fact = 0.25;
    if (cnhist_list > 2)
      set_sens_corr_backtrack_cmd(xg, 1);
  }
  else if (ccont_fact_menu_id == 3)
  {
    xg->corr_cont_fact = third;
    xg->fccont_fact = 0.3333;
    if (cnhist_list > 2)
      set_sens_corr_backtrack_cmd(xg, 1);
  }
  else if (ccont_fact_menu_id == 4)
  {
    xg->corr_cont_fact = half;
    xg->fccont_fact = 0.5;
    if (cnhist_list > 2)
      set_sens_corr_backtrack_cmd(xg, 1);
  }
  else if (ccont_fact_menu_id == 5)
  {
    xg->corr_cont_fact = one;
    xg->fccont_fact = 1;
    if (cnhist_list > 2)
      set_sens_corr_backtrack_cmd(xg, 1);
  }
  else if (ccont_fact_menu_id == 6)
  {
    xg->corr_cont_fact = two;
    xg->fccont_fact = 2;
    if (cnhist_list > 2)
      set_sens_corr_backtrack_cmd(xg, 1);
  }
  else if (ccont_fact_menu_id == 7)
  {
    xg->corr_cont_fact = ten;
    xg->fccont_fact = 10;
    if (cnhist_list > 2)
      set_sens_corr_backtrack_cmd(xg, 1);
  }
  else if (ccont_fact_menu_id == 8)
  {
    xg->corr_cont_fact = infinite;
    set_sens_corr_backtrack_cmd(xg, 0);
  }

  if (xg->corr_cont_fact != infinite)
  {
    init_corr_basis(xg);
    store_corr_basis(xg);
    zero_corr_taus();
    zero_corr_tincs();
  }
}

void
make_corr_cont_fact_menu(xgobidata *xg, Widget parent, Widget vref)
{
  Dimension width = 0, maxwidth = 0;
  int k;

  ccont_fact_menu_label = XtVaCreateManagedWidget("Label",
    labelWidgetClass, parent,
    XtNfromVert, vref,
    XtNlabel, "Path Len:",
    NULL);
  if (mono) set_mono(ccont_fact_menu_label);

  for (k=0; k<9; k++)
  {
    width =
      XTextWidth(appdata.font, ccont_fact_menu_name[k], 
        strlen(ccont_fact_menu_name[k]) + 2*ASCII_TEXT_BORDER_WIDTH);
    if (width > maxwidth)
    {
      maxwidth = width;
    }
  }

  /*
   * Initialize this widget to use the longest name in order
   * to ensure that it is wide enough.
  */
  ccont_fact_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Infinite",
    XtNmenuName, (String) "Menu",
    XtNfromVert, vref,
    XtNfromHoriz, ccont_fact_menu_label,
    XtNresize, False,
    NULL);
  if (mono) set_mono(ccont_fact_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    ccont_fact_menu_cmd, "Tour_PathLen");

  ccont_fact_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, ccont_fact_menu_cmd,
    NULL);
  if (mono) set_mono(ccont_fact_menu_cmd);

  TENTH_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, ccont_fact_menu,
    XtNlabel, TENTH_NAME,
    NULL);
  if (mono) set_mono(TENTH_BTN);
  XtAddCallback(TENTH_BTN, XtNcallback,
    (XtCallbackProc) set_corr_cont_fact_cback, (XtPointer) xg);

  FIFTH_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, ccont_fact_menu,
    XtNlabel, FIFTH_NAME,
    NULL);
  if (mono) set_mono(FIFTH_BTN);
  XtAddCallback(FIFTH_BTN, XtNcallback,
    (XtCallbackProc) set_corr_cont_fact_cback, (XtPointer) xg);

  QUARTER_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, ccont_fact_menu,
    XtNlabel, QUARTER_NAME,
    NULL);
  if (mono) set_mono(QUARTER_BTN);
  XtAddCallback(QUARTER_BTN, XtNcallback,
    (XtCallbackProc) set_corr_cont_fact_cback, (XtPointer) xg);

  THIRD_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, ccont_fact_menu,
    XtNlabel, THIRD_NAME,
    NULL);
  if (mono) set_mono(THIRD_BTN);
  XtAddCallback(THIRD_BTN, XtNcallback,
    (XtCallbackProc) set_corr_cont_fact_cback, (XtPointer) xg);

  HALF_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, ccont_fact_menu,
    XtNlabel, HALF_NAME,
    NULL);
  if (mono) set_mono(HALF_BTN);
  XtAddCallback(HALF_BTN, XtNcallback,
    (XtCallbackProc) set_corr_cont_fact_cback, (XtPointer) xg);

  ONE_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, ccont_fact_menu,
    XtNlabel, ONE_NAME,
    NULL);
  if (mono) set_mono(ONE_BTN);
  XtAddCallback(ONE_BTN, XtNcallback,
    (XtCallbackProc) set_corr_cont_fact_cback, (XtPointer) xg);

  TWO_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, ccont_fact_menu,
    XtNlabel, TWO_NAME,
    NULL);
  if (mono) set_mono(TWO_BTN);
  XtAddCallback(TWO_BTN, XtNcallback,
    (XtCallbackProc) set_corr_cont_fact_cback, (XtPointer) xg);

  TEN_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, ccont_fact_menu,
    XtNlabel, TEN_NAME,
    NULL);
  if (mono) set_mono(TEN_BTN);
  XtAddCallback(TEN_BTN, XtNcallback,
    (XtCallbackProc) set_corr_cont_fact_cback, (XtPointer) xg);

  INFINITE_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, ccont_fact_menu,
    XtNlabel, INFINITE_NAME,
    NULL);
  if (mono) set_mono(INFINITE_BTN);
  XtAddCallback(INFINITE_BTN, XtNcallback,
    (XtCallbackProc) set_corr_cont_fact_cback, (XtPointer) xg);

  XtVaSetValues(ccont_fact_menu_cmd, XtNlabel, "1", NULL);
}

void
make_corr(xgobidata *xg)
{
  char str[30];
  Dimension max_width;

/*
 * Widest button label used in this panel.
*/
  sprintf(str, "What do you no?");
  max_width = XTextWidth(appdata.font, str, strlen(str)) +
    4 * ASCII_TEXT_BORDER_WIDTH + 3 + 2;
  /* borders around text, spacing between widgets, widget borders */

  corr_panel = XtVaCreateManagedWidget("CorrPanel",
    formWidgetClass, xg->box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
    NULL);
  if (mono) set_mono(corr_panel);

/*
 * corr speed scrollbar
*/
  corr_speed_sbar = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, corr_panel,
    XtNwidth, (Dimension) max_width,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(corr_speed_sbar);

  add_sbar_help(&xg->nhelpids.sbar,
    corr_speed_sbar, "Corr_Speed");

  XawScrollbarSetThumb(corr_speed_sbar, 20.*TOURSTEP0 + .05, -1.);
  XtAddCallback(corr_speed_sbar, XtNjumpProc,
    (XtCallbackProc) corr_speed_cback, (XtPointer) xg);

  PAUSE = CreateToggle(xg, "Pause",
    True, (Widget) NULL, (Widget) corr_speed_sbar, (Widget) NULL, 
    False, ANY_OF_MANY, corr_panel, "Corr_Pause");
  XtAddCallback(PAUSE, XtNcallback,
    (XtCallbackProc) corr_pause_cback, (XtPointer) xg);
  XtManageChild(PAUSE);

  REINIT = CreateCommand(xg, "Reinit",
    True, (Widget) PAUSE, (Widget) corr_speed_sbar, 
    (Widget) corr_panel, "Corr_Reinit");
  XtAddCallback(REINIT, XtNcallback,
    (XtCallbackProc) corr_reinit_cback, (XtPointer) xg);
  XtManageChild(REINIT);

/*
 * Menu to manage manipulation type 
*/ 
  make_corr_manip_type_menu(xg, corr_panel, PAUSE);

/*
 * Menu to manage continuation factor
*/ 
  make_corr_cont_fact_menu(xg, corr_panel, cmanip_type_menu_label);

/*
 * corr backtrack control: initiate insensitive, turn on
 * when there are more than two or three elements in the
 * history file
*/
  CBACKTRACK = CreateToggle(xg, "Backtrck",
    True, (Widget) NULL, ccont_fact_menu_label, (Widget) NULL, False,
    ANY_OF_MANY, corr_panel, "Corr_Backtrack");
  XtAddCallback(CBACKTRACK, XtNcallback,
    (XtCallbackProc) corr_bt_cback, (XtPointer) xg);
  XtManageChild(CBACKTRACK);

/*
 * label to record number of bases in history file
*/
  sprintf(str, "%d ", MAXHIST);
  max_width = XTextWidth(appdata.font, str, strlen(str));

  NBASES_HISTORY = XtVaCreateManagedWidget("CorrLabel",
    asciiTextWidgetClass, corr_panel,
    XtNfromVert, (Widget) ccont_fact_menu_label,
    XtNfromHoriz, (Widget) CBACKTRACK,
    XtNstring, (String) "2",
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) max_width,
    NULL);
  if (mono) set_mono(NBASES_HISTORY);

/*
 * Button to allow cycling backwards and forwards through backtracking
*/
  CDIRECTION = CreateCommand(xg, "F",
    False, NBASES_HISTORY, ccont_fact_menu_label,
    corr_panel, "Corr_BtrackDir");
  XtAddCallback(CDIRECTION, XtNcallback,
    (XtCallbackProc) corr_cycleback_cback, (XtPointer) xg);
  XtManageChild(CDIRECTION);

  SYNC_AXES = CreateToggle(xg, "Sync Axes",
    True, (Widget) NULL, (Widget) CBACKTRACK, (Widget) NULL, 
    False, ANY_OF_MANY, corr_panel, "Corr_SyncAxes");
  XtAddCallback(SYNC_AXES, XtNcallback,
    (XtCallbackProc) corr_sync_axes_cback, (XtPointer) xg);
  XtManageChild(SYNC_AXES);

  SPHERE = CreateToggle(xg, "Sphere",
    True, (Widget) NULL, (Widget) SYNC_AXES, (Widget) NULL,
    False, ANY_OF_MANY, corr_panel, "Corr_Sphere");
  XtAddCallback(SPHERE, XtNcallback,
    (XtCallbackProc) corr_sphere_cback, (XtPointer) xg);
  XtManageChild(SPHERE);

  CPURSUIT = CreateToggle(xg, "Pursuit",
    True, (Widget) SPHERE, (Widget) SYNC_AXES, (Widget) NULL,
    False, ANY_OF_MANY, corr_panel, "Corr_Pursuit");
  XtAddCallback(CPURSUIT, XtNcallback,
    (XtCallbackProc) corr_pursuit_cback, (XtPointer) xg);
  XtManageChild(CPURSUIT);

  make_cp_plot(xg, CPURSUIT);
  make_cp_panel(xg, corr_panel);

}

/*
void
alter_target_basis(xg, varno)
  xgobidata *xg;
  int varno;
{
  int i;

  for (i=0; i<2; i++)
    xg->cu1[i][varno] = 0.0;
  norm(xg->cu1[0], xg->ncols_used);
  norm(xg->cu1[1], xg->ncols_used);
  gram_schmidt(xg->cu1[0], xg->cu1[1], xg->ncols_used);
}
*/

/*
void
set_fading_xvar(set_val, varno)
  Boolean set_val;
  int varno;
{
  xvar_to_fade = set_val;
  xvar_fading = varno;
}
*/

/*
void
set_fading_yvar(set_val, varno)
  Boolean set_val;
  int varno;
{
  yvar_to_fade = set_val;
  yvar_fading = varno;
}
*/

void
calc_var_xy(xgobidata *xg, int r, int varno, int *px, int *py)
{
  int x, y;

  x = *px;
  y = *py;

  x = INT(FLOAT(r) * xg->cuold[0][varno]);
  y = INT(FLOAT(r) * xg->cuold[1][varno]);

  *px = x;
  *py = y;
}

void
set_xcorrvar(xgobidata *xg, int varno)
{
  int cvarno;
  int i, j;
  int selected;
  int y_cvarno, x_cvarno;
  int found;
  char message[MSGLENGTH];
  Boolean y_selected, x_selected;

      selected = 0;
      for (j=0; j<xg->ncorr_xvars; j++)
        if (varno == xg->corr_xvars[j])
        {
          selected = 1;
          cvarno = j;
          break;
        }
      if (selected)
      {/* x selected */
        remove_xvariable(xg, cvarno, varno);
      }
      else
      {/* !x selected, check if y selected */
        y_selected = False;
        for (j=0; j<xg->ncorr_yvars; j++)
          if (varno == xg->corr_yvars[j])
          {
            y_selected = True;
            y_cvarno = j;
            break;
          }
        if (y_selected)/* zero y component out now */
        {
          if (xg->ncorr_yvars > 1)
          {
            xg->cu[1][varno] = 0.0;
            norm(xg->cu[1], xg->ncols_used);
            xg->ncorr_yvars--;
            for (j=y_cvarno; j<xg->ncorr_yvars; j++) 
              xg->corr_yvars[j] = xg->corr_yvars[j+1];
            if (xg->ncorr_yvars == 1)
              xg->cu[1][xg->corr_yvars[0]] = 1.;
            /* insert variable in the x variable chosen list */
            xg->varchosen[varno] = True;
            found = 0;
            if (varno < xg->corr_xvars[0])
            {
              for (j=xg->ncorr_xvars; j>0; j--) 
                xg->corr_xvars[j] = xg->corr_xvars[j-1];
              xg->corr_xvars[0] = varno;
              xg->ncorr_xvars += 1;
              found = 1;
            }
            i = 0;
            while (!found && i < xg->ncorr_xvars-1 )
            {
              if ( varno > xg->corr_xvars[i]  &&
                   varno < xg->corr_xvars[i+1] )
              {
                for (j=xg->ncorr_xvars; j>i; j--) 
                  xg->corr_xvars[j] = xg->corr_xvars[j-1];
                xg->corr_xvars[i+1] = varno;
                xg->ncorr_xvars += 1;
                found = 1;
              }
              i++;
            }
            if (!found && (varno > xg->corr_xvars[xg->ncorr_xvars-1]))
            {
              xg->corr_xvars[xg->ncorr_xvars] = varno;
              xg->ncorr_xvars += 1;
              found = 1;
            }
            refresh_vbox(xg, varno, 1);
            init_corr_basis(xg);
            zero_corr_tincs();
            zero_corr_taus();
            update_world(xg);
            world_to_plane(xg);
            plane_to_screen(xg);
            plot_once(xg);
            corr_var_lines(xg);
            reinit_corr_hist();
            cnhist_list = 0;
            nback_update_clabel();
            xg->cp_recalc_max_min = True;
            if (xg->corr_cont_fact == infinite)
              xg->new_corr_dir_flag = True;
          }
          else
          {
            sprintf(message,
              "Too few Y variables to change it to an X variable.\n");
            show_message(message, xg);
          }
        }
        else
        {
          add_xvariable(xg, varno);
          refresh_vbox(xg, varno, 1);
          zero_corr_tincs();
          zero_corr_taus();
          if (xg->corr_cont_fact == infinite)
            xg->new_corr_dir_flag = True;
        }
      }
}

void
set_ycorrvar(xgobidata *xg, int varno)
{
  int cvarno;
  int i, j;
  int selected;
  int y_cvarno, x_cvarno;
  int found;
  char message[MSGLENGTH];
  Boolean y_selected, x_selected;

      selected = 0;
      for (j=0; j<xg->ncorr_yvars; j++)
        if (varno == xg->corr_yvars[j])
        {
          selected = 1;
          cvarno = j;
          break;
        }
      if (selected)
      {/* y selected */
        remove_yvariable(xg, cvarno, varno);
      }
      else
      {/* !y selected, check if x selected */
        x_selected = False;
        for (j=0; j<xg->ncorr_xvars; j++)
          if (varno == xg->corr_xvars[j])
          {
            x_selected = True;
            x_cvarno = j;
            break;
          }
        if (x_selected)/* zero x component out now */
        {
          if (xg->ncorr_xvars > 1)
          {
            xg->cu[0][varno] = 0.0;
            norm(xg->cu[0], xg->ncols_used);
            xg->ncorr_xvars--;
            for (j=x_cvarno; j<xg->ncorr_xvars; j++) 
              xg->corr_xvars[j] = xg->corr_xvars[j+1];
            /* insert variable in the y variable chosen list */
            xg->varchosen[varno] = True;
            found = 0;
            if (varno < xg->corr_yvars[0])
            {
              for (j=xg->ncorr_yvars; j>0; j--) 
                xg->corr_yvars[j] = xg->corr_yvars[j-1];
              xg->corr_yvars[0] = varno;
              xg->ncorr_yvars++;
              found = 1;
            }
            i = 0;
            while (!found && i < xg->ncorr_yvars-1 )
            {
              if ( varno > xg->corr_yvars[i]  &&
                   varno < xg->corr_yvars[i+1] )
              {
                for (j=xg->ncorr_yvars; j>i; j--) 
                  xg->corr_yvars[j] = xg->corr_yvars[j-1];
                xg->corr_yvars[i+1] = varno;
                xg->ncorr_yvars++;
                found = 1;
              }
              i++;
            }
            if (!found && (varno > xg->corr_yvars[xg->ncorr_yvars-1]))
            {
              xg->corr_yvars[xg->ncorr_yvars] = varno;
              xg->ncorr_yvars++;
              found = 1;
            }
            refresh_vbox(xg, varno, 1);
            init_corr_basis(xg);
            zero_corr_tincs();
            zero_corr_taus();
            update_world(xg);
            world_to_plane(xg);
            plane_to_screen(xg);
            plot_once(xg);
            corr_var_lines(xg);
            reinit_corr_hist();
            cnhist_list = 0;
            nback_update_clabel();
            xg->cp_recalc_max_min = True;
            if (xg->corr_cont_fact == infinite)
              xg->new_corr_dir_flag = True;
          }
          else
          {
            sprintf(message,
              "Too few X variables to change it to an Y variable.\n");
            show_message(message, xg);
          }
        }
        else
        {
          xg->varchosen[varno] = True;
          found = 0;
          if (varno < xg->corr_yvars[0])
          {
            for (j=xg->ncorr_yvars; j>0; j--) 
              xg->corr_yvars[j] = xg->corr_yvars[j-1];
            xg->corr_yvars[0] = varno;
            xg->ncorr_yvars++;
            found = 1;
          }
          i = 0;
          while (!found && i < xg->ncorr_yvars-1 )
          {
            if ( varno > xg->corr_yvars[i]  &&
                 varno < xg->corr_yvars[i+1] )
            {
              for (j=xg->ncorr_yvars; j>i; j--) 
                xg->corr_yvars[j] = xg->corr_yvars[j-1];
              xg->corr_yvars[i+1] = varno;
              xg->ncorr_yvars++;
              found = 1;
            }
            i++;
          }
          if (!found && (varno > xg->corr_yvars[xg->ncorr_yvars-1]))
          {
            xg->corr_yvars[xg->ncorr_yvars] = varno;
            xg->ncorr_yvars++;
            found = 1;
          }
          refresh_vbox(xg, varno, 1);
          zero_corr_tincs();
          zero_corr_taus();
          if (xg->corr_cont_fact == infinite)
            xg->new_corr_dir_flag = True;
        }
      }
}

int
corr_varselect(int varno, int button, int state, xgobidata *xg)
/*
 * This is used only during correlation touring.
*/
{
  int newvar = 0;

  if (xg->is_corr_syncd_axes)
    reset_corr_sync_axes_cmd(xg, 1, 0);/* turn off syncing while doing 
                                          variable selection */ 

  if (state == 0 || state == 8) /* No shift modifier down, but alt ok. */
  {
    /*
     * If it's a left click ...
      */
    if (button == 1 && state != 8)  /* alt key not pressed */
    {
      set_xcorrvar(xg, varno);
    }
    /*
     * If it's a middle click ...
    */
    else if (button == 2 || (button == 1 && state == 8))
    {
      set_ycorrvar(xg, varno);
    }
  }
  else if (state == 1) /* shift modifier - set manip variable */
  {
    set_cmanip_var(xg, button, varno);
  }
  else if (state == 4) /* control modifier */
  {
    if (button == 1)
      set_cxfrozen_var(xg, varno);
    else if (button == 2)
      set_cyfrozen_var(xg, varno);
  }
  
  return(newvar);
}

void
draw_corr_axes(xgobidata *xg)
{
  int j, k;
  int naxes;
  int raw_axis_len = MIN(xg->mid.x, xg->mid.y);
  float tol = .0001;
  fcoords axs;
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
  for (j=0; j<xg->ncols_used; j++)
  {
    if (xg->cu[0][j]*xg->cu[0][j] > tol)
    {
      corr_axes[naxes].x1 = cntr.x;
      corr_axes[naxes].x2 = cntr.x +
        (int) (axs.x * xg->cu[0][j]);
      corr_axes[naxes].y1 = cntr.y;
      corr_axes[naxes].y2 = cntr.y;
      strcpy(corr_lab[(int)(naxes/2)], xg->collab_tform2[j]);
      naxes++;
      corr_axes[naxes].x1 = corr_axes[naxes].x2 = cntr.x +
        (int) (axs.x * xg->cu[0][j]);
      corr_axes[naxes].y1 = cntr.y + 2;
      corr_axes[naxes].y2 = cntr.y - 2;
      naxes++;
    }
    if (xg->cu[1][j]*xg->cu[1][j] > tol)
    {
      corr_axes[naxes].x1 = cntr.x;
      corr_axes[naxes].x2 = cntr.x;
      corr_axes[naxes].y1 = cntr.y;
      corr_axes[naxes].y2 = cntr.y -
        (int) (axs.y * xg->cu[1][j]);
      strcpy(corr_lab[(int)(naxes/2)], xg->collab_tform2[j]);
      naxes++;
      corr_axes[naxes].x1 = cntr.x + 2;
      corr_axes[naxes].x2 = cntr.x - 2;
      corr_axes[naxes].y1 = corr_axes[naxes].y2 = cntr.y -
        (int) (axs.y * xg->cu[1][j]);
      naxes++;
    }
  }

/*
 * Draw axes of rotation.
*/
  XSetForeground(display, copy_GC, plotcolors.fg);
  XDrawSegments(display, xg->pixmap0, copy_GC, corr_axes, naxes);
/*
 * Add axis labels.
*/
  for (k=0; k<((int)naxes/2); k++)
  {
    XDrawString(display, xg->pixmap0, copy_GC,
       corr_axes[k*2].x2 + 6, corr_axes[k*2].y2 - 5,
       corr_lab[k], strlen(corr_lab[k]));
  }
}

void
corr_tour_on(xgobidata *xg)
{
  int j, k;

/*
 *  If this mode is currently selected, turn it off.
*/
  if (xg->prev_plot_mode == CTOUR_MODE && xg->plot_mode != CTOUR_MODE)
  {
    map_corr_panel(xg, False);
    corr_event_handlers(xg, 0);
    stop_corr_proc(xg);
  }
  /* Else turn it on */
  else if (xg->prev_plot_mode != CTOUR_MODE &&
           xg->plot_mode == CTOUR_MODE)
  {
    if (!xg->is_corr_touring)
    {
      if (xg->is_plotting1d)
        free_txtr_var();
      else if (xg->is_touring && xg->is_princ_comp)
	set_sph_labs(xg, xg->nsph_vars);
/*        reset_var_labels(xg, PRINCCOMP_OFF);i think not needed anymore
                                             sphering transformation*/

      if (xg->carry_vars)
        carry_corr_vars(xg);
      xg->is_plotting1d = xg->is_xyplotting = False;
      xg->is_spinning = False;
      xg->is_touring = False;
      xg->is_corr_touring = True;
  
      set_varsel_label(xg);
      find_plot_center(xg); 
      update_lims(xg);
      update_world(xg);
      world_to_plane(xg);
      plane_to_screen(xg);
      plot_once(xg);

      for (j=0; j<xg->ncols_used; j++)
        xg->varchosen[j] = False;
      for (j=0; j<xg->ncorr_xvars; j++)
      {
        k = xg->corr_xvars[j];
        xg->varchosen[k] = True;
      }
      for (j=0; j<xg->ncorr_yvars; j++)
      {
        k = xg->corr_yvars[j];
        xg->varchosen[k] = True;
      }
  
      for (j=0; j<xg->ncols_used; j++)
        refresh_vlab(j, xg);
      refresh_vboxes(xg);

    }
    corr_event_handlers(xg, 1);
    map_corr_panel(xg, True);
    start_corr_proc(xg);
  }
}

#undef PAUSE
#undef REINIT

#undef VERTICAL_BTN 
#undef HORIZONTAL_BTN
#undef COMB_BTN
#undef EQ_COMB_BTN
#undef VERTICAL_NAME
#undef HORIZONTAL_NAME
#undef COMB_NAME
#undef EQ_COMB_NAME

#undef TENTH_BTN
#undef FIFTH_BTN
#undef QUARTER_BTN
#undef THIRD_BTN
#undef HALF_BTN
#undef ONE_BTN
#undef TWO_BTN
#undef TEN_BTN
#undef INFINITE_BTN
#undef TENTH_NAME
#undef FIFTH_NAME
#undef QUARTER_NAME
#undef THIRD_NAME
#undef HALF_NAME
#undef ONE_NAME
#undef TWO_NAME
#undef TEN_NAME
#undef INFINITE_NAME
