/* xgobi.c */
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

#define XGOBIINTERN

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include <X11/keysym.h>

xgobidata xgobi;
#include "xgobitop.h"

#if defined RPC_USED || defined DCE_RPC_USED
#define PROGINTERN
#include "rpc_xgobi.h"
#include "rpc_vars.h"
#endif

#ifdef CORBA_USED
  extern initCorba(char *argv[], long argc, char **name, long,
    XtAppContext *, xgobidata *);
  extern void CorbaEventLoop(xgobidata *xgobidata);

    /* Run-time flag indicating whether to use CORBA. */
  int UseCorba = 0;
#endif


static char message[MSGLENGTH];
static char *version_date = "April 12, 2002";

/*
 * These are the panel resources defined uniquely for AtoXgob
 */

#if defined RPC_USED || defined DCE_RPC_USED

static  XtResource rpc_panel_resources[] =
{
/* Monochrome defaults */
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
  XtOffset(AppDataPtr, fg), XtRString, "Black"},

  {XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
  XtOffset(AppDataPtr, bg), XtRString, "White"},

  {XtNborderColor, XtCBorderColor, XtRPixel, sizeof(Pixel),
  XtOffset(AppDataPtr, border), XtRString, "Black"},

/* Font for labels and buttons */
  {
    "font", "Font", XtRFontStruct, sizeof (XFontStruct *),
    XtOffset (PanelDataPtr, Font), XtRString, 
    "-*-helvetica-medium-r-*-*-14-*-*-*-*-*-*-*"
  },

/* Help font */
  {
    "helpFont", "HelpFont", XtRFontStruct, sizeof (XFontStruct *),
    XtOffset(PanelDataPtr, helpFont), XtRString,
    "-*-helvetica-medium-r-*-*-14-*-*-*-*-*-*-*"
   },
};

#endif


/*
 * An X program always has an event loop running, looking for
 * input:  usually user-generated mouse motions and so forth.  The
 * XGobi event loop is called XGobiMainLoop() and it's in
 * xgobitop.h.
 * 
 * A work proc is a routine that runs once whenever the event loop
 * finds no events, a sort of run-while-idle routine.  In XGobi,
 * continuous processes such as rotation are handled using work
 * procs:  for example, if no user input is found, spin_once().
 * Usually, an X programmer doesn't write her own RunWorkProcs()
 * routine, but we found it necessary to do so for XGobi.
 * 
 * In prog, we added one additional background routine to the set
 * used in XGobi -- it is called loop_once(). It checks for incoming
 * data from ARC/INFO and updates the structure xgobidata.
 */


/* ARGSUSED */
Boolean RunWorkProcs (void *dummyArgc)
{
  Boolean RunWorkProc(xgobidata *);

  int   keepgoing = 0;
  int   replot = 0;
  Boolean prev_is_color_painting;
  
  if ((! xgobi.arcview_flag) && (! xgobi.xplore_flag) && (! xgobi.virgis_flag))
  {
    if (xgobi.is_iconified)
      return TRUE;
    else 
      return (! RunWorkProc ((xgobidata *) &xgobi));
  }

#if defined RPC_USED || defined DCE_RPC_USED
  wait_for_rpcs(0);
#endif

  if (xgobi.is_realized /* && !xgobi.is_iconified */) {

/* !!!MS!!! */
#if defined RPC_USED || defined DCE_RPC_USED
    if (is_running) {

      keepgoing = 1;

      /* 
       * Here's the line you'll want to replace with your own
       * background routine or routines.
       */
      replot = update_required;
      update_required = False;
      
      /* 
       * Here is where the change to xg->raw_data[][] made in
       * loop_once() is sent through the data pipeline so that
       * it will show up on the screen at the next plot_once().
       */
      if (replot)
      {
        prev_is_color_painting = xgobi.is_color_painting;
        xgobi.is_color_painting = True && (!mono);
        copy_raw_to_tform (&xgobi);
        update_lims (&xgobi);
        update_world (&xgobi);
        world_to_plane (&xgobi);
        plane_to_screen (&xgobi);
        xgobi.is_color_painting = prev_is_color_painting;
      }
    }
#endif

    keepgoing = (RunWorkProc ((xgobidata *) &xgobi));

    if (replot)
    {
      /* Plot */
      prev_is_color_painting = xgobi.is_color_painting;
      xgobi.is_color_painting = True && (!mono);
      plot_once (&xgobi);
      xgobi.is_color_painting = prev_is_color_painting;
    }
  }

  if (keepgoing || xgobi.arcview_flag || xgobi.xplore_flag || xgobi.virgis_flag)
    return FALSE; /* FALSE => call WorkProc again */
  else
    return TRUE;  /* TRUE => do not call WorkProc again */
}


