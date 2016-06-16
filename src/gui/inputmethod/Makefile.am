GUI_DIRS += src/gui/inputmethod

GUI_PUBLIC_INCLUDES += \
	QInputContext \
	QInputContextFactory \
	QInputContextFactoryInterface \
	QInputContextPlugin

GUI_INPUTMETHOD_INCLUDES = \
	qinputcontext.h \
	qinputcontextfactory.h \
	qinputcontextfactoryinterface.h \
	qinputcontextplugin.h

GUI_INPUTMETHOD_PRIVATE_INCLUDES = \
	qinputcontext_p.h \
	qmacinputcontext_p.h \
	qximinputcontext_p.h \
	qwininputcontext_p.h

GUI_INCLUDES += $(GUI_INPUTMETHOD_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_INPUTMETHOD_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/inputmethod/%,$(GUI_INPUTMETHOD_INCLUDES)) \
		$(patsubst %,src/gui/inputmethod/%,$(GUI_INPUTMETHOD_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/inputmethod/qinputcontextfactory.cpp \
	src/gui/inputmethod/qinputcontextplugin.cpp \
	src/gui/inputmethod/qinputcontext.cpp

if X11
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/inputmethod/qximinputcontext_x11.cpp
endif

if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/inputmethod/qmacinputcontext_mac.cpp
endif

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/inputmethod/qwininputcontext_win.cpp
endif
