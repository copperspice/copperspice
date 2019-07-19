set(PRINTERDRIVERS_COCOA_PRIVATE_INCLUDES)

set(PRINTERDRIVERS_COCOA_OTHER_PRIVATE_INCLUDES)

set(PRINTERDRIVERS_COCOA_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/main.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   set(EXTRA_PRINTERDRIVERS_COCOA_LIBS
      CsCore${BUILD_ABI}
      CsGui${BUILD_ABI}
   )

   set(EXTRA_PRINTERDRIVERS_COCOA_LDFLAGS
      -framework Cocoa
   )

   add_library(CsPrinterDriverCocoa${BUILD_ABI} MODULE ${PRINTERDRIVERS_COCOA_SOURCES})

   target_link_libraries(CsPrinterDriverCocoa${BUILD_ABI}
      ${EXTRA_PRINTERDRIVERS_COCOA_LIBS}
   )

   target_include_directories(
      CsPrinterDriverCocoa${BUILD_ABI} PRIVATE
   )

   target_compile_definitions(CsPrinterDriverCocoa${BUILD_ABI} PRIVATE
      -DQT_PLUGIN
   )

   macro_generate_resources("${PRINTERDRIVERS_COCOA_SOURCES}")

   install(TARGETS CsPrinterDriverCocoa${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
