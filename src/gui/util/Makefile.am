GUI_DIRS += src/gui/util

GUI_PUBLIC_INCLUDES += \
	QCompleter \
	QDesktopServices \
	QScroller \
	QScrollerProperties \
	QSystemTrayIcon \
	QUndoCommand \
	QUndoGroup \
	QUndoStack \
	QUndoView

GUI_UTIL_INCLUDES = \
	qcompleter.h \
	qdesktopservices.h \
	qscroller.h \
	qscrollerproperties.h \
	qsystemtrayicon.h \
	qundocommand.h \
	qundogroup.h \
	qundostack.h \
	qundoview.h

GUI_UTIL_PRIVATE_INCLUDES = \
	qabstractlayoutstyleinfo_p.h \
	qcompleter_p.h \
	qflickgesture_p.h \
	qlayoutpolicy_p.h \
	qscroller_p.h \
	qscrollerproperties_p.h \
	qsystemtrayicon_p.h \
	qundostack_p.h

GUI_INCLUDES += $(GUI_UTIL_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_UTIL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/util/%,$(GUI_UTIL_INCLUDES)) \
		$(patsubst %,src/gui/util/%,$(GUI_UTIL_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/util/qabstractlayoutstyleinfo.cpp \
	src/gui/util/qcompleter.cpp \
	src/gui/util/qdesktopservices.cpp \
	src/gui/util/qlayoutpolicy.cpp \
	src/gui/util/qscroller.cpp \
	src/gui/util/qscrollerproperties.cpp \
	src/gui/util/qsystemtrayicon.cpp \
	src/gui/util/qsystemtrayicon_qpa.cpp \
	src/gui/util/qundogroup.cpp \
	src/gui/util/qundostack.cpp \
	src/gui/util/qundoview.cpp

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/util/qsystemtrayicon_win.cpp
endif

if X11
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/util/qsystemtrayicon_x11.cpp
endif

if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/util/qsystemtrayicon_mac.mm
endif
