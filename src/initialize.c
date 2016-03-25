/* initialize.c */
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
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

void
init_brush_colors(AppData *appdat)
{
  XColor exact;
  register int j;
  Colormap cmap = DefaultColormap(display, DefaultScreen(display));

  ncolors = NCOLORS;
  color_names[0] = (char *) appdat->brushColor0;
  color_names[1] = (char *) appdat->brushColor1;
  color_names[2] = (char *) appdat->brushColor2;
  color_names[3] = (char *) appdat->brushColor3;
  color_names[4] = (char *) appdat->brushColor4;
  color_names[5] = (char *) appdat->brushColor5;
  color_names[6] = (char *) appdat->brushColor6;
  color_names[7] = (char *) appdat->brushColor7;
  color_names[8] = (char *) appdat->brushColor8;
  color_names[9] = (char *) appdat->brushColor9;

  for (j=0; j<ncolors; j++) {
    if (XParseColor(display, cmap, color_names[j], &exact)) {
      if (XAllocColor(display, cmap, &exact)) {
        color_nums[j] = exact.pixel;
      }
    }
    else
      (void) fprintf(stderr, "finding %s failed\n", color_names[j]);
  }
}

void
init_GCs(xgobidata *xg)
{
  unsigned long bg, fg;
  Window root_window = RootWindowOfScreen(XtScreen(xg->shell));
  XGCValues gcv;
  unsigned long mask =
    GCForeground | GCBackground | GCFunction ;

  /* 
   * Some documentation suggests that this might result
   * in some speedup; I see no difference.  dfs 3/93
  */
  gcv.graphics_exposures = False;

  if (mono) {
    plotcolors.fg = appdata.fg;
    plotcolors.bg = appdata.bg;
  }

  XtVaGetValues(xg->vardraww[0],
    XtNbackground, &bg,
    XtNforeground, &fg,
    NULL);

  /*
   * Create a Graphics Context with copy drawing logic.  It's used
   * for drawing the bull's eye or target circle in the middle of
   * the variable circles as well as the circles and variable bars.
  */

  gcv.foreground = fg;
  gcv.background = bg;
  gcv.function = GXcopy;
  varpanel_copy_GC = XCreateGC(display, root_window, mask, &gcv);

  /*
   * Create a Graphics Context and give it xor drawing logic.
   * This GC is used in drawing the variable circles, and its
   * foreground and background colors come from xor-ing the colors
   * of the form widget with a 1, I think.  The reason for using
   * xor logic here is the need for rapid redrawing of lines during
   * rotation and touring.
  */
  gcv.foreground = (unsigned long) 0xffffffff;
  gcv.background = bg;
  gcv.function = GXxor;
  varpanel_xor_GC = XCreateGC(display, root_window, mask, &gcv);
  XSetPlaneMask(display, varpanel_xor_GC, fg ^ bg);

  /*
   * Create a Graphics Context and give it copy drawing logic.
   * The copy_GC and the clear_GC are used for drawing inside the
   * plot window.  Their fg and bg colors come from the resource
   * values of PlotWorkspace.
  */
  gcv.foreground = plotcolors.fg;
  gcv.background = plotcolors.bg;
  gcv.function = GXcopy;
  copy_GC = XCreateGC(display, root_window, mask, &gcv);
  /*
   * Create another Graphics Context and give it copy drawing logic.
  */
  gcv.foreground = plotcolors.bg;
  gcv.background = plotcolors.fg;
  gcv.function = GXcopy;
  clear_GC = XCreateGC(display, root_window, mask, &gcv);
  /*
   * Set font for drawing labels; needed for axis label computations.
  */
  XSetFont(display, copy_GC, appdata.plotFont->fid);
  XSetFont(display, clear_GC, appdata.plotFont->fid);
}

void
init_plotwindow_vars(xgobidata *xg, int firsttime)
{
  if (firsttime)
  {
    xg->max.x = (int) xg->plotsize.width;
    xg->max.y = (int) xg->plotsize.height;
    xg->minxy = MIN(xg->max.x, xg->max.y);
    xg->mid.x = xg->max.x / 2;
    xg->mid.y = xg->max.y / 2;
  }
  if (!mono)
    xg->color_id = plotcolors.fg;
}
