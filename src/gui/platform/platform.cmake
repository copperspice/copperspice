set(GUI_PUBLIC_INCLUDES
   ${GUI_PUBLIC_INCLUDES}
   QPlatformFontDatabase
   QPlatformIntegration
   QPlatformScreen

# none of the following at this time
#    QPlatformClipboard
#    QPlatformCursor
#    QPlatformCursorImage
#    QPlatformCursorPrivate
#    QPlatformEventLoopIntegration
#    QPlatformGLContextQPlatformIntegration
#    QPlatformIntegrationFactoryInterface
#    QPlatformIntegrationPlugin
#    QPlatformNativeInterface
#    QPlatformWindow
#    QPlatformWindowFormat
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformdatabase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformintegration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformscreen.h

# none of the following at this time
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformclipboard.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformclipboard_qpa.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformcursor.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformcursor_qpa.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformcursorimage.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformcursorprivate.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformeventloopintegration.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformeventloopintegration_qpa.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformglcontext.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformglcontext_qpa.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformintegration.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformintegrationfactoryinterface.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformintegrationplugin.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformintegrationplugin_qpa.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformnativeinterface.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformnativeinterface_qpa.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformwindow.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformwindow_qpa.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformwindowformat.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformwindowformat_qpa.h
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformscreen_p.h
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformfontdatabase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformintegration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/qplatformscreen.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(GUI_SOURCES
        ${GUI_SOURCES}

    set(EXTRA_GUI_LDFLAGS
        ${EXTRA_GUI_LDFLAGS}
        -framework AppKit
    )
endif()
