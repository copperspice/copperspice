NETWORK_DIRS += src/network/socket

NETWORK_PUBLIC_INCLUDES += \
	QAbstractSocket \
	QLocalServer \
	QLocalSocket \
	QTcpServer \
	QTcpSocket \
	QUdpSocket

NETWORK_SOCKET_INCLUDES = \
	qabstractsocket.h \
	qlocalserver.h \
	qlocalsocket.h \
	qtcpserver.h \
	qtcpsocket.h \
	qudpsocket.h

NETWORK_SOCKET_PRIVATE_INCLUDES = \
	qabstractsocket_p.h \
	qabstractsocketengine_p.h \
	qtcpserver_p.h \
	qhttpsocketengine_p.h \
	qlocalserver_p.h \
	qlocalsocket_p.h \
	qnativesocketengine_p.h \
	qsocks5socketengine_p.h \
	qnet_unix_p.h \
	qtcpsocket_p.h

NETWORK_INCLUDES += $(NETWORK_SOCKET_INCLUDES)
NETWORK_PRIVATE_INCLUDES += $(NETWORK_SOCKET_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/network/socket/%,$(NETWORK_SOCKET_INCLUDES)) \
		$(patsubst %,src/network/socket/%,$(NETWORK_SOCKET_PRIVATE_INCLUDES))

lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/socket/qabstractsocketengine.cpp \
	src/network/socket/qhttpsocketengine.cpp \
	src/network/socket/qsocks5socketengine.cpp \
	src/network/socket/qabstractsocket.cpp \
	src/network/socket/qtcpsocket.cpp \
	src/network/socket/qudpsocket.cpp \
	src/network/socket/qtcpserver.cpp \
	src/network/socket/qlocalsocket.cpp \
	src/network/socket/qlocalserver.cpp \
	src/network/socket/qnativesocketengine.cpp

if OSTYPE_LINUX
lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/socket/qnativesocketengine_unix.cpp \
	src/network/socket/qlocalsocket_unix.cpp \
	src/network/socket/qlocalserver_unix.cpp
endif

if OSTYPE_BSD
lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
        src/network/socket/qnativesocketengine_unix.cpp \
        src/network/socket/qlocalsocket_unix.cpp \
        src/network/socket/qlocalserver_unix.cpp
endif

if OSTYPE_DARWIN
lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/socket/qnativesocketengine_unix.cpp \
	src/network/socket/qlocalsocket_unix.cpp \
	src/network/socket/qlocalserver_unix.cpp
endif

if OSTYPE_WIN
lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/socket/qnativesocketengine_win.cpp \
	src/network/socket/qlocalsocket_win.cpp \
	src/network/socket/qlocalserver_win.cpp
endif
