include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private

   ${CMAKE_BINARY_DIR}/include/QtGui/platformheaders
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private/platforms

   ${CMAKE_SOURCE_DIR}/src/3rdparty/freetype/include
   ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
)


if(BUILD_PLATFORMS_COCOA_PLUGIN)
   include_directories(
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa
   )

   include(cocoa/cocoa.cmake)

   macro_generate_misc_private("${PLATFORMS_COCOA_PRIVATE_INCLUDES}"          QtGui/private/platforms)
   macro_generate_misc_private("${PLATFORMS_COCOA_OTHER_PRIVATE_INCLUDES}"    QtGui/private/platforms)
endif()


if(BUILD_PLATFORMS_WINDOWS_PLUGIN)
   include_directories(
      ${CMAKE_CURRENT_SOURCE_DIR}/windows
   )

   include(windows/windows.cmake)

   macro_generate_misc_private("${PLATFORMS_WIN_PRIVATE_INCLUDES}"            QtGui/private/platforms)
   macro_generate_misc_private("${PLATFORMS_WIN_OTHER_PRIVATE_INCLUDES}"      QtGui/private/platforms)
endif()


if(BUILD_PLATFORMS_WAYLAND_PLUGIN)
   include_directories(
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland
      ${CMAKE_CURRENT_BINARY_DIR}
   )

   include(wayland/wayland_generic.cmake)
   add_subdirectory(wayland/scanner)

   include(wayland/client/client.cmake)

   macro_generate_misc_private("${PLATFORMS_WAYLAND_PRIVATE_INCLUDES}"     QtGui/private/platforms)
endif()


if(BUILD_PLATFORMS_XCB_PLUGIN)
   include_directories(
      ${CMAKE_CURRENT_SOURCE_DIR}/xcb
   )

   include(xcb/xcb.cmake)
   include(xcb/glx/xcb_glx.cmake)
   include(xcb/xcb_support/xcb_support.cmake)

   macro_generate_misc_private("${PLATFORMS_XCB_PRIVATE_INCLUDES}"         QtGui/private/platforms)
endif()

