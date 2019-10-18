if(CMAKE_SYSTEM_NAME MATCHES "Darwin")

   add_library(CsPrinterDriverCocoa MODULE "")
   add_library(CopperSpice::CsPrinterDriverCocoa ALIAS CsPrinterDriverCocoa)

   set_target_properties(CsPrinterDriverCocoa PROPERTIES OUTPUT_NAME CsPrinterDriverCocoa${BUILD_ABI})

   target_sources(CsPrinterDriverCocoa
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/main.cpp
   )

   list(APPEND EXTRA_PRINTERDRIVERS_COCOA_LDFLAGS
      -framework Cocoa
   )

   target_link_libraries(CsPrinterDriverCocoa
      PRIVATE
      CsCore
      CsGui
   )

   target_compile_definitions(CsPrinterDriverCocoa
      PRIVATE
      -DQT_PLUGIN
   )

   install(TARGETS CsPrinterDriverCocoa DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
