/* tour_init.c */
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

Widget tour_panel, tour_cmd[10], tour_label[2];
Widget tour_sbar;
Widget tour_interp_menu_cmd, tour_interp_menu, tour_interp_menu_btn[3];
int tour_interp_btn = 0;

#define PAUSE     tour_cmd[0]
#define REINIT    tour_cmd[1]
#define STEP      tour_cmd[2]
#define GO        tour_cmd[3]
#define LOCALSCAN tour_cmd[4]
#define BACKTRACK tour_cmd[5]
#define DIRECTION tour_cmd[6]
#define HIST_ON   tour_cmd[7]
#define STORBAS   tour_cmd[8]

/* To manage the tour link state */
Widget link_menu_label, link_menu_cmd, link_menu, link_menu_btn[3], 
  tour_update_cmd;
char *link_menu_name[] = {"Send", "Receive", "Unlink"};
#define SEND_BTN    link_menu_btn[0]
#define RECEIVE_BTN link_menu_btn[1]
#define UNLINK_BTN  link_menu_btn[2]
#define SEND_NAME    link_menu_name[0]
#define RECEIVE_NAME link_menu_name[1]
#define UNLINK_NAME  link_menu_name[2]

/* For setting the type of manipulation */
Widget manip_type_menu_label, manip_type_menu_cmd, manip_type_menu, 
  manip_type_menu_btn[5];
char *manip_type_menu_name[] = {"Oblique", "Vert", "Horiz", 
				  "Radial", "Angle"};
#define OBLIQUE_BTN    manip_type_menu_btn[0]
#define VERTICAL_BTN  manip_type_menu_btn[1]
#define HORIZONTAL_BTN manip_type_menu_btn[2]
#define RADIAL_BTN  manip_type_menu_btn[3]
#define ANGULAR_BTN  manip_type_menu_btn[4]
#define OBLIQUE_NAME    manip_type_menu_name[0]
#define VERTICAL_NAME  manip_type_menu_name[1]
#define HORIZONTAL_NAME manip_type_menu_name[2]
#define RADIAL_NAME  manip_type_menu_name[3]
#define ANGULAR_NAME  manip_type_menu_name[4]

/* For setting continuation factor */
Widget cont_fact_menu_label, cont_fact_menu_cmd, cont_fact_menu, 
  cont_fact_menu_btn[9];
char *cont_fact_menu_name[] = {"1/10", "1/5", "1/4", "1/3", "1/2", "1", 
				  "2", "10", "Infinite"};
#define TENTH_BTN cont_fact_menu_btn[0]
#define FIFTH_BTN cont_fact_menu_btn[1]
#define QUARTER_BTN cont_fact_menu_btn[2]
#define THIRD_BTN cont_fact_menu_btn[3]
#define HALF_BTN cont_fact_menu_btn[4]
#define ONE_BTN cont_fact_menu_btn[5]
#define TWO_BTN cont_fact_menu_btn[6]
#define TEN_BTN cont_fact_menu_btn[7]
#define INFINITE_BTN cont_fact_menu_btn[8]
#define TENTH_NAME cont_fact_menu_name[0]
#define FIFTH_NAME cont_fact_menu_name[1]
#define QUARTER_NAME cont_fact_menu_name[2]
#define THIRD_NAME cont_fact_menu_name[3]
#define HALF_NAME cont_fact_menu_name[4]
#define ONE_NAME cont_fact_menu_name[5]
#define TWO_NAME cont_fact_menu_name[6]
#define TEN_NAME cont_fact_menu_name[7]
#define INFINITE_NAME cont_fact_menu_name[8]

void
set_sens_link_menu(int sens)
{
  XtVaSetValues(link_menu_cmd,
    XtNsensitive, (Boolean) sens,
    NULL);
}

void
set_sens_tour_update(int sens)
{
  XtVaSetValues(tour_update_cmd,
    XtNsensitive, (Boolean) sens,
    NULL);
}

void
turn_off_local_scan(xgobidata *xg)
{
  XtCallCallbacks(LOCALSCAN, XtNcallback, (XtPointer) xg);
  XtVaSetValues(LOCALSCAN,
    XtNstate, (Boolean) False,
    NULL);
}

