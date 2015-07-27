set(NETWORK_PUBLIC_INCLUDES
    ${NETWORK_PUBLIC_INCLUDES}
    QAbstractSocket
    QLocalServer
    QLocalSocket
    QTcpServer
    QTcpSocket
    QUdpSocket
    PARENT_SCOPE
)

set(NETWORK_INCLUDES
    ${NETWORK_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractsocket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qlocalserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qlocalsocket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtcpserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtcpsocket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qudpsocket.h
    PARENT_SCOPE
)

set(NETWORK_PRIVATE_INCLUDES
    ${NETWORK_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractsocket_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractsocketengine_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qhttpsocketengine_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qlocalserver_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qlocalsocket_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnativesocketengine_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsocks5socketengine_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnet_unix_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtcpsocket_p.h
    PARENT_SCOPE
)

set(NETWORK_SOURCES
    ${NETWORK_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractsocketengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qhttpsocketengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsocks5socketengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtcpsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qudpsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtcpserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qlocalsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qlocalserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qnativesocketengine.cpp
    PARENT_SCOPE
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(NETWORK_SOURCES
        ${NETWORK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qnativesocketengine_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qlocalsocket_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qlocalserver_unix.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(NETWORK_SOURCES
        ${NETWORK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qnativesocketengine_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qlocalsocket_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qlocalserver_unix.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(NETWORK_SOURCES
        ${NETWORK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qnativesocketengine_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qlocalsocket_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qlocalserver_win.cpp
        PARENT_SCOPE
    )
endif()
