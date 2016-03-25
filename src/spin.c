/* spin.c */
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
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

/* Used in oblique rotation code */
static struct {float x, y, z;} ob_axis;
static float Rmat1[3][3], Rmat2[3][3];
long lRmat[3][3];
float *spin_coef;

static int spin_dir = 1, turn = -1, curdir = 1;
static icoords qd;

/* --------- Dynamic allocation section ----------- */

void
alloc_rotate_arrays(xgobidata *xg)
/*
 * Dynamically allocate arrays.
*/
{
  Cardinal nc = (Cardinal) xg->ncols;

  spin_coef = (float *)
    XtMalloc((unsigned int) 2 * nc * sizeof(float));
}

void
free_rotate_arrays()
/*
 * Dynamically free arrays.
*/
{
  XtFree((XtPointer) spin_coef);
}


/* --------- End of dynamic allocation section ----------- */

void
init_trig(xgobidata *xg)
/*
 * Initiallize icost and isint, which are scaled up so that the rotation
 * calculations can be done in integer arithmetic.
*/
{
  xg->theta.yaxis = xg->theta.xaxis = 0.0;

  xg->cost.y = xg->cost.x = (float) cos((double) xg->theta0);
  xg->sint.y = xg->sint.x = (float) sin((double) xg->theta0);
  xg->ocost.y = xg->ocost.x = xg->icost.y = xg->icost.x =
    (int) (xg->cost.y * PRECISION2);
  xg->osint.y = xg->osint.x = xg->isint.y = xg->isint.x =
    (int) (xg->sint.y * PRECISION2);
}

void
find_Rmat(xgobidata *xg)
{
  float cosn = (float) cos((double) xg->theta.oblique);
  float sinn = (float) sin((double) xg->theta.oblique);
  float cosm;
/*
 * Calculate new rotation matrix
*/
  cosm = 1.0 - cosn;
  Rmat2[0][0] = (cosm * ob_axis.x * ob_axis.x) + cosn;
  Rmat2[0][1] = (cosm * ob_axis.x * ob_axis.y) + (sinn * ob_axis.z);
  Rmat2[0][2] = (cosm * ob_axis.x * ob_axis.z) - (sinn * ob_axis.y);
  Rmat2[1][0] = (cosm * ob_axis.x * ob_axis.y) - (sinn * ob_axis.z);
  Rmat2[1][1] = (cosm * ob_axis.y * ob_axis.y) + cosn;
  Rmat2[1][2] = (cosm * ob_axis.y * ob_axis.z) + (sinn * ob_axis.x);
  Rmat2[2][0] = (cosm * ob_axis.x * ob_axis.z) + (sinn * ob_axis.y);
  Rmat2[2][1] = (cosm * ob_axis.y * ob_axis.z) - (sinn * ob_axis.x);
  Rmat2[2][2] = (cosm * ob_axis.z * ob_axis.z) + cosn;
}

void print_Rmat(float Rmat[3][3], char *lbl) {
  int j;
  fprintf(stderr, "%s\n", lbl);
  for (j=0; j<3; j++)
    fprintf(stderr, "%f %f %f\n", 
      Rmat[0][j], Rmat[1][j], Rmat[2][j]);
}
void print_lRmat(long Rmat[3][3], char *lbl) {
  int j;
  fprintf(stderr, "%s\n", lbl);
  for (j=0; j<3; j++)
    fprintf(stderr, "%ld %ld %ld\n", 
      Rmat[0][j], Rmat[1][j], Rmat[2][j]);
}

void
init_ob_rotate(xgobidata *xg)
{
  int j, k;

  xg->theta.oblique = 0.;

/*
 * Initialize the axis of rotation.
*/
  ob_axis.x = 0;
  ob_axis.y = 1;
  ob_axis.z = 0;

  for (j=0; j<3; j++) {
    for (k=0; k<3; k++) {
      if (j==k)
        xg->Rmat0[k][j] = Rmat1[j][k] = Rmat2[j][k] = 1.;
      else
        xg->Rmat0[k][j] = Rmat1[j][k] = Rmat2[j][k] = 0.;
    }
  }

  find_Rmat(xg);
}

void
init_rotate_vars(xgobidata *xg)
{
  static Boolean firsttime = True;

  if (firsttime) {
    xg->is_spin_paused = False;
    xg->is_interp = False;
    xg->is_rocking = False;
    xg->run_spin_axis_proc = False;
    xg->run_rock_proc = False;
    xg->run_interp_proc = False;
    xg->run_spin_oblique_proc = False;
    xg->is_spin_type.xaxis = xg->is_spin_type.yaxis = False;
    xg->is_spin_type.oblique = True;

    xg->theta0 = THETA0;
    xg->xax.x = RADIUS;
    xg->xax.y = 0;
    xg->yax.x = 0;
    xg->yax.y = RADIUS;
    xg->zax.x = 0;
    xg->zax.y = 0;

    firsttime = False;
  }

  xg->is_spinning = False;
  init_trig(xg);
  init_ob_rotate(xg);

  xg->spin_vars.x = 0;
  xg->spin_vars.z = 1;
  xg->spin_vars.y = 2;
}


