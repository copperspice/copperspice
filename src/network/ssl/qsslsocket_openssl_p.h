/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSSLSOCKET_OPENSSL_P_H
#define QSSLSOCKET_OPENSSL_P_H

#include <qsslsocket_p.h>

#ifdef Q_OS_WIN

#include <qt_windows.h>

#if defined(OCSP_RESPONSE)
#undef OCSP_RESPONSE
#endif
#if defined(X509_NAME)
#undef X509_NAME
#endif
#endif

#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/pkcs7.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/stack.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/dsa.h>
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/tls1.h>

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
typedef _STACK STACK;
#endif

class QSslSocketBackendPrivate : public QSslSocketPrivate
{
   Q_DECLARE_PUBLIC(QSslSocket)

public:
   QSslSocketBackendPrivate();
   virtual ~QSslSocketBackendPrivate();

   // SSL context
   bool initSslContext();
   void destroySslContext();
   SSL *ssl;

   BIO *readBio;
   BIO *writeBio;
   SSL_SESSION *session;

   QList<QPair<int, int> > errorList;

#if OPENSSL_VERSION_NUMBER >= 0x10001000L
    static int s_indexForSSLExtraData; // index used in SSL_get_ex_data to get the matching QSslSocketBackendPrivate
#endif

   // Platform specific functions
   void startClientEncryption() override;
   void startServerEncryption() override;
   void transmit() override;
   bool startHandshake();

   void disconnectFromHost() override;
   void disconnected() override;

   QSslCipher sessionCipher() const override;
   QSsl::SslProtocol sessionProtocol() const override;
   void continueHandshake() override;
   bool checkSslErrors();
   void storePeerCertificates();
   unsigned int tlsPskClientCallback(const char *hint, char *identity, unsigned int max_identity_len,
                  unsigned char *psk, unsigned int max_psk_len);

#ifdef Q_OS_WIN
    void fetchCaRootForCert(const QSslCertificate &cert);
    void _q_caRootLoaded(QSslCertificate,QSslCertificate) override;
#endif

   static long setupOpenSslOptions(QSsl::SslProtocol protocol, QSsl::SslOptions sslOptions);
   static QSslCipher QSslCipher_from_SSL_CIPHER(SSL_CIPHER *cipher);
   static QList<QSslCertificate> STACKOFX509_to_QSslCertificates(STACK_OF(X509) *x509);
   static QList<QSslError> verify(const QList<QSslCertificate> &certificateChain, const QString &hostName);
   static QString getErrorsFromOpenSsl();
   static bool importPkcs12(QIODevice *device, QSslKey *key, QSslCertificate *cert,
                  QList<QSslCertificate> *caCertificates, const QByteArray &passPhrase);
};

#ifdef Q_OS_WIN
class QWindowsCaRootFetcher : public QObject
{
    NET_CS_OBJECT(QWindowsCaRootFetcher);

 public:
   QWindowsCaRootFetcher(const QSslCertificate &certificate, QSslSocket::SslMode sslMode);
   ~QWindowsCaRootFetcher();

   NET_CS_SIGNAL_1(Public, void finished(QSslCertificate brokenChain, QSslCertificate caroot))
   NET_CS_SIGNAL_2(finished, brokenChain, caroot)

   NET_CS_SLOT_1(Public, void start())
   NET_CS_SLOT_2(start)

 private:
    QSslCertificate cert;
    QSslSocket::SslMode mode;
};
#endif


#endif
