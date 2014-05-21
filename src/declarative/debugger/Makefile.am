DECLARATIVE_DIRS += src/declarative/debugger

DECLARATIVE_PUBLIC_INCLUDES += \
	QDeclarativeDebuggingEnabler 

DECLARATIVE_DEBUGGER_INCLUDES = \
	qdeclarativedebug.h \
	qdeclarativedebuggingenabler.h 

DECLARATIVE_DEBUGGER_PRIVATE_INCLUDES = \
	qdeclarativedebuggerstatus_p.h \
    qpacketprotocol_p.h \
    qdeclarativedebugservice_p.h \
    qdeclarativedebugservice_p_p.h \
    qdeclarativedebugclient_p.h \
    qdeclarativeenginedebug_p.h \
    qdeclarativedebugtrace_p.h \
    qdeclarativedebughelper_p.h \
    qdeclarativedebugserver_p.h \
    qdeclarativedebugserverconnection_p.h \
    qdeclarativeinspectorservice_p.h \
    qdeclarativeinspectorinterface_p.h \
    qjsdebuggeragent_p.h \
    qjsdebugservice_p.h \
    qdeclarativeenginedebugservice_p.h

DECLARATIVE_INCLUDES += $(DECLARATIVE_DEBUGGER_INCLUDES)
DECLARATIVE_PRIVATE_INCLUDES += $(DECLARATIVE_DEBUGGER_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/declarative/debugger/%,$(DECLARATIVE_DEBUGGER_INCLUDES)) \
		$(patsubst %,src/declarative/debugger/%,$(DECLARATIVE_DEBUGGER_PRIVATE_INCLUDES))

lib_libCsDeclarative1_la_SOURCES += \
    src/declarative/debugger/qdeclarativedebuggerstatus.cpp \
    src/declarative/debugger/qpacketprotocol.cpp \
    src/declarative/debugger/qdeclarativedebugservice.cpp \
    src/declarative/debugger/qdeclarativedebugclient.cpp \
    src/declarative/debugger/qdeclarativeenginedebug.cpp \
    src/declarative/debugger/qdeclarativedebugtrace.cpp \
    src/declarative/debugger/qdeclarativedebughelper.cpp \
    src/declarative/debugger/qdeclarativedebugserver.cpp \
    src/declarative/debugger/qdeclarativeinspectorservice.cpp \
    src/declarative/debugger/qjsdebuggeragent.cpp \
    src/declarative/debugger/qjsdebugservice.cpp \
    src/declarative/debugger/qdeclarativeenginedebugservice.cpp
