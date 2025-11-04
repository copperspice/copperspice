if(BUILD_PLATFORMS_WAYLAND_PLUGIN)
   add_library(CsGuiWayland_Egl MODULE "")
   add_library(CopperSpice::CsGuiWayland_Egl ALIAS CsGuiWayland_Egl)

   set_target_properties(CsGuiWayland_Egl PROPERTIES
      OUTPUT_NAME CsGuiWayland_Egl${BUILD_ABI} PREFIX ""
      INSTALL_RPATH "$ORIGIN/.."
   )

   list(APPEND PLATFORMS_WAYLAND_EGL_PRIVATE_INCLUDES
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_decorations_blitter_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_egl_clientbuffer_integration.h
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_egl_config_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_egl_forward.h
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_egl_stateguard_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_egl_window.h
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_gl_context.h
   )

   target_sources(CsGuiWayland_Egl
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/main_egl.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_egl_clientbuffer_integration.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_egl_config.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_egl_window.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/egl/qwayland_gl_context.cpp
   )

   macro_generate_misc_private("${PLATFORMS_WAYLAND_EGL_PRIVATE_INCLUDES}" QtGui/private/platforms)

   target_sources(CsGuiWayland_Egl
      PRIVATE
      ${PLATFORMS_WAYLAND_EGL_PRIVATE_INCLUDES}
   )

   target_link_libraries(CsGuiWayland_Egl
      PRIVATE
      CsCore
      CsGui
      CsWaylandClient
      Wayland::EGL
      OpenGL::EGL
   )

   target_compile_definitions(CsGuiWayland_Egl
      PRIVATE
      -DQT_PLUGIN
   )

   install(TARGETS CsGuiWayland_Egl DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
