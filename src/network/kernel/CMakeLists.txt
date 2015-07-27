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
    PARENT_SCOPE
)

set(NETWORK_INCLUDES
    ${NETWORK_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/q_ipv6addr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qipv6address.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qauthenticator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qhostaddress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qhostinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qurlinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkaddressentry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkproxy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkproxyfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkproxyquery.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtnetwork.h
    PARENT_SCOPE
)

set(NETWORK_PRIVATE_INCLUDES
    ${NETWORK_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qauthenticator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qhostinfo_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkproxy_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qhostaddress_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkinterface_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkinterface_win_p.h
    PARENT_SCOPE
)

set(NETWORK_SOURCES
    ${NETWORK_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qauthenticator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qhostaddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qhostinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qurlinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkproxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkinterface.cpp 
    PARENT_SCOPE
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(NETWORK_SOURCES
        ${NETWORK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qhostinfo_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkinterface_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkproxy_generic.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(NETWORK_SOURCES
        ${NETWORK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qhostinfo_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkinterface_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkproxy_generic.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(NETWORK_SOURCES
        ${NETWORK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qhostinfo_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkinterface_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qnetworkproxy_generic.cpp
        PARENT_SCOPE
    )
endif()

