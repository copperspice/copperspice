set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}
    QFactoryInterface
    QLibrary
    QPluginLoader
    QUuid
    QtPlugin
    QtPluginInstanceFunction
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qpluginloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qtplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qtplugininstancefunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/quuid.h
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryloader_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qsystemlibrary_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qelfparser_p.h
)
set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qpluginloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qfactoryloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/quuid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qelfparser_p.cpp
)

## FIXME platform-dependent

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qsystemlibrary.cpp
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|Darwin|OpenBSD|FreeBSD|NetBSD)")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qlibrary_unix.cpp
    )
endif()

