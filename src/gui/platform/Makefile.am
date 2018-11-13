GUI_DIRS += src/gui/platform

GUI_PUBLIC_INCLUDES += \
	QPlatform_Accessibility \
	QPlatform_BackingStore \
	QPlatform_Clipboard \
	QPlatform_Cursor \
	QPlatform_Drag \
	QPlatform_FontDatabase \
	QPlatform_GraphicsBuffer \
	QPlatform_Integration \
	QPlatform_InputContext \
	QPlatform_Menu \
	QPlatform_NativeInterface \
	QPlatform_OpenGLContext \
	QPlatform_Pixmap \
	QPlatform_Screen \
	QPlatform_Services \
	QPlatform_Surface \
	QPlatform_SystemTrayIcon \
	QPlatform_Theme \
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
	qplatform_inputcontext.h \
	qplatform_integration.h \
	qplatform_menu.h \
	qplatform_nativeinterface.h \
	qplatform_offscreensurface.h \
	qplatform_openglcontext.h \
	qplatform_pixmap.h \
	qplatform_screen.h \
	qplatform_services.h \
	qplatform_sessionmanager.h \
	qplatform_sharedgraphicscache.h \
	qplatform_surface.h \
	qplatform_systemtrayicon.h \
	qplatform_theme.h \
	qplatform_window.h

GUI_PLATFORM_PRIVATE_INCLUDES = \
	qplatform_inputcontext_p.h \
   qplatform_screen_p.h \
   qplatform_theme_p.h \
	qplatform_themefactory_p.h \
   qplatform_window_p.h

GUI_INCLUDES += $(GUI_PLATFORM_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_PLATFORM_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/platform/%,$(GUI_PLATFORM_INCLUDES)) \
	$(patsubst %,src/gui/platform/%,$(GUI_PLATFORM_PRIVATE_INCLUDES))

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
	src/gui/platform/qplatform_integration.cpp \
	src/gui/platform/qplatform_menu.cpp \
	src/gui/platform/qplatform_nativeinterface.cpp \
	src/gui/platform/qplatform_offscreensurface.cpp \
	src/gui/platform/qplatform_openglcontext.cpp \
	src/gui/platform/qplatform_pixmap.cpp \
	src/gui/platform/qplatform_screen.cpp \
	src/gui/platform/qplatform_services.cpp \
	src/gui/platform/qplatform_sessionmanager.cpp \
	src/gui/platform/qplatform_sharedgraphicscache.cpp \
	src/gui/platform/qplatform_surface.cpp \
	src/gui/platform/qplatform_systemtrayicon.cpp \
	src/gui/platform/qplatform_theme.cpp \
	src/gui/platform/qplatform_window.cpp

EXTRA_DIST+=


