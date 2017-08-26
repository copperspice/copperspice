NETWORK_DIRS += src/network/ssl

NETWORK_PUBLIC_INCLUDES += \
	QSsl \
	QSslCertificate \
	QSslConfiguration \
	QSslCipher \
	QSslEllipticCurve \
	QSslError \
	QSslKey \
	QSslSocket

NETWORK_SSL_INCLUDES = \
	qssl.h \
	qsslcertificate.h \
	qsslcertificateextension.h \
	qsslconfiguration.h \
	qsslcipher.h \
	qsslellipticcurve.h \
	qsslerror.h \
	qsslkey.h \
	qsslpresharedkeyauthenticator.h \
	qsslsocket.h

NETWORK_SSL_PRIVATE_INCLUDES = \
	qssl_p.h \
	qsslcertificate_p.h \
	qsslcertificateextension_p.h \
	qsslconfiguration_p.h \
	qsslcontext_openssl_p.h \
	qsslcipher_p.h \
	qsslkey_p.h \
	qsslsocket_openssl_p.h \
	qsslsocket_openssl_symbols_p.h \
	qsslpresharedkeyauthenticator_p.h \
	qsslsocket_p.h

NETWORK_INCLUDES += $(NETWORK_SSL_INCLUDES)
NETWORK_PRIVATE_INCLUDES += $(NETWORK_SSL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/network/ssl/%,$(NETWORK_SSL_INCLUDES)) \
		$(patsubst %,src/network/ssl/%,$(NETWORK_SSL_PRIVATE_INCLUDES))

if SSL
lib_libCsNetwork@BUILD_ABI@_la_SOURCES += \
	src/network/ssl/qssl.cpp \
	src/network/ssl/qsslcertificate.cpp \
	src/network/ssl/qsslcertificate_openssl.cpp \
	src/network/ssl/qsslcertificateextension.cpp \
	src/network/ssl/qsslconfiguration.cpp \
	src/network/ssl/qsslcontext_openssl.cpp \
	src/network/ssl/qsslcipher.cpp \
	src/network/ssl/qsslellipticcurve.cpp \
	src/network/ssl/qsslellipticcurve_openssl.cpp \
	src/network/ssl/qsslerror.cpp \
	src/network/ssl/qsslkey.cpp \
	src/network/ssl/qsslkey_openssl.cpp \
	src/network/ssl/qsslsocket.cpp \
	src/network/ssl/qsslsocket_openssl.cpp \
	src/network/ssl/qsslsocket_openssl_symbols.cpp  \
	src/network/ssl/qsslpresharedkeyauthenticator.cpp
endif