/* gt_ctls.c */
/************************************************************
 *                                                          *
 *  Permission is hereby granted  to  any  individual   or  *
 *  institution   for  use,  copying, or redistribution of  *
 *  the xgobi code and associated documentation,  provided  *
 *  that   such  code  and documentation are not sold  for  *
 *  profit and the  following copyright notice is retained  *
 *  in the code and documentation:                          *
 *     Copyright (c) held jointly by Dianne Cook and        *
 *     AT&T Bell Labs (1995).                               *
 *  All Rights Reserved.                                    *
 *                                                          *
 *  We welcome your questions and comments, and request     *
 *  that you share any modifications with us.               *
 *                                                          *
 *      Dianne Cook                Andreas Buja             *
 *    dicook@iastate.edu       andreas@research.att.com     *
 *                                                          *
 ************************************************************/

#include <stdio.h>
#include <math.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

static float **Rmat1, **Rmat2;
static float *ftmp_vec;
static float **manip_var_basis;
static float **mvar_3dbasis;
static float **old_mvar_3dbasis;
static int actual_numvars_t;

#define OWN_TOUR_SELECTION XtOwnSelection( (Widget) xg->workspace, \
  (Atom) XG_NEWTOUR, (Time) CurrentTime, \
  (XtConvertSelectionProc) pack_tour_data, \
  (XtLoseSelectionProc) pack_tour_lose , \
  (XtSelectionDoneProc) pack_tour_done )
XtConvertSelectionProc pack_tour_data() ;
XtSelectionDoneProc pack_tour_done() ;
XtLoseSelectionProc pack_tour_lose() ;

void alloc_gt_ctls(xgobidata *xg)
{
  int k;
  Cardinal nc = xg->ncols;

  manip_var_basis = (float **) XtMalloc((Cardinal) 3 * sizeof(float *));
  for (k=0; k<3; k++)
    manip_var_basis[k] = (float *) XtMalloc(nc * sizeof(float));
  mvar_3dbasis = (float **) XtMalloc((Cardinal) 3 * sizeof(float *));
  for (k=0; k<3; k++)
    mvar_3dbasis[k] = (float *) XtMalloc((Cardinal) 3 * sizeof(float));
  old_mvar_3dbasis = (float **) XtMalloc((Cardinal) 3 * sizeof(float *));
  for (k=0; k<3; k++)
    old_mvar_3dbasis[k] = (float *) XtMalloc((Cardinal) 3 * sizeof(float));
  ftmp_vec = (float *) XtMalloc(nc * sizeof(float *));

  Rmat1 = (float **)XtMalloc(3 * sizeof(float *));
  for (k=0; k<3; k++)
    Rmat1[k] = (float *) XtMalloc(3 * sizeof(float));
  Rmat2 = (float **)XtMalloc(3 * sizeof(float *));
  for (k=0; k<3; k++)
    Rmat2[k] = (float *) XtMalloc(3 * sizeof(float));
}

void free_gt_ctls(void)
{
  int k;

  for (k=0; k<3; k++)
    XtFree((XtPointer) manip_var_basis[k]);
  XtFree((XtPointer) manip_var_basis);
  for (k=0; k<3; k++)
    XtFree((XtPointer) mvar_3dbasis[k]);
  XtFree((XtPointer) mvar_3dbasis);
  for (k=0; k<3; k++)
    XtFree((XtPointer) old_mvar_3dbasis[k]);
  XtFree((XtPointer) old_mvar_3dbasis);
  XtFree((XtPointer) ftmp_vec);
  for (k=0; k<3; k++)
    XtFree((XtPointer) Rmat1[k]);
  XtFree((XtPointer) Rmat1);
  for (k=0; k<3; k++)
    XtFree((XtPointer) Rmat2[k]);
  XtFree((XtPointer) Rmat2);
}