void
change_spin_direction()
{
  spin_dir = -1 * spin_dir;
}

void
ax_rot_reproject(xgobidata *xg)
{
  int j;
  long nx, ny;
  int ix = xg->spin_vars.x;
  int iy = xg->spin_vars.y;
  int iz = xg->spin_vars.z;

  if (xg->is_spin_type.yaxis) {   /* Rotate around y, vertical axis */
    for (j=0; j<xg->nrows; j++) {
      nx = xg->icost.y * xg->world_data[j][ix] +
         /*xg->isint.y * xg->world_data[j][iz];*/
         xg->isint.y * xg->world_data[j][iy];
      xg->planar[j].x = nx/PRECISION2;
      /*xg->planar[j].y = xg->world_data[j][iy];*/
      xg->planar[j].y = xg->world_data[j][iz];
    }
  }
  else {                          /* Rotate around z, horizontal axis */
    for (j=0; j<xg->nrows; j++) {
      xg->planar[j].x = xg->world_data[j][iz];
      ny = xg->icost.x * xg->world_data[j][ix] +
         /*xg->isint.x * xg->world_data[j][iz];*/
         xg->isint.x * xg->world_data[j][iy];
      xg->planar[j].y = ny/PRECISION2;
    }
  }
}

void
reset_interp_proc()
{
  turn = -1;
  curdir = 1;
  if (spin_dir == 0)
    spin_dir = 1;
}

void
find_quadrant(xgobidata *xg)
/*
 * Figure out in which quadrant theta lies.
*/
{
  spin_dir = 1;

  xg->theta.yaxis = (float) fabs((double) xg->theta.yaxis);
  if (xg->theta.yaxis >= 3*M_PI_2 && xg->theta.yaxis < 2*M_PI)
    qd.y = 4;
  else if (xg->theta.yaxis >= M_PI)
    qd.y = 3;
  else if (xg->theta.yaxis >= M_PI_2)
    qd.y = 2;
  else
    qd.y = 1;

  xg->theta.xaxis = (float) fabs((double) xg->theta.xaxis);
  if (xg->theta.xaxis >= 3*M_PI_2 && xg->theta.yaxis < 2*M_PI)
    qd.x = 4;
  else if (xg->theta.xaxis >= M_PI)
    qd.x = 3;
  else if (xg->theta.xaxis >= M_PI_2)
    qd.x = 2;
  else
    qd.x = 1;
}

void
spin_once(xgobidata *xg)
/*
 * Recalculate the new position for the data in planar[], that is,
 * the data being displayed scaled to +-32K.  The array 'world_data'
 * holds all the data similarly scaled.
*/
{
  world_to_plane(xg);

  if (xg->is_spin_type.yaxis) {  /* Use vertical axis */
    xg->cost.y = (float) cos((double) xg->theta.yaxis);
    xg->sint.y = (float) sin((double) xg->theta.yaxis);
    xg->icost.y = (int) (xg->cost.y * PRECISION2);
    xg->isint.y = (int) (xg->sint.y * PRECISION2);

    xg->theta.yaxis = xg->theta.yaxis + (float) spin_dir * xg->theta0;
    if (xg->theta.yaxis > MAXTHETA)
      xg->theta.yaxis -= TWOPI;
    else if (xg->theta.yaxis < -MAXTHETA)
      xg->theta.yaxis += TWOPI;
  } else if (xg->is_spin_type.xaxis) {
    /*
     * Use horizontal axis in the plane of the screen
    */
    xg->cost.x = (float) cos((double) xg->theta.xaxis);
    xg->sint.x = (float) sin((double) xg->theta.xaxis);
    xg->icost.x = (int) (xg->cost.x * PRECISION2);
    xg->isint.x = (int) (xg->sint.x * PRECISION2);

    xg->theta.xaxis = xg->theta.xaxis +
      (float) spin_dir * xg->theta0;
    if (xg->theta.xaxis > MAXTHETA)
      xg->theta.xaxis -= TWOPI;
    else if (xg->theta.xaxis < -MAXTHETA)
      xg->theta.xaxis += TWOPI;
  }
}

