GUI_DIRS += src/gui/widgets

GUI_PUBLIC_INCLUDES += \
	QAbstractButton \
	QAbstractScrollArea \
	QAbstractSlider \
	QAbstractSpinBox \
	QButtonGroup \
	QCalendarWidget \
	QCheckBox \
	QComboBox \
	QCommandLinkButton \
	QDateEdit \
	QDateTimeEdit \
	QDial \
	QDialogButtonBox \
	QDockWidget \
	QDoubleSpinBox \
	QDoubleValidator \
	QFocusFrame \
	QFontComboBox \
	QFrame \
	QGroupBox \
	QIntValidator \
	QLCDNumber \
	QLabel \
	QLineEdit \
	QMacCocoaViewContainer \
	QMacNativeWidget \
	QMainWindow \
	QMdiArea \
	QMdiSubWindow \
	QMenu \
	QMenuBar \
	QMenuItem \
	QPlainTextDocumentLayout \
	QPlainTextEdit \
	QPrintPreviewWidget \
	QProgressBar \
	QPushButton \
	QRadioButton \
	QRegExpValidator \
	QRubberBand \
	QScrollArea \
	QScrollBar \
	QSizeGrip \
	QSlider \
	QSpinBox \
	QSplashScreen \
	QSplitter \
	QSplitterHandle \
	QStackedWidget \
	QStatusBar \
	QTabBar \
	QTabWidget \
	QTextBrowser \
	QTimeEdit \
	QTextEdit \
	QToolBar \
	QToolBox \
	QToolButton \
	QValidator \
	QWorkspace

GUI_WIDGETS_INCLUDES = \
	qabstractbutton.h \
	qabstractscrollarea.h \
	qabstractslider.h \
	qabstractspinbox.h \
	qbuttongroup.h \
	qcalendarwidget.h \
	qcheckbox.h \
	qcombobox.h \
	qcommandlinkbutton.h \
	qdateedit.h \
	qdatetimeedit.h \
	qdial.h \
	qdialogbuttonbox.h \
	qdockwidget.h \
	qdoublespinbox.h \
	qdoublevalidator.h \
	qfocusframe.h \
	qfontcombobox.h \
	qframe.h \
	qgroupbox.h \
	qintvalidator.h \
	qlabel.h \
	qlcdnumber.h \
	qlineedit.h \
	qmaccocoaviewcontainer.h \
	qmaccocoaviewcontainer_mac.h \
	qmacnativewidget.h \
	qmacnativewidget_mac.h \
	qmainwindow.h \
	qmdiarea.h \
	qmdisubwindow.h \
	qmenu.h \
	qmenubar.h \
	qmenuitem.h \
	qplaintextdocumentlayout.h \
	qplaintextedit.h \
	qprintpreviewwidget.h \
	qprogressbar.h \
	qpushbutton.h \
	qradiobutton.h \
	qregexpvalidator.h \
	qrubberband.h \
	qscrollarea.h \
	qscrollbar.h \
	qsizegrip.h \
	qslider.h \
	qspinbox.h \
	qsplashscreen.h \
	qsplitter.h \
	qsplitterhandle.h \
	qstackedwidget.h \
	qstatusbar.h \
	qtabbar.h \
	qtabwidget.h \
	qtextbrowser.h \
	qtextedit.h \
	qtimeedit.h \
	qtoolbar.h \
	qtoolbox.h \
	qtoolbutton.h \
	qvalidator.h \
	qworkspace.h

GUI_WIDGETS_PRIVATE_INCLUDES = \
	qabstractbutton_p.h \
	qabstractscrollarea_p.h \
	qabstractplatformmenubar_p.h \
	qabstractslider_p.h \
	qabstractspinbox_p.h \
	qcalendartextnavigator_p.h \
	qcocoamenu_mac_p.h \
	qcocoatoolbardelegate_mac_p.h \
	qcombobox_p.h \
	qdatetimeedit_p.h \
	qdockarealayout_p.h \
	qdockwidget_p.h \
	qeffects_p.h \
	qframe_p.h \
	qlabel_p.h \
	qlinecontrol_p.h \
	qlineedit_p.h \
	qmaccocoaviewcontainer_mac.h \
	qmacnativewidget_mac.h \
	qmainwindowlayout_p.h \
	qmdiarea_p.h \
	qmdisubwindow_p.h \
	qmenu_p.h \
	qmenubar_p.h \
	qmenubar_x11_p.h \
	qplaintextedit_p.h \
	qpushbutton_p.h \
	qscrollarea_p.h \
	qsplitter_p.h \
	qtabbar_p.h \
	qtextedit_p.h \
	qtoolbar_p.h \
	qtoolbararealayout_p.h \
	qtoolbarextension_p.h \
	qtoolbarlayout_p.h \
	qtoolbarseparator_p.h \
	qwidgetanimator_p.h \
	qwidgetresizehandler_p.h


