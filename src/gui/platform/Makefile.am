GUI_DIRS += src/gui/platform

GUI_PUBLIC_INCLUDES += \
	QPlatformFontDatabase \
	QPlatformIntegration \
	QPlatformScreen

#	QPlatformClipboard
#	QPlatformCursor
#	QPlatformCursorImage
#	QPlatformCursorPrivate
#	QPlatformEventLoopIntegration
#	QPlatformGLContext

#	QPlatformIntegrationFactoryInterface
#	QPlatformIntegrationPlugin
#	QPlatformNativeInterface
#	QPlatformWindow
#	QPlatformWindowForm

GUI_PLATFORM_INCLUDES = \
   qplatformfontdatabase.h \
   qplatformintegration.h \
   qplatformscreen.h

#	qplatformclipboard.h
#	qplatformclipboard_qpa.h
#	qplatformcursor.h
#	qplatformcursor_qpa.h
#	qplatformcursorimage.h
#	qplatformcursorprivate.h
#	qplatformeventloopintegration.h
#	qplatformeventloopintegration_qpa.h
#	qplatformglcontext.h
#	qplatformglcontext_qpa.h
#	qplatformintegrationfactoryinterface.h
#	qplatformintegrationplugin.h
#	qplatformintegrationplugin_qpa.h
#	qplatformnativeinterface.h
#	qplatformnativeinterface_qpa.h
#	qplatformwindow.h
#	qplatformwindow_qpa.h
#	qplatformwindowformat.h
#	qplatformwindowformat_qpa


GUI_PLATFORM_PRIVATE_INCLUDES = \
   qplatformscreen_p.h


GUI_INCLUDES += $(GUI_PLATFORM_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_PLATFORM_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/platform/%,$(GUI_PLATFORM_INCLUDES)) \
	$(patsubst %,src/gui/platform/%,$(GUI_PLATFORM_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
   src/gui/platform/qplatformfontdatabase.cpp \
   src/gui/platform/qplatformintegration.cpp \
   src/gui/platform/qplatformscreen.cpp


EXTRA_DIST+=


