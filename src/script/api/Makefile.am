SCRIPT_DIRS += src/script/api

SCRIPT_PUBLIC_INCLUDES += \
	QScriptable \
	QScriptClass \
	QScriptClassPropertyIterator \
	QScriptContext \
	QScriptContextInfo \
	QScriptEngine \
	QScriptEngineAgent \
	QScriptExtensionInterface \
	QScriptExtensionPlugin \
	QScriptProgram \
	QScriptString \
	QScriptValue \
	QScriptValueIterator \
	QScriptContextInfoList \
	QScriptSyntaxCheckResult \
	QScriptValueList \
	QtScript

SCRIPT_API_INCLUDES = \
	qscriptable.h \
	qscriptclass.h \
	qscriptclasspropertyiterator.h \
	qscriptcontext.h \
	qscriptcontextinfo.h \
	qscriptcontextinfolist.h \
	qscriptengine.h \
	qscriptengineagent.h \
	qscriptextensioninterface.h \
	qscriptextensionplugin.h \
	qscriptprogram.h \
	qscriptstring.h \
	qscriptsyntaxcheckresult.h \
	qscriptvaluelist.h \
	qscriptvalue.h \
	qscriptvalueiterator.h \
	qtscript.h

SCRIPT_API_PRIVATE_INCLUDES = \
	qscriptable_p.h \
	qscriptcontext_p.h \
	qscriptengine_p.h \
	qscriptengineagent_p.h \
	qscriptprogram_p.h \
	qscriptstring_p.h \
	qscriptvalue_p.h

SCRIPT_INCLUDES += $(SCRIPT_API_INCLUDES)
SCRIPT_PRIVATE_INCLUDES += $(SCRIPT_API_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/script/api/%,$(SCRIPT_API_INCLUDES)) \
		$(patsubst %,src/script/api/%,$(SCRIPT_API_PRIVATE_INCLUDES))

lib_libCsScript@BUILD_ABI@_la_SOURCES += \
	src/script/api/qscriptclass.cpp \
	src/script/api/qscriptclasspropertyiterator.cpp \
	src/script/api/qscriptcontext.cpp \
	src/script/api/qscriptcontextinfo.cpp \
	src/script/api/qscriptengine.cpp \
	src/script/api/qscriptengineagent.cpp \
	src/script/api/qscriptextensionplugin.cpp \
	src/script/api/qscriptprogram.cpp \
	src/script/api/qscriptstring.cpp \
	src/script/api/qscriptvalue.cpp \
	src/script/api/qscriptvalueiterator.cpp \
	src/script/api/qscriptable.cpp

