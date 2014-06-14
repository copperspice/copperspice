bin_PROGRAMS += \
	bin/rcc

noinst_HEADERS += \
	src/tools/rcc/rcc.h

bin_rcc_SOURCES = \
	src/tools/rcc/main.cpp \
	src/tools/rcc/rcc.cpp

bin_rcc_LDFLAGS= -lpthread

$(bin_rcc_SOURCES) : $(CORE_HEADERS_INSTALLED) $(XML_HEADERS_INSTALLED) 

bin_rcc_CXXFLAGS = $(lib_libCsCore1_la_CXXFLAGS)  -I$(top_srcdir)/src/tools/rcc  \
    -Iincludes/QtCore -Iincludes/QtXml \
    -UQT_BUILD_CORE_LIB

bin_rcc_LDADD = lib/libCsCore1.la lib/libCsXml1.la

if OSTYPE_WIN
   bin_rcc_LDADD += -luuid
endif