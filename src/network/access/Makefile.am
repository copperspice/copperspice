NETWORK_DIRS += src/network/access

NETWORK_PUBLIC_INCLUDES += \
	QAbstractNetworkCache \
	QFtp \
	QHttp \
	QHttpHeader \
	QHttpMultiPart \
	QHttpPart \
	QHttpRequestHeader \
	QHttpResponseHeader \
	QNetworkAccessManager \
	QNetworkCacheMetaData \
	QNetworkCookie \
	QNetworkCookieJar \
	QNetworkDiskCache \
	QNetworkReply \
	QNetworkRequest

NETWORK_ACCESS_INCLUDES = \
	qabstractnetworkcache.h \
	qftp.h \
	qhttp.h \
	qhttpheader.h \
	qhttpmultipart.h \
	qhttppart.h \
	qhttprequestheader.h \
	qhttpresponseheader.h \
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
	qhttpnetworkheader_p.h \
	qhttpnetworkrequest_p.h \
	qhttpnetworkreply_p.h \
	qhttpnetworkconnection_p.h \
	qhttpnetworkconnectionchannel_p.h \
	qnetworkaccessauthenticationmanager_p.h \
	qnetworkaccessmanager_p.h \
	qnetworkaccesscache_p.h \
	qnetworkaccessbackend_p.h \
	qnetworkaccesshttpbackend_p.h \
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
	qhttpmultipart_p.h

NETWORK_INCLUDES += $(NETWORK_ACCESS_INCLUDES)
NETWORK_PRIVATE_INCLUDES += $(NETWORK_ACCESS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/network/access/%,$(NETWORK_ACCESS_INCLUDES)) \
		$(patsubst %,src/network/access/%,$(NETWORK_ACCESS_PRIVATE_INCLUDES))

lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/access/qabstractnetworkcache.cpp \
	src/network/access/qabstractprotocolhandler.cpp \
	src/network/access/qftp.cpp \
	src/network/access/qhttp.cpp \
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
	src/network/access/qnetworkaccesshttpbackend.cpp \
	src/network/access/qnetworkcookie.cpp \
	src/network/access/qnetworkcookiejar.cpp \
	src/network/access/qnetworkrequest.cpp \
	src/network/access/qnetworkreply.cpp \
	src/network/access/qnetworkreplyimpl.cpp \
	src/network/access/qnetworkreplydataimpl.cpp \
	src/network/access/qnetworkreplyfileimpl.cpp \
	src/network/access/qnetworkdiskcache.cpp \
	src/network/access/qhttpthreaddelegate.cpp \
	src/network/access/qhttpmultipart.cpp
