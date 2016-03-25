CC=gcc -ansi
CC=cc

#DEBUG=-O2
DEBUG=-g

#OPT= -target sun4
#OPT= -Wall
#OPT=-cckr

CFLAGS=$(DEBUG) $(OPT)

#LD=-non_shared
LD=

#DNET_STUB = -ldnet_stub
DNET_STUB =

# Set IDIR and LDIR to the place where X lives on your machine.
IDIR=/usr/include/X11
LDIR=/usr/lib/X11

LIBS= -lXaw -lXt -lXmu -lXext -lX11 -lm

XGOBID= $(PWD)/..

# Here you specify which random number generator you
# want or need to use, random() or drand48().
R= USE_RANDOM
#R= USE_RAND48

# DEFINES= -D$(R) -DDEFAULTDIR=\"$(XGOBID)\" -DRPC_USED -DXPLORE
DEFINES= -D$(R) -DDEFAULTDIR=\"$(XGOBID)\"

JOINTOBJ = xgobi_init.o pipeline.o read_array.o \
	initialize.o widgets.o callbacks.o getfname.o \
	xyplot.o make_axes.o line_editor.o missing.o  \
	spin_cbacks.o spin.o scaling.o scale_cbacks.o \
	identify.o var_panel.o transform.o sphere.o stdize.o inference.o \
	brush_init.o brush_cbacks.o brush.o paint.o brush_send.o \
	help.o prt_plotwin.o subset.o vc_lists.o exclusion.o \
	plot_once.o plot1d.o texture.o ash1d.o move_points.o \
	show_message.o smooth.o smooth_fns.o smoothsk.o pspline.o \
	save_data.o new_data.o read_data.o jitter.o parcoords.o utils.o \
	tour_init.o tour_util.o tour.o tour_cbacks.o tour_send.o gt_ctls.o \
	tour_pp.o svd.o kernel.o \
	legendre.o hermite.o natrl_hermite.o de_indices.o \
	central_mass.o holes.o skewness.o \
	tour_section.o corr.o corr_index.o corr_pursuit.o corr_util.o \
	DrawingA.o mt19937-1.o

XGOBIOBJ = xgobi.o $(JOINTOBJ)

XGVISOBJ = xgvis.o xgv_dissim.o xgv_mds.o \
	xgv_molecule.o xgv_read_data.o xgv_help.o \
	xgv_stress.o xgv_cbacks.o xgv_widgets.o $(JOINTOBJ)

XGVISOBJ = xgvis.o xgv_histogram.o xgv_qsort.o xgv_mds.o \
	xgv_read_data.o xgv_help.o \
	xgv_stressplot.o xgv_cbacks.o xgv_widgets.o xgv_anchor.o $(JOINTOBJ)

all:
	make xgobi
	make xgvis

xgobi: $(XGOBIOBJ)
	$(CC) $(CFLAGS) $(LD) -o xgobi $(XGOBIOBJ) -L$(LDIR) $(LIBS)

xgvis: $(XGVISOBJ)
	$(CC) $(CFLAGS) $(LD) -o xgvis $(XGVISOBJ) -L$(LDIR) $(LIBS)

clean:
	rm -f core *.o xgobi xgvis

pure: $(XGOBIOBJ)
	purify $(CC) $(CFLAGS) $(LD) -o xgobi $(XGOBIOBJ) -L$(LDIR) $(LIBS)

install:
	mv bin/xgobi bin/oxgobi
	cp xgobi bin
	strip bin/xgobi

.c.o:
	$(CC) $(CFLAGS) -c $(DEFINES) $*.c -I$(IDIR)

lint:
	lint -DDEFAULTDIR=\"$(XGOBIDIR)\" -I$(IDIR) \
		-lm \
		/usr/local/pkg/X11/lib/lint/llib-lX11.ln \
		/usr/local/pkg/X11/lib/lint/llib-lXaw.ln \
		/usr/local/pkg/X11/lib/lint/llib-lXmu.ln \
		/usr/local/pkg/X11/lib/lint/llib-lXt.ln \
		*.c  >lint.out

# dependencies
xgobi.o: xgobitop.h xgobivars.h xgobitypes.h

