XMLPATTERNS_DIRS += src/xmlpatterns/projection

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_PROJECTION_INCLUDES =

XMLPATTERNS_PROJECTION_PRIVATE_INCLUDES = \
	qdocumentprojector_p.h \
	qprojectedexpression_p.h

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_PROJECTION_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_PROJECTION_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/projection/%,$(XMLPATTERNS_PROJECTION_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/projection/%,$(XMLPATTERNS_PROJECTION_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/projection/qdocumentprojector.cpp