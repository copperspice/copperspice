PLATFORMS_DIRS += src/plugins/platforms/xcb

PLATFORMS_XCB_PRIVATE_INCLUDES = \
	qxcbclipboard.h \
	qxcbconnection.h \
	qxcbintegration.h \
	qxcbkeyboard.h \
	qxcbdrag.h \
	qxcbexport.h \
	qxcbmime.h \
	qxcbobject.h \
	qxcbscreen.h \
	qxcbwindow.h \
	qxcbbackingstore.h \
	qxcbwmsupport.h \
	qxcbnativeinterface.h \
	qxcbcursor.h \
	qxcbimage.h \
	qxcbxsettings.h \
	qxcbsystemtraytracker.h \
	qxcbsessionmanager.h

PLATFORMS_XCB_EVENTS_PRIVATE_INCLUDES = \
	events/qgenericunixeventdispatcher_p.h \
	events/qgenericunixservices_p.h \
	events/qunixeventdispatcher_p.h \
	events/qxcbeventdispatcher_glib_p.h

PLATFORMS_XCB_FONTS_PRIVATE_INCLUDES = \
	fonts/qgenericunixfontdatabase_p.h

PLATFORMS_XCB_THEMES_PRIVATE_INCLUDES = \
	themes/qgenericunixthemes_p.h

PLATFORMS_XCB_XGL_PRIVATE_INCLUDES = \
	xgl/qxcbglintegration.h \
	xgl/qxcbglintegrationfactory.h \
	xgl/qxcbglintegrationplugin.h \
	xgl/qxcbnativeinterfacehandler.h

PLATFORMS_XCB_DIRS = events fonts themes xgl

noinst_HEADERS+=$(patsubst %,src/plugins/platforms/xcb/%,$(PLATFORMS_XCB_INCLUDES)) \
	$(patsubst %,src/plugins/platforms/xcb/%,$(PLATFORMS_XCB_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/platforms/xcb/%,$(PLATFORMS_XCB_EVENTS_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/platforms/xcb/%,$(PLATFORMS_XCB_FONTS_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/platforms/xcb/%,$(PLATFORMS_XCB_THEMES_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/platforms/xcb/%,$(PLATFORMS_XCB_XGL_PRIVATE_INCLUDES))

if BUILD_PLATFORMS_XCB_PLUGIN

PLATFORMS_PRIVATE_INCLUDES += $(notdir $(PLATFORMS_XCB_PRIVATE_INCLUDES)) \
	$(notdir $(PLATFORMS_XCB_EVENTS_PRIVATE_INCLUDES)) \
	$(notdir $(PLATFORMS_XCB_FONTS_PRIVATE_INCLUDES)) \
	$(notdir $(PLATFORMS_XCB_THEMES_PRIVATE_INCLUDES)) \
	$(notdir $(PLATFORMS_XCB_XGL_PRIVATE_INCLUDES))

lib_LTLIBRARIES += lib/CsGuiXcb@BUILD_ABI@.la

$(lib_CsGuiXcb@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) $(PLATFORMS_HEADERS_INSTALLED)

lib_CsGuiXcb@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iinclude/QtCore  -Iinclude/QtGui \
   -Iinclude/QtGui/platformheaders \
   -Iprivateinclude -Iprivateinclude/QtCore/private -Iprivateinclude/QtGui/private \
   -Iprivateinclude/QtGui/private/platforms \
   -I$(top_srcdir)/src/3rdparty/freetype/include \
   -I$(top_srcdir)/src/3rdparty/freetype/include/freetype \
   -I$(top_srcdir)/src/3rdparty/harfbuzz/src \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN \
   -DQT_USE_FONTCONFIG \
   -DQT_USE_FREETYPE \
   -DHAVE_POSIX_MEMALIGN \
   -DQT_NO_ACCESSIBILITY_ATSPI_BRIDGE \
   -DQT_NO_DBUS \
   -DXCB_USE_RENDER \
   -DXCB_USE_SM \
   -DXCB_USE_XLIB \
   -DXCB_USE_XINPUT2

lib_CsGuiXcb@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version -no-undefined -module \
   -lxcb -lxcb-image -lxcb-icccm -lxcb-sync -lxcb-xfixes -lxcb-shm -lxcb-randr -lxcb-shape \
   -lxcb-keysyms -lxcb-xinerama -lxcb-xkb -lxcb-render -lxcb-render-util \
   -lSM -lICE -lX11 -lXi \
   -lX11-xcb -lxkbcommon -lxkbcommon-x11

lib_CsGuiXcb@BUILD_ABI@_la_LIBADD=lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

lib_CsGuiXcb@BUILD_ABI@_la_SOURCES = \
	src/plugins/platforms/xcb/qxcbmain.cpp \
	src/plugins/platforms/xcb/qxcbclipboard.cpp \
	src/plugins/platforms/xcb/qxcbconnection.cpp \
	src/plugins/platforms/xcb/qxcbintegration.cpp \
	src/plugins/platforms/xcb/qxcbkeyboard.cpp \
	src/plugins/platforms/xcb/qxcbmime.cpp \
	src/plugins/platforms/xcb/qxcbdrag.cpp \
	src/plugins/platforms/xcb/qxcbscreen.cpp \
	src/plugins/platforms/xcb/qxcbwindow.cpp \
	src/plugins/platforms/xcb/qxcbbackingstore.cpp \
	src/plugins/platforms/xcb/qxcbwmsupport.cpp \
	src/plugins/platforms/xcb/qxcbnativeinterface.cpp \
	src/plugins/platforms/xcb/qxcbcursor.cpp \
	src/plugins/platforms/xcb/qxcbimage.cpp \
	src/plugins/platforms/xcb/qxcbxsettings.cpp \
	src/plugins/platforms/xcb/qxcbsystemtraytracker.cpp \
	src/plugins/platforms/xcb/qxcbsessionmanager.cpp \
	src/plugins/platforms/xcb/qxcbconnection_xi2.cpp \
	\
	src/plugins/platforms/xcb/xgl/qxcbglintegrationfactory.cpp \
	src/plugins/platforms/xcb/xgl/qxcbglintegration.cpp \
	src/plugins/platforms/xcb/xgl/qxcbnativeinterfacehandler.cpp \
	src/plugins/platforms/xcb/events/qgenericunixeventdispatcher.cpp \
	src/plugins/platforms/xcb/events/qgenericunixservices.cpp \
	src/plugins/platforms/xcb/events/qunixeventdispatcher.cpp \
	src/plugins/platforms/xcb/events/qxcbeventdispatcher_glib.cpp \
	src/plugins/platforms/xcb/themes/qgenericunixthemes.cpp

nodist_lib_CsGuiXcb@BUILD_ABI@_la_SOURCES =

endif

EXTRA_DIST +=
