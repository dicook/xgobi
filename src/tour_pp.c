/* tour_pp.c */
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
#include <stdlib.h>
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#define NUM_INDICES 10
/* BUILD_YOUR_OWN_INDEX:
#define NUM_INDICES 11
*/
#define MAX_NBITMAPS 20
#define MAX_NOPTIMZ_CIRCS 20 
#define OPTIMZ_CIRC_WIDTH 20
#define OPTIMZ_CIRC_HEIGHT 20
#define OPTIMZ_CIRC_ANG1 245*64
#define OPTIMZ_CIRC_ANG2 50*64
#define OPTIMZ_OFFSET 10
#define LEN_NEW_BASIS_LINE 10
#define WIDTH_OPTIMZ_ARROW 5
#define HEIGHT_OPTIMZ_ARROW 10

#define NATURAL_HERMITE_BTN 0
#define HERMITE_BTN 1
#define CENTRAL_MASS_BTN 2
#define HOLES_BTN 3
#define SKEWNESS_BTN 4
#define LEGENDRE_BTN 5
#define FTS_BTN 6
#define ENTROPY_BTN 7
#define BIN_FTS_BTN 8
#define BIN_ENTROPY_BTN 9
/* BUILD_YOUR_OWN_INDEX:
#define DUMMY_BTN 8
*/

#define PP_PLOTMARGIN_TOP 0.05

/* to account for scale differences between tour and other modes */
#define FUDGE_FACTOR (xg->numvars_t+1)*2/7

static Widget tour_pp_cmd[6], tour_pp_label[11], tour_pp_options[12];
static Widget tour_pp_panel[3];
Widget pp_plot_form, pp_plot_box0, pp_plot_box1;
static Widget pp_plot_shell;
static Widget ppindex_menu_cmd, ppindex_menu, ppindex_menu_btn[NUM_INDICES];
static Window pp_plot_window;
static Pixmap pp_plot_pixmap;
GC tour_pp_GC, tour_pp_clear_GC;
WidgetSize pp_wksp;

static float window_width_const = 3.12;
static int count = 0;
static int max_pp_storage;
static float *pp_index_array;
static XRectangle *pp_index_rect;
static XPoint *pp_index_pts;
static XPoint **bitmaps;
static int *bitmap_frame;
static int *npts_in_bitmap;
static int **pts_in_bitmap;
static int *new_basis_lines;
static int nbitmaps = 0;
static int nnew_bases = 0;
static int *optimz_circs; 
static int noptimz_circs = 0;
static Boolean is_pp_lines = 1;
static Boolean is_pp_points = 0;
static unsigned int pp_line_width = 2;
static int pp_bw_points = 2;
static float **bitmap_basis0;
static float **bitmap_basis1;
static unsigned int bitmap_size;
/*static int bitmap_space = 70;*/ /* = bitmap_size + 2*15 */
static Dimension bitmap_space = 70; /* = bitmap_size + 2*15 */
static int bitmap_percent = 20;
#define MAX_BITMAP_PERCENT 70
static Boolean return_to_bitmap = 1;
static Boolean record_bitmap = 0;
static FILE *bm_fp;
static char bm_filename[128];
static int bm_counter = 0;
static Boolean singular_vc;

static int MINLJ = 1;
static int MAXLJ = 35; /* this has to be used to allocate space */
static int lJ = 1;
static Widget nterms_leftarr, nterms_rightarr;

static float bandwidth;
static float optimal_bandwidth;
static float min_bandwidth, max_bandwidth;

static long **tv2;
static float *mean, **vc, **vc_active, *eigenval, *a, *a1, **b;
static XPoint *xpts;
static char *index_names[NUM_INDICES];

static float Ihat;
float **dIhat;
static float **X;

static float pp_index_max = 0.1;
static float pp_index_min = 0.;

#define TOUR_PP_PANEL tour_pp_panel[0]

#define PP_BTN xg->proj_pursuit_cmd
#define PP_OPTIMZ tour_pp_cmd[0]
#define PP_BITMAP tour_pp_cmd[1]
#define PP_TERMSINEXP tour_pp_cmd[2]

#define PP_INDEX_LABEL tour_pp_label[0]
#define PP_TERMSINEXP_LABEL tour_pp_label[1]
#define PP_LINEWIDTH_LABEL tour_pp_label[2]
#define PP_BW_POINTS_LABEL tour_pp_label[3]
#define PP_BITMAPSZ_LABEL tour_pp_label[4]
#define PP_RF_LABEL tour_pp_label[5]

#define PP_PLOT_LINES tour_pp_options[0]
#define PP_LINEWIDTH_LEFTARR tour_pp_options[1]
#define PP_LINEWIDTH_RIGHTARR tour_pp_options[2]
#define PP_PLOT_POINTS tour_pp_options[3]
#define PP_BW_POINTS_LEFTARR tour_pp_options[4]
#define PP_BW_POINTS_RIGHTARR tour_pp_options[5]
#define PP_BITMAPSZ_LEFTARR tour_pp_options[6]
#define PP_BITMAPSZ_RIGHTARR tour_pp_options[7]
#define PP_RF_LEFTARR tour_pp_options[8]
#define PP_RF_RIGHTARR tour_pp_options[9]
#define PP_RETN_TO_BM tour_pp_options[10]
#define PP_RECORD_BM tour_pp_options[11]

/* return to basis variables */
int got_new_basis;

/* for sphering */
typedef struct {  /*-- used for obtaining ranks --*/
  float f;
  int indx;
} paird;

int
pcompare (const void *val1, const void *val2)
{
  const paird *pair1 = (const paird *) val1;
  const paird *pair2 = (const paird *) val2;

  if (pair1->f < pair2->f)
    return (-1);
  else if (pair1->f == pair2->f)
    return (0);
  else
    return (1);
}

void
init_tour_pp_GCs(xgobidata *xg)
{
  Window root_window = RootWindowOfScreen(XtScreen(xg->shell));
  XGCValues gcv;
  unsigned long mask =
    GCForeground | GCBackground ;

/* create GCs for tour projection pursuit plot */
  gcv.foreground = tour_pp_colors.fg;
  gcv.background = tour_pp_colors.bg;
  tour_pp_GC = XCreateGC(display, root_window, mask, &gcv);
  XSetFont(display, tour_pp_GC, appdata.plotFont->fid);

  gcv.foreground = tour_pp_colors.bg;
  gcv.background = tour_pp_colors.fg;
  tour_pp_clear_GC = XCreateGC(display, root_window, mask, &gcv);
  XSetFont(display, tour_pp_clear_GC, appdata.plotFont->fid);
}

void
map_tour_pp_panel(Boolean is_tour_on)
{
  if (is_tour_on)
    XtMapWidget(TOUR_PP_PANEL);
  else
    XtUnmapWidget(TOUR_PP_PANEL);
}

void
set_sens_pp_btn(xgobidata *xg, int sens)
{
  XtVaSetValues(PP_BTN, XtNsensitive, (Boolean) sens, NULL);
}

void
set_sens_optimz(int sens)
{
  XtVaSetValues(PP_OPTIMZ, XtNsensitive, (Boolean) sens, NULL);
}

/* not used in this version
void
set_sens_bitmap(int sens)
{
  XtVaSetValues(PP_BITMAP, XtNsensitive, (Boolean) sens, NULL);
}
*/

void
alloc_std_vars(xgobidata *xg)
{
  int i;
  Cardinal nc = xg->ncols ;

  mean = (float *) XtMalloc(nc * sizeof(float));
/* basically scratch arrays with the exception of vc - variance-covariance */
  a = (float *) XtMalloc(nc * sizeof(float));
  eigenval = (float *) XtMalloc(nc * sizeof(float));
  a1 = (float *) XtMalloc(nc * sizeof(float));
  vc = (float **) XtMalloc(nc * sizeof(float *));
  vc_active = (float **) XtMalloc(nc * sizeof(float *));
  b = (float **) XtMalloc(nc * sizeof(float *));
  for (i=0; i<nc; i++)
  {
    vc[i] = (float *) XtMalloc(nc * sizeof(float));
    vc_active[i] = (float *) XtMalloc(nc * sizeof(float));
    b[i] = (float *) XtMalloc(nc * sizeof(float));
  }

  xg->sph_vars = (int *) XtMalloc((Cardinal) (xg->ncols-1) * sizeof(int));/* sphere*/
}

void
free_std_vars(xgobidata *xg)
{
  int i;

  XtFree((XtPointer) mean);
  XtFree((XtPointer)a);
  XtFree((XtPointer)eigenval);
  XtFree((XtPointer)a1);

  for (i=0; i<xg->ncols; i++)
    XtFree((XtPointer)vc[i]);
  XtFree((XtPointer)vc);

  for (i=0; i<xg->ncols; i++)
    XtFree((XtPointer)vc_active[i]);
  XtFree((XtPointer)vc_active);

  for (i=0; i<xg->ncols; i++)
    XtFree((XtPointer)b[i]);
  XtFree((XtPointer)b);

  XtFree((XtPointer)xg->sph_vars);
}

void
alloc_pp(xgobidata *xg)
{
  int i;
/* allocate arrays for pp*/
  X = (float **) XtMalloc(
    (unsigned int) 2*sizeof(float *));
  for (i=0; i<2; i++)
    X[i] = (float *) XtMalloc(
      (unsigned int) xg->nrows*sizeof(float));

  dIhat = (float **) XtMalloc(
    (unsigned int) 2*sizeof(float *));
  for (i=0; i<2; i++)
    dIhat[i] = (float *) XtMalloc(
      (unsigned int) xg->ncols*sizeof(float));

  tv2 = (long **) XtMalloc(
    (unsigned int) 2*sizeof(long *));
  for (i=0; i<2; i++)
    tv2[i] = (long *) XtMalloc(
      (unsigned int) xg->nrows*sizeof(long));

  xpts = (XPoint *) XtMalloc(
    (unsigned int) xg->nrows*sizeof(XPoint));
}

void
free_pp(void)
{
  int i;

  for (i=0; i<2; i++)
    XtFree((XtPointer) X[i]);
  XtFree((XtPointer) X);
  for (i=0; i<2; i++)
    XtFree((XtPointer) dIhat[i]);
  XtFree((XtPointer) dIhat);
  XtFree((XtPointer) xpts);
  for (i=0; i<2; i++)
    XtFree((XtPointer) tv2[i]);
  XtFree((XtPointer) tv2);
}

/*
 * this is the reset routine to change variable labels to
 * principal component labels and back again.
*/
void
reset_var_labels(xgobidata *xg, int ind)
{
  int j;
  char str[COLLABLEN];
  Dimension width, newwidth;

  if (ind == PRINCCOMP_OFF)
  {
    for (j=0; j<xg->numvars_t; j++)
    {
      sprintf(str,"%s", xg->collab[xg->tour_vars[j]]);
      XtVaSetValues(xg->varlabw[xg->tour_vars[j]],
        XtNlabel, (String) str, NULL);
    }
  }
  else if (ind == PRINCCOMP_ON)
  {
    for (j=0; j<xg->numvars_t; j++)
    {
      XtVaGetValues(xg->varlabw[xg->tour_vars[j]],
        XtNwidth, &width, NULL);
      sprintf(str, "PC %d", j+1);

/* 
 * 4.29.94, dfs; small bugfix to handle the case where
 * the PC labels are longer than the regular labels
*/
      newwidth = XTextWidth(appdata.font, str, strlen(str)) +
        2*ASCII_TEXT_BORDER_WIDTH;
      XtVaSetValues(xg->varlabw[xg->tour_vars[j]],
        XtNwidth, MAX(width, newwidth),
        XtNlabel, (String) str, NULL);
    }
  }
}

/* this routine is to refresh labels during linking */
void
refresh_all_var_labels(xgobidata *xg)
{
  int j, k;
  char str[COLLABLEN];
  Dimension width;

  k=0;
  /*for (j=0; j<xg->tour_vars[xg->numvars_t-1]; j++)*/
  for (j=0; j<xg->numvars_t; j++)
  {
    if (j==xg->tour_vars[k])
    {
      XtVaGetValues(xg->varlabw[xg->tour_vars[j]],
        XtNwidth, &width, NULL);
      sprintf(str, "PC %d", k+1);
      XtVaSetValues(xg->varlabw[xg->tour_vars[k]],
        XtNwidth, (Dimension) width,
        XtNlabel, (String) str, NULL);
      k++;
    }
    else
    {
      sprintf(str,"%s", xg->collab[j]);
      XtVaSetValues(xg->varlabw[j],
        XtNlabel, (String) str, NULL);
    }
  }
  /* proviso for additional labels */
  for (j=xg->tour_vars[xg->numvars_t-1]+1; j<xg->ncols_used; j++)
  {
    sprintf(str,"%s", xg->collab[j]);
    XtVaSetValues(xg->varlabw[j],
      XtNlabel, (String) str, NULL);
  }
}

void
reset_pp_plot()
{
  count = 0;
  nbitmaps = 0;
  nnew_bases = 0;
  noptimz_circs = 0;
}

void
proj_scale_bitmap_pts(xgobidata *xg, int indx, int npts, int *pts_pos)
{
  int i, j, k, m, n;

/* project the world data into current basis, and draw into 20x20
   bitmap in projection pursuit window */
  n = npts;
  for (i=0; i<n; i++)
  {
    m = pts_pos[i];
    for (k=0; k<xg->ncols_used; k++)
      a[k] = (float) xg->world_data[m][k];
    tv2[0][i] = (long)(inner_prod(a, bitmap_basis0[indx],
      xg->ncols_used));
    tv2[1][i] = (long)(inner_prod(a, bitmap_basis1[indx],
      xg->ncols_used));
  }

/* scale data into bitmap */
  if (xg->is_princ_comp)
  {
    for (j=0; j<n; j++)
    {
      tv2[0][j] = (long) ((float)tv2[0][j] *
        (((float)bitmap_size * 0.4 * xg->scale.x * (float)FUDGE_FACTOR) /
        (float)PRECISION1) + bitmap_size * 0.5 );
      tv2[1][j] = (long) ((float)tv2[1][j] *
        (((float)bitmap_size * 0.4 * xg->scale.y * (float)FUDGE_FACTOR) /
        (float)PRECISION1) + bitmap_size * 0.5 );
    }
  }
  else
  {
    for (j=0; j<n; j++)
    {
      tv2[0][j] = (long) ((float)tv2[0][j] *
        (((float)bitmap_size * 0.4 * xg->scale.x * (float)FUDGE_FACTOR) /
        (float)PRECISION1) + bitmap_size * 0.5 );
      tv2[1][j] = (long) ((float)tv2[1][j] *
        (((float)bitmap_size * 0.4 * xg->scale.y * (float)FUDGE_FACTOR) /
        (float)PRECISION1) + bitmap_size * 0.5 );
    }
  }

  for (j=0; j<n; j++) /* invert y's */
    tv2[1][j] = (long)(1. - (float)tv2[1][j]);
}

void
shift_bitmap_pts(xgobidata *xg, int indx, int n)
{
  int i;
  for (i=0; i<n; i++)
  {
    bitmaps[indx][i].x = tv2[0][i] + (bitmap_frame[indx]);
    bitmaps[indx][i].y = tv2[1][i] + pp_wksp.height - 17;
  }
}

void
plot_bitmap(int frame_x, int n, int if_draw)
{
  int y1, y2, x;

  y1 = pp_wksp.height - bitmap_size - 15;
  XDrawRectangle(display, pp_plot_pixmap, tour_pp_GC, frame_x, y1,
    bitmap_size, bitmap_size);
  XSetLineAttributes(display, tour_pp_GC, 1, LineOnOffDash, CapRound,
    JoinMiter);

  x = frame_x + bitmap_size -3;
  y2 = (int)(PP_PLOTMARGIN_TOP*FLOAT(pp_wksp.height));
  XDrawLine(display, pp_plot_pixmap, tour_pp_GC,
    x, y1, x, y2);
  XSetLineAttributes(display, tour_pp_GC, 1, LineSolid, CapRound,
    JoinMiter);
  XDrawPoints(display, pp_plot_pixmap, tour_pp_GC, xpts, n, CoordModeOrigin);

  if (if_draw)
    XCopyArea(display, pp_plot_pixmap, pp_plot_window, tour_pp_GC,
      0, 0, pp_wksp.width, pp_wksp.height, 0, 0);
}

