add_definitions(-DQT_BUILD_GUI_LIB
    -DQT_BUILTIN_GIF_READER
    -DMNG_BUILD_SO
    -DMNG_NO_INCLUDE_JNG
    -DPNG_NO_CONFIG_H
    -DMNG_BUILD_SO
)

set(EXTRA_GUI_LIBS CsCore${BUILD_MAJOR})
set(EXTRA_GUI_CXXFLAGS "-Wno-undefined")
set(EXTRA_GUI_LDFLAGS " ") # whitespace is required!

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

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
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

set(EXTRA_DIST
    src/gui/mac/macresources.qrc
    src/gui/mac/images/copyarrowcursor.png
    src/gui/mac/images/forbiddencursor.png
    src/gui/mac/images/leopard-unified-toolbar-on.png
    src/gui/mac/images/pluscursor.png
    src/gui/mac/images/spincursor.png
    src/gui/mac/images/waitcursor.png
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        ${FONTCONFIG_LIBRARIES}
        X11
        Xrender
        Xrandr
        Xcursor
        Xinerama
        SM
        ICE
        audio
        pthread
        rt
    )
    set(EXTRA_GUI_LDFLAGS
        ${EXTRA_GUI_LDFLAGS}
         -Wl,-luuid --export-all-symbols
    )
    add_definitions(
        -DQT_NO_XINPUT
        -DQT_NO_STYLE_MAC
        -DQT_NO_STYLE_WINDOWSXP
        -DQT_NO_STYLE_WINDOWSVISTA
        -DQT_NO_TABLET
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        gdi32
        winmm
        ole32
        winspool
        oleaut32
        comdlg32
        imm32
    )
    set(EXTRA_GUI_LDFLAGS
        ${EXTRA_GUI_LDFLAGS
         -Wl,-luuid --export-all-symbols
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
        ${CMAKE_CURRENT_SOURCE_DIR}/mac/qrc_macresources.cpp
    )
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        ObjC
    )
    set(EXTRA_GUI_LDFLAGS
        ${EXTRA_GUI_LDFLAGS
        -framework Carbon
    )
    add_definitions(-DQT_NO_STYLE_WINDOWSVISTA)

    # FIXME: what the heck?
#     macnibdir=$(prefix)/mac/qt_menu.nib
#     dist_macnib_DATA +=
#         src/gui/mac/qt_menu.nib/classes.nib
#         src/gui/mac/qt_menu.nib/info.nib
#         src/gui/mac/qt_menu.nib/keyedobjects.nib
endif()


add_library(CsGui${BUILD_MAJOR} SHARED ${GUI_SOURCES} ${GUI_INCLUDES})
target_link_libraries(CsGui${BUILD_MAJOR} ${EXTRA_GUI_LIBS})
set_target_properties(CsGui${BUILD_MAJOR} PROPERTIES
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_GUI_CXXFLAGS}
    LINK_FLAGS ${EXTRA_GUI_LDFLAGS}
)

macro_generate_resources("${GUI_SOURCES}")

install(TARGETS CsGui${BUILD_MAJOR} LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtGui DESTINATION include)