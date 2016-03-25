/* *********************************************************
 * mds.c: multidimensional scaling (a la andreas) ML 2/92. *
*/

/*
static Boolean
sticky_case(int jcase)
{
  int j;
  Boolean sticky = false;
  for (j=0; j<xgobi.nsticky_ids; j++) {
    if (xgobi.sticky_ids[j] == jcase) {
      sticky = True;
      break;
    }
  }
  return (sticky);
}
*/

/* Includes. */
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
#include <X11/keysym.h>
#include "xgvis.h"

extern void clear_array(struct array *);
extern void copy_array(struct array *, struct array *);
extern void add_stress_value(double);
extern void make_empty_array(struct array *, int, int);
extern void draw_stress(void);
extern void zero_array(struct array *);
extern void update_shepard_labels(int);

extern Widget metric_cmd[2];
extern int moving_point, move_type;

#define SAMEGLYPH(i,j) \
( xg->color_now[(i)]      == xg->color_now[(j)] &&      \
  xg->glyph_now[(i)].type == xg->glyph_now[(j)].type && \
  xg->glyph_now[(i)].size == xg->glyph_now[(j)].size )

#define IJ i*dist.ncols+j 
#define JI j*dist.nrows+i

int *point_status INIT (= NULL);
int point_is_out=0, point_is_in=1, point_is_anchor=2, point_is_dragged=4;

double delta = 1E-10;
#define signum(x) (((x) < 0.0) ? (-1.0) : (((x) > 0.0) ? (1.0) : (0.0)))

double
sig_pow(double x, double p)
{
  return((x >= 0 ? pow(x, p) : -pow(-x, p)));
}

double
Lp_distance_pow(int i, int j)
{
  double dsum = 0.0;
  int k;

  if(mds_lnorm == 2. && mds_distpow == 1.) {
    for (k = 0; k < mds_dims; k++)  dsum += (pos.data[i][k]-pos.data[j][k])*(pos.data[i][k]-pos.data[j][k]);
    return(sqrt(dsum));
  } else { /* non-Euclidean or distance power != 1. */
    for (k = 0; k < mds_dims; k++)  dsum += pow(fabs(pos.data[i][k]-pos.data[j][k]), mds_lnorm);
    return(pow(dsum, mds_distpow_over_lnorm));
  }
}

/* begin centering and sizing routines */

void
get_center(void)
{
  int i, k, n;

  if (pos_mean == NULL)
    pos_mean = (double *) XtMalloc(MAXDIMS * sizeof(double));

  n = 0;
  for(k=0; k<mds_dims; k++) { pos_mean[k] = 0.; }
  for(i=0; i<pos.nrows; i++)
    if(point_status[i] != point_is_out) {
      for(k=0; k<mds_dims; k++) 
	pos_mean[k] += pos.data[i][k];
      n++;
    }
  for(k=0; k<mds_dims; k++) { pos_mean[k] /= n; }
}

void
get_center_scale(void)
{
  int n, i, k;

  get_center();

  n = 0;
  pos_scl = 0.;
  for(i=0; i<pos.nrows; i++)
    if(point_status[i] =! point_is_out) {
      for(k=0; k<mds_dims; k++) 
        pos_scl += ((pos.data[i][k] - pos_mean[k]) *
                    (pos.data[i][k] - pos_mean[k]));
      n++;
    }
  pos_scl = sqrt(pos_scl/n/mds_dims);
}

void
center_pos(void) 
{
  int i, k;

  get_center();

  for (i=0; i<pos.nrows; i++)
    if(point_status[i] != point_is_out)
      for (k=0; k<mds_dims; k++)
        pos.data[i][k] -= pos_mean[k];
}

/* restore configuration to old scale */
void
scale_pos(void) 
{
  int i, k;

  get_center_scale();

  for (i=0; i<pos.nrows; i++)
    if(point_status[i] != point_is_out)
      for (k=0; k<mds_dims; k++)
        pos.data[i][k] = (pos.data[i][k] - pos_mean[k])/pos_scl + pos_mean[k];
}

