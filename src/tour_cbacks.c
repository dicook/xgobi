/* tour_cbacks.c */
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
#include <stdlib.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

/* external variables */
extern Widget tour_interp_menu_btn[3];
extern int tour_interp_btn;
extern int got_new_basis;

#define OWN_TOUR_SELECTION XtOwnSelection( (Widget) xg->workspace, \
  (Atom) XG_NEWTOUR, (Time) CurrentTime, \
  (XtConvertSelectionProc) pack_tour_data, \
  (XtLoseSelectionProc) pack_tour_lose , \
  (XtSelectionDoneProc) pack_tour_done )
/* Functions used in this file */
XtConvertSelectionProc pack_tour_data() ;
XtSelectionDoneProc pack_tour_done() ;
XtLoseSelectionProc pack_tour_lose() ;

void
start_tour_proc(xgobidata *xg)
{
  if (!xg->is_tour_paused &&
      xg->tour_link_state != receive &&
      xg->step != 0)
/* Can't include xg->is_stepping check here because then
 * Go doesn't restart tour. So there will be a problem if leave
 * tour to use scale or brush, for eg, when return stepping will
 * not be on, although the button is highlighted. Turning Step on
 * and then off again fixes it.
*/
  {
    xg->run_tour_proc = True;
    (void) XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
  }

  OWN_TOUR_SELECTION ;  /* Do this even when not linked to save trouble */
}

void
stop_tour_proc(xgobidata *xg)
{
  xg->run_tour_proc = False;
}

void
set_tour_speed(float slidepos, xgobidata *xg)
{
/*
 * If the slider is near the start of its range, set step = 0.
 * and stop the touring.
*/
  if (slidepos < .05)
  {
    stop_tour_proc(xg);
    xg->step = 0.0;
  }
  else
  {
    if (!xg->is_tour_paused && xg->tour_link_state != receive)
      start_tour_proc(xg);

    /*
     * To cause tour to speed up wildly at the right of the
     * scrollbar range.
    */
    if (slidepos < 0.8)
      xg->step = (slidepos - .05) / 20. ;
    else if ((slidepos >= 0.8) && (slidepos < 0.9))
      xg->step = pow((double)(slidepos-0.8),(double)0.90) + 0.0375;
    else
      xg->step = sqrt((double)(slidepos-0.8)) + 0.0375;
  }
  xg->delta = xg->step/xg->dv;
}

/* ARGSUSED */
XtCallbackProc
tour_speed_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  float slidepos = * (float *) slideposp;
  set_tour_speed(slidepos, xg);
}

/* ARGSUSED */
XtCallbackProc
tour_pause_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (!xg->is_tour_paused)
  {
    xg->is_tour_paused = True;
    stop_tour_proc(xg);
    possibly_draw_bandwidth(xg);
  }
  else
  {
    if (xg->numvars_t > 1)
    {/* pause is called in tour proc now when variables faded
        to 2, or when returning to bitmap. in the first case
        the tour proc shouldn't be turned on until more variables
        are added, but in the second case it should
      */
      xg->is_tour_paused = False;
      start_tour_proc(xg);
    }
  }
  setToggleBitmap(w, xg->is_tour_paused);
}

void
desens_everything(xgobidata *xg)
{
  /*
   * Desensitize the local scan, reinit and principal components commands.
  */
  set_sens_princ_comp(xg, 0);
  set_sens_localscan(false);
  set_sens_reinit(false);
  set_sens_link_menu(0);
  if (xg->is_princ_comp)
    reset_princ_comp(True, xg); /* this needs to be done because
                                 * desensitizing turns off the
                                 * highlighting. */
  /*
   * if princ comp off then desensitize projection pursuit button.
  */
  if (!xg->is_princ_comp)
    set_sens_pp_btn(xg, 0);
  if (xg->is_pp_optimz)
    turn_off_optimz(xg);
  if (xg->is_pp) /* desensitize optimize during backtrack */
    set_sens_optimz(0);
}

