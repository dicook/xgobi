/* UNIX includes. */
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

/* X includes. */
#include "xincludes.h"
#include <X11/keysym.h>

/* XGobi includes. */
# define XGOBIINTERN
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
xgobidata xgobi;
/* Diagnostic xgobi */
xgobidata xgobi_diag;
#include "xgobitop.h"

/* XGvis includes. */
#define XGVISINTERN
#include "xgvis.h"

/* Globals. */
lims *olims, *olims0;  /* min and max values to prevent rescaling */
connect_lines *xg_lines;    /* Lines. */
int xg_nlines = 0;    /* Lines. */

static XtResource panel_resources[] = {
  {"font", "Font", XtRFontStruct, sizeof(XFontStruct *),
  XtOffset(PanelDataPtr, Font), XtRString, "*"},
/* Is it possible to set the initial embedding dimension this way? */
  {"mdsDimension", "MdsDimension", XtRInt, sizeof(int),
  XtOffset(PanelDataPtr, mdsDimension), XtRString, "3"},
};

String
xgvis_fallback_resources[] = {
  "*font:          -*-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*",

  "XGvis*Scrollbar.Translations: #override \\n\
    <Btn1Down>:   StartScroll(Continuous) MoveThumb() NotifyThumb() \\n\
    <Btn1Motion>: MoveThumb() NotifyThumb() \\n\
    <Btn3Down>:   HelpSelect()",

  /*
   * It's important that this be an XGVToggle, because if it's
   * just a Toggle, it collides with the XGobi resources; I don't
   * honestly know why.  dfs
  */
  "XGvis*XGVToggle.Translations: #override \\n\
    <EnterWindow>:         highlight(Always) \\n\
    <LeaveWindow>:         unhighlight() \\n\
    <Btn1Down>, <Btn1Up>:  set()notify() \\n\
    <Btn3Down>:            HelpSelect()",

  "*background: Moccasin",
  "*foreground: NavyBlue",
  "*pointerColor: White",
  "*MainPanel.background: DarkKhaki",
  "*PlotWindow.background: blue4",
  "*PlotWindow.foreground: white",
  "*Command.background: SandyBrown",
  "*Command.borderColor: NavyBlue",
  "*Command.foreground: NavyBlue",
  "*Toggle.background: SandyBrown",
  "*Toggle.borderColor: NavyBlue",
  "*Toggle.foreground: NavyBlue",
  "*MenuButton.background: SandyBrown",
  "*MenuButton.borderColor: NavyBlue",
  "*MenuButton.foreground: NavyBlue",
  /* Menu Icon */
  "*MenuButton.leftBitmap: menu12",
  "*Scrollbar.background: SandyBrown",
  "*Scrollbar.borderColor: NavyBlue",
  "*Scrollbar.foreground: NavyBlue",
  NULL
};

/* resource database */
XrmDatabase dispdb;

/* ARGSUSED */
void  xfer_brushinfo (xgobidata *xg)
{
}

extern void mds_once(Boolean);
extern void initialize_data(int argc, char *argv[]);
extern void reset_data(void);
extern double distance(double *, double *, int, double);
extern double set_distance_factor(void);
extern void init_dissim(void);
extern void draw_stress(void);
extern void make_xgvis_widgets(void);
extern void set_vgroups(void);

void
copy_pos_to_raw(xgobidata *xg)
{
  int i, j;

  for (i = 0; i < pos.nrows; i++) {
    for (j = 0; j < pos.ncols-1; j++)
      xg->raw_data[i][j] = (float) pos.data[i][j];
  }
}

void
copy_diag_to_raw(xgobidata *xg)
{
  int i, j;
  
  for (i = 0; i < diagnostics.nrows; i++) {
    for (j = 0; j < diagnostics.ncols; j++)
      xg->raw_data[i][j] = (float) diagnostics.data[i][j];
  }
}

