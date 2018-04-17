XML_DIRS += src/xml/stream

XML_PUBLIC_INCLUDES += \
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
	QXmlStreamWriter

XML_STREAM_INCLUDES = \
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
	qxmlstreamwriter.h

XML_INCLUDES += $(XML_STREAM_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xml/stream/%,$(XML_STREAM_INCLUDES)) \
		$(patsubst %,src/xml/stream/%,$(XML_STREAM_PRIVATE_INCLUDES))

