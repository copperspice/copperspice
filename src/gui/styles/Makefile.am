GUI_DIRS += src/gui/styles

GUI_PUBLIC_INCLUDES += \
	QCDEStyle \
	QCleanlooksStyle \
	QCommonStyle \
	QGtkStyle \
	QMacStyle \
	QMotifStyle \
	QPlastiqueStyle \
	QProxyStyle \
	QStyle \
	QStyleFactory \
	QStyleFactoryInterface \
	QStyleHintReturn \
	QStyleHintReturnMask \
	QStyleHintReturnVariant \
	QStyleOption \
	QStyleOptionButton \
	QStyleOptionComboBox \
	QStyleOptionComplex \
	QStyleOptionDockWidget \
	QStyleOptionDockWidgetV2 \
	QStyleOptionFocusRect \
	QStyleOptionFrame \
	QStyleOptionFrameV2 \
	QStyleOptionFrameV3 \
	QStyleOptionGraphicsItem \
	QStyleOptionGroupBox \
	QStyleOptionHeader \
	QStyleOptionMenuItem \
	QStyleOptionProgressBar \
	QStyleOptionProgressBarV2 \
	QStyleOptionRubberBand \
	QStyleOptionSizeGrip \
	QStyleOptionSlider \
	QStyleOptionSpinBox \
	QStyleOptionTab \
	QStyleOptionTabBarBase \
	QStyleOptionTabBarBaseV2 \
	QStyleOptionTabV2 \
	QStyleOptionTabV3 \
	QStyleOptionTabWidgetFrame \
	QStyleOptionTabWidgetFrameV2 \
	QStyleOptionTitleBar \
	QStyleOptionToolBar \
	QStyleOptionToolBoxV2 \
	QStyleOptionToolBox \
	QStyleOptionToolBarV2 \
	QStyleOptionToolButton \
	QStyleOptionViewItem \
	QStyleOptionViewItemV2 \
	QStyleOptionViewItemV3 \
	QStyleOptionViewItemV4 \
	QStylePlugin \
	QWindowsStyle \
	QWindowsVistaStyle \
	QWindowsXPStyle

GUI_STYLES_INCLUDES = \
	qcdestyle.h \
	qcleanlooksstyle.h \
	qcommonstyle.h \
	qgtkstyle.h \
	qmacstyle_mac.h \
	qmacstyle.h \
	qmotifstyle.h \
	qplastiquestyle.h \
	qproxystyle.h \
	qstyle.h \
	qstylefactory.h \
	qstylefactoryinterface.h \
	qstylehintreturn.h \
	qstylehintreturnmask.h \
	qstylehintreturnvariant.h \
	qstyleoption.h \
	qstyleoptionbutton.h \
	qstyleoptioncombobox.h \
	qstyleoptioncomplex.h \
	qstyleoptiondockwidget.h \
	qstyleoptiondockwidgetv2.h \
	qstyleoptionfocusrect.h \
	qstyleoptionframe.h \
	qstyleoptionframev2.h \
	qstyleoptionframev3.h \
	qstyleoptiongraphicsitem.h \
	qstyleoptiongroupbox.h \
	qstyleoptionheader.h \
	qstyleoptionmenuitem.h \
	qstyleoptionprogressbar.h \
	qstyleoptionprogressbarv2.h \
	qstyleoptionrubberband.h \
	qstyleoptionsizegrip.h \
	qstyleoptionslider.h \
	qstyleoptionspinbox.h \
	qstyleoptiontab.h \
	qstyleoptiontabbarbase.h \
	qstyleoptiontabbarbasev2.h \
	qstyleoptiontabv2.h \
	qstyleoptiontabv3.h \
	qstyleoptiontabwidgetframe.h \
	qstyleoptiontabwidgetframev2.h \
	qstyleoptiontitlebar.h \
	qstyleoptiontoolbar.h \
	qstyleoptiontoolbox.h \
	qstyleoptiontoolboxv2.h \
	qstyleoptiontoolbarv2.h \
	qstyleoptiontoolbutton.h \
	qstyleoptionviewitem.h \
	qstyleoptionviewitemv2.h \
	qstyleoptionviewitemv3.h \
	qstyleoptionviewitemv4.h \
	qstyleplugin.h \
	qwindowsstyle.h \
	qwindowsvistastyle.h \
	qwindowsxpstyle.h

GUI_STYLES_PRIVATE_INCLUDES = \
	qcommonstylepixmaps_p.h \
	qcommonstyle_p.h \
	qcleanlooksstyle_p.h \
	qgtkstyle_p.h \
	qgtkpainter_p.h \
	qmotifstyle_p.h \
	qmacstyle_mac_p.h \
	qmacstylepixmaps_mac_p.h \
	qrenderrule_p.h \
	qstyle_p.h \
	qstylehelper_p.h \
	qproxystyle_p.h \
	qstylesheetstyle_p.h \
	qwindowsstyle_p.h \
	qwindowsxpstyle_p.h \
	qwindowsvistastyle_p.h