void
make_tour_io_menu(xgobidata *xg, Widget parent)
{
  int k;

  xg->tour_io_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "I/O",
    XtNmenuName, (String) "Menu",
    XtNfromVert, (Widget) HIST_ON,
    NULL);
  if (mono) set_mono(xg->tour_io_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    xg->tour_io_menu_cmd, "Tour_IOMenu");

  xg->tour_io_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass,
    xg->tour_io_menu_cmd,
    NULL);
  if (mono) set_mono(xg->tour_io_menu);

  xg->tour_io_menu_btn[0] = XtVaCreateWidget("Command",
    smeBSBObjectClass, xg->tour_io_menu,
    XtNlabel, (String) "Save Coeffs",
    NULL);
  if (mono) set_mono(xg->tour_io_menu_btn[0]);

  xg->tour_io_menu_btn[1] = XtVaCreateWidget("Command",
    smeBSBObjectClass, xg->tour_io_menu,
    XtNlabel, (String) "Save History",
    XtNsensitive, (Boolean) False,
    NULL);
  if (mono) set_mono(xg->tour_io_menu_btn[1]);

  xg->tour_io_menu_btn[2] = XtVaCreateWidget("Command",
    smeBSBObjectClass, xg->tour_io_menu,
    XtNlabel, (String) "Read History",
    NULL);
  if (mono) set_mono(xg->tour_io_menu_btn[2]);

  XtManageChildren(xg->tour_io_menu_btn, 3);

  for (k=0; k<3; k++)
  {
    XtAddCallback(xg->tour_io_menu_btn[k], XtNcallback,
      (XtCallbackProc) choose_tour_io_cback, (XtPointer) xg);
  }
}

void
reset_tour_link_menu(xgobidata *xg)
{
  char *label;

  if (xg->tour_link_state == send_state)
    label = link_menu_name[0];
  else if (xg->tour_link_state == receive)
    label = link_menu_name[1];
  else if (xg->tour_link_state == unlinked)
    label = link_menu_name[2];

  XtVaSetValues(link_menu_cmd, XtNlabel, label, NULL);
}

void
make_tour_link_menu(xgobidata *xg, Widget parent, Widget vref)
{
  Dimension width = 0, maxwidth = 0;
  int longest = 0;
  int k;

  link_menu_label = XtVaCreateManagedWidget("Label",
    labelWidgetClass, parent,
    XtNfromVert, vref,
    XtNlabel, "Link:",
    NULL);
  if (mono) set_mono(link_menu_label);

  for (k=0; k<3; k++)
  {
    width =
      XTextWidth(appdata.font, link_menu_name[k], strlen(link_menu_name[k]) +
      2*ASCII_TEXT_BORDER_WIDTH);
    if (width > maxwidth)
    {
      maxwidth = width;
      longest = k;
    }
  }

  /*
   * Initialize this widget to use the longest name in order
   * to ensure that it is wide enough.
  */
  link_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) link_menu_name[longest],
    XtNmenuName, (String) "Menu",
    XtNfromVert, vref,
    XtNfromHoriz, link_menu_label,
    XtNresize, False,
    NULL);
  if (mono) set_mono(link_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    link_menu_cmd, "Tour_Linking");
  reset_tour_link_menu(xg);

  link_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, link_menu_cmd,
    NULL);
  if (mono) set_mono(link_menu_cmd);

  SEND_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, link_menu,
    XtNlabel, SEND_NAME,
    NULL);
  if (mono) set_mono(SEND_BTN);
  XtAddCallback(SEND_BTN, XtNcallback,
    (XtCallbackProc) set_tour_link_state_cback, (XtPointer) xg);

  RECEIVE_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, link_menu,
    XtNlabel, RECEIVE_NAME,
    NULL);
  if (mono) set_mono(RECEIVE_BTN);
  XtAddCallback(RECEIVE_BTN, XtNcallback,
    (XtCallbackProc) set_tour_link_state_cback, (XtPointer) xg);

  UNLINK_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, link_menu,
    XtNlabel, UNLINK_NAME,
    NULL);
  if (mono) set_mono(UNLINK_BTN);
  XtAddCallback(UNLINK_BTN, XtNcallback,
    (XtCallbackProc) set_tour_link_state_cback, (XtPointer) xg);

  tour_update_cmd = CreateCommand(xg, "Send Tour Update",
    False, (Widget) NULL, (Widget) link_menu_label,
    parent, "Tour_SendUpdate");
  XtManageChild(tour_update_cmd);
  XtAddCallback(tour_update_cmd, XtNcallback,
    (XtCallbackProc) tour_update_cback, (XtPointer) xg);
}

