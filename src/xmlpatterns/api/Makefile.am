XMLPATTERNS_DIRS += src/xmlpatterns/api

XMLPATTERNS_PUBLIC_INCLUDES += \
	QAbstractMessageHandler \
	QAbstractUriResolver \
	QAbstractXmlNodeModel \
	QAbstractXmlReceiver \
	QSimpleXmlNodeModel \
	QSourceLocation \
	QXmlFormatter \
	QXmlName \
	QXmlNamePool \
	QXmlQuery \
	QXmlResultItems \
	QXmlSchema \
	QXmlSchemaValidator \
	QXmlSerializer \
	QXmlItem \
	QXmlNodeModelIndex \
	QtXmlPatterns

XMLPATTERNS_API_INCLUDES = \
	qabstractmessagehandler.h \
	qabstracturiresolver.h \
	qxmlitem.h \
	qxmlnodemodelindex.h \
	qabstractxmlnodemodel.h \
	qabstractxmlreceiver.h \
	qsimplexmlnodemodel.h \
	qsourcelocation.h \
	qxmlformatter.h \
	qxmlname.h \
	qxmlnamepool.h \
	qxmlquery.h \
	qxmlresultitems.h \
	qxmlschema.h \
	qxmlschemavalidator.h \
	qxmlserializer.h \
	qtxmlpatterns.h

XMLPATTERNS_API_PRIVATE_INCLUDES = \
	qabstractxmlforwarditerator_p.h \
	qabstractxmlnodemodel_p.h \
	qabstractxmlpullprovider_p.h \
	qabstractxmlreceiver_p.h \
	qcoloringmessagehandler_p.h \
	qcoloroutput_p.h \
	qdeviceresourceloader_p.h \
	qiodevicedelegate_p.h \
	qnetworkaccessdelegator_p.h \
	qpullbridge_p.h \
	qresourcedelegator_p.h \
	quriloader_p.h \
	qvariableloader_p.h \
	qxmlpatternistcli_p.h \
	qxmlquery_p.h \
	qxmlresultitems_p.h \
	qxmlschema_p.h \
	qxmlschemavalidator_p.h \
	qxmlserializer_p.h \
	qreferencecountedvalue_p.h

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_API_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_API_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/api/%,$(XMLPATTERNS_API_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/api/%,$(XMLPATTERNS_API_PRIVATE_INCLUDES))

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/api/qvariableloader.cpp \
	src/xmlpatterns/api/qabstractmessagehandler.cpp \
	src/xmlpatterns/api/qabstracturiresolver.cpp \
	src/xmlpatterns/api/qabstractxmlnodemodel.cpp \
	src/xmlpatterns/api/qabstractxmlpullprovider.cpp \
	src/xmlpatterns/api/qabstractxmlreceiver.cpp \
	src/xmlpatterns/api/qiodevicedelegate.cpp \
	src/xmlpatterns/api/qnetworkaccessdelegator.cpp \
	src/xmlpatterns/api/qpullbridge.cpp \
	src/xmlpatterns/api/qresourcedelegator.cpp \
	src/xmlpatterns/api/qsimplexmlnodemodel.cpp \
	src/xmlpatterns/api/qsourcelocation.cpp \
	src/xmlpatterns/api/quriloader.cpp \
	src/xmlpatterns/api/qxmlformatter.cpp \
	src/xmlpatterns/api/qxmlname.cpp \
	src/xmlpatterns/api/qxmlnamepool.cpp \
	src/xmlpatterns/api/qxmlquery.cpp \
	src/xmlpatterns/api/qxmlresultitems.cpp \
	src/xmlpatterns/api/qxmlschema.cpp \
	src/xmlpatterns/api/qxmlschema_p.cpp \
	src/xmlpatterns/api/qxmlschemavalidator.cpp \
	src/xmlpatterns/api/qxmlserializer.cpp \
	src/xmlpatterns/api/qcoloringmessagehandler.cpp \
	src/xmlpatterns/api/qcoloroutput.cpp