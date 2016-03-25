#ifndef _XGOBISERVER_H_
#define _XGOBISERVER_H_

#include <stdio.h>

#include "RSCorba.h"
#include "Sequences.h"
//#include "Matrix.h"      // from IDLTypes
#include "BaseMatrixImpl.h"      // from IDLTypes


#include "XGobiCorbaServer_skel.h"
#include "xgobitypes.h"
#include "xgobivars.h"

#include "Sequences.h"
#include <stdlib.h>
extern "C" {
extern int read_data_from_file(char *filename, xgobidata *xg);
extern void update_world(xgobidata *);
extern void alloc_block(int, xgobidata *);
extern void realloc_lines(xgobidata *);
extern int glyphIDfromName(char *);
extern void refreshXGobiDisplay(xgobidata *xg);
extern void plot_once(xgobidata *);

extern void refresh_vbox(xgobidata *, int, int);
extern void refresh_vlab(xgobidata *, int);

extern char **GetPlotViewTypes(int *num);
extern int SetPlotModeName(const char *name, xgobidata *xg) ;

extern void update_cprof_selectedvars(xgobidata *);
extern void update_cprof_plot(xgobidata *);
extern void reset_nvars_cprof_plot(xgobidata *);
extern int varno_from_name(xgobidata *, char *);
extern Boolean selectVariable(xgobidata *, int, int, int);
extern int glyphNames(char **);

void initializeNewData(xgobidata *xg, char *name, int ncols_prev, int ncols_used_prev, int readFiles);
void reinit_1 (xgobidata *xg, int ncols_prev, int ncols_used_prev);
}


class XGobiServer : public XGobi_XGobi_skel {
 protected:
   xgobidata *xgobi;

 public:
   XGobiServer() {};  // basic constructor.
   XGobiServer(xgobidata *data) : xgobi(data) { };  

   XGobi_XGobi* replicate(CORBABoolean linked, const char *name) {
     extern void Clone_XGobi ();
     //       Clone_XGobi();
     return(this); // This is not correct. Wait till we change the cloning.
     // and get back a reference to it as a CORBA object.
  }

  char * fileName() {
    return(CORBA_string_dup(xgobi->datafilename));
  }

  void fileName(const char *fileName) {
    // load up a new file.
    // see the callback for 
    read_data_from_file((char *)fileName, xgobi);
  }


  // See new_data.c
  void setData(Omegahat_BaseMatrix *m, const Sequences_StringSequence &names) {

    long numVars = names.length();
    Sequences_DoubleSequence *mdata = m->allData(true);

    long c = m->ncol();

    if(numVars != c) {
      // Throw an error.
      return;
    }
    long r = m->nrow();

    int old_ncols = xgobi->ncols;
    int old_ncols_used = xgobi->ncols_used;
//  int old_nrows = xgobi->nrows;

    reinit_1(xgobi, old_ncols, old_ncols_used);
    
    xgobi->ncols = xgobi->ncols_used = c;
    xgobi->nrows = r;

    // realloc dataArea
    alloc_block(1, xgobi);
    float ** dataArea = xgobi->raw_data; 
    int i;
    for(i = 0; i <  r; i++) {
      for(int j = 0; j < c ; j++) {
        dataArea[i][j] = (float) (*mdata)[i*c + j];
      }
    } 

// See end of  reinit_1(xgobi, xgobi-);
    xgobi->collab = (char **) malloc(numVars*sizeof(char*));
    for(int k = 0; k < numVars; k++) {
      /*xgobi->collab[k] = strdup(names[k]);*/
      strncpy(xgobi->collab[k], names[k], COLLABLEN);
      xgobi->collab[k][COLLABLEN-1] = '\0';
    }
      
    /* Allocate and create default row labels */
    xgobi->rowlab = (char **)
      XtMalloc((Cardinal) xgobi->nrows * sizeof (char *));
    for (i=0; i<xgobi->nrows; i++) {
      xgobi->rowlab[i] = (char *) XtMalloc((Cardinal) ROWLABLEN * sizeof(char));
      (void) sprintf(xgobi->rowlab[i], "%d", i+1);
    }

    xgobi->vgroup_ids = (int *) XtMalloc((Cardinal) xgobi->ncols * sizeof(int));
    int j;
    for (j=0; j<xgobi->ncols; j++)
      xgobi->vgroup_ids[j] = 0;

    xgobi->collab = (char **)
      XtMalloc((Cardinal) xgobi->ncols * sizeof(char *));
    xgobi->collab_tform1 = (char **)
      XtMalloc((Cardinal) xgobi->ncols * sizeof(char *));
    xgobi->collab_tform2 = (char **)
      XtMalloc((Cardinal) xgobi->ncols * sizeof(char *));

    for (j=0; j<xgobi->ncols-1; j++) {
      strncpy(xgobi->collab[j], names[j], COLLABLEN);
      xgobi->collab[j][COLLABLEN-1] = '\0';

      strncpy(xgobi->collab_tform1[j], names[j], COLLABLEN+16);
      xgobi->collab[j][COLLABLEN+16-1] = '\0';

      strncpy(xgobi->collab_tform2[j], names[j], COLLABLEN+2*16);
      xgobi->collab[j][COLLABLEN+2*16-1] = '\0';

      /*xgobi->collab_tform2[j] = strdup( xgobi->collab[j]);*/
    }

    // don't read the files
    initializeNewData(xgobi, "CORBA Data", old_ncols, old_ncols_used, 0);
  }