void
pp_plot(xgobidata *xg, float pp_index_val, int restart, int ind_new_basis,
int resized)
{
  short x, y;
  int x1, y1, x2, y2;
  int i, j;
  char string[16];

  if (xg->recalc_max_min)
  {
    pp_index_max = pp_index_val;
    pp_index_min = pp_index_val - (0.001 * pp_index_val);
    xg->recalc_max_min = False;
  }

  if (restart && !resized)
    count = 0;
  else if (resized && !restart)  /* just need to redraw */
  {
    if (count > 0)
    {
      for (i=0; i<count; i++)
      {
        /* adjust rectangle to center of index point */
        pp_index_rect[i].y = FLOAT(pp_wksp.height) - FLOAT(bitmap_space) -
          (1.-PP_PLOTMARGIN_TOP) *
          (FLOAT(pp_wksp.height) - FLOAT(bitmap_space)) *
          (FLOAT(pp_index_array[i]) - FLOAT(pp_index_min)) /
          (FLOAT(pp_index_max) - FLOAT(pp_index_min)) - 1.;
  
        pp_index_pts[i].y = FLOAT(pp_wksp.height) - FLOAT(bitmap_space) -
          (1.-PP_PLOTMARGIN_TOP) *
          (FLOAT(pp_wksp.height) - FLOAT(bitmap_space)) *
          (FLOAT(pp_index_array[i]) - FLOAT(pp_index_min)) /
          (FLOAT(pp_index_max) - FLOAT(pp_index_min));
      }
    }
    XFillRectangle(display, pp_plot_pixmap, tour_pp_clear_GC,
      0, 0, pp_wksp.width, pp_wksp.height);

    x1 = xg->xaxis_indent;
    x2 = xg->xaxis_indent;
    y1 = (int)(PP_PLOTMARGIN_TOP*FLOAT(pp_wksp.height));
    y2 = (int)(pp_wksp.height) - bitmap_space;
    XDrawLine(display, pp_plot_pixmap, tour_pp_GC, x1, y1, x2, y2);
               /*vertical axis*/
    sprintf(string, "%.2e", pp_index_max);
    XDrawString(display, pp_plot_pixmap, tour_pp_GC,
      5*ASCII_TEXT_BORDER_WIDTH, (y1+10),
      string, strlen(string));
    sprintf(string,"%.2e", pp_index_min);
    XDrawString(display, pp_plot_pixmap, tour_pp_GC,
      5*ASCII_TEXT_BORDER_WIDTH, y2, string,
      strlen(string));
    y1 = (int)(pp_wksp.height) - bitmap_space;
    x2 = INT((1.-PP_PLOTMARGIN_TOP) *
      (FLOAT(pp_wksp.width) - FLOAT(xg->xaxis_indent))) +
      xg->xaxis_indent;
    XDrawLine(display, pp_plot_pixmap, tour_pp_GC, x1, y1, x2, y2);
             /*horizontal axis*/

    y1 = y2 - LEN_NEW_BASIS_LINE;
    for (i=0; i<nnew_bases; i++)
      XDrawLine(display, pp_plot_pixmap, tour_pp_GC,
        new_basis_lines[i], y1, new_basis_lines[i], y2);

    for (i=0; i<noptimz_circs; i++)
      XFillArc(display, pp_plot_pixmap, tour_pp_GC,
        optimz_circs[i], y1, OPTIMZ_CIRC_WIDTH, OPTIMZ_CIRC_HEIGHT,
        OPTIMZ_CIRC_ANG1, OPTIMZ_CIRC_ANG2);

    if (is_pp_points)
      XDrawRectangles(display, pp_plot_pixmap, tour_pp_GC,
        pp_index_rect, count);
    if (is_pp_lines)
    { 
      if (count > 0)
      {
        XSetLineAttributes(display, tour_pp_GC, pp_line_width,
          LineSolid, CapRound, JoinBevel);
        XDrawLines(display, pp_plot_pixmap, tour_pp_GC,
          pp_index_pts, count, CoordModeOrigin);
          /* count has
           * already been updated by 1 when we get into
           * this part, so there is no need to add 1
           * to count.*/
          XSetLineAttributes(display, tour_pp_GC, 1, LineSolid,
              CapRound, JoinMiter);
      }
    }

    for (i=0; i<nbitmaps; i++)
    {
      proj_scale_bitmap_pts(xg, i, npts_in_bitmap[i], pts_in_bitmap[i]);
      shift_bitmap_pts(xg, i, npts_in_bitmap[i]);
      for (j=0; j<npts_in_bitmap[i]; j++)
      {
        xpts[j].x = bitmaps[i][j].x;
        xpts[j].y = bitmaps[i][j].y;
      }
      plot_bitmap(bitmap_frame[i], npts_in_bitmap[i], 0);
    }
    XCopyArea(display, pp_plot_pixmap, pp_plot_window, tour_pp_GC,
      0, 0, pp_wksp.width, pp_wksp.height, 0, 0);
  }
  else  /* next point */
  {
    XFillRectangle(display, pp_plot_pixmap, tour_pp_clear_GC,
      0, 0, pp_wksp.width, pp_wksp.height);

/* rescale all projection index points if max or min value exceeded */
    if (pp_index_val > pp_index_max)
    {
      pp_index_max = pp_index_val;
      for (i=0; i<count; i++)
      {
        /* adjust rectangle to center of index point */
        pp_index_rect[i].y = FLOAT(pp_wksp.height) - FLOAT(bitmap_space) -
          (1.-PP_PLOTMARGIN_TOP) *
          (FLOAT(pp_wksp.height) - FLOAT(bitmap_space)) *
          (FLOAT(pp_index_array[i]) - FLOAT(pp_index_min)) /
          (FLOAT(pp_index_max) - FLOAT(pp_index_min)) - 1.;
        pp_index_pts[i].y = FLOAT(pp_wksp.height) - FLOAT(bitmap_space) -
          (1.-PP_PLOTMARGIN_TOP) *
          (FLOAT(pp_wksp.height) - FLOAT(bitmap_space)) *
          (FLOAT(pp_index_array[i]) - FLOAT(pp_index_min)) /
          (FLOAT(pp_index_max) - FLOAT(pp_index_min));

      }
    }

    if (pp_index_val < pp_index_min)
    {
      pp_index_min = pp_index_val;
      for (i=0; i<count; i++)
      {
        /* adjust rectangle to center of index point */
        pp_index_rect[i].y = FLOAT(pp_wksp.height) - FLOAT(bitmap_space) -
          (1.-PP_PLOTMARGIN_TOP) *
          (FLOAT(pp_wksp.height) - FLOAT(bitmap_space)) *
          (FLOAT(pp_index_array[i]) - FLOAT(pp_index_min)) /
          (FLOAT(pp_index_max) - FLOAT(pp_index_min)) - 1.;
        pp_index_pts[i].y = FLOAT(pp_wksp.height) - FLOAT(bitmap_space) -
          (1.-PP_PLOTMARGIN_TOP) *
          (FLOAT(pp_wksp.height) - FLOAT(bitmap_space)) *
          (FLOAT(pp_index_array[i]) - FLOAT(pp_index_min)) /
          (FLOAT(pp_index_max) - FLOAT(pp_index_min));
      }
    }

/* draw axes */
    x1 = xg->xaxis_indent;
    x2 = xg->xaxis_indent;
    y1 = (int)(PP_PLOTMARGIN_TOP*FLOAT(pp_wksp.height));
    y2 = (int)(pp_wksp.height) - bitmap_space;
    XDrawLine(display, pp_plot_pixmap, tour_pp_GC, x1, y1, x2, y2);
               /*vertical axis*/
    sprintf(string, "%.2e", pp_index_max);
    XDrawString(display, pp_plot_pixmap, tour_pp_GC,
      5*ASCII_TEXT_BORDER_WIDTH, (y1+10),
      string, strlen(string));
    sprintf(string,"%.2e", pp_index_min);
    XDrawString(display, pp_plot_pixmap, tour_pp_GC,
      5*ASCII_TEXT_BORDER_WIDTH, y2, string,
      strlen(string));
    y1 = (int)(pp_wksp.height) - bitmap_space;
    x2 = INT((1.-PP_PLOTMARGIN_TOP) *
      (FLOAT(pp_wksp.width) - FLOAT(xg->xaxis_indent))) +
      xg->xaxis_indent;
    XDrawLine(display, pp_plot_pixmap, tour_pp_GC, x1, y1, x2, y2);
             /*horizontal axis*/
    x = count*pp_bw_points + xg->xaxis_indent;

/* draw line indicating a direction (new basis) change */
    if (ind_new_basis)
      new_basis_lines[nnew_bases++] = x;
    y1 = y2 - LEN_NEW_BASIS_LINE;
    for (i=0; i<nnew_bases; i++)
      XDrawLine(display, pp_plot_pixmap, tour_pp_GC,
        new_basis_lines[i], y1, new_basis_lines[i], y2);

/* draw circle indicating when optimz turned on */
    for (i=0; i<noptimz_circs; i++)
      XFillArc(display, pp_plot_pixmap, tour_pp_GC,
        optimz_circs[i], y1, OPTIMZ_CIRC_WIDTH, OPTIMZ_CIRC_HEIGHT,
        OPTIMZ_CIRC_ANG1, OPTIMZ_CIRC_ANG2);

/* calculate plot y-value of the projection index */
    y = FLOAT(pp_wksp.height) - FLOAT(bitmap_space) -
      (1.-PP_PLOTMARGIN_TOP) *
      (FLOAT(pp_wksp.height) - FLOAT(bitmap_space)) *
      (FLOAT(pp_index_val) - FLOAT(pp_index_min)) /
      (FLOAT(pp_index_max) - FLOAT(pp_index_min));
/*
 * if the number of points is greater than the max plotting width
 * drop off the first and shift all points back one space.
*/
    if (count*pp_bw_points >= max_pp_storage)
    {
      for (i=0; i<(max_pp_storage-1); i++)
      {
        pp_index_array[i] = pp_index_array[i+1];
        pp_index_rect[i].y = pp_index_rect[i+1].y;
        pp_index_pts[i].y = pp_index_pts[i+1].y;
      }
      count--;
      pp_index_array[count] = pp_index_val;
      pp_index_rect[count].y = (short)(y-1); 
      pp_index_pts[count].y = (short)y;
    /* need to put in shifting bitmaps and new_basis lines backwards */
      for (i=0; i<nbitmaps; i++)
      {
        for (j=0; j<npts_in_bitmap[i]; j++)
          bitmaps[i][j].x -= pp_bw_points;
        bitmap_frame[i] -= pp_bw_points;
      }
      while (nbitmaps && bitmap_frame[0] < (xg->xaxis_indent-bitmap_size+3))
      {
        for (i=0; i<nbitmaps-1; i++)
        {
          bitmap_frame[i] = bitmap_frame[i+1];
          for (j=0; j<npts_in_bitmap[i]; j++)
          {
            bitmaps[i][j].x = bitmaps[i+1][j].x;
            bitmaps[i][j].y = bitmaps[i+1][j].y;
          }
          for (j=0; j<-xg->ncols_used; j++)
	  {
            bitmap_basis0[i][j] = bitmap_basis0[i+1][j];
            bitmap_basis1[i][j] = bitmap_basis1[i+1][j];
	  }
        }
        nbitmaps--;
      }
      for (i=0; i<nnew_bases; i++)
        new_basis_lines[i] -= pp_bw_points;
      if (new_basis_lines[0] < xg->xaxis_indent)
      {
        for (i=0; i<nnew_bases-1; i++)
          new_basis_lines[i] = new_basis_lines[i+1];
        nnew_bases--;
      }
      for (i=0; i<noptimz_circs; i++)
        optimz_circs[i] -= pp_bw_points;
      if (noptimz_circs &&
        optimz_circs[0] < (xg->xaxis_indent - OPTIMZ_CIRC_WIDTH/2))
      {
        for (i=0; i<noptimz_circs-1; i++)
          optimz_circs[i] = optimz_circs[i+1];
        noptimz_circs--;
      }
    }
    else
    {
      pp_index_array[count] = pp_index_val;
      pp_index_rect[count].x = (short)(x-1); 
      pp_index_rect[count].y = (short)(y-1); 
      pp_index_rect[count].width = 2;
      pp_index_rect[count].height = 2;
      pp_index_pts[count].x = (short)x;
      pp_index_pts[count].y = (short)y;
    }
    if (is_pp_points)
      XDrawRectangles(display, pp_plot_pixmap, tour_pp_GC,
        pp_index_rect, count+1);
    if (is_pp_lines)
    {
      if (count > 0)
      {
        XSetLineAttributes(display, tour_pp_GC, pp_line_width,
          LineSolid, CapRound, JoinBevel);
        XDrawLines(display, pp_plot_pixmap, tour_pp_GC,
          pp_index_pts, count+1, CoordModeOrigin);
        XSetLineAttributes(display, tour_pp_GC, 1, LineSolid,
          CapRound, JoinMiter);
      }
    }

    for (i=0; i<nbitmaps; i++)
    {
      for (j=0; j<npts_in_bitmap[i]; j++)
      {
        xpts[j].x = bitmaps[i][j].x;
        xpts[j].y = bitmaps[i][j].y;
      }
      plot_bitmap(bitmap_frame[i], npts_in_bitmap[i], 0);
    }
    XCopyArea(display, pp_plot_pixmap, pp_plot_window, tour_pp_GC,
      0, 0, pp_wksp.width, pp_wksp.height, 0, 0);
    count++;
  }
}

void
pp_update_label(float pp_index_val)
{
  char str[32];

  sprintf(str, "PPIndx %.2e", pp_index_val);
  XtVaSetValues(PP_INDEX_LABEL, XtNstring, (String) str, NULL);
}

void
zero_ab(xgobidata *xg)
{
  int i, j;

  for (i=0; i<xg->ncols_used; i++) {
    a[i] = 0.;
    for (j=0; j<xg->ncols_used; j++)
      b[i][j] = 0.;
  }
}

/* projection pursuit indices */
void
pp_index(xgobidata *xg, int restart_indic, int newbase_indic)
{
  int i, k, m;
  float tmpf1;

/* Calculate projections into to view axis - store in X*/
  zero_ab(xg);
  if (xg->is_princ_comp)
  {
    for (i=0; i<xg->nrows_in_plot; i++)
    {
      m = xg->rows_in_plot[i];
      for (k=0; k<xg->numvars_t; k++)
        a[xg->tour_vars[k]] = xg->tform2[m][xg->tour_vars[k]];
/*        a[xg->tour_vars[k]] = xg->sphered_data[m][xg->tour_vars[k]];*/
      tmpf1 = inner_prod(xg->u[0], a, xg->ncols_used);
      X[0][m] = tmpf1;
      tmpf1 = inner_prod(xg->u[1], a, xg->ncols_used);
      X[1][m] = tmpf1;
    }
  }
  else
  {
    for (i=0; i<xg->nrows_in_plot; i++)
    {
      m = xg->rows_in_plot[i];
      for (k=0; k<xg->numvars_t; k++)
        a[xg->tour_vars[k]] = xg->tform2[m][xg->tour_vars[k]];
      tmpf1 = inner_prod(xg->u[0], a, xg->ncols_used);
      X[0][m] = tmpf1;
      tmpf1 = inner_prod(xg->u[1], a, xg->ncols_used);
      X[1][m] = tmpf1;
    }
  }


/* Select appropriate index function */
/* 11/23/99: change all the xg->nrows_in_plot to xg->nlinkable_in_plot
   to ignore decoration points. */
  if (xg->pp_index_btn == NATURAL_HERMITE_BTN)
    Ihat = natural_hermite_index(X, xg->nlinkable_in_plot, xg->rows_in_plot, lJ);
  else if (xg->pp_index_btn == HERMITE_BTN)
    Ihat = hermite_index1(X, xg->nlinkable_in_plot, xg->rows_in_plot, lJ);
  else if (xg->pp_index_btn == CENTRAL_MASS_BTN)
    Ihat = central_mass_index(X, xg->nlinkable_in_plot, xg->rows_in_plot);
  else if (xg->pp_index_btn == HOLES_BTN)
    Ihat = holes_index(X, xg->nlinkable_in_plot, xg->rows_in_plot);
  else if (xg->pp_index_btn == SKEWNESS_BTN)
    Ihat = skewness_index(X, xg->nlinkable_in_plot, xg->rows_in_plot);
  else if (xg->pp_index_btn == LEGENDRE_BTN)
    Ihat = legendre_index(X, xg->nlinkable_in_plot, xg->rows_in_plot, lJ);
  else if (xg->pp_index_btn == FTS_BTN)
    Ihat = fts_index(X, xg->nlinkable_in_plot, xg->rows_in_plot,
      bandwidth);
  else if (xg->pp_index_btn == ENTROPY_BTN)
    Ihat = entropy_index(X, xg->nlinkable_in_plot, xg->rows_in_plot,
       bandwidth);
  else if (xg->pp_index_btn == BIN_FTS_BTN)
    Ihat = bin_fts_index(X, xg->nlinkable_in_plot, xg->rows_in_plot,
      bandwidth);
  else if (xg->pp_index_btn == BIN_ENTROPY_BTN)
    Ihat = bin_entropy_index(X, xg->nlinkable_in_plot, xg->rows_in_plot, 
      bandwidth);
/* BUILD_YOUR_OWN_INDEX:
  else if (xg->pp_index_btn == DUMMY_BTN)
    Ihat = dummy_index(X, xg->nlinkable_in_plot, xg->rows_in_plot,
       param);
*****
  X = projected data - 2 x nrows_in_plot
  nrows_in_plot = number of cases included
  rows_in_plot = indices of cases included
  param = variable may be worked off a scrollbar
*/

/* print PP index */
  pp_update_label(Ihat);
  pp_plot(xg,Ihat,restart_indic,newbase_indic, 0);
}

/* projection pursuit indices */
float
pp_index_retval(xgobidata *xg)
{
  int i, k, m;
  float tmpf1;

/* Calculate projections into to view axis - store in X*/
  zero_ab(xg);
  if (xg->is_princ_comp)
  {
    for (i=0; i<xg->nlinkable_in_plot; i++) /* 11/23/99: change to nlinkable */
    {
      m = xg->rows_in_plot[i];
      for (k=0; k<xg->numvars_t; k++)
        a[xg->tour_vars[k]] = xg->tform2[m][xg->tour_vars[k]];
/*        a[xg->tour_vars[k]] = xg->sphered_data[m][xg->tour_vars[k]];*/
      tmpf1 = inner_prod(xg->u[0], a, xg->ncols_used);
      X[0][m] = tmpf1;
      tmpf1 = inner_prod(xg->u[1], a, xg->ncols_used);
      X[1][m] = tmpf1;
    }
  }
  else
  {
    for (i=0; i<xg->nlinkable_in_plot; i++) /* 11/23/99: change to nlinkable */
    {
      m = xg->rows_in_plot[i];
      for (k=0; k<xg->numvars_t; k++)
        a[xg->tour_vars[k]] = xg->tform2[m][xg->tour_vars[k]];
      tmpf1 = inner_prod(xg->u[0], a, xg->ncols_used);
      X[0][m] = tmpf1;
      tmpf1 = inner_prod(xg->u[1], a, xg->ncols_used);
      X[1][m] = tmpf1;
    }
  }

/* Select appropriate index function */
/* 11/23/99: change all the xg->nrows_in_plot to xg->nlinkable_in_plot
   to ignore decoration points. */
  if (xg->pp_index_btn == NATURAL_HERMITE_BTN)
    Ihat = natural_hermite_index(X, xg->nlinkable_in_plot, xg->rows_in_plot, lJ);
  else if (xg->pp_index_btn == HERMITE_BTN)
    Ihat = hermite_index1(X, xg->nlinkable_in_plot, xg->rows_in_plot, lJ);
  else if (xg->pp_index_btn == CENTRAL_MASS_BTN)
    Ihat = central_mass_index(X, xg->nlinkable_in_plot, xg->rows_in_plot);
  else if (xg->pp_index_btn == HOLES_BTN)
    Ihat = holes_index(X, xg->nlinkable_in_plot, xg->rows_in_plot);
  else if (xg->pp_index_btn == SKEWNESS_BTN)
    Ihat = skewness_index(X, xg->nlinkable_in_plot, xg->rows_in_plot);
  else if (xg->pp_index_btn == LEGENDRE_BTN)
    Ihat = legendre_index(X, xg->nlinkable_in_plot, xg->rows_in_plot, lJ);
  else if (xg->pp_index_btn == FTS_BTN)
    Ihat = fts_index(X, xg->nlinkable_in_plot, xg->rows_in_plot,
      bandwidth);
  else if (xg->pp_index_btn == ENTROPY_BTN)
    Ihat = entropy_index(X, xg->nlinkable_in_plot, xg->rows_in_plot,
       bandwidth);
  else if (xg->pp_index_btn == BIN_FTS_BTN)
    Ihat = bin_fts_index(X, xg->nlinkable_in_plot, xg->rows_in_plot,
      bandwidth);
  else if (xg->pp_index_btn == BIN_ENTROPY_BTN)
    Ihat = bin_entropy_index(X, xg->nlinkable_in_plot, xg->rows_in_plot,
       bandwidth);
/* BUILD_YOUR_OWN_INDEX:
  else if (xg->pp_index_btn == DUMMY_BTN)
    Ihat = dummy_index(X, xg->nlinkable_in_plot, xg->rows_in_plot,
       param);
*/

  return(Ihat);
}

