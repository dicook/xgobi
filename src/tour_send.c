/* tour_send.c */
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

#include <stdio.h>
#include "xincludes.h"
#define XGOBIEXTERN
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

/* ARGSUSED */
XtSelectionDoneProc
pack_tour_done(Widget w, Atom *selection, Atom *target)
{
/*
 * This routine does nothing; its only purpose in life is to
 * prevent the Intrinsics from freeing the selection value.
*/
}

/* ARGSUSED  */
XtLoseSelectionProc
pack_tour_lose(Widget w, Atom *selection, XtPointer xgobi)
{
/*
 * This routine does nothing; its only purpose in life is to
 * prevent the Intrinsics from freeing the selection value.
*/
}

void
announce_tour_coefs(xgobidata *xg)
{
/*
 * This sends an event to tell other XGobi windows to
 * execute XtGetSelection().  It sends no data.
*/
  XChangeProperty( display,
    RootWindowOfScreen(XtScreen(xg->shell)),
    XG_NEWTOUR_ANNC, XG_NEWTOUR_ANNC_TYPE,
    (int) 32, (int) PropModeReplace,
    (unsigned char *) NULL, 0);
}

/* ARGSUSED */
XtConvertSelectionProc
pack_tour_data(Widget w, Atom *selection, Atom *target,
Atom *type_ret, XtPointer *retdata, unsigned long *length_ret, int *format_ret )
/*
  w           owning widget, xg->workspace
  selection   XG_NEWTOUR
  target      XG_NEWTOUR_TYPE
  type_ret    XG_NEWTOUR_TYPE again
  retdata     touring data itself
  length_ret  length of retdata
  format_ret  type of retdata
*/
{
/*
 * When another XGobi requests the touring data from the server,
 * this function is executed by the active XGobi.  It provides
 * the touring data to the requestor.
 * It is declared by the call to XtOwnSelection().
*/
  extern xgobidata xgobi;

  if (*target == XG_NEWTOUR_TYPE)
  {
    int k, j, m;
    float ftmp, precis = PRECISION1;

/*
 * This seems to be needed -- there are so many ways to
 * leave the linked tour and re-enter it ...  dfs 3/96
*/
    if (xgobi.tour_senddata == (unsigned long *) NULL)
      xgobi.tour_senddata = (unsigned long *)
        XtRealloc((XtPointer) xgobi.tour_senddata,
        (Cardinal) (4 + 5*xgobi.ncols) * sizeof(long));


/*fprintf(stderr, "packing tour data\n");*/
    xgobi.tour_senddata[0] = (unsigned long) xgobi.ncols_used;
    xgobi.tour_senddata[1] = (unsigned long) xgobi.is_princ_comp;
    xgobi.tour_senddata[2] = (unsigned long) xgobi.new_basis_ind;
    xgobi.tour_senddata[3] = (unsigned long) xgobi.numvars_t;

    m = 4;
    for (j=0; j<xgobi.numvars_t; j++)
      xgobi.tour_senddata[m++] = (long) xgobi.tour_vars[j];
    for (k=0; k<2; k++)
      for (j=0; j<xgobi.ncols_used; j++)
      {
        ftmp = (xgobi.u[k][j] + 1.0) / 2.0 ;
        xgobi.tour_senddata[m++] = (long) (ftmp * precis);
      }

    if (xgobi.is_princ_comp)
    {
      invert_proj_coords(&xgobi);
      for (k=0; k<2; k++)
        for (j=0; j<xgobi.ncols_used; j++)
        {
          ftmp = (xgobi.tv[k][j] + 1.0) / 2.0 ;
          xgobi.tour_senddata[m++] = (long) (ftmp * precis);
        }
    }

    *type_ret = XG_NEWTOUR_TYPE ;
    *retdata = (XtPointer) xgobi.tour_senddata ;
    *length_ret = (unsigned long) m ;
    *format_ret = (int) 32 ;
  }
}

