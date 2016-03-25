/* parcoords.c */
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
#include <string.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

/* To plot the case profile, linked to identification */
static Widget pcr_plot_shell, pcr_panel, pcr_plot_wksp;
static Drawable pcr_plot_window, pcr_pixmap;
#define LBMARGIN 50  /* left and bottom margin */
#define RTMARGIN 20  /* right and top margins */
static Dimension pcr_height, pcr_width;
static Dimension left_margin = LBMARGIN;

/*
 * I increased the right margin to make room for labels.
 * I don't actually think I'm getting this much right margin; I
 * don't know why.  dfs
*/
static Dimension right_margin = 60; 

static int ncolors_used = 1;
static unsigned long colors_used[NCOLORS+2];

/*
 * By making these first two into arrays, I can plot points
 * and lines for nearest_point plus the sticky guys -- or parallel
 * coordinate plots.
*/
static lcoords *pcr_plane;
static icoords *pcr_screen;

static XPoint *points;
static XSegment *segs;
static XRectangle *open_rectangles;
static XRectangle *filled_rectangles;
static XArc *open_circles;
static XArc *filled_circles;

/*
 * If we're doing parallel coordinates, stickysegs contains segments
 * for all the points.
*/
/* nprofiles either points to xg->nsticky_ids or xg->nrows_in_plot. */
static int *nprofiles;
/* profile_ids either points to xg->sticky_ids or xg->rows_in_plot */
static int *profile_ids;
static XSegment *connsegs, *stickysegs;
static int nconnsegs;

static int *pcr_selectedvars;
static int pcr_nselectedvars;
static lcoords pcr_is;
static int pcr_midx, pcr_midy;
static long pcr_shift_wrld_x, pcr_shift_wrld_y;
static lims pcr_xlim, pcr_ylim, pcr_ynicelim;
static icoords pcr_screen_axes[3];
static int pcr_xdeci, pcr_ydeci;
static float pcr_ytickdelta;
static tickinfo pcr_ticks;
static XSegment pcr_ytick_segs[100];

/*
typedef struct {
  int nticks[NCOLS];   [0] for x; [1] for y;
  float xticks[NTICKS];
  float yticks[NTICKS];
  lcoords plane[NTICKS];
  icoords screen[NTICKS];
} tickinfo;
*/

/* For the case profile */
#define STICKYSEGS(case,varoffset) \
  (stickysegs[case * (pcr_nselectedvars-1) + varoffset])

static Boolean show_all_cases = False;
/*****/
#define NCASESBTNS 2
static Widget cases_menu_cmd, cases_menu, cases_menu_btn[NCASESBTNS];
static Widget cases_menu_box, cases_menu_lab;
#define ALLCASES         0
#define LABELLEDCASES    1
static char *cases_menu_str = "Show cases:";
static char *cases_menu_name[] = {
  "Show all cases",
  "Show only labelled cases",
};
static char *cases_menu_nickname[] = {
  "all",
  "labelled",
};

static Boolean use_common_scale = False;
/*****/
#define NSCALEBTNS 2
static Widget scale_menu_cmd, scale_menu, scale_menu_btn[NSCALEBTNS];
static Widget scale_menu_box, scale_menu_lab;
#define SCALE_COMMON         0
#define SCALE_INDEPENDENT    1
static char *scale_menu_str = "Var scales:";
static char *scale_menu_name[] = {
  "Scale variables (or var groups) on a common scale",
  "Scale variables (or var groups) independently",
};
static char *scale_menu_nickname[] = {
  "common",
  "indp't",
};

static Boolean pcr_doublebuffer = False;
/*****/
#define NBUFFERBTNS 2
static Widget buffer_menu_cmd, buffer_menu, buffer_menu_btn[NBUFFERBTNS];
static Widget buffer_menu_box, buffer_menu_lab;
#define DOUBLEBUFFER   0
#define NOBUFFER       1
static char *buffer_menu_str = "Double buffer:";
static char *buffer_menu_name[] = {
  "Use doublebuffering: eliminates blinking",
  "Don't use doublebuffering: speeds drawing for huge data"
};
static char *buffer_menu_nickname[] = {
  "yes",
  "no",
};


static Boolean pcr_uselbl_short = True;
static Boolean pcr_showlines = True;
static Boolean pcr_showpoints = True;
static Boolean pcr_showcoords = True;
static Boolean pcr_showgrid = False;
/*****/
#define NDISPLAYBTNS 5
static Widget display_menu_cmd, display_menu, display_menu_btn[NDISPLAYBTNS];
#define LBL_SHORT      0  /* Use collab_short instead of collab */
#define SHOWPOINTS     1
#define SHOWLINES      2
/*****/
#define SHOWCOORDS     3
#define SHOWGRID       4

#define NLINKBTNS 1
static Widget link_menu_cmd, link_menu, link_menu_btn[NLINKBTNS];
#define LINKPARCOORDS   0

static Boolean plot_imputed_values = True;
Widget plot_imputed_cmd;

void init_parcoords(xg)
  xgobidata *xg;
{
  xg->is_cprof_plotting = False;
  xg->link_cprof_plotting = True;
  pcr_selectedvars = (int *) XtMalloc((Cardinal)
    xg->ncols * sizeof(int));
}

/*
 * Code for ticks and axes on case profile plotting window.
*/

/*
 * dfs, May 99 -- I'm drawing the parallel coordinate
 * axes now, and only drawing the axes described here in
 * one special case, as described in the comment at the
 * end of add_pcr_axes.
 * As a result, axes[0].x and axes[1].x are no longer used,
 * and I don't really know at this point how the leftmost
 * x axis position is determined.  For the moment, I'll
 * be sloppy and ignore this.  I'm basically just working
 * with a wider left margin, and it looks fine.
*/

static void
make_pcr_screen_axes(void)
/*
 * Since there's no shifting or scaling of this plot, just
 * force the axes to be stable.
*/
{
/*     .(axes[0].x, axes[0].y
 *     |
 *     |
 *     |
 *     |
 *     |
 *     |
 *     .----------------. (axes[2].x, axes[2].y)
 * (axes[1].x, axes[1].y)
*/

  pcr_screen_axes[0].x =
    pcr_screen_axes[1].x = left_margin ;
  /*pcr_screen_axes[2].x = pcr_width - RTMARGIN ;*/
  pcr_screen_axes[2].x = pcr_width - right_margin ;

  pcr_screen_axes[0].y = RTMARGIN ;
  pcr_screen_axes[1].y =
    pcr_screen_axes[2].y = pcr_height - LBMARGIN ;
}

/*
 * Let's force a tick at every variable
 * Rewritten; dfs June 96
*/
static void
generate_pcr_xticks(void)
{
/*
 * nticks: number of ticks used by each column
 * This routine will take the ticks up to their planar
 * values -- that is, the locations of the x ticks in
 * plane coordinates.
*/
  int j;
  float tmpf, precis;

  pcr_ticks.nticks[0] = pcr_nselectedvars;
  pcr_xdeci = 0;

  for (j=0; j<pcr_nselectedvars; j++)
  {
    pcr_ticks.xticks[j] = j;

    if (j == NTICKS-1)
    {
      (void) fprintf(stderr, "warning: (generate_ticks) too many x ticks\n");
      pcr_ticks.nticks[0] = NTICKS;
      return;
    }
  }

/*
 * Drawn from scale_ticks()
*/
  precis = PRECISION1;

  for (j=0; j<pcr_ticks.nticks[0]; j++)
  {
    tmpf = -1.0 + 2.0*(pcr_ticks.xticks[j] - pcr_xlim.min)
        /(pcr_xlim.max - pcr_xlim.min);
    pcr_ticks.plane[j].x = (long) (precis * tmpf);
  }

}

