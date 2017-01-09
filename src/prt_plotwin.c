/* prt_plotwin.c */
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

/*
 * The code in this file was written with a great deal of help
 * from Nat Howard at Bellcore.
*/
/*
 * 24-Aug-93: Modified by Sylvain G. Korzennik (sylvain@cfa.havard.edu)
 *
 *  To add the full title and an ID stamp to the PS file
 *
 *    The title is retrieved from the xg->shell "title" resource,
 *      it is printed above the box, in Helvetica, using twice the given 
 *      pointsize.
 *      Note that the full title can be modified via the standard Xt
 *      options -title and -name.
 *    The ID is printed at location (42,42) (in points) in Times-Roman 6 pts
 *      and consist of "version username hostname date".
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"

#include "xgobi_config.h"

static jmp_buf env;

/*ARGSUSED*/
static void
printer_undefined(int arg)
{
/*
 * This is called if the printer command fails miserably, for
 * example, if a printer not in /etc/printcap is named.  It
 * doesn't catch the error message from lpr because I can't
 * figure out how; it also is invoked about 3 times, which
 * is strange.  It prevents a core dump, though, so it's
 * worth it.  dfs, 4/93
 * The longjmp/setjmp should prevent this routine from being
 * executed more than once.  dfs, 9/93
*/
  char message[MSGLENGTH]; 
  extern xgobidata xgobi;

  sprintf(message, "The printing command failed;\n");
  strcat(message, "does the specified printer exist?\n");
  show_message(message, &xgobi);

  longjmp(env, 0);
}

/* ARGSUSED */
XtActionProc
Bell(Widget w, XEvent *evnt, String *params, Cardinal nparams)
/*
 * Used to override carriage return in text widgets, simply
 * beeping instead.
*/
{
  XBell(display, 0);
}

#define PSXMIN 1.0
#define PSXMAX 7.5
#define PSYMIN 1.0
#define PSYMAX 10.0

#define FILENAME 1
#define PRINTER 2

#define NWINBTNS 3
static Widget win_menu_cmd, win_menu, win_menu_btn[NWINBTNS];
static Widget win_menu_box, win_menu_lab;
#define PLOTWIN 0
#define CASEWIN 1   /* Par coords window */
#define PROJPURSUITWIN 2
static int whichwin = PLOTWIN;
static char *win_menu_str = "Window to plot:";
static char *win_menu_name[] = {
  "Main plotting window",
  "Parallel coordinates window",
  "Projection pursuit window",
};
static char *win_menu_nickname[] = {
  "main",
  "par coords",
  "proj pursuit",
};


Widget ppopup, pframe, pform;
Widget file_label, file_text, print_label, print_text;
Widget bg_label, bg_text, fg_label, fg_text, white_label, white_text;
Widget pointsize_label, pointsize_text;
int pointsize = 11;
char *pointsize_str = "11";
Widget file_cmd, print_cmd, close_cmd;
static char *outfile;
static char *printing_cmd;

static char *bgname;
static char *fgname;
static char *whitename;


void
set_fac_and_offsets(float minx, float maxx, float maxy,
float *fac, float *xoff, float *yoff)
{
/*
 * minx was introduced to solve a problem with the axes.  When a
 * large font is used, the scaling and offsets have to be adjusted
 * so that an extra amount of space is alloted for the plot without
 * disturbing the plot's aspect ratio.
*/
  if ( (maxx-minx) / (PSXMAX - PSXMIN) > maxy / (PSYMAX - PSYMIN))
    *fac = (PSXMAX - PSXMIN) / (maxx-minx);
  else
    *fac = (PSYMAX - PSYMIN) / maxy;

  *xoff = 0.5 * (8.5 - *fac * (maxx-minx));
  *yoff = 0.5 * (11. - *fac * maxy);
}

int
find_pgsize(int type, int size)
{
/*
 * This does some adjustments to the raw sizes to make them
 * print nicely.  It uses the same rules as the routines
 * in plot_once():  build_x, build_plus, build_circle, etc.
*/
  int isize = size;

  switch (type)
  {
    case PLUS_GLYPH:
      switch (isize)
      {
        case TINY:
        case SMALL:
          break;
        case MEDIUM:
        case LARGE:
          isize++ ;
          break;
        case JUMBO:
          isize = isize + 2 ;
          break;
        default:
          fprintf(stderr, "error in build_plus; impossible size %d\n", size);
          isize = (short) MEDIUM + 1 ;
          break;
      }
      isize = 2*isize + 1;  /* This is how the plusses are built */
      break;

    case X_GLYPH:
      isize = 2*isize + 1;  /* This is how the xes are built */
      break;

    case OPEN_RECTANGLE_GLYPH:
    case FILLED_RECTANGLE_GLYPH:
      isize = isize * 3 - 1;
      break;

    case OPEN_CIRCLE_GLYPH:
    case FILLED_CIRCLE_GLYPH:
      isize = isize * 3 ;
      break;

    case POINT_GLYPH:
      break;
  }

  return(isize);
}