void
ob_rotate_once(xgobidata *xg)
/*
 * Recalculate the new position for the data in planar[], that is,
 * the data being displayed scaled to +-32K.  The array world_data[]
 * holds all the data similarly scaled.
*/
{
  find_Rmat(xg);  /* calculates Rmat2 */
  world_to_plane(xg);  /* calculates Rmat0 from Rmat2 and Rmat1 */

  xg->theta.oblique = xg->theta.oblique + (float) spin_dir * xg->theta0;
  if (xg->theta.oblique > MAXTHETA)
    xg->theta.oblique -= TWOPI;
  else if (xg->theta.oblique < -MAXTHETA)
    xg->theta.oblique += TWOPI;
}

void
spin_proc(xgobidata *xg)
{
  spin_once(xg);
  plane_to_screen(xg);
  plot_once(xg);
  spin_var_lines(xg);
}

void
interp_proc(xgobidata *xg)
{
  float angle;
  int quadrant;

  spin_once(xg);
  plane_to_screen(xg);
  plot_once(xg);
  spin_var_lines(xg);

  if (xg->is_spin_type.yaxis) {
    angle = xg->theta.yaxis;
    quadrant = qd.y;
  }
  else {
    angle = xg->theta.xaxis;
    quadrant = qd.x;
  }

  if (turn == 0) {
    curdir = -1 * curdir;
    spin_dir = curdir;
    turn = -1;
  } else if (turn < 0) {
    switch(quadrant) {
      case 1:
        if (curdir == 1) {
          if (angle >= M_PI_2 - xg->theta0) {
            angle = M_PI_2;
            turn = 7;
          }
        } else {
          if (angle <= xg->theta0) {
            angle = 0;
            turn = 7;
          }
        }
        break;

      case 2:
        if (curdir == 1) {
          if (angle >= M_PI - xg->theta0) {
            angle = M_PI;
            turn = 7;
          }
        } else {
          if (angle <= M_PI_2 + xg->theta0) {
            angle = M_PI_2;
            turn = 7;
          }
        }
        break;

      case 3:
        if (curdir == 1) {
          if (angle >= 3*M_PI_2 - xg->theta0) {
            angle = 3*M_PI_2;
            turn = 7;
          }
        } else {
          if (angle <= M_PI + xg->theta0) {
            angle = M_PI;
            turn = 7;
          }
        }
        break;

      case 4:
        if (curdir == 1) {
          if (angle >= 2*M_PI - xg->theta0) {
            angle = 2*M_PI;
            turn = 7;
          }
        }
        else {
          if (angle <= 3*M_PI_2 + xg->theta0) {
            angle = 3*M_PI_2;
            turn = 7;
          }
        }
        break;
      }

  }

  if (turn > 0) {
    spin_dir = 0;
    turn--;
  }
  if (xg->is_spin_type.yaxis)
    xg->theta.yaxis = angle;
  else
    xg->theta.xaxis = angle;
}


void
rock_proc(xgobidata *xg)
{
  static int j = 0;

  if (j == 10) {
    spin_dir *= -1;
    j = 0;
  }
  else
    j++;

  if (xg->is_spin_type.oblique) {
    ob_rotate_once(xg);
    draw_ob_var_lines(xg);
  } else {
    spin_once(xg);
    spin_var_lines(xg);
  }
  plane_to_screen(xg);
  plot_once(xg);
}