void
center_scale_pos(void) 
{
  int i, k;

  get_center_scale();

  for (i=0; i<pos.nrows; i++)
    if(point_status[i] != point_is_out)
      for (k=0; k<mds_dims; k++)
        pos.data[i][k] = (pos.data[i][k] - pos_mean[k])/pos_scl;
}

/* end centering and sizing routines */

double
dot_prod(int i, int j)
{
  double dsum = 0.0;
  int k;

  for(k=0; k<mds_dims; k++) 
    dsum += (pos.data[i][k] - pos_mean[k])*(pos.data[j][k] - pos_mean[k]);
  return(dsum);
}

double
L2_norm(double *p1)
{
  double dsum = 0.0;
  int k;

  for (k = mds_freeze_var; k < mds_dims; k++)  
    dsum += (p1[k] - pos_mean[k])*(p1[k] - pos_mean[k]);
  return(dsum);
}


/* weights are only set if mds_weightpow != 0; for 0 there's simpler code throughout, and we save space */
void
set_weights()
{
  int i, j;
  double this_weight;
  static double local_weightpow = 0.;
  static double local_within_between = 1.;
  xgobidata *xg = (xgobidata *) &xgobi; /* used in macros SAMEGLYPH and ACHORED */

  /* the weights will be used in metric and nonmetric scaling 
   * as soon as mds_weightpow != 0. or mds_within_between != 1.
   * weights vector only if needed */
  if((mds_weightpow      != local_weightpow      && mds_weightpow      != 0.) || 
     (mds_within_between != local_within_between && mds_within_between != 1.)) 
    {
      if(weights == NULL) weights = (double *) XtMalloc(ndistances * sizeof(double)); /* power weights */
      
      for(i=0; i<dist.nrows; i++)
	for(j=0; j<dist.ncols; j++) {
	  if(dist.data[i][j] == DBL_MAX) { weights[IJ] = DBL_MAX; continue; }
	  if(mds_weightpow != 0.) {
	    if(dist.data[i][j] == 0.) { /* cap them */
	      if(mds_weightpow < 0.) { weights[IJ] = 1E5; continue; }
	      else { weights[IJ] = 1E-5; }
	    }
	    this_weight = pow(dist.data[i][j], mds_weightpow); 
	    /* cap them */
	    if(this_weight > 1E5)  this_weight = 1E5;
	    if(this_weight < 1E-5) this_weight = 1E-5;
	    /* within-between weighting */
	    if(SAMEGLYPH(i,j)) { this_weight *= (2. - mds_within_between); }
	    else { this_weight *= mds_within_between; }
	    weights[IJ] = this_weight;
	  } else { /* mds_weightpow == 0. */
	    if(SAMEGLYPH(i,j)) { this_weight = (2. - mds_within_between); }
	    else { this_weight = mds_within_between; }
	    weights[IJ] = this_weight;
	  }
	}
    }
} /* end set_weights() */


void
set_random_selection()
{
  int i;

  if (mds_rand_select_val != 1.0) { 
    if(rand_sel == NULL) {
      rand_sel  = (double *) XtMalloc(ndistances * sizeof(double));
      for(i=0; i<ndistances; i++) { 
	rand_sel[i] = (double) randvalue();
      }
    }
    if(mds_rand_select_new) {
      for(i=0; i<ndistances; i++) { rand_sel[i] = (double) randvalue(); }
      mds_rand_select_new = FALSE;
    }
  }
} /* end set_random_selection() */


double stress, stress_dx, stress_dd, stress_xx;
void
update_stress()
{
  int i, j;
  double this_weight, dist_config, dist_trans;

    stress_dx = stress_xx = stress_dd = 0.0;

    for(i=0; i < dist.nrows; i++) 
      for(j=0; j < dist.ncols; j++) {
	dist_trans  = trans_dist[IJ];          	if(dist_trans == DBL_MAX) continue;
	dist_config = config_dist[IJ];
	if(mds_weightpow == 0. && mds_within_between == 1.) { 
	  stress_dx += dist_trans  * dist_config;
	  stress_xx += dist_config * dist_config;
	  stress_dd += dist_trans  * dist_trans;
	} else {
	  this_weight = weights[IJ];
	  stress_dx += dist_trans  * dist_config * this_weight;
	  stress_xx += dist_config * dist_config * this_weight;
	  stress_dd += dist_trans  * dist_trans  * this_weight;
	}
      }

    /* calculate stress and draw it */
    if (stress_dd * stress_xx > delta*delta) {
      stress = pow( 1.0 - stress_dx * stress_dx / stress_xx / stress_dd, 0.5);
      add_stress_value(stress);
      draw_stress();
    } else {
      printf("didn't draw stress: stress_dx = %5.5g   stress_dd = %5.5g   stress_xx = %5.5g\n",
	     stress_dx, stress_dd, stress_xx);
    }
} /* end update_stress() */


