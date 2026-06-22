list(APPEND PLATFORMS_XCB_GLX_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qglx_context.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qxcb_glx_integration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qxcb_glx_nativeinterfacehandler.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qxcb_glx_window.h

   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qglx_convenience_p.h
)

if(BUILD_PLATFORMS_XCB_PLUGIN AND XCB_GLX_LIB)
   add_library(CsGuiXcb_Glx MODULE "")
   add_library(CopperSpice::CsGuiXcb_Glx ALIAS CsGuiXcb_Glx)

   set_target_properties(CsGuiXcb_Glx PROPERTIES
      OUTPUT_NAME CsGuiXcb_Glx${BUILD_ABI} PREFIX ""
      INSTALL_RPATH "$ORIGIN/.."
   )

   target_sources(CsGuiXcb_Glx
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qglx_convenience.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qglx_context.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qxcb_glx_integration.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qxcb_glx_nativeinterfacehandler.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qxcb_glx_window.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/xcb/glx/qxcb_glx_main.cpp
   )

   target_link_libraries(CsGuiXcb_Glx
      PRIVATE
      CsCore
      CsGui
      CsXcbSupport
      ${XCB_LIB}
      ${XCB_GLX_LIB}
      ${X11_X11_LIB}
   )

   target_include_directories(CsGuiXcb_Glx
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/plugins/platforms/xcb/glx
   )

   target_compile_definitions(CsGuiXcb_Glx
      PRIVATE
      -DQT_PLUGIN
      -DQT_NO_XRENDER
      -DXCB_HAS_XCB_GLX
      -DXCB_USE_GLX
      -DXCB_USE_XLIB
      -DXCB_USE_XINPUT2
   )

   install(TARGETS CsGuiXcb_Glx DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