void
draw_ax_spin_axes(xgobidata *xg)
{
  int naxes = 6;
  int raw_axis_len = MIN(xg->mid.x, xg->mid.y);
  XSegment spin_axes[10];
  icoords cntr;

  if (xg->is_axes_centered) {
    cntr.x = xg->cntr.x;
    cntr.y = xg->cntr.y;
  } else {
    raw_axis_len /= 2.0;
    cntr.x = 1.1 * raw_axis_len * xg->scale.x;
    cntr.y = xg->plotsize.height - (1.1 * raw_axis_len * xg->scale.y);
  }

  if (xg->is_spin_type.yaxis) {

    /* horizontal axis ... x */
    spin_axes[0].x1 = cntr.x;
    spin_axes[0].x2 = cntr.x +
      (int) (raw_axis_len * xg->scale.x) * xg->icost.y/PRECISION2;
    spin_axes[0].y1 = spin_axes[0].y2 = cntr.y;
    /* end of x axis */
    spin_axes[1].x1 = spin_axes[1].x2 = spin_axes[0].x2;
    spin_axes[1].y1 = spin_axes[0].y1 - 2;
    spin_axes[1].y2 = spin_axes[0].y1 + 2;

    /* horizontal axis ... y */
    spin_axes[2].x1 = cntr.x;
    spin_axes[2].x2 = cntr.x +
      (int) (raw_axis_len * xg->scale.x) * xg->isint.y/PRECISION2;
    spin_axes[2].y1 = spin_axes[2].y2 = cntr.y;
    /* end of y axis */
    spin_axes[3].x1 = spin_axes[3].x2 = spin_axes[2].x2;
    spin_axes[3].y1 = spin_axes[2].y1 - 2;
    spin_axes[3].y2 = spin_axes[2].y1 + 2;

    /* vertical axis ... z */
    spin_axes[4].x1 = spin_axes[4].x2 = cntr.x;
    spin_axes[4].y1 = cntr.y;
    spin_axes[4].y2 = cntr.y -
      (int) (raw_axis_len * xg->scale.y);
    /* end of z axis */
    spin_axes[5].x1 = spin_axes[4].x1 - 2;
    spin_axes[5].x2 = spin_axes[4].x1 + 2;
    spin_axes[5].y1 = spin_axes[5].y2 = spin_axes[4].y2;

  } else {

    /* horizontal axis ... z */
    spin_axes[4].x1 = cntr.x;
    spin_axes[4].x2 = cntr.x +
      (int) (raw_axis_len * xg->scale.x);
    spin_axes[4].y1 = spin_axes[4].y2 = cntr.y;
    /* end of z axis */
    spin_axes[5].x1 = spin_axes[5].x2 = spin_axes[4].x2;
    spin_axes[5].y1 = spin_axes[4].y1 - 2;
    spin_axes[5].y2 = spin_axes[4].y1 + 2;

    /* vertical axis ... x */
    spin_axes[0].x1 = spin_axes[0].x2 = cntr.x;
    spin_axes[0].y1 = cntr.y;
    spin_axes[0].y2 = cntr.y -
      (int) (raw_axis_len * xg->scale.x) * xg->icost.x/PRECISION2;
    /* end of z axis */
    spin_axes[1].x1 = spin_axes[0].x1 - 2;
    spin_axes[1].x2 = spin_axes[0].x1 + 2;
    spin_axes[1].y1 = spin_axes[1].y2 = spin_axes[0].y2;

    /* vertical axis ... y */
    spin_axes[2].x1 = spin_axes[2].x2 = cntr.x;
    spin_axes[2].y1 = cntr.y;
    spin_axes[2].y2 = cntr.y -
      (int) (raw_axis_len * xg->scale.x) * xg->isint.x/PRECISION2;
    /* end of y axis */
    spin_axes[3].y1 = spin_axes[3].y2 = spin_axes[2].y2;
    spin_axes[3].x1 = spin_axes[2].x1 - 2;
    spin_axes[3].x2 = spin_axes[2].x1 + 2;
  }
/*
 * Draw axes of rotation.
*/
  XSetForeground(display, copy_GC, plotcolors.fg);
  XDrawSegments(display, xg->pixmap0, copy_GC, spin_axes, naxes);
/*
 * Add axis labels.
*/
  XDrawString(display, xg->pixmap0, copy_GC,
    spin_axes[0].x2 + 6, spin_axes[0].y2 - 5,
    xg->collab_tform2[xg->spin_vars.x],
    strlen(xg->collab_tform2[xg->spin_vars.x]));

  XDrawString(display, xg->pixmap0, copy_GC,
    spin_axes[2].x2 + 6, spin_axes[2].y2 - 5,
    xg->collab_tform2[xg->spin_vars.y],
    strlen(xg->collab_tform2[xg->spin_vars.y]));

  XDrawString(display, xg->pixmap0, copy_GC,
    spin_axes[4].x2 + 6, spin_axes[4].y2 - 5,
    xg->collab_tform2[xg->spin_vars.z],
    strlen(xg->collab_tform2[xg->spin_vars.z]));
}

void
store_Rmat(xgobidata *xg)
{
  int j, k;
/*
 * Copy previous rotation matrix into Rmat1.
*/
  for (j=0; j<3; j++)
    for (k=0; k<3; k++)
      Rmat1[k][j] = xg->Rmat0[k][j];
}

void
ob_rot_reproject(xgobidata *xg)
{
  int i, j, k;
  long nx, ny;
  float ftmp;
  int ix = xg->spin_vars.x;
  int iy = xg->spin_vars.y;
  int iz = xg->spin_vars.z;
/*
 * Find the matrix product of the new matrix and the previous one.
*/
  for (i=0; i<3; i++) {
    for (j=0; j<3; j++) {
      ftmp = 0.;
      for (k=0; k<3; k++)
        ftmp = ftmp + (Rmat2[i][k] * Rmat1[k][j]);
      xg->Rmat0[i][j] = ftmp;
    }
  }
/*
 * Convert rotation matrix to type long.
 * We'll fill lRmat even though the 3rd column isn't used here;
 * it is needed in move_points.c, and it could be used here for
 * depth cuing some day.
*/
  for (j=0; j<3; j++)
    for (k=0; k<3; k++)
      lRmat[k][j] = (long) (xg->Rmat0[k][j] * PRECISION2);
/*
 * Find new position of the data.
*/
  for (k=0; k<xg->nrows_in_plot; k++) {
    j = xg->rows_in_plot[k];
    nx =
      lRmat[0][0] * xg->world_data[j][ix] +
      lRmat[0][1] * xg->world_data[j][iy] +
      lRmat[0][2] * xg->world_data[j][iz];

    xg->planar[j].x = nx/PRECISION2;
    ny =
      lRmat[1][0] * xg->world_data[j][ix] +
      lRmat[1][1] * xg->world_data[j][iy] +
      lRmat[1][2] * xg->world_data[j][iz];

    xg->planar[j].y = ny/PRECISION2;
  }
}

