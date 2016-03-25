/* xgobi_init.c */
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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include <X11/bitmaps/target>

static XtResource resources[] = {
/* Monochrome defaults */
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
  XtOffset(AppDataPtr, fg), XtRString, "Black"},
  {XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
  XtOffset(AppDataPtr, bg), XtRString, "White"},
  {XtNborderColor, XtCBorderColor, XtRPixel, sizeof(Pixel),
  XtOffset(AppDataPtr, border), XtRString, "Black"},

  {"axisColor", "AxisColor", XtRString, sizeof(String),
  XtOffset(AppDataPtr, axisColor), XtRString, "White"},

/* Font for labels and buttons */
  {"font", "Font", XtRFontStruct, sizeof(XFontStruct *),
  XtOffset(AppDataPtr, font), XtRString,
    "-*-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*"},
/* Font for PlotWindow */
  {"plotFont", "PlotFont", XtRFontStruct, sizeof(XFontStruct *),
  XtOffset(AppDataPtr, plotFont), XtRString,
    "-*-helvetica-medium-r-*-*-17-*-*-*-*-*-*-*"},
/* Help font */
  {"helpFont", "HelpFont", XtRFontStruct, sizeof (XFontStruct *),
  XtOffset(AppDataPtr, helpFont), XtRString,
    "-*-helvetica-medium-r-*-*-17-*-*-*-*-*-*-*"},
/* Brushing color defaults */
  {"brushColor0", "BrushColor0", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor0), XtRString, "Red"},
  {"brushColor1", "BrushColor1", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor1), XtRString, "Orange"},
  {"brushColor2", "BrushColor2", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor2), XtRString, "Yellow"},
  {"brushColor3", "BrushColor3", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor3), XtRString, "SkyBlue"},
  {"brushColor4", "BrushColor4", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor4), XtRString, "Blue"},
  {"brushColor5", "BrushColor5", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor5), XtRString, "PaleGreen"},
  {"brushColor6", "BrushColor6", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor6), XtRString, "Green"},
  {"brushColor7", "BrushColor7", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor7), XtRString, "Maroon"},
  {"brushColor8", "BrushColor8", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor8), XtRString, "Orchid"},
  {"brushColor9", "BrushColor9", XtRString, sizeof(String),
  XtOffset(AppDataPtr, brushColor9), XtRString, "Peru"},
/* Should axes, points, lines be drawn on startup? */
  {"showAxes", "ShowAxes", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, showAxes), XtRString, "True"},
  {"showPoints", "ShowPoints", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, showPoints), XtRString, "True"},
  {"showLines", "ShowLines", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, showLines), XtRString, "True"},
/* Linking options */
/*
  {"linkBrush", "LinkBrush", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, linkBrush), XtRString, "True"},
*/
  {"linkGlyphBrush", "LinkGlyphBrush", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, linkGlyphBrush), XtRString, "True"},
  {"linkColorBrush", "LinkColorBrush", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, linkColorBrush), XtRString, "True"},
  {"linkEraseBrush", "LinkEraseBrush", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, linkEraseBrush), XtRString, "True"},

  {"linkLineBrush", "LinkLineBrush", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, linkLineBrush), XtRString, "True"},
  {"linkIdentify", "LinkIdentify", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, linkIdentify), XtRString, "True"},
/* Brushing options */
  {"jumpBrush", "JumpBrush", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, jumpBrush), XtRString, "True"},
  {"reshapeBrush", "ReshapeBrush", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, reshapeBrush), XtRString, "False"},
  {"syncBrush", "SyncBrush", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, syncBrush), XtRString, "True"},
  {"carryVars", "CarryVars", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, carryVars), XtRString, "False"},
/* Scaling: start with plot square or not? */
  {"plotSquare", "PlotSquare", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, plotSquare), XtRString, "True"},

/* Starting glyph type and size */
  {"glyphType", "GlyphType", XtRInt, sizeof(int),
  XtOffset(AppDataPtr, glyphType), XtRString, "1"},
  {"glyphSize", "GlyphSize", XtRInt, sizeof(int),
  XtOffset(AppDataPtr, glyphSize), XtRString, "1"},   /* TINY */

