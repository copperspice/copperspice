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

#ifndef QSSLCONTEXT_OPENSSL_P_H
#define QSSLCONTEXT_OPENSSL_P_H

#include <qvariant.h>
#include <qsslcertificate.h>
#include <qsslconfiguration.h>

#include <openssl/ssl.h>

#ifdef QT_SSL

class QSslContextPrivate;

class QSslContext
{
 public:
   ~QSslContext();

   static QSslContext* fromConfiguration(QSslSocket::SslMode mode,
                  const QSslConfiguration &configuration, bool allowRootCertOnDemandLoading);

   QSslError::SslError error() const;
   QString errorString() const;

   SSL* createSsl();
   bool cacheSession(SSL*);           // should be called when handshake completed

   QByteArray sessionASN1() const;
   void setSessionASN1(const QByteArray &sessionASN1);
   int sessionTicketLifeTimeHint() const;

#if OPENSSL_VERSION_NUMBER >= 0x1000100fL && !defined(OPENSSL_NO_NEXTPROTONEG)
   // must be public because we want to use it from an OpenSSL callback

   struct NPNContext {
      NPNContext() : data(0), len(0),
         status(QSslConfiguration::NextProtocolNegotiationNone)
      {
      }

      unsigned char *data;
      unsigned short len;
      QSslConfiguration::NextProtocolNegotiationStatus status;
   };

   NPNContext npnContext() const;
#endif

 protected:
   QSslContext();

 private:
   SSL_CTX* ctx;
   EVP_PKEY *pkey;
   SSL_SESSION *session;
   QByteArray m_sessionASN1;
   int m_sessionTicketLifeTimeHint;
   QSslError::SslError errorCode;
   QString errorStr;
   QSslConfiguration sslConfiguration;

#if OPENSSL_VERSION_NUMBER >= 0x1000100fL && ! defined(OPENSSL_NO_NEXTPROTONEG)
   QByteArray m_supportedNPNVersions;
   NPNContext m_npnContext;
#endif

};

#endif

#endif // QSSLCONTEXT_OPENSSL_P_H