void
resens_everything(xgobidata *xg)
{
  /*
   * Resensitize the local scan, reinit and principal components.
  */
  set_sens_localscan(true);
  set_sens_reinit(true);
  set_sens_princ_comp(xg, 1);
  set_sens_link_menu(1);
  /*
   * if princ comp off then resensitize projection pursuit button.
  */
  if (!xg->is_princ_comp)
    set_sens_pp_btn(xg, 1);
  if (xg->is_pp) /* resensitize optimize when turn off backtracking */
    set_sens_optimz(1);
}

/* ARGSUSED */
XtCallbackProc
tour_backtrack_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This callback invokes scanning back through the history list.
*/
{
  Boolean turn_it_on = False;

  if (!xg->is_backtracking)
  {
    if (xg->tour_hist_just_read)
    {
      xg->backtrack_dir = FORWARD;
      reset_cycleback_cmd(true, true, "F");
      /*
       * Break circular list and make end pointers NULL
      */
      xg->hend = xg->curr->prev;
      xg->hend->next = NULL;
      xg->hfirst->prev = NULL;

      turn_it_on = True;
      xg->tour_hist_just_read = False;
    }
    else
    {
      if ((xg->nhist_list > 2) ||
        (!check_proximity(xg->u, xg->u0, xg->ncols_used) &&
        xg->nhist_list == 2))
      {
        xg->backtrack_dir = BACKWARD;
        reset_cycleback_cmd(true, true, "B");
        /*
         * Store current position if not arbitrarily close to
         * last entry in history list.
        */
        if (!check_proximity(xg->u, xg->u0, xg->ncols_used))
        {
          init_basis(xg);
          if (xg->is_store_history)
            store_basis(xg, xg->u0);
        }

        if ((xg->nhist_list < MAXHIST) ||
          ((xg->nhist_list == MAXHIST) &&
          (xg->old_nhist_list == (MAXHIST-1))))
        {
          xg->nhist_list--;
        }
      /*
       * Break circular list and make end pointers NULL
      */
        xg->hend = xg->curr;
        xg->curr->next = NULL;
        xg->hfirst->prev = NULL;

        zero_tau(xg);
        turn_it_on = True;
      }
    }
    if (turn_it_on)
    {
      desens_everything(xg);
      tour_event_handlers(xg, 0);
      XtUnmapWidget(xg->tour_mouse);

      if (xg->is_stepping)
      {
        set_ready_to_stop_now(1);
        set_counting_to_stop(0);
      }
      xg->is_backtracking = True;
      set_varsel_label(xg);
      turn_it_on = False;
    }
    else
    {
      char message[MSGLENGTH];
      sprintf(message,
       "Can't start backtrack until more than %d basis in the history list.\n",
       xg->nhist_list);
      show_message(message, xg);
      highlight_backtrack_cmd();
    }
  }
  else /* is_backtracking, so turn it off */
  {
    if (xg->backtrack_dir == FORWARD)
    {
      /*
       * If not at end of curr list save previous base,
       * break links and load remaining bases onto the free list.
      */
      if (xg->curr->prev != NULL)
      {
        xg->hend = xg->curr->prev;
        xg->fl = xg->curr;
        xg->fl->prev = NULL;
        while (xg->fl->next != NULL)
          xg->fl = xg->fl->next;
      }
    }
    else
    {
      /*
       * If not at end of curr list save current base,
       * break links and load remaining bases onto the free list.
      */
      if (xg->curr->next != NULL)
      {
        xg->hend = xg->curr;
        nback_update_label(xg);
        xg->curr = xg->curr->next;
        xg->fl = xg->curr;
        xg->fl->prev = NULL;
        while (xg->fl->next != NULL)
          xg->fl = xg->fl->next;
      }
      xg->nhist_list++;
    }
    /* ensure fl ends have NULL pointers */
    if (xg->fl != NULL)
      xg->fl->next = NULL;
    xg->curr = xg->hend;
    /*
     * re-form circular links
    */
    xg->curr->next = xg->hfirst;
    xg->hfirst->prev = xg->curr;
    xg->old_nhist_list = xg->nhist_list - 1;
    xg->max_nhist_list = MAXHIST;
    zero_tau(xg);
    reset_cycleback_cmd(true, false, "F");

    resens_everything(xg);
    tour_event_handlers(xg, 1);
    XtMapWidget(xg->tour_mouse);

    if (xg->is_stepping)
    {
      set_ready_to_stop_now(1);
      set_counting_to_stop(0);
    }
    xg->is_backtracking = False;
    set_varsel_label(xg);
  }

  setToggleBitmap(w, xg->is_backtracking);
}