/* Starting glyph and color */
  {"defaultGlyph", "DefaultGlyph", XtRInt, sizeof(int),
  XtOffset(AppDataPtr, defaultGlyph), XtRString, "26"},
  {"defaultColor", "DefaultColor", XtRString, sizeof(String),
  XtOffset(AppDataPtr, defaultColor), XtRString, "white"},
  {"axisColor", "AxisColor", XtRString, sizeof(String),
  XtOffset(AppDataPtr, axisColor), XtRString, "LightGray"},

/* pointer color */
  {"pointerColor", "PointerColor", XtRString, sizeof(String),
  XtOffset(AppDataPtr, pointerColor), XtRString, "Red"},
/* name of postscript printer */
  {"defaultPrintCmd", "DefaultPrintCmd", XtRString, sizeof(String),
  XtOffset(AppDataPtr, defaultPrintCmd), XtRString, "/usr/ucb/lpr -Pps1"},
/* clone information; users shouldn't set these in resource files */
  {"isCloned", "IsCloned", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, isCloned), XtRString, "False"},
  {"clonePID", "ClonePID", XtRInt, sizeof(int),
  XtOffset(AppDataPtr, clonePID), XtRString, "0"},
  {"cloneTime", "CloneTime", XtRInt, sizeof(int),
  XtOffset(AppDataPtr, cloneTime), XtRString, "0"},
  {"cloneType", "CloneType", XtRInt, sizeof(int),
  XtOffset(AppDataPtr, cloneType), XtRString, "0"},
  {"cloneName", "CloneName", XtRString, sizeof(String),
  XtOffset(AppDataPtr, cloneName), XtRString, "XGobi"},
  {"deleteCloneData", "DeleteCloneData", XtRBoolean, sizeof(Boolean),
  XtOffset(AppDataPtr, deleteCloneData), XtRString, "False"},
};

Boolean
RunWorkProc(xgobidata *xg)
/*
 * If there is a work proc attached to this xgobi, run it.
*/
{
  Boolean run_work_proc = False;

  if (xg->is_identify)
  {
    id_proc(xg);
    run_work_proc = True;
  }
  else if (xg->is_line_editing)
  {
    line_edit_proc(xg);
    run_work_proc = True;
  }
  else if (xg->is_point_moving)
  {
    move_points_proc(xg);
    run_work_proc = True;
  }
  else if (xg->is_scaling)
  {
    if (xg->run_scale_proc)
      scale_proc(xg);
    if (xg->run_shift_proc)
      shift_proc(xg);
    scaling_proc(xg);
    run_work_proc = True;
  }
  else if (xg->run_tour_proc)
  {
    tour_proc(xg);
    run_work_proc = True;
  }
  else if (xg->run_corr_proc)
  {
    corr_proc(xg);
    run_work_proc = True;
  }
  else if (xg->run_spin_oblique_proc)
  {
    ob_rotate_proc(xg);
    run_work_proc = True;
  }
  else if (xg->run_spin_axis_proc)
  {
    spin_proc(xg);
    run_work_proc = True;
  }
  else if (xg->run_rock_proc)
  {
    rock_proc(xg);
    run_work_proc = True;
  }
  else if (xg->run_interp_proc)
  {
    interp_proc(xg);
    run_work_proc = True;
  }
  else if (xg->is_xyplotting)
  {
    if (xg->is_xy_cycle)
    {
      xy_cycle_proc(xg);
      run_work_proc = True;
    }
  }
  else if (xg->is_plot1d_cycle)
  {
    plot1d_cycle_proc(xg);
    run_work_proc = True;
  }

  return(run_work_proc);
}

int
find_mono()
{
  Visual *vis;
  int scrn, mono;
  /*
   * Determine whether this is a monochrome display -- treat
   * grayscale machines as monochrome for now.
  */
  scrn = DefaultScreen(display);
  depth = DefaultDepth(display, scrn);
  vis = DefaultVisual(display, scrn);
  if (depth == 1)
    mono = 1;
  else if (vis->class == GrayScale || vis->class == StaticGray)
    mono = 1;
  else
    mono = 0;

  return(mono);
}