/*
An X program always has an event loop running, looking for
input:  usually user-generated mouse motions and so forth.  The
XGobi event loop is called XGobiMainLoop() and it's in
xgobitop.h.

A work proc is a routine that runs once whenever the event loop
finds no events, a sort of run-while-idle routine.  In XGobi,
continuous processes such as rotation are handled using work
procs:  for example, if no user input is found, spin_once().
Usually, an X programmer doesn't write her own RunWorkProcs()
routine, but we found it necessary to do so for XGobi.

In XGvis, we added a work proc to the set used in XGobi to
handle the multi-dimensional scaling routine, and here's the
RunWorkProcs() routine that we use here.  You are very likely
to need to edit this for your routine.
*/

void
update_plot(xgobidata *xg)
{
  /* copy pos.data to xg->raw_data */
  copy_pos_to_raw(xg);
  copy_raw_to_tform(xg);
  /* commented out July 17, 2001, in an effort to make point motion smoother */
  /*update_lims(xg);*/
  update_world(xg);
  world_to_plane(xg);
  plane_to_screen(xg);

  init_tickdelta(xg);
  if (xg->is_xyplotting)
    init_ticks(&xg->xy_vars, xg);
  else if (xg->is_plotting1d)
    init_ticks(&xg->plot1d_vars, xg);
}

void
update_diagnostics_plot(xgobidata *xg)
{
  copy_diag_to_raw(xg);
  copy_raw_to_tform(xg);
  update_lims(xg);
  update_world(xg);
  world_to_plane(xg);
  plane_to_screen(xg);

  init_tickdelta(xg);
  if (xg->is_xyplotting)
    init_ticks(&xg->xy_vars, xg);
  else if (xg->is_plotting1d)
    init_ticks(&xg->plot1d_vars, xg);
}

/* ARGSUSED */
Boolean
RunWorkProcs(void *dummyArgc)
{
  Boolean keepgoing = False;

  if (xgobi.is_realized)
  {
    if (xgv_is_running)  /* is mds running? */
    {
      keepgoing = True;
      mds_once(True);

      update_plot(&xgobi);
    }

    if (RunWorkProc((xgobidata *) &xgobi))
      keepgoing = True;

    if (xgv_is_running) {
      plot_once(&xgobi);
    }
  }

  if (keepgoing)
    return FALSE;
  else
    return TRUE;
}

void
AddXGobiFallbackResources (Display *dpy)
{
  int i, len, varlen;
  XrmDatabase xgobidb0;
  char *xgfbstr;

  /*
   * Read the fallback_resources, now an array of strings,
   * into a single string.
  */
  len = 2048;
  varlen = 0;
  xgfbstr = XtMalloc(len * sizeof(char));
  i = 0;
  while (fallback_resources[i] != NULL) {
    strcpy(xgfbstr+varlen, (char *) fallback_resources[i]);
    varlen += strlen(fallback_resources[i]);
    i++;
    strcpy(xgfbstr+varlen, "\n");
    varlen += 1;

    if (fallback_resources[i] != NULL) {
      if (varlen + strlen(fallback_resources[i]) + 2 > len) {
        /* printf("reallocating\n"); */
        len = 2*len;
        xgfbstr = XtRealloc(xgfbstr, len*sizeof(char));
      }
    }
  }

  /* Now I can read it in and merge it with the default */
  xgobidb0 = XrmGetStringDatabase(xgfbstr);
  if (xgobidb0) {
    XrmMergeDatabases(xgobidb0, &dispdb);
    XrmSetDatabase(dpy, dispdb);
  }
}


