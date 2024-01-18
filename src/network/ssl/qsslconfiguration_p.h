/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QSSLCONFIGURATION_P_H
#define QSSLCONFIGURATION_P_H

#include <qsslconfiguration.h>
#include <qlist.h>
#include <qsslcertificate.h>
#include <qsslcipher.h>
#include <qsslkey.h>
#include <qsslellipticcurve.h>

class QSslConfigurationPrivate : public QSharedData
{
public:
   QSslConfigurationPrivate()
      : sessionProtocol(QSsl::UnknownProtocol), protocol(QSsl::SecureProtocols),
        peerVerifyMode(QSslSocket::AutoVerifyPeer), peerVerifyDepth(0),
        allowRootCertOnDemandLoading(true), peerSessionShared(false),
        sslOptions(QSslConfigurationPrivate::defaultSslOptions),
        sslSessionTicketLifeTimeHint(-1),
        nextProtocolNegotiationStatus(QSslConfiguration::NextProtocolNegotiationNone)
   {
   }

   QSslCertificate peerCertificate;
   QList<QSslCertificate> peerCertificateChain;

   QList<QSslCertificate> localCertificateChain;

   QSslKey privateKey;
   QSslCipher sessionCipher;
   QSsl::SslProtocol sessionProtocol;
   QList<QSslCipher> ciphers;
   QList<QSslCertificate> caCertificates;

   QSsl::SslProtocol protocol;
   QSslSocket::PeerVerifyMode peerVerifyMode;
   int peerVerifyDepth;
   bool allowRootCertOnDemandLoading;
   bool peerSessionShared;

   static bool peerSessionWasShared(const QSslConfiguration &configuration);

   QSsl::SslOptions sslOptions;
   static const QSsl::SslOptions defaultSslOptions;

   QVector<QSslEllipticCurve> ellipticCurves;

   QByteArray sslSession;
   int sslSessionTicketLifeTimeHint;

   QList<QByteArray> nextAllowedProtocols;
   QByteArray nextNegotiatedProtocol;
   QSslConfiguration::NextProtocolNegotiationStatus nextProtocolNegotiationStatus;

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

#endif
