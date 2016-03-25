/************************************************************************/
/*                                                                      */
/* File "av2xgobi.h"                                                    */
/*                                                                      */
/* Author : Juergen Symanzik                                            */
/* Date   : 01/08/96                                                    */
/*                                                                      */
/************************************************************************/

#ifndef PROGINTERN
#define PROG_ extern
#else
#define PROG_
#endif

PROG_ xgobidata xgobi;
PROG_ Widget prog_shell;

/*
 * You might want to be able to define some resources
 */
typedef struct {
    unsigned long fg, bg, border;
    XFontStruct *Font;
    XFontStruct *helpFont;
} PanelData, *PanelDataPtr;
PROG_ PanelData panel_data;
