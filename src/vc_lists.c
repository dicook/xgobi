/* vc_lists.c */
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
#include <X11/Xatom.h>
#include <X11/Xproto.h>

#define DEFAULT_PROP_SIZE (10240L)

#define TRANSIENT  0
#define PERSISTENT 1
static int casemode = TRANSIENT;

static Widget doallvarsbox, doallcasesbox, doallvars[2], doallcases[2];
static Widget varlist_text, varlist_close, varlist_mgr;
Widget varlist_popup = (Widget) NULL;
static Widget caselist_text, caselist_close, caselist_mgr;
Widget caselist_popup = (Widget) NULL;
static XawTextSelectType select_array[] = {XawselectLine, XawselectNull} ;

static char *allvarnames, *allcasenames;
static Cardinal *initvarpos, *initcasepos;

#define VSELECTALL   doallvars[0]
#define VDESELECTALL doallvars[1]
#define CSELECTALL   doallcases[0]
#define CDESELECTALL doallcases[1]

/* ARGSUSED */
XtCallbackProc
persistent_casemode_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  casemode = !casemode;
  setToggleBitmap(w, casemode);
}

/* ARGSUSED */
XtCallbackProc
select_all_vars_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i;
  XawTextBlock newtext;
  XawTextPosition start, end;

  for (i=0; i<xg->ncols_used; i++)
  {
    allvarnames[initvarpos[i]] = '+' ;
    xg->selectedvars[i] = True ;
  }

  start = 0;
  end = (long) strlen(allvarnames);
  newtext.ptr = allvarnames ;
  newtext.format = FMT8BIT ;
  newtext.firstPos = start;
  newtext.length = end;
  XawTextReplace(varlist_text, start, end, &newtext) ;
  XawTextInvalidate(varlist_text, start, end);

  if (xg->is_cprof_plotting && xg->link_cprof_plotting) {
    update_cprof_selectedvars(xg);
    reset_nvars_cprof_plot(xg);
  }

}

/* ARGSUSED */
XtCallbackProc
select_all_cases_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i;
  XawTextBlock newtext;
  XawTextPosition start, end;

  for (i=0; i<xg->nrows; i++)
  {
    allcasenames[initcasepos[i]] = '+' ;
    xg->selectedcases[i] = True ;
  }

  start = 0;
  end = (long) strlen(allcasenames);
  newtext.ptr = allcasenames ;
  newtext.format = FMT8BIT ;
  newtext.firstPos = start;
  newtext.length = end;
  XawTextReplace(caselist_text, start, end, &newtext) ;
  XawTextInvalidate(caselist_text, start, end);

  update_sticky_ids(xg);
  plot_once(xg);
  if (xg->is_cprof_plotting && xg->link_cprof_plotting)
    passive_update_cprof_plot(xg);
}

/* ARGSUSED */
XtCallbackProc
deselect_all_cases_cback(Widget w, xgobidata *xg, XtPointer callback_data)
{
  int i;
  XawTextBlock newtext;
  XawTextPosition start, end;

  for (i=0; i<xg->nrows; i++)
  {
    allcasenames[initcasepos[i]] = ' ' ;
    xg->selectedcases[i] = False ;
  }
  xg->nearest_point = -1;

  start = 0;
  end = (long) strlen(allcasenames);
  newtext.ptr = allcasenames ;
  newtext.format = FMT8BIT ;
  newtext.firstPos = start;
  newtext.length = end;
  XawTextReplace(caselist_text, start, end, &newtext) ;
  XawTextInvalidate(caselist_text, start, end);

  update_sticky_ids(xg);
  plot_once(xg);
  if (xg->is_cprof_plotting && xg->link_cprof_plotting)
    passive_update_cprof_plot(xg);
}

void
build_replacement_text(Cardinal indx,
Cardinal *initposvec, char *allnames, Boolean *selected)
{
  Boolean has_plus;
  int initpos = initposvec[indx];

  if (allnames[initpos] == ' ')
    has_plus = False;
  else if (allnames[initpos] == '+')
    has_plus = True;
  else
  {
    fprintf(stderr, "Problem!\n");
    return;
  }

  /* the mode is now always TOGGLE */
  if (has_plus) {
    allnames[initpos] = ' ' ;
    selected[indx] = False ;
  } else {
    allnames[initpos] = '+' ;
    selected[indx] = True ;
  }
}