/* ARGSUSED */
XtCallbackProc
tour_cycleback_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This routine allows direction changes during backtracking
*/
{
  int do_it = 1;

  if (xg->is_backtracking)
  {
    if (xg->backtrack_dir == BACKWARD)
    {
      /*
       * If at the end of the list, ignore user request.
      */
      if (!xg->curr->next)
      {
        char message[MSGLENGTH];
        do_it = 0;
        sprintf(message, "Can\'t go forward; at end of list.\n");
        show_message(message, xg);
      }
      else
      {
        reset_cycleback_cmd(false, false, "F");
        xg->backtrack_dir = FORWARD;
        if (!xg->is_stepping)
          xg->curr = xg->curr->next;
      }
    }
    else if (xg->backtrack_dir == FORWARD)
    {
      /*
       * If at the end of the list, ignore user request.
      */
      if (!xg->curr->prev)
      {
        char message[MSGLENGTH];
        do_it = 0;
        sprintf(message, "Can\'t go backward; at end of list.\n");
        show_message(message, xg);
      }
      else
      {
        reset_cycleback_cmd(false, false, "B");
        xg->backtrack_dir = BACKWARD;
        if (!xg->is_stepping)
          xg->curr = xg->curr->prev;
      }
    }

    if (do_it)
    {
      init_basis(xg);
      if (!xg->is_stepping)
        xg->nhist_list += xg->backtrack_dir;
      retrieve_basis(xg);
      /*
       * Sets specific behaviour in tour_proc
      */
      got_new_basis = 1;
      if (xg->is_stepping)
      {
        set_ready_to_stop_now(1);
        set_counting_to_stop(0);
      }
      zero_tau(xg);
    }
  }
}

/* ARGSUSED */
XtCallbackProc
tourhist_on_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This routine allows direction changes during backtracking
*/
{
  xg->is_store_history = !xg->is_store_history;

  setToggleBitmap(w, xg->is_store_history);
}

/* ARGSUSED */
XtCallbackProc
tour_storbas_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This routine allows direction changes during backtracking
*/
{
  store_basis(xg, xg->u);
}

/* ARGSUSED */
XtCallbackProc
tour_step_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This routine invokes step control during touring
*/
{
  if (!xg->is_stepping)
  {
    xg->is_stepping = True;
    set_sens_go(true);
    /*
     * If paused or step = 0.0, set this to force the tour
     * to go forward when pause is turned off or speed increased.
    */
    set_ready_to_stop_now(1);
    set_counting_to_stop(0);
  }
  else
  {
    xg->is_stepping = False;
    set_sens_go(false);
    set_counting_to_stop(0);
    set_ready_to_stop_now(0);
    start_tour_proc(xg);
  }
  setToggleBitmap(w, xg->is_stepping);
}

/* ARGSUSED */
XtCallbackProc
tour_step_go_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This routine is used during step control of tour to restart tour_proc
*/
{
  set_counting_to_stop(1);
  set_ready_to_stop_now(0);
  start_tour_proc(xg);
}

/* ARGSUSED */
XtCallbackProc
tour_local_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This routine invokes local scanning. That is setting the current position
 * to be an anchor basis, and sequentially seaching randomly around it.
 * u0-u1-u0-u2-u0-u3-........
