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

#ifndef QSSLSOCKET_H
#define QSSLSOCKET_H

#include <qlist.h>
#include <qregexp.h>

#ifdef QT_SSL

#include <qtcpsocket.h>
#include <qsslerror.h>

class QDir;
class QSslCipher;
class QSslCertificate;
class QSslConfiguration;
class QSslEllipticCurve;
class QSslPreSharedKeyAuthenticator;
class QSslSocketPrivate;

class Q_NETWORK_EXPORT QSslSocket : public QTcpSocket
{
   NET_CS_OBJECT(QSslSocket)

public:
   enum SslMode {
      UnencryptedMode,
      SslClientMode,
      SslServerMode
   };

   enum PeerVerifyMode {
      VerifyNone,
      QueryPeer,
      VerifyPeer,
      AutoVerifyPeer
   };

   QSslSocket(QObject *parent = nullptr);
   ~QSslSocket();

   void resume() override; // to continue after proxy authentication required, SSL errors etc.

   // Autostarting the SSL client handshake.
   void connectToHostEncrypted(const QString &hostName, quint16 port, OpenMode mode = ReadWrite,
                  NetworkLayerProtocol protocol = AnyIPProtocol);

    void connectToHostEncrypted(const QString &hostName, quint16 port, const QString &sslPeerName,
                  OpenMode mode = ReadWrite, NetworkLayerProtocol protocol = AnyIPProtocol);

   bool setSocketDescriptor(qintptr socketDescriptor, SocketState state = ConnectedState,
                  OpenMode openMode = ReadWrite) override;

   using QAbstractSocket::connectToHost;
   void connectToHost(const QString &hostName, quint16 port, OpenMode openMode = ReadWrite,
                  NetworkLayerProtocol protocol = AnyIPProtocol) override;

   void disconnectFromHost() override;

   virtual void setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value) override;
   virtual QVariant socketOption(QAbstractSocket::SocketOption option) override;

   SslMode mode() const;
   bool isEncrypted() const;

   QSsl::SslProtocol protocol() const;
   void setProtocol(QSsl::SslProtocol protocol);

   QSslSocket::PeerVerifyMode peerVerifyMode() const;
   void setPeerVerifyMode(QSslSocket::PeerVerifyMode mode);

   int peerVerifyDepth() const;
   void setPeerVerifyDepth(int depth);

   QString peerVerifyName() const;
   void setPeerVerifyName(const QString &hostName);

   // From QIODevice
   qint64 bytesAvailable() const override;
   qint64 bytesToWrite() const override;
   bool canReadLine() const override;
   void close() override;
   bool atEnd() const override;
   bool flush();
   void abort();

   // From QAbstractSocket:
   void setReadBufferSize(qint64 size) override;

   // Similar to QIODevice's:
   qint64 encryptedBytesAvailable() const;
   qint64 encryptedBytesToWrite() const;

   // SSL configuration
   QSslConfiguration sslConfiguration() const;
   void setSslConfiguration(const QSslConfiguration &config);

   // Certificate & cipher accessors.
   void setLocalCertificateChain(const QList<QSslCertificate> &localChain);
   QList<QSslCertificate> localCertificateChain() const;

   void setLocalCertificate(const QSslCertificate &certificate);
   void setLocalCertificate(const QString &fileName, QSsl::EncodingFormat format = QSsl::Pem);

   QSslCertificate localCertificate() const;
   QSslCertificate peerCertificate() const;
   QList<QSslCertificate> peerCertificateChain() const;
   QSslCipher sessionCipher() const;
   QSsl::SslProtocol sessionProtocol() const;

   // Private keys, for server sockets
   void setPrivateKey(const QSslKey &key);
   void setPrivateKey(const QString &fileName,
                  QSsl::KeyAlgorithm algorithm = QSsl::Rsa,QSsl::EncodingFormat format = QSsl::Pem,
                  const QByteArray &passPhrase = QByteArray());

   QSslKey privateKey() const;

   // CA settings.
   bool addCaCertificates(const QString &path, QSsl::EncodingFormat format = QSsl::Pem,
                  QRegExp::PatternSyntax syntax = QRegExp::FixedString);

   void addCaCertificate(const QSslCertificate &certificate);
   void addCaCertificates(const QList<QSslCertificate> &certificates);

   static bool addDefaultCaCertificates(const QString &path, QSsl::EncodingFormat format = QSsl::Pem,
                  QRegExp::PatternSyntax syntax = QRegExp::FixedString);

   static void addDefaultCaCertificate(const QSslCertificate &certificate);
   static void addDefaultCaCertificates(const QList<QSslCertificate> &certificates);

   bool waitForConnected(int msecs = 30000) override;
   bool waitForEncrypted(int msecs = 30000);
   bool waitForReadyRead(int msecs = 30000) override;
   bool waitForBytesWritten(int msecs = 30000) override;
   bool waitForDisconnected(int msecs = 30000) override;

   QList<QSslError> sslErrors() const;

   static bool supportsSsl();
   static long sslLibraryVersionNumber();
   static QString sslLibraryVersionString();
   static long sslLibraryBuildVersionNumber();
   static QString sslLibraryBuildVersionString();

   void ignoreSslErrors(const QList<QSslError> &errors);

   NET_CS_SLOT_1(Public, void startClientEncryption())
   NET_CS_SLOT_2(startClientEncryption)

   NET_CS_SLOT_1(Public, void startServerEncryption())
   NET_CS_SLOT_2(startServerEncryption)

   NET_CS_SLOT_1(Public, void ignoreSslErrors())
   NET_CS_SLOT_OVERLOAD(ignoreSslErrors, ())

   NET_CS_SIGNAL_1(Public, void encrypted())
   NET_CS_SIGNAL_2(encrypted)

   NET_CS_SIGNAL_1(Public, void peerVerifyError(const QSslError &error))
   NET_CS_SIGNAL_2(peerVerifyError, error)

   NET_CS_SIGNAL_1(Public, void sslErrors(const QList <QSslError> &errors))
   NET_CS_SIGNAL_OVERLOAD(sslErrors, (const QList <QSslError> &), errors)

   NET_CS_SIGNAL_1(Public, void modeChanged(QSslSocket::SslMode newMode))
   NET_CS_SIGNAL_2(modeChanged, newMode)

   NET_CS_SIGNAL_1(Public, void encryptedBytesWritten(qint64 totalBytes))
   NET_CS_SIGNAL_2(encryptedBytesWritten, totalBytes)

   NET_CS_SIGNAL_1(Public, void preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator))
   NET_CS_SIGNAL_2(preSharedKeyAuthenticationRequired, authenticator)

