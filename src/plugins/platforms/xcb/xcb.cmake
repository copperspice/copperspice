list(APPEND PLATFORMS_XCB_PRIVATE_INCLUDES
)

if(BUILD_PLATFORMS_XCB_PLUGIN)
   add_library(CsGuiXcb MODULE "")
   add_library(CopperSpice::CsGuiXcb ALIAS CsGuiXcb)

   set_target_properties(CsGuiXcb PROPERTIES
      OUTPUT_NAME   CsGuiXcb${BUILD_ABI} PREFIX ""
      INSTALL_RPATH "$ORIGIN/.."
   )

   target_sources(CsGuiXcb
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcb_main.cpp
   )

   target_link_libraries(CsGuiXcb
      PRIVATE
      CsCore
      CsGui
      CsXcbSupport
   )

   target_compile_definitions(CsGuiXcb
      PRIVATE
      -DQT_PLUGIN
      -DXCB_USE_XINPUT2
   )

   if(BUILDING_RPM)
      install(TARGETS CsGuiXcb DESTINATION ${CMAKE_INSTALL_LIBDIR}/copperspice/plugins/platform)
   else()
      install(TARGETS CsGuiXcb DESTINATION ${CMAKE_INSTALL_LIBDIR})
   endif()
endif()