/* ARGSUSED */
XtCallbackProc
set_tour_manip_type_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k;
  extern Widget manip_type_menu_cmd;
  extern Widget manip_type_menu_btn[];
  extern char *manip_type_menu_name[];
  int manip_type_menu_id;

  for (k=0; k<5; k++)
  {
    if (w == manip_type_menu_btn[k])
    {
      manip_type_menu_id = k;
      break;
    }
  }
  
  XtVaSetValues(manip_type_menu_cmd,
    XtNlabel, manip_type_menu_name[manip_type_menu_id],
    NULL);

  if (manip_type_menu_id == 0)
    xg->tour_manip_type = oblique;
  else if (manip_type_menu_id == 1)
    xg->tour_manip_type = vertical;
  else if (manip_type_menu_id == 2)
    xg->tour_manip_type = horizontal;
  else if (manip_type_menu_id == 3)
    xg->tour_manip_type = radial;
  else if (manip_type_menu_id == 4)
    xg->tour_manip_type = angular;

}

/* ARGSUSED */
XtCallbackProc
set_tour_cont_fact_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k;
  extern Widget cont_fact_menu_cmd;
  extern Widget cont_fact_menu_btn[];
  extern char *cont_fact_menu_name[];
  int cont_fact_menu_id;

  for (k=0; k<9; k++)
  {
    if (w == cont_fact_menu_btn[k])
    {
      cont_fact_menu_id = k;
      break;
    }
  }
  
  XtVaSetValues(cont_fact_menu_cmd,
    XtNlabel, cont_fact_menu_name[cont_fact_menu_id],
    NULL);

  if (cont_fact_menu_id == 0)
  {
    xg->tour_cont_fact = tenth;
    xg->fcont_fact = 0.1;
    set_sens_localscan(true);
    if (xg->nhist_list >= 3)
      reset_backtrack_cmd(false, false, true, false);
  }
  else if (cont_fact_menu_id == 1)
  {
    xg->tour_cont_fact = fifth;
    xg->fcont_fact = 0.2;
    set_sens_localscan(true);
    if (xg->nhist_list >= 3)
      reset_backtrack_cmd(false, false, true, false);
  }
  else if (cont_fact_menu_id == 2)
  {
    xg->tour_cont_fact = quarter;
    xg->fcont_fact = 0.25;
    set_sens_localscan(true);
    if (xg->nhist_list >= 3)
      reset_backtrack_cmd(false, false, true, false);
  }
  else if (cont_fact_menu_id == 3)
  {
    xg->tour_cont_fact = third;
    xg->fcont_fact = 0.3333;
    set_sens_localscan(true);
    if (xg->nhist_list >= 3)
      reset_backtrack_cmd(false, false, true, false);
  }
  else if (cont_fact_menu_id == 4)
  {
    xg->tour_cont_fact = half;
    xg->fcont_fact = 0.5;
    set_sens_localscan(true);
    if (xg->nhist_list >= 3)
      reset_backtrack_cmd(false, false, true, false);
  }
  else if (cont_fact_menu_id == 5)
  {
    xg->tour_cont_fact = one;
    xg->fcont_fact = 1;
    set_sens_localscan(true);
    if (xg->nhist_list >= 3)
      reset_backtrack_cmd(false, false, true, false);
  }
  else if (cont_fact_menu_id == 6)
  {
    xg->tour_cont_fact = two;
    xg->fcont_fact = 2;
    set_sens_localscan(true);
    if (xg->nhist_list >= 3)
      reset_backtrack_cmd(false, false, true, false);
  }
  else if (cont_fact_menu_id == 7)
  {
    xg->tour_cont_fact = ten;
    xg->fcont_fact = 10;
    set_sens_localscan(true);
    if (xg->nhist_list >= 3)
      reset_backtrack_cmd(false, false, true, false);
  }
  else if (cont_fact_menu_id == 8)
  {
    xg->tour_cont_fact = infinite;
    set_sens_localscan(false);
    reset_backtrack_cmd(false, false, false, false);
  }

  if (xg->tour_cont_fact != infinite)
  {
    init_basis(xg);
    if (xg->is_store_history)
      store_basis(xg, xg->u0); 
    /* reset angles to start tour */
    zero_tau(xg);
  }
}

void
draw_manip_var(xgobidata *xg)
{
  XDrawArc(display, XtWindow(xg->vardraww[xg->manip_var]),
    varpanel_copy_GC,
    (int) 5, (int) 5,
    (unsigned int) (2*xg->radius - 10), (unsigned int) (2*xg->radius - 10),
    0, 360*64);
}