void
make_tour_manip_type_menu(xgobidata *xg, Widget parent, Widget vref)
{
  Dimension width = 0, maxwidth = 0;
  int longest = 0;
  int k;

  manip_type_menu_label = XtVaCreateManagedWidget("Label",
    labelWidgetClass, parent,
    XtNfromVert, vref,
    XtNlabel, "Manip:",
    NULL);
  if (mono) set_mono(manip_type_menu_label);

  for (k=0; k<5; k++)
  {
    width =
      XTextWidth(appdata.font, manip_type_menu_name[k], 
        strlen(manip_type_menu_name[k]) + 2*ASCII_TEXT_BORDER_WIDTH);
    if (width > maxwidth)
    {
      maxwidth = width;
      longest = k;
    }
  }

  /*
   * Initialize this widget to use the longest name in order
   * to ensure that it is wide enough.
  */
  manip_type_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) manip_type_menu_name[longest],
    XtNmenuName, (String) "Menu",
    XtNfromVert, vref,
    XtNfromHoriz, manip_type_menu_label,
    XtNresize, False,
    NULL);
  if (mono) set_mono(manip_type_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    manip_type_menu_cmd, "Tour_Manip");

  manip_type_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, manip_type_menu_cmd,
    NULL);
  if (mono) set_mono(manip_type_menu);

  OBLIQUE_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, manip_type_menu,
    XtNlabel, OBLIQUE_NAME,
    NULL);
  if (mono) set_mono(OBLIQUE_BTN);
  XtAddCallback(OBLIQUE_BTN, XtNcallback,
    (XtCallbackProc) set_tour_manip_type_cback, (XtPointer) xg);

  VERTICAL_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, manip_type_menu,
    XtNlabel, VERTICAL_NAME,
    NULL);
  if (mono) set_mono(VERTICAL_BTN);
  XtAddCallback(VERTICAL_BTN, XtNcallback,
    (XtCallbackProc) set_tour_manip_type_cback, (XtPointer) xg);

  HORIZONTAL_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, manip_type_menu,
    XtNlabel, HORIZONTAL_NAME,
    NULL);
  if (mono) set_mono(HORIZONTAL_BTN);
  XtAddCallback(HORIZONTAL_BTN, XtNcallback,
    (XtCallbackProc) set_tour_manip_type_cback, (XtPointer) xg);

  RADIAL_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, manip_type_menu,
    XtNlabel, RADIAL_NAME,
    NULL);
  if (mono) set_mono(RADIAL_BTN);
  XtAddCallback(RADIAL_BTN, XtNcallback,
    (XtCallbackProc) set_tour_manip_type_cback, (XtPointer) xg);

  ANGULAR_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, manip_type_menu,
    XtNlabel, ANGULAR_NAME,
    NULL);
  if (mono) set_mono(ANGULAR_BTN);
  XtAddCallback(ANGULAR_BTN, XtNcallback,
    (XtCallbackProc) set_tour_manip_type_cback, (XtPointer) xg);

}

void
reset_tour_cont_fact_menu(xgobidata *xg)
{
  char *label;

  if (xg->tour_cont_fact == tenth)
    label = cont_fact_menu_name[0];
  else if (xg->tour_cont_fact == fifth)
    label = cont_fact_menu_name[1];
  else if (xg->tour_cont_fact == quarter)
    label = cont_fact_menu_name[2];
  else if (xg->tour_cont_fact == third)
    label = cont_fact_menu_name[3];
  else if (xg->tour_cont_fact == half)
    label = cont_fact_menu_name[4];
  else if (xg->tour_cont_fact == one)
    label = cont_fact_menu_name[5];
  else if (xg->tour_cont_fact == two)
    label = cont_fact_menu_name[6];
  else if (xg->tour_cont_fact == ten)
    label = cont_fact_menu_name[7];
  else if (xg->tour_cont_fact == infinite)
    label = cont_fact_menu_name[8];

  XtVaSetValues(cont_fact_menu_cmd, XtNlabel, label, NULL);
}

