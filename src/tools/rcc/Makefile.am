bin_PROGRAMS += \
	bin/rcc

noinst_HEADERS += \
	src/tools/rcc/rcc.h

bin_rcc_SOURCES = \
	src/tools/rcc/main.cpp \
	src/tools/rcc/rcc.cpp

bin_rcc_LDFLAGS= -lpthread

if OSTYPE_BSD
bin_rcc_LDFLAGS+='-Wl,-rpath,$$ORIGIN/../lib'
endif

if OSTYPE_DARWIN
bin_rcc_LDFLAGS+='-Wl,-rpath,$$ORIGIN/../lib'
endif

if OSTYPE_LINUX
bin_rcc_LDFLAGS+='-Wl,-rpath,$$ORIGIN/../lib'
endif

$(bin_rcc_SOURCES) : $(CORE_HEADERS_INSTALLED) $(XML_HEADERS_INSTALLED)

bin_rcc_CXXFLAGS = $(lib_libCsCore@BUILD_ABI@_la_CXXFLAGS)  -I$(top_srcdir)/src/tools/rcc  \
    -Iinclude/QtCore -Iinclude/QtXml \
    -UQT_BUILD_CORE_LIB

bin_rcc_LDADD = lib/libCsCore@BUILD_ABI@.la lib/libCsXml@BUILD_ABI@.la

if OSTYPE_WIN
   bin_rcc_LDADD += -luuid
endif