*/
{
  if (!xg->is_local_scan) {
    xg->is_local_scan = True;
    set_local_scan_dir_in(xg);
    zero_tau(xg);
    /*
     * Desensitize backtrack, reinit, principal components and the
     * interpolation menu.
    */
    reset_backtrack_cmd(false, false, false, false);
    set_sens_reinit(false);
    set_sens_princ_comp(xg, 0);
    reset_interp_cmd(0);
    tour_event_handlers(xg, 0);
    XtUnmapWidget(xg->tour_mouse);
    if (xg->is_princ_comp)
      reset_princ_comp(True, xg); /* this needs to be done because
                                   * desensitizing turns off the
                                   * highlighting. */
    /*
     * if princ comp off then desensitize projection pursuit button.
    */
    if (!xg->is_princ_comp)
      set_sens_pp_btn(xg, 0);
    if (xg->is_pp_optimz)
      turn_off_optimz(xg);
    if (xg->is_pp)
      set_sens_optimz(0);
    if (xg->is_stepping)
    {
      set_ready_to_stop_now(1);
      set_counting_to_stop(0);
    }
  }
  else
  {
    xg->is_local_scan = False;
    zero_tau(xg);
    /*
     * Resensitize backtrack, reinit and principal components
     * and the interpolation menu.
    */
    reset_backtrack_cmd(false, false, true, false);
    set_sens_reinit(true);
    set_sens_princ_comp(xg, 1);
    reset_interp_cmd(1);
    tour_event_handlers(xg, 1);
    XtMapWidget(xg->tour_mouse);
    /*
     * if princ comp off then resensitize projection pursuit button.
    */
    if (!xg->is_princ_comp)
      set_sens_pp_btn(xg, 1);
    if (xg->is_pp)
      set_sens_optimz(1);
    if (xg->is_stepping)
    {
      set_ready_to_stop_now(1);
      set_counting_to_stop(0);
    }
  }
  set_varsel_label(xg);

  setToggleBitmap(w, xg->is_local_scan);
}

/* ARGSUSED */
XtCallbackProc
tour_reinit_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This callback invokes scanning back through the history list.
*/
{
  int i, j;

/* set u0 to be the first two variables, and zero taus */
  for (i=0; i<2; i++)
    for (j=0; j<xg->ncols_used; j++)
      xg->u0[i][j] = 0.;
  xg->u0[0][xg->tour_vars[0]] = 1.0;
  xg->u0[1][xg->tour_vars[1]] = 1.0;
  for (i=0; i<2; i++)
    for (j=0; j<xg->ncols_used; j++)
      xg->u[i][j] = xg->u0[i][j];
  for (i=0; i<2; i++)
    for (j=0; j<xg->ncols_used; j++)
      xg->u1[i][j] = xg->u0[i][j];
  init_V(xg);

  zero_tau(xg);
  zero_princ_angles(xg);
  /*
   * Reset tour history linked list.
  */
  xg->nhist_list = 2;
  reinit_tour_hist(xg);
  reset_backtrack_cmd(false, false, false, false);
  (void)set_bt_firsttime();
  nback_update_label(xg);

  if (xg->is_pp_optimz || xg->tour_cont_fact == infinite)
    xg->new_direction_flag = True;
  span_planes(xg);
  world_to_plane(xg);
  plane_to_screen(xg);
  plot_once(xg);
  tour_var_lines(xg);
  if (xg->is_pp)
  {
    xg->recalc_max_min = True;
    reset_pp_plot();
    pp_index(xg,0,1);
  }
}

void
tour_save_coefs(Widget w, xgobidata *xg)
{
  FILE *fp;
  char *filename;
  int j, k, m;
  long foo;
  float xfoo;
  char Spath[50];
  char message[MSGLENGTH];

  filename = XtMalloc(132 * sizeof(char));
  XtVaGetValues(w, XtNstring, &filename, NULL);

/*
 * In S, just write them out as a vector, column 0 then column 1.
*/
  if (xg->data_mode == Sprocess)
  {
    (void) strcpy(Spath, Spath0);
    (void) strcat(Spath, filename);
    if ( (fp = fopen(Spath, "w")) == NULL)
    {
      sprintf(message,
        "Failed to open the file '%s' for writing.\n", Spath);
      show_message(message, xg);
    }
    else
    {
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
        fprintf(stderr, "error in tour_savecoefs_cback");
      /*
       * "2*ncols" says the following has "2*ncols" elements."
       * "2*numvars_t" says the following has "2*numvars_t" elements."
      */
      if (xg->is_princ_comp)
        foo = (long) 2*xg->numvars_t;
      else
        foo = (long) 2*xg->ncols_used;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "error in tour_savecoefs_cback");

      if (xg->is_princ_comp)
      {
        sprintf(message,
          "Printing coefficients for active variables of sphered data only.\n");
        strcat(message,
          "In order to use these, you need also to save sphered data.\n");
        strcat(message,
          "(See Options panel.)\n");
        show_message(message, xg);
        for (j=0; j<xg->numvars_t; j++)
        {
          for (k=0; k<2; k++)
          {
            m = xg->tour_vars[j];
            xfoo = xg->u[k][m] / (xg->lim[m].max - xg->lim[m].min);
            if (fwrite((char *) &xfoo, sizeof(xfoo), 1, fp) == 0)
              fprintf(stderr, "error in tour_savecoefs_cback");
          }
        }
      }
      else
      {
        for (j=0; j<xg->ncols_used; j++)
        {
          for (k=0; k<2; k++)
          {
            xfoo = xg->u[k][j] / (xg->lim[j].max - xg->lim[j].min);
            if (fwrite((char *) &xfoo, sizeof(xfoo), 1, fp) == 0)
              fprintf(stderr, "error in tour_savecoefs_cback");
          }
        }
      }
      if (fclose(fp) == EOF)
        fprintf(stderr, "tour_savecoefs_cback");
    }
  }
