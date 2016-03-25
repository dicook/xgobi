#include <sys/types.h>
#include <stdio.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include "xgvis.h"

/*
 * Plot the stress function.
*/

#define MAXSTRVALUES 1000
#define STR_VMARGIN 10
#define STR_HMARGIN 10
Widget stress_form;
Widget str_wksp;
Pixmap str_pixmap = (Pixmap) NULL;
Drawable str_window;
int nstrValues = 0;
int maxnstrValues = MAXSTRVALUES;
double strValues[MAXSTRVALUES];
Dimension str_height = 120;
Dimension str_width = 295;
double default_stress = -1;  /* stress before running mds_once */

extern void mds_once(Boolean);

extern Dimension runPanelWidth, mdsPanelWidth;


void
reinit_stress(void)
{
  /* nstrValues = 0; */
  mds_once(False);
}

void
make_str_pixmap(void) {
  str_pixmap = XCreatePixmap(display, str_window,
			     str_width, str_height, depth);
}

void
clear_str_pixmap(void) {
  XFillRectangle(display, str_pixmap, clear_GC,
    0, 0, str_width, str_height);
}

void
copy_str_pixmap(void) {
  /* copy the pixmap to the screen */
  XCopyArea(display, str_pixmap, str_window, copy_GC,
    0, 0, str_width, str_height, 0, 0 );
}

void
add_stress_value(double stress)
{
  int i;

  if (nstrValues == maxnstrValues) {
    for (i=0; i < (maxnstrValues-1); i++) {
      strValues[i] = strValues[i+1];
    }
    nstrValues--;
  }

  strValues[nstrValues] = stress;
  nstrValues++;
}

void
draw_stress(void) {
  int i, j, npoints, start, end;
  /*float width = (float) (str_width) - 2. * (float) STR_HMARGIN;*/
  /*float maxwidth = (float) MAXSTRVALUES;*/
  float height = (float) (str_height) - 2. * (float) STR_VMARGIN;
  float x, y;
  static Boolean initd = False;
  XPoint axes[3];
  char str[32];
  static int strwidth;
  XPoint strPts[MAXSTRVALUES];

  if (!initd) {
    str_window = XtWindow(str_wksp);
    make_str_pixmap();

    sprintf(str, ".9999");
    strwidth = XTextWidth(appdata.plotFont, str, strlen(str));

    initd = True;
  }

  /*
   * The starting point should be zero until npoints has
   * been surpassed; after that, it should be nstrValues
   * minus npoints.  Define the indices for strPts.
  */
  /* plotting one point per pixel ... */
  npoints = MIN(str_width - 2*STR_HMARGIN, nstrValues);
  start = MAX(0, nstrValues - npoints);
  end = nstrValues;

  for (i=start, j=0; i<end; i++, j++) {
    x = (float) j ;
    strPts[j].x = (int) (x + STR_HMARGIN);
    y = (float) (1 - strValues[i]) * height;
    strPts[j].y = (int) (y + STR_VMARGIN);
  }

  /* axes */
  axes[0].x = STR_HMARGIN;
  axes[0].y = STR_VMARGIN;
  axes[1].x = STR_HMARGIN;
  axes[1].y = str_height - STR_VMARGIN;
  axes[2].x = str_width - STR_HMARGIN;
  axes[2].y = str_height - STR_VMARGIN;

  /* stress as a fraction */
  sprintf(str, "%2.4f", strValues[nstrValues-1]);

  XSetForeground(display, copy_GC, plotcolors.fg);
  clear_str_pixmap();
  XDrawLines(display, str_pixmap, copy_GC,
    axes, 3, CoordModeOrigin);
  if (nstrValues) {
    XDrawString(display, str_pixmap, copy_GC,
      str_width - 2*STR_HMARGIN - strwidth,
      FONTHEIGHT(appdata.plotFont),
      str,
      strlen(str));
    XSetLineAttributes(display, copy_GC, 2, LineSolid,
      CapRound, JoinBevel);
    XDrawLines(display, str_pixmap, copy_GC,
      strPts, npoints, CoordModeOrigin);
    XSetLineAttributes(display, copy_GC, 1, LineSolid,
      CapRound, JoinBevel);
  }
  copy_str_pixmap();
}

/* ARGSUSED */
XtCallbackProc
str_expose_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  if (str_pixmap)
    copy_str_pixmap();
}

/* ARGSUSED */
XtCallbackProc
str_resize_cback(Widget w, XtPointer client_data, XtPointer callback_data)
{
  XtVaGetValues(str_wksp,
    XtNwidth, &str_width,
    XtNheight, &str_height,
    NULL);

  XFreePixmap(display, str_pixmap);
  make_str_pixmap();
  clear_str_pixmap();
  draw_stress();
}

void
build_stress_plotwin(Widget parent, Widget href, Widget vref)
{
  Widget str_label;

  stress_form = XtVaCreateManagedWidget("Stress",
    formWidgetClass, parent,
    XtNfromHoriz, href,
    XtNfromVert, vref,
    NULL);
  if (mono) set_mono(stress_form);

  str_width = runPanelWidth + mdsPanelWidth + 15;

  str_label = XtVaCreateManagedWidget("Label",
    labelWidgetClass, stress_form,
    XtNlabel, "Stress function",
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNright, (XtEdgeType) XtChainRight,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNbottom, (XtEdgeType) XtChainTop,
    XtNwidth, str_width,
    XtNborderWidth, 0,
    NULL); 
  if (mono) set_mono(str_label);

  str_wksp = XtVaCreateManagedWidget("Stress",
    labelWidgetClass, stress_form,
    XtNfromVert, str_label,
    XtNresizable, (Boolean) True,
    XtNleft, (XtEdgeType) XtChainLeft,
    XtNtop, (XtEdgeType) XtChainTop,
    XtNright, (XtEdgeType) XtChainRight,
    XtNbottom, (XtEdgeType) XtChainBottom,
    XtNheight, str_height,
    XtNwidth, str_width,
    XtNlabel, (String) "",
    NULL);
  if (mono) set_mono(str_wksp);

  XtAddEventHandler(str_wksp, ExposureMask,
    FALSE, (XtEventHandler) str_expose_cback, (XtPointer) NULL);
  XtAddEventHandler(str_wksp, StructureNotifyMask,
    FALSE, (XtEventHandler) str_resize_cback, (XtPointer) NULL);
}

