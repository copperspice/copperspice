GUI_DIRS += src/gui/accessible

GUI_PUBLIC_INCLUDES += \
	QAccessible \
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
	QAccessibleObject \
	QAccessiblePlugin \
	QAccessibleTableInterface \
	QAccessibleTextInterface \
	QAccessibleValueInterface \
	QAccessibleWidget

GUI_ACCESSIBLE_INCLUDES = \
	qaccessible.h \
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
	qaccessibleobject.h \
	qaccessibleplugin.h \
	qaccessibletableinterface.h \
	qaccessibletextinterface.h \
	qaccessiblevalueinterface.h \
	qaccessiblewidget.h

GUI_ACCESSIBLE_PRIVATE_INCLUDES = \
	qaccessiblecache_p.h \
	qaccessiblemenu_p.h \
	qaccessiblewidget_common_p.h \
	qaccessiblewidget_complex_p.h \
	qaccessiblewidget_factory_p.h \
	qaccessiblewidget_range_p.h  \
	qaccessiblewidget_simple_p.h \
	qaccessiblewidget_views_p.h


GUI_INCLUDES += $(GUI_ACCESSIBLE_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_ACCESSIBLE_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/accessible/%,$(GUI_ACCESSIBLE_INCLUDES)) \
		$(patsubst %,src/gui/accessible/%,$(GUI_ACCESSIBLE_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/accessible/qaccessible.cpp \
	src/gui/accessible/qaccessiblecache.cpp \
	src/gui/accessible/qaccessibleobject.cpp \
	src/gui/accessible/qaccessiblemenu.cpp \
	src/gui/accessible/qaccessibleplugin.cpp \
	src/gui/accessible/qaccessiblewidget.cpp \
	src/gui/accessible/qaccessiblewidget_complex.cpp \
	src/gui/accessible/qaccessiblewidget_common.cpp \
	src/gui/accessible/qaccessiblewidget_factory.cpp \
	src/gui/accessible/qaccessiblewidget_range.cpp  \
	src/gui/accessible/qaccessiblewidget_simple.cpp \
	src/gui/accessible/qaccessiblewidget_views.cpp


if OSTYPE_LINUX
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/accessible/qaccessiblebridge.cpp
endif

if OSTYPE_BSD
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
        src/gui/accessible/qaccessiblebridge.cpp
endif

if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/accessible/qaccessible_cache_mac.mm
endif