void
set_wm_protocols(Widget w)
/*
 * to make xgobi work with the motif title bar menu
*/
{
  Atom wm_delete_window;

  XtOverrideTranslations(w,
    XtParseTranslationTable("<Message>WM_PROTOCOLS: wm_quit()"));

  wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  (void) XSetWMProtocols(display, XtWindow(w),
     &wm_delete_window, 1);
}

static void
init_atoms() {

  /*
   * Initialize Atoms used to pass xg->rows_in_plot
  */
  XG_ROWSINPLOT_ANNC = XInternAtom(display, "Announce Rows in Plot", 0);
  XG_ROWSINPLOT_ANNC_TYPE = XInternAtom(display,
    "Announce Rows in Plot Type", 0);
  XG_ROWSINPLOT = XInternAtom(display, "Rows in Plot", 0);
  XG_ROWSINPLOT_TYPE = XInternAtom(display, "Rows in Plot Type", 0);

  /*
   * Initialize Atoms used to pass xg->erased
  */
  XG_ERASE_ANNC = XInternAtom(display, "Announce Erase", 0);
  XG_ERASE_ANNC_TYPE = XInternAtom(display, "Announce Erase Type", 0);
  XG_ERASE = XInternAtom(display, "Erase", 0);
  XG_ERASE_TYPE = XInternAtom(display, "Erase Type", 0);

  /*
   * Initialize Atoms used in linked glyph and color brushing;
   * points only.
  */
  XG_NEWPAINT_ANNC = XInternAtom(display, "Announce New Paint", 0);
  XG_NEWPAINT_ANNC_TYPE = XInternAtom(display, "Announce New Paint Type", 0);
  XG_NEWPAINT = XInternAtom(display, "New Paint", 0);
  XG_NEWPAINT_TYPE = XInternAtom(display, "New Paint Type", 0);

  /*
   * Initialize Atoms used in linked line brushing.
  */
  XG_NEWLINEPAINT_ANNC = XInternAtom(display, "Announce New Line Paint", 0);
  XG_NEWLINEPAINT_ANNC_TYPE = XInternAtom(display,
    "Announce New Line Paint Type", 0);
  XG_NEWLINEPAINT = XInternAtom(display, "New Line Paint", 0);
  XG_NEWLINEPAINT_TYPE = XInternAtom(display, "New Line Paint Type", 0);

  /*
   * Initialize Atoms used in linked identification.
  */
  XG_IDS_ANNC = XInternAtom(display, "Announce NearestPoint", 0);
  XG_IDS_ANNC_TYPE = XInternAtom(display, "Announce NearestPoint Type", 0);
  XG_IDS = XInternAtom(display, "NearestPoint", 0);
  XG_IDS_TYPE = XInternAtom(display, "NearestPoint Type", 0);

  /*
   * Initialize Atoms used in linked touring.
  */
  XG_NEWTOUR_ANNC = XInternAtom(display, "Announce New Tour Coefs", 0);
  XG_NEWTOUR_ANNC_TYPE = XInternAtom(display,
    "Announce New Tour Coefs Type", 0);
  XG_NEWTOUR = XInternAtom(display, "New Tour Coefs", 0);
  XG_NEWTOUR_TYPE = XInternAtom(display, "New Tour Coefs Type", 0);
}

