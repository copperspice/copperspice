GUI_DIRS += src/gui/statemachine

GUI_PUBLIC_INCLUDES += \
	QKeyEventTransition \
	QMouseEventTransition

GUI_STATEMACHINE_INCLUDES = \
	qkeyeventtransition.h \
	qmouseeventtransition.h

GUI_STATEMACHINE_PRIVATE_INCLUDES = \
	qbasickeyeventtransition_p.h \
	qbasicmouseeventtransition_p.h

GUI_INCLUDES += $(GUI_STATEMACHINE_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_STATEMACHINE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/statemachine/%,$(GUI_STATEMACHINE_INCLUDES)) \
		$(patsubst %,src/gui/statemachine/%,$(GUI_STATEMACHINE_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/statemachine/qkeyeventtransition.cpp \
	src/gui/statemachine/qmouseeventtransition.cpp \
	src/gui/statemachine/qbasickeyeventtransition.cpp \
	src/gui/statemachine/qbasicmouseeventtransition.cpp \
	src/gui/statemachine/qguistatemachine.cpp
