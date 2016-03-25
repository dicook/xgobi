xgobi <- function(matrx,
                  collab     = dimnames(matrx)[[2]],
                  rowlab     = dimnames(matrx)[[1]],
                  colors     = NULL,
                  glyphs     = NULL,
                  erase      = NULL,
                  lines      = NULL,
                  linecolors = NULL,
                  resources  = NULL,
                  title      = NULL,
                  vgroups    = NULL,
                  std        = "mmx",
                  nlinkable  = NULL,
                  subset     = NULL,
                  display    = NULL)
{
    if (!missing(matrx))
    {
        ### data matrix ###
        x <- eval(matrx)
        if (sum(abs(x[!is.na(x)])) == Inf > 0)
        {
          cat("Sorry, xgobi can't handle Infs\n")
          return()
        }
        dfile <- tempfile("unix")
#       cat(t(x), file = dfile, fill = 80)
        x <- as.matrix(x)
        cat(t(x), file=dfile, sep=c(rep(" ",ncol(x)-1),"\n"))

        args <- ""

        ### Column labels ###
        if (missing(collab))
          collab <- dimnames(x)[[2]]
        else {
          # check data type
          if (!is.vector(collab) || !is.character(collab)) {
            cat("The 'collab' argument needs to be a character vector\n")
            return()
          }
        }
        if (length(collab) > 0)
        {
          colfile <- paste(dfile, ".col", sep="")
          cat(collab, file = colfile, sep="\n")
        }

        ### Row labels ###
        if (missing(rowlab))
          rowlab <- dimnames(x)[[1]]
        else {
          # check data type
          if (!is.vector(rowlab) || !is.character(rowlab)) {
            cat("The 'rowlab' argument needs to be a character vector\n")
            return()
          }
        }
        if (length(rowlab) > 0) {
            rowfile <- paste(dfile, ".row", sep="")
            cat(rowlab, file = rowfile, sep="\n")
        }

        ### Variable groups ###
        if (!missing(vgroups)) {
            # check data type
            if (!is.vector(vgroups) || !is.numeric(vgroups)) {
              cat("The 'vgroups' argument needs to be a numeric vector\n")
              return()
            }

            vgroupfile <- paste(dfile, ".vgroups", sep="")
            cat(vgroups, file = vgroupfile, sep="\n")
        }

        ### Colors ###
        if (!missing(colors)) {
            # check data type
            if (!is.vector(colors) || !is.character(colors)) {
              cat("The 'colors' argument needs to be a character vector\n")
              return()
            }
            colorfile <- paste(dfile, ".colors", sep="")
            cat(colors, file = colorfile, sep="\n")
        }
        ### Glyphs ###
        if (!missing(glyphs))
        {
            # check data type
            if (!is.vector(glyphs) || !is.numeric(glyphs)) {
              cat("The 'glyphs' argument needs to be a numeric vector\n")
              return()
            }
            glyphfile <- paste(dfile, ".glyphs", sep="")
            cat(glyphs, file = glyphfile, sep="\n")
        }
        ### Erase ###
        if (!missing(erase))
        {
            # check data type
            if (!is.vector(erase) || !is.numeric(erase)) {
              cat("The 'erase' argument needs to be a numeric vector\n")
              return()
            }
            erasefile <- paste(dfile, ".erase", sep="")
            cat(erase, file = erasefile, sep="\n")
        }

        ### Connected lines ###
        if (!missing(lines))
        {
            # check data type
            if (!is.matrix(lines) || !is.numeric(lines) || dim(lines)[2] != 2) {                cat("The 'lines' argument must be a numeric 2-column matrix\n")
                return()
            }

            linesfile <- paste(dfile, ".lines", sep="")
            unix(paste("rm -f", linesfile), output=F)
            if (nrow(lines) > 0) {
                for (i in 1:nrow(lines))
                    cat(lines[i,], "\n", file = linesfile, append=T)
            }
            ### Line colors ###
            if (!missing(linecolors)) {
                # check data type
                if (!is.vector(linecolors) || !is.character(linecolors)) {
                   cat("The 'linecolors' argument must be a character vector\n")
                   return()
                }
                linecolorfile <- paste(dfile, ".linecolors", sep="")
                cat(linecolors, file = linecolorfile, sep="\n")
            }
        }
        ### Resources ###
        if (!missing(resources))
        {
            # check data type
            if (!is.vector(resources) || !is.character(resources)) {
                cat("The 'resources' argument must be a character vector\n")
                return()
            }
            resourcefile <- paste(dfile, ".resources", sep="")
            cat(resources, file = resourcefile, sep	="\n")
        }
        ### nlinkable ###
        if (!missing(nlinkable)) {
            # check data type
            nlinkable <- as.integer(nlinkable)
            if (length(nlinkable) > 1) {
                cat("The 'nlinkable' argument must be a scalar integer\n")
                return()
            }
            linkablefile <- paste(dfile, ".nlinkable", sep="")
            cat(nlinkable, "\n", file = linkablefile)
        }
        ### subset ###
        subsetarg <- ""
        if (!missing(subset)) {
            # check data type
            subset <- as.integer(subset)
            if (length(subset) > 1) {
                cat("The 'subset' argument must be a scalar integer\n")
                return()
            }
            else if (subset == 0 || subset > nrow(x)) {
                cat("The 'subset' argument must be >0 and <= nrows\n")
                return()
            }
            subsetarg <- paste(" -subset ", subset, sep="")
            args <- paste(args, subsetarg, sep=" ")
        }

        if (!missing(display)) {
            if (!is.character(display))
                warning("display must be a character string")
            else
                args <- paste("-display", display, args)
        }

        if (!missing(title)) {
            if (!is.character(title)) {
                warning("title must be a character string")
                title <- deparse(substitute(matrx))
            }
        }
        else
            title <- deparse(substitute(matrx))
        args <- paste("-title", paste("'", title, "'", sep=""), args)


# Note to installer:
# Here you will need to specify the path to the xgobi executable
# on your system.
        path <- paste("/usr/local/bin/")

        command <- paste(path, "xgobi", sep="")
        command <- paste(command, args)
        command <- paste(command, dfile, "&")
        cat(command, "\n")

        invisible(unix(command,output.to.S=F))
    }
    else
      cat("Matrix argument required\n")
}
