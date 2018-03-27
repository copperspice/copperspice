
CORE_DIRS += src/core/codecs

CORE_PUBLIC_INCLUDES += \
	QTextCodec \
	QTextCodecPlugin \
	QTextDecoder \
	QTextEncoder \
	QTextCodecFactoryInterface

CORE_CODECS_INCLUDES = \
	qtextcodec.h \
	qtextdecoder.h \
	qtextencoder.h \
	qtextcodecfactoryinterface.h \
	qtextcodecplugin.h

CORE_CODECS_PRIVATE_INCLUDES = \
	qisciicodec_p.h \
	qlatincodec_p.h \
	qsimplecodec_p.h \
	qtextcodec_p.h \
	qutfcodec_p.h \
	qiconvcodec_p.h \
	qfontlaocodec_p.h

CORE_INCLUDES += $(CORE_CODECS_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_CODECS_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/core/codecs/%,$(CORE_CODECS_INCLUDES)) \
		$(patsubst %,src/core/codecs/%,$(CORE_CODECS_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/codecs/qisciicodec.cpp \
	src/core/codecs/qlatincodec.cpp \
	src/core/codecs/qsimplecodec.cpp \
	src/core/codecs/qtextcodec.cpp \
	src/core/codecs/qtextcodecplugin.cpp \
	src/core/codecs/qutfcodec.cpp \
	src/core/codecs/qfontlaocodec.cpp

if OSTYPE_LINUX
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/codecs/qiconvcodec.cpp
endif

if OSTYPE_BSD
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
        src/core/codecs/qiconvcodec.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/codecs/qiconvcodec.cpp
endif