static void
generate_pcr_yticks(void)
{
/*
 * nticks: number of ticks used by each column
 * pcr_ytickdelta: increment between subsequent ticks
 * This routine will take the ticks up to their planar
 * values -- that is, the locations of the y ticks in
 * plane coordinates.
*/
  int j;
  float ftmp, precis, fdiff;
  float nicearg;

/*
 * Drawn from SetNiceRange()
*/
  pcr_ticks.nticks[1] = 5;

  nicearg = (pcr_ylim.max - pcr_ylim.min) /
            (float) (pcr_ticks.nticks[1] - 1) ;
  pcr_ytickdelta = NiceValue( nicearg );
  /* sometimes trouble occurs here, for no good reason */

  pcr_ynicelim.min = (float)
    floor((double) (pcr_ylim.min / pcr_ytickdelta)) *
    pcr_ytickdelta;
  pcr_ynicelim.max = (float)
    ceil((double) (pcr_ylim.max / pcr_ytickdelta)) *
    pcr_ytickdelta;

  /* add .01 for rounding */
  pcr_ticks.nticks[1] = 1 +
    (.01 + (pcr_ynicelim.max - pcr_ynicelim.min) / pcr_ytickdelta);

  pcr_ydeci = set_deci(pcr_ytickdelta);

/*
 * Drawn from generate_ticks()
*/
  pcr_ticks.yticks[0] = pcr_ynicelim.min;
  j = 1;
  while (pcr_ticks.yticks[j-1] + pcr_ytickdelta <=
     pcr_ylim.max + pcr_ytickdelta/2 )
  {
    pcr_ticks.yticks[j] = pcr_ticks.yticks[j-1] + pcr_ytickdelta ;
    if (j++ == NTICKS-1)
    {
      (void) fprintf(stderr, "warning: (generate_ticks) too many y ticks\n");
      return;
    }
  }

  pcr_ticks.nticks[1] = j;

/*
 * Drawn from scale_ticks()
*/
  precis = (float) PRECISION1;
  fdiff = pcr_ylim.max - pcr_ylim.min;

  for (j=0; j<pcr_ticks.nticks[1]; j++)
  {
    ftmp = -1.0 + 2.0 * (pcr_ticks.yticks[j] - pcr_ylim.min) / fdiff;
    pcr_ticks.plane[j].y = - (long) (precis * ftmp);
  }

}

static void
convert_pcr_ticks(void)
{
  int j;
  long nx, ny;

  for (j=0; j<pcr_ticks.nticks[0]; j++)
  {
    nx = (pcr_ticks.plane[j].x + pcr_shift_wrld_x) * pcr_is.x ;
    pcr_ticks.screen[j].x = (int) (nx >> EXP1) ;
    pcr_ticks.screen[j].x += pcr_midx ;
  }

  for (j=0; j<pcr_ticks.nticks[1]; j++)
  {
    ny = (pcr_ticks.plane[j].y - pcr_shift_wrld_y) * pcr_is.y ;

    pcr_ticks.screen[j].y = (int) (ny >> EXP1) ;
    pcr_ticks.screen[j].y += pcr_midy ;
  }

}

static void
build_pcr_tick_segs(void)
{
  int j;

/* build y tick segments */
  for (j=0; j<pcr_ticks.nticks[1]; j++)
  {
    pcr_ytick_segs[j].x1 = pcr_screen_axes[1].x;
    pcr_ytick_segs[j].x2 = pcr_screen_axes[1].x - 5;
    pcr_ytick_segs[j].y1 = pcr_ticks.screen[j].y;
    pcr_ytick_segs[j].y2 = pcr_ticks.screen[j].y;
  }
}

static void
find_pcr_tick_label(int deci, int tickn, float *ticknos, char *str)
{
  int length;
  float ftmp;
  int src, dest;

/*
 * deci:  number of decimal places in each tickdelta; one per column
 * ticknos: xticks or yticks, vector of ticklabel values
 * tickn: indec into ticknos array
*/
  if ((deci > 4 ||
       fabs((double) ticknos[tickn]) >= 10000.) &&
       ticknos[tickn] != 0.0)
  {
    number_length(ticknos[tickn], &length);
    ftmp = (float) ticknos[tickn]/pow((double)10, (double)(length-1));
    if (ftmp == (float)(int)ftmp)
      (void) sprintf (str, "%.0e", ticknos[tickn]);
    else
      (void) sprintf (str, "%.2e", ticknos[tickn]);
  }
  else
  {
    switch(deci)
    {
      case 0:
        (void) sprintf (str, "%3.0f", ticknos[tickn]);
        break;
      case 1:
        (void) sprintf (str, "%3.1f", ticknos[tickn]);
        break;
      case 2:
        (void) sprintf (str, "%3.2f", ticknos[tickn]);
        break;
      case 3:
        (void) sprintf (str, "%3.3f", ticknos[tickn]);
        break;
      case 4:
        (void) sprintf (str, "%3.4f", ticknos[tickn]);
        break;
    }
    /*
     * To get better placement, strip blanks.
    */
    for (src=0, dest=0; src<(INT(strlen(str))+1); src++)
      if (str[src] != ' ')
        str[dest++] = str[src];
  }
}

/* Is screen_axes[1] used any longer? */
static int
find_pcr_ytick_offset(void)
{
  int j, offset = 0;

  for (j=0; j<pcr_ticks.nticks[1]; j++)
  {
    if (pcr_screen_axes[1].y < pcr_ticks.screen[j].y )
      offset++;
    else
      break;
  }
  return(offset);
}

static int
find_pcr_ytick_toomany(void)
{
  int j, toomany = 0;

  for (j=(pcr_ticks.nticks[1]-1); j>=0; j--)
  {
    if (pcr_screen_axes[0].y > pcr_ticks.screen[j].y )
      toomany++;
    else
      break;
  }
  return(toomany);
}

void
add_pcr_axes(xgobidata *xg)
{
  int j, offset, toomany, len;
  unsigned int width;
  char str[64];
  Drawable d = pcr_doublebuffer? pcr_pixmap : pcr_plot_window ;
  int x, y1, y2;

  if (!mono)
    XSetForeground(display, copy_GC, plotcolors.fg);

/*
 * Draw x ticks, and if (pcr_showcoords), also draw the axes.
*/
  y1 = pcr_screen_axes[1].y;  /* the origin */
  y2 = (pcr_showcoords) ? pcr_screen_axes[0].y : pcr_screen_axes[1].y +5;
  for (j=0; j<pcr_ticks.nticks[0]; j++) {
    x = pcr_ticks.screen[j].x;
    XDrawLine(display, d, copy_GC, x, y1, x, y2);
  }

  for (j=0; j<pcr_ticks.nticks[0]; j++) {
    sprintf(str, "%d", pcr_selectedvars[j]+1);

    /*
     * Arbitrary:  if there are more than 20 ticks, then
     * only draw every other tick label.  If there are
     * more than 30; draw every fourth.
    */
    if (pcr_ticks.nticks[0] > 30 && j%4 != 0)
      ;
    else if (pcr_ticks.nticks[0] <= 30 &&
             pcr_ticks.nticks[0] > 20 && j%2 != 0)
      ;
    else {

      sprintf(str, "%s",
        pcr_uselbl_short ? xg->collab_short[ pcr_selectedvars[j] ] :
        xg->collab[ pcr_selectedvars[j] ]) ;
      len = strlen(str);
      width = XTextWidth(appdata.plotFont, str, len);
      while (len >= 2 &&
             width > pcr_ticks.screen[1].x - pcr_ticks.screen[0].x - 4)
      {
        str[len-2] = '\0';
        len--;
        width = XTextWidth(appdata.plotFont, str, len);
      }

      XDrawImageString(display, d, copy_GC,
        (int) (pcr_ticks.screen[j].x - width/2),
        pcr_screen_axes[1].y + FONTHEIGHT(appdata.plotFont)+7,
        str, len);
    }
  }

/*
 * Draw y ticks -- if the yaxis scaling style is appropriate.
*/
  if (use_common_scale) {
    offset = find_pcr_ytick_offset();
    toomany = find_pcr_ytick_toomany();

/*
 * Draw the y ticks at the first variable axis; the original vertical
 * axis is gone.
 *
 * Simply skip the first tick if it's below the X axis
*/
    for (j=offset; j<(pcr_ticks.nticks[1] - toomany); j++)
    {
      int x1 = pcr_ticks.screen[0].x;
      int y1 = pcr_ticks.screen[j].y;
      int x2 = x1 - 5;
      int y2 = y1;

      if (pcr_showgrid) {
        x2 = pcr_ticks.screen[pcr_ticks.nticks[0]-1].x;
        XDrawLine(display, d, copy_GC, x1, y1, x2, y2);
      } else {
        XDrawLine(display, d, copy_GC, x1, y1, x2, y2);
      }

      find_pcr_tick_label(pcr_ydeci, j, pcr_ticks.yticks, str);
      width = XTextWidth(appdata.plotFont, str, strlen(str));
      XDrawImageString(display, d, copy_GC,
        (int) (x1-width-10),
        y1 + 5,
        str, strlen(str));
    }
  }

/*
 * There's still one strange case:  If we're going to scale the
 * variables together, so there are y tick marks and labels,
 * but we're not adding the vertical lines for the parallel
 * coordinates, and we're not adding a grid, either.
 * In that case, we get just tick marks everywhere but
 * no axes, which looks bizarre.  In that combination, draw the
 * old x and y axes.
*/
  x = pcr_ticks.screen[0].x;
  if (use_common_scale && !pcr_showcoords && !pcr_showgrid) {
    XDrawLine(display, d, copy_GC,
      x, pcr_screen_axes[1].y,
      x, pcr_screen_axes[0].y);
    XDrawLine(display, d, copy_GC,
      x, pcr_screen_axes[1].y,
      pcr_screen_axes[2].x, pcr_screen_axes[2].y);
  }

}

