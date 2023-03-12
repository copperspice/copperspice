add_executable(linguist${TOOLS_SUFFIX} "")
add_executable(CopperSpice::linguist ALIAS linguist${TOOLS_SUFFIX})

set(EXTRA_LINGUIST_LIBS
   CsCore
   CsGui
   CsXml
)

set(EXTRA_LINGUIST_CXXFLAGS)
set(EXTRA_LINGUIST_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private
   ${CMAKE_BINARY_DIR}/privateinclude/QtXml/private

   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_SOURCE_DIR}/src/tools/shared
)

target_sources(linguist${TOOLS_SUFFIX}
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/batchtranslation_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/errorsview.h
   ${CMAKE_CURRENT_SOURCE_DIR}/find_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/globals.h
   ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
   ${CMAKE_CURRENT_SOURCE_DIR}/messageeditor.h
   ${CMAKE_CURRENT_SOURCE_DIR}/messageeditorwidgets.h
   ${CMAKE_CURRENT_SOURCE_DIR}/messagehighlighter.h
   ${CMAKE_CURRENT_SOURCE_DIR}/messagemodel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/phrasebookbox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/phrase.h
   ${CMAKE_CURRENT_SOURCE_DIR}/phrasemodel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/phraseview.h
   ${CMAKE_CURRENT_SOURCE_DIR}/printout.h
   ${CMAKE_CURRENT_SOURCE_DIR}/recentfiles.h
   ${CMAKE_CURRENT_SOURCE_DIR}/settings_dialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/sourcecodeview.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statistics.h
   ${CMAKE_CURRENT_SOURCE_DIR}/translate_dialog.h

   ${CMAKE_SOURCE_DIR}/src/tools/shared/similartext.h

#  ${CMAKE_CURRENT_SOURCE_DIR}/formpreviewview.h
#  ${CMAKE_CURRENT_SOURCE_DIR}/formpreviewview.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/batchtranslation_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/errorsview.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/find_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/globals.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/messageeditor.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/messageeditorwidgets.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/messagehighlighter.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/messagemodel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/phrasebookbox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/phrase.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/phrasemodel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/phraseview.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/printout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/recentfiles.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/settings_dialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/sourcecodeview.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statistics.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/translate_dialog.cpp

   ${CMAKE_SOURCE_DIR}/src/tools/shared/similartext.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/batchtranslation.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/find_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/phrasebookbox.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/settings_dialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/statistics.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/translate_dialog.ui

   ${CMAKE_CURRENT_SOURCE_DIR}/linguist.qrc
   ${CMAKE_CURRENT_BINARY_DIR}/qrc_linguist.cpp

   $<TARGET_OBJECTS:sharedtools>
)

if(CMAKE_SYSTEM_NAME MATCHES "DARWIN")
   list(APPEND EXTRA_LINGUIST_LDFLAGS '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
   list(APPEND EXTRA_LINGUIST_LDFLAGS '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   list(APPEND EXTRA_LINGUIST_LDFLAGS '-Wl,-rpath,$ORIGIN/../lib')
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   list(APPEND EXTRA_LINGUIST_LIBS
      uuid
   )
endif()

function_variable_fixup("${EXTRA_LINGUIST_CXXFLAGS}" EXTRA_LINGUIST_CXXFLAGS)
function_variable_fixup("${EXTRA_LINGUIST_LDFLAGS}"  EXTRA_LINGUIST_LDFLAGS)

target_link_libraries(linguist${TOOLS_SUFFIX}
   PRIVATE
   ${EXTRA_LINGUIST_LIBS}
)

set_target_properties(linguist${TOOLS_SUFFIX}
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_LINGUIST_CXXFLAGS}
   LINK_FLAGS ${EXTRA_LINGUIST_LDFLAGS}
   EXPORT_NAME linguist
)

function_generate_resources(linguist${TOOLS_SUFFIX})

#
#   If the files in bin didn't name collide with Qt binaries we could just leave them.
#
if(BUILDING_RPM OR BUILDING_DEBIAN)
    install(
       TARGETS linguist${TOOLS_SUFFIX}
       EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
       DESTINATION ${CMAKE_INSTALL_LIBDIR}/copperspice/bin
    )
else()
    install(
       TARGETS linguist${TOOLS_SUFFIX}
       EXPORT CopperSpiceBinaryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
       DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

#   mac {
#    static:CONFIG -= global_init_link_order
#    ICON = linguist.icns
#    TARGET = Linguist
#    INFO_PLIST=Info_mac.plist
#   }

#   phrasebooks.files = $$PWD/../phrasebooks/*
#   INSTALLS += phrasebooks



