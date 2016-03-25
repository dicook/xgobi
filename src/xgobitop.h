/* xgobitop.h */
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

/* ARGSUSED */
XtActionProc
SetCurrentWindow(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  xgobi.current_window = 1;
  /* CORBA */
  return (NULL);
}

/* ARGSUSED */
XtActionProc
UnsetCurrentWindow(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  XCrossingEvent *evnt = (XCrossingEvent *) event;

  if (evnt->detail != NotifyInferior)
    xgobi.current_window = 0;
  /* CORBA */
  return (NULL);
}

/* to make xgobi work with motif */
/* ARGSUSED */
XtActionProc
WM_Quit(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  if (w == xgobi.shell)
  {
    xgobi.is_realized = False;
    XtDestroyApplicationContext(XtWidgetToApplicationContext(w));
    exit(0);
  }
  else
    XtPopdown(w);
  /* CORBA */
  return (NULL);
}

/*ARGSUSED*/
XtActionProc
TogglePause(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;

  if (xg->is_spinning) 
    reset_spin_pause_cmd(xg);
  else if (xg->is_touring) 
    reset_tour_pause_cmd(xg);
  else if (xg->is_corr_touring) 
    reset_corr_pause_cmd(xg);
  else if (xg->is_xyplotting) {
    extern Widget xy_cycle_cmd;
    XtCallCallbacks(xy_cycle_cmd, XtNcallback, (XtPointer) xg);
    XtVaSetValues(xy_cycle_cmd, XtNstate, xg->is_xy_cycle, NULL);
  } else if (xg->is_plotting1d) {
    extern Widget plot1d_cycle_cmd;
    XtCallCallbacks(plot1d_cycle_cmd, XtNcallback, (XtPointer) xg);
    XtVaSetValues(plot1d_cycle_cmd, XtNstate, xg->is_plot1d_cycle, NULL);
  }
  /* CORBA */
  return (NULL);
}

/*ARGSUSED*/
static XtActionProc
RepeatAction(Widget w, XEvent *event, String *params, Cardinal nparams)
{
  extern void permute_again(xgobidata *);
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;

  permute_again(xg);

  /* Maybe with an argument? */
  /* jitter_again(xg); */
  /* CORBA */
  return (NULL);
}
/*ARGSUSED*/
static XtActionProc
RestoreAction(Widget w, XEvent *event, String *params, Cardinal nparams)
{
/*
 * This is intended to restore a variable to its untransformed state.
 * It's a bit more complicated than RepeatAction, because it must also
 * affect menus.  The same sort of thing could happen with jittering.
*/
  extern void restore_variables(xgobidata *);
  extern xgobidata xgobi;
  xgobidata *xg = &xgobi;

  restore_variables(xg);
  /* CORBA */
  return (NULL);
}

XtActionProc HelpSelect(), Bell();
XtActionProc ShowTarget(), DontShowTarget();
XtActionProc NullNearestPoint();
XtActionProc SetPlotMode(), KeyFileMenu(), KeyDisplayMenu(), KeyToolMenu();
XtActionProc StartScaleByArrow(), StopScaleByArrow();
/* For variable and case lists */
/* XtActionProc GetListSelection(); */
XtActionProc ToggleCurrentLine();
XtActionsRec added_actions[] = {
  {"HelpSelect", (XtActionProc) HelpSelect},
  {"Bell", (XtActionProc) Bell},
  {"SetCurrentWindow", (XtActionProc) SetCurrentWindow},
  {"UnsetCurrentWindow", (XtActionProc) UnsetCurrentWindow},
  {"ShowTarget", (XtActionProc) ShowTarget},
  {"DontShowTarget", (XtActionProc) DontShowTarget},
  {"NullNearestPoint", (XtActionProc) NullNearestPoint},
/* For variable and case lists */
  /* {"GetListSelection", (XtActionProc) GetListSelection}, */
  {"ToggleCurrentLine", (XtActionProc) ToggleCurrentLine},
/* to make xgobi work with motif */
  {"wm_quit", (XtActionProc) WM_Quit}, 
/* view menu */
  {"SetPlotMode", (XtActionProc) SetPlotMode},
/* file menu */
  {"KeyFileMenu", (XtActionProc) KeyFileMenu},
/* display and tool menus */
  {"KeyDisplayMenu", (XtActionProc) KeyDisplayMenu},
  {"KeyToolMenu", (XtActionProc) KeyToolMenu},
/* toggle pause in all rotation modes; toggle cycle in xyplot and plot1d */
  {"TogglePause", (XtActionProc) TogglePause},
/* to allow certain actions to be repeated with the "." key */
  {"RepeatAction", (XtActionProc) RepeatAction},
/* to allow certain restore actions to be performed with the "," key */
  {"RestoreAction", (XtActionProc) RestoreAction},
/* scale using the arrow keys */
  {"StartScaleByArrow", (XtActionProc) StartScaleByArrow},
  {"StopScaleByArrow", (XtActionProc) StopScaleByArrow},
};