void
draw_ob_spin_axes(xgobidata *xg)
{
  int naxes = 3;
  int raw_axis_len = MIN(xg->mid.x, xg->mid.y);
  XSegment spin_axes[3];
  icoords cntr;

  if (xg->is_axes_centered) {
    cntr.x = xg->cntr.x;
    cntr.y = xg->cntr.y;
  } else {
    raw_axis_len /= 2.0;
    cntr.x = 1.1 * raw_axis_len * xg->scale.x;
    cntr.y = xg->plotsize.height - (1.1 * raw_axis_len * xg->scale.y);
  }

  /* horizontal axis ... x */
  spin_axes[0].x1 = cntr.x;
  spin_axes[0].x2 = cntr.x +
    (int) (raw_axis_len * xg->scale.x) * xg->Rmat0[0][0];
  spin_axes[0].y1 = cntr.y;
  spin_axes[0].y2 = cntr.y -
    (int) (raw_axis_len * xg->scale.y) * xg->Rmat0[1][0];

  /* vertical axis ... y */
  spin_axes[1].x1 = cntr.x;
  spin_axes[1].x2 = cntr.x +
    (int) (raw_axis_len * xg->scale.x) * xg->Rmat0[0][1];
  spin_axes[1].y1 = cntr.y;
  spin_axes[1].y2 = cntr.y -
    (int) (raw_axis_len * xg->scale.y) * xg->Rmat0[1][1];

  /* horizontal axis ... z */
  spin_axes[2].x1 = cntr.x;
  spin_axes[2].x2 = cntr.x +
    (int) (raw_axis_len * xg->scale.x) * xg->Rmat0[0][2];
  spin_axes[2].y1 = cntr.y;
  spin_axes[2].y2 = cntr.y -
    (int) (raw_axis_len * xg->scale.y) * xg->Rmat0[1][2];
/*
 * Draw x, y, z axes.
*/
  XSetForeground(display, copy_GC, plotcolors.fg);
  XDrawSegments(display, xg->pixmap0, copy_GC, spin_axes, naxes);
/*
 * Add axis labels.
*/
  XDrawString(display, xg->pixmap0, copy_GC,
    spin_axes[0].x2 + 6, spin_axes[0].y2 - 5,
    xg->collab_tform2[xg->spin_vars.x],
    strlen(xg->collab_tform2[xg->spin_vars.x]));
  XDrawString(display, xg->pixmap0, copy_GC,
    spin_axes[1].x2 + 6, spin_axes[1].y2 - 5,
    xg->collab_tform2[xg->spin_vars.y],
    strlen(xg->collab_tform2[xg->spin_vars.y]));
  XDrawString(display, xg->pixmap0, copy_GC,
    spin_axes[2].x2 + 6, spin_axes[2].y2 - 5,
    xg->collab_tform2[xg->spin_vars.z],
    strlen(xg->collab_tform2[xg->spin_vars.z]));
}

void
ob_rotate_proc(xgobidata *xg)
{
  ob_rotate_once(xg);
  plane_to_screen(xg);
  plot_once(xg);
  draw_ob_var_lines(xg);
}

