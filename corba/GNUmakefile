MAKE=gmake

all:
	gmake -f Makefile xgobi

###### dfs adding to test ############################
OMEGA_HOME=/n/project2/swtoolbin/duncan/org/omegahat
INSTALL_HOME=$(OMEGA_HOME)
IDLTYPES_DIR=$(OMEGA_HOME)/include/IDL
######################################################

   # Include the CORBA material.
CORBACONFIG=$(OMEGA_HOME)/Interfaces/CORBA/CORBAConfig
include $(CORBACONFIG)/Makefile.CORBA

include $(OMEGA_HOME)/Config/GNUmakefile.config

OMEGA_INCLUDES=$(INSTALL_HOME)/include

 # Assume all the omegahat header files have been installed as part
 # of the omegahat distribution.
INCLUDES=\
         -I$(OMEGA_INCLUDES) \
         -I$(OMEGA_INCLUDES)/Corba \
         -I/usr/local/include \
         -I/usr/local/include/OB \
         -I.

IDLCFLAGS=-I. -I$(IDLTYPES_DIR)

CXXLAGS+=-DCORBA_USED=1

CXXFLAGS+= $(INCLUDES)

CFLAGS+=$(CXXFLAGS)

include $(OMEGA_HOME)/Config/GNUmakefile.rules

Corba.o: Corba.h Corba.cc XGobiCorbaServer.h XGobiServer.h

XGobiCorbaServer.h: XGobiCorbaServer.idl


MAKE=gmake

CXX=g++
CXX=CC

OMEGA_IDL_INCLUDES=$(OMEGA_HOME)/Interfaces/CORBA/IDLTypes
IDLCFLAGS=-I$(OMEGA_IDL_INCLUDES)

%.c %.h: %.idl
	$(IDLCC) $(IDLCFLAGS) $<

%.h: %.nw
	$(WEB) -RHeader $< > $@

%.cc: %.nw
	$(WEB) $< > $@

%.o: %.cc
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(MAKE) -f Makefile $@

xgobi.o: xgobi.c
	cc -DCORBA_USED=1 $(CXXFLAGS) -c $< -o $@

#	$(CXX) -DCORBA_USED=1 $(CXXFLAGS) -c $< -o $@

Corba.tar.gz:
	tar zcvf $@ GNUmakefile Imakefile Corba.nw XGobiCorbaServer.idl XGobiServer.h xgobi.c xgobitop.h xgobitypes.h xincludes.h


TAGS: $(shell ls *.[ch])
	etags *.[ch]

irfeed:
	irfeed -I${OMEGA_HOME}/Interfaces/CORBA/IDLTypes XGobiCorbaServer.idl
