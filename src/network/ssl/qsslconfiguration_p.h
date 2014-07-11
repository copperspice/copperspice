/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSSLCONFIGURATION_P_H
#define QSSLCONFIGURATION_P_H

#include <qsslconfiguration.h>
#include <qlist.h>
#include <qsslcertificate.h>
#include <qsslcipher.h>
#include <qsslkey.h>

QT_BEGIN_NAMESPACE

class QSslConfigurationPrivate: public QSharedData
{
 public:
   QSslConfigurationPrivate()
      : protocol(QSsl::SecureProtocols),
        peerVerifyMode(QSslSocket::AutoVerifyPeer),
        peerVerifyDepth(0),
        allowRootCertOnDemandLoading(true),
        sslOptions(QSsl::SslOptionDisableEmptyFragments
                   | QSsl::SslOptionDisableLegacyRenegotiation
                   | QSsl::SslOptionDisableCompression) {
   }

   QSslCertificate peerCertificate;
   QList<QSslCertificate> peerCertificateChain;
   QSslCertificate localCertificate;

   QSslKey privateKey;
   QSslCipher sessionCipher;
   QList<QSslCipher> ciphers;
   QList<QSslCertificate> caCertificates;

   QSsl::SslProtocol protocol;
   QSslSocket::PeerVerifyMode peerVerifyMode;
   int peerVerifyDepth;
   bool allowRootCertOnDemandLoading;

   QSsl::SslOptions sslOptions;

   // in qsslsocket.cpp:
   static QSslConfiguration defaultConfiguration();
   static void setDefaultConfiguration(const QSslConfiguration &configuration);
   static void deepCopyDefaultConfiguration(QSslConfigurationPrivate *config);
};

// implemented here for inlining purposes
inline QSslConfiguration::QSslConfiguration(QSslConfigurationPrivate *dd)
   : d(dd)
{
}

QT_END_NAMESPACE

#endif
