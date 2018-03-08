XML_DIRS += src/xml/dom

XML_PUBLIC_INCLUDES += \
	QDomAttr \
	QDomCDATASection \
	QDomCharacterData \
	QDomComment \
	QDomDocument \
	QDomDocumentFragment \
	QDomDocumentType \
	QDomElement \
	QDomEntity \
	QDomEntityReference \
	QDomImplementation \
	QDomNamedNodeMap \
	QDomNode \
	QDomNodeList \
	QDomNotation \
	QDomProcessingInstruction \
	QDomText

XML_DOM_INCLUDES = \
	qdom.h \
	qdomattr.h \
	qdomcdatasection.h \
	qdomcharacterdata.h \
	qdomcomment.h \
	qdomdocument.h \
	qdomdocumentfragment.h \
	qdomdocumenttype.h \
	qdomelement.h \
	qdomentity.h \
	qdomentityreference.h \
	qdomimplementation.h \
	qdomnamednodemap.h \
	qdomnode.h \
	qdomnodelist.h \
	qdomnotation.h \
	qdomprocessinginstruction.h \
	qdomtext.h

XML_INCLUDES += $(XML_DOM_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xml/dom/%,$(XML_DOM_INCLUDES)) \
		$(patsubst %,src/xml/dom/%,$(XML_DOM_PRIVATE_INCLUDES))

lib_libCsXml@BUILD_ABI@_la_SOURCES += \
	src/xml/dom/qdom.cpp

