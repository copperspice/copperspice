bin_PROGRAMS += \
	bin/lconvert

noinst_HEADERS += \
	src/tools/shared/translator.h \
	src/tools/shared/translatormessage.h

bin_lconvert_SOURCES = \
	src/tools/shared/numerus.cpp \
	src/tools/shared/translator.cpp \
	src/tools/shared/translatormessage.cpp \
	src/tools/shared/qm.cpp \
	src/tools/shared/qph.cpp \
	src/tools/shared/po.cpp \
	src/tools/shared/ts.cpp \
	src/tools/shared/xliff.cpp \
	src/tools/lconvert/main.cpp

if OSTYPE_BSD
bin_lconvert_LDFLAGS='-Wl,-rpath,$$ORIGIN/../lib'
endif

if OSTYPE_DARWIN
bin_lconvert_LDFLAGS='-Wl,-rpath,$$ORIGIN/../lib'
endif

if OSTYPE_LINUX
bin_lconvert_LDFLAGS='-Wl,-rpath,$$ORIGIN/../lib'
endif

$(bin_lconvert_SOURCES) : $(CORE_HEADERS_INSTALLED) $(XML_HEADERS_INSTALLED)

bin_lconvert_CXXFLAGS = $(lib_libCsCore@BUILD_ABI@_la_CXXFLAGS)   \
   -DPROEVALUATOR_CUMULATIVE -DPROEVALUATOR_INIT_PROPS  \
   -Iinclude/QtCore  -Iinclude/QtXml -I$(top_srcdir)/src/tools/shared   \
   -Iprivateinclude/QtCore/private

bin_lconvert_LDADD = lib/libCsCore@BUILD_ABI@.la lib/libCsXml@BUILD_ABI@.la

if OSTYPE_WIN
   bin_lconvert_LDADD += -luuid
endif