/*
 * End of ticks and axes section.
*/

static void
init_pcr_plane_x(void)
{
  int i;
  float min, max, rdiff, dtmp;
  float precis = PRECISION1;

/*
 * Scale variable indices into pcr_plane[].x
*/

  min = 0;
  max = (float) pcr_nselectedvars - 1;
  adjust_limits(&min, &max);
  rdiff = max - min;
  for (i=0; i<pcr_nselectedvars; i++)  /* the minimum number of these */
  {
    dtmp = -1.0 + 2.0*((float)i - min)/rdiff;
    pcr_plane[i].x = (long) (precis * dtmp);
  }

/*
 * For use in constructing ticks and axes.
*/
  pcr_xlim.min = min;
  pcr_xlim.max = max;
}

static void
pcr_tform_to_plane(xgobidata *xg)
{
/*
 * Scale tform2[][] into pcr_plane[].y, the planar values.
*/
  int i, j, k, m;
  float ftmp;
  float precis = PRECISION1;
  float min, max;
  float rdiff;
  static int id = 0;

  /*
   * Since this routine is performed whenever the case profile
   * is about to be changed, this is probably a safe place
   * to perform this intialization.
  */
  if (show_all_cases) {
    nprofiles = (int *) &xg->nrows_in_plot;
    profile_ids = (int *) xg->rows_in_plot;
  } else {
    nprofiles = (int *) &xg->nsticky_ids;
    profile_ids = (int *) xg->sticky_ids;
  }

  if (xg->nearest_point != -1)
    id = xg->nearest_point;

  j = (pcr_nselectedvars>0) ? pcr_selectedvars[0] : 0;
  min = xg->lim_tform[j].min;
  max = xg->lim_tform[j].max;

  for (i=1; i<pcr_nselectedvars; i++) {
    k = pcr_selectedvars[i];
    if (xg->lim_tform[k].min < min)
      min = xg->lim_tform[k].min ;
    if (xg->lim_tform[k].max > max)
      max = xg->lim_tform[k].max ;
  }

  adjust_limits(&min, &max);
  rdiff = max - min;

/*
 * Using xg->lim_tform[] has a problem:  If we have imputed but
 * not rescaled, then xg->lim_tform[] has not been updated.  Hmm.
*/

/*
 * Here tform is taken straight to the plane ...
 *   min for scaling each variable as widely as possible
 *   lim_tform[].min for using a common scale.
*/
  for (m=0; m<pcr_nselectedvars; m++) {
    j = pcr_selectedvars[m];

    if (use_common_scale) {
      ftmp = -1.0 + 2.0*(xg->tform2[id][j] - min)/rdiff;
      pcr_plane[m].y = - (long) (precis * ftmp) ;
      pcr_plane[m].y -= xg->jitter_data[id][j];
    } else {
      pcr_plane[j].y = - xg->world_data[id][j];
    }

  }

  for (i=0; i<*nprofiles; i++) {
    k = (i+1) * pcr_nselectedvars ;
    for (m=0; m<pcr_nselectedvars; m++) {
      j = pcr_selectedvars[m];

      if (use_common_scale) {
        ftmp = -1.0 + 2.0*(xg->tform2[ profile_ids[i] ][j] - min)/rdiff;
        pcr_plane[k+m].y = - (long) (precis * ftmp) ;
        pcr_plane[k+m].y -= xg->jitter_data[ profile_ids[i] ][j];
      } else {
        pcr_plane[k+m].y = - xg->world_data[ profile_ids[i] ][j];
      }

    }
  }

/*
 * For use in constructing ticks and axes.
 * Maybe putting the axis back in for the SCALE_SEPARATELY style
 * involves setting these values based on the the first
 * variable instead of the overall value.
*/
  pcr_ylim.min = min;
  pcr_ylim.max = max;
}

static void
pcr_plane_to_screen(void)
{
/*
 * Take the values in
 *  xg->world_data[nearest_point, sticky][] for the y variable
 *  a scaled up version of 1 to the number of columns for x
 * and scale them into pcr_screen for plotting.
*/
  int j, k;
  long nx, ny;
  
  /* Don't know where this 40 came from. */
  float pcr_scale_x =
    (FLOAT(pcr_width) - FLOAT(left_margin) - FLOAT(right_margin)) /
    FLOAT(pcr_width) ;

  /* Also subtract the right&top margin */
  float pcr_scale_y = (FLOAT(pcr_height) -
    FLOAT(LBMARGIN) - FLOAT(RTMARGIN) - 40.0) / FLOAT(pcr_height) ;

  pcr_shift_wrld_x = (long) (FLOAT(left_margin) * PRECISION1 /
    (FLOAT(pcr_width) * FLOAT(pcr_scale_x))) ;
  pcr_shift_wrld_y = (long) (FLOAT(RTMARGIN) * PRECISION1 /
    (FLOAT(pcr_height) * FLOAT(pcr_scale_y))) ;

  pcr_midx = FLOAT(pcr_width)/2. ;
  pcr_midy = FLOAT(pcr_height)/2. ;

  pcr_is.x = (long) (FLOAT(pcr_width) * FLOAT(pcr_scale_x) / 2.);
  pcr_is.y = (long) (FLOAT(pcr_height) * FLOAT(pcr_scale_y) / 2.);

  for (j=0; j<(*nprofiles+1)*pcr_nselectedvars; j++)
  {
    /* Only have nselectedvars pcr_plane[].x values */
    k = j % pcr_nselectedvars;
    nx = (pcr_plane[k].x + pcr_shift_wrld_x) * pcr_is.x ;
    /* But we're building all the pcr_screen[].x values */
    pcr_screen[j].x = (int) (nx >> EXP1);
    pcr_screen[j].x += pcr_midx ;

    ny = (pcr_plane[j].y - pcr_shift_wrld_y) * pcr_is.y ;
    pcr_screen[j].y = (int) (ny >> EXP1);
    pcr_screen[j].y += pcr_midy ;
  }
}

static void
build_pcr_connect_segs(xgobidata *xg)
{
  int i, var1, var2, id;

  nconnsegs = 0;
  if (xg->nearest_point != -1) {
    id = xg->nearest_point;
    for (i=0; i<pcr_nselectedvars-1; i++) {
      var1 = pcr_selectedvars[i];
      var2 = pcr_selectedvars[i+1];
      if (xg->missing_values_present && !plot_imputed_values &&
          (xg->is_missing[id][var1] || xg->is_missing[id][var2]))
        ;
      else {
        connsegs[nconnsegs].x1 = (short) pcr_screen[i].x ;
        connsegs[nconnsegs].y1 = (short) pcr_screen[i].y ;
        connsegs[nconnsegs].x2 = (short) pcr_screen[i+1].x ;
        connsegs[nconnsegs].y2 = (short) pcr_screen[i+1].y ;
        nconnsegs++;
      }
    }
  }

/*
 * The first pcr_nselectedvars positions of pcr_screen
 * are occupied by nearest_point; the remainder by the values
 * corresponding to the sticky ids.
*/

/*
 * This is pretty screwy:  I'm going to start the segments
 * for each new id at j*(nselectedvars-1):  that's because I need
 * to be able to find these boundaries when I determine what
 * color to use and draw each one separately.  Maybe these
 * should be figured and plotted one at a time ...
*/

  /* build the segments for the sticky guys */
  /* ... moved to cprof_plot_once */
}

