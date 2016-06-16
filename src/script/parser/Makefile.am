SCRIPT_DIRS += src/script/parser

#SCRIPT_PUBLIC_INCLUDES +=

#SCRIPT_PARSER_INCLUDES =

SCRIPT_PARSER_PRIVATE_INCLUDES = \
	qscriptastfwd_p.h \
	qscriptast_p.h \
	qscriptastvisitor_p.h \
	qscriptgrammar_p.h \
	qscriptsyntaxchecker_p.h \
	qscriptlexer_p.h \
	qscriptparser_p.h

SCRIPT_INCLUDES += $(SCRIPT_PARSER_INCLUDES)
SCRIPT_PRIVATE_INCLUDES += $(SCRIPT_PARSER_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/script/parser/%,$(SCRIPT_PARSER_INCLUDES)) \
		$(patsubst %,src/script/parser/%,$(SCRIPT_PARSER_PRIVATE_INCLUDES))

lib_libCsScript@BUILD_ABI@_la_SOURCES += \
	src/script/parser/qscriptast.cpp \
	src/script/parser/qscriptastvisitor.cpp \
	src/script/parser/qscriptgrammar.cpp \
	src/script/parser/qscriptsyntaxchecker.cpp \
	src/script/parser/qscriptlexer.cpp

#	src/script/parser/qscriptparser.cpp