/* tour_section.c */
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

#define INIT_EPS_SCROLLBAR 0.5

Widget tour_section_panel, tour_section_label[1], tour_section_sbar;
float **thh;
float tour_section_eps;
float tour_section_eps_p_val;
float tour_section_npts;
float *tour_pt_wgts;

void
alloc_section(xgobidata *xg)
{
  int i;

  xg->section_color_ids = (unsigned long *) XtMalloc(
    (unsigned int) xg->nrows * sizeof(unsigned long));
  xg->section_glyph_ids = (glyphv *) XtMalloc(
    (unsigned int) xg->nrows * sizeof(glyphv));

  thh = (float **) XtMalloc((Cardinal) xg->ncols * sizeof(float *));
  for (i=0; i<xg->ncols; i++)
    thh[i] = (float *) XtMalloc((Cardinal) xg->ncols * sizeof(float));

  tour_pt_wgts = (float *) XtMalloc((Cardinal) xg->nrows * sizeof(float));
}

void
free_section(xgobidata *xg)
{
  int j;

  XtFree((XtPointer) xg->section_color_ids);
  XtFree((XtPointer) xg->section_glyph_ids);

  for (j=0; j<xg->ncols; j++)
    XtFree((XtPointer) thh[j]);
  XtFree((XtPointer) thh);

  XtFree((XtPointer) tour_pt_wgts);
}

void
init_section(xgobidata *xg)
{
  int i;
  static int initd = False;

  if (!initd)
  {
    tour_section_eps_p_val = INIT_EPS_SCROLLBAR;
    tour_section_eps = (tour_section_eps_p_val+0.02)*0.96;
    initd = True;
  }
  for (i=0; i<xg->nrows_in_plot; i++)
    tour_pt_wgts[xg->rows_in_plot[i]] = 1.;
}

void
proj_mat_using_on_basis(xgobidata *xg)
{
  int j1, j2, p = xg->numvars_t;

  for (j1=0; j1<p; j1++)
    for (j2=j1; j2<p; j2++)
    {
      thh[xg->tour_vars[j1]][xg->tour_vars[j2]] =
        -(xg->tv[0][xg->tour_vars[j1]]*xg->tv[0][xg->tour_vars[j2]] +
          xg->tv[1][xg->tour_vars[j1]]*xg->tv[1][xg->tour_vars[j2]]);
      if (j1 == j2)
        thh[xg->tour_vars[j1]][xg->tour_vars[j1]] += 1.;
      thh[xg->tour_vars[j2]][xg->tour_vars[j1]] =
        thh[xg->tour_vars[j1]][xg->tour_vars[j2]];
  }
}


void
form_proj_matrix(xgobidata *xg)
{
  int i, j;

/*
 * This routine calculates I-Pv matrix in which X will be projected.
 * Pv = VV' where V consists of [v0,v1](px2) because V'V=I.
 * This is the projection into the orthogonal complement of the
 * projection plane, and from this we can get the distance of each
 * point from the plane.
*/

  for (i=0; i<xg->ncols_used; i++)
    for (j=0; j<xg->ncols_used; j++)
      thh[i][j] = 0.;

  for (i=0; i<xg->ncols_used; i++)
  {
    xg->tv[0][i] = xg->u[0][i];
    xg->tv[1][i] = xg->u[1][i];
  }
  proj_mat_using_on_basis(xg);
}

float
check_residuals(float **tmpmat, float *tmpvec1, float *tmpvec2, int p)
{
  int i;
  float tmpf, return_val=1.;
/*
 * This routine takes a data vector and projects it into the
 * orthogonal complents of the projection plane, and calculates
 * the norm of this vector, which gives the distance of the
 * point from the plane. Hence the word residual, like getting the
 * residuals from a regression plane.
*/

  for (i=0; i<p; i++)
    tmpvec2[i] = inner_prod(tmpmat[i],tmpvec1, p);

  tmpf = calc_norm_sq(tmpvec2, p);
  return_val = tmpf;
  return(return_val);
}

/* ARGSUSED */ /* do_plot is used in Di's version */
void
tour_section_calcs(xgobidata *xg, int do_plot)
{
  int i, j, m;
  float precis = PRECISION1;

  form_proj_matrix(xg);

  tour_section_npts = 0.;
  for (i=0; i<xg->nrows_in_plot; i++)
  {
    m = xg->rows_in_plot[i];
    for (j=0; j<xg->ncols_used; j++)
      xg->tv[0][j] = (float) xg->world_data[m][j];
    tour_pt_wgts[m] = check_residuals(thh,
      xg->tv[0], xg->tv[1], xg->ncols_used);
    tour_pt_wgts[m] = (float)sqrt((double)tour_pt_wgts[m]) /
      ((float)sqrt((double)(xg->numvars_t-2)) * precis);
    if (tour_pt_wgts[m] > tour_section_eps)
    {
      /* No need to set a size here; points only come in one size */
      xg->section_glyph_ids[m].type = POINT_GLYPH;
      xg->section_color_ids[m] = xg->color_now[m];
    }
    else
    {
      tour_section_npts += 1.;
      xg->section_glyph_ids[m].type = xg->glyph_now[m].type;
      xg->section_glyph_ids[m].size = xg->glyph_now[m].size;
      xg->section_color_ids[m] = xg->color_now[m];
    }
  }
}