  Omegahat_BaseMatrix *getData() {
    Omegahat_BaseMatrix *m;
    long nrow = xgobi->nrows;
    long ncol = xgobi->ncols_used;
    long n =  nrow * ncol;
    double *vals = (double*) malloc(sizeof(double)*n);

    for(int j = 0; j < ncol; j++) {
      for(int i = 0; i < nrow; i++) {
        vals[j*nrow + i] = xgobi->raw_data[i][j];
       }
     }

     m = new BaseMatrixImpl((CORBALong)nrow, (CORBALong)ncol, vals);
     return(m);
  }

  long addVariable(const Sequences_DoubleSequence &data, const char *name) {
    return(-1);
  }

  long
  addVariables(Omegahat_BaseMatrix *data,
    const Sequences_StringSequence &names)
  {
    return(-1);
  }

  void setVariableName(long int which, const char *name) {
    char *oname, strtmp1[COLLABLEN+16], strtmp2[COLLABLEN+2*16];
    char newname[COLLABLEN];
    int i, j;

    if(which < 0 || which >= xgobi->ncols_used) {
      // should throw an exception.
      return;
    }
    oname = XtMalloc(COLLABLEN * sizeof(char));
    oname = xgobi->collab[which];

    sprintf(newname, "%s", name);
    if (strlen(name) >= COLLABLEN) newname[COLLABLEN-1] = '\0';

    strcpy(xgobi->collab[which], newname);
    XtVaSetValues(xgobi->varlabw[which],
      XtNlabel, xgobi->collab_tform2[which],
      NULL);

    // And what about collab_tform1 and collab_tform2?
    // replace oname by xgobi->collab[which] in those strings.

    for (i=0, j=0; i<strlen(xgobi->collab_tform1[which]); i++, j++) {
      if (strncmp(xgobi->collab_tform1[which], oname, strlen(oname)) == 0) {
        strcat(strtmp1, oname);
        i += strlen(oname);
      } else {
        strtmp1[i] = oname[j];
      }
      if (i >= COLLABLEN+16) {
        i = COLLABLEN+16;
        break;
      }
    }
    strtmp1[i] = '\0';
    xgobi->collab_tform1[which] = strtmp1;

    for (i=0, j=0; i<strlen(xgobi->collab_tform2[which]); i++, j++) {
      if (strncmp(xgobi->collab_tform2[which], oname, strlen(oname)) == 0) {
        strcat(strtmp2, oname);
        i += strlen(oname);
      } else {
        strtmp2[i] = oname[j];
      }
      if (i >= COLLABLEN+2*16) {
        i = COLLABLEN+2*16;
        break;
      }
    }
    strtmp2[i] = '\0';
    xgobi->collab_tform2[which] = strtmp1;

    // Update the plot in case labels are displayed
    plot_once(xgobi);
  }

