/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QSSLSOCKET_H
#define QSSLSOCKET_H

#include <qlist.h>
#include <qregexp.h>

#ifndef QT_NO_OPENSSL
#   include <qtcpsocket.h>
#   include <qsslerror.h>
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_NO_OPENSSL

class QDir;
class QSslCipher;
class QSslCertificate;
class QSslConfiguration;
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

   QSslSocket(QObject *parent = 0);
   ~QSslSocket();

   // Autostarting the SSL client handshake.
   void connectToHostEncrypted(const QString &hostName, quint16 port, OpenMode mode = ReadWrite);
   void connectToHostEncrypted(const QString &hostName, quint16 port, const QString &sslPeerName,
                               OpenMode mode = ReadWrite);

   bool setSocketDescriptor(int socketDescriptor, SocketState state = ConnectedState, OpenMode openMode = ReadWrite);

   // ### Qt5/Make virtual
   void setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value);
   QVariant socketOption(QAbstractSocket::SocketOption option);

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
   qint64 bytesAvailable() const;
   qint64 bytesToWrite() const;
   bool canReadLine() const;
   void close();
   bool atEnd() const;
   bool flush();
   void abort();

   // From QAbstractSocket:
   void setReadBufferSize(qint64 size);

   // Similar to QIODevice's:
   qint64 encryptedBytesAvailable() const;
   qint64 encryptedBytesToWrite() const;

   // SSL configuration
   QSslConfiguration sslConfiguration() const;
   void setSslConfiguration(const QSslConfiguration &config);

   // Certificate & cipher accessors.
   void setLocalCertificate(const QSslCertificate &certificate);
   void setLocalCertificate(const QString &fileName, QSsl::EncodingFormat format = QSsl::Pem);
   QSslCertificate localCertificate() const;
   QSslCertificate peerCertificate() const;
   QList<QSslCertificate> peerCertificateChain() const;
   QSslCipher sessionCipher() const;

   // Private keys, for server sockets
   void setPrivateKey(const QSslKey &key);
   void setPrivateKey(const QString &fileName, 
         QSsl::KeyAlgorithm algorithm = QSsl::Rsa,QSsl::EncodingFormat format = QSsl::Pem, 
         const QByteArray &passPhrase = QByteArray());

   QSslKey privateKey() const;

   // Cipher settings.
   QList<QSslCipher> ciphers() const;
   void setCiphers(const QList<QSslCipher> &ciphers);
   void setCiphers(const QString &ciphers);
   static void setDefaultCiphers(const QList<QSslCipher> &ciphers);
   static QList<QSslCipher> defaultCiphers();
   static QList<QSslCipher> supportedCiphers();

   // CA settings.
   bool addCaCertificates(const QString &path, QSsl::EncodingFormat format = QSsl::Pem,
                          QRegExp::PatternSyntax syntax = QRegExp::FixedString);

   void addCaCertificate(const QSslCertificate &certificate);
   void addCaCertificates(const QList<QSslCertificate> &certificates);
   void setCaCertificates(const QList<QSslCertificate> &certificates);
   QList<QSslCertificate> caCertificates() const;

   static bool addDefaultCaCertificates(const QString &path, QSsl::EncodingFormat format = QSsl::Pem,
                                        QRegExp::PatternSyntax syntax = QRegExp::FixedString);

   static void addDefaultCaCertificate(const QSslCertificate &certificate);
   static void addDefaultCaCertificates(const QList<QSslCertificate> &certificates);
   static void setDefaultCaCertificates(const QList<QSslCertificate> &certificates);
   static QList<QSslCertificate> defaultCaCertificates();
   static QList<QSslCertificate> systemCaCertificates();

   bool waitForConnected(int msecs = 30000);
   bool waitForEncrypted(int msecs = 30000);
   bool waitForReadyRead(int msecs = 30000);
   bool waitForBytesWritten(int msecs = 30000);
   bool waitForDisconnected(int msecs = 30000);

   QList<QSslError> sslErrors() const;

   static bool supportsSsl();
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

 protected :
   NET_CS_SLOT_1(Protected, void connectToHostImplementation(const QString &hostName, unsigned short port,
                 QIODevice::OpenMode openMode))
   NET_CS_SLOT_2(connectToHostImplementation)

   NET_CS_SLOT_1(Protected, void disconnectFromHostImplementation())
   NET_CS_SLOT_2(disconnectFromHostImplementation)
 
   qint64 readData(char *data, qint64 maxlen);
   qint64 writeData(const char *data, qint64 len);

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

   friend class QSslSocketBackendPrivate;
};

#endif // QT_NO_OPENSSL

QT_END_NAMESPACE

#ifndef QT_NO_OPENSSL
Q_DECLARE_METATYPE(QList<QSslError>)
#endif

#endif
