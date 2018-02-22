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

#include <qsslconfiguration.h>
#include <qsslconfiguration_p.h>
#include <qsslsocket.h>
#include <qsslsocket_p.h>
#include <qmutex.h>
#include <qdebug.h>

const QSsl::SslOptions QSslConfigurationPrivate::defaultSslOptions = QSsl::SslOptionDisableEmptyFragments
      | QSsl::SslOptionDisableLegacyRenegotiation
      | QSsl::SslOptionDisableCompression
      | QSsl::SslOptionDisableSessionPersistence;

const char QSslConfiguration::NextProtocolSpdy3_0[] = "spdy/3";
const char QSslConfiguration::NextProtocolHttp1_1[] = "http/1.1";

QSslConfiguration::QSslConfiguration()
   : d(new QSslConfigurationPrivate)
{
}

QSslConfiguration::QSslConfiguration(const QSslConfiguration &other)
   : d(other.d)
{
}

QSslConfiguration::~QSslConfiguration()
{
   // QSharedDataPointer deletes d for us if necessary
}

QSslConfiguration &QSslConfiguration::operator=(const QSslConfiguration &other)
{
   d = other.d;
   return *this;
}

bool QSslConfiguration::operator==(const QSslConfiguration &other) const
{
   if (d == other.d) {
      return true;
   }

   return d->peerCertificate == other.d->peerCertificate &&
          d->peerCertificateChain == other.d->peerCertificateChain &&
          d->localCertificateChain == other.d->localCertificateChain &&
          d->privateKey == other.d->privateKey &&
          d->sessionCipher == other.d->sessionCipher &&
          d->sessionProtocol == other.d->sessionProtocol &&
          d->ciphers == other.d->ciphers &&
          d->ellipticCurves == other.d->ellipticCurves &&
          d->caCertificates == other.d->caCertificates &&
          d->protocol == other.d->protocol &&
          d->peerVerifyMode == other.d->peerVerifyMode &&
          d->peerVerifyDepth == other.d->peerVerifyDepth &&
          d->allowRootCertOnDemandLoading == other.d->allowRootCertOnDemandLoading &&
          d->sslOptions == other.d->sslOptions &&
          d->sslSession == other.d->sslSession &&
          d->sslSessionTicketLifeTimeHint == other.d->sslSessionTicketLifeTimeHint &&
          d->nextAllowedProtocols == other.d->nextAllowedProtocols &&
          d->nextNegotiatedProtocol == other.d->nextNegotiatedProtocol &&
          d->nextProtocolNegotiationStatus == other.d->nextProtocolNegotiationStatus;
}

bool QSslConfiguration::isNull() const
{
   return (d->protocol == QSsl::SecureProtocols &&
           d->peerVerifyMode == QSslSocket::AutoVerifyPeer &&
           d->peerVerifyDepth == 0 &&
           d->allowRootCertOnDemandLoading == true &&
           d->caCertificates.count() == 0 &&
           d->ciphers.count() == 0 &&
           d->ellipticCurves.isEmpty() &&
           d->localCertificateChain.isEmpty() &&
           d->privateKey.isNull() &&
           d->peerCertificate.isNull() &&
           d->peerCertificateChain.count() == 0 &&
           d->sslOptions == QSslConfigurationPrivate::defaultSslOptions &&
           d->sslSession.isNull() &&
           d->sslSessionTicketLifeTimeHint == -1 &&
           d->nextAllowedProtocols.isEmpty() &&
           d->nextNegotiatedProtocol.isNull() &&
           d->nextProtocolNegotiationStatus == QSslConfiguration::NextProtocolNegotiationNone);
}


QSsl::SslProtocol QSslConfiguration::protocol() const
{
   return d->protocol;
}

void QSslConfiguration::setProtocol(QSsl::SslProtocol protocol)
{
   d->protocol = protocol;
}

QSslSocket::PeerVerifyMode QSslConfiguration::peerVerifyMode() const
{
   return d->peerVerifyMode;
}

void QSslConfiguration::setPeerVerifyMode(QSslSocket::PeerVerifyMode mode)
{
   d->peerVerifyMode = mode;
}

int QSslConfiguration::peerVerifyDepth() const
{
   return d->peerVerifyDepth;
}

void QSslConfiguration::setPeerVerifyDepth(int depth)
{
   if (depth < 0) {
      qWarning("QSslConfiguration::setPeerVerifyDepth: Can not set negative depth of %d", depth);
      return;
   }

   d->peerVerifyDepth = depth;
}

