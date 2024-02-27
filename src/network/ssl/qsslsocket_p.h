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

#ifndef QSSLSOCKET_P_H
#define QSSLSOCKET_P_H

#include <qsslsocket.h>
#include <qsslkey.h>

#include <qtcpsocket_p.h>
#include <qsslconfiguration_p.h>

#ifdef QT_OPENSSL
#   include <qsslcontext_openssl_p.h>
#else
   class QSslContext;
#endif

#include <qstringlist.h>
#include <qringbuffer_p.h>

#if defined(Q_OS_DARWIN)
#  include <Security/SecCertificate.h>
#  include <CoreFoundation/CFArray.h>

#elif defined(Q_OS_WIN)

#  include <windows.h>
#  include <wincrypt.h>

#  ifndef HCRYPTPROV_LEGACY
#    define HCRYPTPROV_LEGACY HCRYPTPROV
#  endif

#endif

#if defined(Q_OS_DARWIN)
   using PtrSecCertificateCopyData           = CFDataRef (*)(SecCertificateRef);
   using PtrSecTrustSettingsCopyCertificates = OSStatus (*)(int, CFArrayRef*);
   using PtrSecTrustCopyAnchorCertificates   = OSStatus (*)(CFArrayRef*);

#endif

class QSslSocketPrivate : public QTcpSocketPrivate
{
   Q_DECLARE_PUBLIC(QSslSocket)

public:
   QSslSocketPrivate();
   virtual ~QSslSocketPrivate();

   void init();
   bool initialized;

   QSslSocket::SslMode mode;
   bool autoStartHandshake;
   bool connectionEncrypted;
   bool shutdown;
   bool ignoreAllSslErrors;
   QList<QSslError> ignoreErrorsList;
   bool *readyReadEmittedPointer;

   QSslConfigurationPrivate configuration;
   QList<QSslError> sslErrors;
   QSharedPointer<QSslContext> sslContextPointer;

   // if set, this hostname is used for certificate validation instead of the hostname
   // that was used for connecting to.
   QString verificationPeerName;

   bool allowRootCertOnDemandLoading;

   static bool s_loadRootCertsOnDemand;

   static bool supportsSsl();
   static long sslLibraryVersionNumber();
   static QString sslLibraryVersionString();
   static long sslLibraryBuildVersionNumber();
   static QString sslLibraryBuildVersionString();
   static void ensureInitialized();
   static void deinitialize();
   static QList<QSslCipher> defaultCiphers();
   static QList<QSslCipher> supportedCiphers();
   static void setDefaultCiphers(const QList<QSslCipher> &ciphers);
   static void setDefaultSupportedCiphers(const QList<QSslCipher> &ciphers);
   static void resetDefaultCiphers();

   static QVector<QSslEllipticCurve> supportedEllipticCurves();
   static void setDefaultSupportedEllipticCurves(const QVector<QSslEllipticCurve> &curves);
   static void resetDefaultEllipticCurves();

   static QList<QSslCertificate> defaultCaCertificates();
   static QList<QSslCertificate> systemCaCertificates();

   static void setDefaultCaCertificates(const QList<QSslCertificate> &certs);
   static bool addDefaultCaCertificates(const QString &path, QSsl::EncodingFormat format, QPatternOption syntax);
   static void addDefaultCaCertificate(const QSslCertificate &cert);
   static void addDefaultCaCertificates(const QList<QSslCertificate> &certs);

   static bool isMatchingHostname(const QSslCertificate &cert, const QString &peerName);
   static bool isMatchingHostname(const QString &cn, const QString &hostname);

#if defined(Q_OS_DARWIN)
   static PtrSecCertificateCopyData ptrSecCertificateCopyData;
   static PtrSecTrustSettingsCopyCertificates ptrSecTrustSettingsCopyCertificates;
   static PtrSecTrustCopyAnchorCertificates ptrSecTrustCopyAnchorCertificates;
#endif

   QTcpSocket *plainSocket;
   void createPlainSocket(QIODevice::OpenMode openMode);
   static void pauseSocketNotifiers(QSslSocket *);
   static void resumeSocketNotifiers(QSslSocket *);

   static void checkSettingSslContext(QSslSocket*, QSharedPointer<QSslContext>);
   static QSharedPointer<QSslContext> sslContext(QSslSocket *socket);

   bool isPaused() const;
   bool bind(const QHostAddress &address, quint16, QAbstractSocket::BindMode) override;

   void _q_connectedSlot();
   void _q_hostFoundSlot();
   void _q_disconnectedSlot();
   void _q_stateChangedSlot(QAbstractSocket::SocketState);
   void _q_errorSlot(QAbstractSocket::SocketError);
   void _q_readyReadSlot();
   void _q_bytesWrittenSlot(qint64);
   void _q_flushWriteBuffer();
   void _q_flushReadBuffer();
   void _q_resumeImplementation();

#if defined(Q_OS_WIN)
   virtual void _q_caRootLoaded(QSslCertificate cert, QSslCertificate trustedRoot) = 0;
#endif

   static QList<QByteArray> unixRootCertDirectories(); // used also by QSslContext

   qint64 peek(char *data, qint64 maxSize) override;
   QByteArray peek(qint64 maxSize) override;

   // Platform specific functions
   virtual void startClientEncryption() = 0;
   virtual void startServerEncryption() = 0;
   virtual void transmit() = 0;
   virtual void disconnectFromHost() = 0;
   virtual void disconnected() = 0;
   virtual QSslCipher sessionCipher() const = 0;
   virtual QSsl::SslProtocol sessionProtocol() const = 0;
   virtual void continueHandshake() = 0;

   static bool rootCertOnDemandLoadingSupported();

protected:
   bool verifyErrorsHaveBeenIgnored();
   bool paused;

private:
   static bool ensureLibraryLoaded();
   static void ensureCiphersAndCertsLoaded();

   static bool s_libraryLoaded;
   static bool s_loadedCiphersAndCerts;
};

#endif
