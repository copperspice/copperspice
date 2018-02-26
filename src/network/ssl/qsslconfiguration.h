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

#ifndef QSSLCONFIGURATION_H
#define QSSLCONFIGURATION_H

#include <qshareddata.h>
#include <qsslsocket.h>
#include <qssl.h>

#ifdef QT_SSL

template<typename T>
class QList;

class QSslCertificate;
class QSslCipher;
class QSslKey;
class QSslEllipticCurve;

class QSslConfigurationPrivate;

class Q_NETWORK_EXPORT QSslConfiguration
{
public:
   enum NextProtocolNegotiationStatus {
      NextProtocolNegotiationNone,
      NextProtocolNegotiationNegotiated,
      NextProtocolNegotiationUnsupported
   };

   QSslConfiguration();
   QSslConfiguration(const QSslConfiguration &other);
   ~QSslConfiguration();

   QSslConfiguration &operator=(const QSslConfiguration &other);

   QSslConfiguration &operator=(QSslConfiguration &&other) {
      swap(other);
      return *this;
   }

   bool operator==(const QSslConfiguration &other) const;

   bool operator!=(const QSslConfiguration &other) const {
      return !(*this == other);
   }

   bool isNull() const;

   QSsl::SslProtocol protocol() const;
   void setProtocol(QSsl::SslProtocol protocol);

   void swap(QSslConfiguration &other) {
      qSwap(d, other.d);
   }

   // Verification
   QSslSocket::PeerVerifyMode peerVerifyMode() const;
   void setPeerVerifyMode(QSslSocket::PeerVerifyMode mode);

   int peerVerifyDepth() const;
   void setPeerVerifyDepth(int depth);

   // Certificate & cipher configuration
   QList<QSslCertificate> localCertificateChain() const;
   void setLocalCertificateChain(const QList<QSslCertificate> &localChain);

   QSslCertificate localCertificate() const;
   void setLocalCertificate(const QSslCertificate &certificate);

   QSslCertificate peerCertificate() const;
   QList<QSslCertificate> peerCertificateChain() const;
   QSslCipher sessionCipher() const;
   QSsl::SslProtocol sessionProtocol() const;

   // Private keys, for server sockets
   QSslKey privateKey() const;
   void setPrivateKey(const QSslKey &key);

   // Cipher settings
   QList<QSslCipher> ciphers() const;
   void setCiphers(const QList<QSslCipher> &ciphers);
   static QList<QSslCipher> supportedCiphers();

   // Certificate Authority (CA) settings
   QList<QSslCertificate> caCertificates() const;
   void setCaCertificates(const QList<QSslCertificate> &certificates);
   static QList<QSslCertificate> systemCaCertificates();

   void setSslOption(QSsl::SslOption option, bool on);
   bool testSslOption(QSsl::SslOption option) const;

   QByteArray sessionTicket() const;
   void setSessionTicket(const QByteArray &sessionTicket);
   int sessionTicketLifeTimeHint() const;

   // EC settings
   QVector<QSslEllipticCurve> ellipticCurves() const;
   void setEllipticCurves(const QVector<QSslEllipticCurve> &curves);
   static QVector<QSslEllipticCurve> supportedEllipticCurves();

   static QSslConfiguration defaultConfiguration();
   static void setDefaultConfiguration(const QSslConfiguration &configuration);

   void setAllowedNextProtocols(const QList<QByteArray> &protocols);

   QList<QByteArray> allowedNextProtocols() const;

   QByteArray nextNegotiatedProtocol() const;
   NextProtocolNegotiationStatus nextProtocolNegotiationStatus() const;

   static const char NextProtocolSpdy3_0[];
   static const char NextProtocolHttp1_1[];

private:
   friend class QSslSocket;
   friend class QSslConfigurationPrivate;
   friend class QSslSocketBackendPrivate;
   friend class QSslContext;

   QSslConfiguration(QSslConfigurationPrivate *dd);
   QSharedDataPointer<QSslConfigurationPrivate> d;
};

#endif

#endif
