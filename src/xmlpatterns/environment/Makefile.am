XMLPATTERNS_DIRS += src/xmlpatterns/environment

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_ENVIRONMENT_INCLUDES =

XMLPATTERNS_ENVIRONMENT_PRIVATE_INCLUDES = \
	qcurrentitemcontext_p.h            \
	qdelegatingdynamiccontext_p.h      \
	qdelegatingstaticcontext_p.h       \
	qdynamiccontext_p.h                \
	qfocus_p.h                         \
	qgenericdynamiccontext_p.h         \
	qgenericstaticcontext_p.h          \
	qreceiverdynamiccontext_p.h        \
	qreportcontext_p.h                 \
	qstackcontextbase_p.h              \
	qstaticbaseuricontext_p.h          \
	qstaticcontext_p.h                 \
	qstaticcurrentcontext_p.h          \
	qstaticfocuscontext_p.h            \
	qstaticcompatibilitycontext_p.h    \
	qstaticnamespacecontext_p.h \
	qstackcontextbase.cpp

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_ENVIRONMENT_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_ENVIRONMENT_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/environment/%,$(XMLPATTERNS_ENVIRONMENT_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/environment/%,$(XMLPATTERNS_ENVIRONMENT_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/environment/qcurrentitemcontext.cpp            \
	src/xmlpatterns/environment/qdelegatingdynamiccontext.cpp      \
	src/xmlpatterns/environment/qdelegatingstaticcontext.cpp       \
	src/xmlpatterns/environment/qdynamiccontext.cpp                \
	src/xmlpatterns/environment/qfocus.cpp                         \
	src/xmlpatterns/environment/qgenericdynamiccontext.cpp         \
	src/xmlpatterns/environment/qgenericstaticcontext.cpp          \
	src/xmlpatterns/environment/qreceiverdynamiccontext.cpp        \
	src/xmlpatterns/environment/qreportcontext.cpp                 \
	src/xmlpatterns/environment/qstaticbaseuricontext.cpp          \
	src/xmlpatterns/environment/qstaticcontext.cpp                 \
	src/xmlpatterns/environment/qstaticcurrentcontext.cpp          \
	src/xmlpatterns/environment/qstaticfocuscontext.cpp            \
	src/xmlpatterns/environment/qstaticcompatibilitycontext.cpp    \
	src/xmlpatterns/environment/qstaticnamespacecontext.cpp