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

#ifndef QNATIVESOCKETENGINE_P_H
#define QNATIVESOCKETENGINE_P_H

#include <qhostaddress.h>

#include <qabstractsocketengine_p.h>

#ifndef Q_OS_WIN
#  include <qplatformdefs.h>
#  include <netinet/in.h>
#else
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <mswsock.h>
#endif

#ifdef Q_OS_WIN

#define QT_SOCKLEN_T    int
#define QT_SOCKOPTLEN_T int

#  ifndef WSAID_WSARECVMSG
      typedef INT (WINAPI *LPFN_WSARECVMSG)(SOCKET s, LPWSAMSG lpMsg,
                  LPDWORD lpdwNumberOfBytesRecvd, LPWSAOVERLAPPED lpOverlapped,
                  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

#     define WSAID_WSARECVMSG {0xf689d7c8,0x6f1f,0x436b,{0x8a,0x53,0xe5,0x4f,0xe3,0x51,0xc3,0x22}}
#  endif

#  ifndef WSAID_WSASENDMSG
      typedef struct {
        LPWSAMSG lpMsg;
        DWORD dwFlags;
        LPDWORD lpNumberOfBytesSent;
        LPWSAOVERLAPPED lpOverlapped;
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine;
      } WSASENDMSG, *LPWSASENDMSG;

      typedef INT (WSAAPI *LPFN_WSASENDMSG)(SOCKET s, LPWSAMSG lpMsg, DWORD dwFlags,
                     LPDWORD lpNumberOfBytesSent, LPWSAOVERLAPPED lpOverlapped,
                     LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

#     define WSAID_WSASENDMSG {0xa441e712,0x754f,0x43ca,{0x84,0xa7,0x0d,0xee,0x44,0xcf,0x60,0x6d}}
#  endif

#endif

union qt_sockaddr {
   sockaddr a;
   sockaddr_in a4;
   sockaddr_in6 a6;
};

class QSocketNotifier;
class QNativeSocketEnginePrivate;

#ifndef QT_NO_NETWORKINTERFACE
   class QNetworkInterface;
#endif

class QNativeSocketEngine : public QAbstractSocketEngine
{
   NET_CS_OBJECT(QNativeSocketEngine)

 public:
   QNativeSocketEngine(QObject *parent = nullptr);

   QNativeSocketEngine(const QNativeSocketEngine &) = delete;
   QNativeSocketEngine &operator=(const QNativeSocketEngine &) = delete;

   ~QNativeSocketEngine();

   bool initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol =
                  QAbstractSocket::IPv4Protocol) override;

   bool initialize(qintptr socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState) override;

   qintptr socketDescriptor() const override;

   bool isValid() const override;

   bool connectToHost(const QHostAddress &address, quint16 port) override;
   bool connectToHostByName(const QString &name, quint16 port) override;
   bool bind(const QHostAddress &address, quint16 port) override;
   bool listen() override;
   int accept() override;
   void close() override;

   qint64 bytesAvailable() const override;

   qint64 read(char *data, qint64 maxlen) override;
   qint64 write(const char *data, qint64 len)override;

#ifndef QT_NO_UDPSOCKET
#ifndef QT_NO_NETWORKINTERFACE
   bool joinMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &iface) override;
   bool leaveMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &iface) override;
   QNetworkInterface multicastInterface() const override;
   bool setMulticastInterface(const QNetworkInterface &iface) override;
#endif

   qint64 readDatagram(char *data, qint64 maxlen, QIpPacketHeader * = nullptr,
                  PacketHeaderOptions = WantNone) override;

   qint64 writeDatagram(const char *data, qint64 len, const QIpPacketHeader &) override;
   bool hasPendingDatagrams() const override;
   qint64 pendingDatagramSize() const override;
#endif

   qint64 bytesToWrite() const override;

   qint64 receiveBufferSize() const;
   void setReceiveBufferSize(qint64 bufferSize);

   qint64 sendBufferSize() const;
   void setSendBufferSize(qint64 bufferSize);

   int option(SocketOption option) const override;
   bool setOption(SocketOption option, int value) override;

   bool waitForRead(int msecs = 30000, bool *timedOut = nullptr) override;
   bool waitForWrite(int msecs = 30000, bool *timedOut = nullptr) override;
   bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite, bool checkRead, bool checkWrite,
                           int msecs = 30000, bool *timedOut = nullptr) override;

   bool isReadNotificationEnabled() const override;
   void setReadNotificationEnabled(bool enable) override;
   bool isWriteNotificationEnabled() const override;
   void setWriteNotificationEnabled(bool enable) override;
   bool isExceptionNotificationEnabled() const override;
   void setExceptionNotificationEnabled(bool enable) override;

   NET_CS_SLOT_1(Public, void connectionNotification())
   NET_CS_SLOT_2(connectionNotification)

 private:
   Q_DECLARE_PRIVATE(QNativeSocketEngine)
};

