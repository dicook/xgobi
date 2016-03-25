/* xgobitypes.h */
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

#ifndef XGOBITYPES_H
#define XGOBITYPES_H

#define true 1
#define false 0

#define NCOLS 500
/*
 * EXP1: Raw data are scaled to -2^EXP1, +2^EXP1
 * EXP2: Trigonometric coefficients are scaled up by 2^EXP2 to do
 *       integer math.
 * PRECISION: 2^EXP2
*/
/*
#define EXP1 15
#define EXP2 14
#define PRECISION1 32768
#define PRECISION2 16384
*/
#define EXP1 14
#define EXP2 13
#define PRECISION1 16384
#define PRECISION2  8192


/*
 * Used in grand tour code.
*/
#define PP_REPLOT_FREQ 15

/* Not all architectures have these definitions */
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2  1.57079632679489661923
#endif

#define TWOPI 6.2831853071795864
#define MAXTHETA 6.2831853071795864
#define MAXHIST 100    /* maximum number of backtracks */
/* for use in reset_var_labels() */
#define PRINCCOMP_ON 0
#define PRINCCOMP_OFF 1
/* backtrack defines */
#define FORWARD 1
#define BACKWARD -1
/* local scan defines */
#define OUT 1
#define IN 0

/* So that some toggle widgets can be set up to be
 * "0 of many" and some can be forced to be "1 of many". */
#define ANY_OF_MANY 0
#define ONE_OF_MANY 1

/* Default size of varselect circles.  */
#define RADIUS 15
/* Initial rotation angle */
/* #define THETA0 0.015 */
#define THETA0 0.01
/* Initial tour step */
#define TOURSTEP0 0.003

/* margin to use in defining extended brush for plot_bins() */
/* I think this should be half the size of the biggest glyph,
   which is presently the circle with diameter 15
*/
#define BRUSH_MARGIN 8

/* glyph types and sizes */
#define NGLYPHTYPES 7
#define NGLYPHSIZES 5
/* number of brushing glyphs and colors */
#define NGLYPHS ((NGLYPHTYPES-1)*NGLYPHSIZES + 1)
#define NCOLORS 10

#define PLUS_GLYPH 1
#define X_GLYPH 2
#define OPEN_RECTANGLE_GLYPH 3
#define FILLED_RECTANGLE_GLYPH 4
#define OPEN_CIRCLE_GLYPH 5
#define FILLED_CIRCLE_GLYPH 6
#define POINT_GLYPH 7

#define TINY 1
#define SMALL 2
#define MEDIUM 3
#define LARGE 4
#define JUMBO 5

/*
 * fraction of plot window used for default scaling
*/
#define DEF_SCALE 0.65
#define SCALE_MIN 0.02
#define SCALE0 0.005
#define SQUARE_PLOT 1

#define COLLABLEN 25
#define ROWLABLEN 50

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define BETWEEN(a,b,x) ( ((a)<=(x) && (x)<=(b)) || ((a)>=(x) && (x)>=(b)) )
#define INT(x) ((int)(x))
#define DOUBLE(x) ((double)(x))
#define FLOAT(x) ((float)(x))
#define SHORT(x) ((short)(x))
#define FONTHEIGHT(f) ((f)->max_bounds.ascent + (f)->max_bounds.descent)
#define FONTWIDTH(f) ((f)->max_bounds.width)

#define GROUPID(i) (xg->raw_data[(i)][xg->ncols-1])
#define GROUPID_RAW(i) (xg->raw_data[(i)][xg->ncols-1])
#define GROUPID_TFORM1(i) (xg->tform1[(i)][xg->ncols-1])
#define GROUPID_TFORM2(i) (xg->tform2[(i)][xg->ncols-1])

/*
 * number of bins to use in dividing the screen into regions
 * for speed in brushing and identification
*/
#define NVBINS 30
#define NHBINS 30

#define BUFSIZE 24576
#define MSGLENGTH 256

#define ASCII_TEXT_BORDER_WIDTH 4