void
set_sens_pc_axes(Boolean sens, xgobidata *xg)
{
  XtVaSetValues(xg->pc_axes_cmd, XtNsensitive, sens, NULL);
}

Boolean
check_singular_vc()
{
  return(singular_vc);
}

/* ARGSUSED */
XtCallbackProc
princ_comp_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * switch between principal axes and variable axes
*/
{
  int i, j, k;
  Boolean is_one;
  char message[MSGLENGTH];

  if (xg->is_princ_comp)
  {
    /* convert back to variable axes */
    xg->is_princ_comp = False;
    reset_var_labels(xg, PRINCCOMP_OFF);
    set_sens_pc_axes(False, xg);

    xg->nhist_list = 2;/* this isn't done inside
           reinit_tour_hist because it sometimes is 0,1. */
    xg->old_nhist_list = -1; /* reset it different to xg->nhist_list
                           because there it is used in a check in
                           store_basis() */
    reinit_tour_hist(xg);
    reset_backtrack_cmd(false, false, false, false);
    set_bt_firsttime();
    nback_update_label(xg);

    for (j=0; j<xg->nsph_vars; j++) {
      for (i=0; i<xg->nrows; i++) {
        xg->tform2[i][xg->sph_vars[j]] = xg->tform1[i][xg->sph_vars[j]];
      }
    }
    xg->nsph_vars=0;

    init_basis(xg);
    zero_tau(xg);
    zero_princ_angles(xg);
    update_lims(xg);
    update_world(xg);
    world_to_plane(xg);
    plane_to_screen(xg);
    plot_once(xg);

    if (xg->is_pp)
    {
      xg->recalc_max_min = True;
      reset_pp_plot();
      pp_index(xg,0,1);
    }
    if (xg->is_pp_optimz)
      xg->new_direction_flag = True;
  }
  else
  {

    xg->is_princ_comp = True;

/* needs to be done for linked touring */
    if (update_vc_active_and_do_svd(xg, xg->numvars_t, xg->tour_vars))
      spherize_data(xg, xg->numvars_t, xg->numvars_t, xg->tour_vars);
    else
    {
       printf("in copy_tform_to_sphered \n");
       copy_tform_to_sphered(xg);
    }
    if (singular_vc)
    {
      reset_princ_comp(False, xg);
    }
    else
    {
      set_sens_pc_axes(True, xg);
  
  /* in case variables are fading out zero them now */
      for (j=0; j<xg->ncols_used; j++)
      {
        is_one = False;
        for (k=0; k<xg->numvars_t; k++)
          if (j == xg->tour_vars[k])
          {
             is_one = True;
             break;
          }
        if (!is_one)
          xg->u[0][j] = xg->u[1][j] = 0.;
      }
      norm(xg->u[0], xg->ncols_used);
      norm(xg->u[1], xg->ncols_used);
      gram_schmidt(xg->u[0], xg->u[1], xg->ncols_used);
      init_basis(xg);
      copy_basis(xg->u0, xg->u1, xg->ncols_used);
      copy_u0_to_pd0(xg);
      copy_basis(xg->u0, xg->v1, xg->ncols_used);
      /* need to spherize data here. */
      /* set the shp_vars to the active vars, and set the tform type to sphere */
      for (k=0; k<xg->numvars_t; k++)
        xg->sph_vars[k] = xg->tour_vars[k];
      xg->nsph_vars = xg->numvars_t;
      set_sph_tform_tp(xg);

      if (eigenval[0]/eigenval[xg->nsph_vars-1] > 10000.0) {
        sprintf(message, "Use transformation tools to choose less PCs. Var-cov close to singular.\n");
        show_message(message, xg);
      }
      else {
        spherize_data(xg, xg->nsph_vars, xg->nsph_vars, xg->sph_vars);
	/*        set_sph_labs(xg, xg->nsph_vars); this sets the
                     labels on the tform panel, so don't do it here */
      }

      update_lims(xg);
      update_world(xg);
      tour_var_lines(xg);
  
      /* convert to principal component axes */
      reset_var_labels(xg, PRINCCOMP_ON);
  
      xg->nhist_list = 2; /* this isn't done inside
             reinit_tour_hist because it sometimes is 1,2. */
      xg->old_nhist_list = -1; /* reset it different to xg->nhist_list
                             because there it is used in a check in
                             store_basis() */
      reinit_tour_hist(xg);
      reset_backtrack_cmd(false, false, false, false);
      set_bt_firsttime();
      nback_update_label(xg);
  
      zero_tau(xg);
      zero_princ_angles(xg);
      world_to_plane(xg);
      plane_to_screen(xg);
      plot_once(xg);
  
      if (xg->is_pp)
      {
	    xg->recalc_max_min = True;
        reset_pp_plot();
        pp_index(xg,0,1);
      }
      if (xg->is_pp_optimz)
        xg->new_direction_flag = True;
    }
  }
  setToggleBitmap(w, xg->is_princ_comp);
}

/* ARGSUSED */
XtCallbackProc
pc_axes_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * switch between principal axes and variable axes
*/
{
  xg->is_pc_axes = !xg->is_pc_axes;
  setToggleBitmap(w, xg->is_pc_axes);

  plot_once(xg);
}

void
reset_princ_comp(Boolean set_val, xgobidata *xg)
{
  XtVaSetValues(xg->princ_comp_cmd, XtNstate, set_val, NULL);
  setToggleBitmap(xg->princ_comp_cmd, set_val);
}

void
set_sens_princ_comp(xgobidata *xg, int sens)
{
  XtVaSetValues(xg->princ_comp_cmd, XtNsensitive, (Boolean) sens, NULL);
}

void
reset_pp_cmd(Boolean set_val, xgobidata *xg)
{
  XtVaSetValues(xg->proj_pursuit_cmd, XtNstate, set_val, NULL);
  setToggleBitmap(xg->proj_pursuit_cmd, set_val);
}

/* ARGSUSED */
XtEventHandler
ppexpose_cback(Widget w, XtPointer client_data, XEvent *evnt, Boolean *cont)
/*
 * If the plot window is fully or partially exposed, clear and redraw.
*/
{
  if (evnt->xexpose.count == 0)  /* Compress expose events */
    XCopyArea(display, pp_plot_pixmap, pp_plot_window, tour_pp_GC,
      0, 0, pp_wksp.width, pp_wksp.height, 0, 0);
}

void
write_msg_in_pp_window(void)
{
  char str[64];
  int strlength;
/*
 * If the font is big, the string is right-truncated.
 * Should be like this:  draw it flush right unless that
 * causes left truncation; in that case, draw it flush left.
 * Consider calling this in expose routine?
*/
  sprintf(str,"%s","Derivatives ZERO: Turn Optimz Off to Continue");
  strlength = strlen(str);
  XDrawString(display, pp_plot_pixmap, tour_pp_GC,
    (int) (pp_wksp.width - XTextWidth(appdata.font, str, strlength) - 70),
    40, str, strlength);
  XCopyArea(display, pp_plot_pixmap, pp_plot_window, tour_pp_GC,
    0, 0, pp_wksp.width, pp_wksp.height, 0, 0);
}

void
make_pp_pixmap(void)
{
  /* depth is a global variable */
  pp_plot_pixmap = XCreatePixmap(display, pp_plot_window,
    pp_wksp.width, pp_wksp.height, depth);
  XFillRectangle(display, pp_plot_pixmap, tour_pp_clear_GC,
    0, 0, pp_wksp.width, pp_wksp.height);
}

/* ARGSUSED */
XtEventHandler
ppresize_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * If the window is resized, recalculate the size of the plot window, and
 * then, if points are plotted, clear and redraw.  The variable selection
 * windows also need to be redrawn.
*/
{
  float ftmp;
  WidgetSize pp_wksp_old;
  int i = 0, j;
  int strt_val;

  pp_wksp_old.width = pp_wksp.width;
  pp_wksp_old.height = pp_wksp.height;
  XtVaGetValues(xg->pp_plot_wksp,
    XtNwidth, &pp_wksp.width,
    XtNheight, &pp_wksp.height, NULL);
  if ((pp_wksp_old.width != pp_wksp.width) ||
    (pp_wksp_old.height != pp_wksp.height))
  {
    XFreePixmap(display, pp_plot_pixmap);
    make_pp_pixmap();
    ftmp = (float) ((int) pp_wksp.width - (int) xg->xaxis_indent);
    max_pp_storage = (int) ((1.-PP_PLOTMARGIN_TOP) * ftmp);

    count--; /* decrement it to reflect actual number of points
                in arrays */

    if (count*pp_bw_points > max_pp_storage)
    {
      strt_val = count - (int)(max_pp_storage/pp_bw_points);
      for (i=0; i<(int)(max_pp_storage/pp_bw_points); i++)
      {
        pp_index_array[i] = pp_index_array[i+strt_val];
        pp_index_rect[i].y = pp_index_rect[i+strt_val].y;
        pp_index_pts[i].y = pp_index_pts[i+strt_val].y;
      }
      for (i=0; i<nbitmaps; i++)
      {
        for (j=0; j<npts_in_bitmap[i]; j++)
          bitmaps[i][j].x -= (count*pp_bw_points - max_pp_storage);
        bitmap_frame[i] -= (count*pp_bw_points - max_pp_storage);
      }
      while (nbitmaps && bitmap_frame[0] < (xg->xaxis_indent-bitmap_size+3))
      {
        for (i=0; i<nbitmaps-1; i++)
        {
          bitmap_frame[i] = bitmap_frame[i+1];
          for (j=0; j<npts_in_bitmap[i]; j++)
          {
            bitmaps[i][j].x = bitmaps[i+1][j].x;
            bitmaps[i][j].y = bitmaps[i+1][j].y;
          }
          for (j=0; j<-xg->ncols_used; j++)
          {
            bitmap_basis0[i][j] = bitmap_basis0[i+1][j];
            bitmap_basis1[i][j] = bitmap_basis1[i+1][j];
          }
          nbitmaps--;
        }
      }
      if (nnew_bases > 0)
      {
        for (i=0; i<nnew_bases; i++)
          new_basis_lines[i] -= (count*pp_bw_points - max_pp_storage);
        while (new_basis_lines[0] < xg->xaxis_indent)
        {
          for (i=0; i<nnew_bases-1; i++)
            new_basis_lines[i] = new_basis_lines[i+1];
          nnew_bases--;
        }
      }
      if (noptimz_circs > 0)
      {
        for (i=0; i<noptimz_circs; i++)
          optimz_circs[i] -= (count*pp_bw_points - max_pp_storage);
        while ((noptimz_circs) && 
          optimz_circs[0] < (xg->xaxis_indent - OPTIMZ_CIRC_WIDTH/2))
        {
          for (i=0; i<noptimz_circs-1; i++)
            optimz_circs[i] = optimz_circs[i+1];
          noptimz_circs--;
        }
      }
      count = max_pp_storage / pp_bw_points;
    }

/*
 * allocating these to be max_pp_storage+1 in size since it
 * seems to fix a funny allocation bug that purify finds.
*/
    pp_index_array = (float *) XtRealloc((char *) pp_index_array,
      (unsigned int) (max_pp_storage+1) * sizeof(float));
    pp_index_rect = (XRectangle *) XtRealloc((char *) pp_index_rect,
      (unsigned int) (max_pp_storage+1) * sizeof(XRectangle));
    pp_index_pts = (XPoint *) XtRealloc((char *) pp_index_pts,
      (unsigned int) (max_pp_storage+1) * sizeof(XPoint));
    new_basis_lines = (int *) XtRealloc((char *) new_basis_lines,
      (unsigned int) (max_pp_storage+1) * sizeof(int));

    pp_plot(xg, 0., 0, 0, 1);
  }
}

void
reset_optimz(Boolean set_val)
{
  XtVaSetValues(PP_OPTIMZ, XtNstate, set_val, NULL);
  setToggleBitmap(PP_OPTIMZ, set_val);
}

void
reset_record_bitmaps(int set)
{
  XtVaSetValues(PP_RECORD_BM, XtNstate, (Boolean) set, NULL);
  setToggleBitmap(PP_RECORD_BM, (Boolean) set);
}

void
pp_dir(xgobidata *xg)
{
  int i, j;
  float tmpf1, tmpf2;
  float eps = 0.5;

/* call pp_index to update P's, and Rp's */
  pp_index(xg,0,1);

/* select appropriate derivatives */
/* 11/23/99: change all the xg->nrows_in_plot to xg->nlinkable_in_plot
   to ignore decoration points. */
  if (xg->pp_index_btn == NATURAL_HERMITE_BTN)
/*    if (xg->is_princ_comp)
      natural_hermite_deriv(xg->sphered_data, X,
        xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->tour_vars, xg->numvars_t, lJ);
    else*/
      natural_hermite_deriv(xg->tform2, X,
        xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->tour_vars, xg->numvars_t, lJ);

  else if (xg->pp_index_btn == HERMITE_BTN)
/*    if (xg->is_princ_comp)
      hermite_deriv1(xg->sphered_data, X,
        xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->tour_vars, xg->numvars_t, lJ);
    else*/
      hermite_deriv1(xg->tform2, X, xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->tour_vars, xg->numvars_t, lJ);

  else if (xg->pp_index_btn == CENTRAL_MASS_BTN)
/*    if (xg->is_princ_comp)
      central_mass_deriv(xg->sphered_data, X, xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->numvars_t, xg->tour_vars);
    else*/
      central_mass_deriv(xg->tform2, X,
        xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->numvars_t, xg->tour_vars);

  else if (xg->pp_index_btn == HOLES_BTN)
/*    if (xg->is_princ_comp)
      holes_deriv(xg->sphered_data, X, xg->u[0], xg->u[1], dIhat,
      xg->nlinkable_in_plot, xg->rows_in_plot,
      xg->ncols_used, xg->numvars_t, xg->tour_vars);
    else*/
      holes_deriv(xg->tform2, X, xg->u[0], xg->u[1], dIhat,
      xg->nlinkable_in_plot, xg->rows_in_plot,
      xg->ncols_used, xg->numvars_t, xg->tour_vars);

  else if (xg->pp_index_btn == SKEWNESS_BTN)
/*    if (xg->is_princ_comp)
      skewness_deriv(xg->sphered_data, X, xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->numvars_t, xg->tour_vars);
    else*/
      skewness_deriv(xg->tform2, X, xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->numvars_t, xg->tour_vars);

  else if (xg->pp_index_btn == LEGENDRE_BTN)
/*    if (xg->is_princ_comp)
      legendre_deriv(xg->sphered_data, X, xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->tour_vars, xg->numvars_t, lJ);
    else*/
      legendre_deriv(xg->tform2, X, xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->tour_vars, xg->numvars_t, lJ);

  else if (xg->pp_index_btn == FTS_BTN)
/*    if (xg->is_princ_comp)
      fts_deriv(xg->sphered_data, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used,
        xg->u[0], xg->u[1],
        bandwidth, xg->numvars_t, xg->tour_vars);
    else*/
      fts_deriv(xg->tform2, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used,
        xg->u[0], xg->u[1],
        bandwidth, xg->numvars_t, xg->tour_vars);
  else if (xg->pp_index_btn == ENTROPY_BTN)
/*    if (xg->is_princ_comp)
      entropy_deriv(xg->sphered_data, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used,
        xg->u[0], xg->u[1],
        bandwidth, xg->numvars_t, xg->tour_vars);
    else*/
      entropy_deriv(xg->tform2, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used,
        xg->u[0], xg->u[1],
        bandwidth, xg->numvars_t, xg->tour_vars);
  else if (xg->pp_index_btn == BIN_FTS_BTN)
/*    if (xg->is_princ_comp)
      fts_deriv(xg->sphered_data, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used,
        xg->u[0], xg->u[1],
        bandwidth, xg->numvars_t, xg->tour_vars);
    else*/
      fts_deriv(xg->tform2, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used,
        xg->u[0], xg->u[1],
        bandwidth, xg->numvars_t, xg->tour_vars);

/*    if (xg->is_princ_comp)
      bin_fts_deriv(xg->sphered_data, X, 
        xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot, xg->ncols_used,
        xg->numvars_t, xg->tour_vars, bandwidth);
    else
      bin_fts_deriv(xg->tform2, X, 
        xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot, xg->ncols_used,
        xg->numvars_t, xg->tour_vars, bandwidth);*/

  else if (xg->pp_index_btn == BIN_ENTROPY_BTN)
/*    if (xg->is_princ_comp)
      entropy_deriv(xg->sphered_data, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used,
        xg->u[0], xg->u[1],
        bandwidth, xg->numvars_t, xg->tour_vars);
    else*/
      entropy_deriv(xg->tform2, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used,
        xg->u[0], xg->u[1],
        bandwidth, xg->numvars_t, xg->tour_vars);

/* BUILD_YOUR_OWN_INDEX:
  else if (xg->pp_index_btn == DUMMY_BTN)
    if (xg->is_princ_comp)
      dummy_deriv(xg->sphered_data, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot, xg->ncols_used,
        xg->u[0], xg->u[1],
        param, xg->numvars_t, xg->tour_vars);
    else
      dummy_deriv(xg->tform2, X, dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot, xg->ncols_used,
        xg->u[0], xg->u[1],
        param, xg->numvars_t, xg->tour_vars);
*****
  xg->sphered_data = active variables sphered by principal components,
                     xg->nrows_in_plot x xg->numvars_t
  xg->tform2 = raw data that may have had some columns transformed,
                   xg->nrows_in_plot x xg->ncols_used
  dIhat = derivatives matrix to be computed, 2 x xg->ncols_used
  xg->ncols_used = number of columns in data set
  xg->u[0],xg->u[1] = projection directions, length xg->ncols_used
  xg->numvars_t = number of active variables
  xg->tour_vars = indices of active variables
*/

/* update xg->u1 by the direction vectors */
  if (derivs_equal_zero(xg))
  {
    stop_tour_proc(xg);
    write_msg_in_pp_window();
  }
  else
  {
    for (i=0; i<xg->numvars_t; i++)
      a[i] = dIhat[0][xg->tour_vars[i]];
    tmpf1 = calc_norm(a, xg->numvars_t);
    tmpf1 *= tmpf1;
    for (i=0; i<xg->numvars_t; i++)
      a[i] = dIhat[1][xg->tour_vars[i]];
    tmpf2 = calc_norm(a, xg->numvars_t);
    tmpf2 *= tmpf2;
    tmpf1 = sqrt((double) (tmpf1+tmpf2));
    for (j=0; j<2; j++)
      for (i=0; i<xg->numvars_t; i++)
        dIhat[j][xg->tour_vars[i]] *= (eps/tmpf1);
    for (i=0; i<2; i++)
      for (j=0; j<xg->numvars_t; j++)
        xg->u1[i][xg->tour_vars[j]] = xg->u[i][xg->tour_vars[j]]
          + dIhat[i][xg->tour_vars[j]];

    norm(xg->u1[0], xg->ncols_used);
    norm(xg->u1[1], xg->ncols_used);
    for (i=0; i<2; i++)
      for (j=0; j<xg->numvars_t; j++)
        xg->tv[i][j] = xg->u1[i][xg->tour_vars[j]];
    gram_schmidt(xg->tv[0], xg->tv[1],xg->numvars_t);
    for (i=0; i<2; i++)
      for (j=0; j<xg->numvars_t; j++)
        xg->u1[i][xg->tour_vars[j]] = xg->tv[i][j];

    init_basis(xg);
  }
}