protected:
   qint64 readData(char *data, qint64 maxlen) override;
   qint64 writeData(const char *data, qint64 len) override;

private:
   Q_DECLARE_PRIVATE(QSslSocket)
   Q_DISABLE_COPY(QSslSocket)

   NET_CS_SLOT_1(Private, void _q_connectedSlot())
   NET_CS_SLOT_2(_q_connectedSlot)

   NET_CS_SLOT_1(Private, void _q_hostFoundSlot())
   NET_CS_SLOT_2(_q_hostFoundSlot)

   NET_CS_SLOT_1(Private, void _q_disconnectedSlot())
   NET_CS_SLOT_2(_q_disconnectedSlot)

   NET_CS_SLOT_1(Private, void _q_stateChangedSlot(QAbstractSocket::SocketState un_named_arg1))
   NET_CS_SLOT_2(_q_stateChangedSlot)

   NET_CS_SLOT_1(Private, void _q_errorSlot(QAbstractSocket::SocketError un_named_arg1))
   NET_CS_SLOT_2(_q_errorSlot)

   NET_CS_SLOT_1(Private, void _q_readyReadSlot())
   NET_CS_SLOT_2(_q_readyReadSlot)

   NET_CS_SLOT_1(Private, void _q_bytesWrittenSlot(qint64 un_named_arg1))
   NET_CS_SLOT_2(_q_bytesWrittenSlot)

   NET_CS_SLOT_1(Private, void _q_flushWriteBuffer())
   NET_CS_SLOT_2(_q_flushWriteBuffer)

   NET_CS_SLOT_1(Private, void _q_flushReadBuffer())
   NET_CS_SLOT_2(_q_flushReadBuffer)

   NET_CS_SLOT_1(Private, void _q_resumeImplementation())
   NET_CS_SLOT_2(_q_resumeImplementation)

#if defined(Q_OS_WIN)
   NET_CS_SLOT_1(Private, void _q_caRootLoaded(QSslCertificate arg1, QSslCertificate arg2))
   NET_CS_SLOT_2(_q_caRootLoaded)
#endif

   friend class QSslSocketBackendPrivate;
};

#endif

#endif
