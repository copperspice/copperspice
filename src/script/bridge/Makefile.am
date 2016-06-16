SCRIPT_DIRS += src/script/bridge

#SCRIPT_PUBLIC_INCLUDES +=

#SCRIPT_BRIDGE_INCLUDES =

SCRIPT_BRIDGE_PRIVATE_INCLUDES = \
	qscriptfunction_p.h \
	qscriptobject_p.h \
	qscriptclassobject_p.h \
	qscriptvariant_p.h \
	qscriptqobject_p.h \
	qscriptglobalobject_p.h \
	qscriptactivationobject_p.h \
	qscriptstaticscopeobject_p.h \
	qscriptdeclarativeobject_p.h \
	qscriptdeclarativeclass_p.h

SCRIPT_INCLUDES += $(SCRIPT_BRIDGE_INCLUDES)
SCRIPT_PRIVATE_INCLUDES += $(SCRIPT_BRIDGE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/script/bridge/%,$(SCRIPT_BRIDGE_INCLUDES)) \
		$(patsubst %,src/script/bridge/%,$(SCRIPT_BRIDGE_PRIVATE_INCLUDES))

lib_libCsScript@BUILD_ABI@_la_SOURCES += \
	src/script/bridge/qscriptfunction.cpp \
	src/script/bridge/qscriptobject.cpp \
	src/script/bridge/qscriptclassobject.cpp \
	src/script/bridge/qscriptvariant.cpp \
	src/script/bridge/qscriptqobject.cpp \
	src/script/bridge/qscriptglobalobject.cpp \
	src/script/bridge/qscriptactivationobject.cpp \
	src/script/bridge/qscriptstaticscopeobject.cpp \
	src/script/bridge/qscriptdeclarativeobject.cpp \
	src/script/bridge/qscriptdeclarativeclass.cpp
