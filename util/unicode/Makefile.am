AUTOMAKE_OPTIONS = subdir-objects nostdinc
ACLOCAL_AMFLAGS = -I autotools/m4 --install
AM_CFLAGS = -DQT_SHARED
AM_CXXFLAGS = $(AM_CFLAGS)

AM_LDFLAGS =
BUILT_SOURCES =
CLEANFILES =
noinst_HEADERS =
noinst_LTLIBRARIES =
lib_LTLIBRARIES =
EXTRA_LTLIBRARIES =
EXTRA_DIST=LICENSE

bin_PROGRAMS = bin/Generate_Tables
bin_Generate_Tables_SOURCES = main.cpp

# location where CopperSpice libraries are installed
CS_HOME:=$(shell cd $(CS_HOME); pwd)

# location of includes, libs, defines for compiling
AM_CXXFLAGS+=-I$(CS_HOME)/include $(patsubst %,-I$(CS_HOME)/include/Qt%,$(cslibs))

# location of CopperSpice libraries for linking
AM_LDFLAGS+=-L$(CS_HOME)/lib

# path to gcc retrieved from configure (windows)
gccpath=$(dir $(shell which $(word 1,$(CC))))

# list of gcc libraries (windows)  sjlj  or seh
gcclibs=libgcc_s_*-1  libstdc++-6  libwinpthread-1

# list of CopperSpice libraries
cslibs=Core
csver=1.5

# linking
bin_Generate_Tables_LDFLAGS=$(AM_LDFLAGS) $(patsubst %,-lCs%$(csver),$(cslibs))