/* ARGSUSED */
XtCallbackProc
tour_optimz_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This callback invokes optimization, or active projection pursuit.
*/
{
  int x, y2;

  if (!xg->is_pp_optimz)
  {
    xg->is_pp_optimz = True;
    if (xg->is_local_scan)
    {
      turn_off_local_scan(xg);
    }
    if (xg->is_backtracking)
    {
      reset_backtrack_cmd(true, false, false, false);
    }
    tour_event_handlers(xg, 0);
    XtUnmapWidget(xg->tour_mouse);
    if (return_to_bitmap)
    {
      XDefineCursor(display, XtWindow(pp_plot_box1), default_cursor);
      XFlush(display);
      XSync(display, False);
    }
    xg->new_direction_flag = True;
  }
  else
  {
    if (return_to_bitmap)
    {
      XDefineCursor(display, XtWindow(pp_plot_box1), crosshair_cursor);
      XFlush(display);
      XSync(display, False);
    }
    xg->is_pp_optimz = False;
    tour_event_handlers(xg, 1);
    XtMapWidget(xg->tour_mouse);
  }

  zero_tau(xg);
  zero_indx_prev();
  if (xg->is_stepping)
  {
    set_ready_to_stop_now(1);
    set_counting_to_stop(0);
  }
  set_sens_localscan(!xg->is_pp_optimz);
  reset_backtrack_cmd(false, false, !xg->is_pp_optimz, false);
  set_sens_reinit(!xg->is_pp_optimz);
  
  x = count*pp_bw_points + xg->xaxis_indent;
  y2 = (int)(pp_wksp.height) - bitmap_space - 10;
  optimz_circs[noptimz_circs++] = x - OPTIMZ_OFFSET;
  XFillArc(display, pp_plot_pixmap, tour_pp_GC,
    optimz_circs[noptimz_circs-1], y2, OPTIMZ_CIRC_WIDTH, OPTIMZ_CIRC_HEIGHT,
    OPTIMZ_CIRC_ANG1, OPTIMZ_CIRC_ANG2);
  XCopyArea(display, pp_plot_pixmap, pp_plot_window, tour_pp_GC,
    0, 0, pp_wksp.width, pp_wksp.height, 0, 0);

  if (!xg->is_stepping)
    start_tour_proc(xg);

  setToggleBitmap(w, xg->is_pp_optimz);
}

/* ARGSUSED */
XtCallbackProc
tour_pp_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This callback turns on/off projection pursuit guiding.
*/
{
  int i;
  float ftmp;
  static int firsttime = 1;

  if (xg->is_pp)
  {
    xg->is_pp = False;
    if (xg->is_pp_optimz)
    {
      xg->is_pp_optimz = False;
      reset_optimz(0);
    }

    /*
     * This *DOESN'T* seems to turn off the highlighting of the
     * Optimz button -- but without calling the callback.
    set_sens_optimz(0);
    set_sens_bitmap(0);
    */

    set_sens_reinit(True);

    if (!xg->is_backtracking)
      set_sens_localscan(True);
    else /* need to turn backtrack off if it has been on, because
            tour history is reinitialized. */
      reset_backtrack_cmd(true, false, true, true);

    /* This resensitizes backtrack in case optimization has been
     * turned on -- but only if local scan is off. */
    if (!xg->is_local_scan)
      reset_backtrack_cmd(false, false, true, false);

    if (xg->is_princ_comp)
    {
      XtCallCallbacks(xg->princ_comp_cmd, XtNcallback, (XtPointer) xg);
      reset_princ_comp(False, xg);
      set_sens_pc_axes(False, xg);
    }

    reset_pp_plot();
    XtPopdown(pp_plot_shell);
    XtFree((XtPointer) pp_index_array);
    XtFree((XtPointer) pp_index_rect);
    XtFree((XtPointer) pp_index_pts);
    for (i=0; i<MAX_NBITMAPS; i++)
      XtFree((XtPointer) bitmaps[i]);
    XtFree((XtPointer) bitmaps);
    XtFree((XtPointer) bitmap_frame);
    XtFree((XtPointer) npts_in_bitmap);
    for (i=0; i<MAX_NBITMAPS; i++)
      XtFree((XtPointer) pts_in_bitmap[i]);
    XtFree((XtPointer) pts_in_bitmap);
    XtFree((XtPointer) new_basis_lines);
    XtFree((XtPointer) optimz_circs);
    for (i=0; i<MAX_NBITMAPS; i++)
      XtFree((XtPointer) bitmap_basis0[i]);
    XtFree((XtPointer) bitmap_basis0);
    for (i=0; i<MAX_NBITMAPS; i++)
      XtFree((XtPointer) bitmap_basis1[i]);
    XtFree((XtPointer) bitmap_basis1);
    XFreePixmap(display, pp_plot_pixmap);

    nbitmaps = 0;
    nnew_bases = 0;
    noptimz_circs = 0;
    free_pp();
    if (xg->pp_index_btn == NATURAL_HERMITE_BTN)
      free_natural_hermite((int) MAXLJ);
    else if (xg->pp_index_btn == HERMITE_BTN)
      free_hermite((int) MAXLJ);
    else if (xg->pp_index_btn == CENTRAL_MASS_BTN)
      free_central_mass();
    else if (xg->pp_index_btn == HOLES_BTN)
      free_holes();
    else if (xg->pp_index_btn == SKEWNESS_BTN)
      free_skewness();
    else if (xg->pp_index_btn == LEGENDRE_BTN)
      free_legendre((int) MAXLJ);
    else if (xg->pp_index_btn == BIN_FTS_BTN)
      free_bin_fts();
    else if (xg->pp_index_btn == BIN_ENTROPY_BTN)
      free_bin_entropy();
/* BUILD_YOUR_OWN_INDEX:
    else if (xg->pp_index_btn == DUMMY_BTN)
      free_dummy();
*/
    XtUnmapWidget(TOUR_PP_PANEL);

    /*
     * Handle this case: we were optimizing in
     * the tour and reached a local max.  The tour was turned
     * off, but not paused, and now we're turning off pp.
    */
    if (xg->run_tour_proc == False && xg->is_tour_paused == False)
      start_tour_proc(xg);
  }
  else
  {
    optimal_bandwidth = window_width_const / 
      pow((double) xg->nlinkable_in_plot, (double)(1.0/6.0)); /*11/23/99*/
    min_bandwidth = 0.05*optimal_bandwidth;
    max_bandwidth = 5.0*optimal_bandwidth;
    /*    if (!check_sph_on(xg->numvars_t, xg->tour_vars))
    {
       char message[5*MSGLENGTH];
       char str[MSGLENGTH];
       
        sprintf(message,"Transform active variables to PCs before doing PP.\n");
        strcat(message,"Sphered variables are: ");
        for (j=0; j<xg->nsph_vars; j++)
        {
           sprintf(str,"%d ",xg->sph_vars[j]);
           strcat(message, str);
        }
        strcat(message,"\n Active Variables are: ");
        for (j=0; j<xg->numvars_t; j++)
        {
           sprintf(str,"%d ",xg->tour_vars[j]);
           strcat(message, str);
        }
        strcat(message,"\n");
        show_message(message, xg);
        singular_vc = True;
        
    }*/
    if (!xg->is_princ_comp)
    {
      XtCallCallbacks(xg->princ_comp_cmd, XtNcallback, (XtPointer) xg);
    }
    
    if (singular_vc)
      reset_pp_cmd(False, xg);
    else
    {
      reset_princ_comp(True, xg);
      set_sens_pc_axes(True, xg);
      xg->is_pp = True;
      alloc_pp(xg);
      if (xg->pp_index_btn == NATURAL_HERMITE_BTN)
        alloc_natural_hermite(xg->nrows, (int) MAXLJ);
      else if (xg->pp_index_btn == HERMITE_BTN)
        alloc_hermite(xg->nrows, (int) MAXLJ);
      else if (xg->pp_index_btn == CENTRAL_MASS_BTN)
        alloc_central_mass(xg->nrows);
      else if (xg->pp_index_btn == HOLES_BTN)
        alloc_holes(xg->nrows);
      else if (xg->pp_index_btn == SKEWNESS_BTN)
        alloc_skewness(xg->nrows);
      else if (xg->pp_index_btn == LEGENDRE_BTN)
        alloc_legendre(xg->nrows, (int) MAXLJ);
      else if (xg->pp_index_btn == BIN_FTS_BTN)
        alloc_bin_fts(xg->nrows);
      else if (xg->pp_index_btn == BIN_ENTROPY_BTN)
        alloc_bin_entropy(xg->nrows);
  /* BUILD_YOUR_OWN_INDEX:
      else if (xg->pp_index_btn == DUMMY_BTN)
        alloc_dummy(xg->nrows);
  */
  
      if (xg->tour_link_state == receive)
        set_sens_optimz(0);
  
      /* need to turn backtrack off if it has been on, because
       *  tour history is reinitialized.
      */
      if (xg->is_backtracking)
        reset_backtrack_cmd(true, false, true, true);
  
      XtPopup(pp_plot_shell, XtGrabNone);
      XRaiseWindow(display, XtWindow(pp_plot_shell));
  
      if (firsttime)
        set_wm_protocols(pp_plot_shell);
  
      if (return_to_bitmap)
      {
        XDefineCursor(display, XtWindow(pp_plot_box1), crosshair_cursor);
        XFlush(display);
        XSync(display, False);
      }
      else
      {
        XDefineCursor(display, XtWindow(pp_plot_box1), default_cursor);
        XFlush(display);
        XSync(display, False);
      }
  
      XtVaGetValues(xg->pp_plot_wksp,
        XtNwidth, &pp_wksp.width,
        XtNheight, &pp_wksp.height, NULL);
  
      pp_plot_window = XtWindow(xg->pp_plot_wksp);
      make_pp_pixmap();
  
      ftmp = (float) ((int) pp_wksp.width - (int) xg->xaxis_indent);
      max_pp_storage = (int) ((1.-PP_PLOTMARGIN_TOP) * ftmp);
  
  /*
   * allocating these to be max_pp_storage+1 in size since it
   * seems to fix a funny allocation bug that purify finds.
  */
      pp_index_array = (float *) XtMalloc(
        (unsigned int) (max_pp_storage+1) * sizeof(float));
      pp_index_rect = (XRectangle *) XtMalloc(
        (unsigned int) (max_pp_storage+1) * sizeof(XRectangle));
      pp_index_pts = (XPoint *) XtMalloc(
        (unsigned int) (max_pp_storage+1) * sizeof(XPoint));
      new_basis_lines = (int *) XtMalloc(
        (unsigned int) (max_pp_storage+1) * sizeof(int));
  
      bitmaps = (XPoint **) XtMalloc(
        (unsigned int) MAX_NBITMAPS * sizeof(XPoint *));
      for (i=0; i<MAX_NBITMAPS; i++)
        bitmaps[i] = (XPoint *) XtMalloc(
          (unsigned int) xg->nrows*sizeof(XPoint));
      bitmap_frame = (int *) XtMalloc(
        (unsigned int) MAX_NBITMAPS * sizeof(int));
      npts_in_bitmap = (int *) XtMalloc(
        (unsigned int) xg->nrows*sizeof(int));
      pts_in_bitmap = (int **) XtMalloc(
        (unsigned int) MAX_NBITMAPS * sizeof(int *));
      for (i=0; i<MAX_NBITMAPS; i++)
        pts_in_bitmap[i] = (int *) XtMalloc(
          (unsigned int) xg->nrows*sizeof(int));
        
      optimz_circs = (int *) XtMalloc(
        (unsigned int) MAX_NOPTIMZ_CIRCS * sizeof(int));
  
      bitmap_basis0 = (float **) XtMalloc(
        (unsigned int) MAX_NBITMAPS * sizeof(float *));
      for (i=0; i<MAX_NBITMAPS; i++)
        bitmap_basis0[i] = (float *) XtMalloc(
          (unsigned int) xg->ncols_used*sizeof(float));
      bitmap_basis1 = (float **) XtMalloc(
        (unsigned int) MAX_NBITMAPS * sizeof(float *));
      for (i=0; i<MAX_NBITMAPS; i++)
        bitmap_basis1[i] = (float *) XtMalloc(
          (unsigned int) xg->ncols_used*sizeof(float));
  
      pp_index(xg,0,0);
      if (xg->is_stepping)
      {
        set_ready_to_stop_now(1);
        set_counting_to_stop(0);
      }
      if (firsttime)
      {
        bitmap_size = FLOAT(bitmap_percent) * FLOAT(pp_wksp.height) / 100.;
        bitmap_space = bitmap_size + 30;
        firsttime = 0;
      }
  
      XtMapWidget(TOUR_PP_PANEL);
    }
  }

  setToggleBitmap(w, xg->is_pp);
}

/* Save jumpsize-between-base-changes code because we'd like
 * to do fixed jumpsize tour at some stage. */
/*void
set_jumpsize(float slidepos, xgobidata *xg)
{
  char str[64];
  float tmpf;
*
 * scrollbar relationship is f(x) = exp((x*.4375)^2)-0.9999
*
  tmpf = slidepos * 0.4375;
  tmpf *= tmpf;

  xg->jumpsz = (float) exp((double) tmpf) - 0.9999;
  sprintf(str, "%s: %5.4f", "Jumpsz", xg->jumpsz);
  XtVaSetValues(PP_JUMPSZ_LABEL, XtNstring, (String) str, NULL);

}*/

/* ARGSUSED */
/*XtCallbackProc
tour_jumpsize_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  float slidepos = * (float *) slideposp;

  set_jumpsize(slidepos, xg);
}*/

void
set_termsinexpan(xgobidata *xg, float slidepos)
{
  char str[64];
  char ftitle[64];

  lJ = (int) ((float) (MAXLJ-1-MINLJ)*slidepos + (float) MINLJ);
  sprintf(str, "%s: %d", "TermsinExp", lJ);
  XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

  sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
    index_names[xg->pp_index_btn],"(",lJ-1,")","Index");
  XtVaSetValues(pp_plot_shell,
    XtNtitle, (String) ftitle,
    XtNiconName, (String) ftitle, NULL);
}

void
draw_bandwidth(xgobidata *xg)
{
/*   float bandwidth; */
  int on_plot_bandwidth;

/* Draw kernel bandwidth on window */
  if (xg->is_tour_paused == True)
  {
    on_plot_bandwidth = (int) ((bandwidth / (xg->lim[0].max-xg->lim[0].min)) *
      (float) PRECISION1);
    on_plot_bandwidth = (int)((on_plot_bandwidth * xg->is.x) >> EXP1);
    XCopyArea(display, xg->pixmap0, xg->plot_window, copy_GC,
      0, 0, xg->plotsize.width, xg->plotsize.height, 0, 0);
    XDrawArc(display, xg->plot_window, copy_GC, 10, 10,
      (unsigned int) on_plot_bandwidth*2,
      (unsigned int) on_plot_bandwidth*2,
      0, 360*64);
  }
}

void
possibly_draw_bandwidth(xgobidata *xg)
{
  if (xg->pp_index_btn == FTS_BTN ||
    (xg->pp_index_btn == ENTROPY_BTN) || 
    xg->pp_index_btn == BIN_ENTROPY_BTN ||
    xg->pp_index_btn == BIN_FTS_BTN)
      draw_bandwidth(xg);
}

void
set_bandwidth(xgobidata *xg, float slidepos)
{
  char str[64];

  bandwidth = slidepos*(max_bandwidth-min_bandwidth) + min_bandwidth;

  sprintf(str, "%s: %5.3f", "BandWidth", bandwidth);
  XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

   draw_bandwidth(xg);
}

/* BUILD_YOUR_OWN_INDEX:
void
set_param(float slidepos)
{
  char str[64];

  param = something calculated from 0.-1. slidepos;

  sprintf(str, "%s: %5.3f", "Param", param);
  XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);
}
*/

