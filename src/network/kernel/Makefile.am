NETWORK_DIRS += src/network/kernel

NETWORK_PUBLIC_INCLUDES += \
	QAuthenticator \
	QDnsDomainNameRecord \
	QDnsHostAddressRecord \
	QDnsMailExchangeRecord \
	QDnsServiceRecord \
	QDnsTextRecord \
	QDnsLookup \
	QHostAddress \
	QHostInfo \
	QNetworkAddressEntry \
	QNetworkProxy \
	QNetworkProxyFactory \
	QNetworkProxyQuery \
	QNetworkInterface \
	QtNetwork

NETWORK_KERNEL_INCLUDES = \
	qauthenticator.h \
	qdnsdomainnamerecord.h \
	qdnshostaddressrecord.h \
	qdnsmailexchangerecord.h \
	qdnsservicerecord.h \
	qdnstextrecord.h \
	qdnslookup.h \
	qhostaddress.h \
	qhostinfo.h \
	qnetworkaddressentry.h \
	qnetworkproxy.h \
	qnetworkproxyfactory.h \
	qnetworkproxyquery.h \
	qnetworkinterface.h \
	qtnetwork.h

NETWORK_KERNEL_PRIVATE_INCLUDES = \
	qauthenticator_p.h \
	qdnslookup_p.h \
	qhostinfo_p.h \
	qnetworkproxy_p.h \
	qhostaddress_p.h \
	qnetworkinterface_p.h \
	qurlinfo_p.h

NETWORK_INCLUDES += $(NETWORK_KERNEL_INCLUDES)
NETWORK_PRIVATE_INCLUDES += $(NETWORK_KERNEL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/network/kernel/%,$(NETWORK_KERNEL_INCLUDES)) \
		$(patsubst %,src/network/kernel/%,$(NETWORK_KERNEL_PRIVATE_INCLUDES))

lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/kernel/qauthenticator.cpp \
	src/network/kernel/qdnslookup.cpp \
	src/network/kernel/qhostaddress.cpp \
	src/network/kernel/qhostinfo.cpp \
	src/network/kernel/qurlinfo.cpp \
	src/network/kernel/qnetworkproxy.cpp \
	src/network/kernel/qnetworkinterface.cpp

if OSTYPE_LINUX
lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/kernel/qdnslookup_unix.cpp \
	src/network/kernel/qhostinfo_unix.cpp \
	src/network/kernel/qnetworkinterface_unix.cpp \
	src/network/kernel/qnetworkproxy_generic.cpp
endif

if OSTYPE_BSD
lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/kernel/qdnslookup_unix.cpp \
	src/network/kernel/qhostinfo_unix.cpp \
	src/network/kernel/qnetworkinterface_unix.cpp \
	src/network/kernel/qnetworkproxy_generic.cpp
endif

if OSTYPE_DARWIN
lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/kernel/qdnslookup_unix.cpp \
	src/network/kernel/qhostinfo_unix.cpp \
	src/network/kernel/qnetworkinterface_unix.cpp \
	src/network/kernel/qnetworkproxy_generic.cpp
endif

if OSTYPE_WIN
lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/kernel/qdnslookup_win.cpp \
	src/network/kernel/qhostinfo_win.cpp \
	src/network/kernel/qnetworkinterface_win.cpp \
	src/network/kernel/qnetworkproxy_generic.cpp
endif

