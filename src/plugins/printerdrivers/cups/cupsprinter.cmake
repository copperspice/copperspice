list(APPEND PRINTERDRIVERS_CUPS_PRIVATE_INCLUDES
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qppdprintdevice.h
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintersupport_p.h
	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintengine_p.h
)

if(Cups_FOUND)
   add_library(CsPrinterDriverCups MODULE "")
   add_library(CopperSpice::CsPrinterDriverCups ALIAS CsPrinterDriverCups)

   set_target_properties(CsPrinterDriverCups PROPERTIES OUTPUT_NAME CsPrinterDriverCups${BUILD_ABI} PREFIX "")

   target_sources(CsPrinterDriverCups
      PRIVATE
   	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/main.cpp
   	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qppdprintdevice.cpp
   	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintengine.cpp
   	${CMAKE_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintersupport.cpp
   )

   target_link_libraries(CsPrinterDriverCups
      CsCore
      CsGui
      cups
   )

   target_compile_definitions(CsPrinterDriverCups
      PRIVATE
      -DCS_BUILDING_CUPS
      -DQT_PLUGIN
   )

   function_generate_resources(CsPrinterDriverCups)

   install(TARGETS CsPrinterDriverCups DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

