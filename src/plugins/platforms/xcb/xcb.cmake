set(EXTRA_PLATFORMS_XCB_LIBS CsCore${BUILD_ABI} CsGui${BUILD_ABI})

set(PLATFORMS_XCB_PRIVATE_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbclipboard.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbconnection.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbintegration.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbkeyboard.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbdrag.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbexport.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbmime.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbobject.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbscreen.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbwindow.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbbackingstore.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbwmsupport.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbnativeinterface.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbcursor.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbimage.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbxsettings.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbsystemtraytracker.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbsessionmanager.h
)

set(PLATFORMS_XCB_OTHER_PRIVATE_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/xgl/qxcbglintegration.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/xgl/qxcbglintegrationfactory.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/xgl/qxcbglintegrationplugin.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/xgl/qxcbnativeinterfacehandler.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/events/qgenericunixeventdispatcher_p.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/events/qgenericunixservices_p.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/events/qunixeventdispatcher_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/events/qxcbeventdispatcher_glib_p.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/fonts/qfontconfigdatabase_p.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/fonts/qfontenginemultifontconfig_p.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/fonts/qgenericunixfontdatabase_p.h
	${CMAKE_CURRENT_SOURCE_DIR}/xcb/themes/qgenericunixthemes_p.h
)

set(PLATFORMS_XCB_SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbmain.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbclipboard.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbconnection.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbintegration.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbkeyboard.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbmime.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbdrag.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbscreen.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbwindow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbbackingstore.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbwmsupport.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbnativeinterface.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbcursor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbimage.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbxsettings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbsystemtraytracker.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbsessionmanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/qxcbconnection_xi2.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/xgl/qxcbglintegrationfactory.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/xgl/qxcbglintegration.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/xgl/qxcbnativeinterfacehandler.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/events/qgenericunixeventdispatcher.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/events/qgenericunixservices.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/events/qunixeventdispatcher.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/fonts/qfontconfigdatabase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/fonts/qfontenginemultifontconfig.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/xcb/themes/qgenericunixthemes.cpp
)

if(GTK2_FOUND)
   set(PLATFORMS_XCB_SOURCES
      ${PLATFORMS_XCB_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/xcb/events/qxcbeventdispatcher_glib.cpp
   )
endif()

if(BUILD_PLATFORMS_XCB_PLUGIN)

   set(EXTRA_PLATFORMS_XCB_LIBS
      CsCore${BUILD_ABI}
      CsGui${BUILD_ABI}
      ${XCB_LIB}
      ${XCB_IMAGE_LIB}
      ${XCB_ICCCM_LIB}
      ${XCB_SYNC_LIB}
      ${XCB_XFIXES_LIB}
      ${XCB_SHM_LIB}
      ${XCB_RANDR_LIB}
      ${XCB_SHAPE_LIB}
      ${XCB_KEYSYMS_LIB}
      ${XCB_XINERAMA_LIB}
      ${XCB_XKB_LIB}
      ${XCB_RENDER_LIB}
      ${XCB_RENDER_UTIL_LIB}
      ${X11_SM_LIB}
      ${X11_ICE_LIB}
      ${X11_X11_LIB}
      ${X11_Xi_LIB}
      ${X11_XCB_LIB}
      ${XKBCOMMON_LIB}
      ${XKBCOMMON_X11_LIB}
   )

   add_library(CsGuiXcb${BUILD_ABI} MODULE ${PLATFORMS_XCB_SOURCES})

   target_link_libraries(CsGuiXcb${BUILD_ABI}
      ${EXTRA_PLATFORMS_XCB_LIBS})

   target_include_directories(
      CsGuiXcb${BUILD_ABI} PRIVATE
      ${GLIB2_INCLUDES}
      ${CMAKE_SOURCE_DIR}/src/3rdparty/freetype/include
      ${CMAKE_SOURCE_DIR}/src/3rdparty/freetype/include/freetype
      ${CMAKE_SOURCE_DIR}/src/3rdparty/harbuzz/src
   )

   target_compile_definitions(CsGuiXcb${BUILD_ABI} PRIVATE
      -DQT_PLUGIN
      -DQT_USE_FONTCONFIG
      -DQT_USE_FREETYPE
      -DHAVE_POSIX_MEMALIGN
      -DQT_NO_ACCESSIBILITY_ATSPI_BRIDGE
      -DQT_NO_DBUS
      -DXCB_USE_RENDER
      -DXCB_USE_SM
      -DXCB_USE_XLIB
      -DXCB_USE_XINPUT2
   )

   if(NOT GTK2_FOUND)
      target_compile_definitions(CsGuiXcb${BUILD_ABI} PRIVATE
         -DQT_NO_GLIB
      )
   endif()

   install(TARGETS CsGuiXcb${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

