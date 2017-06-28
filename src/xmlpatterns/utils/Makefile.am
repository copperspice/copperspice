XMLPATTERNS_DIRS += src/xmlpatterns/utils

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_UTILS_INCLUDES =

XMLPATTERNS_UTILS_PRIVATE_INCLUDES = \
	qcommonnamespaces_p.h              \
	qcppcastinghelper_p.h              \
	qdebug_p.h                         \
	qgenericnamespaceresolver_p.h      \
	qpatternistlocale_p.h              \
	qnamepool_p.h                      \
	qnamespaceresolver_p.h             \
	qoutputvalidator_p.h               \
	qxpathhelper_p.h                   \
	qdelegatingnamespaceresolver_p.h   \
	qnodenamespaceresolver_p.h

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_UTILS_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_UTILS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/utils/%,$(XMLPATTERNS_UTILS_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/utils/%,$(XMLPATTERNS_UTILS_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/utils/qgenericnamespaceresolver.cpp   \
	src/xmlpatterns/utils/qpatternistlocale.cpp                  \
	src/xmlpatterns/utils/qnamepool.cpp                          \
	src/xmlpatterns/utils/qnamespaceresolver.cpp                 \
	src/xmlpatterns/utils/qoutputvalidator.cpp                   \
	src/xmlpatterns/utils/qxpathhelper.cpp                       \
	src/xmlpatterns/utils/qdelegatingnamespaceresolver.cpp       \
	src/xmlpatterns/utils/qnodenamespaceresolver.cpp