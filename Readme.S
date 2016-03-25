Installing the xgobi S function

a)  Edit the file xgobi.S

  The file xgobi.S contains the definition of the XGobi S
  function, named xgobi().  Find the line in which the pathname
  of the XGobi executable is specified, and edit it to point to
  your copy.

b)  Source xgobi.S

  Initiate a New S session, and then type source("xgobi.S") to
  define the XGobi S function.

c)  Install the help page

  The file doc/xgobi.d contains the S help page.  Move it to
  .Data/.Help/xgobi.


How it works

  When XGobi is initiated from within S, the S data of interest is
  written out to an ascii file, which XGobi then reads in.  When XGobi
  wants to write out coefficients or brushing ids to S, it directly
  writes .Data/filename in the S data format.

