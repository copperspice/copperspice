/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QSOCKS5SOCKETENGINE_P_H
#define QSOCKS5SOCKETENGINE_P_H

#include <qabstractsocketengine_p.h>
#include <qnetworkproxy.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SOCKS5

class QSocks5SocketEnginePrivate;

class QSocks5SocketEngine : public QAbstractSocketEngine
{
   NET_CS_OBJECT(QSocks5SocketEngine)

 public:
   QSocks5SocketEngine(QObject *parent = 0);
   ~QSocks5SocketEngine();

   bool initialize(QAbstractSocket::SocketType type,
                   QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol);
   bool initialize(int socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState);

   void setProxy(const QNetworkProxy &networkProxy);

   int socketDescriptor() const;

   bool isValid() const;

   bool connectInternal();
   bool connectToHost(const QHostAddress &address, quint16 port);
   bool connectToHostByName(const QString &name, quint16 port);
   bool bind(const QHostAddress &address, quint16 port);
   bool listen();
   int accept();
   void close();

   qint64 bytesAvailable() const;

   qint64 read(char *data, qint64 maxlen);
   qint64 write(const char *data, qint64 len);

#ifndef QT_NO_UDPSOCKET

#ifndef QT_NO_NETWORKINTERFACE
   bool joinMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &interface);
   bool leaveMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &interface);
   QNetworkInterface multicastInterface() const;
   bool setMulticastInterface(const QNetworkInterface &iface);
#endif

   qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *addr = 0,quint16 *port = 0);
   qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &addr, quint16 port);
   bool hasPendingDatagrams() const;
   qint64 pendingDatagramSize() const;
#endif

   qint64 bytesToWrite() const;

   int option(SocketOption option) const;
   bool setOption(SocketOption option, int value);

   bool waitForRead(int msecs = 30000, bool *timedOut = 0);
   bool waitForWrite(int msecs = 30000, bool *timedOut = 0);

   bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite, bool checkRead, bool checkWrite,
                           int msecs = 30000, bool *timedOut = 0);

   bool isReadNotificationEnabled() const;
   void setReadNotificationEnabled(bool enable);
   bool isWriteNotificationEnabled() const;
   void setWriteNotificationEnabled(bool enable);
   bool isExceptionNotificationEnabled() const;
   void setExceptionNotificationEnabled(bool enable);

 private:
   Q_DECLARE_PRIVATE(QSocks5SocketEngine)
   Q_DISABLE_COPY(QSocks5SocketEngine)

   NET_CS_SLOT_1(Private, void _q_controlSocketConnected())
   NET_CS_SLOT_2(_q_controlSocketConnected)

   NET_CS_SLOT_1(Private, void _q_controlSocketReadNotification())
   NET_CS_SLOT_2(_q_controlSocketReadNotification)

   NET_CS_SLOT_1(Private, void _q_controlSocketError(QAbstractSocket::SocketError un_named_arg1))
   NET_CS_SLOT_2(_q_controlSocketError)

#ifndef QT_NO_UDPSOCKET
   NET_CS_SLOT_1(Private, void _q_udpSocketReadNotification())
   NET_CS_SLOT_2(_q_udpSocketReadNotification)
#endif

   NET_CS_SLOT_1(Private, void _q_controlSocketBytesWritten())
   NET_CS_SLOT_2(_q_controlSocketBytesWritten)

   NET_CS_SLOT_1(Private, void _q_emitPendingReadNotification())
   NET_CS_SLOT_2(_q_emitPendingReadNotification)

   NET_CS_SLOT_1(Private, void _q_emitPendingWriteNotification())
   NET_CS_SLOT_2(_q_emitPendingWriteNotification)

   NET_CS_SLOT_1(Private, void _q_emitPendingConnectionNotification())
   NET_CS_SLOT_2(_q_emitPendingConnectionNotification)

   NET_CS_SLOT_1(Private, void _q_controlSocketDisconnected())
   NET_CS_SLOT_2(_q_controlSocketDisconnected)

   NET_CS_SLOT_1(Private, void _q_controlSocketStateChanged(QAbstractSocket::SocketState un_named_arg1))
   NET_CS_SLOT_2(_q_controlSocketStateChanged)
};

class QTcpSocket;