/*
 * In ascii, write out a two-column file.
*/
  else if (xg->data_mode == ascii || xg->data_mode == binary) /* if not S */
  {
    if ( (fp = fopen(filename, "w")) == NULL)
    {
      sprintf(message,
        "Failed to open the file  '%s' for writing.\n", filename);
      show_message(message, xg);
    }
    else
    {
      if (xg->is_princ_comp)
      {
        sprintf(message,
          "Printing coefficients for active variables of sphered data only.\n");
        strcat(message,
          "In order to use these, you need also to save sphered data.\n");
        strcat(message,
          "(See Options panel.)\n");
        show_message(message, xg);

        for (j=0; j<xg->numvars_t; j++)
        {
          m = xg->tour_vars[j];
          (void) fprintf(fp, "%f %f\n",
            xg->u[0][m] / (xg->lim[m].max - xg->lim[m].min),
            xg->u[1][m] / (xg->lim[m].max - xg->lim[m].min));
        }
      }
      else
      {
        for (j=0; j<xg->ncols_used; j++)
          (void) fprintf(fp, "%f %f\n",
            xg->u[0][j] / (xg->lim[j].max - xg->lim[j].min),
            xg->u[1][j] / (xg->lim[j].max - xg->lim[j].min));
      }
      if (fclose(fp) == EOF)
        fprintf(stderr, "tour_savecoefs_cback");
    }
  }

  XtFree(filename);

/*
 * Restart interpolation or rotation after saving data.
*/
  start_tour_proc(xg);
}

void
tour_save_hist(Widget w, xgobidata *xg)
{
  FILE *fp;
  char *filename;
  int i, j, k;
  long foo;
  float xfoo;
  char Spath[50];
  hist_rec *base;
  int nhist;

  if (xg->is_backtracking)
    nhist = xg->max_nhist_list;
  else
    nhist = xg->nhist_list;
/*
 * Build an array, ncols_used by 2, that will contain the coefficients
 * of rotation:  The x coefficients will be in column 0 and
 * the y coefficients in the column 1.
*/

  filename = XtMalloc(132 * sizeof(char));
  XtVaGetValues(w, XtNstring, &filename, NULL);

/*
 * In S, just write them out as a vector, column 0 then column 1.
*/
  if (xg->data_mode == Sprocess)
  {
    (void) strcpy(Spath, Spath0);
    (void) strcat(Spath, filename);
    if ( (fp = fopen(Spath, "w")) == NULL)
      fprintf(stderr, "tour_savehist_cback");
    else
    {
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
        fprintf(stderr, "tour_savehist_cback");
      /*
       * "2*ncols_used" says the following has "2*ncols_used" elements."
      */
      foo = (long) 2*xg->ncols_used;
      if (fwrite((char *) &foo, sizeof(foo), 1, fp) == 0)
        fprintf(stderr, "tour_savehist_cback");

      k = 1;
      base = xg->hfirst;
      j = 0;
      while ((base->next != NULL) && (j <= nhist))
      {
        xfoo = k;
        if (fwrite((char *) &xfoo, sizeof(xfoo), 1, fp) == 0)
          fprintf(stderr, "tour_savehist_cback");

        for (j=0; j<2; j++)
        {
          for (i=0; i<xg->ncols_used; i++)
          {
            xfoo = base->hist[j][i];
            if (fwrite((char *) &xfoo, sizeof(xfoo), 1, fp) == 0)
              fprintf(stderr, "tour_savehist_cback");
          }
        }
        base = base->next;
        k++;
      }
      if (fclose(fp) == EOF)
        fprintf(stderr, "tour_savehist_cback");
    }
  }
/*
 * In ascii, write out a two-column file.
*/
  else if (xg->data_mode == ascii || xg->data_mode == binary) /* if not S */
  {
    if ( (fp = fopen(filename, "w")) == NULL)
    {
      char message[MSGLENGTH];
      sprintf(message,
        "Failed to open the file  '%s' for writing.\n", filename);
      show_message(message, xg);
    }
    else
    {
      j = 1;
      base = xg->hfirst;
      while ((base->next != NULL) && (j <= nhist))
      {
        (void) fprintf(fp,"%d\n",j);
        for (i=0; i<xg->ncols_used; i++)
        {
          (void) fprintf(fp, "%f %f\n",
                   base->hist[0][i], base->hist[1][i]);
        }
        base = base->next;
        j++;
      }
      if (fclose(fp) == EOF)
        (void) fprintf(stderr, "tour_savehist_cback: fclose failed\n");
    }
  }

  XtFree(filename);

/*
 * Restart interpolation or rotation after saving data.
*/
  start_tour_proc(xg);
}