void
AddUserResources(Display *dpy, String classname)
/*
 * Read in the resource file for the application named "classname".
 * Use the shell variables XFILESEARCHPATH and/or XUSERFILESEARCHPATH,
 * or use XAPPLRESDIR.  If none of those exists, return NULL.
*/
{
  char *filename;
  XrmDatabase rdb = NULL;
  XrmDatabase xpathrdb = NULL, xuserpathrdb = NULL, xapplpathrdb = NULL;

  char *xpath = getenv("XFILESEARCHPATH");
  char *xuserpath = getenv("XUSERFILESEARCHPATH");
  char *xapplpath = getenv("XAPPLRESDIR");

  if (xpath) {
    if ((filename = XtResolvePathname(dpy, NULL, classname, NULL,
        xpath, NULL, 0, NULL)) != NULL)
    {
      xpathrdb = XrmGetFileDatabase(filename);
    }
  }

  if (xuserpath) {
    if ((filename = XtResolvePathname(dpy, NULL, classname, NULL,
        xuserpath, NULL, 0, NULL)) != NULL)
    {
      xuserpathrdb = XrmGetFileDatabase(filename);
    }
  }

  if (!xpath && !xuserpath) {
    if ((filename = XtResolvePathname(dpy, NULL, classname, NULL,
        xapplpath, NULL, 0, NULL)) != NULL)
    {
      xapplpathrdb = XrmGetFileDatabase(filename);
    }
  }

  if (xpathrdb) {
    if (xuserpathrdb) {
      XrmMergeDatabases(xuserpathrdb, &xpathrdb);
    }
    rdb = xpathrdb;
  }
  else {
    if (xuserpathrdb)
      rdb = xuserpathrdb;
    else if (xapplpathrdb)
      rdb = xapplpathrdb;
  }

  if (rdb) {
    XrmMergeDatabases(rdb, &dispdb);
    XrmSetDatabase(display, dispdb);
  }
}


/* resource database */
XrmDatabase dispdb;

