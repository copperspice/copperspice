if(BUILD_PLATFORMS_WAYLAND_PLUGIN)
   add_library(CsGuiWayland MODULE "")
   add_library(CopperSpice::CsGuiWayland ALIAS CsGuiWayland)

   set_target_properties(CsGuiWayland PROPERTIES
      OUTPUT_NAME CsGuiWayland${BUILD_ABI} PREFIX ""
      INSTALL_RPATH "$ORIGIN/.."
   )

   list(APPEND PLATFORMS_WAYLAND_PRIVATE_INCLUDES
   )

   target_sources(CsGuiWayland
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/main_generic.cpp
   )

   target_link_libraries(CsGuiWayland
      PRIVATE
      CsCore
      CsGui
      CsWaylandClient
   )

   target_compile_definitions(CsGuiWayland
      PRIVATE
      -DQT_PLUGIN
   )

   install(TARGETS CsGuiWayland DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
