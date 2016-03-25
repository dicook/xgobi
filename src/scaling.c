/* scaling.c */
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

#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

static int button1down, button2down;
 /* pointer position in scaling */
static icoords s_w, s_ow = {40,40};
static int nsteps;

void
init_scale_vars(xgobidata *xg)
{
  static Boolean firsttime = True;

  if (firsttime)
  {
    xg->is_scaling = False;
    xg->run_shift_proc = False;
    xg->run_scale_proc = False;

    xg->plot_square = appdata.plotSquare;
    xg->shift_wrld.x = xg->shift_wrld.y = 0;
    xg->is.x = xg->is.y = 0;
    xg->scale0.x = xg->scale0.y = DEF_SCALE;
    xg->scale.x = xg->scale.y = DEF_SCALE;
  }
}

void
find_plot_center(xgobidata *xg)
{
  xg->cntr.x = xg->mid.x + (int) ((xg->shift_wrld.x * xg->is.x) >> EXP1);
  xg->cntr.y = xg->mid.y - (int) ((xg->shift_wrld.y * xg->is.y) >> EXP1);
}

/*
 * Since shift_wrld depends on scale (through xg->is) and scale depends
 * on shift (through xg->cntr), then when scaling is going on, the
 * value of shift_wrld has to change, so that the values of is and
 * cntr can remain unchanged.
*/
void
tune_shift(xgobidata *xg, float prev_scale_x, float prev_scale_y)
{
  xg->shift_wrld.x = (int) (xg->shift_wrld.x * prev_scale_x / xg->scale.x);
  xg->shift_wrld.y = (int) (xg->shift_wrld.y * prev_scale_y / xg->scale.y);
}

void
reinit_nsteps()
{
  nsteps = 1;
}

