
CORE_DIRS += src/core/string

CORE_PUBLIC_INCLUDES += \
	QString8

CORE_STRING_INCLUDES = \
	cs_string.h \
	cs_string_iterator.h \
	cs_encoding.h  \
	cs_char.h  \
	qstring8.h

CORE_STRING_PRIVATE_INCLUDES =

CORE_INCLUDES += $(CORE_STRING_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_STRING_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/core/string/%,$(CORE_STRING_INCLUDES)) \
		$(patsubst %,src/core/string/%,$(CORE_STRING_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/string/qstring8.cpp