  void setRowNames(const Sequences_StringSequence &names) {
    int i, nnames = names.length();
    nnames = nnames < xgobi->nrows ? nnames : xgobi->nrows;
    for (i=0; i<nnames; i++) {
      strncpy(xgobi->rowlab[i], names[i], ROWLABLEN);
      xgobi->rowlab[i][ROWLABLEN-1] = '\0';
    }
  }

  void setRowName(long int which, const char *name) {

    if(which < 0 || which >= xgobi->nrows) {
      // should throw an exception.
      return;
    }

    strncpy(xgobi->rowlab[which], name, ROWLABLEN);
    xgobi->rowlab[which][ROWLABLEN-1] = '\0';
  }

  /* Get the name of the current view. */
  char * getView() {
    char *tmp = "<Unknown>";
    int num = -1;
    char **names =  GetPlotViewTypes(&num);

    for(int i = 0; i < num; i++) {
      /* Match by simple index. */
      if(xgobi->plot_mode == i) {
        tmp = names[i] + strlen("View: "); 
        break;
      }
    }
    return(CORBA_string_dup(tmp));
  }


  /* Set the view to the specified on. */
  CORBABoolean setView(const char *viewName) {
    CORBABoolean ans =  (SetPlotModeName(viewName, xgobi) > -1);
    return(ans);
  }

  /* return the list of potential views. */
  Sequences_StringSequence *getViewTypes() {
    int num = -1;
    char **names = GetPlotViewTypes(&num);

    char *tmp;
    Sequences_StringSequence *seq = new Sequences_StringSequence();
    seq->length(num);

    for(int i = 0; i < num ; i++) {
       // 
      tmp = names[i] + strlen("View: ");
      (*seq)[i] = CORBA_string_dup(tmp);
    }
    return(seq);
  }

  void disconnectObservations(Omegahat_Matrix_ptr pairs) {
  }

  void disconnectAllObservations() {
  }

  /* Add the specified pairs - rowwise - as connected by lines.
     I.e. this augments the existing connected pairs with the new ones.
  */
  void connectObservations(Omegahat_Matrix_ptr pairs) {
    if(pairs->ncol() != 2) {
      // throw an exception.
      return;
    }
   
    long nrow = pairs->nrow();
    Sequences_DoubleSequence *data = pairs->allData(true);

    int offset = xgobi->nlines;
    xgobi->nlines += nrow;
    realloc_lines(xgobi);

    for(int i = 0; i < 2*nrow; i+=2, offset++) {
      xgobi->connecting_lines[offset].a = (int) (*data)[i];
      xgobi->connecting_lines[offset].b = (int) (*data)[i+1];
    }
  }

  Omegahat_BaseMatrix_ptr getConnectedObservations() {
    BaseMatrixImpl *m;

    double *data = (double*)malloc(2*xgobi->nlines*sizeof(double));
    for(int k=0; k < xgobi->nlines; k++) {
      data[2*k] = xgobi->connecting_lines[k].a;
      data[2*k+1] = xgobi->connecting_lines[k].b;
    }

    m = new BaseMatrixImpl(xgobi->nlines, 2, data);

    return(m);
  }

/*
  CORBABoolean connectObservations(long int i, long int j) {
    int n = xgobi->nlines;
    xgobi->nlines += 1;
    realloc_lines(xgobi);

    xgobi->connecting_lines[n].a = i+1;
    xgobi->connecting_lines[n].b = j+1;

    return(true);
  }
*/

  CORBABoolean setObservationColor(const char *colName, long int which) {
    if(which < 0 || which > xgobi->nrows ||
      colName == (char*)NULL || colName[0] == (char)NULL)
    {
      return(false);
    }

    CORBABoolean ans = false;
    for(int i = 0; i < ncolors; i++) {
      if(strcmp(color_names[i], colName) == 0) {
        xgobi->color_ids[which] = xgobi->color_now[which] = color_nums[i];
        ans = true;
        break;
      }
    }

    if(ans)
     plot_once(xgobi);

    return(ans);
  }

