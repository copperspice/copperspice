bin_PROGRAMS += \
	bin/lupdate

noinst_HEADERS += \
	src/tools/shared/translator.h \
	src/tools/shared/translatormessage.h \
	src/tools/shared/simtexth.h \
	src/tools/lupdate/lupdate.h

bin_lupdate_SOURCES = \
	src/tools/shared/numerus.cpp \
	src/tools/shared/translator.cpp \
	src/tools/shared/translatormessage.cpp \
	src/tools/shared/qm.cpp \
	src/tools/shared/qph.cpp \
	src/tools/shared/po.cpp \
	src/tools/shared/ts.cpp \
	src/tools/shared/xliff.cpp \
	src/tools/shared/ioutils.cpp \
   src/tools/shared/simtexth.cpp \
	src/tools/lupdate/main.cpp \
	src/tools/lupdate/merge.cpp \
	src/tools/lupdate/cpp.cpp \
	src/tools/lupdate/java.cpp \
	src/tools/lupdate/qscript.cpp \
	src/tools/lupdate/ui.cpp

#	src/tools/lupdate/qdeclarative.cpp

$(bin_lupdate_SOURCES) : $(CORE_HEADERS_INSTALLED) $(XML_HEADERS_INSTALLED) 

bin_lupdate_CXXFLAGS = $(lib_libCsCore@BUILD_ABI@_la_CXXFLAGS) \
   -Iinclude/QtCore  -Iinclude/QtXml   \
   -Iprivateinclude/QtCore/private -I$(top_srcdir)/src/tools/shared

bin_lupdate_LDADD = lib/libCsCore@BUILD_ABI@.la lib/libCsXml@BUILD_ABI@.la

EXTRA_DIST+=src/tools/lupdate/lupdate.exe.manifest

if OSTYPE_WIN
   bin_lupdate_LDADD += -luuid
   bin_SCRIPTS = src/tools/lupdate/lupdate.exe.manifest
endif