/* we assume in this routine that trans_dist contains 
   dist.data for KRUSKALSHEPARD and 
   -dist.data*dist.data for CLASSIC MDS */
void
power_transform()
{
  double tmp, fac;
  int i;

  if(mds_power == 1.) { 
    return; 
  } else if(mds_power == 2.) {
    if(KruskalShepard_classic == KRUSKALSHEPARD) { 
      for(i=0; i<ndistances; i++) {
	tmp = trans_dist[i];
	if(tmp != DBL_MAX) trans_dist[i] = tmp*tmp/dist_max;
      }
    } else { 
      for(i=0; i<ndistances; i++) {
	tmp = trans_dist[i];
	if(tmp != DBL_MAX) trans_dist[i] = -tmp*tmp/dist_max;
      }
    }
  } else {
    fac = pow(dist_max, mds_power-1);
    if(KruskalShepard_classic == KRUSKALSHEPARD) { 
      for(i=0; i<ndistances; i++) {
	tmp = trans_dist[i];
	if(tmp != DBL_MAX) trans_dist[i] = pow(tmp, mds_power)/fac;
      }
    } else { 
      for(i=0; i<ndistances; i++) {
	tmp = trans_dist[i];
	if(tmp != DBL_MAX) trans_dist[i] = -pow(-tmp, mds_power)/fac;
      }
    }
  }

} /* end power_transform() */