/* ARGSUSED */
XtEventHandler
ob_button(Widget w, xgobidata *xg, XEvent *evnt)
{
  static icoords pos, prev_pos;
  float distx, disty, len_motion;
  float phi, cosphi, sinphi, cosm;
  float denom = (float) MIN(xg->mid.x, xg->mid.y);

  if (evnt->type == ButtonPress) {
    XButtonEvent *xbutton = (XButtonEvent *) evnt;
    if (evnt->xbutton.button == 1 || evnt->xbutton.button == 2) {
      stop_spin_proc(xg);

      pos.x = prev_pos.x = xbutton->x;
      pos.y = prev_pos.y = xbutton->y;
    }
  } else if (evnt->type == MotionNotify) {
    XMotionEvent *xmotion = (XMotionEvent *) evnt;
    prev_pos.x = pos.x;
    prev_pos.y = pos.y;
    pos.x = xmotion->x;
    pos.y = xmotion->y;
    distx = pos.x - prev_pos.x;
    disty = prev_pos.y - pos.y;

    if (xg->is_spin_type.oblique) {
      /* 
       * Done by imitating some notes in Computer Graphics, Vol 22,
       * Number 4, August 1988, pp 127-128; SIGGRAPH '88
      */
      len_motion = (float)
        sqrt((double) pow((double) disty, (double) 2.0) +
           (double) pow((double) distx, (double) 2.0));
      if (len_motion != 0) {
        phi = len_motion / denom;

        ob_axis.x = -disty/len_motion;
        ob_axis.y = distx/len_motion;
        ob_axis.z = 0;

        store_Rmat(xg);
        cosphi = (float) cos((double) phi);
        sinphi = (float) sin((double) phi);
        /*
         * Calculate new rotation matrix.
        */
        cosm = 1.0 - cosphi;
        Rmat2[0][0] = (cosm * ob_axis.x * ob_axis.x) + cosphi;
        Rmat2[0][1] = (cosm * ob_axis.x * ob_axis.y) +
                (sinphi * ob_axis.z);
        Rmat2[0][2] = (cosm * ob_axis.x * ob_axis.z) -
                (sinphi * ob_axis.y);
        Rmat2[1][0] = (cosm * ob_axis.x * ob_axis.y) -
                (sinphi * ob_axis.z);
        Rmat2[1][1] = (cosm * ob_axis.y * ob_axis.y) + cosphi;
        Rmat2[1][2] = (cosm * ob_axis.y * ob_axis.z) +
                (sinphi * ob_axis.x);
        Rmat2[2][0] = (cosm * ob_axis.x * ob_axis.z) +
                (sinphi * ob_axis.y);
        Rmat2[2][1] = (cosm * ob_axis.y * ob_axis.z) -
                (sinphi * ob_axis.x);
        Rmat2[2][2] = (cosm * ob_axis.z * ob_axis.z) + cosphi;

        world_to_plane(xg);
        draw_ob_var_lines(xg);
      }
    } else if (xg->is_spin_type.yaxis) {
      xg->theta.yaxis = xg->theta.yaxis + distx/denom;

      xg->cost.y = (float) cos((double) xg->theta.yaxis);
      xg->sint.y = (float) sin((double) xg->theta.yaxis);
      xg->icost.y = (int) (xg->cost.y * PRECISION2);
      xg->isint.y = (int) (xg->sint.y * PRECISION2);
      if (xg->theta.yaxis > MAXTHETA)
           xg->theta.yaxis -= TWOPI;
      else if (xg->theta.yaxis < -MAXTHETA)
           xg->theta.yaxis += TWOPI;

      world_to_plane(xg);
      spin_var_lines(xg);
    } else if (xg->is_spin_type.xaxis) {
      xg->theta.xaxis = xg->theta.xaxis - disty/denom;

      xg->cost.x = (float) cos((double) xg->theta.xaxis);
      xg->sint.x = (float) sin((double) xg->theta.xaxis);
      xg->icost.x = (int) (xg->cost.x * PRECISION2);
      xg->isint.x = (int) (xg->sint.x * PRECISION2);
      if (xg->theta.xaxis > MAXTHETA)
           xg->theta.xaxis -= TWOPI;
      else if (xg->theta.xaxis < -MAXTHETA)
           xg->theta.xaxis += TWOPI;

      world_to_plane(xg);
      spin_var_lines(xg);
    }
    plane_to_screen(xg);
    plot_once(xg);
  } else if (evnt->type == ButtonRelease) {
    if (xg->is_spin_type.oblique) {
      store_Rmat(xg);
      xg->theta.oblique = xg->theta0;
    }
    start_spin_proc(xg);
  }
}


/*
 * Here's the code for writing the rotation coefficients that
 * produced this particular 2d projection,
 * followed by code for writing and reading the rotation matrix.
*/