GUI_INCLUDES += $(GUI_WIDGETS_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_WIDGETS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/widgets/%,$(GUI_WIDGETS_INCLUDES)) \
		$(patsubst %,src/gui/widgets/%,$(GUI_WIDGETS_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/widgets/qabstractbutton.cpp \
	src/gui/widgets/qabstractslider.cpp \
	src/gui/widgets/qabstractspinbox.cpp \
	src/gui/widgets/qcalendarwidget.cpp \
	src/gui/widgets/qcheckbox.cpp \
	src/gui/widgets/qcombobox.cpp \
	src/gui/widgets/qcommandlinkbutton.cpp \
	src/gui/widgets/qdatetimeedit.cpp \
	src/gui/widgets/qdial.cpp \
	src/gui/widgets/qdialogbuttonbox.cpp \
	src/gui/widgets/qdockwidget.cpp \
	src/gui/widgets/qdockarealayout.cpp \
	src/gui/widgets/qeffects.cpp \
	src/gui/widgets/qfontcombobox.cpp \
	src/gui/widgets/qframe.cpp \
	src/gui/widgets/qgroupbox.cpp \
	src/gui/widgets/qlabel.cpp \
	src/gui/widgets/qlcdnumber.cpp \
	src/gui/widgets/qlineedit_p.cpp \
	src/gui/widgets/qlineedit.cpp \
	src/gui/widgets/qlinecontrol.cpp \
	src/gui/widgets/qmainwindow.cpp \
	src/gui/widgets/qmainwindowlayout.cpp \
	src/gui/widgets/qmdiarea.cpp \
	src/gui/widgets/qmdisubwindow.cpp \
	src/gui/widgets/qmenu.cpp \
	src/gui/widgets/qmenubar.cpp \
	src/gui/widgets/qprogressbar.cpp \
	src/gui/widgets/qpushbutton.cpp \
	src/gui/widgets/qradiobutton.cpp \
	src/gui/widgets/qrubberband.cpp \
	src/gui/widgets/qscrollbar.cpp \
	src/gui/widgets/qsizegrip.cpp \
	src/gui/widgets/qslider.cpp \
	src/gui/widgets/qspinbox.cpp \
	src/gui/widgets/qsplashscreen.cpp \
	src/gui/widgets/qsplitter.cpp \
	src/gui/widgets/qstackedwidget.cpp \
	src/gui/widgets/qstatusbar.cpp \
	src/gui/widgets/qtabbar.cpp \
	src/gui/widgets/qtabwidget.cpp \
	src/gui/widgets/qtextedit.cpp \
	src/gui/widgets/qtextbrowser.cpp \
	src/gui/widgets/qtoolbar.cpp \
	src/gui/widgets/qtoolbarlayout.cpp \
	src/gui/widgets/qtoolbarextension.cpp \
	src/gui/widgets/qtoolbarseparator.cpp \
	src/gui/widgets/qtoolbox.cpp \
	src/gui/widgets/qtoolbutton.cpp \
	src/gui/widgets/qvalidator.cpp \
	src/gui/widgets/qabstractscrollarea.cpp \
	src/gui/widgets/qwidgetresizehandler.cpp \
	src/gui/widgets/qfocusframe.cpp \
	src/gui/widgets/qscrollarea.cpp \
	src/gui/widgets/qworkspace.cpp \
	src/gui/widgets/qwidgetanimator.cpp \
	src/gui/widgets/qtoolbararealayout.cpp \
	src/gui/widgets/qplaintextedit.cpp \
	src/gui/widgets/qprintpreviewwidget.cpp


if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/widgets/qcocoamenu_mac.mm \
	src/gui/widgets/qcocoatoolbardelegate_mac.mm \
	src/gui/widgets/qmaccocoaviewcontainer_mac.mm \
	src/gui/widgets/qmacnativewidget_mac.mm \
	src/gui/widgets/qmainwindowlayout_mac.mm \
	src/gui/widgets/qmenu_mac.mm
endif


if X11
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/widgets/qmenubar_x11.cpp
endif


