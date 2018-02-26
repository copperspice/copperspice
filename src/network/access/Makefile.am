NETWORK_DIRS += src/network/access

NETWORK_PUBLIC_INCLUDES += \
	QAbstractNetworkCache \
	QHttpMultiPart \
	QHttpPart \
	QNetworkAccessManager \
	QNetworkCacheMetaData \
	QNetworkCookie \
	QNetworkCookieJar \
	QNetworkDiskCache \
	QNetworkReply \
	QNetworkRequest

NETWORK_ACCESS_INCLUDES = \
	qabstractnetworkcache.h \
	qhttpmultipart.h \
	qhttppart.h \
	qnetworkaccessmanager.h \
	qnetworkcachemetadata.h \
	qnetworkcookie.h \
	qnetworkcookiejar.h \
	qnetworkdiskcache.h \
	qnetworkreply.h \
	qnetworkrequest.h

NETWORK_ACCESS_PRIVATE_INCLUDES = \
	qabstractnetworkcache_p.h \
	qabstractprotocolhandler_p.h \
	qftp_p.h \
	qhttpheader_p.h \
	qhttpnetworkheader_p.h \
	qhttpnetworkrequest_p.h \
	qhttpnetworkreply_p.h \
	qhttpnetworkconnection_p.h \
	qhttpnetworkconnectionchannel_p.h \
	qnetworkaccessauthenticationmanager_p.h \
	qnetworkaccessmanager_p.h \
	qnetworkaccesscache_p.h \
	qnetworkaccessbackend_p.h \
	qnetworkaccessfilebackend_p.h \
	qnetworkaccesscachebackend_p.h \
	qnetworkaccessftpbackend_p.h \
	qnetworkcookie_p.h \
	qnetworkcookiejar_p.h \
	qnetworkrequest_p.h \
	qnetworkreply_p.h \
	qnetworkreplyimpl_p.h \
	qnetworkreplydataimpl_p.h \
	qnetworkreplyfileimpl_p.h \
	qnetworkdiskcache_p.h \
	qhttpthreaddelegate_p.h \
	qhttpmultipart_p.h \
	qnetworkreplyhttpimpl_p.h \
	qhttpprotocolhandler_p.h \
	qspdyprotocolhandler_p.h


NETWORK_INCLUDES += $(NETWORK_ACCESS_INCLUDES)
NETWORK_PRIVATE_INCLUDES += $(NETWORK_ACCESS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/network/access/%,$(NETWORK_ACCESS_INCLUDES)) \
		$(patsubst %,src/network/access/%,$(NETWORK_ACCESS_PRIVATE_INCLUDES))

lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/access/qabstractnetworkcache.cpp \
	src/network/access/qabstractprotocolhandler.cpp \
	src/network/access/qftp.cpp \
	src/network/access/qhttpheader.cpp \
	src/network/access/qhttpnetworkheader.cpp \
	src/network/access/qhttpnetworkrequest.cpp \
	src/network/access/qhttpnetworkreply.cpp \
	src/network/access/qhttpnetworkconnection.cpp \
	src/network/access/qhttpnetworkconnectionchannel.cpp \
	src/network/access/qnetworkaccessauthenticationmanager.cpp \
	src/network/access/qnetworkaccessmanager.cpp \
	src/network/access/qnetworkaccesscache.cpp \
	src/network/access/qnetworkaccessbackend.cpp \
	src/network/access/qnetworkaccessfilebackend.cpp \
	src/network/access/qnetworkaccesscachebackend.cpp \
	src/network/access/qnetworkaccessftpbackend.cpp \
	src/network/access/qnetworkcookie.cpp \
	src/network/access/qnetworkcookiejar.cpp \
	src/network/access/qnetworkrequest.cpp \
	src/network/access/qnetworkreply.cpp \
	src/network/access/qnetworkreplyimpl.cpp \
	src/network/access/qnetworkreplydataimpl.cpp \
	src/network/access/qnetworkreplyfileimpl.cpp \
	src/network/access/qnetworkdiskcache.cpp \
	src/network/access/qhttpthreaddelegate.cpp \
	src/network/access/qhttpmultipart.cpp \
	src/network/access/qnetworkreplyhttpimpl.cpp \
	src/network/access/qhttpprotocolhandler.cpp \
	src/network/access/qspdyprotocolhandler.cpp