main(int argc, char *argv[])
{
  int i, j;
  char **col_name;
  float **fdata;
  /*XrmDatabase db;*/

/*
 * X Initialization
*/
  shell = XtAppInitialize(&app_con, "XGvis", NULL, 0,
    &argc, argv, xgvis_fallback_resources, NULL, 0);
  display = XtDisplay(shell);
  XtAppAddActions(app_con, added_actions, XtNumber(added_actions));
  find_mono();

/* More X stuff. */
  dispdb = XrmGetDatabase(display);
  /*XrmPutFileDatabase(dispdb, "/usr/dfs/xgvis/sgi/DB0");*/
  AddXGobiFallbackResources (display);
  /* db = XrmGetDatabase(display); */
  /*XrmPutFileDatabase(db, "/usr/dfs/xgvis/sgi/DB1");*/
  AddUserResources(display, "XGobi");
  /* db = XrmGetDatabase(display); */
  /*XrmPutFileDatabase(db, "/usr/dfs/xgvis/sgi/DB2");*/
  XtGetApplicationResources(shell, &panel_data, panel_resources, 
    XtNumber(panel_resources), NULL, 0);

  /* db = XrmGetDatabase(display); */
  /*XrmPutFileDatabase(db, "/usr/dfs/xgvis/sgi/DB3");*/

  /* Allow the embedding dimension to be reset from the resources file */
  /*mds_dims = panel_data.mdsDimension;*/

  /* ... but probably the command line is most useful */
  initialize_data(argc, argv);
  reset_data();    /* Get fresh data. */

/*
 * Set up the control panel
*/
  if (mono) {
    /* These haven't yet been initialized at this point */
    appdata.fg = BlackPixelOfScreen(XtScreen(shell));
    appdata.bg = WhitePixelOfScreen(XtScreen(shell));
  }

  make_xgvis_widgets();

  XtRealizeWidget(shell);

  xgobi.std_type = 0;
  /*xgobi.std_width = 2.0;*/  /* obsolete */
  xgobi.is_iconified = False;
  xgobi.data_mode = ascii;

/* Use more appropriate variable labels in XGobi */
  col_name = (char **) XtMalloc(
    (Cardinal) MAXDIMS * sizeof (char *));
  for (i=0; i<MAXDIMS; i++) {
    col_name[i] = (char *) XtMalloc(
      (Cardinal) 32 * sizeof(char));
    sprintf(col_name[i], "Dim %d", i+1);
  }

/*
 * I'm using doubles internally now, so I have to initialize
 * a float data array for xgobi -- I can free it once xgobi
 * has been initialized.
*/

  fdata = (float **) XtMalloc((unsigned) pos.nrows * sizeof(float *));
  for (i=0; i<pos.nrows; i++) {
    fdata[i] = (float *) XtMalloc((unsigned) pos.ncols * sizeof(float));
    for (j=0; j<pos.ncols; j++)
      fdata[i][j] = (float) pos.data[i][j];
  }

/*
Boolean datapflag  -- from datap or from a file
char *data_in  -- NULL for us; usually a filename
float **datap  -- pointer to data
char *title -- the title for the XGobi window
short **missingp,
Boolean mv_missing_values_present -- unnecessary argument
Boolean mv_is_missing_values_xgobi
int mv_nmissing,
int nr, nc,
char** rowp, colp,
int nlinks
connect_lines * connecting_lines,
xgobidata * xg
Widget parent
*/
  if (make_xgobi (True, (char *) NULL, fdata, "xgvis display",
    False, (short **) NULL, False, 0,
    pos.nrows, rowlab, pos.ncols, col_name,
    xg_nlines, xg_lines,
    &xgobi, shell, False /*don't plot yet*/) == 0)
  {
    return(0);
  }
  else {
    /* A few things that must be done after xgobi is initiated */
/*
 * Setting height seems to be entirely ineffective *
 *
 *  Dimension hgt1, hgt2;
 *  extern Widget mds_dims_label;
 *  XtVaGetValues(dims_left, XtNheight, &hgt1, NULL);
 *  XtVaGetValues(mds_dims_label, XtNheight, &hgt2, NULL);
*/
    XSync(display, False);

    for (i=0; i<pos.nrows; i++)
      XtFree((char *) fdata[i]);
    XtFree((char *) fdata);

    /* addsuffix = True */
    if (strlen(pcolorname) == 0) sprintf(pcolorname, "%s", xgv_basename);
    if (strlen(lcolorname) == 0) sprintf(lcolorname, "%s", xgv_basename);
    if (strlen(glyphname) == 0) sprintf(glyphname, "%s", xgv_basename);

    read_point_colors(pcolorname, True, True, &xgobi);
    read_point_glyphs(glyphname, True, True, &xgobi);
    read_line_colors(lcolorname, True, True, &xgobi);

    set_vgroups();
    update_lims(&xgobi);
    update_world(&xgobi);
    world_to_plane(&xgobi);
    plane_to_screen(&xgobi);

    init_tickdelta(&xgobi);
    init_ticks(&xgobi.xy_vars, &xgobi);
    plot_once(&xgobi);
    /* seems like a kludge, but there's another plot_once called
     * async afterwards which makes lines not show up */
    xgobi.got_new_paint = True;

    mds_once(False);
    draw_stress();  /* initialize the stress plot */

    /* initialize the dissimilarity plot */
    init_dissim();

    XtVaSetValues(dims_left,
      XtNbitmap, (Pixmap) leftarr,
      XtNborderColor, (Pixel) appdata.fg,
      NULL);
    XtVaSetValues(dims_right,
      XtNbitmap, (Pixmap) rightarr,
      XtNborderColor, (Pixel) appdata.fg,
      NULL);

    /* addsuffix = True */
/*
    if (strlen(pcolorname) == 0) sprintf(pcolorname, "%s", xgv_basename);
    if (strlen(lcolorname) == 0) sprintf(lcolorname, "%s", xgv_basename);
    if (strlen(glyphname) == 0) sprintf(glyphname, "%s", xgv_basename);

    read_point_colors(pcolorname, True, True, &xgobi);
    read_point_glyphs(glyphname, True, True, &xgobi);
    read_line_colors(lcolorname, True, True, &xgobi);
*/

    /*
     * I can't seem to make this work with resource files, so
     * I'm going to override the resources here.  Ugly in principle,
     * but probably desirable in practice.
    */
    xgobi.is_axes = False;
    if (edges_orig.nrows > 0 || lines.nrows > 0) {
      xgobi.connect_the_points = True;
      xgobi.plot_the_arrows = False;
    }
    set_display_menu_marks(&xgobi);


    XGobiMainLoop(&xgobi);
  }

  return 1;
}
