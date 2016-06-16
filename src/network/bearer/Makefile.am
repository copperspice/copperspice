NETWORK_DIRS += src/network/bearer

NETWORK_PUBLIC_INCLUDES += \
	QNetworkConfiguration \
	QNetworkSession \
	QNetworkConfigurationManager

NETWORK_BEARER_INCLUDES = \
	qnetworkconfiguration.h \
	qnetworksession.h \
	qnetworkconfigurationmanager.h \
	qnetworkconfigmanager.h

NETWORK_BEARER_PRIVATE_INCLUDES = \
	qnetworkconfigmanager_p.h \
	qnetworkconfiguration_p.h \
	qnetworksession_p.h \
	qbearerengine_p.h \
	qbearerplugin_p.h \
	qsharednetworksession_p.h

NETWORK_INCLUDES += $(NETWORK_BEARER_INCLUDES)
NETWORK_PRIVATE_INCLUDES += $(NETWORK_BEARER_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/network/bearer/%,$(NETWORK_BEARER_INCLUDES)) \
		$(patsubst %,src/network/bearer/%,$(NETWORK_BEARER_PRIVATE_INCLUDES))

lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/bearer/qnetworksession.cpp \
	src/network/bearer/qnetworkconfigmanager.cpp \
	src/network/bearer/qnetworkconfiguration.cpp \
	src/network/bearer/qnetworkconfigmanager_p.cpp \
	src/network/bearer/qbearerengine.cpp \
	src/network/bearer/qbearerplugin.cpp \
	src/network/bearer/qsharednetworksession.cpp
