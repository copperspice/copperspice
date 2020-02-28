list(APPEND NETWORK_PUBLIC_INCLUDES
    QSsl
    QSslCertificate
    QSslCertificate_Extension
    QSslConfiguration
    QSslCipher
    QSslEllipticCurve
    QSslError
    QSslKey
    QSslPreSharedKeyAuthenticator
    QSslSocket
)

list(APPEND NETWORK_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qssl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate_extension.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslconfiguration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcipher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslellipticcurve.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslerror.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslkey.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslpresharedkeyauthenticator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket.h
)

list(APPEND NETWORK_PRIVATE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qssl_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate_extension_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslconfiguration_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcontext_openssl_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcipher_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslkey_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_openssl_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_openssl_symbols_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslpresharedkeyauthenticator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_mac_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qasn1element_p.h
)

if(OPENSSL_FOUND OR SECURETRANSPORT_FOUND)
   target_sources(CsNetwork
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qasn1element.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qssl.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslconfiguration.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcipher.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslellipticcurve.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslkey.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslerror.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslpresharedkeyauthenticator.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate_extension.cpp
   )
endif()

if(OPENSSL_FOUND AND NOT SECURETRANSPORT_FOUND)
   target_sources(CsNetwork
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate_openssl.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcontext_openssl.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslellipticcurve_openssl.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslkey_openssl.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_openssl.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_openssl_symbols.cpp
   )
endif()

if(SECURETRANSPORT_FOUND)
   target_sources(CsNetwork
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslkey_mac.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslsocket_mac.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslcertificate_etc.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslkey_etc.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/ssl/qsslellipticcurve_etc.cpp
    )
endif()