/* ARGSUSED */
XtCallbackProc
tour_termsinexpan_cback(Widget w, xgobidata *xg, XtPointer slideposp)
{
  float slidepos = * (float *) slideposp;

  if (xg->pp_index_btn == LEGENDRE_BTN ||
    (xg->pp_index_btn == HERMITE_BTN) ||
    (xg->pp_index_btn == NATURAL_HERMITE_BTN))
  {
    set_termsinexpan(xg, slidepos);
  }
  else if (xg->pp_index_btn == ENTROPY_BTN ||
    (xg->pp_index_btn == FTS_BTN) || xg->pp_index_btn == BIN_ENTROPY_BTN ||
      xg->pp_index_btn == BIN_FTS_BTN)
  {
    set_bandwidth(xg, slidepos);
  }
/* BUILD_YOUR_OWN_INDEX:
  else if (xg->pp_index_btn == DUMMY_BTN)
    set_param(slidepos);
*/

  xg->recalc_max_min = True;
  if (xg->is_pp)
  {
    reset_pp_plot();
    pp_index(xg, 0, 0);
  }

  if (xg->is_pp_optimz)
    xg->new_direction_flag = True;
}

/* ARGSUSED */
XtEventHandler
tour_termsinexpan_rightarr_cback(Widget w, xgobidata *xg, XEvent *evnt,
Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    float fslidepos;
    char str[64];
    char ftitle[64];

    if ((xg->pp_index_btn == LEGENDRE_BTN) ||
      (xg->pp_index_btn == HERMITE_BTN) ||
      (xg->pp_index_btn == NATURAL_HERMITE_BTN))
    {
      if (lJ < MAXLJ-1)
      {
        lJ++;
        fslidepos = (float) (lJ - MINLJ) / (float) (MAXLJ - MINLJ);
        XawScrollbarSetThumb(PP_TERMSINEXP, fslidepos, -1.);

        sprintf(str, "%s: %d", "TermsinExp", lJ);
        XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

        sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
          index_names[xg->pp_index_btn],"(",lJ-1,")","Index");

        XtVaSetValues(pp_plot_shell,
          XtNtitle, (String) ftitle,
          XtNiconName, (String) ftitle, NULL);
      }
    }
    else if ((xg->pp_index_btn == FTS_BTN) ||
      (xg->pp_index_btn == ENTROPY_BTN) || 
       xg->pp_index_btn == BIN_ENTROPY_BTN ||
       xg->pp_index_btn == BIN_FTS_BTN)
    {
      if (bandwidth < max_bandwidth-(0.05*(max_bandwidth-min_bandwidth)))
      {
        bandwidth += (0.05*(max_bandwidth-min_bandwidth));
        fslidepos = (bandwidth-min_bandwidth)/(max_bandwidth-min_bandwidth);
        XawScrollbarSetThumb(PP_TERMSINEXP, fslidepos, -1.);

        sprintf(str, "%s: %5.3f", "BandWidth", bandwidth);
        XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

        draw_bandwidth(xg);
      }
    }
/* BUILD_YOUR_OWN_INDEX:
    else if (xg->pp_index_btn == DUMMY_BTN)
    {
      ***calculate param incrementally upwards***

        sprintf(str, "%s: %5.3f", "Param", param);
        XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);
      }
    }
*/

    xg->recalc_max_min = True;
    if (xg->is_pp)
    {
      reset_pp_plot();
      pp_index(xg, 0, 0);
    }

    if (xg->is_pp_optimz)
      xg->new_direction_flag = True;
  }
}

/* ARGSUSED */
XtEventHandler
tour_termsinexpan_leftarr_cback(Widget w, xgobidata *xg, XEvent *evnt,
Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    float fslidepos;
    char str[64];
    char ftitle[64];

    if ((xg->pp_index_btn == HERMITE_BTN) ||
      (xg->pp_index_btn == NATURAL_HERMITE_BTN))
    {
      if (lJ > MINLJ)
      {
        lJ--;
        fslidepos = (float) (lJ - MINLJ) / (float) (MAXLJ - MINLJ);
        XawScrollbarSetThumb(PP_TERMSINEXP, fslidepos, -1.);

        sprintf(str, "%s: %d", "TermsinExp", lJ);
        XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

        sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
          index_names[xg->pp_index_btn], "(",lJ-1,")", "Index");

        XtVaSetValues(pp_plot_shell,
          XtNtitle, (String) ftitle,
          XtNiconName, (String) ftitle, NULL);
      }
    }
    else if (xg->pp_index_btn == LEGENDRE_BTN)
    {
      if (lJ > 2)
      {
        lJ--;
        fslidepos = (float) (lJ - MINLJ) / (float) (MAXLJ - MINLJ);
        XawScrollbarSetThumb(PP_TERMSINEXP, fslidepos, -1.);

        sprintf(str, "%s: %d", "TermsinExp", lJ);
        XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

        sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
          index_names[xg->pp_index_btn],"(",lJ-1,")","Index");

        XtVaSetValues(pp_plot_shell,
          XtNtitle, (String) ftitle,
          XtNiconName, (String) ftitle, NULL);
      }
    }
    else if ((xg->pp_index_btn == FTS_BTN) ||
      (xg->pp_index_btn == ENTROPY_BTN) || 
      xg->pp_index_btn == BIN_ENTROPY_BTN ||
      xg->pp_index_btn == BIN_FTS_BTN)
    {
      if (bandwidth > (min_bandwidth+(0.05*(max_bandwidth-min_bandwidth))))
      {
        bandwidth -= (0.05*(max_bandwidth-min_bandwidth));
        fslidepos = (bandwidth-min_bandwidth)/(max_bandwidth-min_bandwidth);
        XawScrollbarSetThumb(PP_TERMSINEXP, fslidepos, -1.);

        sprintf(str, "%s: %5.3f", "BandWidth", bandwidth);
        XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

        draw_bandwidth(xg);
      }
    }
/* BUILD_YOUR_OWN_INDEX:
    else if (xg->pp_index_btn == DUMMY_BTN)
    {
      ***calculate param incrementally downwards***

        sprintf(str, "%s: %5.3f", "Param", param);
        XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);
    }
*/

    xg->recalc_max_min = True;
    if (xg->is_pp)
    {
      reset_pp_plot();
      pp_index(xg, 0, 0);
    }

    if (xg->is_pp_optimz)
      xg->new_direction_flag = True;
  }
}

/* ARGSUSED */
XtCallbackProc
tour_makebitmap_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This callback invokes scanning back through the history list.
*/
{
  int i, k, m;

  /* save current basis into bitmap_basis for later reconstruction
   *   of views.
  */
  for (k=0; k<xg->ncols_used; k++)
  {
    bitmap_basis0[nbitmaps][k] = xg->u[0][k];
    bitmap_basis1[nbitmaps][k] = xg->u[1][k];
  }

  /*
   * This covers the case where the user changes the number of
   * rows in the plot in between the plotting of two bitmaps.
  */
  npts_in_bitmap[nbitmaps] = xg->nrows_in_plot;
  for (i=0; i<xg->nrows_in_plot; i++)
  {
    m = xg->rows_in_plot[i];
    pts_in_bitmap[nbitmaps][i] = m;
  }

  proj_scale_bitmap_pts(xg, nbitmaps, npts_in_bitmap[nbitmaps],
    pts_in_bitmap[nbitmaps]);
  bitmap_frame[nbitmaps] = (count-1)*pp_bw_points + xg->xaxis_indent -
    bitmap_size + 3;/* right hand corner */
  shift_bitmap_pts(xg, nbitmaps, npts_in_bitmap[nbitmaps]);

/* plot the data into pp window */
  for (i=0; i<npts_in_bitmap[nbitmaps]; i++)
  {
    xpts[i].x = bitmaps[nbitmaps][i].x;
    xpts[i].y = bitmaps[nbitmaps][i].y;
  }
  plot_bitmap(bitmap_frame[nbitmaps], npts_in_bitmap[nbitmaps], 1);
  nbitmaps++;
}

void
set_termsinexp_sens(Boolean sens)
{
  Arg args[5];

  XtSetArg(args[0], XtNsensitive, sens);
  XtSetValues(PP_TERMSINEXP, args, 1);
  XtSetValues(nterms_rightarr, args, 1);
  XtSetValues(nterms_leftarr, args, 1);
}

void
reset_scrollbar(xgobidata *xg)
{
  char str[32];
  float fslidepos;
  char ftitle[64];

  if ((xg->pp_index_btn == HERMITE_BTN) ||
      (xg->pp_index_btn == NATURAL_HERMITE_BTN))
  {
    lJ = 1;
    fslidepos = (float) (lJ - MINLJ) / (float) (MAXLJ - MINLJ);
    XawScrollbarSetThumb(PP_TERMSINEXP, fslidepos, -1.);

    sprintf(str, "%s: %d", "TermsinExp", lJ);
    XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

    sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
      index_names[xg->pp_index_btn],"(",lJ-1,")","Index");
    XtVaSetValues(pp_plot_shell,
      XtNtitle, (String) ftitle,
      XtNiconName, (String) ftitle, NULL);

    set_termsinexp_sens(1);
  }
  else if ((xg->pp_index_btn == CENTRAL_MASS_BTN) ||
           (xg->pp_index_btn == HOLES_BTN) ||
           (xg->pp_index_btn == SKEWNESS_BTN) )
  {
    set_termsinexp_sens(0);
  }
  else if (xg->pp_index_btn == LEGENDRE_BTN)
  {
    lJ = 6;
    fslidepos = (float) (lJ - MINLJ) / (float) (MAXLJ - MINLJ);
    XawScrollbarSetThumb(PP_TERMSINEXP, fslidepos, -1.);

    sprintf(str, "%s: %d", "TermsinExp", lJ);
    XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

    sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
      index_names[xg->pp_index_btn],"(",lJ-1,")","Index");
    XtVaSetValues(pp_plot_shell,
      XtNtitle, (String) ftitle,
      XtNiconName, (String) ftitle, NULL);

    set_termsinexp_sens(1);
  }

  else if ((xg->pp_index_btn == FTS_BTN) ||
        (xg->pp_index_btn == ENTROPY_BTN) || 
        xg->pp_index_btn == BIN_ENTROPY_BTN ||
     xg->pp_index_btn == BIN_FTS_BTN) 
  {
    bandwidth = optimal_bandwidth;
    fslidepos = (bandwidth-min_bandwidth)/(max_bandwidth-min_bandwidth);
    XawScrollbarSetThumb(PP_TERMSINEXP, fslidepos, -1.);

    sprintf(str, "%s: %5.3f", "BandWidth", bandwidth);
    XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

    draw_bandwidth(xg);

    set_termsinexp_sens(1);
  }
/* BUILD_YOUR_OWN_INDEX:
  else if (xg->pp_index_btn == DUMMY_BTN)
  {
    *** set position of scrollbar, for eg, for change of index***

    sprintf(str, "%s: %5.3f", "Param", param);
    XtVaSetValues(PP_TERMSINEXP_LABEL, XtNstring, (String) str, NULL);

    set_termsinexp_sens(1);
    set_subsetterms_sens(0);
  }
*/
}

/* ARGSUSED */
XtCallbackProc
choose_index_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int j;
  int old_index_btn;
  char ftitle[128];

  old_index_btn = xg->pp_index_btn;

  for (j=0; j<NUM_INDICES; j++)
  {
    if (ppindex_menu_btn[j] == w)
    {
      xg->pp_index_btn = j;
      break;
    }
  }

  if ((old_index_btn - xg->pp_index_btn) != 0)
  {
    XtVaSetValues(ppindex_menu_btn[old_index_btn],
      XtNleftBitmap, None, NULL);

    XtVaSetValues(ppindex_menu_btn[xg->pp_index_btn],
      XtNleftBitmap, menu_mark, NULL);

    xg->recalc_max_min = True;
    reset_pp_plot();

    reset_scrollbar(xg);

    if ((xg->pp_index_btn == LEGENDRE_BTN) ||
        (xg->pp_index_btn == HERMITE_BTN) ||
        (xg->pp_index_btn == NATURAL_HERMITE_BTN))
    {
      sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
        index_names[xg->pp_index_btn],"(",lJ-1,")","Index");
      XtVaSetValues(pp_plot_shell,
        XtNtitle, (String) ftitle,
        XtNiconName, (String) ftitle, NULL);
    }
/* BUILD_YOUR_OWN_INDEX:
    else if (xg->pp_index_btn == DUMMY_BTN)
    {
      sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
        index_names[xg->pp_index_btn],"(",param,")","Index");
      XtVaSetValues(pp_plot_shell,
        XtNtitle, (String) ftitle,
        XtNiconName, (String) ftitle, NULL);
    }
*/
    else
    {
      sprintf(ftitle, "%s %s %s", "Projection Pursuit:",
        index_names[xg->pp_index_btn],"Index");
      XtVaSetValues(pp_plot_shell,
        XtNtitle, (String) ftitle,
        XtNiconName, (String) ftitle, NULL);
    }

    if (xg->is_pp)
    {
      if (old_index_btn == NATURAL_HERMITE_BTN)
        free_natural_hermite((int) MAXLJ);
      else if (old_index_btn == HERMITE_BTN)
        free_hermite((int) MAXLJ);
      else if (old_index_btn == CENTRAL_MASS_BTN)
        free_central_mass();
      else if (old_index_btn == HOLES_BTN)
        free_holes();
      else if (old_index_btn == SKEWNESS_BTN)
        free_skewness();
      else if (old_index_btn == LEGENDRE_BTN)
        free_legendre((int) MAXLJ);
      else if (old_index_btn == BIN_FTS_BTN)
        free_bin_fts();
      else if (old_index_btn == BIN_ENTROPY_BTN)
        free_bin_entropy();
/* BUILD_YOUR_OWN_INDEX:
      else if (old_index_btn == DUMMY_BTN)
        free_dummy();
*/

      if (xg->pp_index_btn == NATURAL_HERMITE_BTN)
        alloc_natural_hermite(xg->nrows, (int) MAXLJ);
      else if (xg->pp_index_btn == HERMITE_BTN)
        alloc_hermite(xg->nrows, (int) MAXLJ);
      else if (xg->pp_index_btn == CENTRAL_MASS_BTN)
        alloc_central_mass(xg->nrows);
      else if (xg->pp_index_btn == HOLES_BTN)
        alloc_holes(xg->nrows);
      else if (xg->pp_index_btn == SKEWNESS_BTN)
        alloc_skewness(xg->nrows);
      else if (xg->pp_index_btn == LEGENDRE_BTN)
        alloc_legendre(xg->nrows, (int) MAXLJ);
      else if (xg->pp_index_btn == BIN_FTS_BTN)
        alloc_bin_fts(xg->nrows);
      else if (xg->pp_index_btn == BIN_ENTROPY_BTN)
        alloc_bin_entropy(xg->nrows);
/* BUILD_YOUR_OWN_INDEX:
      else if (xg->pp_index_btn == DUMMY_BTN)
        alloc_dummy(xg->nrows);
*/

      pp_index(xg,0,0);

      zero_tau(xg);
      if (xg->is_pp_optimz)
        xg->new_direction_flag = True;
    }

    if (xg->run_tour_proc == False)
      if (!xg->is_tour_paused && !xg->is_stepping)
        start_tour_proc(xg);
  }
}

/* ARGSUSED */
XtCallbackProc
pp_showlines_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This callback determines whether lines are plotted in the pp plot
*/
{
  is_pp_lines = 1 - is_pp_lines;
  setToggleBitmap(w, is_pp_lines);

  pp_plot(xg,Ihat,0,0,1);
}

/* ARGSUSED */
XtCallbackProc
pp_showpoints_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This callback determines whether points are plotted in the pp plot
*/
{
  is_pp_points = 1 - is_pp_points;
  setToggleBitmap(w, is_pp_points);

  pp_plot(xg,Ihat,0,0,1);
}

/* ARGSUSED */
XtEventHandler
pp_linewidth_down_cback(Widget w, xgobidata *xg, XEvent *evnt,
Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    char str[64];

    if (pp_line_width > 0)
    {
      pp_line_width--;

      sprintf(str, "%s: %d", "Width", pp_line_width);
      XtVaSetValues(PP_LINEWIDTH_LABEL, XtNstring, (String) str, NULL);

      pp_plot(xg,Ihat,0,0,1);
    }
  }
}

/* ARGSUSED */
XtEventHandler
pp_linewidth_up_cback(Widget w, xgobidata *xg, XEvent *evnt,
Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    char str[64];

    if (pp_line_width < 4)
    {
      pp_line_width++;

      sprintf(str, "%s: %d", "Width", pp_line_width);
      XtVaSetValues(PP_LINEWIDTH_LABEL, XtNstring, (String) str, NULL);

      pp_plot(xg,Ihat,0,0,1);
    }
  }
}

/* ARGSUSED */
XtEventHandler
pp_bwpoints_down_cback(Widget w, xgobidata *xg, XEvent *evnt,
Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    char str[64];
    int k, k1;

    if (pp_bw_points > 1)
    {
      pp_bw_points--;

      sprintf(str, "%s: %2d", "Scale X Axis", pp_bw_points);
      XtVaSetValues(PP_BW_POINTS_LABEL, XtNstring, (String) str, NULL);

      for (k=0; k<count; k++)
      {
        pp_index_rect[k].x -= xg->xaxis_indent;
        pp_index_rect[k].x += 1;
        pp_index_rect[k].x /= (pp_bw_points+1);
        pp_index_rect[k].x *= pp_bw_points;
        pp_index_rect[k].x += xg->xaxis_indent;
        pp_index_rect[k].x -= 1;
        pp_index_pts[k].x -= xg->xaxis_indent;
        pp_index_pts[k].x /= (pp_bw_points+1);
        pp_index_pts[k].x *= pp_bw_points;
        pp_index_pts[k].x += xg->xaxis_indent;
      }
      for (k=0; k<nnew_bases; k++)
      {
        new_basis_lines[k] -= xg->xaxis_indent;
        new_basis_lines[k] /= (pp_bw_points+1);
        new_basis_lines[k] *= pp_bw_points;
        new_basis_lines[k] += xg->xaxis_indent;
      }
      for (k=0; k<noptimz_circs; k++)
      {
        optimz_circs[k] -= xg->xaxis_indent;
        optimz_circs[k] /= (pp_bw_points+1);
        optimz_circs[k] *= pp_bw_points;
        optimz_circs[k] += xg->xaxis_indent;
      }
      for (k1=0; k1<nbitmaps; k1++)
      {
        bitmap_frame[k1] -= (xg->xaxis_indent - bitmap_size + 3);
        bitmap_frame[k1] /= (pp_bw_points+1);
        bitmap_frame[k1] *= pp_bw_points;
        bitmap_frame[k1] += (xg->xaxis_indent - bitmap_size + 3);
        proj_scale_bitmap_pts(xg, k1, npts_in_bitmap[k1],
          pts_in_bitmap[k1]);
        shift_bitmap_pts(xg, k1, npts_in_bitmap[k1]);
      }

      pp_plot(xg,Ihat,0,0,1);
    }
  }
}

