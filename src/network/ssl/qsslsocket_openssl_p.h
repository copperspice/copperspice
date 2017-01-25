/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#if OPENSSL_VERSION_NUMBER >= 0x0090806fL && ! defined(OPENSSL_NO_TLSEXT)
#include <openssl/tls1.h>
#endif

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
typedef _STACK STACK;
#endif

QT_BEGIN_NAMESPACE

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
   SSL_CTX *ctx;
   EVP_PKEY *pkey;
   BIO *readBio;
   BIO *writeBio;
   SSL_SESSION *session;
   X509_STORE *certificateStore;
   X509_STORE_CTX *certificateStoreCtx;
   QList<QPair<int, int> > errorList;

   // Platform specific functions
   void startClientEncryption();
   void startServerEncryption();
   void transmit();
   bool startHandshake();
   void disconnectFromHost();
   void disconnected();
   QSslCipher sessionCipher() const;

   static QSslCipher QSslCipher_from_SSL_CIPHER(SSL_CIPHER *cipher);
   static QList<QSslCertificate> STACKOFX509_to_QSslCertificates(STACK_OF(X509) *x509);
   static bool isMatchingHostname(const QString &cn, const QString &hostname);
   static QString getErrorsFromOpenSsl();
};

QT_END_NAMESPACE

#endif
