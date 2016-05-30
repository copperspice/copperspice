add_definitions(-DQT_BUILD_GUI_LIB
    -DQT_BUILTIN_GIF_READER
    -DMNG_BUILD_SO
    -DMNG_NO_INCLUDE_JNG
    -DPNG_NO_CONFIG_H
    -DMNG_BUILD_SO
)
set(EXTRA_GUI_LIBS CsCore${BUILD_ABI})
set(EXTRA_GUI_CXXFLAGS)
set(EXTRA_GUI_LDFLAGS)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible
    ${CMAKE_CURRENT_SOURCE_DIR}/animation
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/effects
    ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview
    ${CMAKE_CURRENT_SOURCE_DIR}/image
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod
    ${CMAKE_CURRENT_SOURCE_DIR}/itemviews
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/math3d
    ${CMAKE_CURRENT_SOURCE_DIR}/painting
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine
    ${CMAKE_CURRENT_SOURCE_DIR}/styles
    ${CMAKE_CURRENT_SOURCE_DIR}/text
    ${CMAKE_CURRENT_SOURCE_DIR}/util
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/wintab
    ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libtiff/libtiff
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libmng
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libjpeg
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libpng
)

include(accessible/accessible.cmake)
include(animation/animation.cmake)
include(dialogs/dialogs.cmake)
include(effects/effects.cmake)
# include(embedded/embedded.cmake) # currently unused
include(graphicsview/graphicsview.cmake)
include(image/image.cmake)
include(inputmethod/inputmethod.cmake)
include(itemviews/itemviews.cmake)
include(kernel/kernel.cmake)
include(math3d/math3d.cmake)
include(painting/painting.cmake)
include(statemachine/statemachine.cmake)
include(styles/styles.cmake)
include(text/text.cmake)
include(util/util.cmake)
include(widgets/widgets.cmake)

macro_generate_public("${GUI_PUBLIC_INCLUDES}" QtGui)
macro_generate_private("${GUI_PRIVATE_INCLUDES}" QtGui)
macro_generate_misc("${GUI_INCLUDES}" QtGui)

if(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD)")
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        ${FONTCONFIG_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        # ${X11_LIBRARIES} should contain all of them but it does not sadly
        ${X11_X11_LIB}
        ${X11_Xrender_LIB}
        ${X11_Xrandr_LIB}
        ${X11_Xcursor_LIB}
        ${X11_Xinerama_LIB}
        ${X11_Xext_LIB}
        ${X11_Xfixes_LIB}
        ${X11_SM_LIB}
        ${X11_ICE_LIB}
        ${NAS_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        rt
    )
    add_definitions(
        -DQT_NO_XINPUT
        -DQT_NO_STYLE_MAC
        -DQT_NO_STYLE_WINDOWSXP
        -DQT_NO_STYLE_WINDOWSVISTA
        -DQT_NO_TABLET
    )
    include_directories(
        ${FONTCONFIG_INCLUDES}
        ${FREETYPE_INCLUDE_DIRS}
        # ${X11_INCLUDE_DIR} should contain all of them but it does not sadly
        ${X11_X11_INCLUDE_PATH}
        ${X11_Xrender_INCLUDE_PATH}
        ${X11_Xrandr_INCLUDE_PATH}
        ${X11_Xcursor_INCLUDE_PATH}
        ${X11_Xinerama_INCLUDE_PATH}
        ${X11_Xext_INCLUDE_PATH}
        ${X11_Xfixes_INCLUDE_PATH}
        ${X11_SM_INCLUDE_PATH}
        ${X11_ICE_INCLUDE_PATH}
        ${NAS_INCLUDES}
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        gdi32
        winmm
        ole32
        winspool
        oleaut32
        comdlg32
        imm32
        uuid
    )
    set(EXTRA_GUI_LDFLAGS
        ${EXTRA_GUI_LDFLAGS}
         -Wl,--export-all-symbols
    )
    add_definitions(
        -DQT_NO_STYLE_MAC
        -DQT_NO_LPR
        -DQT_NO_DIRECTWRITE
        -DQT_MAKEDLL
   )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/mac/macresources.qrc
    )
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        objc
    )
    set(EXTRA_GUI_LDFLAGS
        ${EXTRA_GUI_LDFLAGS}
        -framework Carbon
    )
    add_definitions(-DQT_NO_STYLE_WINDOWSVISTA)
endif()

function_variable_fixup("${EXTRA_GUI_CXXFLAGS}" EXTRA_GUI_CXXFLAGS)
function_variable_fixup("${EXTRA_GUI_LDFLAGS}" EXTRA_GUI_LDFLAGS)

add_library(CsGui${BUILD_ABI} SHARED ${GUI_SOURCES} ${GUI_INCLUDES})
target_link_libraries(CsGui${BUILD_ABI} ${EXTRA_GUI_LIBS})
set_target_properties(CsGui${BUILD_ABI} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_GUI_CXXFLAGS}
    LINK_FLAGS ${EXTRA_GUI_LDFLAGS}
    # FIXME: need to put those in resource prefix $(prefix)/mac/qt_menu.nib
    RESOURCE
    ${CMAKE_SOURCE_DIR}/src/gui/mac/qt_menu.nib/classes.nib
    ${CMAKE_SOURCE_DIR}/src/gui/mac/qt_menu.nib/info.nib
    ${CMAKE_SOURCE_DIR}/src/gui/mac/qt_menu.nib/keyedobjects.nib
)

macro_generate_resources("${GUI_SOURCES}")
macro_generate_package(
    CsGui${BUILD_ABI}
    QtGui
    "${EXTRA_GUI_CXXFLAGS}"
    "${EXTRA_GUI_LIBS}"
    "CsCore${BUILD_ABI}"
)

install(
    TARGETS CsGui${BUILD_ABI}
    EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${CMAKE_BINARY_DIR}/include/QtGui
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
