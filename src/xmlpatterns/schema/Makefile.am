XMLPATTERNS_DIRS += src/xmlpatterns/schema

#XMLPATTERNS_PUBLIC_INCLUDES +=

#XMLPATTERNS_SCHEMA_INCLUDES =

XMLPATTERNS_SCHEMA_PRIVATE_INCLUDES = \
	qnamespacesupport_p.h              \
	qxsdalternative_p.h                \
	qxsdannotated_p.h                  \
	qxsdannotation_p.h                 \
	qxsdapplicationinformation_p.h     \
	qxsdassertion_p.h                  \
	qxsdattribute_p.h                  \
	qxsdattributereference_p.h         \
	qxsdattributeterm_p.h              \
	qxsdattributeuse_p.h               \
	qxsdattributegroup_p.h             \
	qxsdcomplextype_p.h                \
	qxsddocumentation_p.h              \
	qxsdelement_p.h                    \
	qxsdfacet_p.h                      \
	qxsdidcache_p.h                    \
	qxsdidchelper_p.h                  \
	qxsdidentityconstraint_p.h         \
	qxsdinstancereader_p.h             \
	qxsdmodelgroup_p.h                 \
	qxsdnotation_p.h                   \
	qxsdparticle_p.h                   \
	qxsdparticlechecker_p.h            \
	qxsdreference_p.h                  \
	qxsdsimpletype_p.h                 \
	qxsdschema_p.h                     \
	qxsdschemachecker_p.h              \
	qxsdschemacontext_p.h              \
	qxsdschemadebugger_p.h             \
	qxsdschemahelper_p.h               \
	qxsdschemamerger_p.h               \
	qxsdschemaparser_p.h               \
	qxsdschemaparsercontext_p.h        \
	qxsdschemaresolver_p.h             \
	qxsdschematoken_p.h                \
	qxsdschematypesfactory_p.h         \
	qxsdstatemachine_p.h               \
	qxsdstatemachinebuilder_p.h        \
	qxsdterm_p.h                       \
	qxsdtypechecker_p.h                \
	qxsduserschematype_p.h             \
	qxsdvalidatedxmlnodemodel_p.h      \
	qxsdvalidatinginstancereader_p.h   \
	qxsdwildcard_p.h                   \
	qxsdxpathexpression_p.h \
	qxsduserschematype.cpp \
	qxsdstatemachine.cpp \
	qxsdschemachecker_helper.cpp

XMLPATTERNS_INCLUDES += $(XMLPATTERNS_SCHEMA_INCLUDES)
XMLPATTERNS_PRIVATE_INCLUDES += $(XMLPATTERNS_SCHEMA_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/xmlpatterns/schema/%,$(XMLPATTERNS_SCHEMA_INCLUDES)) \
		$(patsubst %,src/xmlpatterns/schema/%,$(XMLPATTERNS_SCHEMA_PRIVATE_INCLUDES))

nodist_lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES = \
	src/xmlpatterns/schema/qrc_builtinschemas.cpp

EXTRA_DIST += \
	src/xmlpatterns/schema/schemas/xml.xsd

lib_libCsXmlPatterns@BUILD_ABI@_la_SOURCES += \
	src/xmlpatterns/schema/qnamespacesupport.cpp            \
	src/xmlpatterns/schema/qxsdalternative.cpp              \
	src/xmlpatterns/schema/qxsdannotated.cpp                \
	src/xmlpatterns/schema/qxsdannotation.cpp               \
	src/xmlpatterns/schema/qxsdapplicationinformation.cpp   \
	src/xmlpatterns/schema/qxsdassertion.cpp                \
	src/xmlpatterns/schema/qxsdattribute.cpp                \
	src/xmlpatterns/schema/qxsdattributereference.cpp       \
	src/xmlpatterns/schema/qxsdattributeterm.cpp            \
	src/xmlpatterns/schema/qxsdattributeuse.cpp             \
	src/xmlpatterns/schema/qxsdattributegroup.cpp           \
	src/xmlpatterns/schema/qxsdcomplextype.cpp              \
	src/xmlpatterns/schema/qxsddocumentation.cpp            \
	src/xmlpatterns/schema/qxsdelement.cpp                  \
	src/xmlpatterns/schema/qxsdfacet.cpp                    \
	src/xmlpatterns/schema/qxsdidcache.cpp                  \
	src/xmlpatterns/schema/qxsdidchelper.cpp                \
	src/xmlpatterns/schema/qxsdidentityconstraint.cpp       \
	src/xmlpatterns/schema/qxsdinstancereader.cpp           \
	src/xmlpatterns/schema/qxsdmodelgroup.cpp               \
	src/xmlpatterns/schema/qxsdnotation.cpp                 \
	src/xmlpatterns/schema/qxsdparticle.cpp                 \
	src/xmlpatterns/schema/qxsdparticlechecker.cpp          \
	src/xmlpatterns/schema/qxsdreference.cpp                \
	src/xmlpatterns/schema/qxsdsimpletype.cpp               \
	src/xmlpatterns/schema/qxsdschema.cpp                   \
	src/xmlpatterns/schema/qxsdschemachecker.cpp            \
	src/xmlpatterns/schema/qxsdschemachecker_setup.cpp      \
	src/xmlpatterns/schema/qxsdschemacontext.cpp            \
	src/xmlpatterns/schema/qxsdschemadebugger.cpp           \
	src/xmlpatterns/schema/qxsdschemahelper.cpp             \
	src/xmlpatterns/schema/qxsdschemamerger.cpp             \
	src/xmlpatterns/schema/qxsdschemaparser.cpp             \
	src/xmlpatterns/schema/qxsdschemaparser_setup.cpp       \
	src/xmlpatterns/schema/qxsdschemaparsercontext.cpp      \
	src/xmlpatterns/schema/qxsdschemaresolver.cpp           \
	src/xmlpatterns/schema/qxsdschematoken.cpp              \
	src/xmlpatterns/schema/qxsdschematypesfactory.cpp       \
	src/xmlpatterns/schema/qxsdstatemachinebuilder.cpp      \
	src/xmlpatterns/schema/qxsdterm.cpp                     \
	src/xmlpatterns/schema/qxsdtypechecker.cpp              \
	src/xmlpatterns/schema/qxsdwildcard.cpp                 \
	src/xmlpatterns/schema/qxsdvalidatedxmlnodemodel.cpp    \
	src/xmlpatterns/schema/qxsdvalidatinginstancereader.cpp \
	src/xmlpatterns/schema/qxsdxpathexpression.cpp \
	src/xmlpatterns/schema/builtinschemas.qrc