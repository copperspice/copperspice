bin_PROGRAMS += \
	bin/lrelease

noinst_HEADERS += \
	src/tools/shared/translator.h \
	src/tools/shared/translatormessage.h \
	src/tools/shared/ioutils.h

bin_lrelease_SOURCES = \
	src/tools/shared/numerus.cpp \
	src/tools/shared/translator.cpp \
	src/tools/shared/translatormessage.cpp \
	src/tools/shared/qm.cpp \
	src/tools/shared/qph.cpp \
	src/tools/shared/po.cpp \
	src/tools/shared/ts.cpp \
	src/tools/shared/xliff.cpp \
	src/tools/shared/ioutils.cpp \
	src/tools/lrelease/main.cpp

if OSTYPE_BSD
bin_lrelease_LDFLAGS='-Wl,-rpath,$$ORIGIN/../lib'
endif

if OSTYPE_DARWIN
bin_lrelease_LDFLAGS='-Wl,-rpath,$$ORIGIN/../lib'
endif

if OSTYPE_LINUX
bin_lrelease_LDFLAGS='-Wl,-rpath,$$ORIGIN/../lib'
endif

$(bin_lrelease_SOURCES) : $(CORE_HEADERS_INSTALLED) $(XML_HEADERS_INSTALLED)

bin_lrelease_CXXFLAGS = $(lib_libCsCore@BUILD_ABI@_la_CXXFLAGS) \
   -Iinclude/QtCore -Iinclude/QtXml \
   -Iprivateinclude/QtCore/private  -I$(top_srcdir)/src/tools/shared

bin_lrelease_LDADD = lib/libCsCore@BUILD_ABI@.la lib/libCsXml@BUILD_ABI@.la

if OSTYPE_WIN
   bin_lrelease_LDADD += -luuid
endif