void
tour_read_hist(Widget w, xgobidata *xg)
{
  char *filename;
  int j, k;
  char str[100], newline[100];
  int istr, ostr = -1;
  float tmpf1, tmpf2;
  FILE *fp;

  filename = XtMalloc(132 * sizeof(char));
  XtVaGetValues(w, XtNstring, (String) &filename, NULL);

  if ( (fp = fopen(filename, "r")) == NULL)
  {
      char message[MSGLENGTH];
      sprintf(message,
        "Failed to open the file '%s' for reading.\n", filename);
      show_message(message, xg);
  }
  else
  {
    if (xg->is_backtracking)
      reset_backtrack_cmd(true, false, true, true);
        /* just to make it easier to think about what needs to be
           done with the linked list of bases */
    k = 0;
    if (fgets(str, 60, fp) == NULL)
      fprintf(stderr, "error in tour_read_hist");
    else
    {
      istr = atoi(str);
      xg->nhist_list = 0;
      reinit_tour_hist(xg);
      /* ostr should have some initial value ?? */
      while ((istr > 0) && (istr != ostr))
      {
        k++;
        for (j=0; j<xg->ncols_used; j++)
        {
          (void) fgets(newline, 60, fp);
          (void) sscanf(newline,"%f %f", &tmpf1, &tmpf2);
          xg->u0[0][j] = tmpf1;
          xg->u0[1][j] = tmpf2;
        }
        if (xg->is_store_history)
          store_basis(xg, xg->u0); /* this sets up circular link,
          and increments nhist_list */
        if (k == 1)
          xg->hfirst = xg->curr;
        ostr = istr;
        (void) fgets(str,60,fp);
        istr = atoi(str);
      }

      if (xg->nhist_list > 1)
      {
        xg->max_nhist_list = xg->nhist_list;

/* set the first basis in the list to be curr and set u0 to this */
        xg->curr = xg->hfirst;
        for (k=0; k<2; k++)
          for (j=0; j<xg->ncols_used; j++)
            xg->u0[k][j]= xg->u[k][j] = xg->v0[k][j] = xg->curr->hist[k][j];
        xg->nhist_list = 1;

        zero_tau(xg);
        span_planes(xg);
        world_to_plane(xg);
        plane_to_screen(xg);
        plot_once(xg);
        tour_var_lines(xg);
        XFlush(display);
        XSync(display, False);

        xg->tour_hist_just_read = 1;
        reset_backtrack_cmd(true, true, true, true);

        if (xg->is_pp)
        {
          xg->recalc_max_min = True;
          reset_pp_plot();
          pp_index(xg,0,1);
        }
      }
      else
      {
        char message[MSGLENGTH];
        sprintf(message, "History file has only one record.\n");
        show_message(message, xg);
      }

      if (fclose(fp) == EOF)
        (void) fprintf(stderr, "tour_readhist_cback: fclose failed\n");
    }
  }

  XtFree(filename);

/*
 * Restart interpolation or rotation after saving data.
*/
  start_tour_proc(xg);
}

