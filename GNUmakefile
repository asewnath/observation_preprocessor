#
# Makefile for ESMA components.
#
# REVISION HISTORY:
#
# 09Jun2003  da Silva  First crack. 
#

# Make sure ESMADIR is defined
# ----------------------------
ifndef ESMADIR
       ESMADIR := $(PWD)/../../..
endif

# Compilation rules, flags, etc
# -----------------------------
#  include $(ESMADIR)/Config/ESMA_base.mk  # Generic stuff
#  include $(ESMADIR)/Config/ESMA_arch.mk  # System dependencies
#  include $(ESMADIR)/Config/GMAO_base.mk  # Generic stuff

#                  ---------------------
#                  Standard ESMA Targets
#                  ---------------------

esma_help help:
	@echo "Standard ESMA targets:"
	@echo "% make esma_install    (builds and install under ESMADIR)"
	@echo "% make esma_clean      (removes deliverables: *.[aox], etc)"
	@echo "% make esma_distclean  (leaves in the same state as cvs co)"
	@echo "% make esma_doc        (generates PDF, installs under ESMADIR)"
	@echo "% make esma_help       (this message)"
	@echo "Environment:"
	@echo "      ESMADIR = $(ESMADIR)"
	@echo "      BASEDIR = $(BASEDIR)"
	@echo "         ARCH = $(ARCH)"
	@echo "         SITE = $(SITE)"

#CURR_DIR := /discover/nobackup/asewnath/GEOSadas/src/GMAO_Shared/GMAO_ncdiag/tmpdir/observation_preprocessor
CURR_DIR := ./

THIS := $(shell basename `pwd`)
LIB  = lib$(THIS).a

SUBDIRS = $(wildcard $(ALLDIRS) )

TARGETS = esma_install esma_clean esma_distclean esma_doc \
          install clean distclean doc

NETCDF_LINKS = -L/discover/swdev/mathomp4/Baselibs/ESMA-Baselibs-5.2.8/x86_64-unknown-linux-gnu/ifort_18.0.5.274-intelmpi_18.0.5.274/Linux/lib -lnetcdf_c++4 -L/discover/swdev/mathomp4/Baselibs/ESMA-Baselibs-5.2.8/x86_64-unknown-linux-gnu/ifort_18.0.5.274-intelmpi_18.0.5.274/Linux/lib -L/discover/swdev/mathomp4/Baselibs/ESMA-Baselibs-5.2.8/x86_64-unknown-linux-gnu/ifort_18.0.5.274-intelmpi_18.0.5.274/Linux/lib -lnetcdf -ljpeg -lmfhdf -ldf -ljpeg -lhdf5_hl -lhdf5 -lm -L/discover/swdev/mathomp4/Baselibs/ESMA-Baselibs-5.2.8/x86_64-unknown-linux-gnu/ifort_18.0.5.274-intelmpi_18.0.5.274/Linux/lib -lmfhdf -ldf -lsz -ljpeg -lgpfs -L/discover/swdev/mathomp4/Baselibs/ESMA-Baselibs-5.2.8/x86_64-unknown-linux-gnu/ifort_18.0.5.274-intelmpi_18.0.5.274/Linux/lib -lcurl -lssl -lcrypto -lssl -lcrypto -ldl -lz -lz -lrt -lm -ldl -lm -L/discover/swdev/mathomp4/Baselibs/ESMA-Baselibs-5.2.8/x86_64-unknown-linux-gnu/ifort_18.0.5.274-intelmpi_18.0.5.274/Linux/lib -lcurl -lssl -lcrypto -lssl -lcrypto -ldl -lz -lz -lrt -lm -lnetcdf


$(TARGETS): 
	@ t=$@; argv="$(SUBDIRS)" ;\
	  for d in $$argv; do			 \
	    ( cd $$d				;\
	      echo ""; echo Making $$t in `pwd`          ;\
	      $(MAKE) -e $$t ) \
	  done
	$(MAKE) local_$@