/* for sorting in isotonic regression */
static double *tmpVector;
static int aIndex, bIndex;
static double aReal, bReal;
int realCompare(const void* aPtr, const void* bPtr)
{
  aIndex = *(int*)aPtr;
  bIndex = *(int*)bPtr;
  aReal = tmpVector[aIndex];
  bReal = tmpVector[bIndex];
  if (aReal < bReal) return -1;
  else if (aReal == bReal) return 0;
  else return 1; 
}
/* nonmetric transform with isotonic regression of config_dist on dist */
void
isotonic_transform()
{
  int i, j, ii, ij;
  double tmp_dist, tmp_distsum, tmp_weightsum, this_weight,
    t_d_i, t_d_ii;
  Boolean finished;
  static int prev_nonmetric_active_dist = 0;

  /* the sort index for dist.data */
  if (trans_dist_index == NULL) {
    trans_dist_index = (int *)    XtMalloc(ndistances * sizeof(int)); 
    printf("allocated trans_dist_index \n");
  }
  /* block lengths */
  if (bl == NULL) {
    bl               = (int *)    XtMalloc(ndistances * sizeof(int)); 
    printf("allocated block lengths \n");
  }
  /* block weights */
  if(bl_w == NULL && (mds_weightpow != 0. || mds_within_between != 1.)) {
    bl_w             = (double *) XtMalloc(ndistances * sizeof(double)); 
    printf("allocated block weights \n");
  }

  /* sort if necessary 
   *	 (This is not the proper criterion because the active distances could change 
   *	 while their number remains the same...; needs thought.)  
   */
  if (num_active_dist != prev_nonmetric_active_dist) {
    tmpVector = trans_dist;  /* "tmpVector" is the vector by which to sort; see "realCompare" above */
    for (i = 0 ; i < dist.nrows; i++) {
      for (j = 0; j < dist.ncols; j++) {
	trans_dist_index[IJ] = IJ;
      }}
    Myqsort(trans_dist_index, ndistances, sizeof(int), realCompare);
    prev_nonmetric_active_dist = num_active_dist;
    printf("sorted the dissimilarity data \n");
  }

  /* initialize blocks wrt ties; this should also preserve symmetry if present */
  for (i = 0 ; i < ndistances; i += bl[i]) {  
    ii = i+1; 
    tmp_dist = trans_dist[trans_dist_index[i]];
    while ((ii < ndistances) && (trans_dist[trans_dist_index[ii]] == tmp_dist)) ii++;
    /* ii points to start of the next block */
    bl[i] = ii-i;
  }

  /* trans_dist is computed by isotonic regression of config_dist on trans_dist_index, therefore: */
  for(i = 0; i < ndistances; i++) trans_dist[i] = config_dist[i];

  /* form initial block means (and weights if necessary); need to fill only first element of a block */
  for(i = 0; i < ndistances; i += bl[i]) {	
    if(trans_dist[trans_dist_index[i]] != DBL_MAX) {
      ii = i + bl[i];
      if(mds_weightpow == 0. && mds_within_between == 1.) {
	tmp_distsum = 0.;  
	for(j = i; j < ii; j++) tmp_distsum += trans_dist[trans_dist_index[j]];
	trans_dist[trans_dist_index[i]] = tmp_distsum / bl[i];
      } else {
	tmp_distsum = tmp_weightsum = 0.;  
	for(j = i; j < ii; j++) {
	  this_weight = weights[trans_dist_index[j]];
	  tmp_distsum += trans_dist[trans_dist_index[j]] * this_weight;
	  tmp_weightsum += this_weight;
	}
	bl_w[i] = tmp_weightsum;
	trans_dist[trans_dist_index[i]] = tmp_distsum / tmp_weightsum;
      }
    }
  }

  /* pool-adjacent-violator algorithm for isotonic regression */
  finished = False;
  while (!finished) {
    finished = True;
    i = 0;  ii = i + bl[i];
    while (i < ndistances && ii < ndistances) {
      t_d_i  = trans_dist[trans_dist_index[i]];
      t_d_ii = trans_dist[trans_dist_index[ii]];
      if (t_d_i > t_d_ii) { /* pool blocks starting at i and ii */
	if(mds_weightpow == 0. && mds_within_between == 1.) {
	  trans_dist[trans_dist_index[i]] = 
	    (t_d_i * bl[i] + t_d_ii * bl[ii]) / (bl[i] + bl[ii]);
	} else {
	  trans_dist[trans_dist_index[i]] = 
	    (t_d_i * bl_w[i] + t_d_ii * bl_w[ii]) / (bl_w[i] + bl_w[ii]); 
	  bl_w[i] += bl_w[ii];
	}
	bl[i] += bl[ii];
	finished = False;
      }
      i += bl[i];  
      if(i < ndistances) ii = i + bl[i];
    }
  }

  /* p-a-v sets only the first element of each block, so now we need to fill the blocks: */
  for (i = 0; i < ndistances; i = i + bl[i]) {
    for (j = i + 1; j < i + bl[i]; j++) {
      trans_dist[trans_dist_index[j]] = trans_dist[trans_dist_index[i]];
      bl[j] = 0; /* for debugging: blocks are easier to read w/o historic junk */
    }
  }

  /* mix isotonic with raw according to mds_isotonic_mix entered interactively */
  if(mds_isotonic_mix != 1.0) {
    for (i = 0 ; i < dist.nrows; i++) 
      for (j = 0; j < dist.ncols; j++) {
	ij = IJ;
	if(trans_dist[ij] != DBL_MAX) {
	  if(mds_power == 1.0) {
	    if(KruskalShepard_classic == KRUSKALSHEPARD) {
	      trans_dist[ij] = mds_isotonic_mix * trans_dist[ij] + 
		(1-mds_isotonic_mix) * dist.data[i][j];
	    } else {
	      trans_dist[ij] = mds_isotonic_mix * trans_dist[ij] - 
		(1-mds_isotonic_mix) * dist.data[i][j]*dist.data[i][j];
	    }
	  } else { /* mds_power != 1.0 */
	    if(KruskalShepard_classic == KRUSKALSHEPARD) {
	      trans_dist[ij] = mds_isotonic_mix * trans_dist[ij] + 
		(1-mds_isotonic_mix) * pow(dist.data[i][j], mds_power);
	    } else {
	      trans_dist[ij] = mds_isotonic_mix * trans_dist[ij] - 
		(1-mds_isotonic_mix) * pow(dist.data[i][j], 2*mds_power);
	    }
	  }
	} /* end if(trans_dist[ij] != DBL_MAX) { */
      } /* end for (j = 0; j < dist.ncols; j++) { */
  } /* end if(mds_isotonic_mix != 1.0) */

  update_dissim_plot();

} /* end isotonic_transform() */


