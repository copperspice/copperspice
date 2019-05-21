# Freetype
lib_libCsGui@BUILD_ABI@_la_CXXFLAGS += \
   -DQT_USE_FREETYPE

GUI_DIRS += src/gui/platform  src/gui/platform/platformheaders

GUI_PUBLIC_INCLUDES += \
	QPlatform_Accessibility \
	QPlatform_BackingStore \
	QPlatform_Clipboard \
	QPlatform_Cursor \
	QPlatform_DialogHelper \
	QPlatform_Drag \
	QPlatform_FontDatabase \
	QPlatform_GraphicsBuffer \
	QPlatform_GraphicsBufferHelper \
	QPlatform_HeaderHelper \
	QPlatform_Integration \
	QPlatform_IntegrationPlugin \
	QPlatform_InputContext \
	QPlatform_Menu \
	QPlatform_NativeInterface \
	QPlatform_OffScreenSurface \
	QPlatform_OpenGLContext \
	QPlatform_Pixmap \
	QPlatform_PrintDevice \
	QPlatform_PrinterSupport \
	QPlatform_PrintPlugin \
	QPlatform_Screen \
	QPlatform_Services \
	QPlatform_SessionManager \
	QPlatform_SharedGraphicsCache \
	QPlatform_Surface \
	QPlatform_SystemTrayIcon \
	QPlatform_Theme \
	QPlatform_ThemePlugin \
	QPlatform_Window

GUI_PLATFORM_INCLUDES = \
	qplatform_accessibility.h \
	qplatform_backingstore.h \
	qplatform_clipboard.h \
	qplatform_cursor.h \
	qplatform_dialoghelper.h \
	qplatform_drag.h \
	qplatform_fontdatabase.h \
	qplatform_graphicsbuffer.h \
	qplatform_graphicsbufferhelper.h \
	qplatform_headerhelper.h \
	qplatform_inputcontext.h \
	qplatform_integration.h \
	qplatform_integrationplugin.h \
	qplatform_menu.h \
	qplatform_nativeinterface.h \
	qplatform_offscreensurface.h \
	qplatform_openglcontext.h \
	qplatform_pixmap.h \
	qplatform_printdevice.h \
	qplatform_printersupport.h \
	qplatform_printplugin.h \
	qplatform_screen.h \
	qplatform_services.h \
	qplatform_sessionmanager.h \
	qplatform_sharedgraphicscache.h \
	qplatform_surface.h \
	qplatform_systemtrayicon.h \
	qplatform_theme.h \
	qplatform_themeplugin.h \
	qplatform_window.h

GUI_PLATFORM_PRIVATE_INCLUDES = \
	qplatform_inputcontext_p.h \
	qplatform_inputcontextfactory_p.h \
	qplatform_inputcontextplugin_p.h \
	qplatform_integrationfactory_p.h \
	qplatform_screen_p.h \
	qplatform_theme_p.h \
	qplatform_themefactory_p.h \
	qplatform_window_p.h

GUI_PLATFORMHEADERS_PRIVATE_INCLUDES = \
	platformheaders/qbasicfontdatabase_p.h \
	platformheaders/qfontengine_ft_p.h

GUI_PLATFORMHEADERS_INCLUDES = \
	platformheaders/cocoa/qcocoawindowfunctions.h \
	platformheaders/cocoa/qcocoanativecontext.h \
	platformheaders/xcb/qxcbintegrationfunctions.h \
	platformheaders/xcb/qxcbscreenfunctions.h \
	platformheaders/xcb/qxcbwindowfunctions.h \
	platformheaders/windows/qwindowswindowfunctions.h \
	platformheaders/windows/qwglnativecontext.h

GUI_PLATFORMHEADERS_DIRS = xcb windows cocoa

GUI_INCLUDES += $(GUI_PLATFORM_INCLUDES) \
	$(patsubst %,platformheaders/%,$(notdir $(GUI_PLATFORMHEADERS_INCLUDES)))

GUI_PRIVATE_INCLUDES += $(GUI_PLATFORM_PRIVATE_INCLUDES) \
	$(notdir $(GUI_PLATFORMHEADERS_PRIVATE_INCLUDES))

noinst_HEADERS+=$(patsubst %,src/gui/platform/%,$(GUI_PLATFORM_INCLUDES)) \
	$(patsubst %,src/gui/platform/%,$(GUI_PLATFORM_PRIVATE_INCLUDES)) \
	$(patsubst %,src/gui/platform/%,$(GUI_PLATFORMHEADERS_PRIVATE_INCLUDES))  \
	$(patsubst %,src/gui/platform/%,$(GUI_PLATFORMHEADERS_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/platform/qplatform_accessibility.cpp \
	src/gui/platform/qplatform_backingstore.cpp \
	src/gui/platform/qplatform_clipboard.cpp \
	src/gui/platform/qplatform_cursor.cpp \
	src/gui/platform/qplatform_dialoghelper.cpp \
	src/gui/platform/qplatform_drag.cpp \
	src/gui/platform/qplatform_fontdatabase.cpp \
	src/gui/platform/qplatform_graphicsbuffer.cpp \
	src/gui/platform/qplatform_graphicsbufferhelper.cpp \
	src/gui/platform/qplatform_inputcontext.cpp \
	src/gui/platform/qplatform_inputcontextfactory.cpp \
	src/gui/platform/qplatform_inputcontextplugin.cpp \
	src/gui/platform/qplatform_integration.cpp \
	src/gui/platform/qplatform_integrationfactory.cpp \
	src/gui/platform/qplatform_integrationplugin.cpp \
	src/gui/platform/qplatform_menu.cpp \
	src/gui/platform/qplatform_nativeinterface.cpp \
	src/gui/platform/qplatform_offscreensurface.cpp \
	src/gui/platform/qplatform_openglcontext.cpp \
	src/gui/platform/qplatform_pixmap.cpp \
	src/gui/platform/qplatform_printdevice.cpp \
	src/gui/platform/qplatform_printersupport.cpp \
	src/gui/platform/qplatform_printplugin.cpp \
	src/gui/platform/qplatform_screen.cpp \
	src/gui/platform/qplatform_services.cpp \
	src/gui/platform/qplatform_sessionmanager.cpp \
	src/gui/platform/qplatform_sharedgraphicscache.cpp \
	src/gui/platform/qplatform_surface.cpp \
	src/gui/platform/qplatform_systemtrayicon.cpp \
	src/gui/platform/qplatform_theme.cpp \
	src/gui/platform/qplatform_themefactory.cpp \
	src/gui/platform/qplatform_themeplugin.cpp \
	src/gui/platform/qplatform_window.cpp \
	\
	src/gui/platform/platformheaders/qbasicfontdatabase.cpp \
	src/gui/platform/platformheaders/qfontengine_ft.cpp

EXTRA_DIST+=

