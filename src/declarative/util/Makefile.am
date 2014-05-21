DECLARATIVE_DIRS += src/declarative/util

DECLARATIVE_PUBLIC_INCLUDES += \
	QDeclarativeView \
	QDeclarativePropertyMap

DECLARATIVE_UTIL_INCLUDES = \
	qdeclarativeview.h  \
    qdeclarativepropertymap.h
  
DECLARATIVE_UTIL_PRIVATE_INCLUDES = \
     qdeclarativeapplication_p.h \
     qdeclarativeutilmodule_p.h\
     qdeclarativeconnections_p.h \
     qdeclarativepackage_p.h \
     qdeclarativeanimation_p.h \
     qdeclarativeanimation_p_p.h \
     qdeclarativesystempalette_p.h \
     qdeclarativespringanimation_p.h \
     qdeclarativesmoothedanimation_p.h \
     qdeclarativesmoothedanimation_p_p.h \
     qdeclarativestate_p.h\
     qdeclarativestateoperations_p.h \
     qdeclarativepropertychanges_p.h \
     qdeclarativestate_p_p.h\
     qdeclarativetransitionmanager_p_p.h \
     qdeclarativestategroup_p.h \
     qdeclarativetransition_p.h \
     qdeclarativelistmodel_p.h\
     qdeclarativelistmodel_p_p.h\
     qdeclarativelistaccessor_p.h \
     qdeclarativeopenmetaobject_p.h \
     qdeclarativenullablevalue_p_p.h \
     qdeclarativetimeline_p_p.h \
     qdeclarativetimer_p.h \
     qdeclarativebind_p.h \
     qdeclarativepixmapcache_p.h \
     qdeclarativebehavior_p.h \
     qdeclarativefontloader_p.h \
     qdeclarativestyledtext_p.h \
     qdeclarativelistmodelworkeragent_p.h \
	 qdeclarativexmllistmodel_p.h \
     qlistmodelinterface_p.h


DECLARATIVE_INCLUDES += $(DECLARATIVE_UTIL_INCLUDES)
DECLARATIVE_PRIVATE_INCLUDES += $(DECLARATIVE_UTIL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/declarative/util/%,$(DECLARATIVE_UTIL_INCLUDES)) \
		$(patsubst %,src/declarative/util/%,$(DECLARATIVE_UTIL_PRIVATE_INCLUDES))

lib_libCsDeclarative1_la_SOURCES += \
     src/declarative/util/qdeclarativeapplication.cpp \
     src/declarative/util/qdeclarativeutilmodule.cpp\
     src/declarative/util/qdeclarativeview.cpp \
     src/declarative/util/qdeclarativeconnections.cpp \
     src/declarative/util/qdeclarativepackage.cpp \
     src/declarative/util/qdeclarativeanimation.cpp \
     src/declarative/util/qdeclarativesystempalette.cpp \
     src/declarative/util/qdeclarativespringanimation.cpp \
     src/declarative/util/qdeclarativesmoothedanimation.cpp \
     src/declarative/util/qdeclarativestate.cpp\
     src/declarative/util/qdeclarativetransitionmanager.cpp \
     src/declarative/util/qdeclarativestateoperations.cpp \
     src/declarative/util/qdeclarativepropertychanges.cpp \
     src/declarative/util/qdeclarativestategroup.cpp \
     src/declarative/util/qdeclarativetransition.cpp \
     src/declarative/util/qdeclarativelistmodel.cpp\
     src/declarative/util/qdeclarativelistaccessor.cpp \
     src/declarative/util/qdeclarativeopenmetaobject.cpp \
     src/declarative/util/qdeclarativetimeline.cpp \
     src/declarative/util/qdeclarativetimer.cpp \
     src/declarative/util/qdeclarativebind.cpp \
     src/declarative/util/qdeclarativepropertymap.cpp \
     src/declarative/util/qdeclarativepixmapcache.cpp \
     src/declarative/util/qdeclarativebehavior.cpp \
     src/declarative/util/qdeclarativefontloader.cpp \
     src/declarative/util/qdeclarativestyledtext.cpp \
     src/declarative/util/qdeclarativelistmodelworkeragent.cpp \
     src/declarative/util/qlistmodelinterface.cpp \
     src/declarative/util/qdeclarativexmllistmodel.cpp