void
set_tour_section_eps(float slidepos, xgobidata *xg, int slidermoved)
{
  char str[25];
  float tmpf2;

  if (slidermoved)
    tour_section_eps_p_val = slidepos;
  tmpf2 = (tour_section_eps_p_val+0.02)*0.96;
  tour_section_eps = tmpf2;

  sprintf(str, "%s: %4.2f","Eps", tmpf2);
  XtVaSetValues(tour_section_label[0], XtNstring, (String) str, NULL);

  if (!slidermoved)
    XawScrollbarSetThumb(tour_section_sbar, tmpf2, -1.);/*section*/

  if (xg->is_tour_section)
  {
    tour_section_calcs(xg, 0);
    plot_once(xg);
  }
}

/* ARGSUSED */
XtCallbackProc
tour_section_eps_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  float slidepos = * (float *) slideposp;

  set_tour_section_eps(slidepos, xg, 1);
}

void
set_section_on_glyphs(xgobidata *xg)
{
  int i;

  for (i=0; i<xg->nrows_in_plot; i++)
  {
    xg->section_glyph_ids[xg->rows_in_plot[i]].type =
      xg->glyph_now[xg->rows_in_plot[i]].type;
    xg->section_glyph_ids[xg->rows_in_plot[i]].size =
      xg->glyph_now[xg->rows_in_plot[i]].size;
    xg->section_color_ids[xg->rows_in_plot[i]] =
      xg->color_now[xg->rows_in_plot[i]];
  }

  tour_section_calcs(xg, 0);
}

void
turn_off_section_tour(xgobidata *xg)
{
  XtCallCallbacks(xg->tour_section_cmd, XtNcallback, (XtPointer) NULL);
  XtVaSetValues(xg->tour_section_cmd,
    XtNstate, False, NULL);
}

void
map_section_panel(Boolean map)
{
  if (map)
    XtMapWidget(tour_section_panel);
  else
    XtUnmapWidget(tour_section_panel);
}

/* ARGSUSED */
XtCallbackProc
tour_section_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  if (!xg->is_tour_section)
  {
    xg->is_tour_section = True;
    alloc_section(xg);
    init_section(xg);
    set_section_on_glyphs(xg);
    tour_section_calcs(xg, 0);
    map_section_panel(True);
  }
  else
  {
    xg->is_tour_section = False;
    free_section(xg);
    map_section_panel(False);
  }
  zero_tau(xg);
  plot_once(xg);

  setToggleBitmap(w, xg->is_tour_section);
}

void
make_section_panel(xgobidata *xg, Widget panel, Dimension sbarwidth)
{
  char str[30];
  Dimension width;

/*
 * Panel for section tour
*/
  tour_section_panel = XtVaCreateManagedWidget("Tour_Panel",
    formWidgetClass, xg->box0,
    XtNfromVert, (Widget) panel,
    XtNmappedWhenManaged, (Boolean) False,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    NULL);
 if (mono) set_mono(tour_section_panel);

/*
 * section tour epsilon slice scrollbar
*/
  sprintf(str, "%s: %4.2f", "Eps", 0.5);
  width = XTextWidth(appdata.font, str, strlen(str));

  tour_section_label[0] = XtVaCreateManagedWidget("TourLabel",
    asciiTextWidgetClass, tour_section_panel,
    XtNstring, (String) str,
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH),
    NULL);
  if (mono) set_mono(tour_section_label[0]);

  tour_section_sbar = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, tour_section_panel,
    XtNfromVert, (Widget) tour_section_label[0],
    XtNwidth, (Dimension) sbarwidth,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    XtNvertDistance, (Dimension) 0 ,
    NULL);
  if (mono) set_mono(tour_section_sbar);

  add_sbar_help(&xg->nhelpids.sbar,
    tour_section_sbar, "TourSectionEps");
  XawScrollbarSetThumb(tour_section_sbar, INIT_EPS_SCROLLBAR, -1.);
  XtAddCallback(tour_section_sbar, XtNjumpProc,
    (XtCallbackProc) tour_section_eps_cback, (XtPointer) xg);
}

