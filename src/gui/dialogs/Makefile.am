GUI_DIRS += src/gui/dialogs

GUI_PUBLIC_INCLUDES += \
	QColorDialog \
	QDialog \
	QErrorMessage \
	QFileDialog \
	QFileSystemModel \
	QFontDialog \
	QInputDialog \
	QMessageBox \
	QProgressDialog \
	QWizard \
	QWizardPage

GUI_DIALOGS_INCLUDES = \
	qcolordialog.h \
	qdialog.h \
	qerrormessage.h \
	qfiledialog.h \
	qfilesystemmodel.h \
	qfontdialog.h \
	qinputdialog.h \
	qmessagebox.h \
	qprogressdialog.h \
	qwizardpage.h \
	qwizard.h

GUI_DIALOGS_PRIVATE_INCLUDES = \
	qdialog_p.h \
	qfiledialog_p.h \
	qfileinfogatherer_p.h \
	qfilesystemmodel_p.h \
	qfontdialog_p.h \
	qfscompleter_p.h \
	qsidebar_p.h \
	qwizard_win_p.h

GUI_INCLUDES += $(GUI_DIALOGS_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_DIALOGS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/dialogs/%,$(GUI_DIALOGS_INCLUDES)) \
		$(patsubst %,src/gui/dialogs/%,$(GUI_DIALOGS_PRIVATE_INCLUDES)) \
		src/gui/dialogs/qfiledialog.ui

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qcolordialog.cpp \
	src/gui/dialogs/qdialog.cpp \
	src/gui/dialogs/qerrormessage.cpp \
	src/gui/dialogs/qfiledialog.cpp \
	src/gui/dialogs/qfiledialog_p.cpp \
	src/gui/dialogs/qfileinfogatherer.cpp \
	src/gui/dialogs/qfilesystemmodel.cpp \
	src/gui/dialogs/qfontdialog.cpp \
	src/gui/dialogs/qinputdialog.cpp \
	src/gui/dialogs/qmessagebox.cpp \
	src/gui/dialogs/qprogressdialog.cpp \
	src/gui/dialogs/qsidebar.cpp \
	src/gui/dialogs/qwizard.cpp

nodist_lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qrc_qmessagebox.cpp

EXTRA_DIST+=\
	src/gui/dialogs/qmessagebox.qrc \
	src/gui/dialogs/images/cslogo-64.png

$(lib_libCsGui@BUILD_ABI@_la_OBJECTS): \
	src/gui/dialogs/ui_qfiledialog.h

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/dialogs/qwizard_win.cpp
endif