/* ARGSUSED */
void
print_plot_window(Widget w, int type, xgobidata *xg)
  /* type = FILENAME or PRINTER */
{
  char headfname[156];
  register int ch;
  FILE *psfile, *header;
  XColor *rgb_table, *rgb_points, bg, fg, whitepix, truewhite, exact;
  XColor *rgb_lines;
  int i, k, m;
  int raw_axis_len;
  int start;
  char str[32];
  Colormap cmap = DefaultColormap(display, DefaultScreen(display));
  float maxx, minx = 0., maxy, miny = 0.;
  float fac, xoff, yoff;
  int pgsize;
  time_t now;
  char *xgobidir;
  char message[MSGLENGTH];
  Boolean doit;
/*
  XImage *xi;
  unsigned short int pixout;
  unsigned short int shift;
#define bits 0x0001
*/

/*
 * Determine which window to print.
*/

  switch (whichwin)
  {
    case CASEWIN:

    get_cprof_win_dims(&maxx, &maxy, xg) ;
    break ;

    case PROJPURSUITWIN:

    get_pp_win_dims(xg, &maxx, &maxy) ;
    break ;

    case PLOTWIN:

    maxx = (float) xg->max.x ;
    maxy = (float) xg->max.y ;
    break ;
  }

  if (maxx <= 0 || maxy <= 0)
  {
    fprintf(stderr, "Can't print the selected window.\n");
    return;
  }

/*
 * Get the foreground and background colors, as well as the color
 * to use for drawing white glyphs.
*/
  XtVaGetValues(bg_text, XtNstring, (String) &bgname, NULL);
  if ( !XAllocNamedColor(display, cmap, bgname, &bg, &exact) )
  {
    sprintf(message, "Background pixel value not found.\n");
    show_message(message, xg);
    return;
  }

  XtVaGetValues(fg_text, XtNstring, (String) &fgname, NULL);
  if ( !XAllocNamedColor(display, cmap, fgname, &fg, &exact) )
  {
    sprintf(message, "Foreground pixel value not found.\n");
    show_message(message, xg);
    return;
  }

  XtVaGetValues(white_text, XtNstring, (String) &whitename, NULL);
  if ( !XAllocNamedColor(display, cmap, whitename, &whitepix, &exact) )
  {
    fprintf(stderr, "white pixel value '%s' not found\n", whitename);
    return;
  }

/*
 * Get the pointsize and reject it if it isn't between 2 and 30.
*/
  XtVaGetValues(pointsize_text, XtNstring, (String) &pointsize_str, NULL);
  pointsize = atoi(pointsize_str);
  /* if ( pointsize < 8 || pointsize > 30) */
  if ( pointsize < 2 || pointsize > 30)
  {
    sprintf(message, "Only pointsizes between 2 and 30 are permitted.\n");
    show_message(message, xg);
    return;
  }

/*
 * Scale data onto postscript bounding box, using the previously
 * determined window size.
*/
  set_fac_and_offsets(minx, maxx, maxy, &fac, &xoff, &yoff);

/*
 * Adjust the values of fac, xoff, and yoff to appropriately
 * handle the current point size.
*/
  if (whichwin == PLOTWIN && xg->is_axes)
  {
    /*
     * If an axis is inside the plotting region, assume
     * the user wants it in the plot.  While a label falls
     * outside the postscript plotting region,
     * reduce fac, xoff, and yoff accordingly.
     * Sacrifice the plot aspect ratio.
     * The constants come from xgobi.head.ps.
    */

    if ((xg->is_xyplotting || xg->is_plotting1d) &&
        xg->screen_axes[1].x >= 0)
    {
      while (72.*((xg->screen_axes[1].x - minx) * fac + xoff) -
             4.0*pointsize < 72.*xoff)
      {
        minx = minx - .02 * (maxx - minx) ;
        set_fac_and_offsets(minx, maxx, maxy, &fac, &xoff, &yoff);
      }
    }

    if (xg->is_xyplotting &&
        xg->screen_axes[1].y <= xg->max.y)
    {
      while (72.*((maxy - (float) xg->screen_axes[1].y) * fac + yoff) -
                 4.0*pointsize < 72.*yoff)
      {
        maxy = 1.02 * maxy ;
        set_fac_and_offsets(minx, maxx, maxy, &fac, &xoff, &yoff);
      }
    }

  }
  else if (whichwin == CASEWIN)
  /*
   * I'll assume that we're cprof_plotting here, that it's been
   * checked earlier in this function.
  */
  {
    check_cprof_fac_and_offsets(&minx, &maxx, &maxy, &fac, &xoff, &yoff,
      pointsize);
  }
  else if (whichwin == PROJPURSUITWIN)
  {
    check_pp_fac_and_offsets(xg, &minx, &maxx, &maxy, &fac, &xoff, &yoff,
      pointsize);
    /*
     * At this time, miny is not used for scaling, just for drawing
     * the line above the plot.  Maybe it'll be used more in the
     * future.
    */
    miny = - 4 * pointsize ;
  }

  fac = 72.0 * fac;
  xoff = 72.0 * xoff;
  yoff = 72.0 * yoff;

/*
 * Get the XColor value for the string "white" to use in
 * locating the white glyphs.
*/
  if ( !XAllocNamedColor(display, cmap, "white", &truewhite, &exact) )
  {
    fprintf(stderr, "the color white is not found\n");
    return;
  }

/*
 * Get the XColor values of all the brushing colors.
*/
  rgb_table = (XColor *) XtMalloc((Cardinal)
    ncolors * sizeof(XColor));
  for (k=0; k<ncolors; k++)
    XParseColor(display, cmap, color_names[k], &rgb_table[k]);

  if (type == FILENAME) {
    if ( (psfile = fopen(outfile, "w")) == NULL) {
      sprintf(message, "Failed to open the file %s for writing.\n", outfile);
      show_message(message, xg);
      return;
    }
  }
  else {  /* if PRINTER */
    if (printing_cmd == NULL || INT(strlen(printing_cmd)) < 1) {
      sprintf(message, "The printing command is missing.\n");
      show_message(message, xg);
      return;
    }
    else {
      /*
       * Catch popen errors
      */
      signal(SIGPIPE, printer_undefined);

      /* casting everything for some cranky SGIs */
      if ((psfile = (FILE *) popen(printing_cmd, "w")) == (FILE *) NULL) {
        sprintf(message, "Failed to open connection to printer.\n");
        show_message(message, xg);
        return;
      }
    }
  }

  if (setjmp(env)) {
    XtFree((XtPointer) rgb_table);

    if (type == FILENAME)
      (void) fclose(psfile);
    else if (type == PRINTER)
      (void) pclose(psfile);

    return;
  }

  now = (time_t) time((time_t *) 0);

  (void) fprintf(psfile, "%s\n",   "%!PS-Adobe-3.0 EPSF-3.0" );
  (void) fprintf(psfile, "%s %d %d %d %d\n",
                                   "%%BoundingBox: ",
                   /* x ll */      (int) xoff,
                   /* y ll */      (int) yoff,
                   /* x ur */      (int) ((maxx-minx) * fac + xoff + 1),
                   /* y ur */      (int) ((maxy-miny) * fac + yoff + 1));
  (void) fprintf(psfile, "%s\n",   "%%Creator: XGobi" );
  (void) fprintf(psfile, "%s\n",   "%%Title: XGobi plot window" );
  (void) fprintf(psfile, "%s%s\n", "%%CreationDate: ", ctime(&now) );
  (void) fprintf(psfile, "%s\n",   "%%EndComments" );

/* Make this optional; it's too slow and makes big output files */
/* And watch out: this only handles whichwin = PLOTWIN */
/*
 * Create an X image to be used to create the preview
 * portion, in EPSI format, of the postscript output.
  xi = XGetImage(display, xg->pixmap0, (int) 0, (int) 0,
    (unsigned int) maxx, (unsigned int) maxy,
    (unsigned long) AllPlanes, (int) XYPixmap);
  (void) fprintf(psfile, "%%%%BeginPreview: %d %d %d %d\n",
    (int) maxx, (int) maxy, depth, (int) ((maxx*maxy/depth)/32));
  k = 0;
  (void) fprintf(psfile, "%%");
  for (i=0; i<(int)maxy; i++)
  {
    j = 0;
    while (j < (int)maxx)
    {
      pixout = 0;
      for (m=0; m<8 && j<(int)maxx; m++, j++)
      {
        if (XGetPixel(xi, j, i) != bg.pixel)
        {
          shift = (8-1)-m ;
          pixout |= (bits << shift);
        }
      }
      (void) fprintf(psfile, "%.2x", pixout);
      k++;
      if (k == 32)
      {
        (void) fprintf(psfile, "\n%%");
        k = 0;
      }
    }
  }
  XDestroyImage(xi);
  (void) fprintf(psfile, "\n%%%%EndPreview\n\n");
*/

  (void) fprintf(psfile, "%s\n",   "%%!" );
  (void) fprintf(psfile, "%s %d %s\n",
                                   "/pointsize", pointsize, "def");

/* sylvain */
/*
  (void) gethostname(hostname, 150);
  (void) fprintf(psfile, "/Times-Roman findfont 6 scalefont setfont\n");
  (void) fprintf(psfile, "42 42 moveto (%s@%s %s)show\n",
     cuserid(NULL), hostname, ctime(&now));
*/

/*
 * Now copy xgobi.head.ps into outfile.
*/
  xgobidir = getenv("XGOBID");
  if (xgobidir == NULL || strlen(xgobidir) == 0)
  {
    xgobidir = (char *) XtMalloc((Cardinal) 150 * sizeof(char));
    (void) strcpy(xgobidir, XGOBI_DEFAULTDIR);
    if (xgobidir == NULL || strlen(xgobidir) == 0)
    {
      sprintf(message,
       "XGOBID is not defined in your environment, and\n");
      strcat(message,
       "XGOBI_DEFAULTDIR is not defined in the XGobi Makefile;\n");
      strcat(message,
        "see the person who installed XGobi for assistance.\n");
      show_message(message, xg);

      return;
    }
    else
    {
      (void) strcpy(headfname, xgobidir);
      XtFree((XtPointer) xgobidir);
    }
  }
  else
  {
    (void) strcpy(headfname, xgobidir);
  }

  (void) strcat(headfname, "/ps/xgobi.head.ps");

  header = fopen(headfname, "r");
  if (header == NULL)
  {
    sprintf(message,
      "Unable to open %s.\n", headfname);
    strcat(message,
      "Is the shell variable XGOBID the name of the directory\n");
    strcat(message,
      "which contains the ps subdirectory?\n");
    show_message(message, xg);

    return;
  }
  else
  {
    while ( (ch = getc(header)) != EOF)
      putc((char) ch, psfile);
    fclose(header);
  }

/* sylvain */
/*
  XtVaGetValues(xg->shell, XtNtitle, (String) &fulltitle, NULL);
  (void) fprintf(psfile, "%% put a title above the frame\n");
  (void) fprintf(psfile, 
    "/Helvetica findfont pointsize 2 mul scalefont setfont\n");
  (void) fprintf(psfile, "%f %f pointsize 2.5 mul add moveto (%s) show\n",
     xoff, (maxy-miny) * fac + yoff, fulltitle);
  (void) fprintf(psfile, 
    "/Helvetica findfont pointsize scalefont setfont\n");
*/

/*
 * Print out the size and background color of the plotting window.
*/
  (void) fprintf(psfile,
    "%% xgobiinit:  r g b r g b xoffset yoffset max_x_size max_y_size\n");
  (void) fprintf(psfile,
    "%f %f %f %f %f %f %f %f %f %f gobiinit\n",
    (float) fg.red / (float) 65535,
    (float) fg.green / (float) 65535,
    (float) fg.blue / (float) 65535,
    (float) bg.red / (float) 65535,
    (float) bg.green / (float) 65535,
    (float) bg.blue / (float) 65535,
    (float) xoff,
    (float) yoff,
    (maxx-minx) * fac + xoff,
    (maxy-miny) * fac + yoff );

  if (whichwin == PLOTWIN)
  {
    /*
     * Send the plotting window to the printer or to a postscript file.
    */

  /*
   * Add connected lines.
  */
    if ((xg->connect_the_points || xg->plot_the_arrows) && xg->nlines > 0)
    {
      int from, to;
      extern  Boolean plot_imputed_values;
      float xx, yy;
      (void) fprintf(psfile, "%% ln: red green blue x1 y1 x2 y2\n");
      (void) fprintf(psfile, "%%  draw line from (x1,y1) to (x2,y2)\n");

    /*
     * Build a vector of XColor values for each line.
    */
      rgb_lines = (XColor *) XtMalloc((Cardinal)
        xg->nlines * sizeof(XColor));
      for (i=0; i<xg->nlines; i++) {
        if (xg->line_color_now[i] == truewhite.pixel) {
          rgb_lines[i].red = whitepix.red;
          rgb_lines[i].green = whitepix.green;
          rgb_lines[i].blue = whitepix.blue;
        }
        else {
          for (k=0; k<ncolors; k++) {
            if (xg->line_color_now[i] == color_nums[k]) {
              rgb_lines[i].red = rgb_table[k].red;
              rgb_lines[i].green = rgb_table[k].green;
              rgb_lines[i].blue = rgb_table[k].blue;
              break;
            }
          }
        }
      }

      /* r g b width x1 y1 x2 y2 ln */
      for (i=0; i<xg->nlines; i++)
      {
        from = xg->connecting_lines[i].a - 1;
        to = xg->connecting_lines[i].b - 1;

/* dfs, testing */
        /* If not plotting imputed values, and one is missing, skip it */
        doit = True;
        if (!plot_imputed_values && plotted_var_missing(from, to, xg))
          doit = False;
        /* If either from or to is excluded, move on */
        else if (xg->ncols == xg->ncols_used) {
          if (xg->clusv[(int)GROUPID(from)].excluded)
            doit = False;
          else if (xg->clusv[(int)GROUPID(to)].excluded)
            doit = False;
        }
        if (!doit)
          continue;

        if (!xg->erased[from] && !xg->erased[to]) {
          (void) fprintf(psfile, "%f %f %f 1 %f %f %f %f ln\n",
            (float) rgb_lines[i].red / (float) 65535,
            (float) rgb_lines[i].green / (float) 65535,
            (float) rgb_lines[i].blue / (float) 65535,
            (float) (xg->screen[from].x - minx) * fac + xoff,
            (float) (maxy - xg->screen[from].y) * fac + yoff,
            (float) (xg->screen[to].x - minx) * fac + xoff,
            (float) (maxy - xg->screen[to].y) * fac + yoff );
          if (xg->plot_the_arrows) {
            xx = (.2 * xg->screen[from].x + .8 * xg->screen[to].x) - minx ;
            yy = maxy - (.2 * xg->screen[from].y + .8 * xg->screen[to].y) ;
            (void) fprintf(psfile, "%f %f %f 3 %f %f %f %f ln\n",
              (float) rgb_lines[i].red / (float) 65535,
              (float) rgb_lines[i].green / (float) 65535,
              (float) rgb_lines[i].blue / (float) 65535,
              (float) xx * fac + xoff,
              (float) yy * fac + yoff,
              (float) (xg->screen[to].x - minx) * fac + xoff,
              (float) (maxy - xg->screen[to].y) * fac + yoff );
          }
        }
      }
    }

  /*
   * Add smoother lines.
  */
    if (xg->is_smoothing)
    {
      extern void ps_smooth(FILE *, float, float, float, float, float, XColor,
        XColor *);
      (void) fprintf(psfile, "%% ln: red green blue x1 y1 x2 y2\n");
      (void) fprintf(psfile, "%%  draw line from (x1,y1) to (x2,y2)\n");
      ps_smooth(psfile, fac, xoff, yoff, minx, maxy, whitepix, rgb_table);
    }

    if (xg->plot_the_points)
    {
    /*
     * Build a vector of XColor values for each point.
    */
      rgb_points = (XColor *) XtMalloc((Cardinal)
        xg->nrows * sizeof(XColor));
      for (m=0; m<xg->nrows_in_plot; m++)
      {
        i = xg->rows_in_plot[m];
        if (!xg->erased[i])
        {
          if (xg->color_now[i] == truewhite.pixel)
          {
            rgb_points[i].red =   whitepix.red;
            rgb_points[i].green = whitepix.green;
            rgb_points[i].blue =  whitepix.blue;
          }
          else
          {
            for (k=0; k<ncolors; k++)
            {
              if (xg->color_now[i] == color_nums[k])
              {
                rgb_points[i].red = rgb_table[k].red;
                rgb_points[i].green = rgb_table[k].green;
                rgb_points[i].blue = rgb_table[k].blue;
                break;
              }
            }
          }
        }
      }
      /*
       * Draw the points
      */
      (void) fprintf(psfile,
        "%% pg: red green blue glyph_type glyph_size x y\n");
      for (m=0; m<xg->nrows_in_plot; m++)
      {
        i = xg->rows_in_plot[m];
        if (!xg->erased[i])
        {
          if (xg->is_touring && xg->is_tour_section) /*bugfix*/
            pgsize = find_pgsize(xg->section_glyph_ids[i].type,
              xg->section_glyph_ids[i].size);
          else
            pgsize = find_pgsize(xg->glyph_now[i].type, xg->glyph_now[i].size);

          (void) fprintf(psfile, "%f %f %f %d %d %f %f pg\n",
            (float) rgb_points[i].red / (float) 65535,
            (float) rgb_points[i].green / (float) 65535,
            (float) rgb_points[i].blue / (float) 65535,
            xg->glyph_now[i].type,
            pgsize,
            (float) (xg->screen[i].x - minx) * fac + xoff,
            (float) (maxy - xg->screen[i].y) * fac + yoff );
        }
      }
    }

  /*
   * Write out the locations and strings of sticky labelled points.
  */
    (void) fprintf(psfile,
      "%% sticky: (string) red green blue x y\n");
    (void) fprintf(psfile,
      "%%  place string a small distance to the right and\n");
    (void) fprintf(psfile,
      "%%  above x,y\n");
  /*
    for (m=0; m<xg->nrows; m++)
      i = xg->rows_in_plot[m];
      if (xg->sticky[i])
  */
    for (m=0; m<xg->nsticky_ids; m++)
    {
        i = xg->sticky_ids[m] ;
        (void) fprintf(psfile, "(%s) %f %f %f %f %f sticky\n",
          xg->rowlab[i],
          (float) fg.red / (float) 65535,
          (float) fg.green / (float) 65535,
          (float) fg.blue / (float) 65535,
          (float) (xg->screen[i].x - minx) * fac + xoff,
          (float) (maxy - xg->screen[i].y) * fac + yoff );
    }

  /*
   * Give necessary information for drawing axes.
  */
    if (xg->is_axes)
    {
      if (xg->is_plotting1d)
      {
        if (xg->plot1d_vars.y != -1) {
          (void) fprintf(psfile, "%% yax: (label) red green blue x1 y1 x2 y2\n");
          (void) fprintf(psfile, "%%  draw y axis and label\n");
          (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f yax\n",
            xg->collab_tform2[xg->plot1d_vars.y],
            (float) fg.red / (float) 65535,
            (float) fg.green / (float) 65535,
            (float) fg.blue / (float) 65535,
            (float) (xg->screen_axes[1].x - minx) * fac + xoff,
            (float) (maxy - xg->screen_axes[1].y) * fac + yoff,
            (float) (xg->screen_axes[0].x - minx) * fac + xoff,
            (float) (maxy - xg->screen_axes[0].y) * fac + yoff );

          (void) fprintf(psfile, "%% ytx: (label) red green blue x y\n");
          (void) fprintf(psfile, "%%  draw y axis tick and label\n");
          start = check_y_axis(xg, xg->plot1d_vars.y, &xg->ticks);
          for (i=start; i<xg->ticks.nticks[xg->plot1d_vars.y]; i++)
          {
            find_tick_label(xg, xg->plot1d_vars.y, i,
              xg->ticks.yticks, str);
            (void) fprintf(psfile, "(%s) %f %f %f %f %f ytx\n",
              str,
              (float) fg.red / (float) 65535,
              (float) fg.green / (float) 65535,
              (float) fg.blue / (float) 65535,
              (float) (xg->screen_axes[1].x - minx) * fac + xoff,
              (float) (maxy - xg->ticks.screen[i].y) * fac + yoff );
          }
	} else {
          (void) fprintf(psfile, "%% yax: (label) red green blue x1 y1 x2 y2\n");
          (void) fprintf(psfile, "%%  draw x axis and label\n");
          (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f xax\n",
            xg->collab_tform2[xg->plot1d_vars.x],
            (float) fg.red / (float) 65535,
            (float) fg.green / (float) 65535,
            (float) fg.blue / (float) 65535,
            (float) (xg->screen_axes[1].x - minx) * fac + xoff,
            (float) (maxy - xg->screen_axes[1].y) * fac + yoff,
            (float) (xg->screen_axes[2].x - minx) * fac + xoff,
            (float) (maxy - xg->screen_axes[2].y) * fac + yoff );

          (void) fprintf(psfile, "%% xtx: (label) red green blue x y\n");
          (void) fprintf(psfile, "%%  draw x axis tick and label\n");
          start = check_x_axis(xg, xg->plot1d_vars.x, &xg->ticks);
          for (i=start; i<xg->ticks.nticks[xg->plot1d_vars.x]; i++)
          {
            find_tick_label(xg, xg->plot1d_vars.x, i,
              xg->ticks.xticks, str);
            (void) fprintf(psfile, "(%s) %f %f %f %f %f xtx\n",
              str,
              (float) fg.red / (float) 65535,
              (float) fg.green / (float) 65535,
              (float) fg.blue / (float) 65535,
              (float) (xg->ticks.screen[i].x - minx) * fac + xoff,
              (float) (maxy - xg->screen_axes[1].y) * fac + yoff );
          }
	}


      }
      else if (xg->is_xyplotting)
      {
        (void) fprintf(psfile, "%% yax: (label) red green blue x1 y1 x2 y2\n");
        (void) fprintf(psfile, "%%  draw y axis and label\n");
        (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f yax\n",
          xg->collab_tform2[xg->xy_vars.y],
          (float) fg.red / (float) 65535,
          (float) fg.green / (float) 65535,
          (float) fg.blue / (float) 65535,
          (float) (xg->screen_axes[1].x - minx) * fac + xoff,
          (float) (maxy - xg->screen_axes[1].y) * fac + yoff,
          (float) (xg->screen_axes[0].x - minx) * fac + xoff,
          (float) (maxy - xg->screen_axes[0].y) * fac + yoff );

        (void) fprintf(psfile, "%% ytx: (label) red green blue x y\n");
        (void) fprintf(psfile, "%%  draw y axis tick and label\n");
        start = check_y_axis(xg, xg->xy_vars.y, &xg->ticks);
        for (i=start; i<xg->ticks.nticks[xg->xy_vars.y]; i++)
        {
          find_tick_label(xg, xg->xy_vars.y, i,
            xg->ticks.yticks, str);
          (void) fprintf(psfile, "(%s) %f %f %f %f %f ytx\n",
            str,
            (float) fg.red / (float) 65535,
            (float) fg.green / (float) 65535,
            (float) fg.blue / (float) 65535,
            (float) (xg->screen_axes[1].x - minx) * fac + xoff,
            (float) (maxy - xg->ticks.screen[i].y) * fac + yoff);
        }

        (void) fprintf(psfile, "%% xax: (label) red green blue x1 y1 x2 y2\n");
        (void) fprintf(psfile, "%%  draw x axis and label\n");
        (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f xax\n",
          xg->collab_tform2[xg->xy_vars.x],
          (float) fg.red / (float) 65535,
          (float) fg.green / (float) 65535,
          (float) fg.blue / (float) 65535,
          (float) (xg->screen_axes[1].x - minx) * fac + xoff,
          (float) (maxy - xg->screen_axes[1].y) * fac + yoff,
          (float) (xg->screen_axes[2].x - minx) * fac + xoff,
          (float) (maxy - xg->screen_axes[2].y) * fac + yoff );

        (void) fprintf(psfile, "%% xtx: (label) red green blue x y\n");
        (void) fprintf(psfile, "%%  draw x axis tick and label\n");
        start = check_x_axis(xg, xg->xy_vars.x, &xg->ticks);
        for (i=start; i<xg->ticks.nticks[xg->xy_vars.x]; i++)
        {
          find_tick_label(xg, xg->xy_vars.x, i,
            xg->ticks.xticks, str);
          (void) fprintf(psfile, "(%s) %f %f %f %f %f xtx\n",
            str,
            (float) fg.red / (float) 65535,
            (float) fg.green / (float) 65535,
            (float) fg.blue / (float) 65535,
            (float) (xg->ticks.screen[i].x - minx) * fac + xoff,
            (float) (maxy - xg->screen_axes[1].y) * fac + yoff);
        }
      }
      else if (xg->is_spinning)
      {
        icoords cntr;
        (void) fprintf(psfile,
          "%% arbax: (label) red green blue x1 y1 x2 y2\n");
        (void) fprintf(psfile, "%%  draw arbitrary axis; place label above\n");
        (void) fprintf(psfile, "%%  and to the right of the point\n");
        raw_axis_len = MIN(xg->mid.x, xg->mid.y);

        if (xg->is_axes_centered) {
          cntr.x = xg->cntr.x;
          cntr.y = xg->cntr.y;
        } else {
          raw_axis_len /= 2.0;
          cntr.x = 1.1 * raw_axis_len * xg->scale.x;
          cntr.y = xg->plotsize.height - (1.1 * raw_axis_len * xg->scale.y);
        }

        if (xg->is_spin_type.oblique)
        {
          (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n", /* x */
            xg->collab_tform2[xg->spin_vars.x],
            (float) fg.red / (float) 65535,
            (float) fg.green / (float) 65535,
            (float) fg.blue / (float) 65535,
            (float) (cntr.x - minx) * fac + xoff,
            (float) (maxy - cntr.y) * fac + yoff,
            (float) (cntr.x +
             raw_axis_len * xg->scale.x * xg->Rmat0[0][0] - minx) *
             fac + xoff,
            (float) (maxy -
             (cntr.y -
             raw_axis_len * xg->scale.y * xg->Rmat0[1][0])) *
             fac + yoff );
          (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n", /* y */
            xg->collab_tform2[xg->spin_vars.y],
            (float) fg.red / (float) 65535,
            (float) fg.green / (float) 65535,
            (float) fg.blue / (float) 65535,
            (float) (cntr.x - minx) * fac + xoff,
            (float) (maxy - cntr.y) * fac + yoff,
            (float) (cntr.x +
             raw_axis_len * xg->scale.x * xg->Rmat0[0][1] - minx) *
             fac + xoff,
            (float) (maxy -
             (cntr.y -
             raw_axis_len * xg->scale.y * xg->Rmat0[1][1])) *
             fac + yoff );
          (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n", /* z */
            xg->collab_tform2[xg->spin_vars.z],
            (float) fg.red / (float) 65535,
            (float) fg.green / (float) 65535,
            (float) fg.blue / (float) 65535,
            (float) (cntr.x - minx) * fac + xoff,
            (float) (maxy - cntr.y) * fac + yoff,
            (float) (cntr.x +
             raw_axis_len * xg->scale.x * xg->Rmat0[0][2] - minx) *
             fac + xoff,
            (float) (maxy -
             (cntr.y -
             raw_axis_len * xg->scale.y * xg->Rmat0[1][2])) *
             fac + yoff );
        }
        else
        {
          if (xg->is_spin_type.yaxis)
          {
            (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n", /* y */
              xg->collab_tform2[xg->spin_vars.y],
              (float) fg.red / (float) 65535,
              (float) fg.green / (float) 65535,
              (float) fg.blue / (float) 65535,
              (float) (cntr.x - minx) * fac + xoff,
              (float) (maxy - cntr.y) * fac + yoff,
              (float) (cntr.x - minx) * fac + xoff,
              (float) (maxy -
               (cntr.y -
               raw_axis_len * xg->scale.y)) *
               fac + yoff);
            (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n", /* x */
              xg->collab_tform2[xg->spin_vars.x],
              (float) fg.red / (float) 65535,
              (float) fg.green / (float) 65535,
              (float) fg.blue / (float) 65535,
              (float) (cntr.x - minx) * fac + xoff,
              (float) (maxy - cntr.y) * fac + yoff,
              (float) (cntr.x + raw_axis_len * xg->scale.x *
                xg->icost.y/PRECISION2 - minx) * fac + xoff,
              (float) (maxy - cntr.y) * fac + yoff );
            (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n", /* z */
              xg->collab_tform2[xg->spin_vars.z],
              (float) fg.red / (float) 65535,
              (float) fg.green / (float) 65535,
              (float) fg.blue / (float) 65535,
              (float) (cntr.x - minx) * fac + xoff,
              (float) (maxy - cntr.y) * fac + yoff,
              (float) (cntr.x + raw_axis_len * xg->scale.x *
                xg->isint.y/PRECISION2 - minx) * fac + xoff,
              (float) (maxy - cntr.y) * fac + yoff );
          }
          else
          {
            (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n", /* x */
              xg->collab_tform2[xg->spin_vars.x],
              (float) fg.red / (float) 65535,
              (float) fg.green / (float) 65535,
              (float) fg.blue / (float) 65535,
              (float) (cntr.x - minx) * fac + xoff,
              (float) (maxy - cntr.y) * fac + yoff,
              (float) (cntr.x + raw_axis_len * xg->scale.x - minx) *
               fac + xoff,
              (float) (maxy - cntr.y) * fac + yoff );
            (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n", /* y */
              xg->collab_tform2[xg->spin_vars.y],
              (float) fg.red / (float) 65535,
              (float) fg.green / (float) 65535,
              (float) fg.blue / (float) 65535,
              (float) cntr.x * fac + xoff,
              (float) (maxy - cntr.y) * fac + yoff,
              (float) (cntr.x - minx) * fac + xoff,
              (float) (maxy -
               (cntr.y -
               raw_axis_len * xg->scale.x * xg->icost.x/PRECISION2)) *
               fac + yoff );
            (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n", /* z */
              xg->collab_tform2[xg->spin_vars.z],
              (float) fg.red / (float) 65535,
              (float) fg.green / (float) 65535,
              (float) fg.blue / (float) 65535,
              (float) (cntr.x - minx) * fac + xoff,
              (float) (maxy - cntr.y) * fac + yoff,
              (float) (cntr.x - minx) * fac + xoff,
              (float) (maxy - (cntr.y -
                 raw_axis_len * xg->scale.x * xg->isint.x/PRECISION2)) *
                 fac + yoff );
          }
        }
      }
      else if (xg->is_touring || xg->is_corr_touring)
      {
        int j;
        float tol = .01;
        fcoords axs;
        icoords cntr;

        raw_axis_len = MIN(xg->mid.x, xg->mid.y);
        axs.x = raw_axis_len / 2.;
        axs.y = raw_axis_len / 2.;
        (void) fprintf(psfile,
          "%% arbax: (label) red green blue x1 y1 x2 y2\n");
        (void) fprintf(psfile, "%%  draw arbitrary axis; place label above\n");
        (void) fprintf(psfile, "%%  and to the right of the point\n");

        if (xg->is_axes_centered) {
          cntr.x = xg->cntr.x;
          cntr.y = xg->cntr.y;
        } else {
          axs.x /= 2.0;
          axs.y /= 2.0;
          cntr.x = 1.1 * axs.x;
          cntr.y = xg->plotsize.height - (1.1 * axs.y);
        }

        for (j=0; j<xg->ncols_used; j++)
        {
          xg->tv[0][j] = xg->u[0][j];
          xg->tv[1][j] = xg->u[1][j];
        }

        if (xg->is_princ_comp)
        {
          if (!xg->is_pc_axes)
          {
            invert_proj_coords(xg);
            for (j=0; j<xg->ncols_used; j++)
            {
              if ((xg->tv[0][j]*xg->tv[0][j] +
                 xg->tv[1][j]*xg->tv[1][j]) > tol)
              {
                (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n",
                  xg->collab_tform2[j],
                  (float) fg.red / (float) 65535,
                  (float) fg.green / (float) 65535,
                  (float) fg.blue / (float) 65535,
                  (float) (cntr.x - minx) * fac + xoff,
                  (float) (maxy - cntr.y) * fac + yoff,
                  (float) (cntr.x + axs.x * xg->tv[0][j] - minx) *
                   fac + xoff,
                  (float) (maxy -
                   (cntr.y - axs.y * xg->tv[1][j])) *
                   fac + yoff );
              }
            }
          }
          else
          {
            int naxes = 0;
            for (j=0; j<xg->ncols_used; j++)
            {
              if ((xg->tv[0][j]*xg->tv[0][j] +
                 xg->tv[1][j]*xg->tv[1][j]) > tol)
              {
                (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n",
                  xg->tour_lab[naxes++],
                  (float) fg.red / (float) 65535,
                  (float) fg.green / (float) 65535,
                  (float) fg.blue / (float) 65535,
                  (float) (cntr.x - minx) * fac + xoff,
                  (float) (maxy - cntr.y) * fac + yoff,
                  (float) (cntr.x + axs.x * xg->tv[0][j] - minx) *
                   fac + xoff,
                  (float) (maxy -
                   (cntr.y - axs.y * xg->tv[1][j])) *
                   fac + yoff );
              }
            }
          }
        } else {
          if (xg->is_touring) {
            for (j=0; j<xg->ncols_used; j++) {
              if ((xg->tv[0][j]*xg->tv[0][j] +
                 xg->tv[1][j]*xg->tv[1][j]) > tol)
              {
                (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n",
                  xg->collab_tform2[j],
                  (float) fg.red / (float) 65535,
                  (float) fg.green / (float) 65535,
                  (float) fg.blue / (float) 65535,
                  (float) (cntr.x - minx) * fac + xoff,
                  (float) (maxy - cntr.y) * fac + yoff,
                  (float) (cntr.x + axs.x * xg->tv[0][j] - minx) *
                   fac + xoff,
                  (float) (maxy -
                   (cntr.y - axs.y * xg->tv[1][j])) *
                   fac + yoff );
              }
            }
          } else if (xg->is_corr_touring) {
            for (j=0; j<xg->ncols_used; j++) {
              if (xg->cu[0][j]*xg->cu[0][j] > tol) {
                (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n",
                  xg->collab_tform2[j],
                  (float) fg.red / (float) 65535,
                  (float) fg.green / (float) 65535,
                  (float) fg.blue / (float) 65535,
                  (float) (cntr.x - minx) * fac + xoff,
                  (float) (maxy - cntr.y) * fac + yoff,
                  (float) (cntr.x + axs.x * xg->cu[0][j] - minx) * fac + xoff,
                  (float) (maxy - cntr.y) * fac + yoff);

                /* Also add the tiny bars at the ends of the axes */
                (void) fprintf(psfile, "%f %f %f 0 %f %f %f %f ln\n",
                  (float) fg.red / (float) 65535,
                  (float) fg.green / (float) 65535,
                  (float) fg.blue / (float) 65535,
                  (float) (cntr.x + axs.x * xg->cu[0][j] - minx) * fac + xoff,
                  (float) (maxy - cntr.y - 2) * fac + yoff,
                  (float) (cntr.x + axs.x * xg->cu[0][j] - minx) * fac + xoff,
                  (float) (maxy - cntr.y + 2) * fac + yoff);
                  
              }
              if (xg->cu[1][j]*xg->cu[1][j] > tol) {
                (void) fprintf(psfile, "(%s) %f %f %f %f %f %f %f arbax\n",
                  xg->collab_tform2[j],
                  (float) fg.red / (float) 65535,
                  (float) fg.green / (float) 65535,
                  (float) fg.blue / (float) 65535,
                  (float) (cntr.x - minx) * fac + xoff,
                  (float) (maxy - cntr.y) * fac + yoff,
                  (float) (cntr.x - minx) * fac + xoff,
                  (float) (maxy -
                   (cntr.y - axs.y * xg->cu[1][j])) *
                   fac + yoff );

                /* Also add the tiny bars at the ends of the axes */
                (void) fprintf(psfile, "%f %f %f 0 %f %f %f %f ln\n",
                  (float) fg.red / (float) 65535,
                  (float) fg.green / (float) 65535,
                  (float) fg.blue / (float) 65535,
                  (float) (cntr.x + 2 - minx) * fac + xoff,
                  (float) (maxy -
                   (cntr.y - axs.y * xg->cu[1][j])) *
                   fac + yoff,
                  (float) (cntr.x - 2 - minx) * fac + xoff,
                  (float) (maxy -
                   (cntr.y - axs.y * xg->cu[1][j])) *
                   fac + yoff);
              }
            }
          }
        }
      }


    }

    if (xg->plot_the_points)
      XtFree((XtPointer) rgb_points);
    if (xg->connect_the_points && xg->nlines > 0)
      XtFree((XtPointer) rgb_lines);
  }
  else if (whichwin == CASEWIN)
  {
    print_cprof_win(xg, psfile, minx, maxy, fac, xoff, yoff, &fg,
      rgb_table, ncolors);
  }
  else if (whichwin == PROJPURSUITWIN)
  {
    print_pp_win(xg, psfile, minx, maxy, fac, xoff, yoff, &fg);
  }

  (void) fprintf(psfile, "%s\n", "showpage");
  (void) fprintf(psfile, "%s\n", "%%EOF");

  if (type == FILENAME)
    (void) fclose(psfile);
  else if (type == PRINTER)
    (void) pclose(psfile);

  XtFree((XtPointer) rgb_table);
}