static void
find_point_colors_used(xgobidata *xg)
{
  Boolean new_color;
  int i, k, m;

  if (mono) {
    ncolors_used = 1;
    colors_used[0] = plotcolors.fg;
  } else {

    /*
     * Loop once through xg->color_now[], collecting the colors currently
     * in use by sticky_ids into the colors_used[] vector.
    */
    ncolors_used = 0;
    for (i=0; i<*nprofiles; i++) {
      m = profile_ids[i];
      new_color = True;
      for (k=0; k<ncolors_used; k++) {
        if (colors_used[k] == xg->color_now[m]) {
          new_color = False;
          break;
        }
      }
      if (new_color) {
        colors_used[ncolors_used] = xg->color_now[m];
        (ncolors_used)++;
      }
    }

    if (ncolors_used == 0) {
      ncolors_used = 1;
      colors_used[0] = plotcolors.fg;
    }

    /*
     * Make sure that the current brushing color is
     * last in the list, so that it is drawn on top of
     * the pile of points.
    */
    for (k=0; k<(ncolors_used-1); k++) {
      if (colors_used[k] == xg->color_id) {
        colors_used[k] = colors_used[ncolors_used-1];
        colors_used[ncolors_used-1] = xg->color_id;
        break;
      }
    }
  }
}

void
cprof_plot_once(xgobidata *xg)
{
  int i, j, sticky_id;
  unsigned int width;
  static int id = 0;
  int np, ns, nr_open, nr_filled, nc_open, nc_filled;
  XGCValues *gcv, gcv_inst;
  Drawable d = pcr_doublebuffer? pcr_pixmap : pcr_plot_window ;

/*
 * In order to make the plot update itself during brushing,
 * there are two lines at the end of brush_once().  This
 * is not pretty, since it is another violation of modularity.
 * Can this be handled in the event loop?
*/

  if (!mono && *nprofiles > 1)
    find_point_colors_used(xg);

  XGetGCValues(display, copy_GC, GCCapStyle|GCJoinStyle, &gcv_inst);
  gcv = &gcv_inst;

  XFillRectangle(display, d, clear_GC, 0, 0, pcr_width, pcr_height );

/*
 * Problem:  Even if there's no point to plot, I want to see the
 * vertical axes.  This should take care of it.
*/
  if (xg->nearest_point == -1 && *nprofiles == 0)
    convert_pcr_ticks();

  build_pcr_tick_segs();
  add_pcr_axes(xg);

  if (xg->nearest_point != -1 || *nprofiles != 0) {
    id = xg->nearest_point;
    /* This needs to be done once for for the sticky and non-sticky guys */
    build_pcr_connect_segs(xg);

    /* If there's a nearest_point: ie, if the cursor is in the window .. */
    if (id != -1) {
      if (pcr_showpoints) {
        np = ns = nr_open = nr_filled = nc_open = nc_filled = 0;

        /* Build the glyphs for the nearest point */
        for (i=0; i<pcr_nselectedvars; i++)
          if (xg->missing_values_present && !plot_imputed_values &&
              xg->is_missing[id][ pcr_selectedvars[i] ])
            ; /* do nothing */
          else 
            build_glyph(xg, id, pcr_screen, i,
              points, &np,
              segs, &ns,
              open_rectangles, &nr_open,
              filled_rectangles, &nr_filled,
              open_circles, &nc_open,
              filled_circles, &nc_filled);

        /*
         * Draw the glyphs for nearest point.
        */
        if (!mono)
          XSetForeground(display, copy_GC, xg->color_now[id]);

        if (np)
          XDrawPoints(display, d, copy_GC, points, np, CoordModeOrigin);
        else if (ns)
          XDrawSegments(display, d, copy_GC, segs, ns);
        else if (nr_open)
          XDrawRectangles(display, d, copy_GC, open_rectangles, nr_open);
        else if (nr_filled) {
          XDrawRectangles(display, d, copy_GC, filled_rectangles, nr_filled);
          XFillRectangles(display, d, copy_GC, filled_rectangles, nr_filled);
        }
        else if (nc_open)
          XDrawArcs(display, d, copy_GC, open_circles, nc_open);
        else if (nc_filled) {
          XDrawArcs(display, d, copy_GC, filled_circles, nc_filled);
          XFillArcs(display, d, copy_GC, filled_circles, nc_filled);
        }
      }
      if (pcr_showlines) {
        /*
         * Draw the connected lines for nearest_point; use plotcolors.fg.
        */

        /* Thin lines for sticky guys; thick for current guy */
        width = 5;

        if (!mono)
          XSetForeground(display, copy_GC, xg->color_now[id]);

        XSetLineAttributes(display, copy_GC, width, LineDoubleDash,
          gcv->cap_style, gcv->join_style);

        XDrawSegments(display, d, copy_GC, connsegs, nconnsegs);

        XSetLineAttributes(display, copy_GC, 0, LineSolid,
          gcv->cap_style, gcv->join_style);
      }
    }

/*** Sticky guys, or parallel coordinates plot ***/
/*
 * First draw the points:  loop over colors, gathering vectors
 * of glyphs, and do all the drawing for each glyph type for
 * each color.
*/

    if (pcr_showpoints) {
      int k;
      unsigned long current_color;

      for (k=0; k<ncolors_used; k++) {
        np = ns = nr_open = nr_filled = nc_open = nc_filled = 0;
        current_color = colors_used[k];

        if (!mono)
          XSetForeground(display, copy_GC, current_color);
      
        for (j=0; j<*nprofiles; j++) {
          sticky_id = *(profile_ids+j);
          if (xg->color_now[sticky_id] == current_color) {

            if (!xg->erased[sticky_id]) {
              for (i=0; i<pcr_nselectedvars; i++) {
                if (!plot_imputed_values && xg->missing_values_present &&
                     xg->is_missing[sticky_id][ pcr_selectedvars[i] ])
                  ;
                else
                  build_glyph(xg,
                    sticky_id, &pcr_screen[pcr_nselectedvars*(j+1)], i,
                    points, &np,
                    segs, &ns,
                    open_rectangles, &nr_open,
                    filled_rectangles, &nr_filled,
                    open_circles, &nc_open,
                    filled_circles, &nc_filled);
              }
            }
          }
        }

        if (np)
          XDrawPoints(display, d, copy_GC, points, np, CoordModeOrigin);
        if (ns)
          XDrawSegments(display, d, copy_GC, segs, ns);
        if (nr_open)
          XDrawRectangles(display, d, copy_GC, open_rectangles, nr_open);
        if (nr_filled) {
          XDrawRectangles(display, d, copy_GC, filled_rectangles, nr_filled);
          XFillRectangles(display, d, copy_GC, filled_rectangles, nr_filled);
        }
        if (nc_open) {
          /* work-around for X bug; see plot_once.c */
          int c = 0;
          while (nc_open > MAXARCS) {
            XDrawArcs(display, d, copy_GC, &open_circles[c*MAXARCS], MAXARCS);
            nc_open -= MAXARCS;
            c++;
          }
          XDrawArcs(display, d, copy_GC, &open_circles[c*MAXARCS], nc_open);
        }
        if (nc_filled) {
          /* work-around for X bug; see plot_once.c */
          int c = 0;
          while (nc_filled > MAXARCS) {
            XDrawArcs(display, d, copy_GC, &filled_circles[c*MAXARCS], MAXARCS);
            XFillArcs(display, d, copy_GC, &filled_circles[c*MAXARCS], MAXARCS);
            nc_filled -= MAXARCS;
            c++;
          }
          XDrawArcs(display, d, copy_GC, &filled_circles[c*MAXARCS], nc_filled);
          XFillArcs(display, d, copy_GC, &filled_circles[c*MAXARCS], nc_filled);
        }
      }
    }

/*
 * Then draw the line segments.  Loop over colors again, gathering
 * all the segments to be drawn in a particular color.
*/
    if (pcr_showlines) {
      int k, nv, var1, var2, jsegs;
      unsigned long current_color;

      /*
       * Draw the connected lines for the sticky ids; use thin lines.
      */
      width = 1;
      XSetLineAttributes(display, copy_GC, width, LineSolid,
        gcv->cap_style, gcv->join_style);

      for (k=0; k<ncolors_used; k++) {
        current_color = colors_used[k];
        if (!mono)
          XSetForeground(display, copy_GC, current_color);

        nv = pcr_nselectedvars;
        jsegs = 0;
        for (j=0; j<*nprofiles; j++) {
          id = *(profile_ids+j);
          if (xg->color_now[id] != current_color || xg->erased[id]) {
            nv += (pcr_nselectedvars-1);
          } else {
            for (i=0; i<pcr_nselectedvars-1; i++) {
              var1 = pcr_selectedvars[i];
              var2 = pcr_selectedvars[i+1];
              if (!plot_imputed_values && xg->missing_values_present &&
                  (xg->is_missing[id][var1] || xg->is_missing[id][var2]))
                ;
              else {
                stickysegs[ jsegs ].x1 = (short) pcr_screen[i].x ;
                stickysegs[ jsegs ].y1 = (short) pcr_screen[nv].y ;
                stickysegs[ jsegs ].x2 = (short) pcr_screen[i+1].x ;
                stickysegs[ jsegs ].y2 = (short) pcr_screen[nv+1].y ;
                jsegs++;
              }
              nv++;
            }
          }
          nv++;  /* ok */
        }

        XDrawSegments(display, d, copy_GC, stickysegs, jsegs);
      }

      /* back to the usual line */
      XSetLineAttributes(display, copy_GC, 0, LineSolid,
        gcv->cap_style, gcv->join_style);
    }
  }

  if (pcr_doublebuffer)
    XCopyArea(display, pcr_pixmap, pcr_plot_window, copy_GC,
      0, 0, pcr_width, pcr_height, 0, 0 );
}

