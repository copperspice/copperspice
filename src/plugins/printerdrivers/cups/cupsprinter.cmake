list(APPEND PRINTERDRIVERS_CUPS_PRIVATE_INCLUDES
	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/cups/qppdprintdevice.h
	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintersupport_p.h
	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintengine_p.h
)

if(Cups_FOUND)
   add_library(CsPrinterDriverCups MODULE "")
   add_library(CopperSpice::CsPrinterDriverCups ALIAS CsPrinterDriverCups)

   set_target_properties(CsPrinterDriverCups PROPERTIES OUTPUT_NAME CsPrinterDriverCups${BUILD_ABI} PREFIX "")

   target_sources(CsPrinterDriverCups
      PRIVATE
   	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/cups/main.cpp
   	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/cups/qppdprintdevice.cpp
   	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintengine.cpp
   	${PROJECT_SOURCE_DIR}/src/plugins/printerdrivers/cups/qcupsprintersupport.cpp
   )

   target_include_directories(CsPrinterDriverCups
      PRIVATE
      ${CUPS_INCLUDE_DIRS}
   )

   target_link_libraries(CsPrinterDriverCups
      CsCore
      CsGui
      ${CUPS_LIBRARIES}
   )

   target_compile_definitions(CsPrinterDriverCups
      PRIVATE
      -DCS_BUILDING_CUPS
      -DQT_PLUGIN
   )

   function_generate_resources(CsPrinterDriverCups)

   install(TARGETS CsPrinterDriverCups DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