  CORBABoolean setObservationColors(const char *colName,
    const Sequences_LongSequence &rows)
  {
    int i;
    CORBABoolean ans = false;
    long color = -1;
    for(int k = 0; k < ncolors; k++) {
      if(strcmp(color_names[k], colName) == 0) {
        color = color_nums[k];
        ans = true;
        break;
      }
    }

    if (ans) {
      for (int k=0; k<rows.length(); k++) {
        i = rows[k] - 1;
        if (i >= 0 && i < xgobi->nrows)
          xgobi->color_ids[i] = xgobi->color_now[i] = color;
      }

      plot_once(xgobi);
    }

    return(ans);
  }

  CORBABoolean setObservationSymbol(const char *glyphName, long int glyphSize,
    long int which)
  {
    if (glyphSize < 1 || glyphSize > 5 || which < 0 || which >= xgobi->nrows) {
      return(false);
    }

    CORBABoolean ans = true;
    int id = glyphIDfromName((char *) glyphName);
    ans = (id > 0);

    if(ans) {
      xgobi->glyph_ids[which].type = xgobi->glyph_now[which].type = id;
      xgobi->glyph_ids[which].size = xgobi->glyph_now[which].size = glyphSize;
      plot_once(xgobi);
    }

    return(ans);
  }

  CORBABoolean setObservationSymbols(const char *glyphName, long int glyphSize,
    const Sequences_LongSequence &rows)
  {
    if (glyphSize < 1 || glyphSize > 5) {
      return(false);
    }

    CORBABoolean ans = true;
    int i;
    int id = glyphIDfromName((char *) glyphName);
    ans = (id > 0);

    if(ans) {
      for (int k=0; k<rows.length(); k++) {
        i = rows[k] - 1;
        if (i >= 0 && i < xgobi->nrows) {
          xgobi->glyph_ids[i].type = xgobi->glyph_now[i].type = id;
          xgobi->glyph_ids[i].size = xgobi->glyph_now[i].size = glyphSize;
        }
      }
      plot_once(xgobi);
    }

    return(ans);
  }

/* I don't actually know what would be useful to return -- probably
 * maybe the strings that I test for -- filledcircle, opencircle, etc
*/
  Sequences_StringSequence *getSymbolNames() {
    char* names[32];
    int i, n = glyphNames(names);
    Sequences_StringSequence *seq = new Sequences_StringSequence();
    seq->length(n);

    for (i=0; i<n; i++)
      (*seq)[i] = CORBA_string_dup(names[i]);

    return(seq);
  }

  long int setX(const char *name, CORBABoolean append) {
    int varno = varno_from_name(xgobi, (char *) name);
    if (varno != -1) {
      selectVariable(xgobi, varno, 1, 0);  /* button 1 */
    }

    return(varno);
  }    

  long int setY(const char *name, CORBABoolean append) {
    int varno = varno_from_name(xgobi, (char *) name);
    if (varno != -1) {
      selectVariable(xgobi, varno, 2, 0);  /* button 2 */
    }

    return(varno);
  }    


  /*
   * Set the selected state for a variable -- used to manipulate
   * the parallel coordinates plot
  */
  CORBABoolean setSelected(const char *name, CORBABoolean select) {

    int varno = varno_from_name(xgobi, (char *) name);
    CORBABoolean ans = (varno != -1);

    if (ans && xgobi->is_cprof_plotting) {
      xgobi->selectedvars[varno] = (Boolean) select;
      if (xgobi->is_cprof_plotting) {
        update_cprof_selectedvars(xgobi);
        reset_nvars_cprof_plot(xgobi);
      }
    }

    return(ans);
  }

