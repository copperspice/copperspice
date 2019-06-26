set(EXTRA_PRINTERDRIVERS_WIN_LIBS CsCore${BUILD_ABI} CsGui${BUILD_ABI})

set(PRINTERDRIVERS_WIN_PRIVATE_INCLUDES
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/windows/qwindowsprintersupport.h
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/windows/qwindowsprintdevice.h
)

set(PRINTERDRIVERS_WIN_SOURCES
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/windows/main.cpp
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/windows/qwindowsprintersupport.cpp
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/windows/qwindowsprintdevice.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

   set(EXTRA_PRINTERDRIVERS_WIN_LIBS
      CsCore${BUILD_ABI}
      CsGui${BUILD_ABI}
      winspool
      comdlg32
      gdi32
      user32
   )

   add_library(CsPrinterDriverWin${BUILD_ABI} MODULE ${PRINTERDRIVERS_WIN_SOURCES})

   target_include_directories(
      CsPrinterDriverWin{BUILD_ABI} PRIVATE
   )

   target_compile_definitions(CsPrinterDriverWin${BUILD_ABI} PRIVATE
      -DQT_PLUGIN
   )

   macro_generate_resources("${PRINTERDRIVERS_WIN_SOURCES}")

   install(TARGETS CsPrinterDriverWin${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