void
scale_proc(xgobidata *xg)
{
  float scalefac, mult;
  /* These two should probably be defines */
  float low = 400;
  float high = 2000;
  float prev_scale_x = xg->scale.x;
  float prev_scale_y = xg->scale.y;

  /*
   * This makes the scaling amount get bigger the longer the
   * button is pressed, by an amount that depends on nrows_in_plot.
  */
  nsteps++ ;

  if (xg->nrows_in_plot <= low)
    scalefac = SCALE0 * .25 ;  /* small; ignore nsteps */
  else if (xg->nrows_in_plot > high)
    scalefac = SCALE0 * MIN(20., (float) nsteps) ;
  else
  {
    mult = ((float) xg->nrows_in_plot - low) / (high - low) ;
    scalefac = SCALE0 * MIN(20., (float) nsteps * mult);
  }

  switch(xg->scaling_btn)
  {
    case 0:
      xg->scale.y += scalefac;
      break;
    case 1:
      xg->scale.x += scalefac;
      xg->scale.y += scalefac;
      break;
    case 2:
      xg->scale.x += scalefac;
      break;
    case 5:
      xg->scale.x -= MIN(scalefac, xg->scale.x - SCALE_MIN);
      break;
     case 4:
      xg->scale.x -= MIN(scalefac, xg->scale.x - SCALE_MIN);
      xg->scale.y -= MIN(scalefac, xg->scale.y - SCALE_MIN);
      break;
    case 3:
      xg->scale.y -= MIN(scalefac, xg->scale.y - SCALE_MIN);
      break;
  }

  /*find_plot_center(xg);*/
  tune_shift(xg, prev_scale_x, prev_scale_y);
  plane_to_screen(xg);

  if (xg->is_xyplotting)
  {
    /* use minindex and ticks0 to set the axes */
    convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
    /* now rescale the ticks for the current x and y */
    convert_ticks(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
    extend_axes(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
  }

  else if (xg->is_plotting1d)
  {
    /* use minindex and ticks0 to set the axes */
    convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
    /* now rescale the ticks for the current y */
    convert_ticks(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
    extend_axes(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
  }

  plot_once(xg);
}

void
shift_proc(xgobidata *xg)
{
/*
 * Not using the increasing method that used for scaling, rather
 * the shift step remains the same.
*/
  float high = 2000;
  float sfac = (xg->nrows_in_plot > high) ? 2 : 1;

  switch(xg->scaling_btn) {

    case 3:  /* shift left */
      xg->shift_wrld.x -= (long)(sfac / (float)xg->is.x) * PRECISION1;
      break;

    case 1:   /* shift right */
      xg->shift_wrld.x += (long)(sfac / (float)xg->is.x) * PRECISION1;
      break;

    case 2:  /* shift down */
      xg->shift_wrld.y -= (long)(sfac / (float)xg->is.y) * PRECISION1;
      break;

    case 0:  /* shift up */
      xg->shift_wrld.y += (long)(sfac / (float)xg->is.y) * PRECISION1;
      break;
  }

  find_plot_center(xg);
  plane_to_screen(xg);

  if (xg->is_xyplotting)
  {
    /* use minindex and ticks0 to set the axes */
    convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
    /* now rescale the ticks for the current x and y */
    convert_ticks(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
    extend_axes(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
  }

  else if (xg->is_plotting1d)
  {
    /* use minindex and ticks0 to set the axes */
    convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
    convert_axes(&xg->ticks0, xg);
    /* now rescale the ticks for the current x and y */
    convert_ticks(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
    extend_axes(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
  }

  plot_once(xg);
}

/*
 * Event handler for button presses when in scaling mode.  Modeled
 * after brush_button in brush.c.
*/
/* ARGSUSED */
XtEventHandler
scale_button_event(Widget w, xgobidata *xg, XEvent *evnt)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;
  int root_x, root_y, win_x, win_y;
  unsigned int kb;
  Window root, child;

  if (xg->is_scaling) {
    switch (xbutton->type) {
    case ButtonPress:
      switch (xbutton->button) {

      case 1:
        button1down = 1;
        /* Set initial pointer position. */
        if (XQueryPointer(display, xg->plot_window, &root, &child,
          &root_x, &root_y, &win_x, &win_y, &kb))
        {
          s_ow.x = s_w.x = win_x;
          s_ow.y = s_w.y = win_y;
        }
        break;
      case 2:
        button2down = 1;
        /* Set initial pointer position. */
        if (XQueryPointer(display, xg->plot_window, &root, &child,
          &root_x, &root_y, &win_x, &win_y, &kb))
        {
          s_ow.x = s_w.x = win_x;
          s_ow.y = s_w.y = win_y;
        }
        break;
      }
    break;
    case ButtonRelease:
      switch (xbutton->button)
      {
      case 1:
        button1down = 0;
        break;
      case 2:
        button2down = 0;
        break;
      }
    break;
    }
  }
}

/*
 * Checks if the pointer has moved since the last time "pointer_moved"
 * was called.  It does this by checking the current pointer
 * coordinates against win.x, win.y which are initialized in
 * main().  Returns (win.x,win.y) as globals representing the current
 * position of the pointer.
*/

int
pointer_moved(icoords *pos, icoords *prev_pos, xgobidata *xg)
{
  int root_x, root_y, win_x, win_y;
  int moved = 0;
  unsigned int kb;
  Window root, child;

  if ( XQueryPointer(display, xg->plot_window, &root, &child,
    &root_x, &root_y, &win_x, &win_y, &kb) &&
    (win_x != pos->x  || win_y != pos->y) )
  {
    prev_pos->x = pos->x;
    prev_pos->y = pos->y;
    pos->x = win_x;
    pos->y = win_y;
    moved = 1;
  }
  return(moved);
}

/*
 * Button 1 is pressed.  We are in scaling mode.  The mouse has moved
 * to s_w.x, s_w.y from s_ow.x, s_ow.y.  Change shift_wrld appropriately
*/
void
move_by_mouse(xgobidata *xg)
{
  float tmpx = FLOAT((s_w.x-s_ow.x) * PRECISION1) ;
  float tmpy = FLOAT((s_w.y-s_ow.y) * PRECISION1) ;

  xg->shift_wrld.x += INT(tmpx / (float) xg->is.x);
  xg->shift_wrld.y -= INT(tmpy / (float) xg->is.y);
}

/*
 * Button 2 is pressed.  We are in scaling mode.  The mouse has moved
 * to s_w.x, s_w.y from s_ow.x, s_ow.y and the center of the
 * figure is at mid.x, mid.y.  Change scale.x and scale.y by the
 * appropriate amounts.
*/
void
scale_by_mouse(xgobidata *xg)
{
  float prev_scale_x = xg->scale.x;
  float prev_scale_y = xg->scale.y;

  /*
   * Scale the scaler if far enough from center -- for the moment,
   * let's be arbitrary -- 5 pixels from the crosshair?
  */

  if (s_ow.x - xg->cntr.x > 5 || xg->cntr.x - s_ow.x > 5)
    xg->scale.x *= FLOAT(s_w.x - xg->cntr.x) / FLOAT(s_ow.x - xg->cntr.x);

  if (s_ow.y - xg->cntr.y > 5 || xg->cntr.y - s_ow.y > 5)
    xg->scale.y *= FLOAT(s_w.y - xg->cntr.y) / FLOAT(s_ow.y - xg->cntr.y);

  /* Restore if too small. */
  xg->scale.x = MAX(SCALE_MIN, xg->scale.x);
  xg->scale.y = MAX(SCALE_MIN, xg->scale.y);

  /* Adjust the value of shift_wrld as required to keep cntr unchanged */
  tune_shift(xg, prev_scale_x, prev_scale_y);
}

/*
 * Procedure executed for interactive scaling and translating.  ML.
 * Modeled after brush.c.  If a button is pressed and the pointer has
 * moved, scale or translate as appropriate and replot.
*/
void
scaling_proc(xgobidata *xg)
{
  if (button1down || button2down) {
    if (pointer_moved(&s_w, &s_ow, xg)) {
      if (button1down)
        move_by_mouse(xg);
      if (button2down)
        scale_by_mouse(xg);
      find_plot_center(xg);
      plane_to_screen(xg);

      if (xg->is_xyplotting) {
        /* use minindex and ticks0 to set the axes */
        convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
        convert_axes(&xg->ticks0, xg);
        /* now rescale the ticks for the current x and y */
        convert_ticks(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
        extend_axes(xg->xy_vars.x, xg->xy_vars.y, &xg->ticks, xg);
      }

      else if (xg->is_plotting1d) {
        /* use minindex and ticks0 to set the axes */
        convert_ticks(xg->minindex, xg->minindex, &xg->ticks0, xg);
        convert_axes(&xg->ticks0, xg);
        /* now rescale the ticks for the current x and y */
        convert_ticks(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
        extend_axes(xg->plot1d_vars.x, xg->plot1d_vars.y, &xg->ticks, xg);
      }

      plot_once(xg);
    }
  }
}

void
draw_scaling_crosshair(xgobidata *xg) {
  XDrawLine(display, xg->plot_window, copy_GC,
    0, xg->cntr.y, xg->plotsize.width, xg->cntr.y);
  XDrawLine(display, xg->plot_window, copy_GC,
    xg->cntr.x, 0, xg->cntr.x, xg->plotsize.height);
}
