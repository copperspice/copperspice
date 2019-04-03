set(EXTRA_PLATFORM_WIN_LIBS CsCore${BUILD_ABI} CsGui${BUILD_ABI})

set(PLATFORMS_WIN_PRIVATE_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/windows/array.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsbackingstore.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsfontenginedirectwrite.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsgdiintegration.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsgdinativeinterface.h	
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowswindow.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsintegration.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowscontext.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsscreen.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowskeymapper.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsfontengine.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsfontdatabase.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsmousehandler.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qtwindowsglobal.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qtwindows_additional.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsole.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsmime.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsinternalmimedata.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowscursor.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsinputcontext.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowstheme.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsdialoghelpers.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsservices.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsnativeimage.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsnativeinterface.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsglcontext.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsopenglcontext.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsopengltester.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsthreadpoolrunner.h	
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsclipboard.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsdrag.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsfontdatabase_ft.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowssessionmanager.h
)

set(PLATFORMS_WIN_OTHER_PRIVATE_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/windows/accessible/comutils.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/accessible/qwindowsaccessibility.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/accessible/qwindowsmsaaaccessible.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/events/qwindowsguieventdispatcher_p.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/fonts/qbasicfontdatabase_p.h
	${CMAKE_CURRENT_SOURCE_DIR}/windows/fonts/qfontengine_ft_p.h
)

set(PLATFORMS_WIN_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/windows/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsbackingstore.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsgdiintegration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsgdinativeinterface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowswindow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsintegration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowscontext.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsscreen.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowskeymapper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsfontengine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsfontdatabase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsmousehandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsole.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsmime.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsinternalmimedata.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowscursor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsinputcontext.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowstheme.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsdialoghelpers.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsservices.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsnativeimage.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsnativeinterface.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsglcontext.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsopengltester.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsclipboard.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsdrag.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowsfontdatabase_ft.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/qwindowssessionmanager.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/windows/accessible/comutils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/accessible/qwindowsaccessibility.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/accessible/qwindowsmsaaaccessible.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/events/qwindowsguieventdispatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/fonts/qbasicfontdatabase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/windows/fonts/qfontengine_ft.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/windows/cursors.qrc
)

if(BUILD_PLATFORMS_WINDOWS_PLUGIN)

   set(EXTRA_PLATFORMS_WIN_LIBS
      ${WIN_LIB}
      opengl32
      gdi32
      winmm
      ole32
      winspool
      oleaut32
      comdlg32
      imm32
      uuid
      user32
   )

   add_library(CsGuiWin${BUILD_ABI} MODULE ${PLATFORMS_WIN_SOURCES})
   target_link_libraries(CsGuiWin${BUILD_ABI} $EXTRA_PLATFORMS_WIN_LIBS})

   target_compile_definitions(CsGuiWin${BUILD_ABI} PRIVATE
      -DIN_TRUE
      -DQT_PLUGIN
      -DQT_NO_FONTCONFIG
      -DQT_NO_DIRECTWRITE )

   install(TARGETS CsGuiWin${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

