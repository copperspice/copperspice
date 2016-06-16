XMLPATTERNS_DIRS += src/xmlpatterns/parser

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_PARSER_INCLUDES =

XMLPATTERNS_PARSER_PRIVATE_INCLUDES = \
	qparsercontext_p.h             \
	qmaintainingreader_p.h         \
	qquerytransformparser_p.h      \
	qtokenizer_p.h                 \
	qtokenrevealer_p.h             \
	qtokensource_p.h               \
	qxquerytokenizer_p.h           \
	qxslttokenizer_p.h             \
	qxslttokenlookup_p.h \
	qmaintainingreader.cpp \
	qtokenlookup.cpp

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_PARSER_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_PARSER_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/parser/%,$(XMLPATTERNS_PARSER_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/parser/%,$(XMLPATTERNS_PARSER_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/parser/qquerytransformparser.cpp  \
	src/xmlpatterns/parser/qparsercontext.cpp         \
	src/xmlpatterns/parser/qtokenrevealer.cpp         \
	src/xmlpatterns/parser/qtokensource.cpp           \
	src/xmlpatterns/parser/qxquerytokenizer.cpp       \
	src/xmlpatterns/parser/qxslttokenizer.cpp         \
	src/xmlpatterns/parser/qxslttokenlookup.cpp