/* ARGSUSED */
XtCallbackProc
choose_tour_io_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k;
  int io_btn;

  for (k=0; k<3; k++)
  {
    if (xg->tour_io_menu_btn[k] == w)
    {
      io_btn = k;
      break;
    }
  }
  stop_tour_proc(xg);

  switch(io_btn)
  {
    case 0:
      (void) strcpy(xg->save_type, SAVE_TOUR_COEFS );
      break;
    case 1:
      (void) strcpy(xg->save_type, SAVE_TOUR_HIST );
      break;
    case 2:
      (void) strcpy(xg->save_type, READ_TOUR_HIST );
      break;
   }

  fname_popup(XtParent(w), xg);
}

/* ARGSUSED */
XtCallbackProc
choose_tour_interp_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k;

  stop_tour_proc(xg);
  XtVaSetValues(tour_interp_menu_btn[tour_interp_btn],
    XtNleftBitmap, None, NULL);
  for (k=0; k<3; k++)
  {
    if (tour_interp_menu_btn[k] == w)
    {
      tour_interp_btn = k;
      break;
    }
  }
  XtVaSetValues(tour_interp_menu_btn[tour_interp_btn],
    XtNleftBitmap, menu_mark, NULL);
  if (!check_proximity(xg->u0,xg->u, xg->ncols)) {
    copy_basis(xg->u, xg->u0, xg->ncols);
    if (xg->is_store_history)
      store_basis(xg, xg->u0);
  }
  zero_tau(xg);
  start_tour_proc(xg);

}

/* ARGSUSED */
XtCallbackProc
set_tour_link_state_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int k;
  extern Widget link_menu_cmd;
  extern Widget link_menu_btn[];
  extern char *link_menu_name[];
  int link_menu_id;
  enum link_state prev_tour_link_state = xg->tour_link_state;

  for (k=0; k<3; k++)
  {
    if (w == link_menu_btn[k])
    {
      link_menu_id = k;
      break;
    }
  }
  
  XtVaSetValues(link_menu_cmd,
    XtNlabel, link_menu_name[link_menu_id],
    NULL);

  if (link_menu_id == 0)
  {
    xg->tour_link_state = send_state;
    set_sens_tour_update(1);
  }
  else if (link_menu_id == 1)
  {
    xg->tour_link_state = receive;
    set_sens_tour_update(0);
  }
  else if (link_menu_id == 2)
  {
    xg->tour_link_state = unlinked;
    set_sens_tour_update(0);
  }

  /* If we were receiving, and we're not any more, then turn off receiving */
  if (prev_tour_link_state == receive && xg->tour_link_state != receive)
  {
    init_basis(xg);   /* copies u to u0 */
    zero_tau(xg);
    set_sens_speed(true);
    set_sens_reinit(true);
    set_sens_step(true);
    set_sens_localscan(true);
    reset_backtrack_cmd(false, false, true, false);
    set_sens_interp(true);
    set_sens_io(xg, 1, 1, 1);
    if (xg->is_pp)
      set_sens_optimz(1);
    start_tour_proc(xg);
  }

  /* If we weren't linked, and we are now, nothing needs to be done */
