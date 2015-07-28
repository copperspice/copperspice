set(NETWORK_PUBLIC_INCLUDES
    ${NETWORK_PUBLIC_INCLUDES}
    Q_IPV6ADDR
    QIPv6Address
    QAuthenticator
    QHostAddress
    QHostInfo
    QUrlInfo
    QNetworkAddressEntry
    QNetworkProxy
    QNetworkProxyFactory
    QNetworkProxyQuery
    QNetworkInterface
    QtNetwork
)

set(NETWORK_INCLUDES
    ${NETWORK_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/q_ipv6addr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qipv6address.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qauthenticator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qurlinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkaddressentry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxyfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxyquery.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qtnetwork.h
)

set(NETWORK_PRIVATE_INCLUDES
    ${NETWORK_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qauthenticator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_win_p.h
    PARENT_SCOPE
)

set(NETWORK_SOURCES
    ${NETWORK_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qauthenticator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qurlinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface.cpp 
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(NETWORK_SOURCES
        ${NETWORK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy_generic.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(NETWORK_SOURCES
        ${NETWORK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy_generic.cpp
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(NETWORK_SOURCES
        ${NETWORK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy_generic.cpp
    )
endif()

