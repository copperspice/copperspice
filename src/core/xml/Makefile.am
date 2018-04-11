
CORE_DIRS += src/core/xml


CORE_PUBLIC_INCLUDES += \
	QXmlStreamAttribute \
	QXmlStreamAttributes \
	QXmlStreamEntityDeclaration \
	QXmlStreamEntityDeclarations \
	QXmlStreamEntityResolver \
	QXmlStreamNamespaceDeclaration \
	QXmlStreamNamespaceDeclarations \
	QXmlStreamNotationDeclaration \
	QXmlStreamNotationDeclarations \
	QXmlStreamReader \
	QXmlStreamWriter \
	QXmlStream

CORE_XML_INCLUDES = \
	qxmlstreamattribute.h \
	qxmlstreamattributes.h \
	qxmlstreamentitydeclaration.h \
	qxmlstreamentitydeclarations.h \
	qxmlstreamentityresolver.h \
	qxmlstreamnamespacedeclaration.h \
	qxmlstreamnamespacedeclarations.h \
	qxmlstreamnotationdeclaration.h \
	qxmlstreamnotationdeclarations.h \
	qxmlstreamreader.h \
	qxmlstreamwriter.h \
	qxmlstream.h

CORE_XML_PRIVATE_INCLUDES = \
	qxmlstream_p.h \
	qxmlutils_p.h

CORE_INCLUDES += $(CORE_XML_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_XML_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/core/xml/%,$(CORE_XML_INCLUDES)) \
		$(patsubst %,src/core/xml/%,$(CORE_XML_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/xml/qxmlstream.cpp \
	src/core/xml/qxmlutils.cpp
