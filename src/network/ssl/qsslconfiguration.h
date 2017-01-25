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

#ifndef QSSLCONFIGURATION_H
#define QSSLCONFIGURATION_H

#include <QtCore/qshareddata.h>
#include <QtNetwork/qsslsocket.h>
#include <QtNetwork/qssl.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_OPENSSL

template<typename T> class QList;
class QSslCertificate;
class QSslCipher;
class QSslKey;
class QSslConfigurationPrivate;

class Q_NETWORK_EXPORT QSslConfiguration
{

 public:
   QSslConfiguration();
   QSslConfiguration(const QSslConfiguration &other);
   ~QSslConfiguration();
   QSslConfiguration &operator=(const QSslConfiguration &other);

   bool operator==(const QSslConfiguration &other) const;
   inline bool operator!=(const QSslConfiguration &other) const {
      return !(*this == other);
   }

   bool isNull() const; // ### Qt5/remove; who would need this?

   QSsl::SslProtocol protocol() const;
   void setProtocol(QSsl::SslProtocol protocol);

   // Verification
   QSslSocket::PeerVerifyMode peerVerifyMode() const;
   void setPeerVerifyMode(QSslSocket::PeerVerifyMode mode);

   int peerVerifyDepth() const;
   void setPeerVerifyDepth(int depth);

   // Certificate & cipher configuration
   QSslCertificate localCertificate() const;
   void setLocalCertificate(const QSslCertificate &certificate);

   QSslCertificate peerCertificate() const;
   QList<QSslCertificate> peerCertificateChain() const;
   QSslCipher sessionCipher() const;

   // Private keys, for server sockets
   QSslKey privateKey() const;
   void setPrivateKey(const QSslKey &key);

   // Cipher settings
   QList<QSslCipher> ciphers() const;
   void setCiphers(const QList<QSslCipher> &ciphers);

   // Certificate Authority (CA) settings
   QList<QSslCertificate> caCertificates() const;
   void setCaCertificates(const QList<QSslCertificate> &certificates);

   void setSslOption(QSsl::SslOption option, bool on);
   bool testSslOption(QSsl::SslOption option) const;

   static QSslConfiguration defaultConfiguration();
   static void setDefaultConfiguration(const QSslConfiguration &configuration);

 private:
   friend class QSslSocket;
   friend class QSslConfigurationPrivate;
   QSslConfiguration(QSslConfigurationPrivate *dd);
   QSharedDataPointer<QSslConfigurationPrivate> d;
};

#else

class Q_NETWORK_EXPORT QSslConfiguration
{
};

#endif  // QT_NO_OPENSSL

QT_END_NAMESPACE

#endif