/* ---------------------------------------------------------------- */
/*
 * Perform one loop of the iterative mds function.
 *
 * If doit is False, then we really want to determine the
 * stress function without doing anything to the gradient
*/
void
mds_once(Boolean doit)
{
  static int i, j, k, n;

  static Boolean gradient_p = True;
  static struct array gradient;
  
  static int prev_active_dist = -1;

  static double dist_config, dist_trans, resid, weight;
  static double step_mag, gsum, psum, gfactor;
  static double tmp;

  /* used in macros SAMEGLYPH and ACHORED  */
  xgobidata *xg = (xgobidata *) &xgobi;

  /* preparation for transformation */
  if (trans_dist == NULL) {
    /* transformation of raw_dist */
    trans_dist       = (double *) XtMalloc(ndistances * sizeof(double)); 
    /* distances of configuration points */
    config_dist      = (double *) XtMalloc(ndistances * sizeof(double)); 
  }
  /* initialize everytime we come thru because missings may change due to user interaction */
  for (i = 0 ; i < dist.nrows; i++) {
    for (j = 0; j < dist.ncols; j++) {
      config_dist[IJ] = DBL_MAX;
      trans_dist[IJ]  = DBL_MAX;
    } 
  }

  /* weight vector */
  set_weights();

  /* random selection vector */
  set_random_selection();

  /* -------- collect activity status for each point: excluded, erased, anchor (2), dragged ------- */
  if(point_status == NULL) point_status = (int *) XtMalloc(pos.nrows * sizeof(int)); 
  /* out */
  for(i=0; i<pos.nrows; i++) 
    point_status[i] = point_is_out;
  /* in */
  for(i=0; i<xg->nrows_in_plot; i++) { 
    n = xg->rows_in_plot[i]; 
    if(!xg->erased[n]) point_status[n] = point_is_in;
  }
  /* excluded points and anchors of either kind */  
  if (xg->ncols == xg->ncols_used) {
    for (i=0; i<pos.nrows; i++)
      if (xg->clusv[(int)GROUPID(i)].excluded == 1) 
        point_status[i] = point_is_out;
    if(anchor_group != NULL) {
      if (mds_group_ind == anchorfixed || mds_group_ind == anchorscales)
        for (i=0; i<pos.nrows; i++)
          if (point_status[i] != point_is_out &&
              anchor_group[(int) xg->raw_data[(i)][xg->ncols-1]])
          {
            point_status[i] = point_is_anchor;
          }
    }
  }
  /* dragged by mouse */
  if (xg->is_point_moving && moving_point != -1) {
    if(move_type==0) {
      point_status[moving_point] = point_is_dragged; }
    else if(move_type==1) {
      for (i=0; i<pos.nrows; i++)
	if (point_status[i] != point_is_out && SAMEGLYPH(i,moving_point)) 
	  point_status[i] = point_is_dragged; }
    else if(move_type==2) {
      for (i=0; i<pos.nrows; i++)
	if (point_status[i] != point_is_out)
	  point_status[i] = point_is_dragged; }
  }

  /* allocate position and compute means */
  get_center();

  /* --------------- collect and count active dissimilarities (j's move i's) --------------- */
  num_active_dist = 0;

  /* i's are moved by j's */
  for (i = 0; i < dist.nrows; i++) {
    /* do not exclude moving i's: in nonmetric MDS it matters what the set of distances is!  */

    /* these points are not moved by the gradient */
    if (point_status[i] == point_is_out || 
	(mds_group_ind == anchorfixed && point_status[i] == point_is_anchor) ||
	point_status[i] == point_is_dragged) 
      continue;

    /* j's are moving i's */    
    for (j = 0; j < dist.ncols; j++) {

      /* skip diagonal elements for distance scaling */
      if (i == j && KruskalShepard_classic == KRUSKALSHEPARD) continue; 

      /* these points do not contribute to the gradient */
      if (point_status[j] == point_is_out) continue;
      if ((mds_group_ind == anchorscales || mds_group_ind == anchorfixed) && 
          point_status[j] != point_is_anchor &&
          point_status[j] != point_is_dragged)
            continue;

      /* if the target distance is missing, skip */
      if (dist.data[i][j] == DBL_MAX) continue;

      /* if weight is zero, skip */
      if (weights != NULL && weights[IJ] == 0.) continue;

      /* using groups */
      if (mds_group_ind == within && !SAMEGLYPH(i,j)) continue;
      if (mds_group_ind == between && SAMEGLYPH(i,j)) continue;

      /*
       * if the target distance is within the thresholds
       * set using the barplot of distances, keep going.
       */
      if (dist.data[i][j] < mds_threshold_low || 
          dist.data[i][j] > mds_threshold_high) continue;

      /*
       * random selection: needs to be done symmetrically
       */
      if (mds_rand_select_val < 1.0) {
	if (i < j && rand_sel[IJ] > mds_rand_select_val) continue;
	if (i > j && rand_sel[JI] > mds_rand_select_val) continue;
      }

      /* 
       * zero weights:
       * assume weights exist if test is positive, and
       * can now assume that weights are >0 for non-NA
       */
      if(mds_weightpow != 0. || mds_within_between != 1.) {
	if(weights[IJ] == 0.) continue;
      }	

      /* another active dissimilarity */
      num_active_dist++;  

      /* configuration distance */
      if(KruskalShepard_classic == KRUSKALSHEPARD) {
	config_dist[IJ] = Lp_distance_pow(i, j);
	trans_dist[IJ]  = dist.data[i][j];	
      } else { /* CLASSIC */
	config_dist[IJ] = dot_prod(i, j);
	trans_dist[IJ]  = -dist.data[i][j]*dist.data[i][j];
      }
      /* store untransformed dissimilarity in transform vector for now:
       * METRIC will transform it; NONMETRIC will used it for sorting first.
       */

    } /* j */
  } /* i */
  /* -------------------- end collecting active dissimilarities ------------------ */


  /* --------------- for active dissimilarities, do some work --------------------- */ 
  if (num_active_dist > 0) {

    /* ---- power transform for metric MDS and isotonic transform for nonmetric*/
    if (metric_nonmetric == METRIC) {
      power_transform();
    } else { /* NONMETRIC */
      isotonic_transform();
    }

    /* ----------- stress (always lags behind gradient by one step) ------- */
    update_stress();

  } /* if (num_active_dist > 0) { */
    
  /* ------------------------- end active dissimilarities ------------------- */

  /* ---------- for active dissimilarities, do the gradient push if asked for ------ */
  if (doit && num_active_dist > 0) {

    /* all of the following need to be run thru rows_in_plot and erase !!!!!!!!!!! */

    /* Zero out the gradient matrix. */
    if (gradient_p) { 
      clear_array(&gradient);
      make_empty_array(&gradient, pos.nrows, pos.ncols);
      gradient_p = False;
    }
    zero_array(&gradient);

    /* ------------- gradient accumulation: j's push i's ----------- */
    for (i = 0; i < dist.nrows; i++) {
      for (j = 0; j < dist.ncols; j++) {

	dist_trans  = trans_dist[IJ];             	
	if (dist_trans  ==  DBL_MAX) continue;
        dist_config = config_dist[IJ];
	if(mds_weightpow == 0. && mds_within_between == 1.) { weight = 1.0; } else { weight = weights[IJ]; }

        /* gradient */
	if(KruskalShepard_classic == KRUSKALSHEPARD) {
	  if (fabs(dist_config) < delta) dist_config = delta;
	  /* scale independent version: */
          resid = (dist_trans - stress_dx / stress_xx * dist_config);
	  /**/
	  /* scale dependent version: 
	  resid = (dist_trans - dist_config);
	  */
	  if(mds_lnorm != 2) { /* non-Euclidean Minkowski/Lebesgue metric */
	    step_mag = weight * resid * pow(dist_config, 1 - mds_lnorm_over_distpow);
	    for (k = 0; k < mds_dims; k++) {
	      gradient.data[i][k] += 
		step_mag * 
		sig_pow(pos.data[i][k]-pos.data[j][k], mds_lnorm-1.0);
	    }
	  } else { /* Euclidean Minkowski/Lebesgue metric */
	    if(mds_distpow == 1)      { step_mag = weight * resid / dist_config; }
	    else if(mds_distpow == 2) { step_mag = weight * resid; }
	    else if(mds_distpow == 3) { step_mag = weight * resid * dist_config; }
	    else if(mds_distpow == 4) { step_mag = weight * resid * dist_config * dist_config; }
	    else                      { step_mag = weight * resid * pow(dist_config, mds_distpow-2.); }
	    for (k = 0; k < mds_dims; k++) {
	      gradient.data[i][k] += step_mag * (pos.data[i][k]-pos.data[j][k]); /* Euclidean! */
	    }
	  }
	} else { /* CLASSIC */
	  /* scale independent version: */
 	  resid = (dist_trans - stress_dx / stress_xx * dist_config);
	  /**/
	  /* scale dependent version:
	  resid = (dist_trans - dist_config);
	  */
	  step_mag = weight * resid; 
	  for (k = 0; k < mds_dims; k++) {
	    gradient.data[i][k] += step_mag * (pos.data[j][k] - pos_mean[k]);
	      /* exact formula would be:
	      ((1-1/pos.nrows)*pos.data[j][k] - (1-2/pos.nrows)*pos_mean[k] - pos.data[i][k]/pos.nrows); 
	      */
	  }
	}

      } /* for (j = 0; j < dist.nrows; j++) { */
    } /* for (i = 0; i < dist.nrows; i++) { */
    /* ------------- end gradient accumulation ----------- */   

    /* center the classical gradient */
    if(KruskalShepard_classic == CLASSIC)
      for(k=0; k<mds_dims; k++) {
	tmp = 0.;  n = 0;
	for (i=0; i<pos.nrows; i++)
	  if (point_status[i] == point_is_in || 
	      (mds_group_ind == anchorscales && point_status[i] == point_is_anchor)) 
	    {
	    tmp += gradient.data[i][k];  n++;
	    }
	tmp /= n;
	for (i=0; i<pos.nrows; i++)
	  if (point_status[i] == point_is_in || 
	      (mds_group_ind == anchorscales && point_status[i] == point_is_anchor)) 
	    gradient.data[i][k] -= tmp;	
      }

    /* gradient normalizing factor to scale gradient to a fraction of the size of the configuration */
    gsum = psum = 0.0 ;
    for (i=0; i<pos.nrows; i++) {
      if (point_status[i] == point_is_in || 
	  (mds_group_ind == anchorscales && point_status[i] == point_is_anchor)) 
	{
	gsum += L2_norm(gradient.data[i]);
	psum += L2_norm(pos.data[i]);
	}
    }
    if (gsum < delta) gfactor = 0.0;
    else gfactor = mds_stepsize * sqrt(psum/gsum);

    /* add the gradient matrix to the position matrix and drag points */
    for (i=0; i<pos.nrows; i++) {
      if(point_status[i] != point_is_dragged) {
        for (k=mds_freeze_var; k<mds_dims; k++)
          pos.data[i][k] += (gfactor * gradient.data[i][k]);
      } else {
        for (k=0; k < mds_dims; k++) 
          pos.data[i][k] = xg->raw_data[i][k] ;
      }
    }

    /* experiment: normalize point cloud after using simplified gradient */
    center_scale_pos();

  } /*   if (doit && num_active_dist > 0) { */

  /* update Shepard labels */
  if(num_active_dist != prev_active_dist) {
    update_shepard_labels(num_active_dist);
    prev_active_dist = num_active_dist;
  }

} /* end mds_once() */

/* ---------------------------------------------------------------- */









