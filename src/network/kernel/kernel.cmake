list(APPEND NETWORK_PUBLIC_INCLUDES
    QAuthenticator
    QDnsDomainNameRecord
    QDnsHostAddressRecord
    QDnsMailExchangeRecord
    QDnsServiceRecord
    QDnsTextRecord
    QDnsLookup
    QHostAddress
    QHostInfo
    QNetworkAddressEntry
    QNetworkProxy
    QNetworkProxyFactory
    QNetworkProxyQuery
    QNetworkInterface
    QtNetwork
)

list(APPEND NETWORK_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qauthenticator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnsdomainnamerecord.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnshostaddressrecord.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnsmailexchangerecord.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnsservicerecord.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnstextrecord.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnslookup.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkaddressentry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxyfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxyquery.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qtnetwork.h
)

list(APPEND NETWORK_PRIVATE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qauthenticator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnslookup_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qurlinfo_p.h
)

target_sources(CsNetwork
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qauthenticator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnslookup.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostaddress.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qurlinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   target_sources(CsNetwork
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnslookup_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy_mac.cpp
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD)")
   target_sources(CsNetwork
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnslookup_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy_generic.cpp
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_sources(CsNetwork
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qdnslookup_win.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qhostinfo_win.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkinterface_win.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/kernel/qnetworkproxy_win.cpp
    )

endif()
