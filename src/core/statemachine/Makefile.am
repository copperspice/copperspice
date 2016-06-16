
CORE_DIRS += src/core/statemachine

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


noinst_HEADERS+=$(patsubst %,src/core/statemachine/%,$(CORE_STATEMACHINE_INCLUDES)) \
		$(patsubst %,src/core/statemachine/%,$(CORE_STATEMACHINE_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/statemachine/qstatemachine.cpp \
	src/core/statemachine/qabstractstate.cpp \
	src/core/statemachine/qeventtransition.cpp \
	src/core/statemachine/qstate.cpp \
	src/core/statemachine/qfinalstate.cpp \
	src/core/statemachine/qhistorystate.cpp \
	src/core/statemachine/qabstracttransition.cpp \
	src/core/statemachine/qsignaltransition.cpp
