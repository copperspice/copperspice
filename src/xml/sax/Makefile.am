XML_DIRS += src/xml/sax

XML_PUBLIC_INCLUDES += \
	QXmlAttributes \
	QXmlContentHandler \
	QXmlDTDHandler \
	QXmlDeclHandler \
	QXmlDefaultHandler \
	QXmlEntityResolver \
	QXmlErrorHandler \
	QXmlInputSource \
	QXmlLexicalHandler \
	QXmlLocator \
	QXmlNamespaceSupport \
	QXmlParseException \
	QXmlReader \
	QXmlSimpleReader


XML_SAX_INCLUDES = \
	qxml.h \
	qxmlattributes.h \
	qxmlcontenthandler.h \
	qxmldtdhandler.h \
	qxmldeclhandler.h \
	qxmldefaulthandler.h \
	qxmlentityresolver.h \
	qxmlerrorhandler.h \
	qxmlinputsource.h \
	qxmllexicalhandler.h \
	qxmllocator.h \
	qxmlnamespacesupport.h \
	qxmlparseexception.h \
	qxmlreader.h \
	qxmlsimplereader.h

XML_INCLUDES += $(XML_SAX_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xml/sax/%,$(XML_SAX_INCLUDES)) \
		$(patsubst %,src/xml/sax/%,$(XML_SAX_PRIVATE_INCLUDES))

lib_libCsXml@BUILD_ABI@_la_SOURCES += \
	src/xml/sax/qxml.cpp