void
draw_frozen_var(xgobidata *xg)
{
  int k;
  XGCValues *gcv, gcv_inst;

  XGetGCValues(display, varpanel_xor_GC, GCCapStyle|GCJoinStyle, &gcv_inst);
  gcv = &gcv_inst;

  XSetLineAttributes(display, varpanel_copy_GC, 0, LineOnOffDash,
    gcv->cap_style, gcv->join_style);
  for (k=0; k<xg->nfrozen_vars; k++)
  {
    XDrawArc(display, XtWindow(xg->vardraww[xg->frozen_vars[k]]),
      varpanel_copy_GC,
      (int) 10, (int) 10,
      (unsigned int) (2*xg->radius - 20), (unsigned int) (2*xg->radius - 20),
      0, 360*64);
  }
  XSetLineAttributes(display, varpanel_copy_GC, 0, LineSolid,
    gcv->cap_style, gcv->join_style);
}

void
set_manip_var(xgobidata *xg, int varno)
{
  xg->manip_var = varno;
  draw_manip_var(xg);
  refresh_vboxes(xg);
}

Boolean 
var_frozen(xgobidata *xg, int varno)
{
  int k;
  Boolean chosen;

  chosen = False;
  for (k=0; k<xg->nfrozen_vars; k++)
  {
    if (xg->frozen_vars[k] == varno)
      chosen = True;
  }

  return(chosen);
}

void
set_frozen_var(xgobidata *xg, int varno)
{
  int j, k, chosen_id;
  Boolean chosen, actv;

  chosen = False;
  for (k=0; k<xg->nfrozen_vars; k++)
  {
    if (xg->frozen_vars[k] == varno)
    {
      chosen = True;
      chosen_id = k;
    }
  }

  if (chosen)
  {
    for (k=chosen_id+1; k<xg->nfrozen_vars; k++)
      xg->frozen_vars[k-1] = xg->frozen_vars[k];
    xg->nfrozen_vars--;
    xg->uwarm[0][varno] = xg->ufrozen[0][varno];
    xg->uwarm[1][varno] = xg->ufrozen[1][varno];
    xg->ufrozen[0][varno] = 0.;
    xg->ufrozen[1][varno] = 0.;
    zero_tau(xg);
    zero_princ_angles(xg);
    set_sens_pp_btn(xg, 1);
    set_sens_princ_comp(xg, 1);
    reset_backtrack_cmd(false, false, true, false);
    if (xg->tour_cont_fact == infinite)
      xg->new_direction_flag = True;
    /* add to the active list */
    if (add_variable(xg, varno))
      ;
  }
  else
  {
    actv = False;
    for (j=0; j<xg->numvars_t; j++)
    {
      if (xg->tour_vars[j] == varno)
        actv = True;
    }
    if (actv) /* only freeze if it is already active */
    {
      xg->frozen_vars[xg->nfrozen_vars] = varno;
      xg->ufrozen[0][varno] = xg->u[0][varno];
      xg->ufrozen[1][varno] = xg->u[1][varno];
      if (xg->nfrozen_vars == 0)
      {
        copy_basis(xg->u, xg->u0, xg->ncols_used);
        xg->u0[0][varno] = 0.;
        xg->u0[1][varno] = 0.;
        copy_basis(xg->u0, xg->uwarm, xg->ncols_used);
      }
      else
      {
        xg->uwarm[0][varno] = 0.;
        xg->uwarm[1][varno] = 0.;
      }
      xg->nfrozen_vars++;
      zero_tau(xg);
      zero_princ_angles(xg);
      remove_variable(xg, varno);
      set_sens_pp_btn(xg, 0);
      set_sens_princ_comp(xg, 0);
      reset_backtrack_cmd(false, false, false, false);
      if (xg->tour_cont_fact == infinite)
        xg->new_direction_flag = True;
    }
  }
  draw_frozen_var(xg);
  refresh_vboxes(xg);
}

