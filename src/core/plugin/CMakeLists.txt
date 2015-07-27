set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}
    QFactoryInterface
    QLibrary
    QPluginLoader
    QUuid
    QtPlugin
    QtPluginInstanceFunction
    PARENT_SCOPE
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qfactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qlibrary.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qpluginloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtplugininstancefunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quuid.h
    PARENT_SCOPE
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qlibrary_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfactoryloader_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsystemlibrary_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qelfparser_p.h
    PARENT_SCOPE
)
set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qpluginloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfactoryloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/quuid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qlibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qelfparser_p.cpp
    PARENT_SCOPE
)

## FIXME platform-dependent

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qlibrary_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qsystemlibrary.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qlibrary_unix.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qlibrary_unix.cpp
        PARENT_SCOPE
    )
endif()
