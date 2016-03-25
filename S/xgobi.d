.tr @@
.BG
.FN xgobi
.TL
XGobi: Dynamic Graphics for Data Analysis
.DN
Dynamic graphics, including  brushing, rotation, grand tour, projection
pursuit, slicing. Most effectively used when called more than once on same
data, which then allows linked plots.  Brushing with several glyphs and
colors is supported.  (On monochrome displays, only glyphs can be used.)
.CS
xgobi(matrx,
collab=NULL, rowlab=NULL,
colors=NULL, glyphs=NULL, erase=NULL,
lines=NULL, linecolors=NULL,
resources=NULL, title=NULL,
vgroups=NULL, std="mmx",
nlinkable=0, subset=NULL,
display=NULL)
.RA
.AG matrx
Any numeric matrix or data.frame.
.OA
.AG collab
Optional character vector of column labels; the default is
`dimnames(matrx)[[2]]'.  If no default exists, xgobi constructs its own
defaults.
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
pairs of points to be connected by line segments.  The default
connecting line matrix connects each point to the one that follows
it in the data; that is, (1 2), (2 3), (3 4), ..., (n-1, n).
.AG linecolors
Optional integer vector, of length n where n is the number of
lines specified by the 'lines' argument.  It is used to supply
line colors to be used on startup; the default is for all the
lines to be drawn in the standard foreground color.
.AG resources
Optional character vector created by clicking on the "Save
Resources" button in XGobi (if this XGobi was initiated during
an S session).
.AG title
Optional character string which defines the `-title' argument used by
X. The default is the name of the current matrix matrx.  See documentation
for xgobi, or for X.
.AG vgroups
Optional integer vector, used to assign columns to groups for 
transformation and axis scaling.  This vector must contain one integer
for each variable.  Columns to be grouped together should share the
same integer.  Default is the vector 1:(ncol(matrx)).
.AG std
Optional string; which standardization of view to use.  Default is
`"mmx"', minimum-maximum scaling, in which the view is centered at the
midpoint of the data, and all the data fits inside the plotting
window.  Alternatives are `"msd"', in which the plot is centered at the
mean of the data, or `"mmd"' in which the plot is centered at the median.
In those two cases, the view is standardized using the largest distance.
.AG dev
Optional numeric scalar; the number of standard deviations (if `"msd"'
is chosen) or median absolute deviations (if `"mmd"' is chosen) that will
be plotted inside the plotting window.  Default is 2.
.AG nlinkable
Optional integer scalar, the number of rows to be used in linking
of brushing and identification;
the default is for all rows to be used.  This feature can be used
to link ordinary scatterplots with plots that have some decorations
requiring additional points, such as clustering trees.
.AG subset
Optional integer scalar, the number of rows to be included in the
initial display.  That is, all data will be read in, but an
initial random sample will be drawn for display.  Use the Subset
panel on the Tools Menu to select a new subset during the session.
.AG display
Optional character string, identifying the monitor on which to display
the xgobi window.  The default is `"machine:0.0"' where `machine' is the
name of the user's workstation.  See documentation for xgobi or for X.
.RT
The UNIX `status' upon completion, i.e. `0' if ok.
.SE
The xgobi S function executes a call to the C program of the same name,
an interactive statistical graphics program which runs under the X
Window System, and returns control of the S shell to the user.
.PP
XGobi can be used to create vectors of brushing information and rotation
coefficients; see the documentation for XGobi for details.
.SH REFERENCE
http://www.research.att.com/areas/stat/xgobi/
http://www.public.iastate.edu/~dicook/
.SH CONTACT
D. F. Swayne dfs@research.att.com
.EX
xgobi(x) 
xgobi(cbind(x,y,z), name="laser")
xgobi(x, collabels, rowlabels, subset=5000)

.KW dynamic, interactive, graphics, plotting, rotation, grand tour, identify
.WR