GUI_INCLUDES += $(GUI_STYLES_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_STYLES_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/styles/%,$(GUI_STYLES_INCLUDES)) \
		$(patsubst %,src/gui/styles/%,$(GUI_STYLES_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/styles/qstyle.cpp \
	src/gui/styles/qmotifstyle.cpp \
	src/gui/styles/qstylefactory.cpp \
	src/gui/styles/qstyleoption.cpp \
	src/gui/styles/qstyleplugin.cpp \
	src/gui/styles/qstylehelper.cpp \
	src/gui/styles/qcommonstyle.cpp \
	src/gui/styles/qproxystyle.cpp \
   src/gui/styles/qstylesheetstyle.cpp \
   src/gui/styles/qstylesheetstyle_default.cpp \
	src/gui/styles/qplastiquestyle.cpp \
	src/gui/styles/qcleanlooksstyle.cpp \
	src/gui/styles/qcdestyle.cpp

nodist_lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/styles/qrc_qstyle.cpp

EXTRA_DIST+=\
	src/gui/styles/qstyle.qrc \
	src/gui/styles/images/filelink-16.png \
	src/gui/styles/images/filelink-32.png \
	src/gui/styles/images/filelink-128.png \
	src/gui/styles/images/file-16.png \
	src/gui/styles/images/file-32.png \
	src/gui/styles/images/file-128.png \
	src/gui/styles/images/newdirectory-16.png \
	src/gui/styles/images/newdirectory-32.png \
	src/gui/styles/images/newdirectory-128.png \
	src/gui/styles/images/parentdir-16.png \
	src/gui/styles/images/parentdir-32.png \
	src/gui/styles/images/parentdir-128.png \
	src/gui/styles/images/dvd-16.png \
	src/gui/styles/images/dvd-32.png \
	src/gui/styles/images/dvd-128.png \
	src/gui/styles/images/cdr-16.png \
	src/gui/styles/images/cdr-32.png \
	src/gui/styles/images/cdr-128.png \
	src/gui/styles/images/floppy-16.png \
	src/gui/styles/images/floppy-32.png \
	src/gui/styles/images/floppy-128.png \
	src/gui/styles/images/harddrive-16.png \
	src/gui/styles/images/harddrive-32.png \
	src/gui/styles/images/harddrive-128.png \
	src/gui/styles/images/trash-16.png \
	src/gui/styles/images/trash-32.png \
	src/gui/styles/images/trash-128.png \
	src/gui/styles/images/networkdrive-16.png \
	src/gui/styles/images/networkdrive-32.png \
	src/gui/styles/images/networkdrive-128.png \
	src/gui/styles/images/computer-16.png \
	src/gui/styles/images/computer-32.png \
	src/gui/styles/images/desktop-16.png \
	src/gui/styles/images/desktop-32.png \
	src/gui/styles/images/dirclosed-16.png \
	src/gui/styles/images/dirclosed-32.png \
	src/gui/styles/images/dirclosed-128.png \
	src/gui/styles/images/dirlink-16.png \
	src/gui/styles/images/dirlink-32.png \
	src/gui/styles/images/dirlink-128.png \
	src/gui/styles/images/diropen-16.png \
	src/gui/styles/images/diropen-32.png \
	src/gui/styles/images/diropen-128.png \
	src/gui/styles/images/left-16.png \
	src/gui/styles/images/left-32.png \
	src/gui/styles/images/left-128.png \
	src/gui/styles/images/right-16.png \
	src/gui/styles/images/right-32.png \
	src/gui/styles/images/right-128.png \
	src/gui/styles/images/up-16.png \
	src/gui/styles/images/up-32.png \
	src/gui/styles/images/up-128.png \
	src/gui/styles/images/down-16.png \
	src/gui/styles/images/down-32.png \
	src/gui/styles/images/down-128.png \
	src/gui/styles/images/filecontents-16.png \
	src/gui/styles/images/filecontents-32.png \
	src/gui/styles/images/filecontents-128.png \
	src/gui/styles/images/fileinfo-16.png \
	src/gui/styles/images/fileinfo-32.png \
	src/gui/styles/images/fileinfo-128.png \
	src/gui/styles/images/viewdetailed-16.png \
	src/gui/styles/images/viewdetailed-32.png \
	src/gui/styles/images/viewdetailed-128.png \
	src/gui/styles/images/viewlist-16.png \
	src/gui/styles/images/viewlist-32.png \
	src/gui/styles/images/viewlist-128.png \
	src/gui/styles/images/fontbitmap-16.png \
	src/gui/styles/images/fonttruetype-16.png \
	src/gui/styles/images/standardbutton-apply-128.png \
	src/gui/styles/images/standardbutton-apply-16.png \
	src/gui/styles/images/standardbutton-apply-32.png \
	src/gui/styles/images/standardbutton-cancel-128.png \
	src/gui/styles/images/standardbutton-cancel-16.png \
	src/gui/styles/images/standardbutton-cancel-32.png \
	src/gui/styles/images/standardbutton-clear-128.png \
	src/gui/styles/images/standardbutton-clear-16.png \
	src/gui/styles/images/standardbutton-clear-32.png \
	src/gui/styles/images/standardbutton-close-128.png \
	src/gui/styles/images/standardbutton-close-16.png \
	src/gui/styles/images/standardbutton-close-32.png \
	src/gui/styles/images/standardbutton-delete-128.png \
	src/gui/styles/images/standardbutton-delete-16.png \
	src/gui/styles/images/standardbutton-delete-32.png \
	src/gui/styles/images/standardbutton-help-128.png \
	src/gui/styles/images/standardbutton-help-16.png \
	src/gui/styles/images/standardbutton-help-32.png \
	src/gui/styles/images/standardbutton-no-128.png \
	src/gui/styles/images/standardbutton-no-16.png \
	src/gui/styles/images/standardbutton-no-32.png \
	src/gui/styles/images/standardbutton-ok-128.png \
	src/gui/styles/images/standardbutton-ok-16.png \
	src/gui/styles/images/standardbutton-ok-32.png \
	src/gui/styles/images/standardbutton-open-128.png \
	src/gui/styles/images/standardbutton-open-16.png \
	src/gui/styles/images/standardbutton-open-32.png \
	src/gui/styles/images/standardbutton-save-128.png \
	src/gui/styles/images/standardbutton-save-16.png \
	src/gui/styles/images/standardbutton-save-32.png \
	src/gui/styles/images/standardbutton-yes-128.png \
	src/gui/styles/images/standardbutton-yes-16.png \
	src/gui/styles/images/standardbutton-yes-32.png \
	src/gui/styles/images/standardbutton-closetab-16.png \
	src/gui/styles/images/standardbutton-closetab-down-16.png \
	src/gui/styles/images/standardbutton-closetab-hover-16.png \
	src/gui/styles/images/refresh-24.png \
	src/gui/styles/images/refresh-32.png \
	src/gui/styles/images/stop-24.png \
	src/gui/styles/images/stop-32.png \
	src/gui/styles/images/media-stop-16.png \
	src/gui/styles/images/media-stop-32.png \
	src/gui/styles/images/media-play-16.png \
	src/gui/styles/images/media-play-32.png \
	src/gui/styles/images/media-pause-16.png \
	src/gui/styles/images/media-pause-32.png \
	src/gui/styles/images/media-seek-forward-16.png \
	src/gui/styles/images/media-seek-forward-32.png \
	src/gui/styles/images/media-seek-backward-16.png \
	src/gui/styles/images/media-seek-backward-32.png \
	src/gui/styles/images/media-skip-forward-16.png \
	src/gui/styles/images/media-skip-forward-32.png \
	src/gui/styles/images/media-skip-backward-16.png \
	src/gui/styles/images/media-skip-backward-32.png \
	src/gui/styles/images/media-volume-16.png \
	src/gui/styles/images/media-volume-muted-16.png \
	src/gui/styles/images/closedock-16.png \
	src/gui/styles/images/closedock-down-16.png \
	src/gui/styles/images/dockdock-16.png \
	src/gui/styles/images/dockdock-down-16.png


if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/styles/qwindowsstyle.cpp \
	src/gui/styles/qwindowsxpstyle.cpp  \
   src/gui/styles/qwindowsvistastyle.cpp
endif

if X11
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/styles/qwindowsstyle.cpp
endif

if OSTYPE_BSD
lib_libCsGui@BUILD_ABI@_la_CXXFLAGS += -DQT_NO_STYLE_MAC -DQT_NO_STYLE_WINDOWSXP
endif

if OSTYPE_DARWIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/styles/qwindowsstyle.cpp  \
	src/gui/styles/qmacstyle_mac.mm
lib_libCsGui@BUILD_ABI@_la_CXXFLAGS += -DQT_NO_STYLE_WINDOWSXP -DQT_NO_STYLE_GTK

endif

if GTK
lib_libCsGui@BUILD_ABI@_la_CXXFLAGS += $(GTK_CFLAGS)
lib_libCsGui@BUILD_ABI@_la_LDFLAGS += $(GTK_LIBS)
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/styles/qgtkstyle.cpp \
	src/gui/styles/qgtkpainter.cpp \
	src/gui/styles/qgtkstyle_p.cpp
else
lib_libCsGui@BUILD_ABI@_la_CXXFLAGS += -DQT_NO_STYLE_GTK
endif