/* ARGSUSED */
XtSelectionCallbackProc
unpack_tour_data(Widget w, xgobidata *xg, Atom *atom, Atom *atom_type,
XtPointer retdata, unsigned long *lendata, int *fmt)
{
  int i, j, k, m, nc;
  unsigned long *rdata;
  static int count = 0, is_pc, is_new_basis, old_numvars_t;
  static Boolean compatible = True;
  char message[MSGLENGTH];

  if (*atom == XG_NEWTOUR &&
      *atom_type &&
      *atom_type != XT_CONVERT_FAIL)
  {
    if (*lendata > 0)
    {
      unsigned long ltmp;
      float ftmp, precis = PRECISION1 ;
      old_numvars_t = xg->numvars_t;/* record old value before reading new */
      rdata = (unsigned long *) retdata;
      nc = (int) *rdata++ ;
      is_pc = (int) *rdata++ ;

/*fprintf(stderr, "unpacking tour data\n");*/
      if (compatible && !is_pc && xg->is_princ_comp)
      {
        compatible = False;
        sprintf(message,
          "Linking doesn't make sense!\n");
        strcat(message,
          "One data set is sphered and the other is not.\n");
        show_message(message, xg);
      }

      is_new_basis = (int) *rdata++ ;
      xg->numvars_t = (int) *rdata++ ;

      for (j=0; j<xg->numvars_t; j++)
        xg->tour_vars[j] = (int) *rdata++ ;

      if (xg->is_princ_comp && (old_numvars_t != xg->numvars_t))
      {

        if (update_vc_active_and_do_svd(xg, xg->numvars_t, xg->tour_vars))
          spherize_data(xg, xg->numvars_t, xg->numvars_t, xg->tour_vars);
        else
          copy_tform_to_sphered(xg);
       update_lims(xg);
       update_world(xg);
       refresh_all_var_labels(xg);
      }

      /* copy received data into appropriate data structures */
      for (k=0; k<2; k++)
        for (j=0; j<nc; j++)
        {
          ltmp = *rdata++ ;
          ftmp = (float) ltmp / precis;  /* May lose precision */
          xg->u[k][j] = (float) ((ftmp * 2.0) - 1.0) ;
        }

      if (is_pc && !xg->is_princ_comp)
        /* if sending xgobi is displaying sphered data and
         * receiving xgobi is displaying unsphered data then
         * use the inverted projection coordinates.
        */
      {
        for (k=0; k<2; k++)
          for (j=0; j<nc; j++)
          {
            ltmp = *rdata++ ;
            ftmp = (float) ltmp / precis;  /* May lose precision */
            xg->u[k][j] = (float) ((ftmp * 2.0) - 1.0) ;
          }
      }

      /* run it through the pipeline */
      for (m=0; m<xg->nrows_in_plot; m++)
      {
        i = xg->rows_in_plot[m];
        for (j=0; j<xg->ncols_used; j++)
          xg->tnx[j] = FLOAT(xg->world_data[i][j]);
        xg->planar[i].x = (long) inner_prod(xg->tnx, xg->u[0], xg->ncols_used);
        xg->planar[i].y = (long) inner_prod(xg->tnx, xg->u[1], xg->ncols_used);
      }
      plane_to_screen(xg);
      plot_once(xg);
      tour_var_lines(xg);
      if (xg->is_pp)
      {
        count++;
        if (count >= xg->pp_replot_freq)
        {
          pp_index(xg,0,is_new_basis);
          count = 0;
        }
      }
      else if (xg->is_tour_section)
        tour_section_calcs(xg, 1);
      if (is_new_basis && xg->is_store_history)
        store_basis(xg, xg->u);
    }
  }
  XtFree((XtPointer) retdata);
  /*xg->nevents = 0;*/
}

void
read_tour_coefs(xgobidata *xg)
{
  XtGetSelectionValue(
    xg->workspace,
    (Atom) XG_NEWTOUR,
    (Atom) XG_NEWTOUR_TYPE,
    (XtSelectionCallbackProc) unpack_tour_data,
    (XtPointer) xg,
    (Time) XtLastTimestampProcessed(display) );
    /*(Time) CurrentTime );*/
}
