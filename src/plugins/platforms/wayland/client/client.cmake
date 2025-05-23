if(BUILD_PLATFORMS_WAYLAND_PLUGIN)
   add_library(CsWaylandClient SHARED "")
   add_library(CopperSpice::CsWaylandClient ALIAS CsWaylandClient)

   set_target_properties(CsWaylandClient PROPERTIES
      OUTPUT_NAME CsWaylandClient${BUILD_ABI})

   target_compile_definitions(CsWaylandClient
      PRIVATE
      -DQT_NO_DBUS
      -DQT_FONTCONFIGDATABASE
      -DQT_USE_FREETYPE
   )

   list(APPEND WAYLAND_CLIENT_PRIVATE_INCLUDES
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/client/qwayland_integration_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/client/qwayland_popup_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/client/qwayland_shellsurface_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/client/qwayland_toplevel_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qgenericunix_eventdispatcher_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qgenericunix_fontdatabase_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qgenericunix_services_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qgenericunix_theme_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qunix_eventdispatcher_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qfontconfig_database_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qfontengine_multifontconfig_p.h
   )

   target_sources(CsWaylandClient
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/client/qwayland_integration.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/client/qwayland_shellsurface.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/client/qwayland_toplevel.cpp

      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qgenericunix_eventdispatcher.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qgenericunix_services.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qgenericunix_theme.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qunix_eventdispatcher.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qfontconfig_database.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/unix_generic/qfontengine_multifontconfig.cpp

      # wayland xml extensions and protocol files
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/extensions/wl-hardware-integration.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/extensions/wl-qtkey-extension.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/extensions/wl-server-buffer-extension.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/extensions/wl-sub-surface-extension.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/extensions/wl-surface-extension.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/extensions/wl-touch-extension.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/extensions/wl-windowmanager.xml

      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/protocol/wl-text.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/protocol/wl-wayland.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/wayland/xml/protocol/wl-xdg-shell.xml

      # generated cs_wayland_scanner "client-code"
      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-text.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-wayland.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-xdg-shell.cpp

      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-hardware-integration.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-qtkey-extension.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-server-buffer-extension.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-sub-surface-extension.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-surface-extension.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-touch-extension.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/qwayland-windowmanager.cpp

      # generated upstream wayland scanner "public-code"
      ${CMAKE_CURRENT_BINARY_DIR}/wayland-text-protocol.c
      ${CMAKE_CURRENT_BINARY_DIR}/wayland-wayland-protocol.c
      ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-shell-protocol.c

      ${CMAKE_CURRENT_BINARY_DIR}/wayland-hardware-integration-protocol.c
      ${CMAKE_CURRENT_BINARY_DIR}/wayland-qtkey-extension-protocol.c
      ${CMAKE_CURRENT_BINARY_DIR}/wayland-server-buffer-extension-protocol.c
      ${CMAKE_CURRENT_BINARY_DIR}/wayland-sub-surface-extension-protocol.c
      ${CMAKE_CURRENT_BINARY_DIR}/wayland-surface-extension-protocol.c
      ${CMAKE_CURRENT_BINARY_DIR}/wayland-touch-extension-protocol.c
      ${CMAKE_CURRENT_BINARY_DIR}/wayland-windowmanager-protocol.c
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
      ${FONTCONFIG_LIBRARIES}
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