class QNativeSocketEnginePrivate : public QAbstractSocketEnginePrivate
{
 public:
   QNativeSocketEnginePrivate();
   ~QNativeSocketEnginePrivate();

   qintptr socketDescriptor;

   QSocketNotifier *readNotifier, *writeNotifier, *exceptNotifier;

#ifdef Q_OS_WIN
   LPFN_WSASENDMSG sendmsg;
   LPFN_WSARECVMSG recvmsg;
#endif

   enum ErrorString {
      NonBlockingInitFailedErrorString,
      BroadcastingInitFailedErrorString,
      NoIpV6ErrorString,
      RemoteHostClosedErrorString,
      TimeOutErrorString,
      ResourceErrorString,
      OperationUnsupportedErrorString,
      ProtocolUnsupportedErrorString,
      InvalidSocketErrorString,
      HostUnreachableErrorString,
      NetworkUnreachableErrorString,
      AccessErrorString,
      ConnectionTimeOutErrorString,
      ConnectionRefusedErrorString,
      AddressInuseErrorString,
      AddressNotAvailableErrorString,
      AddressProtectedErrorString,
      DatagramTooLargeErrorString,
      SendDatagramErrorString,
      ReceiveDatagramErrorString,
      WriteErrorString,
      ReadErrorString,
      PortInuseErrorString,
      NotSocketErrorString,
      InvalidProxyTypeString,
      TemporaryErrorString,
      NetworkDroppedConnectionErrorString,
      ConnectionResetErrorString,

      UnknownSocketErrorString = -1
   };

   void setError(QAbstractSocket::SocketError error, ErrorString errorString) const;
   QHostAddress adjustAddressProtocol(const QHostAddress &address) const;

   // native functions
   int option(QNativeSocketEngine::SocketOption option) const;
   bool setOption(QNativeSocketEngine::SocketOption option, int value);

   bool createNewSocket(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol &protocol);

   bool nativeConnect(const QHostAddress &address, quint16 port);
   bool nativeBind(const QHostAddress &address, quint16 port);
   bool nativeListen(int backlog);
   int nativeAccept();

#ifndef QT_NO_NETWORKINTERFACE
   bool nativeJoinMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &iface);
   bool nativeLeaveMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &iface);
   QNetworkInterface nativeMulticastInterface() const;
   bool nativeSetMulticastInterface(const QNetworkInterface &iface);
#endif

   qint64 nativeBytesAvailable() const;

   bool nativeHasPendingDatagrams() const;
   qint64 nativePendingDatagramSize() const;
   qint64 nativeReceiveDatagram(char *data, qint64 maxLength, QIpPacketHeader *header,
                  QAbstractSocketEngine::PacketHeaderOptions options);

   qint64 nativeSendDatagram(const char *data, qint64 length, const QIpPacketHeader &header);
   qint64 nativeRead(char *data, qint64 maxLength);
   qint64 nativeWrite(const char *data, qint64 length);
   int nativeSelect(int timeout, bool selectForRead) const;
   int nativeSelect(int timeout, bool checkRead, bool checkWrite, bool *selectForRead, bool *selectForWrite) const;

   void nativeClose();

   bool checkProxy(const QHostAddress &address);
   bool fetchConnectionParameters();
   static uint scopeIdFromString(const QString &scopeid);

#ifdef Q_OS_WIN
   void setPortAndAddress(quint16 port, const QHostAddress &address, qt_sockaddr *sa_struct, int *sockAddrSize) {
#else
   void setPortAndAddress(quint16 port, const QHostAddress &address, qt_sockaddr *sa_struct, QT_SOCKLEN_T *sockAddrSize) {
#endif

        if (address.protocol() == QAbstractSocket::IPv6Protocol
                  || address.protocol() == QAbstractSocket::AnyIPProtocol
                  || socketProtocol == QAbstractSocket::IPv6Protocol
                  || socketProtocol == QAbstractSocket::AnyIPProtocol) {

            memset(&sa_struct->a6, 0, sizeof(sockaddr_in6));
            sa_struct->a6.sin6_family   = AF_INET6;
            sa_struct->a6.sin6_scope_id = scopeIdFromString(address.scopeId());
            sa_struct->a6.sin6_port     = htons(port);

            Q_IPV6ADDR tmp = address.toIPv6Address();
            memcpy(&sa_struct->a6.sin6_addr, &tmp, sizeof(tmp));

            *sockAddrSize = sizeof(sockaddr_in6);

        } else {
            memset(&sa_struct->a, 0, sizeof(sockaddr_in));
            sa_struct->a4.sin_family      = AF_INET;
            sa_struct->a4.sin_port        = htons(port);
            sa_struct->a4.sin_addr.s_addr = htonl(address.toIPv4Address());

            *sockAddrSize = sizeof(sockaddr_in);
        }
   }

   bool check_valid_socketlayer(const char *function) const;

 private:
   Q_DECLARE_PUBLIC(QNativeSocketEngine)
};

#endif
