list(APPEND PRINTERDRIVERS_WIN_PRIVATE_INCLUDES
	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/windows/qwindowsprintersupport.h
	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/windows/qwindowsprintdevice.h
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")

   add_library(CsPrinterDriverWin MODULE "")
   add_library(CopperSpice::CsPrinterDriverWin ALIAS CsPrinterDriverWin)

   set_target_properties(CsPrinterDriverWin PROPERTIES OUTPUT_NAME CsPrinterDriverWin${BUILD_ABI} PREFIX "")

   target_sources(CsPrinterDriverWin
      PRIVATE
   	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/windows/main.cpp
   	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/windows/qwindowsprintersupport.cpp
   	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/windows/qwindowsprintdevice.cpp
   )

   target_link_libraries(CsPrinterDriverWin
      PRIVATE
      CsCore
      CsGui
      winspool
      comdlg32
      gdi32
      user32
   )

   target_compile_definitions(CsPrinterDriverWin
      PRIVATE
      -DQT_PLUGIN
   )

   install(TARGETS CsPrinterDriverWin DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