void
update_cprof_selectedvars(xgobidata *xg)
{
  int j, k = 0;

  for (j=0; j<xg->ncols_used; j++)
    if (xg->selectedvars[j])
      pcr_selectedvars[k++] = j ;

  pcr_nselectedvars = k;
}

void
update_cprof_plot(xgobidata *xg)
{
/*
 * This is called when a variable transformation occurs,
 * when nselectedvars changes, when the passive window
 * receives new data, and so forth.
*/
  int j;
  char str[128];
  Dimension width;

  pcr_tform_to_plane(xg);
  pcr_plane_to_screen();
  /*
   * Calculate the ticks and axes for the
   * y axis; scale ticks and axes.
  */
  generate_pcr_yticks();
  convert_pcr_ticks();

  /*
   * Check the length of the y axis labels.  If the length
   * of any axis plus 5 for the tick length plus 2 for breathing
   * room is greater than the margin, then reinitialize the axes.
  */
  width = 0;
  for (j=0; j<pcr_ticks.nticks[1]; j++)
  {
    find_pcr_tick_label(pcr_ydeci, j, pcr_ticks.yticks, str);
    width = MAX(width,
      ((Dimension)XTextWidth(appdata.plotFont, str, strlen(str))));
  }
  width = width+5+2 ;
  if (width > left_margin)
  {
    left_margin = width;
    make_pcr_screen_axes();
    pcr_plane_to_screen();
    convert_pcr_ticks();
  }

  cprof_plot_once(xg);
}

void
realloc_tform(xgobidata *xg)
{
  int npts;

  if (show_all_cases)
    npts = xg->nrows_in_plot+1;
  else
    npts = xg->nsticky_ids+1;

  pcr_plane = (lcoords *) XtRealloc( (XtPointer) pcr_plane,
    (Cardinal) (npts * xg->ncols_used * sizeof(lcoords)));
  pcr_screen = (icoords *) XtRealloc( (XtPointer) pcr_screen,
    (Cardinal) (npts * xg->ncols_used * sizeof(icoords)));

  points = (XPoint *) XtRealloc( (XtPointer) points,
    (Cardinal) (npts * xg->ncols_used) * sizeof(XPoint));
  segs = (XSegment *) XtRealloc( (XtPointer) segs,
    (Cardinal) (2 * npts * xg->ncols_used) * sizeof(XSegment));
  open_rectangles = (XRectangle *) XtRealloc( (XtPointer) open_rectangles,
    (Cardinal) (npts * xg->ncols_used) * sizeof(XRectangle));
  filled_rectangles = (XRectangle *) XtRealloc( (XtPointer) filled_rectangles,
    (Cardinal) (npts * xg->ncols_used) * sizeof(XRectangle));
  open_circles = (XArc *) XtRealloc( (XtPointer) open_circles,
    (Cardinal) (npts * xg->ncols_used) * sizeof(XArc));
  filled_circles = (XArc *) XtRealloc( (XtPointer) filled_circles,
    (Cardinal) (npts * xg->ncols_used) * sizeof(XArc));

  connsegs = (XSegment *) XtRealloc( (XtPointer) connsegs,
    (Cardinal) (xg->ncols_used-1) * sizeof(XSegment));
  stickysegs = (XSegment *) XtRealloc( (XtPointer) stickysegs,
    (Cardinal) (npts * (xg->ncols_used-1)) * sizeof(XSegment));
}

/* ARGSUSED */
XtCallbackProc
pcr_resize_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtVaGetValues(pcr_plot_wksp,
    XtNwidth, &pcr_width,
    XtNheight, &pcr_height,
    NULL);

/*  Allocate, and free it even if it isn't going to be used */
  XFreePixmap(display, pcr_pixmap);
  pcr_pixmap = XCreatePixmap(display, pcr_plot_window,
    pcr_width, pcr_height, depth);

  if (pcr_doublebuffer)
    XFillRectangle(display, pcr_pixmap, clear_GC,
      0, 0, pcr_width, pcr_height );
  else
    XFillRectangle(display, pcr_plot_window, clear_GC,
      0, 0, pcr_width, pcr_height );

  pcr_tform_to_plane(xg);
  pcr_plane_to_screen();

  make_pcr_screen_axes();
  generate_pcr_yticks();
  convert_pcr_ticks();
}

/* ARGSUSED */
XtEventHandler
pcr_expose_cback(Widget w, xgobidata *xg, XEvent *evnt, Boolean *cont)
{
  if (evnt->xexpose.count == 0)  /* Compress expose events */
  {
    pcr_tform_to_plane(xg);
    pcr_plane_to_screen();
    cprof_plot_once(xg);
  }
}

/* ARGSUSED */
static XtCallbackProc
plot_imputed_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  plot_imputed_values = !plot_imputed_values;
  update_cprof_plot(xg);
}

/* ARGSUSED */
static XtCallbackProc
pcr_close_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtPopdown(pcr_plot_shell);

  xg->is_cprof_plotting = False;

  XtDestroyWidget( pcr_plot_shell );
  XFreePixmap( display, pcr_pixmap );
  XtFree( (XtPointer) pcr_plane );
  XtFree( (XtPointer) pcr_screen );
  XtFree( (XtPointer) segs );
  XtFree( (XtPointer) connsegs );
  XtFree( (XtPointer) stickysegs );
}

/**** Link Menu ****/

static void
set_link_menu_marks(xgobidata *xg)
{
  if (xg->link_cprof_plotting)
    XtVaSetValues(link_menu_btn[0],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
  else
    XtVaSetValues(link_menu_btn[0],
      XtNleftBitmap, (Pixmap) None,
      NULL);
}

/* ARGSUSED */
static XtCallbackProc
link_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<1; btn++)
    if (link_menu_btn[btn] == w)
      break;

  switch (btn) {
    case 0 :
      xg->link_cprof_plotting = !xg->link_cprof_plotting;
      break;
  }
  set_link_menu_marks(xg);
}