/* ARGSUSED */
void
reset_list_selection(Widget w, xgobidata *xg, char *allnames,
Cardinal *initpos, Boolean *selected, int n)
{
  XawTextPosition start, end;
  Cardinal indx;
  XawTextBlock newtext;
  Boolean error = False;
  /*
   * start is position 0
   * end is the next unhighlighted position after the selection
  */
  XawTextGetSelectionPos(w, &start, &end);

  /*
   * Find the first index, then step through allnames[]
  */
  if (allnames[start] == '+' || allnames[start] == ' ')
  {
    (void) sscanf(&allnames[start+2], "%d", &indx);
    if (indx < 1 || indx > n)
      error = True;
  }

  if (error)
  {
    (void) fprintf(stderr,
      "Selection imperfectly formed; please select again\n");
  }
  else
  {
    indx-- ;
    while (indx < n) {
      if (initpos[indx] < end - 4) {
         build_replacement_text(indx, initpos, allnames, selected) ;
         indx++ ;
      }
      else
        break ;
    }
    newtext.ptr = allnames ;
    newtext.format = FMT8BIT ;
    newtext.firstPos = start ;
    newtext.length = end - start ;
    XawTextReplace(w, start, end, &newtext) ;
    XawTextInvalidate(w, start, end);
  }
}

/* ARGSUSED */
XtActionProc
ToggleCurrentLine(Widget w, XEvent *evnt, String *params, Cardinal nparams)
{
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;
  int line = -1;
  Boolean ok = True;

  XawTextPosition start, end;
  int i;
  static int prev_line = -1;

  /* This routine was kicked off by a button press or by button motion */
  Boolean button_press = True;
  if (strcmp(params[0], "m") == 0) button_press = False;

  /* Kludge */
  xg->current_window = True;

  if (w == caselist_text)
  {
/*
 * Try a different way of getting the line number.
*/
    XawTextGetSelectionPos(w, &start, &end);
    for (i=0; i<xg->nrows; i++) {
      if (initcasepos[i] >= start) {
        line = i;
        break;
      }
    }

    if (line == -1)
      return((XtActionProc) 0);

    if (casemode == TRANSIENT)
    {
      /* For transient labels, try to prevent unnecessary redrawing */
      if (line == prev_line)
        return((XtActionProc) 0);

      if (line >= 0 && line <= xg->nrows)
          xg->nearest_point = line;
      else
        ok = False;

    } else {  /* if PERSISTENT */

       reset_list_selection(w, xg, allcasenames, initcasepos,
         xg->selectedcases, xg->nrows);

        update_sticky_ids(xg);
        plot_once(xg);
        if (xg->is_cprof_plotting && xg->link_cprof_plotting)
          passive_update_cprof_plot(xg);
    }

    if (ok) {

      if (xg->link_identify) {
        XtOwnSelection( (Widget) xg->workspace,
          (Atom) XG_IDS,
          (Time) CurrentTime, /* doesn't work with XtLastTimeStamp...? */
          /*(Time) XtLastTimestampProcessed(display),*/
          (XtConvertSelectionProc) pack_ids,
          (XtLoseSelectionProc) NULL ,
          (XtSelectionDoneProc) NULL );
        announce_ids(xg);
      }

      if ( (xg->is_plotting1d && xg->is_plot1d_cycle) ||
           (xg->is_xyplotting && xg->is_xy_cycle) ||
           (xg->is_spinning && !xg->is_spin_paused ) ||
           (xg->is_touring && !xg->is_tour_paused) ||
           (xg->is_corr_touring && !xg->is_corr_paused))
        ;
      else
        quickplot_once(xg);
  
      if (xg->is_cprof_plotting && xg->link_cprof_plotting)
        update_cprof_plot(xg);
    }
  }

  else if (w == varlist_text) {

    XawTextGetSelectionPos(w, &start, &end);
    for (i=0; i<xg->ncols_used; i++) {
      if (initvarpos[i] >= start) {
        line = i;
        break;
      }
    }

    if (line == -1)
      return((XtActionProc) 0);

    /* If this event is responding to button motion, ignore repeated
     * movements on the same line */
    else if (line == prev_line && button_press == False)
      return((XtActionProc) 0);

    prev_line = line;

    /* Always persistent ... */
    reset_list_selection(w, xg, allvarnames,
      initvarpos, xg->selectedvars, xg->ncols_used);

    if (xg->is_cprof_plotting && xg->link_cprof_plotting) {
      update_cprof_selectedvars(xg);
      reset_nvars_cprof_plot(xg);
    }
  }

  prev_line = line;
}

