bin_PROGRAMS += \
	bin/uic

noinst_HEADERS += \
	src/tools/uic/cpp/cppextractimages.h \
	src/tools/uic/cpp/cppwritedeclaration.h \
	src/tools/uic/cpp/cppwriteicondata.h \
	src/tools/uic/cpp/cppwriteicondeclaration.h \
	src/tools/uic/cpp/cppwriteiconinitialization.h \
	src/tools/uic/cpp/cppwriteincludes.h \
	src/tools/uic/cpp/cppwriteinitialization.h \
	src/tools/uic/customwidgetsinfo.h \
	src/tools/uic/databaseinfo.h \
	src/tools/uic/driver.h \
	src/tools/uic/globaldefs.h \
	src/tools/uic/option.h \
	src/tools/uic/qclass_lib_map.h \
	src/tools/uic/treewalker.h \
	src/tools/uic/ui4.h \
	src/tools/uic/uic.h \
	src/tools/uic/utils.h \
	src/tools/uic/validator.h

bin_uic_SOURCES = \
	src/tools/uic/cpp/cppextractimages.cpp \
	src/tools/uic/cpp/cppwritedeclaration.cpp \
	src/tools/uic/cpp/cppwriteicondata.cpp \
	src/tools/uic/cpp/cppwriteicondeclaration.cpp \
	src/tools/uic/cpp/cppwriteiconinitialization.cpp \
	src/tools/uic/cpp/cppwriteincludes.cpp \
	src/tools/uic/cpp/cppwriteinitialization.cpp \
	src/tools/uic/customwidgetsinfo.cpp \
	src/tools/uic/databaseinfo.cpp \
	src/tools/uic/driver.cpp \
	src/tools/uic/main.cpp \
	src/tools/uic/treewalker.cpp \
	src/tools/uic/ui4.cpp \
	src/tools/uic/uic.cpp \
	src/tools/uic/validator.cpp

bin_uic_LDFLAGS= -lpthread

if OSTYPE_BSD
bin_uic_LDFLAGS+='-Wl,-rpath,$$ORIGIN/../lib'
endif

if OSTYPE_DARWIN
bin_uic_LDFLAGS+='-Wl,-rpath,$$ORIGIN/../lib'
endif

if OSTYPE_LINUX
bin_uic_LDFLAGS+='-Wl,-rpath,$$ORIGIN/../lib'
endif

$(bin_uic_SOURCES) : $(CORE_HEADERS_INSTALLED)

bin_uic_CXXFLAGS = $(lib_libCsCore@BUILD_ABI@_la_CXXFLAGS) -I$(top_srcdir)/src/tools/uic/cpp -I$(top_srcdir)/src/tools/uic \
            -I$(top_srcdir)/src/core/global \
            -DQT_UIC   \
            -DQT_UIC_CPP_GENERATOR \
            -UQT_BUILD_CORE_LIB

bin_uic_LDADD = lib/libCsCore@BUILD_ABI@.la

if OSTYPE_WIN
bin_uic_LDADD += -luuid
endif