void
make_tour_cont_fact_menu(xgobidata *xg, Widget parent, Widget vref)
{
/*
  Dimension width = 0, maxwidth = 0;
  int k;
*/

  cont_fact_menu_label = XtVaCreateManagedWidget("Label",
    labelWidgetClass, parent,
    XtNfromVert, vref,
    XtNlabel, "Path Len:",
    NULL);
  if (mono) set_mono(cont_fact_menu_label);

/*
  for (k=0; k<9; k++)
  {
    width =
      XTextWidth(appdata.font, cont_fact_menu_name[k], 
        strlen(cont_fact_menu_name[k]) + 2*ASCII_TEXT_BORDER_WIDTH);
    if (width > maxwidth)
    {
      maxwidth = width;
    }
  }
*/

  /*
   * Initialize this widget to use the longest name in order
   * to ensure that it is wide enough.
  */
  cont_fact_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Infinite",
    XtNmenuName, (String) "Menu",
    XtNfromVert, vref,
    XtNfromHoriz, cont_fact_menu_label,
    XtNresize, False,
    NULL);
  if (mono) set_mono(cont_fact_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    cont_fact_menu_cmd, "Tour_PathLen");

  cont_fact_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass, cont_fact_menu_cmd,
    NULL);
  if (mono) set_mono(cont_fact_menu_cmd);

  TENTH_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cont_fact_menu,
    XtNlabel, TENTH_NAME,
    NULL);
  if (mono) set_mono(TENTH_BTN);
  XtAddCallback(TENTH_BTN, XtNcallback,
    (XtCallbackProc) set_tour_cont_fact_cback, (XtPointer) xg);

  FIFTH_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cont_fact_menu,
    XtNlabel, FIFTH_NAME,
    NULL);
  if (mono) set_mono(FIFTH_BTN);
  XtAddCallback(FIFTH_BTN, XtNcallback,
    (XtCallbackProc) set_tour_cont_fact_cback, (XtPointer) xg);

  QUARTER_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cont_fact_menu,
    XtNlabel, QUARTER_NAME,
    NULL);
  if (mono) set_mono(QUARTER_BTN);
  XtAddCallback(QUARTER_BTN, XtNcallback,
    (XtCallbackProc) set_tour_cont_fact_cback, (XtPointer) xg);

  THIRD_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cont_fact_menu,
    XtNlabel, THIRD_NAME,
    NULL);
  if (mono) set_mono(THIRD_BTN);
  XtAddCallback(THIRD_BTN, XtNcallback,
    (XtCallbackProc) set_tour_cont_fact_cback, (XtPointer) xg);

  HALF_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cont_fact_menu,
    XtNlabel, HALF_NAME,
    NULL);
  if (mono) set_mono(HALF_BTN);
  XtAddCallback(HALF_BTN, XtNcallback,
    (XtCallbackProc) set_tour_cont_fact_cback, (XtPointer) xg);

  ONE_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cont_fact_menu,
    XtNlabel, ONE_NAME,
    NULL);
  if (mono) set_mono(ONE_BTN);
  XtAddCallback(ONE_BTN, XtNcallback,
    (XtCallbackProc) set_tour_cont_fact_cback, (XtPointer) xg);

  TWO_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cont_fact_menu,
    XtNlabel, TWO_NAME,
    NULL);
  if (mono) set_mono(TWO_BTN);
  XtAddCallback(TWO_BTN, XtNcallback,
    (XtCallbackProc) set_tour_cont_fact_cback, (XtPointer) xg);

  TEN_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cont_fact_menu,
    XtNlabel, TEN_NAME,
    NULL);
  if (mono) set_mono(TEN_BTN);
  XtAddCallback(TEN_BTN, XtNcallback,
    (XtCallbackProc) set_tour_cont_fact_cback, (XtPointer) xg);

  INFINITE_BTN = XtVaCreateManagedWidget("Command",
    smeBSBObjectClass, cont_fact_menu,
    XtNlabel, INFINITE_NAME,
    NULL);
  if (mono) set_mono(INFINITE_BTN);
  XtAddCallback(INFINITE_BTN, XtNcallback,
    (XtCallbackProc) set_tour_cont_fact_cback, (XtPointer) xg);

  XtVaSetValues(cont_fact_menu_cmd, XtNlabel, "1", NULL);
}

