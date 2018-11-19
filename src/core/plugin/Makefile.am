
CORE_DIRS += src/core/plugin

CORE_PUBLIC_INCLUDES += \
	QFactoryInterface \
	QLibrary \
	QPluginLoader \
	QUuid

CORE_PLUGIN_INCLUDES = \
	qfactoryinterface.h \
	qlibrary.h \
	qplugin.h \
	qpluginloader.h \
	quuid.h

CORE_PLUGIN_PRIVATE_INCLUDES = \
	qelfparser_p.h \
	qfactoryloader_p.h \
	qlibrary_p.h \
	qmachparser_p.h \
	qsystemlibrary_p.h

CORE_INCLUDES += $(CORE_PLUGIN_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_PLUGIN_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/core/plugin/%,$(CORE_PLUGIN_INCLUDES)) \
		$(patsubst %,src/core/plugin/%,$(CORE_PLUGIN_PRIVATE_INCLUDES)) \
		src/plugins/codecs/jp/qfontjpcodec.h

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/plugin/qelfparser_p.cpp \
	src/core/plugin/qfactoryloader.cpp \
	src/core/plugin/qfactoryinterface.cpp \
	src/core/plugin/qlibrary.cpp \
	src/core/plugin/qmachparser.cpp \
	src/core/plugin/qpluginloader.cpp \
	src/core/plugin/quuid.cpp

if OSTYPE_WIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/plugin/qlibrary_win.cpp \
	src/core/plugin/qsystemlibrary.cpp
endif

if OSTYPE_LINUX
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/plugin/qlibrary_unix.cpp
endif

if OSTYPE_BSD
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
        src/core/plugin/qlibrary_unix.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/plugin/qlibrary_unix.cpp
endif
