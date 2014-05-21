DECLARATIVE_DIRS += src/declarative/qml/parser

# DECLARATIVE_PUBLIC_INCLUDES +=

# DECLARATIVE_QML_PARSER_INCLUDES =

DECLARATIVE_QML_PARSER_PRIVATE_INCLUDES = \
    qdeclarativejsast_p.h \
    qdeclarativejsastfwd_p.h \
    qdeclarativejsastvisitor_p.h \
    qdeclarativejsengine_p.h \
    qdeclarativejsgrammar_p.h \
    qdeclarativejslexer_p.h \
    qdeclarativejsmemorypool_p.h \
    qdeclarativejsnodepool_p.h \
    qdeclarativejsparser_p.h \
    qdeclarativejsglobal_p.h
   
DECLARATIVE_INCLUDES += $(DECLARATIVE_QML_PARSER_INCLUDES)
DECLARATIVE_PRIVATE_INCLUDES += $(DECLARATIVE_QML_PARSER_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/declarative/qml/parser/%,$(DECLARATIVE_QML_PARSER_INCLUDES)) \
		$(patsubst %,src/declarative/qml/parser/%,$(DECLARATIVE_QML_PARSER_PRIVATE_INCLUDES))

lib_libCsDeclarative1_la_SOURCES += \
	src/declarative/qml/parser/qdeclarativejsast.cpp \
    src/declarative/qml/parser/qdeclarativejsastvisitor.cpp \
    src/declarative/qml/parser/qdeclarativejsengine_p.cpp \
    src/declarative/qml/parser/qdeclarativejsgrammar.cpp \
    src/declarative/qml/parser/qdeclarativejslexer.cpp \
    src/declarative/qml/parser/qdeclarativejsparser.cpp

