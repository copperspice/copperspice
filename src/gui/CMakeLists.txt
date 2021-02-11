add_library(CsGui SHARED "")
add_library(CopperSpice::CsGui ALIAS CsGui)

set_target_properties(CsGui PROPERTIES OUTPUT_NAME CsGui${BUILD_ABI})

target_compile_definitions(CsGui
   PRIVATE
   -DQT_BUILD_GUI_LIB
   -DQT_BUILTIN_GIF_READER
   -DMNG_BUILD_SO
   -DMNG_NO_INCLUDE_JNG
   -DPNG_NO_CONFIG_H
)

set(EXTRA_GUI_CXXFLAGS)
set(EXTRA_GUI_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude

   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_BINARY_DIR}/include/QtGui
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private

   ${CMAKE_CURRENT_SOURCE_DIR}/accessible
   ${CMAKE_CURRENT_SOURCE_DIR}/animation
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
   ${CMAKE_CURRENT_SOURCE_DIR}/effects
   ${CMAKE_CURRENT_SOURCE_DIR}/graphicsview
   ${CMAKE_CURRENT_SOURCE_DIR}/image
   ${CMAKE_CURRENT_SOURCE_DIR}/itemviews
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel
   ${CMAKE_CURRENT_SOURCE_DIR}/layout
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d
   ${CMAKE_CURRENT_SOURCE_DIR}/painting
   ${CMAKE_CURRENT_SOURCE_DIR}/platform
   ${CMAKE_CURRENT_SOURCE_DIR}/printing
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine
   ${CMAKE_CURRENT_SOURCE_DIR}/styles
   ${CMAKE_CURRENT_SOURCE_DIR}/text
   ${CMAKE_CURRENT_SOURCE_DIR}/util
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets


   ${CMAKE_SOURCE_DIR}/src/3rdparty/freetype/include
   ${CMAKE_SOURCE_DIR}/src/3rdparty/freetype/include/freetype
   ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
   ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib
   ${CMAKE_SOURCE_DIR}/src/3rdparty/libtiff/libtiff
   ${CMAKE_SOURCE_DIR}/src/3rdparty/libmng
   ${CMAKE_SOURCE_DIR}/src/3rdparty/libpng
)

if (${JPEG_FOUND})
   include_directories(
      # using system library
      ${JPEG_INCLUDE_DIR}
   )
else()
   include_directories(
      ${CMAKE_SOURCE_DIR}/src/3rdparty/libjpeg
   )
endif()

include(accessible/accessible.cmake)
include(animation/animation.cmake)
include(dialogs/dialogs.cmake)
include(effects/effects.cmake)
include(graphicsview/graphicsview.cmake)
include(image/image.cmake)
include(itemviews/itemviews.cmake)
include(kernel/kernel.cmake)
include(layout/layout.cmake)
include(math3d/math3d.cmake)
include(painting/painting.cmake)
include(opengl/opengl.cmake)
include(painting/painting.cmake)
include(platform/platform.cmake)
include(printing/printing.cmake)
include(statemachine/statemachine.cmake)
include(styles/styles.cmake)
include(text/text.cmake)
include(util/util.cmake)
include(widgetbase/widgetbase.cmake)
include(widgets/widgets.cmake)

macro_generate_public("${GUI_PUBLIC_INCLUDES}" QtGui)
macro_generate_private("${GUI_PRIVATE_INCLUDES}" QtGui)
macro_generate_misc("${GUI_INCLUDES}" QtGui)
macro_generate_misc("${GUI_PLATFORMHEADERS_INCLUDES}" QtGui/platformheaders)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")

   target_link_libraries(CsGui
      PRIVATE
      objc
   )

   list(APPEND EXTRA_GUI_LDFLAGS
      -framework Carbon
      -framework OpenGL
   )

   add_definitions(-DQT_NO_STYLE_WINDOWSVISTA)

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")

   target_link_libraries(CsGui
      PRIVATE
      ${X11_X11_LIB}
      ${X11_Xrender_LIB}
      ${X11_Xrandr_LIB}
      ${X11_Xcursor_LIB}
      ${X11_Xinerama_LIB}
      ${X11_Xext_LIB}
      ${X11_Xfixes_LIB}
      ${X11_SM_LIB}
      ${X11_ICE_LIB}
      ${CMAKE_THREAD_LIBS_INIT}
      rt
   )

   add_definitions(
      -DHAVE_POSIX_MEMALIGN
      -DQT_NO_XINPUT
      -DQT_NO_STYLE_MAC
      -DQT_NO_STYLE_WINDOWSXP
      -DQT_NO_STYLE_WINDOWSVISTA
      -DQT_NO_TABLET
   )

   include_directories(
      ${FONTCONFIG_INCLUDES}
      ${X11_X11_INCLUDE_PATH}
      ${X11_Xrender_INCLUDE_PATH}
      ${X11_Xrandr_INCLUDE_PATH}
      ${X11_Xcursor_INCLUDE_PATH}
      ${X11_Xinerama_INCLUDE_PATH}
      ${X11_Xext_INCLUDE_PATH}
      ${X11_Xfixes_INCLUDE_PATH}
      ${X11_SM_INCLUDE_PATH}
      ${X11_ICE_INCLUDE_PATH}
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")

   target_link_libraries(CsGui
      PRIVATE
      gdi32
      winmm
      ole32
      winspool
      oleaut32
      comdlg32
      imm32
      uuid
      opengl32
      kernel32
      user32
   )

   add_definitions(
      -DQT_NO_STYLE_MAC
      -DQT_NO_LPR
   )
endif()

function_variable_fixup("${EXTRA_GUI_CXXFLAGS}" EXTRA_GUI_CXXFLAGS)
function_variable_fixup("${EXTRA_GUI_LDFLAGS}"  EXTRA_GUI_LDFLAGS)

target_sources(CsGui
   PRIVATE
   ${GUI_INCLUDES}
   ${GUI_PLATFORMHEADERS_INCLUDES}
)

target_include_directories(CsGui
   PUBLIC
   $<INSTALL_INTERFACE:include/QtGui>
   ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(CsGui
   PUBLIC
   CsCore
   ${OPENGL_LIBRARIES}
)

if (${JPEG_FOUND})
   target_link_libraries(CsGui
      PRIVATE
      ${JPEG_LIBRARIES}
   )
endif()

set_target_properties(CsGui
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_GUI_CXXFLAGS}
   LINK_FLAGS ${EXTRA_GUI_LDFLAGS}
)

function_generate_resources(CsGui)

macro_generate_package(
   CsGui
   QtGui
   "${EXTRA_GUI_CXXFLAGS}"
   "CsCore"
)

install(
   TARGETS CsGui
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