/* ARGSUSED */
XtEventHandler
pp_bwpoints_up_cback(Widget w, xgobidata *xg, XEvent *evnt,
Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    char str[64];
    int k, k1;

    if ((pp_bw_points < 10) && (count*(pp_bw_points+1) <= max_pp_storage))
    {
      pp_bw_points++;

      sprintf(str, "%s: %2d", "Scale X Axis", pp_bw_points);
      XtVaSetValues(PP_BW_POINTS_LABEL, XtNstring, (String) str, NULL);

      for (k=0; k<count; k++)
      {
        pp_index_rect[k].x -= xg->xaxis_indent;
        pp_index_rect[k].x += 1;
        pp_index_rect[k].x /= (pp_bw_points-1);
        pp_index_rect[k].x *= pp_bw_points;
        pp_index_rect[k].x += xg->xaxis_indent;
        pp_index_rect[k].x -= 1;
        pp_index_pts[k].x -= xg->xaxis_indent;
        pp_index_pts[k].x /= (pp_bw_points-1);
        pp_index_pts[k].x *= pp_bw_points;
        pp_index_pts[k].x += xg->xaxis_indent;
      }
      for (k=0; k<nnew_bases; k++)
      {
        new_basis_lines[k] -= xg->xaxis_indent;
        new_basis_lines[k] /= (pp_bw_points-1);
        new_basis_lines[k] *= pp_bw_points;
        new_basis_lines[k] += xg->xaxis_indent;
      }
      for (k=0; k<noptimz_circs; k++)
      {
        optimz_circs[k] -= xg->xaxis_indent;
        optimz_circs[k] /= (pp_bw_points-1);
        optimz_circs[k] *= pp_bw_points;
        optimz_circs[k] += xg->xaxis_indent;
      }
      for (k1=0; k1<nbitmaps; k1++)
      {
        bitmap_frame[k1] -= (xg->xaxis_indent - bitmap_size + 3);
        bitmap_frame[k1] /= (pp_bw_points-1);
        bitmap_frame[k1] *= pp_bw_points;
        bitmap_frame[k1] += (xg->xaxis_indent - bitmap_size + 3);
        proj_scale_bitmap_pts(xg, k1, npts_in_bitmap[k1],
          pts_in_bitmap[k1]);
        shift_bitmap_pts(xg, k1, npts_in_bitmap[k1]);
      }
      pp_plot(xg,Ihat,0,0,1);
    }
  }
}

/* ARGSUSED */
XtEventHandler
pp_bitmapsz_down_cback(Widget w, xgobidata *xg, XEvent *evnt,
Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    char str[64];
    int i, bitmap_inc;

    bitmap_inc = (int)(0.1*(float)pp_wksp.height);
    if (bitmap_percent > 0.1)
    {
      bitmap_percent -= 10;
      bitmap_size = FLOAT(bitmap_percent)*FLOAT(pp_wksp.height)/100;
      sprintf(str, "%s: %d", "Bitmap Size", bitmap_percent);
      XtVaSetValues(PP_BITMAPSZ_LABEL, XtNstring, (String) str, NULL);

      for (i=0; i<nbitmaps; i++)
      {
        bitmap_frame[i] += bitmap_inc;
        proj_scale_bitmap_pts(xg, i, npts_in_bitmap[i],
          pts_in_bitmap[i]);
        shift_bitmap_pts(xg, i, npts_in_bitmap[i]);
      }
      bitmap_space = bitmap_size + 30;  /* 2*15 */
      pp_plot(xg, Ihat, 0, 0, 1);
    }
  }
}

/* ARGSUSED */
XtEventHandler
pp_bitmapsz_up_cback(Widget w, xgobidata *xg, XEvent *evnt,
Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    char str[64];
    int i, bitmap_inc;

    bitmap_inc = (int)(0.1*(float)pp_wksp.height);
    if (bitmap_percent < MAX_BITMAP_PERCENT)
    {
      bitmap_percent += 10;
      bitmap_size = FLOAT(bitmap_percent)*FLOAT(pp_wksp.height)/100.;
      sprintf(str, "%s: %d", "Bitmap Size", bitmap_percent);
      XtVaSetValues(PP_BITMAPSZ_LABEL, XtNstring, (String) str, NULL);

      for (i=0; i<nbitmaps; i++)
      {
        bitmap_frame[i] -= bitmap_inc;
        proj_scale_bitmap_pts(xg, i, npts_in_bitmap[i], pts_in_bitmap[i]);
        shift_bitmap_pts(xg, i, npts_in_bitmap[i]);
      }
      bitmap_space = bitmap_size + 30;  /* 2*15 */
      pp_plot(xg, Ihat, 0, 0, 1);
    }
  }
}

/* ARGSUSED */
XtEventHandler
pp_replot_down_cback(Widget w, xgobidata *xg, XEvent *evnt,
Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    char str[64];

    if (xg->pp_replot_freq > 1)
    {
      xg->pp_replot_freq--;

      sprintf(str, "%s: %d", "Replot Freq", xg->pp_replot_freq);
      XtVaSetValues(PP_RF_LABEL, XtNstring, (String) str, NULL);
    }
  }
}

/* ARGSUSED */
XtEventHandler
pp_replot_up_cback(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    char str[64];

    if (xg->pp_replot_freq < 40)
    {
      xg->pp_replot_freq++;

      sprintf(str, "%s: %d", "Replot Freq", xg->pp_replot_freq);
      XtVaSetValues(PP_RF_LABEL, XtNstring, (String) str, NULL);
    }
  }
}

/* ARGSUSED */
XtCallbackProc
tour_retnbm_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This callback invokes scanning back through the history list.
*/
{
  if (return_to_bitmap)
  {
    return_to_bitmap = 0;
    if (!record_bitmap)
    {
      XDefineCursor(display, XtWindow(pp_plot_box1), default_cursor);
      XFlush(display);
      XSync(display, False);
    }
  }
  else
  {
    return_to_bitmap = 1;
    XDefineCursor(display, XtWindow(pp_plot_box1), crosshair_cursor);
    XFlush(display);
    XSync(display, False);
  }
  setToggleBitmap(w, return_to_bitmap);
}

void
bm_cancel()
{
  record_bitmap = 0;
  reset_record_bitmaps(0);
}

Boolean
nogood_bm_filename(xgobidata *xg)
{
  Boolean ret_val = 0;

  if ( (bm_fp = fopen(bm_filename, "a")) == NULL)
  {
    char message[MSGLENGTH];
    sprintf(message,
      "Failed to open the file '%s' for writing.\n", bm_filename);
    show_message(message, xg);
    ret_val = 1;
  }

  return(ret_val);
}

void
read_bm_filename(Widget w, xgobidata *xg)
{
  char *filename;

  filename = XtMalloc(132 * sizeof(char));
  XtVaGetValues(w, XtNstring, &filename, NULL);
  strcpy(bm_filename, filename);
  if (nogood_bm_filename(xg))
    bm_cancel();

  XtFree(filename);
  start_tour_proc(xg);
}

Boolean
bm_file_open(xgobidata *xg)
{
  Boolean ret_val = 1;

  if ( (bm_fp = fopen(bm_filename, "a")) == NULL)
  {
    char message[MSGLENGTH];
    sprintf(message,
      "Failed to open the file '%s' for writing.\n", bm_filename);
    show_message(message, xg);
    bm_cancel();
    ret_val = 0;
  }
  return(ret_val);
}

void
bm_file_close(void)
{
  if (fclose(bm_fp) == EOF)
    fprintf(stderr, "tour_recordbm_cback");
}

/* ARGSUSED */
XtCallbackProc
tour_recordbm_cback(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * This callback invokes scanning back through the history list.
*/
{
  if (!record_bitmap)
  {
    record_bitmap = 1;
    bm_counter = 0;
    (void) strcpy(xg->save_type, OPEN_BITMAP_FILE );
    stop_tour_proc(xg);
    fname_popup(XtParent(w), xg);
    XDefineCursor(display, XtWindow(pp_plot_box1), crosshair_cursor);
    XFlush(display);
    XSync(display, False);
  }
  else
  {
    record_bitmap = 0;
    if (!return_to_bitmap)
    {
      XDefineCursor(display, XtWindow(pp_plot_box1), default_cursor);
      XFlush(display);
      XSync(display, False);
    }
  }

  setToggleBitmap(w, record_bitmap);
}

/* ARGSUSED */
XtEventHandler
pp_button_press(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  XButtonEvent *xbutton = (XButtonEvent *) evnt;

  if (xbutton->button == 1 || xbutton->button == 2)
  {
    int i, j, indx;
    int root_x, root_y, win_x, win_y;
    unsigned int kb;
    Window root, child;
    Boolean inside_bitmap = False;

    if ((record_bitmap) || (return_to_bitmap && !xg->is_pp_optimz))
    {
      if (XQueryPointer(display, pp_plot_window, &root, &child,
        &root_x, &root_y, &win_x, &win_y, &kb))
      {
        for (i=0; i<nbitmaps; i++)
        {
          if ((win_x > bitmap_frame[i]) &&
            (win_x < (bitmap_frame[i]+bitmap_size)) &&
            (win_y > (FLOAT(pp_wksp.height) - FLOAT(bitmap_space))))
          {
            inside_bitmap = True;
            indx = i;
            break;
          }
        }
      }
    }

    if (inside_bitmap)
    {
      if (return_to_bitmap && !xg->is_pp_optimz)
      {
        init_basis(xg);
        for (j=0; j<xg->ncols_used; j++)
        {
          xg->u1[0][j] = bitmap_basis0[indx][j];
          xg->u1[1][j] = bitmap_basis1[indx][j];
        }
        zero_tau(xg);
        got_new_basis = 1;
/* These lines make the tour stop once it reaches the chosen
   bitmap basis.
*/
        set_counting_to_stop(1);
        set_ready_to_stop_now(0);
      }
      if (record_bitmap)
      {
        if (bm_file_open(xg))
        {
          bm_counter++;
          (void) fprintf(bm_fp, "%d\n", bm_counter);
          for (j=0; j<xg->ncols_used; j++)
          {
            (void) fprintf(bm_fp, "%f %f\n",
               bitmap_basis0[indx][j], bitmap_basis1[indx][j]);
          }
          (void) fprintf(bm_fp, "\n");
          bm_file_close();
        }
      }
    }
  }
}

void
make_pp_plot(xgobidata *xg)
{
  int i;
  char ftitle[64], str[128];
  Dimension width;

  for (i=0; i<NUM_INDICES; i++)
    index_names[i] = (char *) XtMalloc((unsigned int) 32 * sizeof(char));

  strcpy(index_names[NATURAL_HERMITE_BTN], "Natural Hermite");
  strcpy(index_names[HERMITE_BTN], "Hermite");
  strcpy(index_names[CENTRAL_MASS_BTN], "Central Mass");
  strcpy(index_names[HOLES_BTN], "Holes");
  strcpy(index_names[SKEWNESS_BTN], "Skewness");
  strcpy(index_names[LEGENDRE_BTN], "Legendre");
  strcpy(index_names[FTS_BTN], "Friedman-Tukey");
  strcpy(index_names[ENTROPY_BTN], "Entropy");
  strcpy(index_names[BIN_FTS_BTN], "Binned Friedman-Tukey");
  strcpy(index_names[BIN_ENTROPY_BTN], "Binned Entropy");
/* BUILD_YOUR_OWN_INDEX:
  strcpy(index_names[DUMMY_BTN], "Dummy");
*/

  if ((xg->pp_index_btn == LEGENDRE_BTN) ||
      (xg->pp_index_btn == HERMITE_BTN) ||
      (xg->pp_index_btn == NATURAL_HERMITE_BTN))
  {
    sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
      index_names[xg->pp_index_btn],"(",lJ-1,")","Index");
  }
/* BUILD_YOUR_OWN_INDEX:
    else if (xg->pp_index_btn == DUMMY_BTN)
    {
      sprintf(ftitle, "%s %s %s%d%s %s", "Projection Pursuit:",
        index_names[xg->pp_index_btn],"(",param,")","Index");
    }
*/
  else
    sprintf(ftitle, "%s %s %s", "Projection Pursuit:",
      index_names[xg->pp_index_btn],"Index");

  pp_plot_shell = XtVaCreatePopupShell("PPshell",
    topLevelShellWidgetClass, PP_BTN,
    XtNtitle, (String) ftitle,
    XtNiconName, (String) ftitle,
    NULL);
  if (mono) set_mono(pp_plot_shell);

  pp_plot_form = XtVaCreateManagedWidget("PPForm",
    panedWidgetClass, pp_plot_shell,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtRubber,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtRubber,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(pp_plot_form);

  pp_plot_box0 = XtVaCreateManagedWidget("Box",
    formWidgetClass, pp_plot_form,
    NULL);
  if (mono) set_mono(pp_plot_box0);

  tour_pp_panel[1] = XtVaCreateManagedWidget("TourPPPlotPanel",
    formWidgetClass, pp_plot_box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) True,
    NULL);
  if (mono) set_mono(tour_pp_panel[1]);

/*
 * Panel for line width controls
*/
  tour_pp_panel[2] = XtVaCreateManagedWidget("TourPPPlotPanel",
    formWidgetClass, tour_pp_panel[1],
    NULL);
  if (mono) set_mono(tour_pp_panel[2]);

  PP_PLOT_LINES = (Widget) CreateToggle(xg, "Show Lines",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL, True, ANY_OF_MANY,
    tour_pp_panel[2], "TourPP_ShowLines");
  XtManageChild(PP_PLOT_LINES);
  XtAddCallback(PP_PLOT_LINES, XtNcallback,
    (XtCallbackProc) pp_showlines_cback, (XtPointer) xg);

/* arrows to line width in pp index plot
*/
  PP_LINEWIDTH_LEFTARR = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, tour_pp_panel[2],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromVert, (Widget) PP_PLOT_LINES,
    XtNfromHoriz, (Widget) NULL,
    XtNhorizDistance, (Dimension) 3,
    XtNvertDistance, (Dimension) 5 ,
    XtNbitmap, (Pixmap) leftarr,
    NULL);
  if (mono) set_mono(PP_LINEWIDTH_LEFTARR);
  add_pb_help(&xg->nhelpids.pb,
    PP_LINEWIDTH_LEFTARR, "TourPP_TermsinExp");
  XtAddEventHandler(PP_LINEWIDTH_LEFTARR, ButtonPressMask,
    FALSE, (XtEventHandler) pp_linewidth_down_cback, (XtPointer) xg);

  sprintf(str, "%s: %d", "Width", pp_line_width);
  width = XTextWidth(appdata.font, str, strlen(str));
  PP_LINEWIDTH_LABEL = XtVaCreateManagedWidget("TourLabel",
    asciiTextWidgetClass, tour_pp_panel[2],
    XtNstring, (String) str,
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNfromVert, (Widget) PP_PLOT_LINES,
    XtNfromHoriz, (Widget) PP_LINEWIDTH_LEFTARR,
    XtNhorizDistance, (Dimension) 0 ,
    NULL);
  if (mono) set_mono(PP_LINEWIDTH_LABEL);

  PP_LINEWIDTH_RIGHTARR = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, tour_pp_panel[2],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) rightarr,
    XtNfromHoriz, (Widget) PP_LINEWIDTH_LABEL,
    XtNhorizDistance, (Dimension) 0,
    XtNvertDistance, (Dimension) 5,
    XtNfromVert, (Widget) PP_PLOT_LINES,
    NULL);
  if (mono) set_mono(PP_LINEWIDTH_RIGHTARR);
  add_pb_help(&xg->nhelpids.pb,
    PP_LINEWIDTH_RIGHTARR, "TourPP_TermsinExp");
  XtAddEventHandler(PP_LINEWIDTH_RIGHTARR, ButtonPressMask,
    FALSE, (XtEventHandler) pp_linewidth_up_cback, (XtPointer) xg);

  PP_PLOT_POINTS = (Widget) CreateToggle(xg, "Show Points",
    True, (Widget) NULL, tour_pp_panel[2], (Widget) NULL, False, ANY_OF_MANY,
    tour_pp_panel[1], "TourPP_ShowPoints");
  XtManageChild(PP_PLOT_POINTS);
  XtAddCallback(PP_PLOT_POINTS, XtNcallback,
    (XtCallbackProc) pp_showpoints_cback, (XtPointer) xg);

