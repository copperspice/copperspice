XMLPATTERNS_DIRS += src/xmlpatterns/functions

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_FUNCTIONS_INCLUDES =

XMLPATTERNS_FUNCTIONS_PRIVATE_INCLUDES = \
	qabstractfunctionfactory_p.h      \
	qaccessorfns_p.h                  \
	qaggregatefns_p.h                 \
	qaggregator_p.h                   \
	qassemblestringfns_p.h            \
	qbooleanfns_p.h                   \
	qcomparescaseaware_p.h            \
	qcomparestringfns_p.h             \
	qcomparingaggregator_p.h          \
	qconstructorfunctionsfactory_p.h  \
	qcontextfns_p.h                   \
	qcurrentfn_p.h                    \
	qdatetimefn_p.h                   \
	qdatetimefns_p.h                  \
	qdeepequalfn_p.h                  \
	qdocumentfn_p.h                   \
	qelementavailablefn_p.h           \
	qerrorfn_p.h                      \
	qfunctionargument_p.h             \
	qfunctionavailablefn_p.h          \
	qfunctioncall_p.h                 \
	qfunctionfactorycollection_p.h    \
	qfunctionfactory_p.h              \
	qfunctionsignature_p.h            \
	qgenerateidfn_p.h                 \
	qnodefns_p.h                      \
	qnumericfns_p.h                   \
	qpatternmatchingfns_p.h           \
	qpatternplatform_p.h              \
	qqnamefns_p.h                     \
	qresolveurifn_p.h                 \
	qsequencefns_p.h                  \
	qsequencegeneratingfns_p.h        \
	qstaticbaseuricontainer_p.h       \
	qstaticnamespacescontainer_p.h    \
	qstringvaluefns_p.h               \
	qsubstringfns_p.h                 \
	qsystempropertyfn_p.h             \
	qtimezonefns_p.h                  \
	qtracefn_p.h                      \
	qtypeavailablefn_p.h              \
	qunparsedentitypublicidfn_p.h     \
	qunparsedentityurifn_p.h          \
	qunparsedtextavailablefn_p.h      \
	qunparsedtextfn_p.h               \
	qcontextnodechecker_p.h           \
	qxpath10corefunctions_p.h         \
	qxpath20corefunctions_p.h         \
	qxslt20corefunctions_p.h \
	qcomparingaggregator.cpp \
	qdatetimefns.cpp

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_FUNCTIONS_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_FUNCTIONS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/functions/%,$(XMLPATTERNS_FUNCTIONS_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/functions/%,$(XMLPATTERNS_FUNCTIONS_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/functions/qabstractfunctionfactory.cpp        \
	src/xmlpatterns/functions/qaccessorfns.cpp                    \
	src/xmlpatterns/functions/qaggregatefns.cpp                   \
	src/xmlpatterns/functions/qaggregator.cpp                     \
	src/xmlpatterns/functions/qassemblestringfns.cpp              \
	src/xmlpatterns/functions/qbooleanfns.cpp                     \
	src/xmlpatterns/functions/qcomparescaseaware.cpp              \
	src/xmlpatterns/functions/qcomparestringfns.cpp               \
	src/xmlpatterns/functions/qconstructorfunctionsfactory.cpp    \
	src/xmlpatterns/functions/qcontextfns.cpp                     \
	src/xmlpatterns/functions/qcontextnodechecker.cpp             \
	src/xmlpatterns/functions/qcurrentfn.cpp                      \
	src/xmlpatterns/functions/qdatetimefn.cpp                     \
	src/xmlpatterns/functions/qdeepequalfn.cpp                    \
	src/xmlpatterns/functions/qdocumentfn.cpp                     \
	src/xmlpatterns/functions/qelementavailablefn.cpp             \
	src/xmlpatterns/functions/qerrorfn.cpp                        \
	src/xmlpatterns/functions/qfunctionargument.cpp               \
	src/xmlpatterns/functions/qfunctionavailablefn.cpp            \
	src/xmlpatterns/functions/qfunctioncall.cpp                   \
	src/xmlpatterns/functions/qfunctionfactorycollection.cpp      \
	src/xmlpatterns/functions/qfunctionfactory.cpp                \
	src/xmlpatterns/functions/qfunctionsignature.cpp              \
	src/xmlpatterns/functions/qgenerateidfn.cpp                   \
	src/xmlpatterns/functions/qnodefns.cpp                        \
	src/xmlpatterns/functions/qnumericfns.cpp                     \
	src/xmlpatterns/functions/qpatternmatchingfns.cpp             \
	src/xmlpatterns/functions/qpatternplatform.cpp                \
	src/xmlpatterns/functions/qqnamefns.cpp                       \
	src/xmlpatterns/functions/qresolveurifn.cpp                   \
	src/xmlpatterns/functions/qsequencefns.cpp                    \
	src/xmlpatterns/functions/qsequencegeneratingfns.cpp          \
	src/xmlpatterns/functions/qstaticnamespacescontainer.cpp      \
	src/xmlpatterns/functions/qstringvaluefns.cpp                 \
	src/xmlpatterns/functions/qsubstringfns.cpp                   \
	src/xmlpatterns/functions/qsystempropertyfn.cpp               \
	src/xmlpatterns/functions/qtimezonefns.cpp                    \
	src/xmlpatterns/functions/qtracefn.cpp                        \
	src/xmlpatterns/functions/qtypeavailablefn.cpp                \
	src/xmlpatterns/functions/qunparsedentitypublicidfn.cpp       \
	src/xmlpatterns/functions/qunparsedentityurifn.cpp            \
	src/xmlpatterns/functions/qunparsedtextavailablefn.cpp        \
	src/xmlpatterns/functions/qunparsedtextfn.cpp                 \
	src/xmlpatterns/functions/qxpath10corefunctions.cpp           \
	src/xmlpatterns/functions/qxpath20corefunctions.cpp           \
	src/xmlpatterns/functions/qxslt20corefunctions.cpp
