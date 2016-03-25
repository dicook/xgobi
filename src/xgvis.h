#ifndef XGVISINTERN
#define XGVIS_ extern 
#define INIT(x)
#else
#define XGVIS_
#define INIT(x) x
#endif

XGVIS_ xgobidata xgobi;

typedef struct {
    XFontStruct *Font;
    int mdsDimension;
} PanelData, *PanelDataPtr;
XGVIS_ PanelData panel_data;

XGVIS_ Widget shell;
XGVIS_ Widget form0;
XGVIS_ Widget dims_left, dims_right;

/* Defines. */
#define METRIC    0
#define NONMETRIC 1

#define KRUSKALSHEPARD 0
#define CLASSIC        1

#define USER_SUPPLIED 0
#define LINK 1
#define ADJACENCY 2
#define EUCLIDIAN 3
#define MANHATTAN 4
#define MAHALANOBIS 5
#define DOTPROD 6
#define COSDIST 7
#define CLOSE 8
#define NDISTTYPES (CLOSE+1)
XGVIS_ Widget apply_dist_cmd;

#define MAXDIMS 13

#define EPSILON .00001

/* Global variables. */ 
XGVIS_ int xgv_is_running INIT(= 0);
XGVIS_ int max_dims INIT(= 10);
XGVIS_ int is_rescale  INIT(= 0);
XGVIS_ Widget dist_cmd, dist_popup, dist_mgr, dist_types[NDISTTYPES] ;
XGVIS_ int dist_type INIT(= 0) ;

XGVIS_ enum {deflt, within, between, anchorscales, anchorfixed} mds_group_ind;
XGVIS_ double mds_stepsize  INIT(= 0.02);
XGVIS_ double mds_power  INIT(= 1.0);
XGVIS_ double mds_isotonic_mix  INIT(= 1.0);
XGVIS_ double mds_distpow INIT(= 1.0);
XGVIS_ double mds_lnorm  INIT(= 2.0);
XGVIS_ double mds_distpow_over_lnorm  INIT(= 0.5);
XGVIS_ double mds_lnorm_over_distpow  INIT(= 2.0);
XGVIS_ double mds_weightpow INIT(= 0.0);
XGVIS_ double mds_within_between INIT(=1.0);
XGVIS_ double mds_rand_select_val INIT(=1.0);
XGVIS_ double mds_rand_select_new INIT(=FALSE);
XGVIS_ double mds_perturb_val INIT(=1.0);
XGVIS_ double mds_threshold_high  INIT(= 0.0);
XGVIS_ double mds_threshold_low  INIT(= 0.0);  /* what initial value? */
XGVIS_ int    mds_dims INIT(= 3);
XGVIS_ int    mds_freeze_var INIT(= 0);
XGVIS_ Boolean *anchor_group INIT(= NULL);

/* Used in scaling during each mds loop; set in reset_data */
XGVIS_ double *config_dist   INIT(= NULL); /* spave vs time: store configuration distances to save recalculation */
XGVIS_ double *raw_dist      INIT(= NULL); /* pointer, a vector version of dist.data */
XGVIS_ double *weights       INIT(= NULL); /* formed only when mds_weightpow != 0. */
XGVIS_ double *trans_dist    INIT(= NULL); /* transformed dissimilarities: power (metric), isotonic (nonmetric) */
XGVIS_ int *trans_dist_index INIT(= NULL); /* index array for sort of raw_dist */
XGVIS_ int *bl               INIT(= NULL); /* blocklengths for isotonic regression */
XGVIS_ double *bl_w          INIT(= NULL); /* blockweights for isotonic regression (only when mds_weightpow != 0.) */
XGVIS_ double *rand_sel      INIT(= NULL); /* random selection probabilities (only when mds_rand_select != 1.) */

XGVIS_ int ndistances;
XGVIS_ int num_active_dist;
XGVIS_ double dist_max INIT (= 0.0);
XGVIS_ double dist_min INIT (= 0.0);

XGVIS_ double configuration_factor;

/* Used to hold matrix structures. */
struct array {
  double **data;
  int nrows;
  int ncols;
};

/* Global data structures. */
XGVIS_ struct array dist_orig;
XGVIS_ struct array dist;
XGVIS_ struct array edges_orig;
XGVIS_ struct array edges;
XGVIS_ struct array lines;
XGVIS_ struct array *linesptr; /* points to lines, if present; else to edges */
XGVIS_ struct array pos_orig;
XGVIS_ struct array pos;
XGVIS_ double *pos_mean INIT (=NULL);
XGVIS_ double pos_scl INIT (=0.);
XGVIS_ char **rowlab INIT(= NULL);	/* Row labels. */
XGVIS_ char *xgv_basename;

/* for diagnostic xgobi, for distance residuals */
XGVIS_ struct array diagnostics;
XGVIS_ char **drowlab INIT(= NULL);

XGVIS_ char pcolorname[128];
XGVIS_ char lcolorname[128];
XGVIS_ char glyphname[128];

XGVIS_ int metric_nonmetric INIT(= METRIC);
XGVIS_ int KruskalShepard_classic INIT(= KRUSKALSHEPARD);
