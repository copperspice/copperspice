NETWORK_DIRS += src/network/kernel

NETWORK_PUBLIC_INCLUDES += \
	Q_IPV6ADDR \
	QIPv6Address \
	QAuthenticator \
	QHostAddress \
	QHostInfo \
	QUrlInfo \
	QNetworkAddressEntry \
	QNetworkProxy \
	QNetworkProxyFactory \
	QNetworkProxyQuery \
	QNetworkInterface \
	QtNetwork

NETWORK_KERNEL_INCLUDES = \
	q_ipv6addr.h \
	qipv6address.h \
	qauthenticator.h \
	qhostaddress.h \
	qhostinfo.h \
	qurlinfo.h \
	qnetworkaddressentry.h \
	qnetworkproxy.h \
	qnetworkproxyfactory.h \
	qnetworkproxyquery.h \
	qnetworkinterface.h \
	qtnetwork.h

NETWORK_KERNEL_PRIVATE_INCLUDES = \
	qauthenticator_p.h \
	qhostinfo_p.h \
	qnetworkproxy_p.h \
	qhostaddress_p.h \
	qnetworkinterface_p.h \
	qnetworkinterface_win_p.h

NETWORK_INCLUDES += $(NETWORK_KERNEL_INCLUDES)
NETWORK_PRIVATE_INCLUDES += $(NETWORK_KERNEL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/network/kernel/%,$(NETWORK_KERNEL_INCLUDES)) \
		$(patsubst %,src/network/kernel/%,$(NETWORK_KERNEL_PRIVATE_INCLUDES))

lib_libCsNetwork1_la_SOURCES += \
	src/network/kernel/qauthenticator.cpp \
	src/network/kernel/qhostaddress.cpp \
	src/network/kernel/qhostinfo.cpp \
	src/network/kernel/qurlinfo.cpp \
	src/network/kernel/qnetworkproxy.cpp \
	src/network/kernel/qnetworkinterface.cpp 

if OSTYPE_LINUX
lib_libCsNetwork1_la_SOURCES += \
	src/network/kernel/qhostinfo_unix.cpp \
	src/network/kernel/qnetworkinterface_unix.cpp \
	src/network/kernel/qnetworkproxy_generic.cpp
endif

if OSTYPE_DARWIN
lib_libCsNetwork1_la_SOURCES += \
	src/network/kernel/qhostinfo_unix.cpp \
	src/network/kernel/qnetworkinterface_unix.cpp \
	src/network/kernel/qnetworkproxy_generic.cpp
endif

if OSTYPE_WIN
lib_libCsNetwork1_la_SOURCES += \
	src/network/kernel/qhostinfo_win.cpp \
	src/network/kernel/qnetworkinterface_win.cpp \
	src/network/kernel/qnetworkproxy_generic.cpp
endif

