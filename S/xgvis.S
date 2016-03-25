
xgvis <-
  function(dmat       = NULL,
           edges      = NULL,
           pos        = NULL,
           rowlab     = NULL,
           colors     = NULL,
           glyphs     = NULL,
           erase      = NULL,
           lines      = NULL,
           linecolors = NULL,
           resources  = NULL,
           display    = NULL)
{
  if (missing(edges) && missing(pos) && missing(dmat)) {
    cat("One of dmat, edges, or pos must be present\n")
    return()
  }

  tpath <- getenv("S_TMP")
  if(nchar(tpath) == 0) tpath <- getenv("TEMP")
  basefile <- paste(tpath, tempfile("xgvis"), sep = "/")

  ### distance matrix ###
  if (!missing(dmat)) {
    dmat <- eval(dmat)
    if (sum(abs(dmat[!is.na(dmat)])) == Inf > 0) {
      cat("xgvis can't handle Infs in dmat, replaced with NA\n")
      dmat[dmat==Inf] <- NA
    }
    dfile <- paste(basefile, ".dist", sep="")
    write(t(dmat), file = dfile, ncolumns = ncol(dmat))
    on.exit(unlink(dfile), add=T)
  }

  ### Edges ###
  if (!missing(edges))
    {
      # check data type
      if (!is.matrix(edges) || !is.numeric(edges) || dim(edges)[2] != 2) {
        cat("The 'edges' argument must be a numeric 2-column matrix\n")
        return()
      }

      edgesfile <- paste(basefile, ".edges", sep="")
      if (nrow(edges) > 0) {
	write(t(edges), file = edgesfile, ncol=2)
      }
      on.exit(unlink(edgesfile), add=T)
    }

  ### position matrix ###
  if (!missing(pos)) {
    pos <- eval(pos)
    if (sum(abs(pos[!is.na(pos)])) == Inf > 0)
      {
        cat("xgvis can't handle Inf in pos; replaced with NA\n")
        pos[pos==Inf] <- NA
      }
    pfile <- paste(basefile, ".pos", sep="")
    write(t(pos), file = pfile, ncolumns = ncol(pos))
    on.exit(unlink(pfile), add = T)
  }

  ### Row labels ###
  if (!missing(rowlab))
    # check data type
    if (!is.vector(rowlab) || !is.character(rowlab)) {
      cat("The 'rowlab' argument needs to be a character vector\n")
      return()
    }
  if (length(rowlab) > 0) {
    rowfile <- paste(basefile, ".row", sep="")
    write(rowlab, file = rowfile, ncol=1)
    on.exit(unlink(rowfile), add = T)
  }

  ### Colors ###
  if (!missing(colors)) {
    # check data type
    if (!is.vector(colors) || !is.character(colors)) {
      cat("The 'colors' argument needs to be a character vector\n")
      return()
    }
    colorfile <- paste(basefile, ".colors", sep="")
    write(colors, file = colorfile, ncol=1)
    on.exit(unlink(colorfile), add = T)
  }
  
  ### Glyphs ###
  if (!missing(glyphs)) {
    # check data type
    if (!is.vector(glyphs) || !is.numeric(glyphs)) {
      cat("The 'glyphs' argument needs to be a numeric vector\n")
      return()
    }
    glyphfile <- paste(basefile, ".glyphs", sep="")
    write(glyphs, file = glyphfile, ncol=1)
    on.exit(unlink(glyphfile), add = T)
  }
  
  ### Erase ###
  if (!missing(erase)) {
    # check data type
    if (!is.vector(erase) || !is.numeric(erase)) {
      cat("The 'erase' argument needs to be a numeric vector\n")
      return()
    }
    erasefile <- paste(basefile, ".erase", sep="")
    write(erase, file = erasefile, ncol=1)
    on.exit(unlink(erasefile), add = T)
  }

  ### Connected lines ###
  if (!missing(lines)) {
    # check data type
    if (!is.matrix(lines) || !is.numeric(lines) || dim(lines)[2] != 2) {
      cat("The 'lines' argument must be a numeric 2-column matrix\n")
      return()
    }

    linesfile <- paste(basefile, ".lines", sep="")
    if (nrow(lines) > 0) {
      write(t(lines), file = linesfile, ncol=2)
      on.exit(unlink(linesfile), add = T)
    }
  }

  ### Line colors ###
  if ((!missing(lines) || !missing(edges)) && !missing(linecolors)) {
    # check data type
    if (!is.vector(linecolors) || !is.character(linecolors)) {
      cat("The 'linecolors' argument must be a character vector\n")
      return()
    }
    linecolorfile <- paste(basefile, ".linecolors", sep="")
    write(linecolors, file = linecolorfile, ncol=1)
    on.exit(unlink(linecolorfile), add = T)
  }

  ### Resources ###
  if (!missing(resources)) {
    # check data type
    if (!is.vector(resources) || !is.character(resources)) {
      cat("The 'resources' argument must be a character vector\n")
      return()
    }
    resourcefile <- paste(basefile, ".resources", sep="")
    write(resources, file = resourcefile, ncol=1)
    on.exit(unlink(resourcefile), add = T)
  }


  # Note to installer:
  # Here you need to specify the path to the xgvis batch file/ executable
  # on your system.
  #
  # dos example:
#  xgpath <- "c:/packages/xgobi/xgvis.bat"
  # unix example:
  xgpath <- "/usr/dfs/xgobi/joint/src/xgvis"
  command <- paste(xgpath, basefile)
  cat(command, "\n")
  # dos:
#  invisible(dos(command, multi = F, minimized = T, output.to.S = F, translate = T))
  # unix:
  invisible(unix(command))
}