/*
 * arrows for between points distance in pp index plot
*/
  PP_BW_POINTS_LEFTARR = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, tour_pp_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromVert, (Widget) PP_PLOT_POINTS,
    XtNfromHoriz, (Widget) NULL,
    XtNhorizDistance, (Dimension) 3,
    XtNvertDistance, (Dimension) 5,
    XtNbitmap, (Pixmap) leftarr,
    NULL);
  if (mono) set_mono(PP_BW_POINTS_LEFTARR);
  add_pb_help(&xg->nhelpids.pb,
    PP_BW_POINTS_LEFTARR, "ToPP_ScaleXAxis");
  XtAddEventHandler(PP_BW_POINTS_LEFTARR, ButtonPressMask,
    FALSE, (XtEventHandler) pp_bwpoints_down_cback, (XtPointer) xg);

  sprintf(str, "%s: %2d", "Scale X Axis", 10);  /* maximum value */
  width = XTextWidth(appdata.font, str, strlen(str));
  sprintf(str, "%s: %2d", "Scale X Axis", pp_bw_points);
  PP_BW_POINTS_LABEL = XtVaCreateManagedWidget("PPLabel",
    asciiTextWidgetClass, tour_pp_panel[1],
    XtNstring, (String) str,
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNfromVert, (Widget) PP_PLOT_POINTS,
    XtNfromHoriz, (Widget) PP_BW_POINTS_LEFTARR,
    XtNhorizDistance, (Dimension) 0,
    NULL);
  if (mono) set_mono(PP_BW_POINTS_LABEL);
  add_pb_help(&xg->nhelpids.pb,
    PP_BW_POINTS_LABEL, "ToPP_ScaleXAxis");

  PP_BW_POINTS_RIGHTARR = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, tour_pp_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) rightarr,
    XtNfromHoriz, (Widget) PP_BW_POINTS_LABEL,
    XtNhorizDistance, (Dimension) 0 ,
    XtNvertDistance, (Dimension) 5 ,
    XtNfromVert, (Widget) PP_PLOT_POINTS,
    NULL);
  if (mono) set_mono(PP_BW_POINTS_RIGHTARR);
  add_pb_help(&xg->nhelpids.pb,
    PP_BW_POINTS_RIGHTARR, "ToPP_ScaleXAxis");
  XtAddEventHandler(PP_BW_POINTS_RIGHTARR, ButtonPressMask,
    FALSE, (XtEventHandler) pp_bwpoints_up_cback, (XtPointer) xg);

/*
 * arrows for bitmap resizing in pp index plot
*/
  PP_BITMAPSZ_LEFTARR = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, tour_pp_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromVert, (Widget) PP_BW_POINTS_LABEL,
    XtNfromHoriz, (Widget) NULL,
    XtNhorizDistance, (Dimension) 3 ,
    XtNvertDistance, (Dimension) 5 ,
    XtNbitmap, (Pixmap) leftarr,
    NULL);
  if (mono) set_mono(PP_BITMAPSZ_LEFTARR);
  add_pb_help(&xg->nhelpids.pb,
    PP_BITMAPSZ_LEFTARR, "TourPP_BMSZ");
  XtAddEventHandler(PP_BITMAPSZ_LEFTARR, ButtonPressMask,
    FALSE, (XtEventHandler) pp_bitmapsz_down_cback, (XtPointer) xg);

  sprintf(str, "%s: %d", "Bitmap Size", MAX_BITMAP_PERCENT);
  width = XTextWidth(appdata.font, str, strlen(str));
  sprintf(str, "%s: %d", "Bitmap Size", bitmap_percent);
  PP_BITMAPSZ_LABEL = XtVaCreateManagedWidget("PPLabel",
    asciiTextWidgetClass, tour_pp_panel[1],
    XtNstring, (String) str,
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH) ,
    XtNfromVert, (Widget) PP_BW_POINTS_LABEL,
    XtNfromHoriz, (Widget) PP_BITMAPSZ_LEFTARR,
    XtNhorizDistance, (Dimension) 0,
    NULL);
  if (mono) set_mono(PP_BITMAPSZ_LABEL);
  add_pb_help(&xg->nhelpids.pb,
    PP_BITMAPSZ_LABEL, "TourPP_BMSZ");

  PP_BITMAPSZ_RIGHTARR = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, tour_pp_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) rightarr,
    XtNfromHoriz, (Widget) PP_BITMAPSZ_LABEL,
    XtNhorizDistance, (Dimension) 0 ,
    XtNvertDistance, (Dimension) 5 ,
    XtNfromVert, (Widget) PP_BW_POINTS_LABEL,
    NULL);
  if (mono) set_mono(PP_BITMAPSZ_RIGHTARR);
  add_pb_help(&xg->nhelpids.pb,
    PP_BITMAPSZ_RIGHTARR, "TourPP_BMSZ");
  XtAddEventHandler(PP_BITMAPSZ_RIGHTARR, ButtonPressMask,
    FALSE, (XtEventHandler) pp_bitmapsz_up_cback, (XtPointer) xg);

/* arrows to adjust plotting frequency of pp index
*/
  PP_RF_LEFTARR = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, tour_pp_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromVert, (Widget) PP_BITMAPSZ_LABEL,
    XtNvertDistance, (Dimension) 5 ,
    XtNhorizDistance, (Dimension) 3 ,
    XtNbitmap, (Pixmap) leftarr,
    NULL);
  if (mono) set_mono(PP_RF_LEFTARR);
  add_pb_help(&xg->nhelpids.pb,
    PP_RF_LEFTARR, "TourPP_RF");
  XtAddEventHandler(PP_RF_LEFTARR, ButtonPressMask,
    FALSE, (XtEventHandler) pp_replot_down_cback, (XtPointer) xg);

  /* xg->pp_replot_freq = 15; initialized in init_tour() */
  sprintf(str, "%s: %d", "Replot Freq", xg->pp_replot_freq);
  width = XTextWidth(appdata.font, str, strlen(str));
  PP_RF_LABEL = XtVaCreateManagedWidget("PPLabel",
    asciiTextWidgetClass, tour_pp_panel[1],
    XtNstring, (String) str,
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNfromVert, (Widget) PP_BITMAPSZ_LABEL ,
    XtNfromHoriz, (Widget) PP_RF_LEFTARR,
    XtNhorizDistance, (Dimension) 0,
    NULL);
  if (mono) set_mono(PP_RF_LABEL);
  add_pb_help(&xg->nhelpids.pb,
    PP_RF_LABEL, "TourPP_RF");

  PP_RF_RIGHTARR = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, tour_pp_panel[1],
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNbitmap, (Pixmap) rightarr,
    XtNfromHoriz, (Widget) PP_RF_LABEL,
    XtNvertDistance, (Dimension) 5 ,
    XtNhorizDistance, (Dimension) 0,
    XtNfromVert, (Widget) PP_BITMAPSZ_LABEL ,
    NULL);
  if (mono) set_mono(PP_RF_RIGHTARR);
  add_pb_help(&xg->nhelpids.pb,
    PP_RF_RIGHTARR, "TourPP_RF");
  XtAddEventHandler(PP_RF_RIGHTARR, ButtonPressMask,
    FALSE, (XtEventHandler) pp_replot_up_cback, (XtPointer) xg);

  PP_RETN_TO_BM = CreateToggle(xg, "Return to Bitmap",
    True, (Widget) NULL, (Widget) PP_RF_LABEL, (Widget) NULL,
    True, ANY_OF_MANY, tour_pp_panel[1], "TourPP_RetnToBM");
  XtManageChild(PP_RETN_TO_BM);
  XtAddCallback(PP_RETN_TO_BM, XtNcallback,
    (XtCallbackProc) tour_retnbm_cback, (XtPointer) xg);

  PP_RECORD_BM = CreateToggle(xg, "Record Bitmap",
    True, (Widget) NULL, (Widget) PP_RETN_TO_BM, (Widget) NULL,
    False, ANY_OF_MANY, tour_pp_panel[1], "TourPP_RecordBM");
  XtManageChild(PP_RECORD_BM);
  XtAddCallback(PP_RECORD_BM, XtNcallback,
    (XtCallbackProc) tour_recordbm_cback, (XtPointer) xg);

/* pp plot workspace */
  pp_plot_box1 = XtVaCreateManagedWidget("Box",
    formWidgetClass, pp_plot_form,
    NULL);
  if (mono) set_mono(pp_plot_box1);

  xg->pp_plot_wksp = XtVaCreateManagedWidget("PPplot",
    labelWidgetClass, pp_plot_box1,
    XtNresizable, (Boolean) True,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNright, (XtEdgeType) XtRubber,
    XtNbottom, (XtEdgeType) XtRubber,
    XtNlabel, (String) "",
    NULL);
  if (mono) set_mono(xg->pp_plot_wksp);

  XtAddEventHandler(xg->pp_plot_wksp,
    ButtonPressMask,
    FALSE, (XtEventHandler) pp_button_press, (XtPointer) xg);
  XtAddEventHandler(xg->pp_plot_wksp,
    ExposureMask,
    FALSE, (XtEventHandler) ppexpose_cback, (XtPointer) NULL);
  XtAddEventHandler(xg->pp_plot_wksp,
    StructureNotifyMask,
    FALSE, (XtEventHandler) ppresize_cback, (XtPointer) xg);
}

void
make_pp_menu(xgobidata *xg, Widget parent)
/*
 * Build a menu to contain projection pursuit index choices.
*/
{
  int j;

  ppindex_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "PP Index",
    XtNmenuName, (String) "Menu",
    XtNfromVert, (Widget) PP_TERMSINEXP,
    NULL);
  if (mono) set_mono(ppindex_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    ppindex_menu_cmd, "ToPP_IndexMenu");

  ppindex_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, ppindex_menu_cmd,
    NULL);
  if (mono) set_mono(ppindex_menu);
/*
 * index_names[] already initialized
*/
  for (j=0; j<NUM_INDICES; j++)
  {
    ppindex_menu_btn[j] = XtVaCreateWidget("Command",
      smeBSBObjectClass, ppindex_menu,
      XtNlabel, (String) index_names[j],
      XtNleftMargin, (Dimension) 24,
      NULL);
    if (mono) set_mono(ppindex_menu_btn[j]);
  }
  XtManageChildren(ppindex_menu_btn, NUM_INDICES);

  for (j=0; j<NUM_INDICES; j++)
    XtAddCallback(ppindex_menu_btn[j], XtNcallback,
      (XtCallbackProc) choose_index_cback, (XtPointer) xg);
}

void
make_pp_panel(xgobidata *xg, Widget panel)
{
  char str[64];
  Dimension width;

/*
 * Panel for projection pursuit: now initiate it unmapped, mapping
 * it only when PP is turned on.  This panel and the section tour
 * panel will occupy the same space.
*/
  TOUR_PP_PANEL = XtVaCreateManagedWidget("TourPPPanel",
    formWidgetClass, xg->box0,
    XtNfromVert, (Widget) panel,
    XtNmappedWhenManaged, (Boolean) False,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    NULL);
  if (mono) set_mono(TOUR_PP_PANEL);

/*
 * label to record projection pursuit index
*/

  sprintf(str, "PPIndx %.2e", -999.99);
  width = XTextWidth(appdata.font, str, strlen(str));

  PP_INDEX_LABEL = XtVaCreateManagedWidget("TourLabel",
    asciiTextWidgetClass, TOUR_PP_PANEL,
    XtNstring, (String) " ",
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNdisplayCaret, (Boolean) False,
    NULL);
  if (mono) set_mono(PP_INDEX_LABEL);

/*
 * Button to do active or passive projection pursuit
*/
  PP_OPTIMZ = CreateToggle(xg, "Optimz",
    True, (Widget) NULL, PP_INDEX_LABEL, (Widget) NULL, False,
    ANY_OF_MANY,
    TOUR_PP_PANEL, "ToPP_Optmz");
  XtManageChild(PP_OPTIMZ);
  XtAddCallback(PP_OPTIMZ, XtNcallback,
    (XtCallbackProc) tour_optimz_cback, (XtPointer) xg);

  PP_BITMAP = CreateCommand(xg, "Bitmap",
    True, PP_OPTIMZ, PP_INDEX_LABEL,
    TOUR_PP_PANEL, "ToPP_Bitmap");
  XtManageChild(PP_BITMAP);
  XtAddCallback(PP_BITMAP, XtNcallback,
    (XtCallbackProc) tour_makebitmap_cback, (XtPointer) xg);

/*
 * tour jumpsize-between-base-changes scrollbar
 * save this because would like to do fixed jumpsize tour
 * at some stage.
*
  sprintf(str, "%s: %5.4f", "Jumpsz", 0.0409);
  width = XTextWidth(appdata.font, str, strlen(str));

  xg->jumpsz = 0.0409;
  sprintf(str, "%s: %5.4f", "Jumpsz", xg->jumpsz);
  PP_JUMPSZ_LABEL = XtVaCreateManagedWidget("TourLabel",
    asciiTextWidgetClass, TOUR_PP_PANEL,
    XtNstring, (String) str,
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNfromVert, (Widget) PP_BITMAP,
    NULL);
  if (mono) set_mono(PP_JUMPSZ_LABEL);

  PP_JUMPSZ = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, TOUR_PP_PANEL,
    XtNfromVert, (Widget) PP_JUMPSZ_LABEL,
    XtNvertDistance, (int) 0,
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(PP_JUMPSZ);
  add_sbar_help(&xg->nhelpids.sbar,
    PP_JUMPSZ, "ToPP_Jumpsz");
  XawScrollbarSetThumb(PP_JUMPSZ, 0.452, -1.);
*/

/*
 * number of terms in Polynomial Expansion of empirical DF scrollbar
*/

  sprintf(str, "%s: %5.3f", "BandWidth", 0.0);
  width = XTextWidth(appdata.font, str, strlen(str));

  sprintf(str, "%s: %d", "TermsinExp", lJ);
  PP_TERMSINEXP_LABEL = XtVaCreateManagedWidget("TourLabel",
    asciiTextWidgetClass, TOUR_PP_PANEL,
    XtNstring, (String) str,
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH),
    XtNfromVert, (Widget) PP_OPTIMZ,
    NULL);
  if (mono) set_mono(PP_TERMSINEXP_LABEL);

  nterms_leftarr = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, TOUR_PP_PANEL,
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromVert, (Widget) PP_TERMSINEXP_LABEL,
    XtNvertDistance, (Dimension) 0,
    XtNbitmap, (Pixmap) leftarr,
    NULL);
  if (mono) set_mono(nterms_leftarr);
  add_pb_help(&xg->nhelpids.pb,
    nterms_leftarr, "ToPP_TermsExp");
  XtAddEventHandler(nterms_leftarr, ButtonPressMask,
    FALSE, (XtEventHandler) tour_termsinexpan_leftarr_cback,
    (XtPointer) xg);
/*
 * In the following, 16 is the width and height of the arrow icon
 * and 36 = 2 * (16 - border width).
*/
  PP_TERMSINEXP = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, TOUR_PP_PANEL,
    XtNfromVert, (Widget) PP_TERMSINEXP_LABEL,
    XtNfromHoriz, (Widget) nterms_leftarr,
    XtNwidth, (Dimension) (width + 2*ASCII_TEXT_BORDER_WIDTH - 36),
    XtNheight, (Dimension) 16,
    XtNhorizDistance, (Dimension) 0,
    XtNvertDistance, (int) 0,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(PP_TERMSINEXP);
  add_sbar_help(&xg->nhelpids.sbar,
    PP_TERMSINEXP, "ToPP_TermsExp");
  XawScrollbarSetThumb(PP_TERMSINEXP,
    (float) (lJ - MINLJ) / (float) MAXLJ, -1.);
  XtAddCallback(PP_TERMSINEXP, XtNjumpProc,
    (XtCallbackProc) tour_termsinexpan_cback, (XtPointer) xg);

  nterms_rightarr = XtVaCreateManagedWidget("Icon",
    labelWidgetClass, TOUR_PP_PANEL,
    XtNinternalHeight, (Dimension) 0,
    XtNinternalWidth, (Dimension) 0,
    XtNborderColor, (Pixel) appdata.fg,
    XtNfromVert, (Widget) PP_TERMSINEXP_LABEL,
    XtNfromHoriz, (Widget) PP_TERMSINEXP,
    XtNhorizDistance, (Dimension) 0,
    XtNvertDistance, (Dimension) 0,
    XtNbitmap, (Pixmap) rightarr,
    NULL);
  if (mono) set_mono(nterms_rightarr);
  add_pb_help(&xg->nhelpids.pb,
    nterms_rightarr, "ToPP_TermsExp");
  XtAddEventHandler(nterms_rightarr, ButtonPressMask,
    FALSE, (XtEventHandler) tour_termsinexpan_rightarr_cback,
    (XtPointer) xg);

  make_pp_plot(xg);

/*
 * make menu to select index, and make pushbutton to restart pp
 * at first two principal components
*/

  make_pp_menu(xg, TOUR_PP_PANEL);

}

void
init_tour_pp_menu(void)
{
  XtVaSetValues(ppindex_menu_btn[0],
    XtNleftBitmap, menu_mark,
    NULL);
}

int
derivs_equal_zero(xgobidata *xg)
{
  int i, j;
  float tmpf, tol = 0.0000001;
  int retn_val = 1;

  for (i=0; i<2; i++)
  {
    tmpf = 0.;
    for (j=0; j<xg->ncols_used; j++)
      tmpf += (dIhat[i][j]*dIhat[i][j]);
    tmpf /= (xg->numvars_t * (pp_index_max - pp_index_min));
    if (tmpf > tol)
    {
      retn_val = 0;
      break;
    }
  }
  return(retn_val);
}

void
spherize_data(xgobidata *xg, int num_pcs, int nsvars, int *svars)
{
  int i, j, k, m;
  float tmpf;

/*printf("in spherize\n");*/
/* spherize data */
/*  for (i=0; i<num_pcs; i++)
  {
    a[i] = sqrt((double) eigenval[i]);
    printf("%f ",a[i]);
  }
  printf("\n");
  */

  for (m=0; m<xg->nrows_in_plot; m++) {
    i = xg->rows_in_plot[m];

    for (j=0; j<num_pcs; j++) {
      tmpf = 0.;
      for (k=0; k<nsvars; k++) {
        tmpf = tmpf +
          vc_active[k][j] * (xg->tform1[i][svars[k]] - mean[svars[k]]);
      }
      tmpf /= (eigenval[j]);
      b[0][j] = tmpf;
    }
    for (j=0; j<num_pcs; j++)
      xg->tform2[i][svars[j]] = b[0][j];
  }
}

void
compute_vc_matrix(xgobidata *xg)
{
  float tmpf;
  int i, j, k;
  /*  int n = xg->nrows_in_plot, p = xg->ncols_used;
      11/23/99: this should only use the actual data points not
      the decoration points. */
  int n = xg->nlinkable_in_plot, p = xg->ncols_used; 

  /* bug fix, for sphering transformation: 
     this routine needs to use tform1 not tform2 */
/* calculate mean vector */
  for (i=0; i<p; i++) {
    tmpf = 0.;
    for (j=0; j<n; j++) 
      tmpf += xg->tform1[xg->rows_in_plot[j]][i];
    tmpf /= n;
    mean[i] = tmpf;
  }

/* calculate variance-covariance matrix */
  for (i=0; i<p; i++) {
    for (j=i; j<p; j++) {
      tmpf = 0.;
      for (k=0; k<n; k++) {
        tmpf = tmpf +
             (xg->tform1[xg->rows_in_plot[k]][i] - mean[i]) *
             (xg->tform1[xg->rows_in_plot[k]][j] - mean[j]);
      }
      tmpf /= (n-1);
      vc[i][j] = tmpf;
      vc[j][i] = vc[i][j];
    }
  }

}

