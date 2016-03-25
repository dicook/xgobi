#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "xincludes.h"
#include "xgobitypes.h"
#include "xgobivars.h"
#include "xgobiexterns.h"
/*
#include <stdlib.h>
*/

extern int *grp_id_sm;
extern int *n_smgp_pts;
extern long *x_sm;
extern long *y_sm;

int 
point_compar(const void *arg1, const void *arg2)
{ 
  register long **el1 = (long **) arg1;
  register long **el2 = (long **) arg2;

  if (**el1 < **el2)
    return(-1);
  else if (**el1 == **el2 )
    return(0);
  else 
    return(1);
}

void 
mysort(long **point_x, int n)
{ 
  qsort((char *) point_x, n, sizeof(long), point_compar);
}

void 
median_smoother(long *x, long *y, int n, long *sm_pars, int num_grp,
unsigned long *grp_id, int *pnum_pts, int max_pts)
{
  int i,j,k,m,p,s,r,r1,s1,n_pts_sm,p1,p2;
  long left,right,upbound,x_cent;
  int num_pts;
  long *left_bound,*right_bound;
  long **x_ord, **y_ord;
  int *y_2, *count, **med_grp;
  long step, width,width2;

  if (x_sm != NULL)
    XtFree((XtPointer) x_sm);
  if (y_sm != NULL)
    XtFree((XtPointer) y_sm);
  if (grp_id_sm != NULL)
    XtFree((XtPointer) grp_id_sm);
  if (n_smgp_pts != NULL)
    XtFree((XtPointer) n_smgp_pts);

  x_ord=(long **) XtMalloc((Cardinal) n*sizeof(long *));
  y_ord=(long **) XtMalloc((Cardinal) n*sizeof(long *));
  n_smgp_pts=(int *) XtMalloc((Cardinal) num_grp*sizeof(int));
  left_bound=(long *) XtMalloc((Cardinal) num_grp*sizeof(long));
  right_bound=(long *) XtMalloc((Cardinal) num_grp*sizeof(long));
  y_2=(int *) XtMalloc((Cardinal) n*sizeof(int));
  count=(int *) XtMalloc((Cardinal) num_grp*sizeof(int));
  med_grp=(int **) XtMalloc((Cardinal) num_grp*sizeof(int *));

  num_pts = *pnum_pts;
  step = sm_pars[0];
  width = sm_pars[1];
  width2=width/2;

  for(i=0;i<num_grp;i++)
    med_grp[i]=(int *) XtMalloc((Cardinal) n*sizeof(int ));
  for(i=0;i<num_grp;i++)
    for(j=0;j<n;j++)
      med_grp[i][j]=0;
  for(i=0;i<num_grp;i++)
    count[i]=0;

  for(i=0;i<n;i++)
    x_ord[i]=&x[i];
  for(i=0;i<n;i++)
    y_ord[i]=&y[i];
  mysort(x_ord,n);
  mysort(y_ord,n);

  for(i=0;i<num_grp;i++)
  { 
    left_bound[i]=*x_ord[n-1];
    right_bound[i]=*x_ord[0];
    n_smgp_pts[i]=0;
  }

  for(i=0;i<n;i++)
  { 
    m = (int) grp_id[i]-1;
    if (x[i]<left_bound[m])
      left_bound[m]=x[i];
    if (x[i]>right_bound[m])
      right_bound[m]=x[i]; 
  }

  for (i=0;i<num_grp;i++)
  { 
    left_bound[i]=left_bound[i]+width2;
    right_bound[i]=right_bound[i]-width2;
  }

  for (j=0;j<n;j++)
  { 
    i=y_ord[j]-&y[0];
    y_2[i]=j;
  }
  left=*x_ord[0];
  right=left+width;
  upbound=*x_ord[n-1];
  x_cent=*x_ord[0]+width2;
  if (right>upbound) 
  { 
    printf("Window is too wide\n");
    exit(0);
  }
  n_pts_sm= ((int) ceil((double) (*x_ord[n-1]-*x_ord[0]-width)/ 
    (double) step +1))*num_grp;
  x_sm=(long *) XtMalloc ((Cardinal) n_pts_sm*sizeof(long));
  y_sm=(long *) XtMalloc ((Cardinal) n_pts_sm*sizeof(long));
  grp_id_sm=(int *) XtMalloc ((Cardinal) n_pts_sm*sizeof(int));
  
  k=0;
  while (right<=upbound)
  { 
    i=k;
    while (*x_ord[i]<=right)
    {
      j=x_ord[i]-&x[0];
      m = (int) grp_id[j];
      p=y_2[j];
      med_grp[m-1][p]=1;
      count[m-1]++;
      i++;
      if (i==n) break;
    }
    for (m=0;m<num_grp;m++)
    { 
      if ((count[m]>0) && (x_cent>=left_bound[m]) && 
         (x_cent<=right_bound[m]))
      { 
        s=(int) floor((double) count[m]/2);
        r=0; 
        p=0;
        if (count[m]==1) 
          r=-1; 
        while ((r<s) && (p<n))
        { 
          r=r+med_grp[m][p];
          p++;
        }
        if (count[m]==1) 
          p2=p-1;
        else
        { 
          p1=p-1;
          r1=r; 
          s1=s+1;
          while ((r1<s1) && (p<n))
          { 
            r1=r1+med_grp[m][p];
            p++;
          }
          p2=p-1; 
        }
        x_sm[num_pts]=x_cent;
        if (s*2==count[m])
          y_sm[num_pts]=(long) (*y_ord[p1]+*y_ord[p2])/2;
        else
          y_sm[num_pts]=*y_ord[p2];
        grp_id_sm[num_pts]=m+1; 
        num_pts++;
        n_smgp_pts[m]++;
      }
      count[m]=0;
    }
    if (upbound-right>=step)
      x_cent=x_cent+step;
    else 
      break;
    right=right+step;
    left=left+step;
    while(*x_ord[k]<left)
      if (k<n-1) 
        k++;
    for (m=0;m<num_grp;m++)
      for(j=0;j<n;j++)
        med_grp[m][j]=0;
    
    /* Temporary fix for a dynamic allocation problem */
    if (num_pts >= max_pts)
      break;
    /* */
  }
  *pnum_pts = num_pts;  
  XtFree((XtPointer) x_ord);
  XtFree((XtPointer) y_ord);
  XtFree((XtPointer) y_2);
  XtFree((XtPointer) count);
  for (i=0; i< num_grp; i++)
    XtFree((XtPointer) med_grp[i]);
  XtFree((XtPointer) med_grp);
  XtFree((XtPointer) left_bound);
  XtFree((XtPointer) right_bound);
}

