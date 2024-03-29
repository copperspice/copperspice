include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private

   ${CMAKE_BINARY_DIR}/include/QtGui/platformheaders
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private/platforms

   ${CMAKE_SOURCE_DIR}/src/3rdparty/freetype/include
   ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src

   ${CMAKE_CURRENT_SOURCE_DIR}/cocoa
   ${CMAKE_CURRENT_SOURCE_DIR}/windows
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb
)

include(cocoa/cocoa.cmake)
include(windows/windows.cmake)
include(xcb/xcb.cmake)
include(xcb/glx/xcb_glx.cmake)

include(xcb/xcb_support/xcb_support.cmake)

macro_generate_misc_private("${PLATFORMS_COCOA_PRIVATE_INCLUDES}" QtGui/private/platforms)
macro_generate_misc_private("${PLATFORMS_WIN_PRIVATE_INCLUDES}"   QtGui/private/platforms)
macro_generate_misc_private("${PLATFORMS_XCB_PRIVATE_INCLUDES}"   QtGui/private/platforms)

macro_generate_misc_private("${PLATFORMS_COCOA_OTHER_PRIVATE_INCLUDES}" QtGui/private/platforms)
macro_generate_misc_private("${PLATFORMS_WIN_OTHER_PRIVATE_INCLUDES}"   QtGui/private/platforms)