void
init_tour_interp_menu(void)
{
  XtVaSetValues(tour_interp_menu_btn[0],
    XtNleftBitmap, menu_mark, NULL);
}

void
make_tour_interp_menu(xgobidata *xg, Widget parent)
/*
 * Build a menu to contain interpolation choices.
*/
{
  int j;

  tour_interp_menu_cmd = XtVaCreateManagedWidget("MenuButton",
    menuButtonWidgetClass, parent,
    XtNlabel, (String) "Interp",
    XtNmenuName, (String) "Menu",
    XtNfromHoriz, (Widget) xg->tour_io_menu_cmd,
    XtNfromVert, (Widget) HIST_ON,
    NULL);
  if (mono) set_mono(tour_interp_menu_cmd);
  add_menupb_help(&xg->nhelpids.menupb,
    tour_interp_menu_cmd, "Tour_InterpMenu");

  tour_interp_menu = XtVaCreatePopupShell("Menu",
    simpleMenuWidgetClass,
    tour_interp_menu_cmd, NULL);
  if (mono) set_mono(tour_interp_menu);

  tour_interp_menu_btn[0] = XtVaCreateWidget("Command",
    smeBSBObjectClass, tour_interp_menu,
    XtNlabel, (String) "Geodesic",
    XtNleftMargin, (Dimension) 24,
    NULL);
  if (mono) set_mono(tour_interp_menu_btn[0]);

  tour_interp_menu_btn[1] = XtVaCreateWidget("Command",
    smeBSBObjectClass, tour_interp_menu,
    XtNlabel, (String) "Householder",
    XtNleftMargin, (Dimension) 24,
    NULL);
  if (mono) set_mono(tour_interp_menu_btn[1]);

  tour_interp_menu_btn[2] = XtVaCreateWidget("Command",
    smeBSBObjectClass, tour_interp_menu,
    XtNlabel, (String) "Givens",
    XtNleftMargin, (Dimension) 24,
    NULL);
  if (mono) set_mono(tour_interp_menu_btn[2]);

  XtManageChildren(tour_interp_menu_btn, 3);

  for (j=0; j<3; j++)
    XtAddCallback(tour_interp_menu_btn[j], XtNcallback,
      (XtCallbackProc) choose_tour_interp_cback, (XtPointer) xg);
}