/*
 * This routine is meant to handle the case where you have
 * both an XGobi resource file and a resource file for your
 * parent program, and you'd like to read them both and
 * fold them together.  You can lift this routine as is,
 * or leave it out if you aren't interested in using a
 * resource file for your parent program.
 */

/*
 * Read in the resource file for the application named "classname".
 * Use the shell variables XFILESEARCHPATH and/or XUSERFILESEARCHPATH,
 * or use XAPPLRESDIR.  If none of those exists, return NULL.
 */
XrmDatabase GetExtraResources (Display *dpy, String classname)
{
  char *filename;
  XrmDatabase rdb = NULL;
  XrmDatabase xpathrdb = NULL, xuserpathrdb = NULL, xapplpathrdb = NULL;

  char *xpath = getenv ("XFILESEARCHPATH");
  char *xuserpath = getenv ("XUSERFILESEARCHPATH");
  char *xapplpath = getenv ("XAPPLRESDIR");

  if (xpath)
  {
    if ((filename = XtResolvePathname (dpy, NULL, classname, NULL,
	    xpath, NULL, 0, NULL)) != NULL)
    {
      xpathrdb = XrmGetFileDatabase (filename);
    }
  }

  if (xuserpath)
  {
    if ((filename = XtResolvePathname (dpy, NULL, classname, NULL,
	    xuserpath, NULL, 0, NULL)) != NULL)
    {
      xuserpathrdb = XrmGetFileDatabase (filename);
    }
  }

  if (!xpath && !xuserpath)
  {
    if ((filename = XtResolvePathname (dpy, NULL, classname, NULL,
	    xapplpath, NULL, 0, NULL)) != NULL)
    {
      xapplpathrdb = XrmGetFileDatabase (filename);
    }
  }

  if (xpathrdb)
  {
    if (xuserpathrdb)
    {
      XrmMergeDatabases (xuserpathrdb, &xpathrdb);
    }
    rdb = xpathrdb;
  }
  else
  {
    if (xuserpathrdb)
      rdb = xuserpathrdb;
    else
      if (xapplpathrdb)
	rdb = xapplpathrdb;
  }
  return rdb;
}


