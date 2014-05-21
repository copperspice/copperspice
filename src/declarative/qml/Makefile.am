DECLARATIVE_DIRS += src/declarative/qml

DECLARATIVE_PUBLIC_INCLUDES += \
	QDeclarativeError  \
	QDeclarativeEngine  \
	QDeclarativeContext  \
	QDeclarativeProperty  \
	QDeclarativeComponent  \
	QDeclarativeExpression  \
	QDeclarativeExtensionInterface  \
	QDeclarativeExtensionPlugin  \
	QDeclarativeImageProvider  \
	QDeclarativeInfo  \
	QDeclarativeNetworkAccessManagerFactory \
	QDeclarativeScriptString \
	QDeclarativePropertyValueInterceptor  \
	QDeclarativePropertyValueSource \
	QDeclarativeParserStatus \
	QDeclarativeAttachedPropertiesFunc  \
	QDeclarativeListProperty  \
	QDeclarativeListReference  \
	QDeclarativeProperties  \
	QDeclarativeTypeInfo   \
	QtDeclarative

DECLARATIVE_QML_INCLUDES = \
	qdeclarative.h \
	qdeclarativeproperty.h \
	qdeclarativecomponent.h \
    qdeclarativepropertyvaluesource.h \
    qdeclarativepropertyvalueinterceptor.h \
    qdeclarativeparserstatus.h \
	qdeclarativeprivate.h \
	qdeclarativeengine.h \
    qdeclarativecontext.h \
    qdeclarativeexpression.h \
	qdeclarativeinfo.h \
	qdeclarativelist.h \
	qdeclarativeerror.h \
    qdeclarativeimageprovider.h  \
	qdeclarativenetworkaccessmanagerfactory.h \
	qdeclarativeextensioninterface.h \
	qdeclarativeextensionplugin.h   \
	qdeclarativescriptstring.h \
	qdeclarativeattachedpropertiesfunc.h  \
	qdeclarativelistproperty.h \
	qdeclarativelistreference.h   \
	qdeclarativeproperties.h \
	qdeclarativetypeinfo.h \
	qtdeclarative.h

DECLARATIVE_QML_PRIVATE_INCLUDES = \
     qdeclarativeparser_p.h \
     qdeclarativeglobal_p.h \
     qdeclarativeinstruction_p.h \
     qdeclarativevmemetaobject_p.h \
     qdeclarativebinding_p.h \
     qdeclarativebinding_p_p.h \
     qdeclarativecomponent_p.h \
     qdeclarativecustomparser_p.h \
     qdeclarativecustomparser_p_p.h \
     qdeclarativeboundsignal_p.h \
     qdeclarativeproxymetaobject_p.h \
     qdeclarativevme_p.h \
     qdeclarativecompiler_p.h \
     qdeclarativeengine_p.h \
     qdeclarativeexpression_p.h \
     qdeclarativedom_p.h \
     qdeclarativedom_p_p.h \
     qdeclarativerefcount_p.h \
     qdeclarativemetatype_p.h \
     qdeclarativestringconverters_p.h \
     qdeclarativeproperty_p.h \
     qdeclarativecontext_p.h \
     qdeclarativeinclude_p.h \
     qdeclarativetypeloader_p.h \
     qdeclarativelist_p.h \
     qdeclarativedata_p.h \
     qdeclarativescriptparser_p.h \
     qdeclarativerewrite_p.h \
     qpodvector_p.h \
     qbitfield_p.h \
     qdeclarativevaluetype_p.h \
     qdeclarativecompiledbindings_p.h \
     qdeclarativefastproperties_p.h \
     qdeclarativexmlhttprequest_p.h \
     qdeclarativesqldatabase_p.h \
     qmetaobjectbuilder_p.h \
     qdeclarativewatcher_p.h \
     qdeclarativecleanup_p.h \
     qdeclarativepropertycache_p.h \
     qdeclarativenotifier_p.h \
     qdeclarativeintegercache_p.h \
     qdeclarativetypenotavailable_p.h \
     qdeclarativetypenamecache_p.h \
     qdeclarativeobjectscriptclass_p.h \
     qdeclarativecontextscriptclass_p.h \
     qdeclarativeglobalscriptclass_p.h \
     qdeclarativevaluetypescriptclass_p.h \
     qdeclarativetypenamescriptclass_p.h \
     qdeclarativelistscriptclass_p.h \
     qdeclarativeworkerscript_p.h \
     qdeclarativeguard_p.h \
     qdeclarativedirparser_p.h \
     qdeclarativeimport_p.h