void
make_tour(xgobidata *xg)
{
  Dimension width;
  char str[30];
  Dimension max_width;
/*
 * Widest button label used in this panel.
*/
  sprintf(str, "ProjPrstSection");
  max_width = XTextWidth(appdata.font, str, strlen(str)) +
    4 * ASCII_TEXT_BORDER_WIDTH + 3 + 2;
  /* borders around text, spacing between widgets, widget borders */

/*
 * TourPanel
*/
  tour_panel = XtVaCreateManagedWidget("TourPanel",
    formWidgetClass, xg->box0,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNmappedWhenManaged, (Boolean) False,
    NULL);
  if (mono) set_mono(tour_panel);
/*
 * tour speed scrollbar
*/
  tour_sbar = XtVaCreateManagedWidget("Scrollbar",
    scrollbarWidgetClass, tour_panel,
    XtNwidth, (Dimension) max_width,
    XtNorientation, (XtOrientation) XtorientHorizontal,
    NULL);
  if (mono) set_mono(tour_sbar);

  add_sbar_help(&xg->nhelpids.sbar,
    tour_sbar, "Tour_Speed");

  XawScrollbarSetThumb(tour_sbar, 20.*TOURSTEP0 + .05, -1.);
  XtAddCallback(tour_sbar, XtNjumpProc,
    (XtCallbackProc) tour_speed_cback, (XtPointer) xg);

/*
 * tour pause control
*/
  PAUSE = CreateToggle(xg, "Pause",
    True, (Widget) NULL, tour_sbar, (Widget) NULL, False, ANY_OF_MANY,
    tour_panel, "Tour_Pause");
  XtAddCallback(PAUSE, XtNcallback,
    (XtCallbackProc) tour_pause_cback, (XtPointer) xg);

/*
 * Reinit button
*/
  REINIT = CreateCommand(xg, "Reinit",
    True, PAUSE, tour_sbar,
    tour_panel, "Tour_Reinit");
  XtAddCallback(REINIT, XtNcallback,
    (XtCallbackProc) tour_reinit_cback, (XtPointer) xg);

/*
 * Menu to manage linked touring
*/
  make_tour_link_menu(xg, tour_panel, REINIT);

/*
 * Menu to manage manipulation type 
*/ 
  make_tour_manip_type_menu(xg, tour_panel, tour_update_cmd);

/*
 * Menu to manage manipulation type 
*/ 
  make_tour_cont_fact_menu(xg, tour_panel, manip_type_menu_label);

/*
 * Button to allow stepping instead of continuous grand tour
*/
  STEP = CreateToggle(xg, "Step",
    True, (Widget) NULL, cont_fact_menu_label, (Widget) NULL, False, 
    ANY_OF_MANY, tour_panel, "Tour_StepGo");
  XtAddCallback(STEP, XtNcallback,
    (XtCallbackProc) tour_step_cback, (XtPointer) xg);
/* */

  GO = CreateCommand(xg, "Go",
    False, STEP, cont_fact_menu_label,
    tour_panel, "Tour_StepGo");
  XtAddCallback(GO, XtNcallback,
    (XtCallbackProc) tour_step_go_cback, (XtPointer) xg);

/*
 * Button for local scan
*/
  LOCALSCAN = CreateToggle(xg, "Local Scan",
    True, (Widget) NULL, STEP, (Widget) NULL, False, ANY_OF_MANY,
    /*True, (Widget) GO, cont_fact_menu_label, (Widget) NULL, False, ANY_OF_MANY,*/
    tour_panel, "Tour_LocalScan");
  XtAddCallback(LOCALSCAN, XtNcallback,
    (XtCallbackProc) tour_local_cback, (XtPointer) xg);

/*
 * tour backtrack control: initiate insensitive, turn on
 * when there are more than two or three elements in the
 * history file
*/
  BACKTRACK = CreateToggle(xg, "Backtrck",
    False, (Widget) NULL, LOCALSCAN, (Widget) NULL, False, ANY_OF_MANY,
    tour_panel, "Tour_Backtrack");
  XtAddCallback(BACKTRACK, XtNcallback,
    (XtCallbackProc) tour_backtrack_cback, (XtPointer) xg);

/*
 * label to record number of bases in history file
*/
  sprintf(str, "%d ", MAXHIST);
  width = XTextWidth(appdata.font, str, strlen(str));

  tour_label[0] = XtVaCreateManagedWidget("TourLabel",
    asciiTextWidgetClass, tour_panel,
    XtNfromVert, (Widget) LOCALSCAN,
    XtNfromHoriz, (Widget) BACKTRACK,
    XtNstring, (String) "1",
    XtNdisplayCaret, (Boolean) False,
    XtNwidth, (Dimension) width,
    NULL);
  if (mono) set_mono(tour_label[0]);

/*
 * Button to allow cycling backwards and forwards through backtracking
*/
  DIRECTION = CreateCommand(xg, "F",
    False, tour_label[0], LOCALSCAN,
    tour_panel, "Tour_BtrackDir");
  XtAddCallback(DIRECTION, XtNcallback,
    (XtCallbackProc) tour_cycleback_cback, (XtPointer) xg);

  XtManageChildren(&PAUSE, 7);

/*
 * Button for recording history
*/
  HIST_ON = CreateToggle(xg, "Hist On",
    True, (Widget) NULL, BACKTRACK, (Widget) NULL, True,
    ANY_OF_MANY,
    tour_panel, "Tour_HistOn");
  XtManageChild(HIST_ON);
  XtAddCallback(HIST_ON, XtNcallback,
    (XtCallbackProc) tourhist_on_cback, (XtPointer) xg);

/*
 * Store basis
*/
  STORBAS = CreateCommand(xg, "Store",
    True, HIST_ON, BACKTRACK,
    tour_panel, "Tour_Store");
  XtManageChild(STORBAS);
  XtAddCallback(STORBAS, XtNcallback,
    (XtCallbackProc) tour_storbas_cback, (XtPointer) xg);

  make_tour_io_menu(xg, tour_panel );

  make_tour_interp_menu(xg, tour_panel);

/*
 * Button for switching to principal component axes
*/
  xg->princ_comp_cmd = CreateToggle(xg, "PC Basis",
    True, (Widget) NULL, xg->tour_io_menu_cmd, (Widget) NULL, False,
    ANY_OF_MANY,
    tour_panel, "Tour_PrinComp");
  XtManageChild(xg->princ_comp_cmd);
  XtAddCallback(xg->princ_comp_cmd, XtNcallback,
    (XtCallbackProc) princ_comp_cback, (XtPointer) xg);

/*
 * Button for switching between axes in raw data space or pc space
*/
  xg->pc_axes_cmd = CreateToggle(xg, "PC Axes",
    False, (Widget) xg->princ_comp_cmd, xg->tour_io_menu_cmd, 
    (Widget) NULL, False, ANY_OF_MANY,
    tour_panel, "Tour_PC_Axes");
  XtManageChild(xg->pc_axes_cmd);
  XtAddCallback(xg->pc_axes_cmd, XtNcallback,
    (XtCallbackProc) pc_axes_cback, (XtPointer) xg);

/*
 * tour projection pursuit control
*/
  xg->proj_pursuit_cmd = CreateToggle(xg, "ProjPrst",
    True, (Widget) NULL, (Widget) xg->princ_comp_cmd,
    (Widget) NULL, False, ANY_OF_MANY,
    tour_panel, "ToPP");
  XtManageChild(xg->proj_pursuit_cmd);
  XtAddCallback(xg->proj_pursuit_cmd, XtNcallback,
    (XtCallbackProc) tour_pp_cback, (XtPointer) xg);

/*
 * Button for section tour
*/
  xg->tour_section_cmd = CreateToggle(xg, "Section",
    True, (Widget) xg->proj_pursuit_cmd, (Widget)  xg->princ_comp_cmd,
    (Widget) xg->proj_pursuit_cmd, False, ANY_OF_MANY,
    tour_panel, "Tour_Section");
  XtManageChild(xg->tour_section_cmd);
  XtAddCallback(xg->tour_section_cmd, XtNcallback,
    (XtCallbackProc) tour_section_cback, (XtPointer) xg);

  make_pp_panel(xg, tour_panel);
  make_section_panel(xg, tour_panel, max_width);
}

