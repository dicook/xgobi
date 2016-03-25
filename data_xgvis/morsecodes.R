################################################################
#
# DIMACS 2001: learnt from Willem Heiser about the conventional
# conversions of similarities to dissimilarities:
# 1) Gaussian:    sim = exp(-dissim^2/sigma^2)      dissim = sqrt(-log(sim)*sigma^2)
# 2) Laplacian:   sim = exp(-dissim/sigma)          dissim = -log(sim)*sigma
# 3) Reciprocal:  sim = 1/dissim                    dissim = 1/sim
# (remaining Q: how is sigma chosen?)
#
# my conversion was  dissim_ij^2 = sim_ii + sim_jj - 2*sim_ij

# here's a Gaussian conversion, but in some details it works less well than
# my conversion, e.g., in the circular structure of the digits

m.raw <- matrix(scan("morsecodes.raw"), ncol=36, byrow=T)
morse.dist  <- m.raw
x11()
hist(m.raw, breaks=20)

m.sim <- matrix(scan("morsecodes.sim"), ncol=36, byrow=T)
m.sim[1:10,1:10]
max(m.sim)

m.dist <- sqrt(-log(m.sim/100))
diag(m.dist) <- 0
write(t(round(m.dist,3)), "morse.dist", ncol=36)

################################################################
#
# dirty Procrustes: two dissim matrices for comparison,
# with off-diagonal to match them
#
dist <- function(x) {
  y <- x %*% t(x)
  dy <- diag(y)
  return(sqrt(outer(dy, dy, "+") - 2*y))
}

# example 1: created 2D metric and nonmetric configuration in xgvis,
# now compute their distance matrices and stash them into a 72x72 matrix
morse.metric    <- matrix(scan("morse_metric.dat"),    ncol=11,  byrow=T)
morse.nonmetric <- matrix(scan("morse_nonmetric.dat"), ncol=11,  byrow=T)
morse.metric.dist    <- dist(morse.metric[,1:2])
morse.nonmetric.dist <- dist(morse.nonmetric[,1:2])
morse.nonmetric.dist <- morse.nonmetric.dist / sum(morse.nonmetric.dist) * sum(morse.metric.dist)

morse.both <- matrix(NA, ncol=72, nrow=72)
morse.both[1:36,1:36]       <- morse.metric.dist
morse.both[1:36+36,1:36+36] <- morse.nonmetric.dist
diag(morse.both[1:36,1:36+36]) <- 0
diag(morse.both[1:36+36,1:36]) <- 0
morse.both

write(t(morse.both), "morse_procrustes.dist", ncol=72)
write(c(rep(27,36),rep(23,36)), "morse_procrustes.glyphs", ncol=1)
morse.lines <- matrix(scan("morsecodes.lines"), ncol=2, byrow=T)
write(t(rbind(morse.lines,morse.lines+36)), "morse_procrustes.lines", ncol=2)
write(c(scan("morsecodes.linecolors",w=""),scan("morsecodes.linecolors",w="")), "morse_procrustes.linecolors", ncol=1)
system("xgvis /home/andreas/XGOBI-H/data_xgvis/morse_procrustes")

# example2: compare two conversions of similarities to dissims
# namely my formula and the Gaussian conversion
morse.gauss.dist <- matrix(scan("morse.dist"), ncol=36)
morse.gauss.dist[1:10,1:10]
morse.inner.dist <- matrix(scan("morsecodes.dist"), ncol=36)
morse.inner.dist[1:10,1:10]