int
parse_command_line(int *ac, char **av, char *data_in, xgobidata *xg)
{
  int retval = 1;
  unsigned long server_number = 0L;

/*
 * Now parse the command line.
*/
  for( ; (*ac)>1 && av[1][0]=='-'; (*ac)--,av++) {
    /*
     * -s:  xgobi initiated from inside S
    */
    if (strcmp(av[1], "-s") == 0)
      xg->data_mode = Sprocess;

    /*
     * -std:  look for one of mmx (default), msd, or mmd
    */
    else if (strcmp(av[1], "-std") == 0) {
      if (strcmp(av[2], "mmx") == 0) {
        xg->std_type = 0;
        av++; (*ac)--;
      }
      else if (strcmp(av[2], "msd") == 0) {
        xg->std_type = 1;
        av++; (*ac)--;
      }
      else if (strcmp(av[2], "mmd") == 0) {
        xg->std_type = 2;
        av++; (*ac)--;
      }
    }

    /*
     * -subset:  Sample size n; look for an integer.
    */
    else if (strcmp(av[1], "-subset") == 0) {
      int n = atoi(av[2]);
      if (n > 1) {
        xg->nrows_in_plot = n;
        fprintf(stderr, "Starting xgobi with a sample of size %d\n", n);
      }
      av++; (*ac)--;
    }

    /*
     * -only n/N:  Draw a sample of n assuming there are N rows in the data.
     * -only a,n:  Starting with row a, read only n rows
    */
    else if (strcmp(av[1], "-only") == 0) {
      int n1 = -1, n2 = -1;
      char spec[128];
      xg->file_read_type = read_all;

      /*
       * we normally start at the first row of the file, and read until
       * reaching EOF
      */
      xg->file_start_row = 0;
      xg->file_length = 0;
      xg->file_sample_size = 0;

      strcpy(spec, av[2]);

      if (strchr((const char *) spec, '/') != NULL)
        xg->file_read_type = draw_sample;
      else if (strchr((const char *) spec, ',') != NULL)
        xg->file_read_type = read_block;

      if (xg->file_read_type == read_all)
        exit(0);

      n1 = atoi(strtok(spec, ",/"));
      n2 = atoi(strtok((char *) NULL, ",/"));

      if (n1 == -1 || n2 == -1)
        exit(0);

      if (xg->file_read_type == draw_sample) {
        xg->file_sample_size = n1;
        xg->file_length = n2;
        fprintf(stderr, "drawing a sample of %d of %d\n", n1, n2);
      }
      else if (xg->file_read_type == read_block) {
        xg->file_start_row = n1 - 1;
        xg->file_sample_size = n2;
        fprintf(stderr, "reading %d rows, starting with %d\n", n2, n1);
      }

      av++; (*ac)--;
    }


    /*
     * -mono:  force black and white display
    */
    else if (strcmp(av[1], "-mono") == 0) 
      mono = 1;

    /*
     * -scatmat:  construct a scatterplot matrix
    */
    else if (strcmp(av[1], "-scatmat") == 0) 
      xg->is_scatmat = True;

    /*
     * -version:  print version date, return
    */
    else if (strcmp(av[1], "-version") == 0) {
      fprintf(stdout, "This version of XGobi is dated %s\n", version_date);
      exit(0);
    }

    else if (strcmp (av[1], "-vtitle") == 0) {
    /*
     * If the argument starts with a quote, start with the second
     * character and read up to but not including the next quote, ie,
     * read past blanks.  This only works if the argument is doubly
     * nested inside quotation marks; either way will do:
     *   -vtitle "'abc def'"
     *   -vtitle '"abc def"'
     */
      if (av[2][0] == '\'')
        (void) sscanf (&av[2][1], "%[^']s", xgobi.vtitle);
      else if (av[2][0] == '"')
        (void) sscanf (&av[2][1], "%[^\"]s", xgobi.vtitle);
      else
        (void) sscanf (av[2], "%s", xgobi.vtitle);
      av++; (*ac)--;
    }

#ifdef CORBA_USED
    else if (strcmp(av[1],"-CORBAName") == 0) {
      UseCorba = 1;
      parseCorbaName(av[2]);
      av++; (*ac)--;
    } else if (strcmp(av[1],"-CORBA") == 0) {
      UseCorba = 1;
    }
    /* Other CORBA arguments such as the default naming service's IOR,
       configuration file, etc. will be handled by the particular
       ORB implementation's ORB_init() method called in the initCorba.
       So we don't have to handle them here.
     */
#endif

#ifdef RPC_USED

    else if (strcmp (av[1], "-server") == 0) {
      server_number = 0L;
      (void) sscanf (av[2], "%x", &server_number);
      av++; (*ac)--;
    }

    else if (strcmp (av[1], "-client") == 0) {
      xg_server_number = 0L;
      (void) sscanf (av[2], "%x", &xg_server_number);
      av++; (*ac)--;
    }

    else if (strcmp (av[1], "-link") == 0) {
      if (strcmp (av[2], "arcview") == 0) {
        xgobi.arcview_flag = True;
        av++; (*ac)--;
      }
      else if (strcmp (av[2], "xplore") == 0) {
        xgobi.xplore_flag = True;
        av++; (*ac)--;
      }
      else if (strcmp (av[2], "virgis") == 0) {
        xgobi.virgis_flag = True;
        av++; (*ac)--;
      }
    }

#endif

#ifdef DCE_RPC_USED
    else if (strcmp(av[1], "-link") == 0) {
      /* uses the virgis-flag */
      xgobi.virgis_flag = True;
    } else if (strcmp(av[1], "-server") == 0) {
      strcpy(pszNWAddressCl, av[2]);
      av++; (*ac)--;
    }
#endif

  }
  (*ac)--;
  av++;

/*
 * Test the values
*/

#ifdef RPC_USED

  if (xg -> arcview_flag)
    av_server_number = server_number;
  else if (xg -> xplore_flag)
    xpl_server_number = server_number;
  else if (xg -> virgis_flag)
    vg_server_number = server_number;

  if (xg -> arcview_flag || xg -> xplore_flag || xg -> virgis_flag)
    printf ("other RPC server is %x, xg_server is %x\n",
      server_number, xg_server_number);

#endif

  if (xg->std_type != 0 && xg->std_type != 1 && xg->std_type != 2)
  {
    (void) fprintf(stderr,
      "std: Standardization type not valid; aborting.\n");
    retval = 0;
    return(retval);
  }

  if (xg->data_mode == Sprocess)
  {
    if (*ac < 3)
    {
      (void) fprintf(stderr,
        "Usage: xgobi [-s nrows ncols path] filename\n");
      retval = 0;
      return(retval);
    }
    else
    {
      xg->nrows = atoi(*av); av++;
      xg->ncols_used = atoi(*av); av++;
      xg->ncols = xg->ncols_used + 1;
      (void) strcpy(Spath0, *av); av++;
      (void) strcat(Spath0, "/");
      (void) strcpy(data_in, *av);
    }
  }
  else /* if (xg->data_mode == ascii || xg->data_mode == binary) */
  {
    if (*ac == 0)
      (void) strcpy(data_in, "stdin");
    else
      (void) strcpy(data_in, *av);
  }
  return(retval);
}


