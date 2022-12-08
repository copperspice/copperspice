include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_BINARY_DIR}/include/QtGui
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private

   ${CMAKE_BINARY_DIR}/include/QtSvg
   ${CMAKE_BINARY_DIR}/privateinclude/QtSvg/private

   ${CMAKE_CURRENT_SOURCE_DIR}/svg
)

include(svg/svg.cmake)

macro_generate_misc_private("${IMAGEFORMATS_SVG_PRIVATE_INCLUDES}"  QtSvg/private/imageformats)

