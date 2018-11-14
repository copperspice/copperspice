GUI_DIRS += src/gui/printing

GUI_PUBLIC_INCLUDES += \
	QAbstractPrintDialog \
	QPageLayout \
	QPageSetupDialog \
	QPageSize \
	QPagedPaintDevice \
	QPrinter \
	QPrintDialog \
	QPrintEngine \
	QPrinterInfo\
	QPrintPreviewDialog \
	QUnixPrintWidget

GUI_PRINTING_INCLUDES = \
	qabstractprintdialog.h \
	qpagelayout.h \
	qpagesize.h \
	qpagesetupdialog.h \
	qpagedpaintdevice.h \
	qprinter.h \
	qprintdialog.h \
	qprintengine.h \
	qprinterinfo.h \
	qprintpreviewdialog.h \
	qunixprintwidget.h

GUI_PRINTING_PRIVATE_INCLUDES = \
	qabstractprintdialog_p.h \
	qcups.cpp \
	qpagesetupdialog_p.h \
	qpagedpaintdevice_p.h \
	qpagesetupdialog_unix_p.h \
	qprinter_p.h \
	qprinterinfo_p.h \
	qprintengine_pdf_p.h \
	qprintengine_win_p.h


GUI_INCLUDES += $(GUI_PRINTING_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_PRINTING_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/printing/%,$(GUI_DIALOGS_INCLUDES)) \
		$(patsubst %,src/gui/printing/%,$(GUI_PRINTING_PRIVATE_INCLUDES)) \
		src/gui/printing/qpagesetupwidget.ui


lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/printing/qabstractprintdialog.cpp \
	src/gui/printing/qpagedpaintdevice.cpp \
	src/gui/printing/qpagelayout.cpp \
	src/gui/printing/qpagesetupdialog.cpp \
	src/gui/printing/qpagesize.cpp \
	src/gui/printing/qprintengine_pdf.cpp \
	src/gui/printing/qprinter.cpp \
	src/gui/printing/qprinterinfo.cpp \
	src/gui/printing/qprintpreviewdialog.cpp

nodist_lib_libCsGui@BUILD_ABI@_la_SOURCES +=


EXTRA_DIST+=\
	src/gui/printing/qprintdialog.qrc \
	src/gui/printing/qprintsettingsoutput.ui \
	src/gui/printing/qprintwidget.ui \
	src/gui/printing/qprintpropertieswidget.ui\
	src/gui/printing/images/fit-page-24.png \
	src/gui/printing/images/fit-page-32.png \
	src/gui/printing/images/fit-width-24.png \
	src/gui/printing/images/fit-width-32.png \
	src/gui/printing/images/go-first-24.png \
	src/gui/printing/images/go-first-32.png \
	src/gui/printing/images/go-last-24.png \
	src/gui/printing/images/go-last-32.png \
	src/gui/printing/images/go-next-24.png \
	src/gui/printing/images/go-next-32.png \
	src/gui/printing/images/go-previous-24.png \
	src/gui/printing/images/go-previous-32.png \
	src/gui/printing/images/layout-landscape-24.png \
	src/gui/printing/images/layout-landscape-32.png \
	src/gui/printing/images/layout-portrait-24.png \
	src/gui/printing/images/layout-portrait-32.png \
	src/gui/printing/images/page-setup-24.png \
	src/gui/printing/images/page-setup-32.png \
	src/gui/printing/images/print-24.png \
	src/gui/printing/images/print-32.png \
	src/gui/printing/images/view-page-multi-24.png \
	src/gui/printing/images/view-page-multi-32.png \
	src/gui/printing/images/view-page-one-24.png \
	src/gui/printing/images/view-page-one-32.png \
	src/gui/printing/images/view-page-sided-24.png \
	src/gui/printing/images/view-page-sided-32.png \
	src/gui/printing/images/zoom-in-24.png \
	src/gui/printing/images/zoom-in-32.png \
	src/gui/printing/images/zoom-out-24.png \
	src/gui/printing/images/zoom-out-32.png \
	src/gui/printing/images/status-color.png \
	src/gui/printing/images/status-gray-scale.png


$(lib_libCsGui@BUILD_ABI@_la_OBJECTS): \
	src/gui/printing/ui_qpagesetupwidget.h \
	src/gui/printing/ui_qprintpropertieswidget.h \
	src/gui/printing/ui_qprintsettingsoutput.h \
	src/gui/printing/ui_qprintwidget.h \
	src/gui/printing/qrc_qprintdialog.cpp

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/printing/qpagesetupdialog_win.cpp \
	src/gui/printing/qprintdialog_win.cpp \
	src/gui/printing/qprintengine_win.cpp

nodist_lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/printing/qrc_qprintdialog.cpp
endif

if OSTYPE_LINUX
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/printing/qcups.cpp \
	src/gui/printing/qprintdialog_unix.cpp \
	src/gui/printing/qpagesetupdialog_unix.cpp
endif

if OSTYPE_BSD
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/printing/qcups.cpp \
	src/gui/printing/qprintdialog_unix.cpp \
	src/gui/printing/qpagesetupdialog_unix.cpp
endif

if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/printing/qpagesetupdialog_mac.mm \
	src/gui/printing/qprintdialog_mac.mm

nodist_lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/printing/qrc_qprintdialog.cpp
endif
