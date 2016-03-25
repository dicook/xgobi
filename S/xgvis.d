.tr @@
.BG
.FN xgvis
.TL
xgvis: interactive multidimensional scaling using xgobi for display
.DN
an interactive multidimensional scaling (MDS) program that
consists of a control panel to manipulate the parameters of the
MDS stress function and an xgobi window for data display.  It
can be used either for visualization of dissimilarity data, for
dimension reduction, or for graph layout.  Graph layout is
usually done in 2D, but xgvis allows layouts in arbitrary
dimensions, 3D being the default.  It permits missing values,
which can be used to implement multidimensional unfolding.
.CS
xgvis(matrx,
edges=NULL, pos=NULL, rowlab=NULL, colors=NULL, glyphs=NULL,
erase=NULL, lines=NULL, linecolors=NULL, resources=NULL,
display=NULL)
.RA
.AG matrx
A n nxn distance matrix
.AG edges
Line segments: specifications for the pattern of line segments which
connect pairs of points.  The file must contain at least two numbers per
line.  The first two numbers represent the row numbers of the two
points that should be connected.  (This is exactly like the structure
of a fname.lines file in xgobi.)  In addition, if a third number is
present, it is taken to be an edge weight.
.B NOTE:
If this line is present and the distance matrix is absent, then
the distance matrix is computed from it, with each
edge representing a distance of one.
.AG pos
Starting positions:  an n x p matrix, with entries separated by
white space and one row per line. 
.B NOTE:
If this file is present and a
distance file is absent, the distance matrix is computed from it.
.AG rowlab
Optional character vector of row labels; the default is
`dimnames(matrx)[[1]]'.  If no default exists, xgobi constructs its own
defaults.
.AG colors
Optional character vector, used to supply initial point colors to be used;
the default is that all points are the same color.
.AG glyphs
Optional integer vector, used to supply glyphs to be used on
startup; the default is that all points are drawn with the same glyph.
.AG erase
Optional integer vector of length equal to the number of rows in the
data and composed of 1s and 0s.  A 1 in position i specifies that 
point i should be erased.  The default is a vector of 0s.
.AG lines
Optional integer matrix, n by 2, which specifies by row number
pairs of points to be connected by line segments.
.B NOTE:
If this file is present, then the edges
file is used to create the distance matrix but the lines file is
used to draw the edges.
.AG linecolors
Optional integer vector, of length n where n is the number of
lines specified by the 'lines' argument.  It is used to supply
line colors to be used on startup; the default is for all the
lines to be drawn in the standard foreground color.
.AG resources
Optional character vector created by clicking on the "Save
Resources" button in XGobi (if this XGvis was initiated during
an S session).
.AG display
Optional character string, identifying the monitor on which to display
the xgvis window.  The default is `"machine:0.0"' where `machine' is the
name of the user's workstation.  See documentation for X.
.RT
The UNIX `status' upon completion, i.e. `0' if ok.
.SE
The xgvis S function executes a call to the C program of the same name,
and returns control of the S shell to the user.
.SH REFERENCE
http://www.research.att.com/areas/stat/xgobi/
http://www.public.iastate.edu/~dicook/
.SH CONTACT
D. F. Swayne dfs@research.att.com
.EX
The following examples use sample data files in the data_xgvis
directory in the xgobi and xgvis distribution directory.

morsecodes.dist _ matrix(scan("data_xgvis/morsecodes.dist"),
  ncol=36, byrow=T)
morsecodes.pos _ matrix(scan("data_xgvis/morsecodes.pos"),
  ncol=12, byrow=T)
morsecodes.lines _ matrix(scan("data_xgvis/morsecodes.lines"),
  ncol=2, byrow=T)
morsecodes.glyphs _ scan("data_xgvis/morsecodes.glyphs")
morsecodes.colors _ scan("data_xgvis/morsecodes.colors", what="")
morsecodes.linecolors _ scan("data_xgvis/morsecodes.linecolors", what="")
morsecodes.row _ scan("data_xgvis/morsecodes.row", what="")
xgvis(dmat=morsecodes.dist, 
      pos=morsecodes.pos, 
      row=morsecodes.row,
      colors=morsecodes.colors,
      glyphs=morsecodes.glyphs,
      lines=morsecodes.lines,
      linecolors=morsecodes.linecolors)

perm4.edges _ matrix(scan("data_xgvis/perm4.edges"), ncol=2, byrow=T)
perm4.colors _ scan("data_xgvis/perm4.colors", what="")
perm4.linecolors _ scan("data_xgvis/perm4.linecolors", what="")
xgvis(edges=perm4.edges)
xgvis(edges=perm4.edges,
  colors=perm4.colors,
  linecolors=perm4.linecolors)

.KW dynamic, interactive, graphics, plotting, multidimensional scaling
.WR
