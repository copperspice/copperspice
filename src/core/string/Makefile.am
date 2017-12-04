
CORE_DIRS += src/core/string

CORE_PUBLIC_INCLUDES += \
	QChar32   \
	QString8  \
	QString16 \
	QStringParser

CORE_STRING_INCLUDES = \
	cs_string.h \
	cs_string_iterator.h \
	cs_encoding.h  \
	cs_char.h   \
	qchar32.h   \
	qstring8.h  \
	qstring16.h  \
	qstringparser.h

CORE_STRING_PRIVATE_INCLUDES =

CORE_INCLUDES += $(CORE_STRING_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_STRING_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/core/string/%,$(CORE_STRING_INCLUDES)) \
		$(patsubst %,src/core/string/%,$(CORE_STRING_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/string/qchar32.cpp  \
	src/core/string/qstring8.cpp \
	src/core/string/qstring16.cpp \
	src/core/string/qstringparser.cpp