static void
make_link_menu(xgobidata *xg, Widget parent)
{
  int k;

  static char *link_menu_name[] = {
    "Link par coords plot",
  };

  link_menu_cmd = XtVaCreateManagedWidget("LinkParCoord",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Link",
    XtNmenuName, (String) "Menu",
    XtNfromVert, buffer_menu_box,
    XtNvertDistance, 3,
    XtNfromHoriz, display_menu_cmd,
    XtNhorizDistance, 3,
    NULL);
  if (mono) set_mono(link_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    link_menu_cmd, "IdentifyLink");

  link_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, link_menu_cmd,
    XtNinput, True,
    NULL);
  if (mono) set_mono(link_menu);

  for (k=0; k<1; k++)
  {
    link_menu_btn[k] = XtVaCreateWidget("Command",
      smeBSBObjectClass, link_menu,
      XtNleftMargin, (Dimension) 24,
      XtNleftBitmap, menu_mark,
      XtNlabel, (String) link_menu_name[k],
      NULL);
    if (mono) set_mono(link_menu_btn[k]);

    XtAddCallback(link_menu_btn[k], XtNcallback,
      (XtCallbackProc) link_menu_cback, (XtPointer) xg);
  }

  XtManageChildren(link_menu_btn, 1);
}

/**** End Link Menu ****/

/**** Cases Menu ****/

/* ARGSUSED */
static XtCallbackProc
cases_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<NCASESBTNS; btn++)
    if (cases_menu_btn[btn] == w)
      break;

  show_all_cases = (btn == ALLCASES) ? True : False;

  XtVaSetValues(cases_menu_cmd,
    XtNlabel, cases_menu_nickname[btn],
    NULL);

  realloc_tform(xg);
  update_cprof_plot(xg);
}

/**** End Cases Menu ****/

/**** Scale Menu ***/

/* ARGSUSED */
static XtCallbackProc
scale_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<NSCALEBTNS; btn++)
    if (scale_menu_btn[btn] == w)
      break;

  use_common_scale = (btn == SCALE_COMMON) ? True : False;
  XtSetSensitive(display_menu_btn[SHOWGRID], use_common_scale);

  XtVaSetValues(scale_menu_cmd,
    XtNlabel, scale_menu_nickname[btn],
    NULL);

  update_cprof_plot(xg);
}

/**** End Scale Menu ***/

/**** Display Menu ***/

static void
pcr_set_display_menu_marks(void)
{
  int k;
  for (k=0; k<NDISPLAYBTNS; k++)
    XtVaSetValues(display_menu_btn[k],
      XtNleftBitmap, (Pixmap) None,
      NULL);

  if (pcr_uselbl_short)
    XtVaSetValues(display_menu_btn[LBL_SHORT],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
  if (pcr_showpoints)
    XtVaSetValues(display_menu_btn[SHOWPOINTS],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
  if (pcr_showlines)
    XtVaSetValues(display_menu_btn[SHOWLINES],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
  if (pcr_showcoords)
    XtVaSetValues(display_menu_btn[SHOWCOORDS],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
  if (pcr_showgrid)
    XtVaSetValues(display_menu_btn[SHOWGRID],
      XtNleftBitmap, (Pixmap) menu_mark,
      NULL);
}

/* ARGSUSED */
static XtCallbackProc
display_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<NDISPLAYBTNS; btn++)
    if (display_menu_btn[btn] == w)
      break;

  switch (btn) {
    case LBL_SHORT :
      pcr_uselbl_short = !pcr_uselbl_short;
      break;
    case SHOWPOINTS :
      pcr_showpoints = !pcr_showpoints;
      break;
    case SHOWLINES :
      pcr_showlines = !pcr_showlines;
      break;
    case SHOWCOORDS :
      pcr_showcoords = !pcr_showcoords;
      break;
    case SHOWGRID :
      pcr_showgrid = !pcr_showgrid;
      break;
  }
  cprof_plot_once(xg);
  pcr_set_display_menu_marks();
}

static void
make_display_menu(xgobidata *xg, Widget parent)
{
  int k;

  static char *display_menu_name[] = {
    "Use short variable label (if provided)",
    "Plot the points",
    "Plot the lines",
    "Display the vertical parallel coordinates",
    "Display the horizontal gridlines",
  };

  display_menu_cmd = XtVaCreateManagedWidget("Command",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Display",
    XtNmenuName, (String) "Menu",
    XtNfromVert, buffer_menu_box,
    XtNvertDistance, 3,
    NULL);
  if (mono) set_mono(display_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    display_menu_cmd, "DisplayOptions");

  display_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, display_menu_cmd,
    XtNinput, True,
    NULL);
  if (mono) set_mono(display_menu);

  for (k=0; k<NDISPLAYBTNS; k++) {
    display_menu_btn[k] = XtVaCreateWidget("Command",
      smeBSBObjectClass, display_menu,
      XtNleftMargin, (Dimension) 24,
      XtNleftBitmap, menu_mark,
      XtNlabel, (String) display_menu_name[k],
      NULL);
    if (mono) set_mono(display_menu_btn[k]);
    XtAddCallback(display_menu_btn[k], XtNcallback,
      (XtCallbackProc) display_menu_cback, (XtPointer) xg);

    if (k==0 || k==2) {
      Widget line = XtVaCreateManagedWidget("Line",
        smeLineObjectClass, display_menu,
        NULL);
      if (mono) set_mono(line);
    }
  }

  XtSetSensitive(display_menu_btn[SHOWGRID], use_common_scale);
  XtManageChildren(display_menu_btn, NDISPLAYBTNS);
}

/**** End Display Menu ***/

/* ARGSUSED */
static XtCallbackProc
buffer_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int btn;

  for (btn=0; btn<NBUFFERBTNS; btn++)
    if (buffer_menu_btn[btn] == w)
      break;

  pcr_doublebuffer = (btn == DOUBLEBUFFER) ? True : False;

  XtVaSetValues(buffer_menu_cmd,
    XtNlabel, buffer_menu_nickname[btn],
    NULL);
}