String
fallback_resources[] = {
  "*XGobi.title:    XGobi",
  "*XGobi.iconName: XGobi",
  "*font:          -*-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*",
  "*plotFont:      -*-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*",
  "*helpFont:      -*-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*",

  /* for Motif and LessTif */
  "*fontList:      -*-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*",

  "*showAxes:      True",
  "*showPoints:    True",
  "*showLines:     False",
  "*linkGlyphBrush: True",
  "*linkColorBrush: True",
  "*linkEraseBrush: True",
  "*linkLineBrush: True",
  "*linkIdentify:  True",
  "*carryVars:     False",
  "*jumpBrush:     True",
  "*reshapeBrush:  False",
  "*plotSquare:    True",
  "*glyphType:     6",
  "*glyphSize:     1",

  "*defaultGlyph:  26",
  "*defaultColor:  white",
  "*axisColor:     lightgray",

  "*brushColor0:   DeepPink",
  "*brushColor1:   OrangeRed1",
  "*brushColor2:   DarkOrange",
  "*brushColor3:   Gold",
  "*brushColor4:   Yellow",
  "*brushColor5:   DeepSkyBlue1",
  "*brushColor6:   SlateBlue1",
  "*brushColor7:   YellowGreen",
  "*brushColor8:   MediumSpringGreen",
  "*brushColor9:   MediumOrchid",
  "*defaultPrintCmd: lp",

  "*XGobi*horizDistance: 5",
  "*XGobi*vertDistance: 5",
  "*XGobi*Options*vertDistance: 2",

  "*XGobi*Form1.gripIndent: 5",
  "*XGobi*MainPanel.defaultDistance: 2",

  "*XGobi*PlotWindow.height: 300",
  "*XGobi*PlotWindow.width: 300",

  "*XGobi*VarPanel.hSpace: 1",
  "*XGobi*VarPanel.vSpace: 1",
  "*XGobi*VarPanel.borderWidth: 0",
  "*XGobi*VarPanel.width: 180",
  "*XGobi*VarPanel.top: ChainTop",
  "*XGobi*VarPanel.left: ChainLeft",
  "*XGobi*VarPanel.right: ChainRight",
  "*XGobi*VarPanel.bottom: ChainBottom",

  "*XGobi*VarForm.borderWidth: 0",
  "*XGobi*VarForm.hSpace: 0",
  "*XGobi*VarForm.vSpace: 0",
  "*XGobi*VarWindow.borderWidth: 0",
  "*XGobi*VarWindow.width: 44",
  "*XGobi*VarWindow.vertDistance: 0",

  "*XGobi*TourPanel*hSpace: 3",
  "*XGobi*TourPanel*vSpace: 3",
  "*XGobi*TourPanel*vertDistance: 3",
  "*XGobi*TourPanel*horizDistance: 3",
  "*XGobi*TourPanel*Panel.Scrollbar.vertDistance: 0",

  "*XGobi*PPplot.height:  180",
  "*XGobi*PPplot.width:   500",

  "*XGobi*ScaleShiftPanel*hSpace: 3",
  "*XGobi*ScaleShiftPanel*vSpace: 3",
  "*XGobi*ScaleShiftPanel*Icon.horizDistance: 0",
  "*XGobi*ScaleShiftPanel*Icon.vertDistance: 0",

  "*XGobi*StdizePanel*hSpace: 3",
  "*XGobi*StdizePanel*vSpace: 3",
  "*XGobi*StdizePanel*horizDistance: 3",
  "*XGobi*StdizePanel*vertDistance: 3",

  "*XGobi*BrushPanel.Panel.hSpace: 2",
  "*XGobi*BrushPanel.Panel.vSpace: 2",

  "*XGobi*FSavePopup*FSaveName.height: 25",
  "*XGobi*FSavePopup*FSaveName.width: 265",
  "*XGobi*FSavePopup*FSaveText.height: 25",

  "*XGobi*Help*Text.height: 250",
  "*XGobi*Help*Text.width: 600",

  "*XGobi*MouseLabel.left: ChainLeft",
  "*XGobi*MouseLabel.right: ChainLeft",
  "*XGobi*MouseLabel.top: ChainTop",
  "*XGobi*MouseLabel.bottom: ChainTop",
  "*XGobi*MouseLabel.mappedWhenManaged: False",
  "*XGobi*VarMouseLabel.left: ChainLeft",
  "*XGobi*VarMouseLabel.right: ChainLeft",
  "*XGobi*VarMouseLabel.top: ChainTop",
  "*XGobi*VarMouseLabel.bottom: ChainTop",

  /* An alternate method of programming accelerators */
  "*XGobi*Form0.accelerators: #augment \\n\
    <Key>p: TogglePause()",

/* "." : permutation with a keystroke */
/* " " : restore variable transformation */
/* It doesn't work inside the variable circles, I think
 * because there's a KeyPressMask in use there.
*/
  "*XGobi*VarPanel.accelerators: #augment \\n\
    <Key>.: RepeatAction() \\n\
    <Key>Return: RestoreAction()",

  /* Translations */
  "*XGobi*Form0.Translations: #augment \\n\
    <EnterWindow>:  SetCurrentWindow() \\n\
    <LeaveWindow>:  UnsetCurrentWindow()",

  "*XGobi*VarPanel.Translations: #augment \\n\
    <EnterWindow>:  ShowTarget() \\n\
    <LeaveWindow>:  DontShowTarget()",

  "*XGobi*Scrollbar.Translations: #override \\n\
    <Btn1Down>:   StartScroll(Continuous) MoveThumb() NotifyThumb() \\n\
    <Btn1Motion>: MoveThumb() NotifyThumb() \\n\
    <Btn3Down>:   HelpSelect()",

  "*XGobi*List.Translations: #augment \\n\
    <EnterWindow>:  SetCurrentWindow() \\n\
    <LeaveWindow>:  UnsetCurrentWindow()",
  "*XGobi*List*Text.Translations: #replace \\n\
    <Btn1Down>:     select-start() ToggleCurrentLine(p) \\n\
    <Btn1Motion>:   select-start() ToggleCurrentLine(m)",

  "*XGobi*PrintText.Translations: #override <Key>Return: Bell()",
  "*XGobi*MissingText.Translations: #override <Key>Return: Bell()",

  "*XGobi*Toggle.Translations: #override \\n\
    <EnterWindow>:         highlight(Always) \\n\
    <LeaveWindow>:         unhighlight() \\n\
    <Btn1Down>, <Btn1Up>:  set()notify() \\n\
    <Btn3Down>:            HelpSelect()",

  "*XGobi*Command.Translations: #override <Btn3Down>: HelpSelect()",
  "*XGobi*MenuButton.Translations: #override <Btn3Down>: HelpSelect()",
  "*XGobi*PlotWindow.Translations: #override \\n\
     <LeaveWindow>:  NullNearestPoint() \\n\
     <Btn3Down>:     HelpSelect()",

  "*XGobi*VarLabel.Translations: #override <Btn3Down>: HelpSelect()",
  "*XGobi*VarWindow.Translations: #override <Btn3Down>: HelpSelect()",
  "*XGobi*Icon.Translations: #override <Btn3Down>: HelpSelect()",

  /* Menu Icon */
  "*XGobi*MenuButton.leftBitmap: menu12",
  /* "*XGobi*MenuButton.leftBitmap: ../bitmaps/menu12", */

  /* Colors */

  "*XGobi*background: Moccasin",
  "*XGobi*foreground: NavyBlue",
  "*pointerColor: White",
  "*XGobi*MainPanel.background: DarkKhaki",
  "*XGobi*PlotWindow.background: blue4",
  "*XGobi*PlotWindow.foreground: white",
  "*XGobi*Command.background: SandyBrown",
  "*XGobi*Command.borderColor: NavyBlue",
  "*XGobi*Command.foreground: NavyBlue",
  "*XGobi*Toggle.background: SandyBrown",
  "*XGobi*Toggle.borderColor: NavyBlue",
  "*XGobi*Toggle.foreground: NavyBlue",
  "*XGobi*MenuButton.background: SandyBrown",
  "*XGobi*MenuButton.borderColor: NavyBlue",
  "*XGobi*MenuButton.foreground: NavyBlue",
  "*XGobi*Scrollbar.background: SandyBrown",
  "*XGobi*Scrollbar.borderColor: NavyBlue",
  "*XGobi*Scrollbar.foreground: NavyBlue",
  /*
   * This is ignored; the arrows are created with certain
   * color characteristics.  See make_arrows() in widgets.c.
  "*XGobi*Arrow.background: SandyBrown",
  "*XGobi*Arrow.borderColor: NavyBlue",
  */

  "*XGobi*PPplot.background: Blue4",
  "*XGobi*PPplot.foreground: white",

  "*XGobi*BrushPanel.background: PeachPuff",
  "*XGobi*BrushPanel.Panel.background: DarkKhaki",
  "*XGobi*BrushPanel*Menu*background: NavyBlue",
  "*XGobi*BrushPanel*Menu*borderColor: White",
  "*XGobi*BrushPanel*Menu*foreground: White",
  "*XGobi*Help*background: Gray85",
  "*XGobi*Help*foreground: NavyBlue",
  "*XGobi*Help*Done.background: Moccasin",

  "*XGobi*IdentifyPanel.Panel.background: DarkKhaki",

  "*XGobi*ScalePanel.background: DarkKhaki",
  "*XGobi*ShiftPanel.background: DarkKhaki",
  "*XGobi*StdizePanel.background: DarkKhaki",

  "*XGobi*Subset*Param.background: DarkKhaki",

  "*XGobi*SpinPanel.Panel.background: DarkKhaki",
  "*XGobi*SpinPanel.background: PeachPuff",
  "*XGobi*TourPanel*Menu*background: DarkKhaki",
  "*XGobi*TourPanel*Menu*foreground: NavyBlue",
  "*XGobi*TourPanel*Panel.background: DarkKhaki",
  "*XGobi*TourPanel*TourLabel.background: PeachPuff",
  "*XGobi*TourPanel.background: PeachPuff",
  "*XGobi*XYPlotPanel.background: PeachPuff",

  /*"*XGobi*VarPanel.foreground: DarkKhaki",*/
  "*XGobi*VarPanel*VarLabel.background: PeachPuff",
  "*XGobi*VarPanel*VarLabel.foreground: NavyBlue",

  "*XGobi*MouseLabel.background: Salmon",
  "*XGobi*MouseLabel.foreground: NavyBlue",
  "*XGobi*VarMouseLabel.background: Salmon",
  "*XGobi*VarMouseLabel.foreground: NavyBlue",

  "*XGobi*ParCoords.Panel.background: DarkKhaki",

  /* For Juergen */
  "*isCloned:  False",
  "*clonePID:  0",
  "*cloneTime: 0",
  "*cloneType: 0",
  "*cloneName: XGobi",
  "*deleteCloneData: True",

  NULL
};

