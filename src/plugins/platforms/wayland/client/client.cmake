if(BUILD_PLATFORMS_WAYLAND_PLUGIN)
   add_library(CsWaylandClient SHARED "")
   add_library(CopperSpice::CsWaylandClient ALIAS CsWaylandClient)

   set_target_properties(CsWaylandClient PROPERTIES
      OUTPUT_NAME CsWaylandClient${BUILD_ABI})

   target_compile_definitions(CsWaylandClient
      PRIVATE
      -DQT_NO_DBUS
   )

   list(APPEND WAYLAND_CLIENT_PRIVATE_INCLUDES
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/client/qwayland_integration_p.h
   )

   target_sources(CsWaylandClient
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/client/qwayland_integration.cpp
   )

   macro_generate_misc_private("${WAYLAND_CLIENT_PRIVATE_INCLUDES}" QtGui/private/platforms)

   target_sources(CsWaylandClient
      PRIVATE
      ${WAYLAND_CLIENT_PRIVATE_INCLUDES}
   )

   target_link_libraries(CsWaylandClient
      PRIVATE
      CsCore
      CsGui
      Wayland::Client
   )

   if(GTK2_FOUND)
      target_link_libraries(CsWaylandClient
         PRIVATE
         ${GLIB2_LIBRARIES}
      )

   else()
      target_compile_definitions(CsWaylandClient
         PRIVATE
         -DQT_NO_GLIB
      )

   endif()

   function_generate_resources(CsWaylandClient)

   install(
      TARGETS CsWaylandClient
      EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
   )
endif()