#define SPIN_COEF(n,p) spin_coef[(2*(n)) + (p)]
/* ARGSUSED */
XtCallbackProc
spin_place_save_coefs_popup(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Determine where to place the popup window in which the user enters
 * a filename; invoked before spin_save.
*/
{
  /*
   * Shut off spinning until the file name is entered.
  */
  stop_spin_proc(xg);
  (void) strcpy(xg->save_type, SAVE_SPIN_COEFS );
  fname_popup(w, xg);
}

void
spin_save_coefs(Widget w, xgobidata *xg)
{
  char *filename;
  int j;
  long foo;
  float xfoo;
  char Spath[50];
  int x = xg->spin_vars.x, y = xg->spin_vars.y, z = xg->spin_vars.z;
  FILE *fp;

/*
 * spin_coef will contain the coefficients
 * of rotation:  The x coefficients will be in column 0 and
 * the y coefficients in column 1.
*/
  for (j=0; j<2*xg->ncols_used; j++)
    spin_coef[j] = 0.0;

  if (xg->is_spin_type.yaxis) {
    SPIN_COEF(x, 0) = xg->cost.y /
      (xg->lim[x].max - xg->lim[x].min);
    SPIN_COEF(z, 0) = xg->sint.y /
      (xg->lim[z].max - xg->lim[z].min);
    SPIN_COEF(y, 1) = 1.0;
  } else if (xg->is_spin_type.xaxis) {
    SPIN_COEF(x, 0) = 1.0;
    SPIN_COEF(y, 1) = xg->cost.x /
      (xg->lim[y].max - xg->lim[y].min);
    SPIN_COEF(z, 1) = xg->sint.x /
      (xg->lim[z].max - xg->lim[z].min);
  } else if (xg->is_spin_type.oblique) {
    SPIN_COEF(x, 0) = xg->Rmat0[0][0] /
      (xg->lim[x].max - xg->lim[x].min);
    SPIN_COEF(x, 1) = xg->Rmat0[1][0] /
      (xg->lim[x].max - xg->lim[x].min);
    SPIN_COEF(y, 0) = xg->Rmat0[0][1] /
      (xg->lim[y].max - xg->lim[y].min);
    SPIN_COEF(y, 1) = xg->Rmat0[1][1] /
      (xg->lim[y].max - xg->lim[y].min);
    SPIN_COEF(z, 0) = xg->Rmat0[0][2] /
      (xg->lim[z].max - xg->lim[z].min);
    SPIN_COEF(z, 1) = xg->Rmat0[1][2] /
      (xg->lim[z].max - xg->lim[z].min);
  }

  filename = XtMalloc(132 * sizeof(char));
  XtVaGetValues(w, XtNstring, (String) &filename, NULL);
/*
 * In S, just write them out as a vector, column 0 then column 1.
*/
  if (xg->data_mode == Sprocess) {
    (void) strcpy(Spath, Spath0);
    (void) strcat(Spath, filename);
    if ( (fp = fopen(Spath, "w")) == NULL) {
      char message[MSGLENGTH];
      sprintf(message, "Failed to open the file '%s' for writing.\n", Spath);
      show_message(message, xg);
    } else {
      /*
       * "1" indicates that the following is an
       * S data structure of one element.
      */
      (void) fprintf(fp, "%cS data%c", (char) 0 , (char) 1);
      /*
       * "3" indicates that the following is of type
       * single; "4" would imply numeric, or double.
      */
      foo = (long) 3;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "xgobi: error in writing coefficients\n");
      /*
       * "2*ncols_used" says the following has "2*ncols_used" elements."
      */
      foo = (long) (2*xg->ncols_used);
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "xgobi: error in writing coefficients\n");

      for (j=0; j<2*xg->ncols_used; j++) {
        xfoo = spin_coef[j];
        if (fwrite((char *) &xfoo, sizeof(xfoo), 1, fp) == 0)
          fprintf(stderr, "xgobi: error in writing coefficients\n");
      }
      if (fclose(fp) == EOF)
        fprintf(stderr, "xgobi: error in writing coefficients\n");
    }
  }
/*
 * In ascii, write out a two-column file.
*/
  else if (xg->data_mode == ascii || xg->data_mode == binary) { /* if not S */
    if ( (fp = fopen(filename, "w")) == NULL) {
      char message[MSGLENGTH];
      sprintf(message, "Failed to open the file '%s' for writing.\n", filename);
      show_message(message, xg);
    } else {
      for (j=0; j<xg->ncols_used; j++)
        (void) fprintf(fp, "%f %f\n",
          SPIN_COEF(j, 0), SPIN_COEF(j, 1));

      if (fclose(fp) == EOF)
        fprintf(stderr, "error in fclose in spin_save_coefs_cback\n");
    }
  }

  XtFree(filename);
/*
 * Restart interpolation or rotation after saving data.
*/
  start_spin_proc(xg);
}
#undef SPIN_COEF

