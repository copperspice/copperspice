
CORE_DIRS += src/corelib/codecs

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
	qtsciicodec_p.h \
	qutfcodec_p.h \
	qiconvcodec_p.h \
	qfontlaocodec_p.h

CORE_INCLUDES += $(CORE_CODECS_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_CODECS_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/corelib/codecs/%,$(CORE_CODECS_INCLUDES)) \
		$(patsubst %,src/corelib/codecs/%,$(CORE_CODECS_PRIVATE_INCLUDES))

lib_libCsCore1_la_SOURCES += \
	src/corelib/codecs/qisciicodec.cpp \
	src/corelib/codecs/qlatincodec.cpp \
	src/corelib/codecs/qsimplecodec.cpp \
	src/corelib/codecs/qtextcodec.cpp \
	src/corelib/codecs/qtextcodecplugin.cpp \
	src/corelib/codecs/qtsciicodec.cpp \
	src/corelib/codecs/qutfcodec.cpp \
	src/corelib/codecs/qfontlaocodec.cpp

if OSTYPE_LINUX
lib_libCsCore1_la_SOURCES += \
	src/corelib/codecs/qiconvcodec.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore1_la_SOURCES += \
	src/corelib/codecs/qiconvcodec.cpp
endif