  /* Returns a list of the selected variables.     
     There are as many elements in the returned sequence
     as thre are selected variables. (i.e. not the total number
     of variables with an indicator of whether it is selected.)
   */
  Sequences_StringSequence *getSelectedVariables() {

    Sequences_StringSequence  *seq = new Sequences_StringSequence();

    int num = 0, i;
    for(i = 0; i < xgobi->ncols_used; i++) {
      if(xgobi->varchosen[i])
      num++;
    } 
    seq->length(num);

    num = 0;
    for(i = 0; i < xgobi->ncols_used; i++) {
      if (xgobi->varchosen[i]) {
        (*seq)[num++] = CORBA_string_dup(xgobi->collab[i]);
      }
    } 
    return(seq);
  }

  /* Returns whether the identified variable is selected in the
     variable panel or not.
   */
  CORBABoolean isSelected(const char *varName) {

    int varno = varno_from_name(xgobi, (char *) varName);
    CORBABoolean ans = (varno != -1);

    return(ans);
  }

  Sequences_StringSequence* getVariableNames() {
    Sequences_StringSequence *seq;
    seq = new Sequences_StringSequence();
    seq->length(xgobi->ncols);
    for(int i = 0; i < xgobi->ncols ; i++) {
      (*seq)[i] = CORBA_string_dup(xgobi->collab[i]);
    }
    return(seq);
  }

  Sequences_StringSequence* getRowNames() {

     Sequences_StringSequence *seq;
     seq = new Sequences_StringSequence();
     seq->length(xgobi->nrows);
     for(int i = 0; i < xgobi->nrows ; i++) {
       (*seq)[i] = CORBA_string_dup(xgobi->rowlab[i]);
     }
     return(seq);
  }

  Sequences_StringSequence* getRowColors() {
    Sequences_StringSequence *seq = new Sequences_StringSequence();
    seq->length(xgobi->nrows);
    for (int i=0; i<xgobi->nrows; i++) {
      for (int k=0; k<ncolors; k++) {
        if(color_nums[k] == xgobi->color_now[i]) {
          (*seq)[i] = CORBA_string_dup(color_names[k]);
          break;
        }
      }
    }

    return(seq);
  }

  Sequences_StringSequence* getColorNames() {
    Sequences_StringSequence *seq = new Sequences_StringSequence();
    seq->length(ncolors);
    for (int k=0; k<ncolors; k++) {
      (*seq)[k] = CORBA_string_dup(color_names[k]);

    }
    return(seq);
  }


  void setSelectedIndices(const Sequences_LongSequence &rows) {
  }

  Sequences_LongSequence* getSelectedIndices() {
    return(NULL);
  }

  void setRotation(Omegahat_BaseMatrix_ptr rotation) {
    for(int j =0; j < 3; j++) {
      for(int i =0; i < 3; i++) {
        xgobi->Rmat0[i][j] = rotation->getElement(i,j);
      }
    }
  }

  Omegahat_BaseMatrix_ptr getRotation() {
     BaseMatrixImpl *m;

     double *data = (double*)malloc(9*sizeof(double));
     for(int j =0; j < 3; j++) {
       for(int i =0; i < 3; i++) {
         data[j*3 + i] = xgobi->Rmat0[i][j];
       }
     }
     m = new BaseMatrixImpl(3,3, data);

     return(m);
  }

  Sequences_DoubleSequence* getCoefficients() {
    return(NULL);
  }

  Sequences_DoubleSequence* getVariable(const char* name)
  {
    Sequences_DoubleSequence *seq = new Sequences_DoubleSequence();
    seq->length(xgobi->nrows);

    int varno = varno_from_name(xgobi, (char *) name);

    if (varno != -1) {
      for (int i = 0; i < xgobi->nrows; i++) {
        (*seq)[i] = xgobi->raw_data[i][varno];
      }
    }

    return(seq);
  }

  CORBA_Double getCurrentVariableValue(const char* name) {
     return(-100.0);
  }

  Sequences_DoubleSequence* getCurrentValues() {
     return(NULL);
  }

/*
    Sequences_StringSequence* getColorNames();
    Sequences_StringSequence* getSelectedVariableNames();
    CORBABoolean loadDataFrame(Omegahat_DataFrame_ptr df);
    CORBABoolean loadMatrix(Omegahat_BaseMatrix_ptr df);
*/
};

#endif  // end of conditional inclusion of this file.