#define SAVE_SPIN_COEFS          save_types[0]
#define SAVE_SPIN_RMAT           save_types[1]
#define READ_SPIN_RMAT           save_types[2]
#define SAVE_TOUR_COEFS          save_types[3]
#define SAVE_TOUR_HIST           save_types[4]
#define READ_POINT_COLORS_GLYPHS save_types[5]
#define READ_TOUR_HIST           save_types[6]
#define OPEN_BITMAP_FILE         save_types[7]
#define READ_LINES               save_types[8]
#define READ_LINE_COLORS         save_types[9]

#define PLOT1D_MODE 0
#define XYPLOT_MODE 1
#define ROTATE_MODE 2
#define GTOUR_MODE 3
#define CTOUR_MODE 4
#define SCALE_MODE 5
#define BRUSH_MODE 6
#define IDENTIFY_MODE 7
#define LINEEDIT_MODE 8
#define MOVEPTS_MODE 9
#define NVIEWMODES 10

typedef struct {
	long x, y;
} lcoords;
typedef struct {
	int x, y;
} icoords;
typedef struct {
	float x, y;
} fcoords;
typedef struct {
	float min, max;
} lims;
typedef struct {
    int a, b;
} connect_lines;
typedef struct {
    int x1, y1, x2, y2;
} brush_coords;

typedef struct {
    Pixel fg, bg, border;
} WidgetColors;

/* For Juergen's cloning code */
enum clone_data_type {XGData, ArcData, CDF1, CDFm};

/*
 * When changing this structure, don't forget to edit the resources[]
 * vector in xgobi_init.c
*/
typedef struct {
    unsigned long fg, bg, border;
	XFontStruct *font;
	XFontStruct *plotFont;
	XFontStruct *helpFont;
/* make these all pixels?  If I get axisColor working ... */
	String brushColor0, brushColor1, brushColor2, brushColor3;
	String brushColor4, brushColor5, brushColor6, brushColor7;
	String brushColor8, brushColor9;
    Boolean showAxes, showPoints, showLines;
    /* linking options */
    Boolean linkGlyphBrush, linkColorBrush, linkEraseBrush;
    Boolean linkLineBrush;
    Boolean linkIdentify;
    /* other brushing options */
    Boolean jumpBrush, reshapeBrush, syncBrush;
    Boolean carryVars;
    Boolean plotSquare;
    int glyphType;
    int glyphSize;

    int defaultGlyph;
    String defaultColor;
    String axisColor;

	String pointerColor;
    String defaultPrintCmd;
    /* Cloning Information */
    Boolean isCloned;
    int clonePID, cloneTime, cloneType;
    String cloneName;
    Boolean deleteCloneData;
} AppData, *AppDataPtr;

typedef struct {
	Dimension width, height;
} WidgetSize;

/* glyph vectors */
typedef struct {
    int type;
    int size;
} glyphv;

/* cluster; to be used in Group Exclusion tool */
typedef struct {
  char name[32];
  int glyphtype, glyphsize;
  unsigned long color;
  int glyphtype_prev, glyphsize_prev;
  unsigned long color_prev;
  Boolean hidden, excluded;
} cluster;

/* grand tour history */
typedef struct hist_rec {
    struct hist_rec *prev, *next;
    float *hist[2];
} hist_rec;

/* row groups */
typedef struct {
  long id;
  long *els;
  long nels;
  Boolean excluded;  /* will use this for linked brushing */
  Boolean hidden;  /* may not use this, but add it just in case */
} rg_struct;

/* axis plotting */
#define NTICKS 100
typedef struct {
	int nticks[NCOLS];
	float xticks[NTICKS];
	float yticks[NTICKS];
	lcoords plane[NTICKS];
	icoords screen[NTICKS];
} tickinfo;

#define ChooseX(w,r,x) XDrawLine(display, w, varpanel_xor_GC, \
  (int) r, (int) r, (int) (r+x), (int) r)
#define ChooseY(w,r,x) XDrawLine(display, w, varpanel_xor_GC, \
  (int) r, (int) r, (int) r, (int) (r-x))
#define ChooseVar(w,r,x,y) XDrawLine(display, w, varpanel_xor_GC, \
  (int) r, (int) r, (int) (r+x), (int) (r-y))

enum link_state {send_state, receive, unlinked};

/* interactive gt*/
enum manip_type {oblique, vertical, horizontal, radial, angular, combined,
  eqcomb};

enum cont_fact {tenth, fifth, quarter, third, half, one, two, ten, infinite};
enum brush_mode_type {persistent, transient, undo};