/* ARGSUSED */
XtCallbackProc
spin_place_save_rmat_popup(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Determine where to place the popup window in which the user enters
 * a filename; invoked before spin_save.
*/
{
  /*
   * Shut off spinning until the file name is entered.
  */
  stop_spin_proc(xg);
  (void) strcpy(xg->save_type, SAVE_SPIN_RMAT);
  fname_popup(w, xg);
}

void
spin_save_rmat(Widget w, xgobidata *xg)
{
  char *filename;
  int j;
  FILE *fp;

  filename = XtMalloc(132 * sizeof(char));
  XtVaGetValues(w, XtNstring, (String) &filename, NULL);

/*
 * In S, don't do this ...
*/
  if (xg->data_mode == Sprocess) {
    ;
  }

/*
 * In ascii, write out a three-column file.
*/
  else if (xg->data_mode == ascii || xg->data_mode == binary) { /* if not S */
    if ( (fp = fopen(filename, "w")) == NULL) {
      char message[MSGLENGTH];
      sprintf(message, "Failed to open the file '%s' for writing.\n", filename);
      show_message(message, xg);
    } else {
      /*
       * In order that rotation restarts smoothly, it seems I have
       * to save an extraordinary amount of variables ...
      */
      for (j=0; j<3; j++)
        fprintf(fp, "%f %f %f\n",
          xg->Rmat0[j][0], xg->Rmat0[j][1], xg->Rmat0[j][2]);
      for (j=0; j<3; j++)
        fprintf(fp, "%f %f %f\n",
          Rmat1[j][0], Rmat1[j][1], Rmat1[j][2]);
      fprintf(fp, "%f\n", xg->theta.oblique);
      fprintf(fp, "%f %f %f\n", ob_axis.x, ob_axis.y, ob_axis.z);

      if (fclose(fp) == EOF)
        fprintf(stderr, "error in fclose in spin_save_rmat_cback\n");
    }
  }

/* XtFree(filename); */

/*
 * Restart interpolation or rotation after saving data.
*/
  start_spin_proc(xg);
}

/* ARGSUSED */
XtCallbackProc
spin_place_read_rmat_popup(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Determine where to place the popup window in which the user enters
 * a filename; invoked before spin_save.
*/
{
  /*
   * Shut off spinning ...
  */
  stop_spin_proc(xg);
  (void) strcpy(xg->save_type, READ_SPIN_RMAT);
  fname_popup(w, xg);
}

void
spin_read_rmat(Widget w, xgobidata *xg)
{
  char *filename;
  int j, k;
  int x = xg->spin_vars.x, y = xg->spin_vars.y, z = xg->spin_vars.z;
  FILE *fp;
  long nx, ny;

  filename = XtMalloc(132 * sizeof(char));
  XtVaGetValues(w, XtNstring, (String) &filename, NULL);

/*
 * No reading from S ...
*/
  if (xg->data_mode == Sprocess)
    return;

/*
 * In ascii, read in the two-column file.
*/
  else if (xg->data_mode == ascii || xg->data_mode == binary) { /* if not S */
    if ( (fp = fopen(filename, "r")) == NULL) {
      char message[MSGLENGTH];
      sprintf(message, "Failed to open the file '%s' for reading.\n", filename);
      show_message(message, xg);
    } else {

      for (j=0; j<3; j++) {
        for (k=0; k<3; k++) {
          if ( fscanf(fp, "%f\n", &xg->Rmat0[j][k]) < 0) {
            char msg[MSGLENGTH];
            sprintf(msg,
              "The rotation matrix file %s is smaller than 3x3\n", filename);
            show_message(msg, xg);
            break;
      } } }
      for (j=0; j<3; j++) {
        for (k=0; k<3; k++) {
          if ( fscanf(fp, "%f\n", &Rmat1[j][k]) < 0) {
            char msg[MSGLENGTH];
            sprintf(msg,
              "The rotation matrix file %s is smaller than 3x3\n", filename);
            show_message(msg, xg);
            break;
      } } }
      fscanf(fp, "%f\n", &xg->theta.oblique);
      fscanf(fp, "%f %f %f\n", &ob_axis.x, &ob_axis.y, &ob_axis.z);
            

      if (fclose(fp) == EOF)
        fprintf(stderr, "error in fclose in spin_read_rmat_cback\n");

    }
  }

  find_Rmat(xg);

  /* XtFree(filename); */

  /* run through pipeline and replot -- this is
   * the latter half of ob_rot_reproject; this is what
   * would happen if world_to_plane were called
  */

  /*
   * Convert rotation matrix to type long.
  */
  for (j=0; j<3; j++)
    for (k=0; k<2; k++)
      lRmat[k][j] = (long) (xg->Rmat0[k][j] * PRECISION2);

  /*
   * Find new position of the data.
  */
  for (k=0; k<xg->nrows_in_plot; k++) {
    j = xg->rows_in_plot[k];
    nx =
      lRmat[0][0] * xg->world_data[j][x] +
      lRmat[0][1] * xg->world_data[j][y] +
      lRmat[0][2] * xg->world_data[j][z];

    xg->planar[j].x = nx/PRECISION2;
    ny =
      lRmat[1][0] * xg->world_data[j][x] +
      lRmat[1][1] * xg->world_data[j][y] +
      lRmat[1][2] * xg->world_data[j][z];

    xg->planar[j].y = ny/PRECISION2;
  }

  plane_to_screen(xg);
  plot_once(xg);
  spin_var_lines(xg);

/*
 * In this case, don't restart rotation.  That can't possibly
 * be what the user would want.
*/

}