/* ARGSUSED */
XtCallbackProc
print_to_file_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int outputtype = FILENAME;

  /* outfile is global in this file */
  XtVaGetValues(file_text, XtNstring, (String) &outfile, NULL);
  print_plot_window(w, outputtype, xg);
}

/* ARGSUSED */
XtCallbackProc
print_to_printer_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int outputtype = PRINTER;

  /* printing_cmd is global in this file */
  XtVaGetValues(print_text, XtNstring, (String) &printing_cmd, NULL);
  print_plot_window(w, outputtype, xg);
}

/* ARGSUSED */
XtCallbackProc
close_print_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  XtPopdown(ppopup);
}

/* ARGSUSED */
static XtCallbackProc
win_menu_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  for (whichwin=0; whichwin<NWINBTNS; whichwin++)
    if (win_menu_btn[whichwin] == w)
      break;

  XtVaSetValues(win_menu_cmd,
    XtNlabel, win_menu_nickname[whichwin],
    NULL);
}

/* ARGSUSED */
XtCallbackProc
print_panel_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  Dimension width, height;
  Position x, y;
  static int initd = 0;
  int k;

  if (!initd)
  {
    outfile = XtMalloc(32 * sizeof(char));
    bgname = XtMalloc(32 * sizeof(char));
    fgname = XtMalloc(32 * sizeof(char));
    whitename = XtMalloc(32 * sizeof(char));
    printing_cmd = XtMalloc(32 * sizeof(char));
    strcpy(outfile, "foo.ps");
    strcpy(bgname, "White");
    strcpy(fgname, "NavyBlue");
    strcpy(whitename, "NavyBlue");

    XtVaGetValues(w,
      XtNwidth, &width,
      XtNheight, &height, NULL);
    XtTranslateCoords(w,
      (Position) (width/2), (Position) (height/2), &x, &y);
    /*
     * Create the popup to solicit printing arguments.
    */
    ppopup = XtVaCreatePopupShell("Print",
      topLevelShellWidgetClass, XtParent(w),
      XtNx, (Position) x,
      XtNy, (Position) y,
      XtNinput, (Boolean) True,
      XtNallowShellResize, (Boolean) True,
      XtNtitle, (String) "Print plotting window",
      NULL);
    if (mono) set_mono(ppopup);

    /*
     * Create the paned widget so the 'Click here ...' can
     * be spread along the bottom.
    */
    pframe = XtVaCreateManagedWidget("Form",
      panedWidgetClass, ppopup,
      XtNorientation, (XtOrientation) XtorientVertical,
      NULL);

    /*
     * Create the form widget.
    */
    pform = XtVaCreateManagedWidget("Print",
      formWidgetClass, pframe,
      NULL);
    if (mono) set_mono(pform);

    file_label = XtVaCreateManagedWidget("Print",
      labelWidgetClass, pform,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNlabel, (String) "Filename:",
      XtNresize, False,
      NULL);
    if (mono) set_mono(file_label);

    file_text = XtVaCreateManagedWidget("PrintText",
      asciiTextWidgetClass, pform,
      XtNfromHoriz, (Widget) file_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) outfile,
      NULL);
    if (mono) set_mono(file_text);

    print_label = XtVaCreateManagedWidget("Print",
      labelWidgetClass, pform,
      XtNfromVert, (Widget) file_text,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNlabel, (String) "Postscript printer:",
      XtNresize, False,
      NULL);
    if (mono) set_mono(print_label);

    printing_cmd = appdata.defaultPrintCmd;
    print_text = XtVaCreateManagedWidget("PrintText",
      asciiTextWidgetClass, pform,
      XtNfromHoriz, (Widget) print_label,
      XtNfromVert, (Widget) file_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) printing_cmd,
      NULL);
    if (mono) set_mono(print_text);

    bg_label = XtVaCreateManagedWidget("Print",
      labelWidgetClass, pform,
      XtNfromVert, (Widget) print_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNlabel, (String) "Background color:",
      XtNresize, False,
      NULL);
    if (mono) set_mono(bg_label);

    bg_text = XtVaCreateManagedWidget("PrintText",
      asciiTextWidgetClass, pform,
      XtNfromHoriz, (Widget) bg_label,
      XtNfromVert, (Widget) print_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) bgname,
      NULL);
    if (mono) set_mono(bg_text);

    fg_label = XtVaCreateManagedWidget("Print",
      labelWidgetClass, pform,
      XtNfromVert, (Widget) bg_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNlabel, (String) "Foreground color:",
      XtNresize, False,
      NULL);
    if (mono) set_mono(fg_label);

    fg_text = XtVaCreateManagedWidget("PrintText",
      asciiTextWidgetClass, pform,
      XtNfromHoriz, (Widget) fg_label,
      XtNfromVert, (Widget) bg_text,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) fgname,
      NULL);
    if (mono) set_mono(fg_text);

    white_label = XtVaCreateManagedWidget("Print",
      labelWidgetClass, pform,
      XtNfromVert, (Widget) fg_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNlabel, (String) "Color for white glyphs:",
      XtNresize, False,
      NULL);
    if (mono) set_mono(white_label);

    white_text = XtVaCreateManagedWidget("PrintText",
      asciiTextWidgetClass, pform,
      XtNfromHoriz, (Widget) white_label,
      XtNfromVert, (Widget) fg_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) whitename,
      NULL);
    if (mono) set_mono(white_text);

    pointsize_label = XtVaCreateManagedWidget("Print",
      labelWidgetClass, pform,
      XtNfromVert, (Widget) white_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainLeft,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNlabel, (String) "Pointsize:",
      XtNresize, False,
      NULL);
    if (mono) set_mono(pointsize_label);

    pointsize_text = XtVaCreateManagedWidget("PrintText",
      asciiTextWidgetClass, pform,
      XtNfromHoriz, (Widget) pointsize_label,
      XtNfromVert, (Widget) white_label,
      XtNleft, (XtEdgeType) XtChainLeft,
      XtNright, (XtEdgeType) XtChainRight,
      XtNtop, (XtEdgeType) XtChainTop,
      XtNbottom, (XtEdgeType) XtChainTop,
      XtNresizable, (Boolean) True,
      XtNeditType, (int) XawtextEdit,
      XtNresize, (XawTextResizeMode) XawtextResizeWidth,
      XtNstring, (String) pointsize_str,
      NULL);
    if (mono) set_mono(pointsize_text);

    build_labelled_menu(&win_menu_box, &win_menu_lab, win_menu_str,
      &win_menu_cmd, &win_menu, win_menu_btn,
      win_menu_name, win_menu_nickname,
      NWINBTNS,
      PLOTWIN,
      pform, pointsize_label,
      XtorientHorizontal, appdata.font, "Print", xg);
    for (k=0; k<NWINBTNS; k++)
      XtAddCallback(win_menu_btn[k],  XtNcallback,
        (XtCallbackProc) win_menu_cback, (XtPointer) xg);

    file_cmd = (Widget) CreateCommand(xg, "Write to file",
      1, (Widget) NULL, (Widget) win_menu_box,
      pform, "Print");
    XtManageChild(file_cmd);
    XtAddCallback(file_cmd, XtNcallback,
      (XtCallbackProc) print_to_file_cback, (XtPointer) xg);

    print_cmd = (Widget) CreateCommand(xg, "Send to printer",
      1, (Widget) file_cmd, (Widget) win_menu_box,
      pform, "Print");
    XtManageChild(print_cmd);
    XtAddCallback(print_cmd, XtNcallback,
      (XtCallbackProc) print_to_printer_cback, (XtPointer) xg);

    close_cmd = XtVaCreateManagedWidget("Close",
      commandWidgetClass, pframe,
      XtNshowGrip, (Boolean) False,
      XtNskipAdjust, (Boolean) True,
      XtNlabel, (String) "Click here to dismiss",
      NULL);
    if (mono) set_mono(close_cmd);
    XtAddCallback(close_cmd, XtNcallback,
      (XtCallbackProc) close_print_cback, (XtPointer) xg);
  }

  XtPopup(ppopup, (XtGrabKind) XtGrabNone);
  XRaiseWindow(display, XtWindow(ppopup));

  if (!initd)
  {
    set_wm_protocols(ppopup);
    initd = 1;
  }
}