/* ARGSUSED */
XtCallbackProc
cprof_plot_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  static Boolean initd = False;
  String main_title;
  static char cp_title[256];
  Widget dform;
  Widget paned0;

  int k;

  if (!xg->is_cprof_plotting)
  {
    int npts;
    Widget cpaned, pcr_close, panelform;
    Widget plot_form;

    /*
     * Weird behavior here:  if I free this and reallocate it,
     * the title does not reappear.
    */
    if (!initd) {
      main_title = XtMalloc((Cardinal) 132 * sizeof(char));
      XtVaGetValues(xg->shell, XtNtitle, &main_title, NULL);
      sprintf(cp_title, "%s, parallel coordinates", main_title);
      XtFree(main_title);
      initd = True;
    }

    if (show_all_cases)
      npts = xg->nrows_in_plot + 1;
    else
      npts = xg->nsticky_ids + 1;

    /*
     * Allocate enough space to plot the points and lines
     * for all the sticky ids for the current id.
    */
    pcr_plane = (lcoords *) XtMalloc( (Cardinal)
      npts * xg->ncols_used * sizeof(lcoords));
    pcr_screen = (icoords *) XtMalloc( (Cardinal)
      npts * xg->ncols_used * sizeof(icoords));

    points = (XPoint *) XtMalloc( (Cardinal)
      npts * xg->ncols_used * sizeof(XPoint));
    segs = (XSegment *) XtMalloc( (Cardinal)
      2 * npts * xg->ncols_used * sizeof(XSegment));
    open_rectangles = (XRectangle *) XtMalloc( (Cardinal)
      npts * xg->ncols_used * sizeof(XRectangle));
    filled_rectangles = (XRectangle *) XtMalloc( (Cardinal)
      npts * xg->ncols_used * sizeof(XRectangle));
    open_circles = (XArc *) XtMalloc( (Cardinal)
      npts * xg->ncols_used * sizeof(XArc));
    filled_circles = (XArc *) XtMalloc( (Cardinal)
      npts * xg->ncols_used * sizeof(XArc));

    connsegs = (XSegment *) XtMalloc( (Cardinal)
      (xg->ncols_used-1) * sizeof(XSegment));
    stickysegs = (XSegment *) XtMalloc( (Cardinal)
      npts * (xg->ncols_used-1) * sizeof(XSegment));

    pcr_width = 400;
    pcr_height = 200;

    pcr_plot_shell = XtVaCreatePopupShell("CaseProfile",
      topLevelShellWidgetClass, xg->shell,
      XtNtitle, (String) cp_title,
      XtNiconName, "XGobi: ParCoords",
      NULL);
    if (mono) set_mono(pcr_plot_shell);

    /* Horizontal paned widget with a grip; two children */
    paned0 = XtVaCreateManagedWidget("Panel",
      panedWidgetClass, pcr_plot_shell,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtRubber,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtRubber,
      XtNorientation, (XtOrientation) XtorientHorizontal,
      NULL);

    panelform = XtVaCreateManagedWidget("Form1",
      formWidgetClass, paned0,
      XtNborderWidth, 0,
      XtNmin, 5,
      NULL);
    if (mono) set_mono(panelform);

    /*
     * Create a paned widget so the 'Click here ...'
     * can be all across the bottom.
    */
    cpaned = XtVaCreateManagedWidget("Panel",
      panedWidgetClass, panelform,
      XtNorientation, (XtOrientation) XtorientVertical,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNinternalBorderWidth, 1,
      XtNhorizDistance, 5,
      NULL);

    pcr_panel = XtVaCreateManagedWidget("Panel",
      formWidgetClass, cpaned,
      NULL);
    if (mono) set_mono(pcr_panel);

    build_labelled_menu(&cases_menu_box, &cases_menu_lab, cases_menu_str,
      &cases_menu_cmd, &cases_menu, cases_menu_btn,
      cases_menu_name, cases_menu_nickname,
      NCASESBTNS,
      (show_all_cases) ? ALLCASES : LABELLEDCASES,
      pcr_panel, NULL,
      XtorientHorizontal, appdata.font, "ParCoords", xg);
    for (k=0; k<NCASESBTNS; k++)
      XtAddCallback(cases_menu_btn[k],  XtNcallback,
        (XtCallbackProc) cases_menu_cback, (XtPointer) xg);

    build_labelled_menu(&scale_menu_box, &scale_menu_lab, scale_menu_str,
      &scale_menu_cmd, &scale_menu, scale_menu_btn,
      scale_menu_name, scale_menu_nickname,
      NSCALEBTNS,
      (use_common_scale) ? SCALE_COMMON : SCALE_INDEPENDENT,
      pcr_panel, cases_menu_box,
      XtorientHorizontal, appdata.font, "ParCoords", xg);
    for (k=0; k<NSCALEBTNS; k++)
      XtAddCallback(scale_menu_btn[k],  XtNcallback,
        (XtCallbackProc) scale_menu_cback, (XtPointer) xg);

    if (xg->nrows_in_plot < 1000) pcr_doublebuffer = True;
    build_labelled_menu(&buffer_menu_box, &buffer_menu_lab, buffer_menu_str,
      &buffer_menu_cmd, &buffer_menu, buffer_menu_btn,
      buffer_menu_name, buffer_menu_nickname,
      NBUFFERBTNS,
      (pcr_doublebuffer) ? DOUBLEBUFFER : NOBUFFER,
      pcr_panel, scale_menu_box,
      XtorientHorizontal, appdata.font, "ParCoords", xg);
    for (k=0; k<NBUFFERBTNS; k++)
      XtAddCallback(buffer_menu_btn[k],  XtNcallback,
        (XtCallbackProc) buffer_menu_cback, (XtPointer) xg);

    make_display_menu(xg, pcr_panel);
    pcr_set_display_menu_marks();

    make_link_menu(xg, pcr_panel);

    /*
     * Let this button insensitive and unhighlighted for the
     * missing values xgobi
    */
    if (xg->is_missing_values_xgobi) {
      plot_imputed_cmd = (Widget) CreateToggle(xg, "Display missings",
        False, (Widget) NULL, (Widget) display_menu_cmd, (Widget) NULL,
        False, ANY_OF_MANY,
        pcr_panel, "ParCoords");
    } else {
      if (!xg->missing_values_present) plot_imputed_values = False;
      plot_imputed_cmd = (Widget) CreateToggle(xg, "Display missings",
        /* the button is insensitive if there are no missings */
        xg->missing_values_present,
        (Widget) NULL, (Widget) display_menu_cmd, (Widget) NULL,
        plot_imputed_values, ANY_OF_MANY,
        pcr_panel, "ParCoords");
      XtAddCallback(plot_imputed_cmd, XtNcallback,
        (XtCallbackProc) plot_imputed_cback, (XtPointer) xg);
    }
    XtManageChild(plot_imputed_cmd);

    /* Add a form so that the dismiss button can be farther
       away from the 'display missings' button
    */
    dform = XtVaCreateManagedWidget("Form",
      formWidgetClass, cpaned,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      NULL);
    if (mono) set_mono(dform);

    pcr_close = XtVaCreateManagedWidget("Close",
      commandWidgetClass, dform,
      XtNlabel, (String) "Click here to dismiss",
      XtNvertDistance, 10,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      NULL);
    if (mono) set_mono(pcr_close);
    XtAddCallback(pcr_close, XtNcallback,
      (XtCallbackProc) pcr_close_cback, (XtPointer) xg);

    plot_form = XtVaCreateManagedWidget("Form1",
      formWidgetClass, paned0,
      NULL);
    if (mono) set_mono(plot_form);

    pcr_plot_wksp = XtVaCreateManagedWidget("CaseProfile",
      labelWidgetClass, plot_form,
      XtNlabel, (String) "",
      XtNforeground, (Pixel) plotcolors.fg,
      XtNbackground, (Pixel) plotcolors.bg,
      XtNwidth, (Dimension) pcr_width,
      XtNheight, (Dimension) pcr_height,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainBottom,
      NULL);
    if (mono) set_mono(pcr_plot_wksp);

    XtAddEventHandler(pcr_plot_wksp, ExposureMask,
      FALSE, (XtEventHandler) pcr_expose_cback, (XtPointer) xg);
    XtAddEventHandler(pcr_plot_wksp, StructureNotifyMask,
      FALSE, (XtEventHandler) pcr_resize_cback, (XtPointer) xg);

    XtPopup(pcr_plot_shell, XtGrabNone);
    XRaiseWindow(display, XtWindow(pcr_plot_shell));

    pcr_plot_window = XtWindow( pcr_plot_wksp );
    pcr_pixmap = XCreatePixmap(display, pcr_plot_window,
      pcr_width, pcr_height, depth);

    update_cprof_selectedvars(xg);

    /*
     * The pcr_plane[].x values don't change, so just
     * set them up here.  The same goes for the x ticks,
     * and the axes only need to be initialized once.
    */
    init_pcr_plane_x();

    make_pcr_screen_axes();
    generate_pcr_xticks();

    xg->is_cprof_plotting = True;
  }
  else
  {
    XtPopup(pcr_plot_shell, XtGrabNone);
    XRaiseWindow(display, XtWindow(pcr_plot_shell));
  }
}

void
passive_update_cprof_plot(xgobidata *xg)
{
/*
 * This is called when the passive window receives new data.
*/
  if (xg->is_cprof_plotting && xg->link_cprof_plotting)
  {
    realloc_tform(xg);
    update_cprof_plot(xg);
  }
}

void
reset_nvars_cprof_plot(xgobidata *xg)
{
/*
 * If case profile plotting when an additional variable is
 * added or changed, especially the group variable, then do this:
*/
  if (xg->is_cprof_plotting)
  {
    /*
     * This isn't always necessary, but I don't know
     * how to tell when it is, so I'll do it in any case.
    */
    realloc_tform(xg);

    init_pcr_plane_x();

    generate_pcr_xticks();
    update_cprof_plot(xg);
  }
}

/*
 * Used by print_plotwin
*/
void
get_cprof_win_dims(float *maxx, float *maxy, xgobidata *xg)
{
  if (xg->is_cprof_plotting) {
    *maxx = (float) pcr_width ;
    *maxy = (float) pcr_height ;
  }
  else
    *maxx = *maxy = 0 ;
}