local_esma_install local_install:

	ifort -c $(CURR_DIR)/observation_implementation/fortran/c_pbutil.f -L $(ESMADIR)/install/lib/ -lNCEP_bufr_r8i4 -I $(ESMADIR)/install/include/GMAO_mpeu -extend-source 132
	ifort -c $(CURR_DIR)/observation_implementation/fortran/c_pbmin.f -I. -L $(ESMADIR)/install/lib/ -lNCEP_bufr_r8i4 -I $(ESMADIR)/install/include/GMAO_mpeu -extend-source 132
	ifort -c $(CURR_DIR)/observation_implementation/fortran/preprocess_bufr.f -lstdc++ -I/discover/nobackup/asewnath/GEOSgcm/install/include/NCEP_bufr_r8i4/
	mpiicpc -c $(SRCS) -I. -I/discover/nobackup/asewnath/boost_1_70_0 -I/discover/nobackup/asewnath/json/single_include -I$(BASEDIR)/Linux/include/netcdf -L$(BASEDIR)/Linux/lib -I/discover/nobackup/asewnath/github/ioda-engines/jedi/include/ -I/discover/nobackup/asewnath/github/ioda-engines/ioda/include -I/discover/nobackup/asewnath/github/ioda-engines/deps/gsl-single/include/ -I/discover/nobackup/asewnath/github/eigen/ -std=c++14
	mpiicpc -c $(IMPL_SRCS) -I. -I/discover/nobackup/asewnath/boost_1_70_0 -I/discover/nobackup/asewnath/json/single_include -I$(BASEDIR)/Linux/include/netcdf -L$(BASEDIR)/Linux/lib -I/discover/nobackup/asewnath/github/ioda-engines/jedi/include/ -I/discover/nobackup/asewnath/github/ioda-engines/ioda/include/ -I/discover/nobackup/asewnath/github/ioda-engines/deps/gsl-single/include/ -I/discover/nobackup/asewnath/github/eigen/ -std=c++14
	mpiicpc -o run_preprocess c_pbutil.o c_pbmin.o preprocess_bufr.o -L $(ESMADIR)/install/lib/ -lNCEP_bufr_r8i4 -lNCEP_w3_r8i4 -I. -I$(ESMADIR)/install/include/GMAO_mpeu -lGMAO_mpeu $(OBJS) $(IMPL_OBJS) -lifcore -I/discover/nobackup/asewnath/github/ioda-engines/jedi/include/  -I/discover/nobackup/asewnath/boost_1_70_0 -I/discover/nobackup/asewnath/github/ioda-engines/ioda/include/ -I/discover/nobackup/asewnath/github/ioda-engines/deps/gsl-single/include/ -I/discover/nobackup/asewnath/github/eigen/ -I/discover/nobackup/asewnath/json $(NETCDF_LINKS) $(LIB_MPI) $(LIB_SYS) -lstdc++fs -lpthread -L/discover/nobackup/asewnath/github/ioda-engines/build/lib -lioda-engines -std=c++14 


local_esma_clean local_clean:
	-$(RM) *~ *.[aox] *.mod *.x

local_esma_distclean local_distclean:
	-$(RM) *~ *.[aoxd] *.mod *.x

local_esma_doc local_doc:
	@echo "Target $@ not yet implemented in $(THIS)"


#                  --------------------
#                  User Defined Targets
#                  --------------------

SRCS := observation.cpp run_preprocess.cpp
         
IMPL_SRCS := observation_implementation/observation_implementation.cpp observation_implementation/nrl_observation.cpp \
             observation_implementation/uw_observation.cpp observation_implementation/gsi_observation.cpp \
             observation_implementation/observation_functions.cpp observation_implementation/metsat_observation.cpp \
	     observation_implementation/jma_observation.cpp
       
OBJS := $(addsuffix .o, $(basename $(SRCS)))
IMPL_OBJS := $(addsuffix .o, $(notdir $(basename $(IMPL_SRCS))))


# Hack to prevent remaking dep files during cleaning
# --------------------------------------------------
  ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
    -include $(DEPS)
  endif

#.

  -include $(ESMADIR)/Config/ESMA_post.mk  # ESMA additional targets, macros