QList<QSslCertificate> QSslConfiguration::localCertificateChain() const
{
   return d->localCertificateChain;
}

void QSslConfiguration::setLocalCertificateChain(const QList<QSslCertificate> &localChain)
{
   d->localCertificateChain = localChain;
}

QSslCertificate QSslConfiguration::localCertificate() const
{
   if (d->localCertificateChain.isEmpty()) {
      return QSslCertificate();
   }

   return d->localCertificateChain[0];
}

void QSslConfiguration::setLocalCertificate(const QSslCertificate &certificate)
{
   d->localCertificateChain = QList<QSslCertificate>();
   d->localCertificateChain += certificate;
}

QSslCertificate QSslConfiguration::peerCertificate() const
{
   return d->peerCertificate;
}

QList<QSslCertificate> QSslConfiguration::peerCertificateChain() const
{
   return d->peerCertificateChain;
}

QSslCipher QSslConfiguration::sessionCipher() const
{
   return d->sessionCipher;
}

QSsl::SslProtocol QSslConfiguration::sessionProtocol() const
{
   return d->sessionProtocol;
}

QSslKey QSslConfiguration::privateKey() const
{
   return d->privateKey;
}

void QSslConfiguration::setPrivateKey(const QSslKey &key)
{
   d->privateKey = key;
}

QList<QSslCipher> QSslConfiguration::ciphers() const
{
   return d->ciphers;
}

void QSslConfiguration::setCiphers(const QList<QSslCipher> &ciphers)
{
   d->ciphers = ciphers;
}

QList<QSslCipher> QSslConfiguration::supportedCiphers()
{
   return QSslSocketPrivate::supportedCiphers();
}

QList<QSslCertificate> QSslConfiguration::caCertificates() const
{
   return d->caCertificates;
}

void QSslConfiguration::setCaCertificates(const QList<QSslCertificate> &certificates)
{
   d->caCertificates = certificates;
   d->allowRootCertOnDemandLoading = false;
}

QList<QSslCertificate> QSslConfiguration::systemCaCertificates()
{
   // we are calling ensureInitialized() in the method below
   return QSslSocketPrivate::systemCaCertificates();
}

void QSslConfiguration::setSslOption(QSsl::SslOption option, bool on)
{
   if (on) {
      d->sslOptions |= option;
   } else {
      d->sslOptions &= ~option;
   }
}

bool QSslConfiguration::testSslOption(QSsl::SslOption option) const
{
   return d->sslOptions & option;
}

QByteArray QSslConfiguration::sessionTicket() const
{
   return d->sslSession;
}

void QSslConfiguration::setSessionTicket(const QByteArray &sessionTicket)
{
   d->sslSession = sessionTicket;
}

int QSslConfiguration::sessionTicketLifeTimeHint() const
{
   return d->sslSessionTicketLifeTimeHint;
}

QVector<QSslEllipticCurve> QSslConfiguration::ellipticCurves() const
{
   return d->ellipticCurves;
}

void QSslConfiguration::setEllipticCurves(const QVector<QSslEllipticCurve> &curves)
{
   d->ellipticCurves = curves;
}

QVector<QSslEllipticCurve> QSslConfiguration::supportedEllipticCurves()
{
   return QSslSocketPrivate::supportedEllipticCurves();
}

QByteArray QSslConfiguration::nextNegotiatedProtocol() const
{
   return d->nextNegotiatedProtocol;
}

void QSslConfiguration::setAllowedNextProtocols(const QList<QByteArray> &protocols)
{
   d->nextAllowedProtocols = protocols;
}

QList<QByteArray> QSslConfiguration::allowedNextProtocols() const
{
   return d->nextAllowedProtocols;
}

QSslConfiguration::NextProtocolNegotiationStatus QSslConfiguration::nextProtocolNegotiationStatus() const
{
   return d->nextProtocolNegotiationStatus;
}

QSslConfiguration QSslConfiguration::defaultConfiguration()
{
   return QSslConfigurationPrivate::defaultConfiguration();
}

void QSslConfiguration::setDefaultConfiguration(const QSslConfiguration &configuration)
{
   QSslConfigurationPrivate::setDefaultConfiguration(configuration);
}

// internal
bool QSslConfigurationPrivate::peerSessionWasShared(const QSslConfiguration &configuration)
{
   return configuration.d->peerSessionShared;
}