void
XGobiMainLoop(xgobidata *xg)
{
  XEvent event;
  XEvent ahead;
  Drawable root_window = DefaultRootWindow(display);
  Boolean RunWorkProcs(void *);

/*
 * Set up to receive Property Change events.
*/
  XSelectInput(display, DefaultRootWindow(display), PropertyChangeMask);

/* ? dfs */
  xgobi.current_window = 0;

  while (1)
  {
    XtAppNextEvent(app_con, &event);
    /*printf("event.type = %d\n", event.type);*/

    if (event.type == MotionNotify &&
        event.xmotion.window == xg->plot_window)
    {
      /* Compress motion events */
      while (XEventsQueued(display, QueuedAfterFlush) > 0)
      {
        XPeekEvent(display, &ahead);
        if (ahead.type == MotionNotify &&
            ahead.xmotion.window == xg->plot_window)
        {
          XtAppNextEvent(app_con, &event);
        }
        else if ( (ahead.type == PropertyNotify ||
                   ahead.type == NoExpose) &&
                   ahead.xany.window == xg->plot_window )
        {
          XtAppNextEvent(app_con, &ahead);
        }
        else
          break;
      }
      XtDispatchEvent(&event);
    }

    else if (event.type == PropertyNotify &&
             event.xproperty.window == root_window)
    {
      if (event.xproperty.atom == XG_NEWTOUR_ANNC)
      {
        if (xg->is_touring &&
            xg->tour_link_state == receive &&
            !xg->is_tour_paused &&
            !xg->is_scaling && !xg->is_brushing &&
            !xg->is_line_editing && !xg->is_identify)
        {
          read_tour_coefs(xg);
        }
      }
      else
      {
        /*fprintf(stderr, "property notify event\n");*/

        if (!xg->current_window &&
           ( (event.xproperty.atom == XG_ROWSINPLOT_ANNC &&
              xg->link_points_to_points) || /* points-to-lines handled here? */
             (event.xproperty.atom == XG_NEWPAINT_ANNC &&
              xg->link_points_to_points || xg->link_points_to_lines) ||
             (event.xproperty.atom == XG_NEWLINEPAINT_ANNC &&
              xg->link_lines_to_lines || xg->link_points_to_lines) ||
             (event.xproperty.atom == XG_IDS_ANNC &&
              xg->link_identify) ||
             (event.xproperty.atom == XG_ERASE_ANNC && xg->link_points_to_points)
          )
          )
        {
           /* Compress PropertyNotify events */
/*
 * Danger:  if line brushing and point brushing at the same
 * time, will some important events be thrown away?
*/
          while (XEventsQueued(display, QueuedAfterFlush) > 0)
          {
            XPeekEvent(display, &ahead);
            if (ahead.type == PropertyNotify && 
                ahead.xproperty.window == event.xproperty.window &&
                ahead.xproperty.atom == event.xproperty.atom)
            {
              XtAppNextEvent(app_con, &event);
            }
            else
              break;
          }
        
          if (event.xproperty.atom == XG_ROWSINPLOT_ANNC)
            read_rows_in_plot(xg);
          else if (event.xproperty.atom == XG_NEWPAINT_ANNC)
            read_paint(xg);
          else if (event.xproperty.atom == XG_NEWLINEPAINT_ANNC)
            read_line_paint(xg);
          else if (event.xproperty.atom == XG_IDS_ANNC)
            read_ids(xg) ;
        }
      }
    }
    /*
     * This section is to turn off identification when xgobi is
     * iconified.
    */
      /* Shut off the work proc if the main window is iconified */
    else if (event.type == UnmapNotify &&
             event.xany.window == XtWindow(xgobi.shell))
    {
      xgobi.is_iconified = True;
    }
    /*
     * Restart the work proc if deiconified.  If there shouldn't
     * be a work proc running, it will turn itself off after one
     * run-through
    */
    else if (event.type == MapNotify)
    {
      xgobi.is_iconified = False;
      XtAppAddWorkProc(app_con, RunWorkProcs, (XtPointer) NULL);
    }
    else if (event.xany.window == XtWindow(xgobi.shell) &&
             event.type == DestroyNotify)
    {
      XtDestroyApplicationContext(app_con);
      exit(0);
    }

    else
      XtDispatchEvent(&event);
  }
}