int
main(int argc, char *argv[])
{
  XrmDatabase xgobidb;
  char data_in[100];
  struct stat buf;
  char **rowl, **coll;

/* For testing whether to show the splash screen */
  char fname[128];
  mode_t mode = 0600;
  char *uhome = getenv("HOME");

#ifdef RPC_USED
  xg_server_number = 0x42000000; 
  av_server_number = 0x43000000;
  xpl_server_number = 0x44000000;
  vg_server_number = 0x45000000;
#endif

#ifdef DCE_RPC_USED
  strcpy(pszNWAddressCl, "");
#endif

  xgobi.shell = XtAppInitialize(&app_con, "XGobi", 
    (XrmOptionDescRec *) NULL, (Cardinal) 0,
    &argc, argv, fallback_resources, (ArgList) NULL, (Cardinal) 0);
  display = XtDisplay(xgobi.shell);
  XtAppAddActions(app_con, added_actions, XtNumber(added_actions));
  mono = find_mono();




/*
 * Initialize a few values before beginning to read the
 * command line.
*/
  xgobi.std_type = 0;
  xgobi.data_mode = ascii;
  xgobi.nrows_in_plot = -1;
  xgobi.file_read_type = read_all;

  xgobi.arcview_flag = False;
  xgobi.xplore_flag = False;
  xgobi.virgis_flag = False;
  xgobi.xgobi_is_up = False;

  xgobi.is_scatmat = False;

  xgobi.progname = argv[0];

  sprintf(xgobi.vtitle, "");
  if (!parse_command_line(&argc, argv, data_in, &xgobi))
    return(0);


#ifdef CORBA_USED
  if(UseCorba != 0) {
     initCorba(argv, argc, NULL, 0, &app_con, &xgobi);
  }
#endif


#if defined RPC_USED || defined DCE_RPC_USED
  DTEXT("RPC Initlalisierung\n");

  if (xgobi.arcview_flag || xgobi.xplore_flag || xgobi.virgis_flag) {

    /* 
     * Mandatory X initialization line; replace YourProg with your
     * own application name.  This name could be used in resource
     * files or to provide the name that appears in the title 
     * bar.
     */

    prog_shell = XtAppInitialize (&app_con, "AV2XGobi", NULL, 0,
      &argc, argv, fallback_resources, NULL, 0);
    display = XtDisplay (prog_shell);
    XtAppAddActions (app_con, added_actions, XtNumber (added_actions));
    mono = find_mono ();

    /* 
     * More X and XGobi stuff having to do with resources. 
     */

    XtGetApplicationResources (prog_shell, &panel_data, rpc_panel_resources,
      XtNumber (rpc_panel_resources), NULL, 0);
    /*
      GetApplResources(&xgobi);
    */
    xgobidb = GetExtraResources (display, "XGobi");
    if (xgobidb) {
      /* XrmMergeDatabases (xgobidb, &display->db); */
      XrmDatabase dispdb = XrmGetDatabase(display);
      XrmMergeDatabases(xgobidb, &dispdb);
      XrmSetDatabase(display, dispdb);
    }

    /*
     * Initialize each XGobi with a unique name.
     */
    (void) sprintf (message, "AV2XGobi [%d]", getpid ());
    XtVaSetValues (/* prog_shell */ xgobi.shell,
      XtNfont, /* panel_data.Font */ appdata.font,
      XtNtitle, (String) message,
      XtNiconName, (String) message,
      NULL);


    /* 
     * Some XGobi variables that require initialization.
    */
    xgobi.std_type = 0;
    xgobi.is_realized = True;
    xgobi.is_iconified = False;
    xgobi.data_mode = ascii;
    xgobi.shell = NULL;
    xgobi.isCloned = False;
    is_running = False;
    update_required = False;

    DTEXT("Initialisiere Server\n");

#ifdef DCE_RPC_USED
    dce_rpc_server_main();
    dce_rpc_client_init();
#else

    rpc_server_main (xg_server_number, (u_long) 1);

    if (xgobi.arcview_flag)
      av_server_id = rpc_client_main (av_server_number, (u_long) 1);
    else if (xgobi.xplore_flag)
      xpl_server_id = rpc_client_main (xpl_server_number, (u_long) 1);
    else if (xgobi.virgis_flag)
      vg_server_id = rpc_client_main (vg_server_number, (u_long) 1);
#endif
    DTEXT("Vor Initialisierungsschleife\n");

    while (! is_running)
      wait_for_rpcs(1);

    DTEXT("Nach Initialisierungsschleife\n");

    (void) XtAppAddWorkProc (app_con, (XtWorkProc) RunWorkProcs, NULL);

    XGobiMainLoop (&xgobi);
    return (0);
  }

#endif

#ifdef SENDMAIL
  /* In the local version, send mail when xgobi is initiated.  */
/*
  if (stat("/usr/lib/sendmail", &buf) == 0)
  {
    system("/usr/lib/sendmail dfs@research.att.com \
      <<!\nSubject: Starting XGobi\n.\n!");
  }
  else if (stat("/usr/public/bin/sendmail", &buf) == 0)
  {
    system("/usr/public/bin/sendmail - dfs@research.att.com \
      +elec -check subject='Starting XGobi' file='/tmp/foo'\n");
  }
  else if (stat("/usr/local/bin/sendmail", &buf) == 0)
  * On 214 machines, don't use the "-check" argument *
  {
    system("/usr/local/bin/sendmail - dfs@research.att.com \ 
      subject='Starting XGobi' file='/tmp/foo'");
  }
  * 214 solaris machines don't seem to have a user sendmail *
  else if (stat("/usr/lib/sendmail", &buf) == 0)
  {
    system("/usr/lib/sendmail dfs@research.att.com \
      <<!\nSubject: Starting XGobi\n.\n!");
  }
*/
#endif

/*
 * make_xgobi(datapflag, data_in, datap, xgobi_title,
 *  missingpflag, missingp, mv_is_missing_values_xgobi, mv_nmissing,
 *  nr, rowp, nc, colp,
 *  nlines, connecting_lines,
 *  xg, parent, Boolean plot)
*/

/*
 * If the init file isn't present, show the splash screen
*/
  sprintf(fname, "%s/.xgobiinit", uhome);
  /*free(uhome);*/
  /*fprintf(stderr, "Looking for the file %s\n", fname);*/
  if (stat(fname, &buf) != 0) {
    extern void show_splash_screen(xgobidata *);
    show_splash_screen(&xgobi);
    (void) creat((char *) fname, mode);
  }

  rowl = (char **) NULL;
  coll = (char **) NULL;
  if (make_xgobi(False, data_in, (float **) NULL, (char *) NULL,
    False, (short **) NULL, False, 0,
    (int) NULL, rowl, (int) NULL, coll,
    (int) NULL, (connect_lines *) NULL,
    &xgobi, (Widget) NULL, True) == 0)
  {
    return(0);
  }
  else
  {
#ifdef CORBA_USED
  if (UseCorba)
    CorbaEventLoop(&xgobi);
  else
#endif
    XGobiMainLoop(&xgobi);
  }

  return(1);
}
