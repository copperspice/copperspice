set(GUI_PUBLIC_INCLUDES
    ${GUI_PUBLIC_INCLUDES}
    QAccessible
    QAccessible2Interface
    QAccessibleActionInterface
    QAccessibleApplication
    QAccessibleBridge
    QAccessibleBridgeFactoryInterface
    QAccessibleBridgePlugin
    QAccessibleEditableTextInterface
    QAccessibleEvent
    QAccessibleFactoryInterface
    QAccessibleImageInterface
    QAccessibleInterface
    QAccessibleInterfaceEx
    QAccessibleObject
    QAccessibleObjectEx
    QAccessiblePlugin
    QAccessibleSimpleEditableTextInterface
    QAccessibleTable2CellInterface
    QAccessibleTable2Interface
    QAccessibleTableInterface
    QAccessibleTextInterface
    QAccessibleValueInterface
    QAccessibleWidget
    QAccessibleWidgetEx
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible2interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleactioninterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleapplication.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblebridge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblebridgefactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblebridgeplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleeditabletextinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblefactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleimageinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleinterfaceex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleobjectex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblesimpleeditabletextinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibletable2cellinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibletable2interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibletableinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibletextinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblevalueinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblewidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblewidgetex.h
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible_mac_p.h
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessibleplugin.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible_win.cpp
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD)")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessiblebridge.cpp
    )
endif()

# FIXME: COCOCA?
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/accessible/qaccessible_mac_cocoa.mm
    )
endif()