static Boolean
get_data_from_parent(char *data_in, float **datap,
  Boolean missingpflag, short **missingp,
  Boolean mv_is_missing_values_xgobi, int mv_nmissing, 
  int nlines, connect_lines *connecting_lines,
  int nr, char **rowp, int nc, char **colp,
  Boolean firsttime, xgobidata *xg)
{

/*
 * Parent can override previous definitions made in
 *  set_title_and_icon()
 * These set on command line:
 *  nrows, nrows_in_plot, ncols, nlines, raw_data[]
 *  collab[], rowlab[]
 *  connecting_lines[]
 * Where can these come from? Parent must allocate?
 *  collab_tform1[]
 *  collab_tform2[]
 *  nlinkable
 *  vgroup_ids[]
 *  erased[]
 *  delete_erased_pts
 * Parent could handle read_extra_resources() in some other way.
*/
  int ok = true;
  int i, j;
  int ncols_prev;

  if (!firsttime)
    ncols_prev = xg->ncols;

/*
 * Define nrows, ncols
*/

  xg->nrows = nr;
  if (xg->nrows_in_plot < 2 || xg->nrows_in_plot > nr)
    xg->nrows_in_plot = nr;
  if (data_in != NULL)
    xg->nrows_in_plot = nr;

  xg->ncols = nc;
  xg->ncols_used = nc-1;

  if (xg->nrows < 1 || xg->ncols_used < 1 || datap == NULL) {
    (void) fprintf(stderr, "problem with input data\n");
    ok = false;
    return(ok);
  }

/*
 * Allocate and copy raw data.  (why not just pass the pointer?)
*/

  xg->raw_data = (float **) XtMalloc ((Cardinal) xg->nrows *
    sizeof (float *));
  for (i = 0; i < xg->nrows; i++)
    xg->raw_data[i] = (float *) XtMalloc ((Cardinal) xg->ncols *
    sizeof (float));

  for (i = 0; i < xg->nrows; i++)
    for (j = 0; j < xg->ncols_used; j++)
      xg->raw_data[i][j] = datap[i][j];

/*
 * Deal with missing data
*/

  if (missingpflag) {
    if (missingp == NULL) {
      (void) fprintf(stderr,
        "problem with missing values arrays in input data\n");
      ok = false;
      return(ok);
    }
    xg->is_missing = (short **) XtMalloc ((Cardinal) xg->nrows *
      sizeof (short *));
    for (i = 0; i < xg->nrows; i++)
      xg->is_missing[i] = (short *) XtMalloc ((Cardinal) xg->ncols *
      sizeof (short));

    for (i = 0; i < xg->nrows; i++) /* copy missings from missingp */
      for (j = 0; j < xg->ncols_used; j++)
        xg->is_missing[i][j] = missingp[i][j];

    xg->missing_values_present = True;
    xg->is_missing_values_xgobi = False;
    xg->nmissing = mv_nmissing;
  }
  else {
    xg->missing_values_present = False;
    xg->is_missing_values_xgobi = mv_is_missing_values_xgobi;
  }

/*
 * Populate extra column
*/
  fill_extra_column(xg);

/*
 * For now, force each column to be its own group.
 * The group ids have to be sorted, and the first one
 * has to be zero.  (numvargroups is awfully stupid)
*/
  read_vgroups(xg->datafname, True, xg);

/* For row groups, I'm going to try just setting the number */
  xg->nrgroups = 0;

/*
 * Assume the links will always be supplied.
*/
  xg->nlines = nlines;
  xg->connecting_lines = connecting_lines ;

/*
 * Allocate and populate row and column labels
*/

/* I probably just broke this -- dfs */
  alloc_rowlabels(xg);
  if (rowp)                         /* Copy labels */
    for (i = 0; i< xg->nrows; i++)
       strcpy(xg->rowlab[i], rowp[i]);
  else                              /* Use default row labels */
    for (i=0; i<xg->nrows; i++)
      (void) sprintf(xg->rowlab[i], "%d", i+1);

  alloc_collabels(xg);
  if (colp)
    for (i = 0; i<xg->ncols_used; i++) {
       strcpy(xg->collab[i], colp[i]);
      (void) sprintf(xg->collab_short[i], "V%d", i+1);  /* for now */
    }
  else
    for (i=0; i<xg->ncols; i++) {
      (void) sprintf(xg->collab[i], "Var %d", i+1);
      (void) sprintf(xg->collab_short[i], "V%d", i+1);
    }

  /*
   * Set the label for the last variable
  */
  strcpy(xg->collab[xg->ncols-1], "group");

  /*
   * Copy collab into the collab_tform arrays
  */
  for (j=0; j<xg->ncols; j++) {
    (void) strcpy(xg->collab_tform1[j], xg->collab[j]);
    (void) strcpy(xg->collab_tform2[j], xg->collab[j]);
  }

  if (! firsttime)
    destroy_varsel_widgets(ncols_prev, xg);

  xg->nlinkable = xg->nrows;

  return ok;
}


