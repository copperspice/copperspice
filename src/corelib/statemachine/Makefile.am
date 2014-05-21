
CORE_DIRS += src/corelib/statemachine

CORE_PUBLIC_INCLUDES += \
	QAbstractState \
	QAbstractTransition \
	QEventTransition \
	QFinalState \
	QHistoryState \
	QSignalTransition \
	QState \
	QStateMachine


CORE_STATEMACHINE_INCLUDES = \
	qabstractstate.h \
	qabstracttransition.h \
	qeventtransition.h \
	qfinalstate.h \
	qhistorystate.h \
	qsignaltransition.h \
	qstate.h \
	qstatemachine.h

CORE_STATEMACHINE_PRIVATE_INCLUDES = \
	qabstractstate_p.h \
	qabstracttransition_p.h \
	qeventtransition_p.h \
	qhistorystate_p.h \
	qsignaleventgenerator_p.h \
	qstate_p.h \
	qstatemachine_p.h

CORE_INCLUDES += $(CORE_STATEMACHINE_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_STATEMACHINE_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/corelib/statemachine/%,$(CORE_STATEMACHINE_INCLUDES)) \
		$(patsubst %,src/corelib/statemachine/%,$(CORE_STATEMACHINE_PRIVATE_INCLUDES))

lib_libCsCore1_la_SOURCES += \
	src/corelib/statemachine/qstatemachine.cpp \
	src/corelib/statemachine/qabstractstate.cpp \
	src/corelib/statemachine/qeventtransition.cpp \
	src/corelib/statemachine/qstate.cpp \
	src/corelib/statemachine/qfinalstate.cpp \
	src/corelib/statemachine/qhistorystate.cpp \
	src/corelib/statemachine/qabstracttransition.cpp \
	src/corelib/statemachine/qsignaltransition.cpp