void
make_coord_basis(xgobidata *xg)
{
  int j, k;
  float tol = 0.001, ftmp;
  
  for (j=0; j<xg->ncols_used; j++)
  {
    manip_var_basis[0][j] = xg->u0[0][j];
    manip_var_basis[1][j] = xg->u0[1][j];
    manip_var_basis[2][j] = 0.;
  }
  manip_var_basis[2][xg->manip_var] = 1.;
  gram_schmidt(manip_var_basis[0],  manip_var_basis[2],
    xg->ncols_used);
  gram_schmidt(manip_var_basis[1],  manip_var_basis[2],
    xg->ncols_used);
  ftmp = calc_norm(manip_var_basis[2], xg->ncols_used);
  while (ftmp < tol)
  {
    gen_norm_variates(1, xg->numvars_t, ftmp_vec);
    for (j=0; j<xg->ncols_used; j++)
      manip_var_basis[2][j] = 0.;
    for (j=0; j<xg->numvars_t; j++)
      manip_var_basis[2][xg->tour_vars[j]] = ftmp_vec[j];
    norm(manip_var_basis[2], xg->ncols_used);
    gram_schmidt(manip_var_basis[0],  manip_var_basis[2],
      xg->ncols_used);
    gram_schmidt(manip_var_basis[1],  manip_var_basis[2],
      xg->ncols_used);
    ftmp = calc_norm(manip_var_basis[2], xg->ncols_used);
  }
  for (j=0; j<3; j++)
  {
    for (k=0; k<3; k++)
      mvar_3dbasis[j][k] = old_mvar_3dbasis[j][k] = 0.;
    mvar_3dbasis[j][j] = old_mvar_3dbasis[j][j] = 1.;
  }
  for (j=0; j<xg->ncols_used; j++)
  {
    xg->uold[0][j] = xg->u[0][j] = manip_var_basis[0][j];
    xg->uold[1][j] = xg->u[1][j] = manip_var_basis[1][j];
  }
}

/* 
   Preproject the data into the 3d manipulation space: 
   uses 3 of the 4 vectors of the usual grand tour preprojection.
*/
void
gti_span_planes(xgobidata *xg)
{
  int i, j, m;

  for (m=0; m<xg->nrows_in_plot; m++)
  {
    i = xg->rows_in_plot[m];
    for (j=0; j<xg->ncols_used; j++)
      xg->tnx[j] = FLOAT(xg->world_data[i][j]);
      xg->xi0[i][0] = (long)(inner_prod(xg->tnx,
        manip_var_basis[0], xg->ncols_used));
      xg->xi0[i][1] = (long)(inner_prod(xg->tnx,
        manip_var_basis[1], xg->ncols_used));
      xg->xi1[i][0] = (long)(inner_prod(xg->tnx,
        manip_var_basis[2], xg->ncols_used));
  }
}

void
gti_reproject(xgobidata *xg)
{
  int i, j, m;

  for (m=0; m<xg->nrows_in_plot; m++)
  {
    i = xg->rows_in_plot[m];

    xg->planar[i].x = (long) (mvar_3dbasis[0][0]*(float)xg->xi0[i][0] +
      mvar_3dbasis[0][1]*(float)xg->xi0[i][1] +
      mvar_3dbasis[0][2]*(float)xg->xi1[i][0]) ;
    xg->planar[i].y = (long) (mvar_3dbasis[1][0]*(float)xg->xi0[i][0] +
      mvar_3dbasis[1][1]*(float)xg->xi0[i][1] +
      mvar_3dbasis[1][2]*(float)xg->xi1[i][0]) ;
  }  

  for (j=0; j<xg->ncols_used; j++)
  {
    xg->u[0][j] = manip_var_basis[0][j]*mvar_3dbasis[0][0] +
      manip_var_basis[1][j]*mvar_3dbasis[0][1] +
      manip_var_basis[2][j]*mvar_3dbasis[0][2];
    xg->u[1][j] = manip_var_basis[0][j]*mvar_3dbasis[1][0] +
      manip_var_basis[1][j]*mvar_3dbasis[1][1] +
      manip_var_basis[2][j]*mvar_3dbasis[1][2];
  }/* handle sphereing somehow */

  if (xg->is_tour_section)
    tour_section_calcs(xg, 1);
}

