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
    PARENT_SCOPE
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible2interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleactioninterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleapplication.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblebridge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblebridgefactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblebridgeplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleeditabletextinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblefactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleimageinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleinterfaceex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleobjectex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblesimpleeditabletextinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibletable2cellinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibletable2interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibletableinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibletextinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblevalueinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblewidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblewidgetex.h
    PARENT_SCOPE
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible_mac_p.h
    PARENT_SCOPE
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qaccessibleplugin.cpp
    PARENT_SCOPE
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible_win.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qaccessiblebridge.cpp
        PARENT_SCOPE
    )
endif()

# FIXME: COCOCA?
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/qaccessible_mac_cocoa.mm
        PARENT_SCOPE
    )
endif()