void
check_cprof_fac_and_offsets(float *minx, float *maxx, float *maxy,
float *fac, float *xoff, float *yoff, int pointsize)
{
  /*
   * Shift x axis.  The constant 4.5 comes from the header file.
  */
  while (72.*((*maxy - pcr_screen_axes[1].y) * *fac + *yoff) -
             4.0*pointsize < 72. * *yoff)
  {
    *maxy = 1.02 * *maxy ;
    set_fac_and_offsets(*minx, *maxx, *maxy, fac, xoff, yoff);
  }

  /*
   * Shift x axis.  The constant 4.5 comes from the header file.
  */
  while (72.*((pcr_screen_axes[1].x - *minx) * *fac + *xoff) -
         4.0*pointsize < 72. * *xoff)
  {
    *minx = *minx - .02 * (*maxx - *minx) ;
    /**maxx = 1.02 * *maxx ;*/
    set_fac_and_offsets(*minx, *maxx, *maxy, fac, xoff, yoff);
  }
}

void
print_cprof_win(xgobidata *xg, FILE *psfile, float minx, float maxy,
  float fac, float xoff, float yoff, XColor *fg, XColor *rgb_table,
  unsigned int ncolors)
{
  int i, j, k, indx, id;
  int offset, toomany;
  char str[64];
  int pgsize;
  XColor *rgb;

  rgb = (XColor *) XtMalloc((Cardinal) *nprofiles * sizeof(XColor));
  for (j=0; j<*nprofiles; j++) {
    id = *(profile_ids+j);  
    for (k=0; k<ncolors; k++) {
      if (xg->color_now[id] == color_nums[k]) {
        rgb[j].red = rgb_table[k].red;
        rgb[j].green = rgb_table[k].green;
        rgb[j].blue = rgb_table[k].blue;
        break;
      }
    }
  }

/*
 * id is the row number of the point in question;
 * indx indexes the points of the profile curve.
*/
  /* first the points */
  if (pcr_showpoints) {
    for (j=0; j<*nprofiles; j++) {
      id = *(profile_ids+j);

      pgsize = find_pgsize(xg->glyph_now[id].type, xg->glyph_now[id].size);
      for (i=0; i<pcr_nselectedvars; i++) {
        /*
         * pcr_screen[] for the sticky guys starts at
         * pcr_nselectedvars; this accounts for the (j+1) here
        */
        indx = (j+1)*pcr_nselectedvars + i ;

        if (!xg->erased[id]) {
          if (!plot_imputed_values && xg->missing_values_present &&
               xg->is_missing[id][ pcr_selectedvars[i] ])
            ;
          else {
            (void) fprintf(psfile, "%f %f %f %d %d %f %f pg\n",
              (float) rgb[j].red / (float) 65535,
              (float) rgb[j].green / (float) 65535,
              (float) rgb[j].blue / (float) 65535,
              xg->glyph_now[id].type,
              pgsize,
              (float) (pcr_screen[indx].x - minx) * fac + xoff,
              (float) (maxy - pcr_screen[indx].y) * fac + yoff );
           }
         }
       }
    }
  }

  /*
   * Then the lines.  I have to use the same colors_used
   * vector that's been used to draw to the window.
  */
  if (pcr_showlines) {
    int nv, i, var1, var2;
    unsigned long current_color;
    for (k=0; k<ncolors_used; k++) {
      current_color = colors_used[k];
      nv = pcr_nselectedvars;
      for (j=0; j<*nprofiles; j++) {
        id = *(profile_ids+j);
        if (xg->color_now[id] != current_color || xg->erased[id]) 
          nv += (pcr_nselectedvars-1);
        else {  /* current_color && !erased */
          for (i=0; i<pcr_nselectedvars-1; i++) {
            var1 = pcr_selectedvars[i];
            var2 = pcr_selectedvars[i+1];
            if (!plot_imputed_values && xg->missing_values_present &&
                (xg->is_missing[id][var1] || xg->is_missing[id][var2]))
              ;
            else {
              (void) fprintf(psfile, "%f %f %f 0 %f %f %f %f ln\n",
                (float) rgb[j].red / (float) 65535,
                (float) rgb[j].green / (float) 65535,
                (float) rgb[j].blue / (float) 65535,
                (float) (pcr_screen[i].x - minx) * fac + xoff,
                (float) (maxy - pcr_screen[nv].y) * fac + yoff,
                (float) (pcr_screen[i+1].x - minx) * fac + xoff,
                (float) (maxy - pcr_screen[nv+1].y) * fac + yoff );
            }
            nv++;
          }
        }
        nv++;
      }
    }
  }

/*
 * At the moment, once the cursor leaves the plot window,
 * no profile is drawn.  So it is meaningless to draw
 * the point and line for nearest_point.  Leave them out.
*/


  /*
   * Draw x ticks, and if (pcr_showcoords), also draw the axes
  */

  (void) fprintf(psfile, "%% xtx: (label) red green blue x y\n");
  (void) fprintf(psfile, "%%  draw x axis tick and label\n");
  for (i=0; i<pcr_ticks.nticks[0]; i++)
  {
    find_pcr_tick_label(pcr_xdeci, pcr_selectedvars[i],
      pcr_ticks.xticks, str);

    /*
     * we're printing 'i' here instead of the variable index;
     * this should fix it.  It assumes we're labelling all
     * ticks -- but we can use the same arbitrary scheme used
     * elsewhere in this file.
     */
    if (pcr_ticks.nticks[0] > 30 && i%4 != 0)
      sprintf(str, "");
    else if (pcr_ticks.nticks[0] <= 30 &&
             pcr_ticks.nticks[0] > 20 && i%2 != 0)
      sprintf(str, "");

    (void) fprintf(psfile, "(%s) %f %f %f %f %f xtx\n",
      str,
      (float) fg->red / (float) 65535,
      (float) fg->green / (float) 65535, 
      (float) fg->blue / (float) 65535,
      (float) (pcr_ticks.screen[i].x - minx) * fac + xoff,
      (float) (maxy - pcr_screen_axes[1].y) * fac + yoff);

    if (pcr_showcoords)
      (void) fprintf(psfile, "%f %f %f 0 %f %f %f %f ln\n",
        (float) fg->red / (float) 65535,
        (float) fg->green / (float) 65535,
        (float) fg->blue / (float) 65535,
        (float) (pcr_ticks.screen[i].x - minx) * fac + xoff,
        (float) (maxy - pcr_screen_axes[1].y) * fac + yoff,
        (float) (pcr_ticks.screen[i].x - minx) * fac + xoff,
        (float) (maxy - pcr_screen_axes[0].y) * fac + yoff );
  }

  /*
   * Draw y axis
  */
  if (use_common_scale) {

    (void) fprintf(psfile, "%% yax: (label) red green blue x1 y1 x2 y2\n");
    (void) fprintf(psfile, "%%  draw y axis (and null label)\n");
    (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f yax\n",
      "",
      (float) fg->red / (float) 65535,
      (float) fg->green / (float) 65535,
      (float) fg->blue / (float) 65535,
      (float) (pcr_screen_axes[1].x - minx) * fac + xoff,
      (float) (maxy - pcr_screen_axes[1].y) * fac + yoff,
      (float) (pcr_screen_axes[0].x - minx) * fac + xoff,
      (float) (maxy - pcr_screen_axes[0].y) * fac + yoff );

    (void) fprintf(psfile, "%% ytx: (label) red green blue x y\n");
    (void) fprintf(psfile, "%%  draw y axis tick and label\n");
    offset = find_pcr_ytick_offset();
    toomany = find_pcr_ytick_toomany();
    for (i=offset; i<pcr_ticks.nticks[1]-toomany; i++)
    {
      find_pcr_tick_label(pcr_ydeci, i, pcr_ticks.yticks, str);
      (void) fprintf(psfile, "(%s) %f %f %f %f %f ytx\n",
        str,
        (float) fg->red / (float) 65535,
        (float) fg->green / (float) 65535,
        (float) fg->blue / (float) 65535,
        (float) (pcr_screen_axes[1].x - minx) * fac + xoff,
        (float) (maxy - pcr_ticks.screen[i].y) * fac + yoff);
    }
  }

  XtFree((XtPointer) rgb);
}