void
highlight_pause_cmd()
{
  XtVaSetValues(PAUSE, XtNstate, (Boolean) True, NULL);
}

void
reset_tour_pause_cmd(xgobidata *xg)
{
  XtCallCallbacks(PAUSE, XtNcallback, (XtPointer) xg);
  XtVaSetValues(PAUSE, XtNstate, xg->is_tour_paused, NULL);
}

void
reset_cycleback_cmd(Boolean set, Boolean sens, char *label)
{
  if (set)
    XtVaSetValues(DIRECTION,
      XtNsensitive, sens,
      XtNlabel, (String) label,
      NULL);
  else
    XtVaSetValues(DIRECTION,
      XtNlabel, (String) label,
      NULL);
}

void
set_sens_localscan(Boolean sens)
{
  XtVaSetValues(LOCALSCAN,
    XtNsensitive, sens,
    NULL);
}

void
set_sens_reinit(Boolean sens)
{
  XtVaSetValues(REINIT, XtNsensitive, sens, NULL);
}

void
set_sens_go(Boolean sens)
{
  XtVaSetValues(GO, XtNsensitive, sens, NULL);
}

void
turn_off_stepping(void)
{
  XtCallCallbacks(STEP, XtNcallback, (XtPointer) NULL);
  XtVaSetValues(STEP, XtNstate, False, NULL);

}

