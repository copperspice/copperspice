CORE_DIRS += src/core/json

CORE_PUBLIC_INCLUDES += \
	QJsonArray \
	QJsonDocument \
	QJsonObject \
	QJsonParseError \
	QJsonValue

CORE_JSON_INCLUDES = \
	qjson.h \
	qjsonarray.h \
	qjsondocument.h \
	qjsonobject.h \
	qjsonparseerror.h \
	qjsonvalue.h

CORE_JSON_PRIVATE_INCLUDES = \
	qjsonparser_p.h \
	qjsonwriter_p.h

CORE_INCLUDES += $(CORE_JSON_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_JSON_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/core/json/%,$(CORE_JSON_INCLUDES)) \
		$(patsubst %,src/core/json/%,$(CORE_JSON_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/json/qjson.cpp \
	src/core/json/qjsonarray.cpp \
	src/core/json/qjsondocument.cpp \
	src/core/json/qjsonobject.cpp \
	src/core/json/qjsonparser.cpp \
	src/core/json/qjsonvalue.cpp \
	src/core/json/qjsonwriter.cpp

