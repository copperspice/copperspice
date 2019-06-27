include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/privateinclude

   ${CMAKE_BINARY_DIR}/include/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_BINARY_DIR}/include/QtGui
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private

   ${CMAKE_CURRENT_SOURCE_DIR}/cocoa
   ${CMAKE_CURRENT_SOURCE_DIR}/windows
   ${CMAKE_CURRENT_SOURCE_DIR}/cups
)

#   include(cocoa/cocoaprinter.cmake)
include(windows/windowsprinter.cmake)
#   include(cups/cupsprinter.cmake)

macro_generate_misc_private("${PRINTERDRIVERS_COCOA_PRIVATE_INCLUDES}" QtGui/private/printerdrivers)
macro_generate_misc_private("${PRINTERDRIVERS_WIN_PRIVATE_INCLUDES}"   QtGui/private/printerdrivers)
macro_generate_misc_private("${PRINTERDRIVERS_CUPS_PRIVATE_INCLUDES}"  QtGui/private/printerdrivers)

macro_generate_misc_private("${PRINTERDRIVERS_COCOA_OTHER_PRIVATE_INCLUDES}" QtGui/private/printerdrivers)
macro_generate_misc_private("${PRINTERDRIVERS_WIN_OTHER_PRIVATE_INCLUDES}"   QtGui/private/printerdrivers)
macro_generate_misc_private("${PRINTERDRIVERS_CUPS_OTHER_PRIVATE_INCLUDES}"  QtGui/private/printerdrivers)
