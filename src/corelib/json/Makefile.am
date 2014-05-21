CORE_DIRS += src/corelib/json

CORE_PUBLIC_INCLUDES += \
	QJsonArray \
	QJsonDocument \
	QJsonObject \
	QJsonParseError \
	QJsonValue 

CORE_JSON_INCLUDES = \
	qjsonarray.h \
	qjsondocument.h \
	qjsonobject.h \
	qjsonparseerror.h \
	qjsonvalue.h 

CORE_JSON_PRIVATE_INCLUDES = \
	qjson_p.h \
	qjsonparser_p.h \
	qjsonwriter_p.h

CORE_INCLUDES += $(CORE_JSON_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_JSON_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/corelib/json/%,$(CORE_JSON_INCLUDES)) \
		$(patsubst %,src/corelib/json/%,$(CORE_JSON_PRIVATE_INCLUDES))

lib_libCsCore1_la_SOURCES += \
	src/corelib/json/qjson.cpp \
	src/corelib/json/qjsonarray.cpp \
	src/corelib/json/qjsondocument.cpp \
	src/corelib/json/qjsonobject.cpp \
	src/corelib/json/qjsonparser.cpp \
	src/corelib/json/qjsonvalue.cpp \
	src/corelib/json/qjsonwriter.cpp

