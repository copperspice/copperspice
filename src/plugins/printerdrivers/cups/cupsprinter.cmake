set(PRINTERDRIVERS_CUPS_PRIVATE_INCLUDES
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qppdprintdevice.h
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintersupport_p.h
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintengine_p.h
)

set(PRINTERDRIVERS_CUPS_SOURCES
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/main.cpp
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qppdprintdevice.cpp
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintengine.cpp
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintersupport.cpp
)

if(Cups_FOUND)

   set(EXTRA_PRINTERDRIVERS_CUPS_LIBS
      CsCore${BUILD_ABI}
      CsGui${BUILD_ABI}
      cups
   )

   add_library(CsPrinterDriverCups${BUILD_ABI} MODULE ${PRINTERDRIVERS_CUPS_SOURCES})

   target_link_libraries(CsPrinterDriverCups${BUILD_ABI}
      ${EXTRA_PRINTERDRIVERS_CUPS_LIBS}
   )

   target_include_directories(
      CsPrinterDriverCups${BUILD_ABI} PRIVATE
   )

   target_compile_definitions(CsPrinterDriverCups${BUILD_ABI} PRIVATE
      -DCS_BUILDING_CUPS
      -DQT_PLUGIN
   )

   macro_generate_resources("${PRINTERDRIVERS_CUPS_SOURCES}")

   set_target_properties(CsPrinterDriverCups${BUILD_ABI} PROPERTIES PREFIX "")

   install(TARGETS CsPrinterDriverCups${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

