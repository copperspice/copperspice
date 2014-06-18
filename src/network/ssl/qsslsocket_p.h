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

#ifndef QSSLSOCKET_P_H
#define QSSLSOCKET_P_H

#include <qsslsocket.h>
#include <qtcpsocket_p.h>
#include <qsslkey.h>
#include <qsslconfiguration_p.h>
#include <QtCore/qstringlist.h>
#include <qringbuffer_p.h>

#if defined(Q_OS_MAC)
#  include <Security/SecCertificate.h>
#  include <CoreFoundation/CFArray.h>

#elif defined(Q_OS_WIN)

#  include <windows.h>
#  include <wincrypt.h>

#  ifndef HCRYPTPROV_LEGACY
#    define HCRYPTPROV_LEGACY HCRYPTPROV
#  endif
#endif

QT_BEGIN_NAMESPACE

#if defined(Q_OS_MAC) && !defined(Q_OS_IOS)
typedef OSStatus (*PtrSecCertificateGetData)(SecCertificateRef, CSSM_DATA_PTR);
typedef OSStatus (*PtrSecTrustSettingsCopyCertificates)(int, CFArrayRef *);
typedef OSStatus (*PtrSecTrustCopyAnchorCertificates)(CFArrayRef *);
#endif

#if defined(Q_OS_WIN)
typedef HCERTSTORE (WINAPI *PtrCertOpenSystemStoreW)(HCRYPTPROV_LEGACY, LPCWSTR);
typedef PCCERT_CONTEXT (WINAPI *PtrCertFindCertificateInStore)(HCERTSTORE, DWORD, DWORD, DWORD, const void *,
      PCCERT_CONTEXT);
typedef BOOL (WINAPI *PtrCertCloseStore)(HCERTSTORE, DWORD);
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

   // if set, this hostname is used for certificate validation instead of the hostname
   // that was used for connecting to.
   QString verificationPeerName;

   bool allowRootCertOnDemandLoading;

   static bool supportsSsl();
   static void ensureInitialized();
   static void deinitialize();
   static QList<QSslCipher> defaultCiphers();
   static QList<QSslCipher> supportedCiphers();
   static void setDefaultCiphers(const QList<QSslCipher> &ciphers);
   static void setDefaultSupportedCiphers(const QList<QSslCipher> &ciphers);
   static void resetDefaultCiphers();

   static QList<QSslCertificate> defaultCaCertificates();
   static QList<QSslCertificate> systemCaCertificates();
   static void setDefaultCaCertificates(const QList<QSslCertificate> &certs);
   static bool addDefaultCaCertificates(const QString &path, QSsl::EncodingFormat format,
                                        QRegExp::PatternSyntax syntax);
   static void addDefaultCaCertificate(const QSslCertificate &cert);
   static void addDefaultCaCertificates(const QList<QSslCertificate> &certs);

#if defined(Q_OS_MAC) && !defined(Q_OS_IOS)
   static PtrSecCertificateGetData ptrSecCertificateGetData;
   static PtrSecTrustSettingsCopyCertificates ptrSecTrustSettingsCopyCertificates;
   static PtrSecTrustCopyAnchorCertificates ptrSecTrustCopyAnchorCertificates;
#elif defined(Q_OS_WIN)
   static PtrCertOpenSystemStoreW ptrCertOpenSystemStoreW;
   static PtrCertFindCertificateInStore ptrCertFindCertificateInStore;
   static PtrCertCloseStore ptrCertCloseStore;
#endif

   // The socket itself, including private slots.
   QTcpSocket *plainSocket;
   void createPlainSocket(QIODevice::OpenMode openMode);
   static void pauseSocketNotifiers(QSslSocket *);
   static void resumeSocketNotifiers(QSslSocket *);
   void _q_connectedSlot();
   void _q_hostFoundSlot();
   void _q_disconnectedSlot();
   void _q_stateChangedSlot(QAbstractSocket::SocketState);
   void _q_errorSlot(QAbstractSocket::SocketError);
   void _q_readyReadSlot();
   void _q_bytesWrittenSlot(qint64);
   void _q_flushWriteBuffer();
   void _q_flushReadBuffer();

   virtual qint64 peek(char *data, qint64 maxSize);
   virtual QByteArray peek(qint64 maxSize);

   // Platform specific functions
   virtual void startClientEncryption() = 0;
   virtual void startServerEncryption() = 0;
   virtual void transmit() = 0;
   virtual void disconnectFromHost() = 0;
   virtual void disconnected() = 0;
   virtual QSslCipher sessionCipher() const = 0;

   static bool rootCertOnDemandLoadingSupported();

 private:
   static bool ensureLibraryLoaded();
   static void ensureCiphersAndCertsLoaded();

   static bool s_libraryLoaded;
   static bool s_loadedCiphersAndCerts;
 protected:
   static bool s_loadRootCertsOnDemand;
   static QList<QByteArray> unixRootCertDirectories();
};

QT_END_NAMESPACE

#endif
