DECLARATIVE_DIRS += src/declarative/qml/rewriter

# DECLARATIVE_PUBLIC_INCLUDES +=

DECLARATIVE_QML_REWRITER_PRIVATE_INCLUDES = \
	textwriter_p.h	

# DECLARATIVE_INCLUDES += $(DECLARATIVE_QML_REWRITER_INCLUDES)
DECLARATIVE_PRIVATE_INCLUDES += $(DECLARATIVE_QML_REWRITER_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/declarative/qml/rewriter/%,$(DECLARATIVE_QML_REWRITER_INCLUDES)) \
		$(patsubst %,src/declarative/qml/rewriter/%,$(DECLARATIVE_QML_REWRITER_PRIVATE_INCLUDES))

lib_libCsDeclarative1_la_SOURCES += \
	src/declarative/qml/rewriter/textwriter.cpp

