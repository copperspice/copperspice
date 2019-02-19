
# incomplete, pending commands


set(EXTRA_PLATFORMS_COCOA_LIBS CsCore${BUILD_ABI} CsGui${BUILD_ABI})

set(PLATFORMS_INCLUDES
   ${PLATFORMS_INCLUDES}
#	${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoabackingstore.h

)

set(PLATFORMS_SOURCES
   ${PLATFORMS_SOURCES}
#	${CMAKE_CURRENT_SOURCE_DIR}/cocoa/main.mm
)

if(BUILD_PLATFORMS_COCOA_PLUGIN)
   add_library(CsGuiCocoa${BUILD_ABI} MODULE ${PLATFORM_COCOA_SOURCES})
   target_link_libraries(CsGuiCocoa${BUILD_ABI} $EXTRA_PLATFORM_COCOA_LIBS})

   target_compile_definitions(CsGuiCocoa${BUILD_ABI} PRIVATE -DIN_TRUE -DQT_PLUGIN)

   install(TARGETS CsGuiCocoa${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()