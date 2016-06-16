GUI_DIRS += src/gui/accessible

GUI_PUBLIC_INCLUDES += \
	QAccessible \
	QAccessible2Interface \
	QAccessibleActionInterface \
	QAccessibleApplication \
	QAccessibleBridge \
	QAccessibleBridgeFactoryInterface \
	QAccessibleBridgePlugin \
	QAccessibleEditableTextInterface \
	QAccessibleEvent \
	QAccessibleFactoryInterface \
	QAccessibleImageInterface \
	QAccessibleInterface \
	QAccessibleInterfaceEx \
	QAccessibleObject \
	QAccessibleObjectEx \
	QAccessiblePlugin \
	QAccessibleSimpleEditableTextInterface \
	QAccessibleTable2CellInterface \
	QAccessibleTable2Interface \
	QAccessibleTableInterface \
	QAccessibleTextInterface \
	QAccessibleValueInterface \
	QAccessibleWidget \
	QAccessibleWidgetEx

GUI_ACCESSIBLE_INCLUDES = \
	qaccessible.h \
	qaccessible2.h \
	qaccessible2interface.h \
	qaccessibleactioninterface.h \
	qaccessibleapplication.h \
	qaccessiblebridge.h \
	qaccessiblebridgefactoryinterface.h \
	qaccessiblebridgeplugin.h \
	qaccessibleeditabletextinterface.h \
	qaccessibleevent.h \
	qaccessiblefactoryinterface.h \
	qaccessibleimageinterface.h \
	qaccessibleinterface.h \
	qaccessibleinterfaceex.h \
	qaccessibleobject.h \
	qaccessibleobjectex.h \
	qaccessibleplugin.h \
	qaccessiblesimpleeditabletextinterface.h \
	qaccessibletable2cellinterface.h \
	qaccessibletable2interface.h \
	qaccessibletableinterface.h \
	qaccessibletextinterface.h \
	qaccessiblevalueinterface.h \
	qaccessiblewidget.h \
	qaccessiblewidgetex.h


GUI_ACCESSIBLE_PRIVATE_INCLUDES = \
	qaccessible_mac_p.h

GUI_INCLUDES += $(GUI_ACCESSIBLE_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_ACCESSIBLE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/accessible/%,$(GUI_ACCESSIBLE_INCLUDES)) \
		$(patsubst %,src/gui/accessible/%,$(GUI_ACCESSIBLE_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/accessible/qaccessible.cpp \
	src/gui/accessible/qaccessible2.cpp \
	src/gui/accessible/qaccessibleobject.cpp \
	src/gui/accessible/qaccessiblewidget.cpp \
	src/gui/accessible/qaccessibleplugin.cpp

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/accessible/qaccessible_win.cpp
endif

if OSTYPE_LINUX
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/accessible/qaccessible_unix.cpp \
	src/gui/accessible/qaccessiblebridge.cpp
endif

if OSTYPE_BSD
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
        src/gui/accessible/qaccessible_unix.cpp \
        src/gui/accessible/qaccessiblebridge.cpp
endif

if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/accessible/qaccessible_mac.mm \
	src/gui/accessible/qaccessible_mac_cocoa.mm
endif

