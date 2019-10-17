list(APPEND NETWORK_PUBLIC_INCLUDES
   QAbstractSocket
   QLocalServer
   QLocalSocket
   QTcpServer
   QTcpSocket
   QUdpSocket
)

list(APPEND NETWORK_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocket.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpserver.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpsocket.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qudpsocket.h
)

list(APPEND NETWORK_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocket_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocketengine_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpserver_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qhttpsocketengine_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qnativesocketengine_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qsocks5socketengine_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qnet_unix_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpsocket_p.h
)

target_sources(CsNetwork
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocketengine.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qhttpsocketengine.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qsocks5socketengine.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qabstractsocket.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpsocket.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qudpsocket.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qtcpserver.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/socket/qnativesocketengine.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   target_sources(CsNetwork
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/socket/qnativesocketengine_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver_unix.cpp
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD)")
   target_sources(CsNetwork
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/socket/qnativesocketengine_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver_unix.cpp
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_sources(CsNetwork
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/socket/qnativesocketengine_win.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalsocket_win.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/socket/qlocalserver_win.cpp
   )

endif()
