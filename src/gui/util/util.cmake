set(GUI_PUBLIC_INCLUDES
    ${GUI_PUBLIC_INCLUDES}
    QCompleter
    QDesktopServices
    QScroller
    QScrollerProperties
    QSystemTrayIcon
    QUndoCommand
    QUndoGroup
    QUndoStack
    QUndoView
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qcompleter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qdesktopservices.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qscroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qscrollerproperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qsystemtrayicon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qundocommand.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qundogroup.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qundostack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qundoview.h
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qabstractlayoutstyleinfo_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qcompleter_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qflickgesture_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qscroller_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qscrollerproperties_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qlayoutpolicy_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qsystemtrayicon_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qundostack_p.h
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qabstractlayoutstyleinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qcompleter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qdesktopservices.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qflickgesture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qlayoutpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qscroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qscrollerproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qsystemtrayicon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qundogroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qundostack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/util/qundoview.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   set(GUI_SOURCES
       ${GUI_SOURCES}
       ${CMAKE_CURRENT_SOURCE_DIR}/util/qsystemtrayicon_win.cpp
   )

elseif(X11_FOUND)
   set(GUI_SOURCES
       ${GUI_SOURCES}
       ${CMAKE_CURRENT_SOURCE_DIR}/util/qsystemtrayicon_x11.cpp
   )

else()
   set(GUI_SOURCES
       ${GUI_SOURCES}
       ${CMAKE_CURRENT_SOURCE_DIR}/util/qsystemtrayicon_qpa.cpp
   )

endif()
