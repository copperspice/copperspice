if(BUILD_PLATFORMS_COCOA_PLUGIN)

   list(APPEND PLATFORMS_COCOA_PRIVATE_INCLUDES
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/cglconvenience_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/messages.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qmacclipboard.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qmacdefines_mac.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qmultitouch_mac_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qnsview.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qnswindowdelegate.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qpaintengine_mac_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qprintengine_mac_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qt_mac_p.h

      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaaccessibilityelement.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaaccessibility.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaapplicationdelegate.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaapplication.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoabackingstore.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoacolordialoghelper.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaclipboard.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoacursor.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoadrag.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaeventdispatcher.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoafiledialoghelper.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoafontdialoghelper.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaglcontext.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoahelpers.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoainputcontext.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaintegration.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaintrospection.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoakeymapper.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamenu.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamenuitem.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamenubar.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamenuloader.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamimetypes.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoanativeinterface.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaprintersupport.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaprintdevice.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaservices.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoasystemtrayicon.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoasystemsettings.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoatheme.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoawindow.h
   )

   list(APPEND PLATFORMS_COCOA_OTHER_PRIVATE_INCLUDES
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/accessible/qaccessiblebridgeutils_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/clipboard/qmacmime_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/fonts/qcoretextfontdatabase_p.h
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/fonts/qfontengine_coretext_p.h
   )

   add_library(CsGuiCocoa MODULE "")
   add_library(CopperSpice::CsGuiCocoa ALIAS CsGuiCocoa )

   set_target_properties(CsGuiCocoa PROPERTIES OUTPUT_NAME CsGuiCocoa${BUILD_ABI} PREFIX "")

   target_sources(CsGuiCocoa
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/cglconvenience.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/messages.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/main.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qmacclipboard.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qmultitouch_mac.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qnsview.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qnsviewaccessibility.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qnswindowdelegate.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qpaintengine_mac.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qprintengine_mac.mm

      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaaccessibilityelement.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaaccessibility.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaapplication.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaapplicationdelegate.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoabackingstore.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoacolordialoghelper.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoacursor.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaclipboard.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoadrag.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaeventdispatcher.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoafiledialoghelper.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoafontdialoghelper.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaglcontext.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoahelpers.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoainputcontext.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaintegration.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaintrospection.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoakeymapper.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamenu.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamenuitem.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamenubar.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamenuloader.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoamimetypes.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoanativeinterface.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaprintersupport.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaprintdevice.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaservices.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoasystemsettings.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoasystemtrayicon.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoatheme.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoawindow.mm

      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/accessible/qaccessiblebridgeutils.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/clipboard/qmacmime.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/fonts/qcoretextfontdatabase.mm
      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/fonts/qfontengine_coretext.mm

      ${CMAKE_CURRENT_SOURCE_DIR}/cocoa/qcocoaresources.qrc
      ${CMAKE_CURRENT_BINARY_DIR}/qrc_qcocoaresources.cpp
   )

   list(APPEND EXTRA_PLATFORMS_COCOA_LDFLAGS
      -framework Cocoa
      -framework Carbon
      -framework IOKit
      -framework ApplicationServices
      -framework AppKit
      -framework OpenGL
      -framework AGL
   )

   function_variable_fixup("${EXTRA_PLATFORMS_COCOA_CXXFLAGS}" EXTRA_PLATFORMS_COCOA_CXXFLAGS)
   function_variable_fixup("${EXTRA_PLATFORMS_COCOA_LDFLAGS}"  EXTRA_PLATFORMS_COCOA_LDFLAGS)

   target_link_libraries(CsGuiCocoa
      PRIVATE
      CsCore
      CsGui
      -lcups
   )

   target_include_directories(CsGuiCocoa
      PRIVATE
      ${CMAKE_SOURCE_DIR}/src/3rdparty/freetype/include
      ${CMAKE_SOURCE_DIR}/src/3rdparty/freetype/include/freetype
      ${CMAKE_SOURCE_DIR}/src/3rdparty/harbuzz/src
   )

   target_compile_definitions(CsGuiCocoa
      PRIVATE
      -DQT_PLUGIN
      -DQT_USE_FREETYPE
   )

   target_compile_options(CsGuiCocoa
      PRIVATE
      $<$<COMPILE_LANGUAGE:CXX>:SHELL:${EXTRA_PLATFORMS_COCOA_CXXFLAGS}>
   )

   set_target_properties(CsGuiCocoa
       PROPERTIES
       LINK_FLAGS ${EXTRA_PLATFORMS_COCOA_LDFLAGS}
   )

   function_generate_resources(CsGuiCocoa)

   install(TARGETS CsGuiCocoa DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
