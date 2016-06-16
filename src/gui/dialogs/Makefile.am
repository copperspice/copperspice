GUI_DIRS += src/gui/dialogs

GUI_PUBLIC_INCLUDES += \
	QAbstractPageSetupDialog \
	QAbstractPrintDialog \
	QColorDialog \
	QDialog \
	QErrorMessage \
	QFileDialog \
	QFileSystemModel \
	QFontDialog \
	QInputDialog \
	QMessageBox \
	QPageSetupDialog \
	QPrintDialog \
	QUnixPrintWidget \
	QPrintPreviewDialog \
	QProgressDialog \
	QWizard \
	QWizardPage 

GUI_DIALOGS_INCLUDES = \
	qabstractpagesetupdialog.h \
	qabstractprintdialog.h \
	qcolordialog.h \
	qdialog.h \
	qunixprintwidget.h \
	qerrormessage.h \
	qfiledialog.h \
	qfilesystemmodel.h \
	qfontdialog.h \
	qinputdialog.h \
	qmessagebox.h \
	qpagesetupdialog.h \
	qprintdialog.h \
	qprintpreviewdialog.h \
	qprogressdialog.h \
	qwizardpage.h \
	qwizard.h

GUI_DIALOGS_PRIVATE_INCLUDES = \
	qabstractpagesetupdialog_p.h \
	qabstractprintdialog_p.h \
	qcolordialog_p.h \
	qdialog_p.h \
	qfiledialog_p.h \
	qfileinfogatherer_p.h \
	qfilesystemmodel_p.h \
	qwizard_win_p.h \
	qfiledialog_win_p.h \
	qpagesetupdialog_unix_p.h \
	qfontdialog_p.h \
	qfscompleter_p.h \
	qsidebar_p.h

GUI_INCLUDES += $(GUI_DIALOGS_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_DIALOGS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/dialogs/%,$(GUI_DIALOGS_INCLUDES)) \
		$(patsubst %,src/gui/dialogs/%,$(GUI_DIALOGS_PRIVATE_INCLUDES)) \
		src/gui/dialogs/qpagesetupwidget.ui \
		src/gui/dialogs/qfiledialog.ui


lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qabstractpagesetupdialog.cpp \
	src/gui/dialogs/qabstractprintdialog.cpp \
	src/gui/dialogs/qcolordialog.cpp \
	src/gui/dialogs/qdialog.cpp \
	src/gui/dialogs/qerrormessage.cpp \
	src/gui/dialogs/qfiledialog.cpp \
	src/gui/dialogs/qfileinfogatherer.cpp \
	src/gui/dialogs/qfilesystemmodel.cpp \
	src/gui/dialogs/qfontdialog.cpp \
	src/gui/dialogs/qinputdialog.cpp \
	src/gui/dialogs/qmessagebox.cpp \
	src/gui/dialogs/qpagesetupdialog.cpp \
	src/gui/dialogs/qprintpreviewdialog.cpp \
	src/gui/dialogs/qprogressdialog.cpp \
	src/gui/dialogs/qsidebar.cpp \
	src/gui/dialogs/qwizard.cpp

nodist_lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qrc_qmessagebox.cpp

EXTRA_DIST+=\
	src/gui/dialogs/qmessagebox.qrc \
	src/gui/dialogs/qprintdialog.qrc \
	src/gui/dialogs/images/cslogo-64.png \
	src/gui/dialogs/images/fit-page-24.png \
	src/gui/dialogs/images/fit-page-32.png \
	src/gui/dialogs/images/fit-width-24.png \
	src/gui/dialogs/images/fit-width-32.png \
	src/gui/dialogs/images/go-first-24.png \
	src/gui/dialogs/images/go-first-32.png \
	src/gui/dialogs/images/go-last-24.png \
	src/gui/dialogs/images/go-last-32.png \
	src/gui/dialogs/images/go-next-24.png \
	src/gui/dialogs/images/go-next-32.png \
	src/gui/dialogs/images/go-previous-24.png \
	src/gui/dialogs/images/go-previous-32.png \
	src/gui/dialogs/images/layout-landscape-24.png \
	src/gui/dialogs/images/layout-landscape-32.png \
	src/gui/dialogs/images/layout-portrait-24.png \
	src/gui/dialogs/images/layout-portrait-32.png \
	src/gui/dialogs/images/page-setup-24.png \
	src/gui/dialogs/images/page-setup-32.png \
	src/gui/dialogs/images/print-24.png \
	src/gui/dialogs/images/print-32.png \
	src/gui/dialogs/images/view-page-multi-24.png \
	src/gui/dialogs/images/view-page-multi-32.png \
	src/gui/dialogs/images/view-page-one-24.png \
	src/gui/dialogs/images/view-page-one-32.png \
	src/gui/dialogs/images/view-page-sided-24.png \
	src/gui/dialogs/images/view-page-sided-32.png \
	src/gui/dialogs/images/zoom-in-24.png \
	src/gui/dialogs/images/zoom-in-32.png \
	src/gui/dialogs/images/zoom-out-24.png \
	src/gui/dialogs/images/zoom-out-32.png \
	src/gui/dialogs/images/status-color.png \
	src/gui/dialogs/images/status-gray-scale.png

$(lib_libCsGui@BUILD_ABI@_la_OBJECTS): src/gui/dialogs/ui_qfiledialog.h \
	src/gui/dialogs/ui_qpagesetupwidget.h \
	src/gui/dialogs/ui_qprintpropertieswidget.h \
	src/gui/dialogs/ui_qprintsettingsoutput.h \
	src/gui/dialogs/ui_qprintwidget.h \
	src/gui/dialogs/qrc_qprintdialog.cpp

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qfiledialog_win.cpp \
	src/gui/dialogs/qpagesetupdialog_win.cpp \
	src/gui/dialogs/qprintdialog_win.cpp \
	src/gui/dialogs/qwizard_win.cpp 
nodist_lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qrc_qprintdialog.cpp
endif

if OSTYPE_LINUX
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qprintdialog_unix.cpp \
	src/gui/dialogs/qpagesetupdialog_unix.cpp 
endif

if OSTYPE_BSD
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
        src/gui/dialogs/qprintdialog_unix.cpp \
        src/gui/dialogs/qpagesetupdialog_unix.cpp
endif

EXTRA_DIST += \
	src/gui/dialogs/qprintsettingsoutput.ui \
	src/gui/dialogs/qprintwidget.ui \
	src/gui/dialogs/qprintpropertieswidget.ui

if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qfiledialog_mac.mm \
	src/gui/dialogs/qfontdialog_mac.mm \
	src/gui/dialogs/qnspanelproxy_mac.mm \
	src/gui/dialogs/qpagesetupdialog_mac.mm \
	src/gui/dialogs/qprintdialog_mac.mm \
	src/gui/dialogs/qcolordialog_mac.mm 
nodist_lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qrc_qprintdialog.cpp
endif
