if(BUILD_PLATFORMS_WAYLAND_PLUGIN)
   add_library(CsGuiWayland_bradient MODULE "")
   add_library(CopperSpice::CsGuiWayland_bradient ALIAS CsGuiWayland_bradient)

   set_target_properties(CsGuiWayland_bradient PROPERTIES
      OUTPUT_NAME CsGuiWayland_bradient${BUILD_ABI} PREFIX ""
      INSTALL_RPATH "$ORIGIN/.."
   )

   list(APPEND PLATFORMS_WAYLAND_BRADIENT_PRIVATE_INCLUDES
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/bradient/qwayland_bradient_decoration.h
   )

   target_sources(CsGuiWayland_bradient
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/main_bradient.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/bradient/qwayland_bradient_decoration.cpp
   )

   macro_generate_misc_private("${PLATFORMS_WAYLAND_BRADIENT_PRIVATE_INCLUDES}" QtGui/private/platforms)

   target_sources(CsGuiWayland_bradient
      PRIVATE
      ${PLATFORMS_WAYLAND_BRADIENT_PRIVATE_INCLUDES}
   )

   target_link_libraries(CsGuiWayland_bradient
      PRIVATE
      CsCore
      CsGui
      CsWaylandClient
   )

   target_compile_definitions(CsGuiWayland_bradient
      PRIVATE
      -DQT_PLUGIN
   )

   install(TARGETS CsGuiWayland_bradient DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
