/***********************************************************************
 * Permission is hereby granted to any individual or institution       *
 * for use, copying, or redistribution of the AV2XGobi C code          *
 * and associated documentation, provided such code and documentation  *
 * are not sold for profit and the following copyright notice is       *
 * retained in the code and documentation:                             *
 *                                                                     *
 *   Copyright (c) 1995, 1996, 1997 Iowa State University              *
 *                                                                     *
 * We encourage you to share questions, comments and modifications.    *
 *                                                                     *
 *   Juergen Symanzik (symanzik@iastate.edu)                           *
 *   Dianne Cook (dicook@iastate.edu)                                  *
 *   James J. Majure (majure@iastate.edu)                              *
 *   Inna Megretskaia                                                  *
 *   Philip G. Jones                                                   *
 *                                                                     *
 ***********************************************************************/
/* !!!MS!!! */
#if defined RPC_USED || defined DCE_RPC_USED

#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include "xincludes.h"
#include <X11/keysym.h>
#include "xgobitypes.h"
#include "xgobivars.h"

typedef struct{
        int fir_obs;
        int sec_obs;
      } vario_lag_record;

static int l = sizeof (unsigned int) * 8;



/* getbit: get bit from position p in unsigned int x */

unsigned getbit (x, p)

unsigned int x;
int p;
   
{ 
  return (x>>p&1);
}



/* setbit1: set to 1 bit in position p */

void setbit1(x, p)

unsigned int *x;
int p;

{
  *x = *x | (1<<p);
}



/* setbit0: set to 0 bit in position p */

void setbit0 (x, p)

unsigned int *x;
int p;

{
  *x = *x & (~(1<<p));
}  



void ecdf (z, s, n, p, num_pts)

     float **z;
     int *s;
     int n, p;
     int *num_pts;

/*
   z    = (p+1) x n matrix of attributes, 
          column 0 contains ecdf values when done
   s    = n x 1 vector of location indicators
   n    = number of points in z
   p    = number of attributes
   num_pts = number of values for which s = 1         

   z[][0] returns the vector of empirical cdf values evaluated at
   the points in z indicated by the vector s (i.e., those
   z-values for which s = 1). 
   Ecdf values are present only for those points
   corresponding to s = 1; thus z[i][0] = ecdf value only if
   s[i] = 1; otherwise z[i][0] = 0.
*/

{
  int j, incl_pt;
  int i, k, indic;

  *num_pts = 0;
  for (i = 0; i < n; i++)
    if (s[i])
      (*num_pts)++;

  for (k=0; k < n; k++)
  {
    if (!s[k])
      z[k][0] = 0.0;
    else
    {
      indic = 0;
      for (i = 0; i < n; i++)
	if (s[i])
	{
	  incl_pt = 1;
	  for (j = 1; j < p + 1; j++)
	    if (z[i][j] > z[k][j])
	      {
		incl_pt = 0;
		break;
	      }
	  if (incl_pt)
	    indic++;
	}
      z[k][0] = ((float) indic) / ((float) (*num_pts));
    }
  }
}



void weighted_ecdf (z, s, n, p, num_pts, weights)

     float **z;
     int *s;
     int n, p;
     int *num_pts;
     double *weights;

/*
   z    = (p+1) x n matrix of attributes, 
          column 0 contains ecdf values when done
   s    = n x 1 vector of location indicators
   n    = number of points in z
   p    = number of attributes
   num_pts = number of values for which s = 1
   weights = n x 1 vector of weights      

   z[][0] returns the vector of empirical cdf values evaluated at
   the points in z indicated by the vector s (i.e., those
   z-values for which s = 1). 
   Ecdf values are present only for those points
   corresponding to s = 1; thus z[i][0] = ecdf value only if
   s[i] = 1; otherwise z[i][0] = 0.
*/

