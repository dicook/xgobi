xgobi.tree _ function( c.hclust, c.dat, glyph.types=c(16,31) ) {
  c.n  _ dim(c.dat)[1]
  c.xy _ cbind( order( c.hclust$order ), 0 )    # coordinates for points
  c.cl _ cbind( rep(0,c.n-1), 0 )               # coordinates for cluster nodes
  c.mu _ matrix(0,c.n-1,dim(c.dat)[2])          # multivariate coordinates for cluster nodes
  c.nn _ rep(0,c.n)
  c.lk _ matrix( 0, ncol=2, nrow=2*(c.n-1) )    # list of links
  for(i in 1:(c.n-1) ) {
    ii _ c.hclust$merge[i,1];  jj _ c.hclust$merge[i,2]
    if(ii<0 & jj<0) { c.xy[-ii,2] _ 0
                      c.xy[-jj,2] _ 0
                      c.cl[  i,1] _ ( c.xy[-ii,1] + c.xy[-jj,1] )/2
                      c.cl[  i,2] _ c.hclust$height[i]
                      c.nn[i]     _ 2
                      c.mu[i,]    _ ( c.dat[-ii,] + c.dat[-jj,] )/c.nn[i]
                    }
    if(ii<0 & jj>0) { c.xy[-ii,2] _ c.hclust$height[jj]
                      c.cl[  i,1] _ ( c.xy[-ii,1] + c.cl[jj,1] )/2
                      c.cl[  i,2] _ c.hclust$height[i]
                      c.nn[i]     _ c.nn[jj] + 1
                      c.mu[i,]    _ ( c.dat[-ii,] + c.nn[jj]*c.mu[jj,] )/c.nn[i]
                    }
    if(ii>0 & jj<0) { c.xy[-jj,2] _ c.hclust$height[ii]
                      c.cl[  i,1] _ ( c.xy[-jj,1] + c.cl[ii,1] )/2
                      c.cl[  i,2] _ c.hclust$height[i]
                      c.nn[i]     _ c.nn[ii] + 1
                      c.mu[i,]    _ ( c.nn[ii]*c.mu[ii,] + c.dat[-jj,] )/c.nn[i]
                    }
    if(ii>0 & jj>0) { c.cl[  i,1] _ ( c.cl[ii,1] + c.cl[jj,1] )/2
                      c.cl[  i,2] _ c.hclust$height[i]
                      c.nn[i]     _ c.nn[ii] + c.nn[jj]
                      c.mu[i,]    _ ( c.nn[ii]*c.mu[ii,] + c.nn[jj]*c.mu[jj,] )/c.nn[i]
                    }
    c.lk[2*i-1,1] _ ifelse(ii>0,c.n+ii,-ii)
    c.lk[2*i-1,2] _ c.n+i
    c.lk[2*i,  1] _ ifelse(jj>0,c.n+jj,-jj)
    c.lk[2*i,  2] _ c.n+i
    if( i%%10 == 0 ) print( c(i,ii,jj) )
    }
  list( data=cbind( rbind(c.xy,c.cl), rbind(c.dat,c.mu), c(rep(0,c.n),rep(1,c.n-1)) ),
        lines=c.lk,
        collab=c("Objects","Merge Level",dimnames(c.dat)[[2]], "Nodes"),
        glyphs=c(rep(glyph.types[1],c.n),rep(glyph.types[2],c.n-1))
      )
  }

# example:

# first line: the iris data come in a 3-way array; collapse into a matrix and append a
# grouping vector
c.dat _ cbind( rbind(iris[,,1],iris[,,2],iris[,,3]), ,grp=rep(1:3,rep(50,3)) )
# interpoint distances computed from all variables except the grouping variable:
c.dist _ dist( c.dat[,-5] )
# hclust:
c.hclust _ hclust( dist=c.dist, method="ave" )

# call the above function to compute cluster node points and lines:
c.tox _ xgobi.tree( c.hclust, c.dat )

# call xgobi twice for linking:
xgobi( c.tox$dat, lines=c.tox$lines, collab=c.tox$coll, 
       glyphs=c.tox$glyphs, colors=rep("Yellow",299), title="", 
       nlinkable=dim(c.dat)[1] )  # remove this argument if it bombs

# or write out to files and call xgobi from unix:
write( t(c.tox$dat),   file="iris.tree.dat",    ncol=dim(c.tox$dat)[2] )
write( t(c.tox$lines), file="iris.tree.lines",  ncol=2 )
write( c.tox$coll,     file="iris.tree.col",    ncol=1 )
write( c.tox$glyphs,   file="iris.tree.glyphs", ncol=1 )
write( dim(c.dat)[1],  file="iris.tree.nlinkable" ) # does your xgobi version have this?
write( rep("Yellow",dim(c.tox$dat)[1]), file="iris.tree.colors", ncol=1 )
!xgobi iris.tree &
!xgobi iris.tree &
# Although it looks like more work, I prefer the use of xgobi outside S: 
# sometimes a new feature has not made it into the xgobi S function, 
# but it may be present in the raw xgobi.  An example is the "nlinkable"
# feature which allows you to brush the terminal nodes only and not the
# branches.


# Now do the following: it's really fun!
# 1st xgobi: place at top of screen, make as wide as you can;
#            widen the plotting area by pulling the handles in the bottom corners;
#            select "LineEdit" or hit "l" to show lines (play toggle "Show Lines");
#            select "Scale" or hit "s" and scale plot to fit the wide plotting area;
# 2nd xgobi: place underneath, widen the plotting area to square shape;
#            select "LineEdit" or hit "l" if you want to see the tree
#            select "Tour" or hit "t" to enter the grand tour;
#            select the four variables ".. width" and ".. length" by clicking/toggling
#            the variable boxes; deselect the first two variables;
#            adjust speed (>0) by pulling the gauge above the "Pause" button;
#            the following works only if you have the most recent version of xgobi:
#              manual control of projections: depress "<Shift>" and click on the circle
#              of the variable "MergeLevel": it gets marked by an inside circle;
#              now depress left anytime on the plot and drag the mouse in any direction:
#              you will be pulling the "MergeLevel" into the projection; as you release,
#              "MergeLevel" will gradually fade out of the projection;
# 1s xgobi:  select "Brush" or hit "b" and brush parts of the tree with another 
#            color and glyph;