DECLARATIVE_INCLUDES += $(DECLARATIVE_QML_INCLUDES)
DECLARATIVE_PRIVATE_INCLUDES += $(DECLARATIVE_QML_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/declarative/qml/%,$(DECLARATIVE_QML_INCLUDES)) \
		$(patsubst %,src/declarative/qml/%,$(DECLARATIVE_QML_PRIVATE_INCLUDES))

lib_libCsDeclarative1_la_SOURCES += \
     src/declarative/qml/qdeclarativeparser.cpp \
     src/declarative/qml/qdeclarativeinstruction.cpp \
     src/declarative/qml/qdeclarativevmemetaobject.cpp \
     src/declarative/qml/qdeclarativeengine.cpp \
     src/declarative/qml/qdeclarativeexpression.cpp \
     src/declarative/qml/qdeclarativebinding.cpp \
     src/declarative/qml/qdeclarativeproperty.cpp \
     src/declarative/qml/qdeclarativecomponent.cpp \
     src/declarative/qml/qdeclarativecontext.cpp \
     src/declarative/qml/qdeclarativeinclude.cpp \
     src/declarative/qml/qdeclarativecustomparser.cpp \
     src/declarative/qml/qdeclarativepropertyvaluesource.cpp \
     src/declarative/qml/qdeclarativepropertyvalueinterceptor.cpp \
     src/declarative/qml/qdeclarativeproxymetaobject.cpp \
     src/declarative/qml/qdeclarativevme.cpp \
     src/declarative/qml/qdeclarativecompiler.cpp \
     src/declarative/qml/qdeclarativecompileddata.cpp \
     src/declarative/qml/qdeclarativeboundsignal.cpp \
     src/declarative/qml/qdeclarativedom.cpp \
     src/declarative/qml/qdeclarativerefcount.cpp \
     src/declarative/qml/qdeclarativemetatype.cpp \
     src/declarative/qml/qdeclarativestringconverters.cpp \
     src/declarative/qml/qdeclarativeparserstatus.cpp \
     src/declarative/qml/qdeclarativetypeloader.cpp \
     src/declarative/qml/qdeclarativeinfo.cpp \
     src/declarative/qml/qdeclarativeerror.cpp \
     src/declarative/qml/qdeclarativescriptparser.cpp \
     src/declarative/qml/qdeclarativerewrite.cpp \
     src/declarative/qml/qdeclarativevaluetype.cpp \
     src/declarative/qml/qdeclarativecompiledbindings.cpp \
     src/declarative/qml/qdeclarativefastproperties.cpp \
     src/declarative/qml/qdeclarativexmlhttprequest.cpp \
     src/declarative/qml/qdeclarativesqldatabase.cpp \
     src/declarative/qml/qmetaobjectbuilder.cpp \
     src/declarative/qml/qdeclarativewatcher.cpp \
     src/declarative/qml/qdeclarativecleanup.cpp \
     src/declarative/qml/qdeclarativepropertycache.cpp \
     src/declarative/qml/qdeclarativenotifier.cpp \
     src/declarative/qml/qdeclarativeintegercache.cpp \
     src/declarative/qml/qdeclarativetypenotavailable.cpp \
     src/declarative/qml/qdeclarativetypenamecache.cpp \
     src/declarative/qml/qdeclarativescriptstring.cpp \
     src/declarative/qml/qdeclarativeobjectscriptclass.cpp \
     src/declarative/qml/qdeclarativecontextscriptclass.cpp \
     src/declarative/qml/qdeclarativeglobalscriptclass.cpp \
     src/declarative/qml/qdeclarativevaluetypescriptclass.cpp \
     src/declarative/qml/qdeclarativetypenamescriptclass.cpp \
     src/declarative/qml/qdeclarativelistscriptclass.cpp \
     src/declarative/qml/qdeclarativeworkerscript.cpp \
     src/declarative/qml/qdeclarativeimageprovider.cpp \
     src/declarative/qml/qdeclarativenetworkaccessmanagerfactory.cpp \
     src/declarative/qml/qdeclarativedirparser.cpp \
     src/declarative/qml/qdeclarativeextensionplugin.cpp \
     src/declarative/qml/qdeclarativeimport.cpp \
     src/declarative/qml/qdeclarativelist.cpp
