PLATFORMS_DIRS += src/plugins/platforms/cocoa

PLATFORMS_COCOA_PRIVATE_INCLUDES = \
	messages.h \
	qcocoaintegration.h \
	qcocoatheme.h \
	qcocoabackingstore.h \
	qcocoawindow.h \
	qnsview.h \
	qnswindowdelegate.h \
	qcocoanativeinterface.h \
	qcocoaeventdispatcher.h \
	qcocoaapplicationdelegate.h \
	qcocoaapplication.h \
	qcocoamenu.h \
	qcocoamenuitem.h \
	qcocoamenubar.h \
	qcocoamenuloader.h \
	qcocoahelpers.h \
	qmultitouch_mac_p.h \
	qcocoaaccessibilityelement.h \
	qcocoaaccessibility.h \
	qcocoacolordialoghelper.h \
	qcocoafiledialoghelper.h \
	qcocoafontdialoghelper.h \
	qcocoacursor.h \
	qcocoaclipboard.h \
	qcocoadrag.h \
	qmacclipboard.h \
	qcocoasystemsettings.h \
	qcocoainputcontext.h \
	qcocoaservices.h \
	qcocoasystemtrayicon.h \
	qcocoaintrospection.h \
	qcocoakeymapper.h \
	qcocoamimetypes.h

noinst_HEADERS+=\
   $(patsubst %,src/plugins/platforms/cocoa/%,$(PLATFORMS_COCOA_PRIVATE_INCLUDES))

if BUILD_PLATFORMS_COCOA_PLUGIN

PLATFORMS_PRIVATE_INCLUDES += $(PLATFORMS_COCOA_PRIVATE_INCLUDES)

lib_LTLIBRARIES += lib/CsGuiCocoa@BUILD_ABI@.la

$(lib_CsGuiCocoa@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) $(PLATFORMS_HEADERS_INSTALLED)

lib_CsGuiCocoa@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iinclude/QtCore -Iinclude/QtGui \
   -Iinclude/QtGui/platformheaders \
   -Iprivateinclude -Iprivateinclude/QtCore/private -Iprivateinclude/QtGui/private \
   -Iprivateinclude/QtGui/private/platforms \
   -I$(top_srcdir)/src/3rdparty/freetype/include \
   -I$(top_srcdir)/src/3rdparty/freetype/include/freetype \
   -I$(top_srcdir)/src/3rdparty/harfbuzz/src \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\"  \
   -DQT_PLUGIN \
   -DQT_USE_FREETYPE

lib_CsGuiCocoa@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version -no-undefined -module \
   -framework Cocoa -framework Carbon -framework IOKit -lcups

lib_CsGuiCocoa@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

lib_CsGuiCocoa@BUILD_ABI@_la_SOURCES = \
	src/plugins/platforms/cocoa/messages.cpp \
	src/plugins/platforms/cocoa/main.mm \
	src/plugins/platforms/cocoa/qcocoaintegration.mm \
	src/plugins/platforms/cocoa/qcocoatheme.mm \
	src/plugins/platforms/cocoa/qcocoabackingstore.mm \
	src/plugins/platforms/cocoa/qcocoawindow.mm \
	src/plugins/platforms/cocoa/qnsview.mm \
	src/plugins/platforms/cocoa/qnsviewaccessibility.mm \
	src/plugins/platforms/cocoa/qnswindowdelegate.mm \
	src/plugins/platforms/cocoa/qcocoanativeinterface.mm \
	src/plugins/platforms/cocoa/qcocoaeventdispatcher.mm \
	src/plugins/platforms/cocoa/qcocoaapplicationdelegate.mm \
	src/plugins/platforms/cocoa/qcocoaapplication.mm \
	src/plugins/platforms/cocoa/qcocoamenu.mm \
	src/plugins/platforms/cocoa/qcocoamenuitem.mm \
	src/plugins/platforms/cocoa/qcocoamenubar.mm \
	src/plugins/platforms/cocoa/qcocoamenuloader.mm \
	src/plugins/platforms/cocoa/qcocoahelpers.mm \
	src/plugins/platforms/cocoa/qmultitouch_mac.mm \
	src/plugins/platforms/cocoa/qcocoaaccessibilityelement.mm \
	src/plugins/platforms/cocoa/qcocoaaccessibility.mm \
	src/plugins/platforms/cocoa/qcocoacolordialoghelper.mm \
	src/plugins/platforms/cocoa/qcocoafiledialoghelper.mm \
	src/plugins/platforms/cocoa/qcocoafontdialoghelper.mm \
	src/plugins/platforms/cocoa/qcocoacursor.mm \
	src/plugins/platforms/cocoa/qcocoaclipboard.mm \
	src/plugins/platforms/cocoa/qcocoadrag.mm \
	src/plugins/platforms/cocoa/qmacclipboard.mm \
	src/plugins/platforms/cocoa/qcocoasystemsettings.mm \
	src/plugins/platforms/cocoa/qcocoainputcontext.mm \
	src/plugins/platforms/cocoa/qcocoaservices.mm \
	src/plugins/platforms/cocoa/qcocoasystemtrayicon.mm \
	src/plugins/platforms/cocoa/qcocoaintrospection.mm \
	src/plugins/platforms/cocoa/qcocoakeymapper.mm \
	src/plugins/platforms/cocoa/qcocoamimetypes.mm

nodist_lib_CsGuiCocoa@BUILD_ABI@_la_SOURCES = \
	src/plugins/platforms/cocoa/qrc_qcocoaresources.cpp

endif

EXTRA_DIST += src/plugins/platforms/cocoa/qcocoaresources.qrc \
   src/plugins/platforms/cocoa/APPLE_LICENSE.TXT