/* ARGSUSED */
static XtCallbackProc
PopdownCaselist(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Pop down the caselist popup.
*/
{
  /*fprintf(stderr, "%d\n", ((ShellWidget) caselist_popup)->shell.popped_up);*/
  XtPopdown(caselist_popup);
  /*fprintf(stderr, "%d\n", ((ShellWidget) caselist_popup)->shell.popped_up);*/
}

/* ARGSUSED */
static XtCallbackProc
PopdownVarlist(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Pop down the scrollable variable list.
*/
{
  XtPopdown(varlist_popup);
}

void
free_varlist(xgobidata *xg)
{
  XtFree((XtPointer) allvarnames);
  XtFree((XtPointer) initvarpos);
  XtFree((XtPointer) xg->selectedvars);
  /*XtDestroyWidget(varlist_popup);*/
}

void
varlist_add_group_var(xgobidata *xg)
{
  char str[32];
  XawTextBlock newtext;
  XawTextPosition start, end;
  Boolean popped_up = False;

  sprintf(str, "\n  %d : group", xg->ncols);
  initvarpos[xg->ncols-1] = strlen(allvarnames)+1 ;
  strcat(allvarnames,  str);
  /* Unselect it */
  xg->selectedvars[xg->ncols-1] = False ;
  allvarnames[initvarpos[xg->ncols-1]] = ' ' ;

  if (varlist_popup != (Widget) NULL) {
    if (((ShellWidget) varlist_popup)->shell.popped_up) {
      XtPopdown(varlist_popup);
      popped_up = true;
    }
    /* Update the text widget as well */
    start = 0;
    end = (long) strlen(allvarnames);
    newtext.ptr = allvarnames ;
    newtext.format = FMT8BIT ;
    newtext.firstPos = start;
    newtext.length = end;
    XawTextReplace(varlist_text, start, end, &newtext) ;
    XawTextInvalidate(varlist_text, start, end);
    if (popped_up)
      XtPopup(varlist_popup, XtGrabNone);
  }
}

void
build_varlist(xgobidata *xg)
{
  char strtmp[200];
  int allvarlen = 0;
  Cardinal j, len1name;

  allvarnames = (char *) XtMalloc((Cardinal)
    (xg->ncols * (COLLABLEN+2*16)) * sizeof(char));
  initvarpos = (Cardinal *) XtMalloc((Cardinal)
    xg->ncols * sizeof(Cardinal));
  xg->selectedvars = (Boolean *) XtMalloc((Cardinal)
    xg->ncols * sizeof(Boolean));

  for (j=0; j<xg->ncols_used; j++)
  {
    xg->selectedvars[j] = True ;  /* Initialize to True, all but the last one */
    sprintf(strtmp, "+ %d : %s\n", j+1, xg->collab[j]);
    len1name = strlen(strtmp) ;
    initvarpos[j] = allvarlen ;
/* bug fix a la mcintosh
*   allvarlen += len1name ;
*   strcat(allvarnames, strtmp);
*/
    strcpy(allvarnames+allvarlen, strtmp);
    allvarlen += len1name ;
  }
  /* unselect 'group' if it has already been created */
  if (xg->ncols_used == xg->ncols) {
    xg->selectedvars[xg->ncols-1] = False ;
    allvarnames[initvarpos[xg->ncols-1]] = ' ' ;
  }

  allvarnames[allvarlen - 1] = '\0';

  /* Make it long enough to contain the group variable later */
  allvarlen += (COLLABLEN+2*16);
  allvarnames = (char *)
    XtRealloc((XtPointer) allvarnames, (Cardinal) allvarlen);
}

void
build_varlist_popup(xgobidata *xg)
{
  char strtmp[200];
  Cardinal j, len1name, longest_len = 0;
  Cardinal longest_indx = 0;
  Dimension width;
  Widget pane;

  for (j=0; j<xg->ncols_used; j++)
  {
    len1name = strlen(xg->collab[j]);
    if (len1name > longest_len )
    {
      longest_len = len1name;
      longest_indx = j;
    }
  }

  sprintf(strtmp, "+ %d : %s", longest_indx+1, xg->collab[longest_indx]) ;
  width = (Dimension) XTextWidth(appdata.font, strtmp, strlen(strtmp));

  varlist_popup = XtVaCreatePopupShell("List",
    topLevelShellWidgetClass, xg->shell,
    XtNinput, True,
    XtNtitle, "Variable List",
    NULL);
  if (mono) set_mono(varlist_popup);

  pane = XtVaCreateManagedWidget("Form",
    panedWidgetClass, varlist_popup,
    XtNorientation, (XtOrientation) XtorientVertical,
    NULL);

  varlist_mgr = XtVaCreateManagedWidget("Panel",
    formWidgetClass, pane,
    NULL);
  if (mono) set_mono(varlist_mgr);

  /*
   * Create the text widget.
  */
  varlist_text = XtVaCreateManagedWidget("Text",
    asciiTextWidgetClass, varlist_mgr,
    XtNfont, appdata.font,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainBottom,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainRight,
    XtNwidth, (Dimension)
      (width + 2*ASCII_TEXT_BORDER_WIDTH + 14),  /* Margins, scrollbar */
    XtNheight, (Dimension) MIN(xg->ncols_used,10)*FONTHEIGHT(appdata.font),
    XtNallowResize, (Boolean) True,
    XtNtype, (XawAsciiType) XawAsciiString,
/*
 * Setting editType to textRead makes the search popup behave correctly,
 * disabling the replace functions, but it also makes it impossible
 * for the programmer to change the text; ie, the '+' signs can't
 * be removed and added as needed.
*/
    XtNeditType, (XawTextEditType) XawtextEdit,
    XtNstring, (String) allvarnames,
    XtNscrollVertical, (XawTextScrollMode) XawtextScrollWhenNeeded,
    XtNdisplayCaret, (Boolean) False,
    XtNselectTypes, (XawTextSelectType) select_array,
    NULL);
  if (mono) set_mono(varlist_text);

  doallvarsbox = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, varlist_mgr,
    XtNfromVert, (Widget) varlist_text,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainBottom,
    XtNbottom, (XtEdgeType) XtChainBottom,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(doallvarsbox);

  VSELECTALL = (Widget) CreateCommand(xg, "Select All",
    True, (Widget) NULL, (Widget) NULL, doallvarsbox, "VarList");
  XtManageChildren(doallvars, 1);
  XtAddCallback(VSELECTALL, XtNcallback,
    (XtCallbackProc) select_all_vars_cback, (XtPointer) xg);

  /*
   * Create the Close button.
  */
  varlist_close = XtVaCreateManagedWidget("Click here to dismiss",
    commandWidgetClass, pane,
    XtNshowGrip, (Boolean) False,
    XtNskipAdjust, (Boolean) True,
    NULL);

  if (mono) set_mono(varlist_close);
  XtAddCallback(varlist_close, XtNcallback,
    (XtCallbackProc) PopdownVarlist, (XtPointer) xg);
}

/* ARGSUSED */
XtCallbackProc
PopupVarlist(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Pop up the scrollable variable list.
*/
{
  Dimension width, height;
  Position x, y;
  Boolean initd = True;
  if (varlist_popup == NULL)
    initd = False;

  if (!initd)
  {
    build_varlist_popup(xg);

    XtVaGetValues(w,
      XtNwidth, &width,
      XtNheight, &height, NULL);
    XtTranslateCoords(w,
      (Position) (width/2), (Position) (height/2), &x, &y);

    XtVaSetValues(varlist_popup,
      XtNx, (Position) x,
      XtNy, (Position) y, NULL);
  }

  XtPopup(varlist_popup, XtGrabNone);
  XRaiseWindow(display, XtWindow(varlist_popup));

  if (!initd)
    set_wm_protocols(varlist_popup);
}

void
free_caselist(xgobidata *xg)
{
  XtFree((XtPointer) allcasenames);
  XtFree((XtPointer) initcasepos);
  XtFree((XtPointer) xg->selectedcases);
  /*XtDestroyWidget(caselist_popup);*/
}

void
build_caselist(xgobidata *xg)
{
  char strtmp[200];
  int allcaselen = 0;
  int j, len1name;

  allcasenames = (char *) XtMalloc((Cardinal)
    (xg->nrows * (ROWLABLEN+8)) * sizeof(char));
  initcasepos = (Cardinal *) XtMalloc((Cardinal)
    xg->nrows * sizeof(Cardinal));
  xg->selectedcases = (Boolean *) XtMalloc((Cardinal)
    xg->nrows * sizeof(Cardinal));

  for (j=0; j<xg->nrows; j++)
  {
    xg->selectedcases[j] = False ;  /* Initialize to False */
    sprintf(strtmp, "  %d : %s\n", j+1, xg->rowlab[j]);
    len1name = strlen(strtmp) ;
    initcasepos[j] = allcaselen ;
/* bug fix; mcintosh
*   allcaselen += len1name ;
*   strcat(allcasenames, strtmp);
*/
    strcpy(allcasenames+allcaselen, strtmp);
    allcaselen += len1name ;
  }

  allcasenames[allcaselen - 1] = '\0';
  allcasenames = (char *)
    XtRealloc((XtPointer) allcasenames, (Cardinal) allcaselen);
}

void
build_caselist_popup(xgobidata *xg)
{
  char strtmp[200];
  Cardinal j, len1name, longest_len = 0;
  Cardinal longest_indx = 0;
  Dimension width;
  Widget modebox, modebtn;
  Widget pane;

  for (j=0; j<xg->nrows; j++)
  {
    len1name = strlen(xg->rowlab[j]) ;
    if (len1name > longest_len )
    {
      longest_len = len1name;
      longest_indx = j;
    }
  }

  sprintf(strtmp, "+ %d : %s", longest_indx+1, xg->rowlab[longest_indx]) ;
  width = (Dimension) XTextWidth(appdata.font, strtmp, strlen(strtmp));

  caselist_popup = XtVaCreatePopupShell("List",
    topLevelShellWidgetClass, xg->shell,
    XtNinput, True,
    XtNtitle, "Case List",
    NULL);
  if (mono) set_mono(caselist_popup);

  pane = XtVaCreateManagedWidget("Form",
    panedWidgetClass, caselist_popup,
    XtNorientation, (XtOrientation) XtorientVertical,
    NULL);

  caselist_mgr = XtVaCreateManagedWidget("Panel",
    formWidgetClass, pane,
    NULL);
  if (mono) set_mono(caselist_mgr);

  /*
   * Create the text widget.
  */
  caselist_text = XtVaCreateManagedWidget("Text",
    asciiTextWidgetClass, caselist_mgr,
    XtNfont, appdata.font,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainBottom,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainRight,
    XtNwidth, (Dimension)
      (width + 2*ASCII_TEXT_BORDER_WIDTH + 20),  /* Margins, scrollbar */
    XtNheight, (Dimension) MIN(xg->nrows,20)*FONTHEIGHT(appdata.font),
    XtNallowResize, (Boolean) True,
    XtNtype, (XawAsciiType) XawAsciiString,
    XtNeditType, (XawTextEditType) XawtextEdit,
    XtNstring, (String) allcasenames,
    XtNscrollVertical, (XawTextScrollMode) XawtextScrollWhenNeeded,
    XtNdisplayCaret, (Boolean) False,
    XtNselectTypes, (XawTextSelectType) select_array,
    NULL);
  if (mono) set_mono(caselist_text);

  /*
   * Create the command buttons that all labels to be selected
   * or deselected at once.
  */
  doallcasesbox = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, caselist_mgr,
    XtNfromVert, (Widget) caselist_text,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainBottom,
    XtNbottom, (XtEdgeType) XtChainBottom,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(doallcasesbox);

  CSELECTALL = (Widget) CreateCommand(xg, "Select All",
    True, (Widget) NULL, (Widget) NULL, doallcasesbox, "CaseList");
  CDESELECTALL = (Widget) CreateCommand(xg, "Deselect All",
    True, (Widget) CSELECTALL, (Widget) NULL, doallcasesbox, "CaseList");
  XtManageChildren(doallcases, 2);

  XtAddCallback(CSELECTALL, XtNcallback,
    (XtCallbackProc) select_all_cases_cback, (XtPointer) xg);
  XtAddCallback(CDESELECTALL, XtNcallback,
    (XtCallbackProc) deselect_all_cases_cback, (XtPointer) xg);

  /*
   * Create the buttons that set persistent or transient highlighting
  */
  modebox = XtVaCreateManagedWidget("Panel",
    boxWidgetClass, caselist_mgr,
    XtNfromVert, (Widget) doallcasesbox,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainBottom,
    XtNbottom, (XtEdgeType) XtChainBottom,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(modebox);

  modebtn = (Widget) CreateToggle(xg, "Persistent Labels",
    True, (Widget) NULL, (Widget) NULL, (Widget) NULL,
    False, ANY_OF_MANY, modebox, "CaseList");
  XtManageChild(modebtn);

  XtAddCallback(modebtn, XtNcallback,
    (XtCallbackProc) persistent_casemode_cback, (XtPointer) xg);

  /*
   * Create the Close button.
  */
  caselist_close = XtVaCreateManagedWidget("Click here to dismiss",
    commandWidgetClass, pane,
    XtNshowGrip, (Boolean) False,
    XtNskipAdjust, (Boolean) True,
    NULL);
  if (mono) set_mono(caselist_close);
  XtAddCallback(caselist_close, XtNcallback,
    (XtCallbackProc) PopdownCaselist, (XtPointer) xg);
}

/* ARGSUSED */
XtCallbackProc
PopupCaselist(Widget w, xgobidata *xg, XtPointer callback_data)
/*
 * Pop up the scrollable case list.
*/
{
  Dimension width, height;
  Position x, y;
  int k;
  Boolean initd = True;
  if (caselist_popup == NULL)
    initd = False;

  if (!initd)
  {
    build_caselist_popup(xg);

    XtVaGetValues(w,
      XtNwidth, &width,
      XtNheight, &height,
      NULL);
    XtTranslateCoords(w,
      (Position) (width/2), (Position) (height/2), &x, &y);

    XtVaSetValues(caselist_popup,
      XtNx, (Position) x,
      XtNy, (Position) y, NULL);
  }

  XtPopup(caselist_popup, XtGrabNone);
  XRaiseWindow(display, XtWindow(caselist_popup));

  /* Make sure current sticky labels are reflected in the display */
  for (k=0; k<xg->nsticky_ids; k++) {
    update_list_selection(xg, xg->sticky_ids[k], True);
  }

  if (!initd)
    set_wm_protocols(caselist_popup);
}

void
update_list_selection(xgobidata *xg, int id, Boolean add) {
  /* add or delete a selection */

  XawTextPosition start, end;
  XawTextBlock newtext;
  char prevch, newch;

  if (add) {
    prevch = ' ';
    newch = '+';
  } else {
    prevch = '+';
    newch = ' ';
  }

  if (allcasenames[initcasepos[id]] == prevch) {
    xg->selectedcases[id] = add;

    start = (long) initcasepos[id];
    end = (long) initcasepos[id+1];
    allcasenames[start] = newch;
    newtext.ptr = allcasenames ;
    newtext.format = FMT8BIT ;
    newtext.firstPos = start ;
    newtext.length = end - start ;
    XawTextReplace(caselist_text, start, end, &newtext) ;
    XawTextInvalidate(caselist_text, start, end);
  }
}