/* This routine is used to reset one label
 * only. Currently this is only needed for
 * variable selection when the data is sphered.
 * Labels are change back and forwards from
 * variable name to PC #.
*/
void
reset_one_label(xgobidata *xg, int var, int reset_to_var_name)
{
  char str[COLLABLEN];
  Dimension width;
  int i;

  if (reset_to_var_name)
  {
    sprintf(str,"%s", xg->collab[var]);
    XtVaSetValues(xg->varlabw[var],
      XtNlabel, (String) str,
      NULL);
  }
  else
  {
    XtVaGetValues(xg->varlabw[var],
      XtNwidth, &width, NULL);
    for (i=0; i<xg->numvars_t; i++)
    {
      if (xg->tour_vars[i] == var)
      {
        sprintf(str, "PC %d", i+1);
        break;
      }
    }

    XtVaSetValues(xg->varlabw[var],
      XtNwidth, width,
      XtNlabel, (String) str,
      NULL);
  }
}

void
recalc_vc(int var, xgobidata *xg)
{
  int i, j;
  float tmpf = 0.;
  int n = xg->nlinkable_in_plot; /* 11/23/99 should only use actual data */

  /* bug fix, for sphering transformation: 
     this routine needs to use tform1 not tform2 */
  for (i=0; i<n; i++)
    tmpf += xg->tform1[xg->rows_in_plot[i]][var];
  mean[var] = tmpf / ((float)n);

  tmpf = 0.;
  for (i=0; i<xg->ncols_used; i++) {
    for (j=0; j<n; j++) {
      tmpf = tmpf +
        (xg->tform1[xg->rows_in_plot[j]][var] - mean[var]) *
        (xg->tform1[xg->rows_in_plot[j]][i] - mean[i]);
    }
    tmpf /= ((float)(n - 1));
    vc[var][i] = tmpf;
    vc[i][var] = tmpf;
  }

  for (i=0; i<xg->numvars_t; i++)
    for (j=0; j<xg->numvars_t; j++)
      vc_active[i][j] = vc[xg->tour_vars[i]][xg->tour_vars[j]];

  zero_ab(xg);
}

int
check_for_sphd_data(float **matrx, int n)
{
  int i, j;
  int retn_val = 1;
  float tol=0.001;

  for (i=0; i<n; i++)
    for (j=0; j<n; j++) {
      if ((i==j) && (fabs((double)(1.000-matrx[i][j]))>tol)) {
          retn_val = 0;
          break;
      }
      else if ((i!=j) && (fabs((double)matrx[i][j])>tol)) {
          retn_val = 0;
          break;
      }
    }
  return(retn_val);/* returns 1 if vc = I */
}

void
get_evals(int nevals, float *evals)
{
 int j;

 for (j=0; j<nevals; j++)
    evals[j] = eigenval[j];
}

/* this routine overwrites the var-cov matrix of the old active variables
 * with the new active variables. then this matrix is put through
 * singular value decomposition and overwritten with the matrix of
 * eigenvectors, and the eigenvalues are returned in a.
*/
int
update_vc_active_and_do_svd(xgobidata *xg, int nsvars, int *svars)
{
  int i, j, k;
  int vc_equals_I;
/*   char message[5*MSGLENGTH], str[100]; */
  paird *pairs;
  int rank;
  float *e;

  for (i=0; i<nsvars; i++)
    for (j=0; j<nsvars; j++)
      vc_active[i][j] = vc[svars[i]][svars[j]];

  zero_ab(xg);
  vc_equals_I = check_for_sphd_data(vc_active,nsvars);
  singular_vc = False;
  if (!vc_equals_I) 
  {
    dsvd(vc_active, nsvars, nsvars, a, b);
    for (i=0; i<nsvars; i++)
    {
      /*      eigenval[i] = a[i];*/
      eigenval[i] = sqrt((double) a[i]);
/*      printf("%f ",a[i]);*/
/*      if (eigenval[i] == 0 || eigenval[0]/eigenval[i] > 10000)
      {
        singular_vc = True;
      }*/
    }

    pairs = (paird *) XtMalloc (nsvars * sizeof (paird));
    e = (float *) XtMalloc (nsvars * sizeof (float));

    for (i=0; i<nsvars; i++) {
      pairs[i].f = (float) eigenval[i]; 
      pairs[i].indx = i;
    }
    qsort ((char *) pairs, nsvars, sizeof (paird), pcompare);

    /*-- sort the eigenvalues and eigenvectors into temporary arrays --*/
    for (i=0; i<nsvars; i++) {
      k = (nsvars - i) - 1;  /*-- to reverse the order --*/
      rank = pairs[i].indx;
      e[k] = eigenval[rank];
      for (j=0; j<nsvars; j++) {
        b[j][k] = vc_active[j][rank];/*-- note that this is reverse indexing
                                        compared to ggobi --*/
      }
      printf("%d ",rank);
    }
    /*-- copy the sorted eigenvalues and eigenvectors back --*/
    for (i=0; i<nsvars; i++) {
      eigenval[i] = e[i];
      for (j=0; j<nsvars; j++)
        vc_active[j][i] = b[j][i];
    }
    
    XtFree((XtPointer) pairs);
    XtFree((XtPointer) e);
/*    if (singular_vc)
    {
      vc_equals_I = 1;
      sprintf(message,
        "Variance covariance matrix is close to singular.\n");
      strcat(message,
        "Cannot sphere until a variable is dropped\n");
      strcat(message,
        "eigenvalues = ");
      sprintf(str, "%f ",eigenval[0]);
      strcat(message, str);
      for (j=1; j<nsvars; j++)
      {
        sprintf(str, "%f ",eigenval[j]);
        if (strlen(str) + strlen(message) >= 5*MSGLENGTH) {
          if (strlen(message) + 4 < 5*MSGLENGTH)
            strcat(message, "...");
          break;
        }
        else 
          strcat(message, str);
      }
      strcat(message,"\n");
      show_message(message, xg);
    }*/
  }

  printf("%d \n",vc_equals_I);
  
  return(!vc_equals_I);
}

void
invert_proj_coords(xgobidata *xg)
{
  int i, j, k;
  float tmpf;

  if (!singular_vc)
  {
    for (i=0; i<2; i++)
      for (j=0; j<xg->ncols_used; j++)
        xg->tv[i][j] = xg->u[i][j];
  
    for (i=0; i<xg->numvars_t; i++) {
      a[i] = eigenval[i];
      for (j=0; j<2; j++)
        xg->tv[j][xg->tour_vars[i]] *= a[i];
    }
  
    for (k=0; k<2; k++)
    {
      for (i=0; i<xg->numvars_t; i++)
      {
        tmpf = 0.;
  
        for (j=0; j<xg->numvars_t; j++)
          tmpf += (vc_active[i][j]*
            xg->tv[k][xg->tour_vars[j]]);
        b[k][i] = tmpf;
      }
      for (i=0; i<xg->numvars_t; i++)
        xg->tv[k][xg->tour_vars[i]] = b[k][i];
    }
    norm(xg->tv[0], xg->ncols_used);
    norm(xg->tv[1], xg->ncols_used);
  }
}

void
turn_off_optimz(xgobidata *xg)
{
  XtCallCallbacks(PP_OPTIMZ, XtNcallback, (XtPointer) xg);
  XtVaSetValues(PP_OPTIMZ, XtNstate, (Boolean) False, NULL);
  setToggleBitmap(PP_OPTIMZ, False);
}

void
turn_off_pp(xgobidata *xg)
{
/*
 * Turn off active projection pursuit.
*/
  if (xg->is_pp_optimz)
  {
    turn_off_optimz(xg);
  }
/*
 * Turn off the projection pursuit mode.
*/
  XtCallCallbacks(PP_BTN, XtNcallback, (XtPointer) xg);
  XtVaSetValues(PP_BTN, XtNstate, (Boolean) False, NULL);
  setToggleBitmap(PP_BTN, False);
}

void
turn_off_pc(xgobidata *xg)
{
/*
 * Turn off the principal components basis
*/
  if (xg->is_princ_comp)
  {
    xg->is_princ_comp = False;
    reset_princ_comp(False, xg);
  }
}

/* for printing pp window */

void
get_pp_win_dims(xgobidata *xg, float *max_x, float *max_y)
{
  if (xg->is_pp)
  {
    *max_x = (float) pp_wksp.width ;
    *max_y = (float) pp_wksp.height ;
  }
  else
    *max_x = *max_y = 0 ;
}

void
check_pp_fac_and_offsets(xgobidata *xg, float *minx, float *maxx, float *maxy,
float *fac, float *xoff, float *yoff, int pointsize)
{
  /*
   * Shift x axis.  The constant 4.5 comes from the header file.
  */
  while (FLOAT(72.*((*maxy - (FLOAT(pp_wksp.height) - FLOAT(bitmap_space))) *
        *fac + *yoff) - 4.0*pointsize) < FLOAT(72. * *yoff))
  {
    *maxy = 1.02 * *maxy ;
    set_fac_and_offsets(*minx, *maxx, *maxy, fac, xoff, yoff);
  }

  /*
   * Shift x axis.  The constant 4.5 comes from the header file.
  */
  while (72.*((xg->xaxis_indent - *minx) * *fac + *xoff) -
         4.0*pointsize < 72. * *xoff)
  {
    *minx = *minx - .02 * (*maxx - *minx) ;
    set_fac_and_offsets(*minx, *maxx, *maxy, fac, xoff, yoff);
  }
}

/*
 * I don't see why I need to have a prototype both here and in
 * xgobiexterns.h, but it does make things compile without comment.
*/
void
print_pp_win(xgobidata *xg, FILE *psfile, float minx, float maxy,
  float fac, float xoff, float yoff, XColor *fg)
{
  int i, k, m;
  char str[128];

  /*
   * Draw plot of the projection pursuit indices as a continous
   * line for now.
  */
  (void) fprintf(psfile, "%% ln: red green blue width x1 y1 x2 y2\n");
  (void) fprintf(psfile, "%%  draw line from (x1,y1) to (x2,y2)\n");
  for (i=0; i<(count-1); i++)
  {
    (void) fprintf(psfile, "%f %f %f %d %f %f %f %f ln\n",
      (float) fg->red / (float) 65535,
      (float) fg->green / (float) 65535,
      (float) fg->blue / (float) 65535,
      pp_line_width,
      (float) (pp_index_rect[i].x - minx) * fac + xoff,
      (float) (maxy - pp_index_rect[i].y) * fac + yoff,
      (float) (pp_index_rect[i+1].x - minx) * fac + xoff,
      (float) (maxy - pp_index_rect[i+1].y) * fac + yoff );
  }

  /*
   * Draw bitmaps
  */
  (void) fprintf(psfile,
    "%% pg: red green blue type=OPEN_RECTANGLE_GLYPH size=bitmap_size/2 x y\n");

  for (i=0; i<nbitmaps; i++)
  {
    /*
     * First the squares.  Use the top right corner of the bitmap squares.
    */
    (void) fprintf(psfile, "%f %f %f %d %f %f bitmap\n",
      (float) fg->red / (float) 65535,
      (float) fg->green / (float) 65535,
      (float) fg->blue / (float) 65535,
      (int) (bitmap_size*fac) ,
      (float) (bitmap_frame[i] + bitmap_size - minx) * fac + xoff,
      (float) (maxy - (pp_wksp.height - bitmap_size - 15) ) * fac + yoff );
    /*
     * Then the dotted lines.
    */
    (void) fprintf(psfile, "%f %f %f %f %f %f %f lndashed\n",
      (float) fg->red / (float) 65535,
      (float) fg->green / (float) 65535,
      (float) fg->blue / (float) 65535,
      (float) (bitmap_frame[i] + bitmap_size - 3 - minx) * fac + xoff,
      FLOAT(maxy - (FLOAT(pp_wksp.height) - FLOAT(bitmap_space) + 15.)) *
        fac + yoff,
      (float) (bitmap_frame[i] + bitmap_size - 3 - minx) * fac + xoff,
      FLOAT(maxy - (FLOAT(PP_PLOTMARGIN_TOP) * FLOAT(pp_wksp.height))) *
        fac + yoff );
    /*
     * Then the points
    */
    for (m=0; m<npts_in_bitmap[i]; m++)
    {
      k = m;
      if (!xg->erased[k])
      {
        (void) fprintf(psfile, "%f %f %f %d %d %f %f pg\n",
          (float) fg->red / (float) 65535,
          (float) fg->green / (float) 65535,
          (float) fg->blue / (float) 65535,
          OPEN_RECTANGLE_GLYPH ,
          1 ,
          (float) (bitmaps[i][k].x - minx) * fac + xoff,
          (float) (maxy - bitmaps[i][k].y) * fac + yoff );
      }
    }
  }

  /*
   * Draw y axis
  */
  sprintf(str, "%s%s%d%s Index", index_names[xg->pp_index_btn],"(",lJ-1,")");
  (void) fprintf(psfile, "%% yax: (label) red green blue x1 y1 x2 y2\n");
  (void) fprintf(psfile, "%%  draw y axis (and null label)\n");
  (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f ppyax\n", str,
    (float) fg->red / (float) 65535,
    (float) fg->green / (float) 65535,
    (float) fg->blue / (float) 65535,
    (float) (xg->xaxis_indent - minx) * fac + xoff,
    FLOAT(maxy - FLOAT(PP_PLOTMARGIN_TOP)*FLOAT(pp_wksp.height)) *
      fac + yoff,
    (float) (xg->xaxis_indent - minx) * fac + xoff,
    FLOAT(maxy - (FLOAT(pp_wksp.height) - FLOAT(bitmap_space))) * fac + yoff) ;

  /*
   * Add the axis labels
  */
  sprintf(str, "%.2e", pp_index_max);
  (void) fprintf(psfile, "(%s) %f %f %f %f %f ppytx\n",
    str,
    (float) fg->red / (float) 65535,
    (float) fg->green / (float) 65535,
    (float) fg->blue / (float) 65535,
    (float) (xg->xaxis_indent - minx) * fac + xoff,
    FLOAT(maxy - FLOAT(PP_PLOTMARGIN_TOP)*FLOAT(pp_wksp.height)) *
      fac + yoff );

  sprintf(str, "%.2e", pp_index_min);
  (void) fprintf(psfile, "(%s) %f %f %f %f %f ppytx\n",
    str,
    (float) fg->red / (float) 65535,
    (float) fg->green / (float) 65535,
    (float) fg->blue / (float) 65535,
    (float) (xg->xaxis_indent - minx) * fac + xoff,
    FLOAT(maxy - (FLOAT(pp_wksp.height) - FLOAT(bitmap_space))) * fac + yoff );

  if (nbitmaps == 0)
    strcpy(str, "Time");
  else
    strcpy(str, "");
  /*
   * Draw x axis; no room for text if bitmaps present.
  */
  (void) fprintf(psfile, "%% xax: (label) red green blue x1 y1 x2 y2\n");
  (void) fprintf(psfile, "%%  draw x axis (and null label)\n");
  (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f xax\n",
    str,
    (float) fg->red / (float) 65535,
    (float) fg->green / (float) 65535,
    (float) fg->blue / (float) 65535,
    (float) (xg->xaxis_indent - minx) * fac + xoff,
    FLOAT(maxy - (FLOAT(pp_wksp.height) - FLOAT(bitmap_space))) * fac + yoff,
    FLOAT(((1.-PP_PLOTMARGIN_TOP) *
     (FLOAT(pp_wksp.width) - FLOAT(xg->xaxis_indent))) +
     xg->xaxis_indent - minx) * fac + xoff,
    FLOAT(maxy - (FLOAT(pp_wksp.height) - FLOAT(bitmap_space))) * fac + yoff) ;

  /*
   * Draw the new basis markers
  */
  (void) fprintf(psfile, "%% ln: red green blue width x1 y1 x2 y2\n");
  (void) fprintf(psfile, "%%  draw new basis line from (x1,y1) to (x2,y2)\n");
  for (i=0; i<nnew_bases; i++)
  {
    float ftmp = (float)pp_wksp.height - (float)bitmap_space;
    (void) fprintf(psfile, "%f %f %f 1 %f %f %f %f ln\n",
      (float) fg->red / (float) 65535,
      (float) fg->green / (float) 65535,
      (float) fg->blue / (float) 65535,
      (float) (new_basis_lines[i] - minx) * fac + xoff,
      /*(float) (maxy - (pp_wksp.height - bitmap_space)) * fac + yoff,*/
      (float) (maxy - ftmp) * fac + yoff,
      (float) (new_basis_lines[i] - minx) * fac + xoff,
      (float) (maxy - ftmp - (float)LEN_NEW_BASIS_LINE) * fac + yoff );
      /*(float) (maxy - (pp_wksp.height - bitmap_space - LEN_NEW_BASIS_LINE)) * fac + yoff );*/
  }

  /*
   * draw the optimz markers
  */
  (void) fprintf(psfile, "%% pp_arc: red green blue x1 y1 x2 y2 x3 y3\n");
  (void) fprintf(psfile, "%%  draw optimz markers from (x1,y1) to (x2,y2)\n");
  for (i=0; i<noptimz_circs; i++)
  {
    float ftmp = (float)pp_wksp.height - (float)bitmap_space;
    (void) fprintf(psfile, "%f %f %f %d %d %d %f %f pparc\n",
      (float) fg->red / (float) 65535,
      (float) fg->green / (float) 65535,
      (float) fg->blue / (float) 65535,
      OPTIMZ_CIRC_WIDTH/3,
      OPTIMZ_CIRC_ANG1/64,
      OPTIMZ_CIRC_ANG1/64 + OPTIMZ_CIRC_ANG2/64,
      (float) (optimz_circs[i] - minx + OPTIMZ_OFFSET) * fac + xoff,
      /*(float) (maxy - ((float)pp_wksp.height - (float)bitmap_space)) **/
      (maxy - ftmp) * fac + yoff );
  }
}