class QSocks5Authenticator
{
 public:
   QSocks5Authenticator();
   virtual ~QSocks5Authenticator();
   virtual char methodId();
   virtual bool beginAuthenticate(QTcpSocket *socket, bool *completed);
   virtual bool continueAuthenticate(QTcpSocket *socket, bool *completed);

   virtual bool seal(const QByteArray buf, QByteArray *sealedBuf);
   virtual bool unSeal(const QByteArray sealedBuf, QByteArray *buf);
   virtual bool unSeal(QTcpSocket *sealedSocket, QByteArray *buf);

   virtual QString errorString() {
      return QString();
   }
};

class QSocks5PasswordAuthenticator : public QSocks5Authenticator
{
 public:
   QSocks5PasswordAuthenticator(const QString &userName, const QString &password);
   char methodId();
   bool beginAuthenticate(QTcpSocket *socket, bool *completed);
   bool continueAuthenticate(QTcpSocket *socket, bool *completed);

   QString errorString();

 private:
   QString userName;
   QString password;
};

struct QSocks5Data;
struct QSocks5ConnectData;
struct QSocks5UdpAssociateData;
struct QSocks5BindData;

class QSocks5SocketEnginePrivate : public QAbstractSocketEnginePrivate
{
   Q_DECLARE_PUBLIC(QSocks5SocketEngine)

 public:
   QSocks5SocketEnginePrivate();
   ~QSocks5SocketEnginePrivate();

   enum Socks5State {
      Uninitialized = 0,
      ConnectError,
      AuthenticationMethodsSent,
      Authenticating,
      AuthenticatingError,
      RequestMethodSent,
      RequestError,
      Connected,
      UdpAssociateSuccess,
      BindSuccess,
      ControlSocketError,
      SocksError,
      HostNameLookupError
   };
   Socks5State socks5State;

   enum Socks5Mode {
      NoMode,
      ConnectMode,
      BindMode,
      UdpAssociateMode
   };
   Socks5Mode mode;

   enum Socks5Error {
      SocksFailure = 0x01,
      ConnectionNotAllowed = 0x02,
      NetworkUnreachable = 0x03,
      HostUnreachable = 0x04,
      ConnectionRefused = 0x05,
      TTLExpired = 0x06,
      CommandNotSupported = 0x07,
      AddressTypeNotSupported = 0x08,
      LastKnownError = AddressTypeNotSupported,
      UnknownError
   };

   void initialize(Socks5Mode socks5Mode);

   void setErrorState(Socks5State state, const QString &extraMessage = QString());
   void setErrorState(Socks5State state, Socks5Error socks5error);

   void reauthenticate();
   void parseAuthenticationMethodReply();
   void parseAuthenticatingReply();
   void sendRequestMethod();
   void parseRequestMethodReply();
   void parseNewConnection();

   bool waitForConnected(int msecs, bool *timedOut);

   void _q_controlSocketConnected();
   void _q_controlSocketReadNotification();
   void _q_controlSocketError(QAbstractSocket::SocketError);

#ifndef QT_NO_UDPSOCKET
   void checkForDatagrams() const;
   void _q_udpSocketReadNotification();
#endif

   void _q_controlSocketBytesWritten();
   void _q_controlSocketDisconnected();
   void _q_controlSocketStateChanged(QAbstractSocket::SocketState);

   QNetworkProxy proxyInfo;

   bool readNotificationEnabled, writeNotificationEnabled, exceptNotificationEnabled;

   int socketDescriptor;

   QSocks5Data *data;
   QSocks5ConnectData *connectData;

#ifndef QT_NO_UDPSOCKET
   QSocks5UdpAssociateData *udpData;
#endif

   QSocks5BindData *bindData;
   QString peerName;

   mutable bool readNotificationActivated;
   mutable bool writeNotificationActivated;

   bool readNotificationPending;
   void _q_emitPendingReadNotification();
   void emitReadNotification();
   bool writeNotificationPending;
   void _q_emitPendingWriteNotification();
   void emitWriteNotification();
   bool connectionNotificationPending;
   void _q_emitPendingConnectionNotification();
   void emitConnectionNotification();
};

class QSocks5SocketEngineHandler : public QSocketEngineHandler
{
 public:
   virtual QAbstractSocketEngine *createSocketEngine(QAbstractSocket::SocketType socketType,
         const QNetworkProxy &, QObject *parent);
   virtual QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent);
};

QT_END_NAMESPACE
#endif // QT_NO_SOCKS5
#endif // QSOCKS5SOCKETENGINE_H