void
set_sens_step(Boolean sens)
{
  XtVaSetValues(STEP, XtNsensitive, sens, NULL);
}

void
set_sens_speed(Boolean sens)
{
  XtVaSetValues(tour_sbar, XtNsensitive, sens, NULL);
}

void
set_sens_interp(Boolean sens)
{
  XtVaSetValues(tour_interp_menu_cmd, XtNsensitive, sens, NULL);
}

void
set_sens_io(xgobidata *xg, int sens0, int sens1, int sens2)
{
  XtVaSetValues(xg->tour_io_menu_btn[0],
    XtNsensitive, (Boolean) sens0,
    NULL);
  XtVaSetValues(xg->tour_io_menu_btn[1],
    XtNsensitive, (Boolean) sens1,
    NULL);
  XtVaSetValues(xg->tour_io_menu_btn[2],
    XtNsensitive, (Boolean) sens2,
    NULL);
}

void
highlight_backtrack_cmd()
{
  XtVaSetValues(BACKTRACK, XtNstate, (Boolean) False, NULL);
}

void
reset_backtrack_cmd(Boolean reset, Boolean set, Boolean sens, Boolean activ)
{
  if (reset)
    XtVaSetValues(BACKTRACK,
      XtNstate, set,
      XtNsensitive, sens,
      NULL);
  else
    XtVaSetValues(BACKTRACK,
      XtNsensitive, sens,
      NULL);

  if (activ)
    XtCallCallbacks(BACKTRACK, XtNcallback, (XtPointer) NULL);
}

void
set_sens_direction(Boolean sens)
{
  XtVaSetValues(DIRECTION,
    XtNsensitive, sens,
    NULL);
}

void
reset_tourhist_cmds(xgobidata *xg, int sens)
{
  XtVaSetValues(xg->tour_io_menu_btn[1],
    XtNsensitive, (Boolean) sens,
    NULL);

  XtVaSetValues(xg->tour_io_menu_btn[2],
    XtNsensitive, (Boolean) sens,
    NULL);
}

void
reset_interp_cmd(int sens)
{
  XtVaSetValues(tour_interp_menu_cmd,
    XtNsensitive, (Boolean) sens,
    NULL);
}

void
map_tour_panel(xgobidata *xg, Boolean is_tour_on)
{
  if (is_tour_on)
  {
    XtMapWidget(tour_panel);
    XtMapWidget(xg->tour_mouse);
   }
  else
  {
    XtUnmapWidget(tour_panel);
    XtUnmapWidget(xg->tour_mouse);
  }
}

void
nback_update_label(xgobidata *xg)
{
  char str[10];

  (void) sprintf(str, "%d", xg->nhist_list);
  XtVaSetValues(tour_label[0],
    XtNstring, (String) str,
    NULL);
}

void
reinit_tour(xgobidata *xg)
{
  XtCallCallbacks(REINIT, XtNcallback, (XtPointer) xg);
}

#undef PANEL1
#undef PANEL2
#undef PANEL3

#undef PAUSE
#undef STEP
#undef GO
#undef LOCALSCAN
#undef BACKTRACK
#undef DIRECTION
#undef REINIT
#undef HIST_ON
#undef STORBAS

#undef OBLIQUE_BTN
#undef VERTICAL_BTN
#undef HORIZONTAL_BTN
#undef RADIAL_BTN
#undef ANGULAR_BTN
#undef OBLIQUE_NAME
#undef VERTICAL_NAME
#undef HORIZONTAL_NAME
#undef RADIAL_NAME
#undef ANGULAR_NAME

#undef TENTH_BTN
#undef FIFTH_BTN
#undef QUARTER_BTN
#undef THIRD_BTN
#undef HALF_BTN
#undef ONE_BTN
#undef TWO_BTN
#undef TEN_BTN
#undef INFINITE_BTN
#undef TENTH_NAME
#undef FIFTH_NAME
#undef QUARTER_NAME
#undef THIRD_NAME
#undef HALF_NAME
#undef ONE_NAME
#undef TWO_NAME
#undef TEN_NAME
#undef INFINITE_NAME
