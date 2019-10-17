list(APPEND NETWORK_PUBLIC_INCLUDES
   QNetworkConfiguration
   QNetworkSession
   QNetworkConfigurationManager
)

list(APPEND NETWORK_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworkconfiguration.h
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworksession.h
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworkconfigurationmanager.h
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworkconfigmanager.h
)

list(APPEND NETWORK_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworkconfigmanager_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworkconfiguration_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworksession_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qbearerengine_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qbearerplugin_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qsharednetworksession_p.h
)

target_sources(CsNetwork
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworksession.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworkconfigmanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworkconfiguration.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qnetworkconfigmanager_p.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qbearerengine.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qbearerplugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/bearer/qsharednetworksession.cpp
)