{
  int i, j, k, incl_pt;
  double indic, totalweight;

  *num_pts = 0;
  totalweight = 0.0;
  for (i = 0; i < n; i++)
    if (s[i])
    {
      (*num_pts)++;
      totalweight += weights[i];
    }

  if (totalweight == 0.0)
    totalweight = 1.0;

  for (k = 0; k < n; k++)
  {
    if (!s[k])
      z[k][0] = 0.0;
    else
    {
      indic = 0.0;
      for (i = 0; i < n; i++)
	if (s[i])
	{
	  incl_pt = 1;
	  for (j = 1; j < p + 1; j++)
	    if (z[i][j] > z[k][j])
	      {
		incl_pt = 0;
		break;
	      }
	  if (incl_pt)
	    indic += weights[i];
	}
      z[k][0] = (float) (indic / totalweight);
    }
  }
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * This function constructs binary map for all pairs of records      *
 * in the initial table ( bit, that corresponds to pair (i,j) is set *       
 * to 1 if sqrt((z[i][0]-z[j][0])^2+(z[i][1]-z[j][1])^2)<=maxdist or *
 * to 0 otherwise.                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void construct_vario_lag_bitmap (n, p, z, miss, maxdist, num_prs, np_e_p1, np_e_p2, bin_map)

int n, p;
float **z;
short **miss;
float maxdist;
int *num_prs;
int *np_e_p1;
int *np_e_p2;
unsigned int **bin_map;


/* z - nx(p+2) matrix of attributes
   z[i][1],z[i][2]         - coordinates of point #i
   z[i][3], ..., z[i][p+2] - different statistics measured at this point
   n       - number of rows in z
   maxdist - cutoff parameter (we consider only those pairs of points
             for which eucledian distance between them is smaller than 
             or equal to maxdist)
 
   bin_map - binary matrix nxn, organized as integer
             matrix n x ceil(n/sizeof(int))
*/
 
{
  double maxdist2 = maxdist * maxdist;
  int i, j; 
  double x, y;

  
  for (i = 0; i < n; i++)
    for (j = i; j < n ; j++)
      if (! miss[i][0] && ! miss[j][0] && ! miss[i][1] && ! miss[j][1])
      { 
        x = z[i][0] - z[j][0];
        y = z[i][1] - z[j][1];

        if ((x * x + y * y) <= maxdist2)
        { 
          setbit1 (&bin_map[i][j/l], j%l);
          ++(*num_prs);
          ++np_e_p1[i];
          ++np_e_p2[j];

          if (i != j)
          {
            setbit1 (&bin_map[j][i/l], i%l);
            ++(*num_prs);
            ++np_e_p1[j];
            ++np_e_p2[i];
          }
        }
      }  
} 



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  This function constructs table of records corresponding to all       *
 *  pairs of points in initial table that satisfy the cutoff condition   *
 *  and calculates p*p+3 different statistics for each of these          *
 *  pairs of points.                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void  construct_vario_data (n, p, z, miss, datap, missingp,
        missing_present, num_missing,
        np_e_p1, np_e_p2, bin_map, w, ptarr1, ptarr2)

int n, p;
float **z;
short **miss;
float **datap;
short **missingp;
Boolean *missing_present;
int *num_missing;
int *np_e_p1;
int *np_e_p2;
unsigned int **bin_map;
vario_lag_record *w;
int **ptarr1;
int **ptarr2;

/* z - n x (p+2) matrix of attributes  
   n - number of rows in z
   p - number of statistics measured in each point
*/

{
  int i, j, m, s, mm, ss, numb;
  float a;
  double dx, dy, d, sum, msum, gms;
  int p1 = p + 1;
  int p2 = p + 2;
  int n_obs = 0;


  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      if (getbit (bin_map[i][j/l], j%l))
      {
        w[n_obs].fir_obs = i;
        w[n_obs].sec_obs = j;
        dx = -z[i][0] + z[j][0];
        dy = -z[i][1] + z[j][1];
        d = sqrt (dx * dx + dy * dy);
        datap[n_obs][0] = (float) d;
        missingp[n_obs][0] = 0;

        if ((d == 0) && (j == i))
        {
          datap[n_obs][1] = - 90.0;
          missingp[n_obs][1] = 0;
          datap[n_obs][2] = 0;
          missingp[n_obs][2] = 0;
          datap[n_obs][3] = - 0.5;
          missingp[n_obs][3] = 0;
        }
        if ((d == 0) && (j != i))
        {
          datap[n_obs][1] = - 45.0;
          missingp[n_obs][1] = 0;
          datap[n_obs][2] = 0;
          missingp[n_obs][2] = 0;
          datap[n_obs][3] = 0.5;
          missingp[n_obs][3] = 0;
        }
        if (d != 0)
        { 
          a = (float) (acos (dx / d) / M_PI * 180.0);
          if (dy < 0)
            a = 360.0 - a;
          datap[n_obs][1] = a; 
          missingp[n_obs][1] = 0;
          datap[n_obs][2] = (float) (dy / d);
          missingp[n_obs][2] = 0;
          datap[n_obs][3] = (float) (dx / d);
          missingp[n_obs][3] = 0;
        }

        numb = 4;
        for (m = 2; m < p2; m++)
          for (s = 2; s < p2; s++)
          { 
            if (! miss[i][m] && ! miss[j][s])
            {
              gms = fabs ((double) (z[i][m] - z[j][s]));
              datap[n_obs][numb] = (float) sqrt (gms);
              missingp[n_obs][numb] = 0;
            }
            else
              {
                datap[n_obs][numb] = 0.0;
                missingp[n_obs][numb] = 1;
                *missing_present = True;
                *num_missing++;
              }
            numb++;
          }            

        sum = pow ((double) datap[n_obs][4], 4.0);
        msum = missingp[n_obs][4];
        for (m = 1; m < p; m++)
        {
          sum += pow ((double) datap[n_obs][4 + m * p1], 4.0);
          msum += missingp[n_obs][4 + m * p1];
        }
        if (! msum)
        {
          datap[n_obs][numb] = (float) sqrt (sum);
          missingp[n_obs][numb] = 0;
        }
        else
          {
            datap[n_obs][numb] = 0.0;
            missingp[n_obs][numb] = 1;
            *missing_present = True;
            *num_missing++;
          }

        mm = np_e_p1[i]++;
        ss = np_e_p2[j]++;
        ptarr1[i][mm] = ptarr2[j][ss] = n_obs;

        n_obs++; 
      }
}                



void  construct_vario2_data (n, p, z, miss, datap, missingp,
        missing_present, num_missing,
        np_e_p1, np_e_p2, bin_map, w, ptarr1, ptarr2)

int n, p;
float **z;
short **miss;
float **datap;
short **missingp;
Boolean *missing_present;
int *num_missing;
int *np_e_p1;
int *np_e_p2;
unsigned int **bin_map;
vario_lag_record *w;
int **ptarr1;
int **ptarr2;

/* z - n x (p+2) matrix of attributes  
   n - number of rows in z
   p - number of statistics measured in each point
*/

{
  int i, j, m, s, mm, ss, numb;
  float a;
  double dx, dy, d, sum, msum, gms;
  int p2 = p + 2;
  int n_obs = 0;


  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      if (getbit (bin_map[i][j/l], j%l))
      {
        w[n_obs].fir_obs = i;
        w[n_obs].sec_obs = j;
        dx = -z[i][0] + z[j][0];
        dy = -z[i][1] + z[j][1];
        d = sqrt (dx * dx + dy * dy);
        if ((dx > 0) || ((dx == 0) && (dy >= 0)))
          datap[n_obs][0] = (float) d;
        else
          datap[n_obs][0] = (float) (- d);
        missingp[n_obs][0] = 0;

        if ((d == 0) && (j == i))
        {
          datap[n_obs][1] = - 90.0;
          missingp[n_obs][1] = 0;
          datap[n_obs][2] = 0;
          missingp[n_obs][2] = 0;
          datap[n_obs][3] = - 0.5;
          missingp[n_obs][3] = 0;
        }
        if ((d == 0) && (j != i))
        {
          datap[n_obs][1] = - 45.0;
          missingp[n_obs][1] = 0;
          datap[n_obs][2] = 0;
          missingp[n_obs][2] = 0;
          datap[n_obs][3] = 0.5;
          missingp[n_obs][3] = 0;
        }
        if (d != 0)
        { 
          a = (float) (acos (dx / d) / M_PI * 180.0);
          if (dy < 0)
            a = 360.0 - a;
          datap[n_obs][1] = a; 
          missingp[n_obs][1] = 0;
          datap[n_obs][2] = (float) (dy / d);
          missingp[n_obs][2] = 0;
          datap[n_obs][3] = (float) (dx / d);
          missingp[n_obs][3] = 0;
        }

        numb = 4;
        for (m = 2; m < p2; m++)
          for (s = m; s < p2; s++)
          { 
            if (! miss[i][m] && ! miss[j][s])
            {
              gms = fabs ((double) (z[i][m] - z[j][s]));
              datap[n_obs][numb] = (float) sqrt (gms);
              missingp[n_obs][numb] = 0;
            }
            else
              {
                datap[n_obs][numb] = 0.0;
                missingp[n_obs][numb] = 1;
                *missing_present = True;
                *num_missing++;
              }
            numb++;
          }            

        sum = pow ((double) datap[n_obs][4], 4.0);
        msum = missingp[n_obs][4];
        for (m = 1; m < p; m++)
        {
          sum += pow ((double) datap[n_obs][4 + m * p - m * (m - 1) / 2], 4.0);
          msum += missingp[n_obs][4 + m * p - m * (m - 1) / 2];
        }
        if (! msum)
        {
          datap[n_obs][numb] = (float) sqrt (sum);
          missingp[n_obs][numb] = 0;
        }
        else
          {
            datap[n_obs][numb] = 0.0;
            missingp[n_obs][numb] = 1;
            *missing_present = True;
            *num_missing++;
          }

        mm = np_e_p1[i]++;
        ss = np_e_p2[j]++;
        ptarr1[i][mm] = ptarr2[j][ss] = n_obs;

        n_obs++; 
      }
} 


void  construct_vario2_data_old (n, p, z, miss, datap, missingp,
        missing_present, num_missing,
        np_e_p1, np_e_p2, bin_map, w, ptarr1, ptarr2)

int n, p;
float **z;
short **miss;
float **datap;
short **missingp;
Boolean *missing_present;
int *num_missing;
int *np_e_p1;
int *np_e_p2;
unsigned int **bin_map;
vario_lag_record *w;
int **ptarr1;
int **ptarr2;

/* z - n x (p+2) matrix of attributes  
   n - number of rows in z
   p - number of statistics measured in each point
*/

{
  int i, j, m, s, mm, ss, mm1, ss1, numb;
  float a;
  double dx, dy, d, sum, sum1, gms;
  int p2 = p + 2;
  int n_obs = 0;
  int n_obs1; 


  for (i = 0; i < n; i++)
    for (j = i; j < n; j++)
      if (i == j)
        {
          w[n_obs].fir_obs = i;
          w[n_obs].sec_obs = i;
          datap[n_obs][0] = 0.0;
          datap[n_obs][1] = - 90.0;
          datap[n_obs][2] = 0;
          datap[n_obs][3] = - 0.5;

          numb = 4;
          for (m = 2; m < p2; m++)
            for (s = m; s < p2; s++)
            { 
              gms = fabs ((double) (z[i][m] - z[j][s]));
              datap[n_obs][numb] = (float) sqrt (gms);
              numb++;
            }            

          sum = pow ((double) datap[n_obs][4], 4.0);
          for (m = 1; m < p; m++)
            sum += pow ((double) datap[n_obs][4 + m * p - m * (m - 1) / 2], 4.0);
          datap[n_obs][numb] = (float) sqrt (sum); 

          mm = np_e_p1[i]++;
          ss = np_e_p2[j]++;
          ptarr1[i][mm] = ptarr2[j][ss] = n_obs;

          n_obs++;
        } 
      else if (getbit (bin_map[i][j/l], j%l))
      {
        n_obs1 = n_obs + 1;
        w[n_obs].fir_obs = i;
        w[n_obs].sec_obs = j;
        w[n_obs1].fir_obs = j;
        w[n_obs1].sec_obs = i;
        dx = -z[i][0] + z[j][0];
        dy = -z[i][1] + z[j][1];
        d = sqrt (dx * dx + dy * dy);
        if (dx < 0)
          d = -d;
        datap[n_obs][0] = (float) d;
        datap[n_obs1][0] = (float) -d;
    
        if (d == 0)
        {
          datap[n_obs][1] = - 45.0;
          datap[n_obs][2] = 0;
          datap[n_obs][3] = 0.5;
          datap[n_obs1][1] = - 45.0;
          datap[n_obs1][2] = 0;
          datap[n_obs1][3] = 0.5;
        }
        if (d != 0)
        { 
          a = (float) (acos (dx / d) / M_PI * 180.0);
          if (dy < 0)
            a = 180.0 - a;
          datap[n_obs][1] = a; 
          datap[n_obs][2] = (float) sin (2.0 * a);
          datap[n_obs][3] = (float) cos (2.0 * a);
          datap[n_obs1][1] = a; 
          datap[n_obs1][2] = (float) sin (2.0 * a);
          datap[n_obs1][3] = (float) cos (2.0 * a);
        }

        numb = 4;
        for (m = 2; m < p2; m++)
          for (s = m; s < p2; s++)
          { 
            gms = fabs ((double) (z[i][m] - z[j][s]));
            datap[n_obs][numb] = (float) sqrt (gms);
            gms = fabs ((double) (z[i][s] - z[j][m]));
            datap[n_obs1][numb] = (float) sqrt (gms);
            numb++;
          }            

        sum = pow ((double) datap[n_obs][4], 4.0);
        sum1 = pow ((double) datap[n_obs1][4], 4.0);
        for (m = 1; m < p; m++)
        {
          sum += pow ((double) datap[n_obs][4 + m * p - m * (m - 1) / 2], 4.0);
          sum1 += pow ((double) datap[n_obs1][4 + m * p - m * (m - 1) / 2], 4.0);
        }
        datap[n_obs][numb] = (float) sqrt (sum);
        datap[n_obs1][numb] = (float) sqrt (sum1); 

        mm = np_e_p1[i]++;
        ss = np_e_p2[j]++;
        ptarr1[i][mm] = ptarr2[j][ss] = n_obs;
        mm1 = np_e_p1[j]++;
        ss1 = np_e_p2[i]++;
        ptarr1[j][mm1] = ptarr2[i][ss1] = n_obs1;

        n_obs = n_obs + 2; 
      }
}                



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  This function constructs table of records corresponding to all       *
 *  pairs of points in initial table that satisfy the cutoff condition   *
 *  and calculates p*2+2 different statistics for each of these          *
 *  pairs of points.                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void  construct_lag_data (n, p, z, miss, datap, missingp,
        missing_present, num_missing,
        np_e_p1, np_e_p2, bin_map, w, ptarr1, ptarr2)

int n, p;
float **z;
short **miss;
float **datap;
short **missingp;
Boolean *missing_present;
int *num_missing;
int *np_e_p1;
int *np_e_p2;
unsigned int **bin_map;
vario_lag_record *w;
int **ptarr1;
int **ptarr2;

/* z - n x (p+2) matrix of attributes  
   n - number of rows in z
   p - number of statistics measured in each point
*/

{
  int i, j, m, s, mm, ss, numb;
  float a;
  double dx, dy, d, sum, gms;
  int p2 = p + 2;
  int n_obs = 0;


  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      if (getbit (bin_map[i][j/l], j%l))
      {
        w[n_obs].fir_obs = i;
        w[n_obs].sec_obs = j;
        dx = -z[i][0] + z[j][0];
        dy = -z[i][1] + z[j][1];
        d = sqrt (dx * dx + dy * dy);
        datap[n_obs][0] = (float) d;
        missingp[n_obs][0] = 0;

        if ((d == 0) && (j == i))
        {
          datap[n_obs][1] = - 90.0;
          missingp[n_obs][1] = 0;
          datap[n_obs][2] = 0;
          missingp[n_obs][2] = 0;
          datap[n_obs][3] = - 0.5;
          missingp[n_obs][3] = 0;
        }
        if ((d == 0) && (j != i))
        {
          datap[n_obs][1] = - 45.0;
          missingp[n_obs][1] = 0;
          datap[n_obs][2] = 0;
          missingp[n_obs][2] = 0;
          datap[n_obs][3] = 0.5;
          missingp[n_obs][3] = 0;
        }
        if (d != 0)
        {         
          a = (float) (acos (dx / d) / M_PI * 180.0);
          if (dy < 0)
            a = 360.0 - a;
          datap[n_obs][1] = a;
          missingp[n_obs][1] = 0;
          datap[n_obs][2] = (float) (dy / d);
          missingp[n_obs][2] = 0;
          datap[n_obs][3] = (float) (dx / d); 
          missingp[n_obs][3] = 0;
        }

        numb = 4;
        for (m = 2; m < p2; m++)
        { 
          if (! miss[i][m])
          {
            datap[n_obs][numb] = z[i][m];
            missingp[n_obs][numb] = 0;
          }
          else
            {
              datap[n_obs][numb] = 0.0;
              missingp[n_obs][numb] = 1;
              *missing_present = True;
              *num_missing++;
            }
          numb++;

          if (! miss[j][m])
          {
            datap[n_obs][numb] = z[j][m];
            missingp[n_obs][numb] = 0;
          }
          else
            {
              datap[n_obs][numb] = 0.0;
              missingp[n_obs][numb] = 1;
              *missing_present = True;
              *num_missing++;
            }
          numb++;
        }            

        mm = np_e_p1[i]++;
        ss = np_e_p2[j]++;
        ptarr1[i][mm] = ptarr2[j][ss] = n_obs;

        n_obs++; 
      }
}

#endif