/* Huh, maybe not: dfs */
  if (prev_tour_link_state != send_state && xg->tour_link_state == send_state)
  {
    if (xg->tour_senddata == (unsigned long *) NULL)
      xg->tour_senddata = (unsigned long *)
        XtRealloc((XtPointer) xg->tour_senddata,
        (Cardinal) (4 + 5*xg->ncols) * sizeof(long));
  }

  /* If we were sending, and we're not now, we need to disown the selection */
  if (prev_tour_link_state == send_state && xg->tour_link_state == unlinked)
    XtDisownSelection( (Widget) xg->workspace,
      (Atom) XG_NEWTOUR,
      (Time) XtLastTimestampProcessed(display));

  /* If we were not receiving, and we are now, set up to receive */
  if (prev_tour_link_state != receive && xg->tour_link_state == receive)
  {
    stop_tour_proc(xg);
    set_sens_speed(false);
    set_sens_reinit(false);
    if (xg->is_stepping)
    {
      turn_off_stepping();
      set_sens_go(false);
    }
    set_sens_step(false);
    if (xg->is_local_scan)
      turn_off_local_scan(xg);
    set_sens_localscan(false);
    reset_backtrack_cmd(xg->is_backtracking, xg->is_backtracking,
      false, xg->is_backtracking);
    if (xg->is_backtracking)
      set_sens_direction(False);
    set_sens_interp(false);
    set_sens_io(xg, 1, 1, 0);
    if (xg->is_pp_optimz)
      turn_off_optimz(xg);
    set_sens_optimz(0);
  }

  /*
   * state changes from 'send' to 'unlink' should have no effect on the
   * variable selection panel, only changes in and out of the 'receive'
   * state.
  */
  if ((prev_tour_link_state == receive && xg->tour_link_state != receive) ||
      (prev_tour_link_state != receive && xg->tour_link_state == receive))
  {
    set_varpanel_for_receive_tour(xg);
  }
}

/* ARGSUSED */
XtCallbackProc
tour_update_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (xg->tour_link_state == send_state) {
    xg->new_basis_ind = False;
    OWN_TOUR_SELECTION ;
    announce_tour_coefs(xg);
  }
}

void
grand_tour_on(xgobidata *xg)
{
  int j, k;

/*
 *  If this mode is currently selected, turn it off.
*/
  if (xg->prev_plot_mode == GTOUR_MODE && xg->plot_mode != GTOUR_MODE)
  {
    xg->run_tour_proc = False;
    tour_event_handlers(xg, 0);
    if (xg->is_pp)
      map_tour_pp_panel(False);
    else if (xg->is_tour_section)
      map_section_panel(False);
    map_tour_panel(xg, False);

    /* Free the vector used for linked touring */
    if (xg->tour_senddata != (unsigned long *) NULL) {
      XtFree((char *) xg->tour_senddata);
      xg->tour_senddata = (unsigned long *) NULL;
    }
  }
  /* Else turn it on */
  else if (xg->prev_plot_mode != GTOUR_MODE &&
           xg->plot_mode == GTOUR_MODE)
  {
    if (!xg->is_touring)
    {
      if (xg->is_plotting1d)
        free_txtr_var();

      if (xg->carry_vars)
        carry_tour_vars(xg);
      xg->is_plotting1d = xg->is_xyplotting = False;
      xg->is_spinning = False;
      xg->is_corr_touring = False;
      xg->is_touring = True;

      /* Reallocate the vector used for linked touring */
      xg->tour_senddata = (unsigned long *)
        XtRealloc((XtPointer) xg->tour_senddata,
        (Cardinal) (4 + 5*xg->ncols) * sizeof(long));

      /*
       * All these data pipeline calls should occur <after>
       * the mode state variables have been reset; that is,
       * xg->is_touring = 1.
      */
/*      if (xg->is_princ_comp)
      {
        update_lims(xg);
        reset_var_labels(xg, PRINCCOMP_ON);
      }*//* bug fix: sphering transformation */

      /*
       * xgobi only needs this line if xg->is_princ_comp,
       * but xgvis needs it in any case.
      */
      update_world(xg);

      find_plot_center(xg);
      world_to_plane(xg);
      plane_to_screen(xg);
      plot_once(xg);
      tour_var_lines(xg);

      for (j=0; j<xg->ncols_used; j++)
        xg->varchosen[j] = False;
      for (j=0; j<xg->numvars_t; j++)
      {
        k = xg->tour_vars[j];
        xg->varchosen[k] = True;
      }

      refresh_vboxes(xg);
    }

    tour_event_handlers(xg, 1);
  
    map_tour_panel(xg, True);
    if (xg->is_pp)
      map_tour_pp_panel(True);
    else if (xg->is_tour_section)
      map_section_panel(True);
    set_varsel_label(xg);
  
    start_tour_proc(xg);
  }
}