/* ARGSUSED */
XtEventHandler
interact_gt(Widget w, xgobidata *xg, XEvent *evnt)
{
  int i;
  static icoords pos, prev_pos;
  float distx, disty, len_motion;
  static float rx, ry;
  float phi, cosphi, sinphi, cosm;
  static float cospsi, sinpsi; /* angular constrained vars */
  float x1, y1, x2, y2; 
  float denom = (float) MIN(xg->mid.x, xg->mid.y);
  float ca, sa;
  float tol = 0.01;
  double dtmp1, dtmp2;
  static Boolean no_dir_flag = False;
  Boolean overlap = False;

  if (evnt->type == ButtonPress)
  {
    XButtonEvent *xbutton = (XButtonEvent *) evnt;
    if (evnt->xbutton.button == 1 || evnt->xbutton.button == 2)
    {
      stop_tour_proc(xg);
      while (xg->nfrozen_vars > 0)
        set_frozen_var(xg, xg->frozen_vars[0]);

      pos.x = prev_pos.x = xbutton->x;
      pos.y = prev_pos.y = xbutton->y;

      actual_numvars_t = xg->numvars_t;
      overlap = False;
      for (i=0; i<xg->numvars_t; i++)
        if (xg->manip_var == xg->tour_vars[i])
          overlap = True;
      if (!overlap)
        actual_numvars_t++;

      if (actual_numvars_t > 2)
      {
        init_basis(xg);
        if (xg->is_store_history)
          store_basis(xg, xg->u0); 
        /* reset angles to start tour */
        zero_tau(xg);

        alloc_gt_ctls(xg);
        make_coord_basis(xg);
/* Set rotation axis in the radial manipulation type */
        if (xg->tour_manip_type == radial)
        {
          if ((xg->u0[0][xg->manip_var]*xg->u0[0][xg->manip_var] +
            xg->u0[1][xg->manip_var]*xg->u0[1][xg->manip_var]) < tol)
            no_dir_flag = True;
          else
          {
            rx = xg->u0[0][xg->manip_var];
            ry = xg->u0[1][xg->manip_var];
            dtmp1 = sqrt(rx*rx+ry*ry);
            rx /= dtmp1;
            ry /= dtmp1;
          }
        }
  
        gti_span_planes(xg);
        gti_reproject(xg);
        plane_to_screen(xg);
        plot_once(xg);
        tour_var_lines(xg);
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

    if (actual_numvars_t > 2)
    {
      if (xg->tour_manip_type != angular)
      {
        if (xg->tour_manip_type == oblique)
        {
          distx = pos.x - prev_pos.x;
          disty = prev_pos.y - pos.y;
        }
        else if (xg->tour_manip_type == vertical)
        {
          distx = 0.;
          disty = prev_pos.y - pos.y;
        }
        else if (xg->tour_manip_type == horizontal)
        {
          distx = pos.x - prev_pos.x;
          disty = 0.;
        }
        else if (xg->tour_manip_type == radial)
        {
          if (no_dir_flag)
          {
            distx = pos.x - prev_pos.x;
            disty = prev_pos.y - pos.y;
            rx = distx;
            ry = disty; 
            dtmp1 = sqrt(rx*rx+ry*ry);
            rx /= dtmp1;
            ry /= dtmp1;
            no_dir_flag = False;
          }
          distx = (rx*(pos.x - prev_pos.x) + ry*(prev_pos.y - pos.y))*rx;
          disty = (rx*(pos.x - prev_pos.x) + ry*(prev_pos.y - pos.y))*ry;
        }
        dtmp1 = (double) (distx*distx+disty*disty);
        len_motion = (float) sqrt(dtmp1);
    
        if (len_motion != 0)
        {
          phi = len_motion / denom;
    
          ca = distx/len_motion;
          sa = disty/len_motion;
    
          cosphi = (float) cos((double) phi);
          sinphi = (float) sin((double) phi);
          cosm = 1.0 - cosphi;
          Rmat2[0][0] = ca*ca*cosphi + sa*sa;
          Rmat2[0][1] = -cosm*ca*sa;
          Rmat2[0][2] = sinphi*ca;
          Rmat2[1][0] = -cosm*ca*sa;
          Rmat2[1][1] = sa*sa*cosphi + ca*ca;
          Rmat2[1][2] = sinphi*sa;
          Rmat2[2][0] = -sinphi*ca;
          Rmat2[2][1] = -sinphi*sa;
          Rmat2[2][2] = cosphi;
          matrix_mult(mvar_3dbasis, Rmat2, Rmat1, 3, 3, 3);
          copy_matrix(Rmat1, mvar_3dbasis, 3, 3);
      
          gram_schmidt(mvar_3dbasis[0], mvar_3dbasis[1], 3);
          gram_schmidt(mvar_3dbasis[0], mvar_3dbasis[2], 3);
          gram_schmidt(mvar_3dbasis[1], mvar_3dbasis[2], 3);
    
          gti_reproject(xg);
          plane_to_screen(xg);
          plot_once(xg);
          tour_var_lines(xg);
          if (xg->tour_link_state == send_state)
          {
            xg->new_basis_ind = False;
            OWN_TOUR_SELECTION ;
            announce_tour_coefs(xg);
          }
          if (xg->is_pp)
            pp_index(xg,0,0);
        }
      }
      else /* angular constrained manipulation */
      {
        if (prev_pos.x != xg->mid.x && prev_pos.y != xg->mid.y &&
          pos.x != xg->mid.x && pos.y != xg->mid.y)
        {
          x1 = prev_pos.x - xg->mid.x;
          y1 = prev_pos.y - xg->mid.y;
          dtmp1 = sqrt(x1*x1+y1*y1);
          x1 /= dtmp1;
          y1 /= dtmp1;
          x2 = pos.x - xg->mid.x;
          y2 = pos.y - xg->mid.y;
          dtmp2 = sqrt(x2*x2+y2*y2);
          x2 /= dtmp2;
          y2 /= dtmp2;
          if (dtmp1 > tol && dtmp2 > tol)
          {
            cospsi = x1*x2+y1*y2;
            sinpsi = x1*y2-y1*x2;
          }
          else
          {
            cospsi = 1.;
            sinpsi = 0.;
          }
        }
        else
        {
          cospsi = 1.;
          sinpsi = 0.;
        }
        Rmat2[0][0] = cospsi;
        Rmat2[0][1] = sinpsi;
        Rmat2[0][2] = 0.;
        Rmat2[1][0] = -sinpsi;
        Rmat2[1][1] = cospsi;
        Rmat2[1][2] = 0.;
        Rmat2[2][0] = 0.;
        Rmat2[2][1] = 0.;
        Rmat2[2][2] = 1.;
        matrix_mult(mvar_3dbasis, Rmat2, Rmat1, 3, 3, 3);
        copy_matrix(Rmat1, mvar_3dbasis, 3, 3);
    
        gram_schmidt(mvar_3dbasis[0], mvar_3dbasis[1], 3);
        gram_schmidt(mvar_3dbasis[0], mvar_3dbasis[2], 3);
        gram_schmidt(mvar_3dbasis[1], mvar_3dbasis[2], 3);
  
        gti_reproject(xg);
        plane_to_screen(xg);
        plot_once(xg);
        tour_var_lines(xg);
        if (xg->tour_link_state == send_state)
        {
          xg->new_basis_ind = False;
          OWN_TOUR_SELECTION ;
          announce_tour_coefs(xg);
        }
        if (xg->is_pp)
          pp_index(xg,0,0);
      }
    }
  }
  else if (evnt->type == ButtonRelease)
  {
    if (actual_numvars_t > 2)
    {
    /* need to get current projection and store it */
      init_basis(xg);
      if (xg->is_store_history)
        store_basis(xg, xg->u0);
      copy_u0_to_pd0(xg);
      zero_tau(xg);
      if (xg->tour_cont_fact == infinite)
        xg->new_direction_flag = True; 
      free_gt_ctls();
    }
    actual_numvars_t = 0;
    start_tour_proc(xg);
  }
}

void
tour_event_handlers(xgobidata *xg, Boolean add)
{
  if (add)
  {
    XtAddEventHandler(xg->workspace,
      ButtonPressMask | ButtonReleaseMask |
      Button1MotionMask | Button2MotionMask,
      FALSE, (XtEventHandler) interact_gt, (XtPointer) xg);
    XDefineCursor(display, XtWindow(xg->workspace), spin_cursor);
  }
  else
  {
    XtRemoveEventHandler(xg->workspace, XtAllEvents,
      TRUE, (XtEventHandler) interact_gt, (XtPointer) xg);
    XDefineCursor(display, XtWindow(xg->workspace), default_cursor);
  }
}