void mean_smoother(long *x, long *y, int n, long *sm_pars, int num_grp,
unsigned long *grp_id, int *pnum_pts, int max_pts)
{
  int i,j,k,m,p;
  double *sum;
  long left,right,upbound,x_cent;
  int n_pts_sm=0;
  int num_pts;
  long **x_ord, **y_ord;
  long *left_bound, *right_bound;
  int *y_2, *count;
  long step, width, width2;

  if (x_sm != NULL)
    XtFree((XtPointer) x_sm);
  if (y_sm != NULL)
    XtFree((XtPointer) y_sm);
  if (grp_id_sm != NULL)
    XtFree((XtPointer) grp_id_sm);
  if (n_smgp_pts != NULL)
    XtFree((XtPointer) n_smgp_pts);

  sum=(double *) XtMalloc((Cardinal) num_grp*sizeof(double));
  x_ord=(long **) XtMalloc((Cardinal) n*sizeof(long *));
  y_ord=(long **) XtMalloc((Cardinal) n*sizeof(long *));
  n_smgp_pts=(int *) XtMalloc((Cardinal) num_grp*sizeof(int));
  left_bound=(long *) XtMalloc((Cardinal) num_grp*sizeof(long));
  right_bound=(long *) XtMalloc((Cardinal) num_grp*sizeof(long));
  y_2=(int *) XtMalloc((Cardinal) n*sizeof(int));
  count=(int *) XtMalloc((Cardinal) num_grp*sizeof(int));

  num_pts = *pnum_pts;
  step = sm_pars[0];
  width = sm_pars[1];
  width2=width/2;

  for (i=0;i<num_grp;i++)
    count[i]=0;
  for (i=0;i<n;i++)
    x_ord[i]=&x[i];
  for (i=0;i<n;i++)
    y_ord[i]=&y[i];
  mysort(x_ord,n);
  mysort(y_ord,n);

  for(i=0;i<num_grp;i++)
  { 
    left_bound[i]=*x_ord[n-1];
    right_bound[i]=*x_ord[0];
    n_smgp_pts[i]=0;
  }

  for (i=0;i<n;i++)
  { 
    m = (int) grp_id[i]-1;
    if (x[i]<left_bound[m])
      left_bound[m]=x[i];
    if (x[i]>right_bound[m])
      right_bound[m]=x[i]; 
  }

  for (i=0;i<num_grp;i++)
  { 
    left_bound[i]=left_bound[i]+width2;
    right_bound[i]=right_bound[i]-width2;
  }

  for (j=0;j<n;j++)
  { 
    i=y_ord[j]-&y[0];
    y_2[i]=j;
  }

  left=*x_ord[0];
  right=left+width;
  upbound=*x_ord[n-1];
  x_cent=*x_ord[0]+width2;
  if (right>upbound) 
  { 
    printf("Window is too wide\n");
    exit(0);
  }
  n_pts_sm= ((int) ceil((double) (*x_ord[n-1]-*x_ord[0]-width)/ 
   (double) step +1))*num_grp;
  x_sm=(long *) XtMalloc((Cardinal) n_pts_sm*sizeof(long));
  y_sm=(long *) XtMalloc((Cardinal) n_pts_sm*sizeof(long));
  grp_id_sm=(int *) XtMalloc((Cardinal) n_pts_sm*sizeof(int));

  for(j=0;j<num_grp;j++)
    sum[j]=0;
  k=0;
  while (right<=upbound)
  { 
    i=k;
    while (*x_ord[i]<=right)
    {
      j=x_ord[i]-&x[0];
      m = (int) grp_id[j];
      p=y_2[j];
      sum[m-1]=sum[m-1]+(double) *y_ord[p];
      count[m-1]++;
      i++;
      if (i==n) 
        break;
    }
    for (m=0;m<num_grp;m++)
    { 
      if ((count[m]>0) && (x_cent>left_bound[m]) && (x_cent<right_bound[m]))
      {              
        x_sm[num_pts]=x_cent;
        y_sm[num_pts]= ( long ) sum[m]/count[m];
        grp_id_sm[num_pts]=m+1; num_pts++;
        n_smgp_pts[m]++;
        count[m]=0;
        sum[m]=0;
      }
    }
    if (upbound-right>=step)
      x_cent=x_cent+step;
    else 
      break;
    right=right+step;
    left=left+step;
    while (*x_ord[k]<left)
      if (k<n-1) k++;

    /* Temporary fix for a dynamic allocation problem */
    if (num_pts >= max_pts)
      break;
    /* */
  }

  *pnum_pts = num_pts;  
  XtFree((XtPointer) sum);
  XtFree((XtPointer) x_ord);
  XtFree((XtPointer) y_ord);
  XtFree((XtPointer) y_2);
  XtFree((XtPointer) count);
  XtFree((XtPointer) left_bound);
  XtFree((XtPointer) right_bound);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Function smoother() allows to do different kind of smoothing of the data *
* based on different algorithms.                                           *
* smtype - controlling parameter for smoothing algorithm                   *
*   smtype=0 - "mean" smoothing                                            *
*   smtype=1 - "median" smoothing                                          *
*   smtype=3                                                               *
* z(2xn) - initial vector of points in 2 dimentional space                 *
* n    - number of points given originally                                 *
* grp_id(1xn) - group id - number identifyihg to what group each           *
*                point belong                                              *
* smpar - array of parameters                                              *
* n-pts_sm - number of points for smoothing lines                          *
* z_sm(2xn_pts_sm) - array of points for smoothing curves                  *
* grp_id_sm(1xn_pts_sm) array of group identities for points in z_sm       *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

