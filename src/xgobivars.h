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

#ifndef XGOBIINTERN
#define XGOBI_ extern
#else
#define XGOBI_
#endif

XGOBI_ int mono;
XGOBI_ unsigned int depth;
XGOBI_ Display *display;
XGOBI_ char *save_types[13];
XGOBI_ XtAppContext app_con;
XGOBI_ int update_required;

/* Resources */
XGOBI_ AppData appdata;
XGOBI_ WidgetColors plotcolors, tour_pp_colors;

/* Graphics contexts */
XGOBI_ GC copy_GC, clear_GC, varpanel_copy_GC, varpanel_xor_GC;

/* Atoms to represent nrows_in_plot and rows_in_plot */
XGOBI_ Atom XG_ROWSINPLOT_ANNC;
XGOBI_ Atom XG_ROWSINPLOT_ANNC_TYPE;
XGOBI_ Atom XG_ROWSINPLOT;
XGOBI_ Atom XG_ROWSINPLOT_TYPE;

/* Atoms used in linked brushing */
XGOBI_ Atom XG_NEWPAINT_ANNC;
XGOBI_ Atom XG_NEWPAINT_ANNC_TYPE;
XGOBI_ Atom XG_NEWPAINT;
XGOBI_ Atom XG_NEWPAINT_TYPE;

/* Atoms used for erasing */
XGOBI_ Atom XG_ERASE_ANNC;
XGOBI_ Atom XG_ERASE_ANNC_TYPE;
XGOBI_ Atom XG_ERASE;
XGOBI_ Atom XG_ERASE_TYPE;

/* Atoms used in linked line brushing */
XGOBI_ Atom XG_NEWLINEPAINT_ANNC;
XGOBI_ Atom XG_NEWLINEPAINT_ANNC_TYPE;
XGOBI_ Atom XG_NEWLINEPAINT;
XGOBI_ Atom XG_NEWLINEPAINT_TYPE;

/* Atoms used in linked identification */
XGOBI_ Atom XG_IDS_ANNC;
XGOBI_ Atom XG_IDS_ANNC_TYPE;
XGOBI_ Atom XG_IDS;
XGOBI_ Atom XG_IDS_TYPE;

/* Atoms used in linked touring */
XGOBI_ Atom XG_NEWTOUR_ANNC;
XGOBI_ Atom XG_NEWTOUR_ANNC_TYPE;
XGOBI_ Atom XG_NEWTOUR;
XGOBI_ Atom XG_NEWTOUR_TYPE;

/* color brushing */
XGOBI_ Cardinal ncolors;
XGOBI_ char *color_names[NCOLORS+2];
XGOBI_ unsigned long color_nums[NCOLORS+2];

/* Cursors and other pixmaps */
XGOBI_ Cursor default_cursor, spin_cursor, scale_cursor;
XGOBI_ Cursor crosshair_cursor, xy_cursor;
XGOBI_ Pixmap menu_mark;

/* Arrows */
XGOBI_ Pixmap leftarr, rightarr, uparr, downarr, plus, minus, lrarr;
XGOBI_ Pixmap toggle_on, toggle_off;

/*
 * kludge to get around X bug:  Can't draw
 * more than 65535 circles according to the source code in
 * XDrArcs.c.  But 65535 fails, too, and 32768.
 * Used in plot_once.c and in parcoords.c
*/
#define MAXARCS 16384


/* link to S */
/*
* XGOBI_ int Sprocess, Snetwork;
*/
XGOBI_ char Spath0[100];