typedef enum {ascii, Sprocess, binary} DataMode;
typedef enum {read_all, read_block, draw_sample} FileReadType;

typedef struct {
 Boolean is_realized;
 Boolean is_iconified;
 DataMode data_mode;
 Cardinal plot_mode, prev_plot_mode;

 char *progname; /* argv[0] */
 char datafilename[128]; /* full file name, including path */
 char *datarootname;     /* file name without path */
 char datafname[128];    /* file name without suffix: .dat, .missing */
 char title[128];        /* title to be used in window manager */
 char vtitle[128];       /* visible title (passed through -vtitle) */

 /*
  * Do we read in the entire file, or do we only read in some
  * block or sample of cases?
 */
 FileReadType file_read_type;
 long file_start_row;     /* needed for block type */
 long file_length;        /* needed for sample */
 long file_sample_size;   /* needed for both */
 /*
  * To be used in reading in associated row-wise files
 */
 long *file_rows_sampled; /* of length file_sample_size */

 Widget shell, form0, box0, box1, box2, workspace;
 Window plot_window;
 Drawable pixmap0;
 /* Vectors of min and max values - including current fixed or imputed data
  * lim0 contains the min_max limits -- for the tform_data!
  * lim contains the limits in use
  * lim_tform contains the limits of the tform_data -- used in
  *   parallel coordinates in case the main window is displaying
  *   sphered data (is there a difference between lim0 and lim_tform?)
  * lim_raw contains the min_max limits -- for the raw_data!
 */
 lims *lim0, *lim, *lim_tform, *lim_raw;
 lims *nicelim;
 float **raw_data;
 float **sphered_data;
 float **tform1;
 float **tform2;
 long **world_data;
 long **jitter_data;
 lcoords *planar;
 icoords *screen;

/* Missing values */
 Boolean missing_values_present;
 Boolean is_missing_values_xgobi;
 short **is_missing;
 int nmissing;

 icoords max, mid;
 int minxy;
 lcoords is;
 int ncols, nrows;
 int ncols_used;
 char save_type[50];
 WidgetSize plotsize;

/* Carry variables between plotting modes? */
 Boolean carry_vars;

 /* Deleting the erased points; subsetting */
 Boolean delete_erased_pts;
 int *rows_in_plot;
 int nrows_in_plot;

 /* Deleting the erased points -- this notion will change to hidden/excluded */
 cluster *clusv;
 int nclust;
 unsigned short *erased;
 unsigned short *excluded; /* it's too slow to get this from rows_in_plot */

 /* axis plotting */
 Boolean is_axes, add_gridlines;
 Boolean is_axes_centered;
 unsigned long axisColor;

/* Variable grouping, as read and in its current state */
 int *vgroup_ids_ori, *vgroup_ids;

/* Row grouping */
 long nrgroups, nrgroups_in_plot;
 long *rgroup_ids;
 rg_struct *rgroups;

/* Line grouping */
 long nlgroups;
 long *lgroup_ids;
 rg_struct *lgroups;  /* id, nels, *els */

/* standardization options */
 int std_type;  /* Can be 0, 1 or 2 */
 /* float std_width; */  /* no longer used */

/* textured dot plotting */
 Boolean is_plotting1d;
 Boolean is_plot1d_cycle;
 icoords plot1d_vars;

/* xy plotting */
 Boolean is_xyplotting, is_xy_cycle;
 icoords xy_vars;

/* Axes */
 float *tickdelta;
 tickinfo ticks0, ticks;
 /* axis positions in screen coordinates */
 icoords screen_axes[3];
 int *deci;
 int maxwidth, minindex;

 /* for connecting points with segments */
 connect_lines *connecting_lines;
 Boolean connect_the_points;
 Boolean plot_the_points;
 Boolean plot_the_arrows;  /* Add the suggestion of an arrow to each line */
 int nlines;
 Widget le_add_mouse, le_delete_mouse;
 Boolean is_line_editing;

 /* for <gasp> moving points */
 Boolean is_point_moving;
 Widget movepts_mouse;

/* case identification */
 Widget identify_mouse ;
 Boolean is_identify;
 Boolean is_identify_cycle;
 int nearest_point;  /* -1 if outside the window; 0:nrows otherwise */
 Cardinal *sticky_ids;
 int nsticky_ids;
 char **rowlab;

/* smoothing */
  Boolean is_smoothing;
 
/* case profile plots */
 Boolean is_cprof_plotting;
 Boolean link_cprof_plotting;

/* magnifying glass */
 Boolean is_magnify;

/* help */
 struct {int pb, menupb, sbar;} nhelpids;

/* variable selection */
 Widget var_panel;
 Widget *varboxw, *varlabw, *vardraww;
 Boolean *varchosen;
 char **collab;
 char **collab_short;  /* a short label to use in the parcoords plot */
 char **collab_tform1, **collab_tform2;
 unsigned int radius;  /* change to var_radius ? */

/* Boolean: does this data contain only one variable? False by default */
 Boolean single_column;

/* variable and case selection lists */
 Boolean *selectedvars, *selectedcases;

/* forwarding of brushing information */
 int *last_forward;

/* brushing */
 Widget brush_mouse ;
 Boolean is_brushing;
 Boolean is_point_painting;
 unsigned short *under_new_brush;
 enum brush_mode_type brush_mode;

 Boolean brush_on ;
 Boolean is_glyph_painting;
 Boolean is_color_painting;
 /* if True, brush jumps to cursor; if False, cursor jumps to brush */
 Boolean jump_brush;
 /* if True, brush is reshaped between modes */
 Boolean reshape_brush;
 /* if True, brushing updates are performed synchronously */
 Boolean sync_brush;

 glyphv glyph_id, glyph_0;
 glyphv *glyph_ids, *glyph_now, *glyph_prev;

 unsigned long color_id, color_0;
 unsigned long *color_ids, *color_now, *color_prev;
/* line brushing */
 unsigned short *xed_by_new_brush;
 Boolean is_line_painting;
 unsigned long *line_color_ids, *line_color_now, *line_color_prev;

/* linking of brushing, identification, touring ... */
 Boolean link_glyph_brushing, link_color_brushing, link_erase_brushing;
 Boolean link_points_to_points, link_points_to_lines;
 Boolean link_lines_to_lines;
 Boolean link_identify;
 enum link_state tour_link_state;
 Boolean current_window;
 unsigned long *tour_senddata; /* Separate vector for tour linking */
 Boolean got_new_paint;

 unsigned long *senddata;
 int nlinkable, nlinkable_in_plot;  /* used in sizing senddata */

/* for binning the plot window */
 int nhbins, nvbins;
 Cardinal ***binarray;
 Cardinal **bincounts;
/*
 * These are initialized so that the first merge_brushbins()
 * call will behave reasonably.
*/
 icoords bin0, bin1;

/* shifting and scaling */
 Boolean plot_square;
 Widget scale_mouse;
 fcoords scale;
 fcoords scale0;
 icoords shift_wrld;
 Boolean is_scaling;
 Boolean run_shift_proc, run_scale_proc;
 int scaling_btn;
 icoords cntr;

/* Rotation */
 Widget spin_mouse ;
 Boolean is_spinning;
 Boolean is_spin_paused;
 struct {int x, y, z;} spin_vars;
 struct {Boolean yaxis, xaxis, oblique;} is_spin_type;
 float theta0;
 struct {float yaxis, xaxis, oblique;} theta;
/* Axis-based rotation */
 fcoords cost, sint;
 icoords icost, isint, ocost, osint;
/* Oblique rotation */
 icoords xax, yax, zax;
 float Rmat0[3][3];
 Boolean is_interp;
 Boolean is_rocking;
 Boolean run_spin_axis_proc;
 Boolean run_rock_proc;
 Boolean run_interp_proc;
 Boolean run_spin_oblique_proc;

/* grand tour */
 Widget tour_io_menu_cmd, tour_io_menu;
 Widget tour_io_menu_btn[3];
 Widget princ_comp_cmd, proj_pursuit_cmd, tour_section_cmd;
 Widget pc_axes_cmd;/*interactive gt*/
 /* bases */
 float **u0, **u1, **u, **uold;
 /* projections onto span of starting and ending plane */
 long  **xi0, **xi1;
 /* principle directions, with tv a scratch array */
 float **tv, **v0, **v1;

 /* angles of rotation from u to v */
 float s[2];
 /* cosines and sines of s */
 float coss[2], sins[2];
 /* ditto in integer */
 long  icoss[2], isins[2];
 /* principle angles incrementer*/
 float tinc[5];  /* Givens interpolation requires 5 here */
 /* principle angles */
 float tau[5];  /* Givens interpolation requires 5 here */
 /* scratch arrays */
 float *tnx;

 /* number of steps between bases */
 float step;
 /* step increment based on intrinsic L2-distance */
 float delta;
 /* "intrinsic L2-distance" */
 float dv;
 /* indicator for in grand tour mode */
 Boolean is_touring;
 /*number of variables on tour */
 int numvars_t;
 /*numbers of chosen variables */
 int *tour_vars;
 /*numbers of sphered variables */
 int nsph_vars;
 int *sph_vars;
 /* for backtracking */
 hist_rec *fl, *curr, *hfirst, *hend;
 int nhist_list, max_nhist_list, old_nhist_list;
 /* indicator for backtracking running */
 Boolean is_backtracking;
 /* indicator for in step mode */
 Boolean is_stepping;
 /* indicator for locally scanning U0-U1-U0-U2-..*/
 Boolean is_local_scan;
 int local_scan_dir;
 /* indicator for cycling backwards through history */
 int backtrack_dir;  /* Can be FORWARD or BACKWARD */
 /* indicator for switching between different actions in backtrack_proc() */
 int backtrack_action;  /* 0, 1 or 2 */
 char **tour_lab;
 /* needs to be global to be passed into pack_data vector for linked touring */
 Boolean new_basis_ind;

 Boolean wait_for_more_vars;

 Boolean is_tour_paused;
 Boolean run_tour_proc;
 Boolean tour_hist_just_read;
 Boolean is_store_history;

 /* principal components and projection pursuit */
 Widget pp_plot_wksp;
 Boolean is_pp;
 Boolean is_pp_optimz;
 Boolean is_princ_comp;
 Boolean is_pc_axes;/* interactive gt */
 int pp_index_btn;
 Boolean recalc_max_min;
 int xaxis_indent;
 Boolean new_direction_flag;
 int pp_replot_freq;

/* For section tour */
 Boolean is_tour_section;
 glyphv *section_glyph_ids;
 unsigned long *section_color_ids;

/* correlation */
 Boolean is_corr_touring;
 Boolean run_corr_proc;
 int ncorr_xvars, *corr_xvars, ncorr_yvars, *corr_yvars;
 Boolean is_corr_sphered;
 Boolean is_corr_pursuit;
 Boolean is_corr_optimz;
 Boolean cp_recalc_max_min;
 Boolean cp_replot_freq;

 Boolean new_corr_dir_flag;
 Boolean is_corr_bt;
 Boolean is_corr_paused;
 Boolean is_corr_syncd_axes;
 float **cu0, **cu1, **cu, **cuold;

/* interactive tour controls */
 Widget tour_mouse;/* interactive gt*/
 int manip_var;
 enum manip_type tour_manip_type;
 enum cont_fact tour_cont_fact;
 float fcont_fact;
 int *frozen_vars;
 int nfrozen_vars;
 long  **xif;
 float **ufrozen;
 float **uwarm;
 
/* interactive corr controls */
 Widget corr_mouse;/* interactive gt*/
 int corr_xmanip_var, corr_ymanip_var;
 enum manip_type corr_manip_type;
 enum cont_fact corr_cont_fact;
 float fccont_fact;

/* corr frozen vars */
 int *corr_xfrozen_vars;
 int ncxfrozen_vars;
 int *corr_yfrozen_vars;
 int ncyfrozen_vars;
 float **cufrozen;
 float **cuwarm;

 /* clone information */
 Boolean isCloned;
 int clone_PID;
 int clone_Time;
 enum clone_data_type clone_Type;
 char *clone_Name;
 Boolean delete_clone_data;

 /* RPC stuff */
 Boolean arcview_flag;
 Boolean xplore_flag;
 Boolean virgis_flag;
 Boolean xgobi_is_up;

 /* Scatterplot matrices */
 int sm_nrows, sm_ncols;
 Boolean is_scatmat;

} xgobidata;


#endif