int
make_xgobi(Boolean datapflag, char *data_in, float **datap, char *xgobi_title,
 Boolean missingpflag, short **missingp, Boolean mv_is_missing_values_xgobi,
 int mv_nmissing,
 int nr, char **rowp, int nc, char **colp,
 int nlines, connect_lines *connecting_lines,
 xgobidata *xg, Widget parent, Boolean plotp)

/*
  Boolean datapflag  * calling program supplies pointer to data? *
  char *data_in      * if datapflag = F; name of data file, or stdin *
  float **datap      * if datapflag = T; data pointer *
  char *xgobi_title  * title to use in window manager (optional) *

  Boolean missingpflag  * if datapflag = T; are there missings? *
  short **missingp      * if missingpflag = T; pointer to missings array *
  Boolean mv_is_missing_values_xgobi  * if missingpflag = T *
  int mv_nmissing                     * if missingpflag = T *

  int nr, nc          * if datapflag = T; number of rows and cols *
  char **rowp, **colp * if datapflag = T; pointers to row and col labels *

  int nlines                        * if datapflag = T *
  connect_lines *connecting_lines   * if datapflag = T; pointer to lines *

  xgobidata *xg;
  Widget parent;
*/
{
  int i;
  Colormap cmap;
  XColor cfore, cback, exact;
  static int firsttime = 1;
  extern void init_random_seed(void);

  xg->is_realized = False;

  if (firsttime)
  {
    /*
     * Create the shell if it hasn't already been created.
    */
    if (parent)
      xg->shell = XtVaCreatePopupShell("XGobi",
         topLevelShellWidgetClass, parent,
         NULL);
  
    init_random_seed();  /* before any data values are read */
    init_atoms();

    /*
     * Initialize the strings in the save_types[] array.
    */
    SAVE_SPIN_COEFS = "spin coefs";
    SAVE_SPIN_RMAT = "save spin matrix";
    READ_SPIN_RMAT = "read spin matrix";
    SAVE_TOUR_COEFS = "tour coefs";
    SAVE_TOUR_HIST = "tour savehist";
    READ_POINT_COLORS_GLYPHS = "read brush vec";
    READ_TOUR_HIST = "tour readhist";
    OPEN_BITMAP_FILE = "open bitmap file";
  }

  /*
   * xg->datafilename is the full file name including path and suffixes
   * xg->datafname is the name of the data file without any suffixes
  */
  if (data_in != NULL) {
    strcpy(xg->datafilename, data_in);
    (void) strip_suffixes(xg);
  }
  else {
    xg->datafilename[0] = '\0';
    xg->datafname[0] = '\0';
  }

  /*
   * Read data-specific resource file and merge all resources.
  */
  (void) read_extra_resources(xg->datafname);

  if (xgobi_title != NULL) {
    strcpy(xg->title, xgobi_title);
    set_title_and_icon(xg->title, xg);
  } else {
    if (xg->isCloned)
      set_title_and_icon(xg->clone_Name, xg);
    else
      set_title_and_icon(xg->datafname, xg);
  }

  /*
   * Input data:
   *  If data is being supplied by by a calling program,
   *  set up xg->nrows and xg->ncols and grab the data:
   *  either copy it into xg->raw_data or assign it.
   *  Else, read data from ascii files.
  */
  if (parent && datapflag)
  {
    get_data_from_parent(data_in, datap,
      missingpflag, missingp, mv_nmissing, mv_nmissing,
      nlines, connecting_lines,
      nr, rowp, nc, colp, firsttime, xg);

    read_erase(xg->datafname, True, xg);
  }
  else
  {
    /* Initialize missing values variables */
    xg->nmissing = 0;
    xg->missing_values_present = False;
    xg->is_missing_values_xgobi = False;
    xg->is_missing = (short **) NULL;

    /*
     * Read input data files
    */

    if (xg->data_mode == Sprocess)
      (void) Sread_array(xg); /* it doesn't matter which file name */
    else if (xg->data_mode == ascii || xg->data_mode == binary) /* if not S */
    {
      (void) read_array(xg);
      /* This should take care of the .bin case as well ... */
      if (!xg->is_missing_values_xgobi)
        read_missing_values(xg);
      fill_extra_column(xg);
    }

    /*
     * nrows_in_plot can be preset in xgobi.c; make sure it's sensible.
    */
    if (xg->nrows_in_plot < 2 || xg->nrows_in_plot > xg->nrows)
      xg->nrows_in_plot = xg->nrows;

    (void) read_collabels(xg->datafname, True, xg);
    (void) read_rowlabels(xg->datafname, True, xg);
  
    if (xg->is_missing_values_xgobi || xg->is_scatmat)
      init_single_vgroup(xg);  /* force all variable into one group */
    else
      read_vgroups(xg->datafname, True, xg);

    (void) read_nlinkable(xg->datafname, True, xg);
    (void) read_erase(xg->datafname, True, xg);
    read_rgroups(xg->datafname, True, xg);

    if (!xg->is_scatmat) {
      (void) read_connecting_lines(xg->datafname, True, xg);
      set_lgroups(True, xg);
    }
  }

  xg->last_forward = (int *) XtMalloc((Cardinal) xg->nrows * sizeof(int));
  for (i=0; i<xg->nrows; i++)
    xg->last_forward[i] = -1;

  xg->xgobi_is_up = True;

  /*
   * Get the application foreground and background colors, as well
   * as the brushing colors.
   * This needs to be done after read_extra_resources()
  */
  if (parent)
    XtGetApplicationResources(parent, (XtPointer) &appdata, resources,
      XtNumber(resources), (ArgList) NULL, 0);
  else
    XtGetApplicationResources(xg->shell, (XtPointer) &appdata, resources,
        XtNumber(resources), (ArgList) NULL, 0);

  /*
   * Force white on black -- this is the easiest way
   * to get decent pictures for publications.
  */
  if (mono) {
    appdata.fg = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
    appdata.bg = WhitePixelOfScreen(DefaultScreenOfDisplay(display));
    appdata.border = appdata.fg;
    xg->axisColor = appdata.fg;
  }

  if (!mono)
    init_brush_colors(&appdata);

  /*
   * Initialize variables: the sequence of these is important.
  */
  alloc_pipeline_arrays(xg);
  alloc_plot_arrays(xg);

  /*
   * Set up the widgets and add the callbacks.
  */

  init_help(xg);
  make_widgets(xg);

  init_options(xg);

  build_varlist(xg);
  build_caselist(xg);

  make_arrows(xg);

  init_plot1d_vars(xg);
  make_plot1d(xg);

  init_xyplot_vars(xg);
  make_xyplot(xg);

  alloc_rotate_arrays(xg);
  init_rotate_vars(xg);
  make_rotate(xg);

  alloc_brush_arrays(xg);
  init_brush_vars(xg);
  make_brush(xg);

  init_scale_vars(xg);
  make_scaling(xg);

  init_identify_vars(xg);
  make_identify(xg);

  init_parcoords(xg);

  alloc_transform_tp(xg);
  reset_tform(xg);  /* initialize */

  if (xg->ncols_used > 2)
  {
    alloc_tour(xg);
    init_tour(xg, 1); /* Some lines here have to precede sphered_data_fn() */
  }
  make_tour(xg);

  if (xg->ncols_used > 2)
  {
    alloc_corr(xg);
    init_corr(xg);
  }
  make_corr(xg);

  alloc_line_edit_arrays(xg);
  init_line_edit_vars(xg);
  make_line_editor(xg);

  init_point_moving(xg);
  make_move_points(xg);

  alloc_smooth_arrays(xg);
  init_smooth_vars(xg);

  if (xg->nrows_in_plot < xg->nrows)
    sample_xgobi(xg->nrows_in_plot, xg);

  /* This must be made late, after at least one mouse label */
  make_plot_window(xg);
  /* Let's try the mouse labels below the workspace */
  make_plotwindow_mouse_labels(xg);

/*
 * alloc_transform_types(xg);
 * init_transform_types(xg);
*/
  make_varpanel(xg);

  /*
   * Put a NULL at the end of the help tables.
  */
  add_pb_help(&xg->nhelpids.pb, (Widget) NULL, "");
  add_menupb_help(&xg->nhelpids.menupb, (Widget) NULL, "");
  add_sbar_help(&xg->nhelpids.sbar, (Widget) NULL, "");

  if (firsttime)
    XtRealizeWidget(xg->shell);

  XtInstallAllAccelerators(xg->form0, xg->form0);
  /*XtInstallAllAccelerators(xg->box2, xg->box2);*/

  if (firsttime)
  {
    /*
     * Get the colors of the plot window and the tour_pp plotting window.
     * These are needed for setting the GC colors.
    */
    XtVaGetValues(xg->pp_plot_wksp,
      XtNforeground, (Pixel) &tour_pp_colors.fg,
      XtNbackground, (Pixel) &tour_pp_colors.bg,
      XtNborderColor, (Pixel) &tour_pp_colors.border,
      NULL);
    init_tour_pp_GCs(xg);

    XtVaGetValues(xg->workspace,
      XtNforeground, (Pixel) &plotcolors.fg,
      XtNbackground, (Pixel) &plotcolors.bg,
      XtNborderColor, (Pixel) &plotcolors.border,
      NULL);
    init_GCs(xg);

    /*
     * Define cursors, set their colors.
    */
    default_cursor = XCreateFontCursor(display, XC_center_ptr);
    scale_cursor = XCreateFontCursor(display, XC_fleur);
    spin_cursor = XCreateFontCursor(display, XC_hand2);
    /* used in identify and projection pursuit */
    crosshair_cursor = XCreateFontCursor(display, XC_tcross);

    if (!mono) {
      cmap = DefaultColormap(display, DefaultScreen(display));
      if (XParseColor(display, cmap, appdata.pointerColor, &exact))
        cfore.pixel =
          XAllocColor(display, cmap, &exact) ? exact.pixel : plotcolors.fg;

      /* Work out the color for axes and gridlines */
      if (XParseColor(display, cmap, appdata.axisColor, &exact))
        if (XAllocColor(display, cmap, &exact))
          xg->axisColor = exact.pixel;

      /*
       * Test to see whether we've made the pointer invisible,
       * and if so, assume that to be a mistake.  Make the
       * pointer the same color as the plotting foreground color.
      */
      if (cfore.pixel == plotcolors.bg)
        cfore.pixel = plotcolors.fg;

      cback.pixel = plotcolors.bg ;
      XQueryColor(display, cmap, &cfore);
      XQueryColor(display, cmap, &cback);
      XRecolorCursor(display, default_cursor, &cfore, &cback);
      XRecolorCursor(display, spin_cursor, &cfore, &cback);
      XRecolorCursor(display, scale_cursor, &cfore, &cback);
      XRecolorCursor(display, crosshair_cursor, &cfore, &cback);
    }

    menu_mark = XCreateBitmapFromData(display,
      RootWindowOfScreen(XtScreen(xg->shell)),
      target_bits, target_width, target_height);
    /*
     * Once the menu_mark has been created,
     * make sure a couple of menus are properly marked.
    */
    set_display_menu_marks(xg);
    set_br_linkopt_menu_marks(xg);
    set_br_opt_menu_marks(xg);
    set_id_linkopt_menu_marks(xg);

    reinit_brush_colors(xg);
  }

  /*
   * Create the background pixmap and clear it.
  */
  XtVaGetValues(xg->workspace,
    XtNwidth, &xg->plotsize.width,
    XtNheight, &xg->plotsize.height, NULL);

  xg->plot_window = XtWindow(xg->workspace);
  xg->pixmap0 = XCreatePixmap(display, xg->plot_window,
    xg->plotsize.width,
    xg->plotsize.height,
    depth);
  XFillRectangle(display, xg->pixmap0, clear_GC,
    0, 0, xg->plotsize.width, xg->plotsize.height);

/*
 * These are the vars which depend on the plot window having
 * been realized: size, color.
*/
  init_plotwindow_vars(xg, 1);

  copy_raw_to_tform(xg);
  if (xg->ncols_used > 2)
    compute_vc_matrix(xg);
  update_lims(xg);
  update_world(xg);

/*
 * Reinitialize variable selection panel.
*/
  if (! firsttime) {
    set_varsel_label(xg);
    XSync(display, False);
    reset_var_panel(xg);
  }

  /*
   * The line color arrays are set up in allocate_line_edit_arrays(),
   * after which we can try to read in the line colors file.
  */
  if (xg->nlines > 0) {
    if (read_line_colors(xg->datafname, True, True, xg) == 0) {
      init_line_colors(xg);
    }
  }

  /*
   * Mark the default menu selections.
  */
  init_brush_menus();
  init_tour_interp_menu();
  init_tour_pp_menu();

  /* Draw the current glyph in the glyph workspace on the brush panel */
  draw_current_glyph(xg);

  if (parent)
  {
    XtPopup(xg->shell, XtGrabNone);
  }
  XDefineCursor(display, XtWindow(xg->form0), default_cursor);
  xg->is_realized = True;
  xg->is_iconified = False;

/*
 * This needs to be done after the brush menus are initialized
 * and the brushing arrays are allocated.
*/
  (void) read_point_colors(xg->datafname, True, True, xg);
  (void) read_point_glyphs(xg->datafname, True, True, xg);

  if (xg->isCloned)
  {
    char command[200];

    xg->xy_vars.x = 0; /* do not swap x & y axis */
    xg->xy_vars.y = 1;

    strcpy (xg->datafilename, xg->clone_Name);

    if (xg->clone_Type == CDF1) {
      xg->xy_vars.x = 1; /* swap x & y axis */
      xg->xy_vars.y = 0;
    }

    /* disable options not required for cdf mode */
    if ((xg->clone_Type == CDF1) || (xg->clone_Type == CDFm)) {
      set_Edit_Lines_cmd (xg, False);
      set_brush_menu_cmd (False);
    }

    /* Delete for cloning but not for copying */
    if (xg->delete_clone_data) {
      sprintf (command, "rm /tmp/%s_%d_%d.* &", xg->datarootname,
        xg->clone_PID, xg->clone_Time);
      system (command);
    }
  }

  /*
   * Just before opening the window, when we know the plot mode etc,
   * jitter the 0s and 1s for the missing values xgobi.
  */
  if (xg->is_missing_values_xgobi) {
    init_jitfac(xg);
    jitter_data(xg);
    update_world(xg);  /* So the jittering takes effect */
  }

  /*
   * Scale data to planar and then to screen coordinates.
  */
  world_to_plane(xg);
  plane_to_screen(xg);

  /*
   * Initialize axes and ticks.
  */
  alloc_axis_arrays(xg);
  init_axes(xg, True);
  init_ticks(&xg->xy_vars, xg);

  /*
   * First scatter plot.
  */
  if (xg->delete_erased_pts) {
    for (i=0; i<xg->nrows; i++) xg->excluded[i] = xg->erased[i];
    update_nrgroups_in_plot(xg);
    reset_rows_in_plot(xg, True);
  }
  if (plotp)
    plot_once(xg);
  refresh_vlab(xg->xy_vars.x, xg);
  refresh_vlab(xg->xy_vars.y, xg);

  if (firsttime)
    set_wm_protocols(xg->shell);

  firsttime = 0;
  return(1);
}


void
GetApplResources (xgobidata *xg)
{
    XtGetApplicationResources(xg->shell, (XtPointer) &appdata, resources,
        XtNumber(resources), (ArgList) NULL, 0);
}

