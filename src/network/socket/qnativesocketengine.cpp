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

#include <qnativesocketengine_p.h>

#include <qabstracteventdispatcher.h>
#include <qsocketnotifier.h>
#include <qnetworkinterface.h>

#include <qthread_p.h>

#if ! defined(QT_NO_NETWORKPROXY)
# include <qnetworkproxy.h>
# include <qabstractsocket.h>
# include <qtcpserver.h>
#endif

// Common constructs
#define Q_CHECK_STATE(function, checkState, returnValue) do { \
    if (d->socketState != (checkState)) { \
        qWarning(""#function" was not called in "#checkState); \
        return (returnValue); \
    } } while (false)

#define Q_CHECK_NOT_STATE(function, checkState, returnValue) do { \
    if (d->socketState == (checkState)) { \
        qWarning(""#function" was called in "#checkState); \
        return (returnValue); \
    } } while (false)

#define Q_CHECK_STATES(function, state1, state2, returnValue) do { \
    if (d->socketState != (state1) && d->socketState != (state2)) { \
        qWarning(""#function" was called" \
                 " not in "#state1" or "#state2); \
        return (returnValue); \
    } } while (false)

#define Q_CHECK_STATES3(function, state1, state2, state3, returnValue) do { \
    if (d->socketState != (state1) && d->socketState != (state2) && d->socketState != (state3)) { \
        qWarning(""#function" was called" \
                 " not in "#state1" or "#state2); \
        return (returnValue); \
    } } while (false)

#define Q_CHECK_TYPE(function, type, returnValue) do { \
    if (d->socketType != (type)) { \
        qWarning(#function" was called by a" \
                 " socket other than "#type""); \
        return (returnValue); \
    } } while (false)

QNativeSocketEnginePrivate::QNativeSocketEnginePrivate()
   : socketDescriptor(-1), readNotifier(nullptr), writeNotifier(nullptr), exceptNotifier(nullptr)
{

#if defined(Q_OS_WIN)
   QSysInfo::machineHostName();        // this initializes ws2_32.dll
#endif
}

QNativeSocketEnginePrivate::~QNativeSocketEnginePrivate()
{
}

void QNativeSocketEnginePrivate::setError(QAbstractSocket::SocketError error, ErrorString errorString) const
{
   if (hasSetSocketError) {
      // Only set socket errors once for one engine; expect the
      // socket to recreate its engine after an error. Note: There's
      // one exception: SocketError(11) bypasses this as it's purely
      // a temporary internal error condition.
      // Another exception is the way the waitFor*() functions set
      // an error when a timeout occurs. After the call to setError()
      // they reset the hasSetSocketError to false
      return;
   }
   if (error != QAbstractSocket::SocketError(11)) {
      hasSetSocketError = true;
   }

   socketError = error;

   switch (errorString) {
      case NonBlockingInitFailedErrorString:
         socketErrorString = QNativeSocketEngine::tr("Unable to initialize non-blocking socket");
         break;
      case BroadcastingInitFailedErrorString:
         socketErrorString = QNativeSocketEngine::tr("Unable to initialize broadcast socket");
         break;
      // should not happen anymore
      case NoIpV6ErrorString:
         socketErrorString = QNativeSocketEngine::tr("Attempt to use IPv6 socket on a platform with no IPv6 support");
         break;
      case RemoteHostClosedErrorString:
         socketErrorString = QNativeSocketEngine::tr("The remote host closed the connection");
         break;
      case TimeOutErrorString:
         socketErrorString = QNativeSocketEngine::tr("Network operation timed out");
         break;
      case ResourceErrorString:
         socketErrorString = QNativeSocketEngine::tr("Out of resources");
         break;
      case OperationUnsupportedErrorString:
         socketErrorString = QNativeSocketEngine::tr("Unsupported socket operation");
         break;
      case ProtocolUnsupportedErrorString:
         socketErrorString = QNativeSocketEngine::tr("Protocol type not supported");
         break;
      case InvalidSocketErrorString:
         socketErrorString = QNativeSocketEngine::tr("Invalid socket descriptor");
         break;
      case HostUnreachableErrorString:
         socketErrorString = QNativeSocketEngine::tr("Host unreachable");
         break;
      case NetworkUnreachableErrorString:
         socketErrorString = QNativeSocketEngine::tr("Network unreachable");
         break;
      case AccessErrorString:
         socketErrorString = QNativeSocketEngine::tr("Permission denied");
         break;
      case ConnectionTimeOutErrorString:
         socketErrorString = QNativeSocketEngine::tr("Connection timed out");
         break;
      case ConnectionRefusedErrorString:
         socketErrorString = QNativeSocketEngine::tr("Connection refused");
         break;
      case AddressInuseErrorString:
         socketErrorString = QNativeSocketEngine::tr("The bound address is already in use");
         break;
      case AddressNotAvailableErrorString:
         socketErrorString = QNativeSocketEngine::tr("The address is not available");
         break;
      case AddressProtectedErrorString:
         socketErrorString = QNativeSocketEngine::tr("The address is protected");
         break;
      case DatagramTooLargeErrorString:
         socketErrorString = QNativeSocketEngine::tr("Datagram was too large to send");
         break;
      case SendDatagramErrorString:
         socketErrorString = QNativeSocketEngine::tr("Unable to send a message");
         break;
      case ReceiveDatagramErrorString:
         socketErrorString = QNativeSocketEngine::tr("Unable to receive a message");
         break;
      case WriteErrorString:
         socketErrorString = QNativeSocketEngine::tr("Unable to write");
         break;
      case ReadErrorString:
         socketErrorString = QNativeSocketEngine::tr("Network error");
         break;
      case PortInuseErrorString:
         socketErrorString = QNativeSocketEngine::tr("Another socket is already listening on the same port");
         break;
      case NotSocketErrorString:
         socketErrorString = QNativeSocketEngine::tr("Operation on non-socket");
         break;
      case InvalidProxyTypeString:
         socketErrorString = QNativeSocketEngine::tr("The proxy type is invalid for this operation");
         break;
      case TemporaryErrorString:
         socketErrorString = QNativeSocketEngine::tr("Temporary error");
         break;
      case NetworkDroppedConnectionErrorString:
         socketErrorString = QNativeSocketEngine::tr("Network dropped connection on reset");
         break;
      case ConnectionResetErrorString:
         socketErrorString = QNativeSocketEngine::tr("Connection reset by peer");
         break;
      case UnknownSocketErrorString:
         socketErrorString = QNativeSocketEngine::tr("Unknown error");
         break;
   }
}

QHostAddress QNativeSocketEnginePrivate::adjustAddressProtocol(const QHostAddress &address) const
{

   QAbstractSocket::NetworkLayerProtocol targetProtocol = socketProtocol;
   if (Q_LIKELY(targetProtocol == QAbstractSocket::UnknownNetworkLayerProtocol)) {
      return address;
   }

   QAbstractSocket::NetworkLayerProtocol sourceProtocol = address.protocol();

   if (targetProtocol == QAbstractSocket::AnyIPProtocol) {
      targetProtocol = QAbstractSocket::IPv6Protocol;
   }

   if (targetProtocol == QAbstractSocket::IPv6Protocol && sourceProtocol == QAbstractSocket::IPv4Protocol) {
      // convert to IPv6 v4-mapped address. This always works.
      return QHostAddress(address.toIPv6Address());
   }

   if (targetProtocol == QAbstractSocket::IPv4Protocol && sourceProtocol == QAbstractSocket::IPv6Protocol) {
      // convert to IPv4 if the source is a v4-mapped address
      quint32 ip4 = address.toIPv4Address();

      if (ip4) {
         return QHostAddress(ip4);
      }
   }

   return address;
}

bool QNativeSocketEnginePrivate::checkProxy(const QHostAddress &address)
{
   if (address.isLoopback()) {
      return true;
   }

#if ! defined(QT_NO_NETWORKPROXY)
   QObject *parent = q_func()->parent();
   QNetworkProxy proxy;
   if (QAbstractSocket *socket = dynamic_cast<QAbstractSocket *>(parent)) {
      proxy = socket->proxy();
   } else if (QTcpServer *server = dynamic_cast<QTcpServer *>(parent)) {
      proxy = server->proxy();
   } else {
      // no parent -> no proxy
      return true;
   }

   if (proxy.type() == QNetworkProxy::DefaultProxy) {
      proxy = QNetworkProxy::applicationProxy();
   }

   if (proxy.type() != QNetworkProxy::DefaultProxy &&
         proxy.type() != QNetworkProxy::NoProxy) {
      // QNativeSocketEngine doesn't do proxies
      setError(QAbstractSocket::UnsupportedSocketOperationError,
               QNativeSocketEnginePrivate::InvalidProxyTypeString);
      return false;
   }
#endif

   return true;
}

bool QNativeSocketEnginePrivate::check_valid_socketlayer(const char *function) const
{
   Q_Q(const QNativeSocketEngine);

   if (! q->isValid()) {
      qWarning("%s was called on an uninitialized socket device", function);
      return false;
   }

   return true;
}

QNativeSocketEngine::QNativeSocketEngine(QObject *parent)
   : QAbstractSocketEngine(*new QNativeSocketEnginePrivate(), parent)
{
}

QNativeSocketEngine::~QNativeSocketEngine()
{
   close();
}

bool QNativeSocketEngine::initialize(QAbstractSocket::SocketType socketType, QAbstractSocket::NetworkLayerProtocol protocol)
{
   Q_D(QNativeSocketEngine);

   if (isValid()) {
      close();
   }

   // Create the socket
   if (!d->createNewSocket(socketType, protocol)) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      QString typeStr = "UnknownSocketType";

      if (socketType == QAbstractSocket::TcpSocket) {
         typeStr = "TcpSocket";

      } else if (socketType == QAbstractSocket::UdpSocket) {
         typeStr = "UdpSocket";
      }

      QString protocolStr = "UnknownProtocol";
      if (protocol == QAbstractSocket::IPv4Protocol) {
         protocolStr = "IPv4Protocol";

      } else if (protocol == QAbstractSocket::IPv6Protocol) {
         protocolStr = "IPv6Protocol";
      }

      qDebug("QNativeSocketEngine::initialize(type == %s, protocol == %s) failed: %s",
             typeStr.toLatin1().constData(), protocolStr.toLatin1().constData(), d->socketErrorString.toLatin1().constData());
#endif

      return false;
   }

   if (socketType == QAbstractSocket::UdpSocket) {

      // Set the broadcasting flag if it's a UDP socket.
      if (!setOption(BroadcastSocketOption, 1)) {
         d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                     QNativeSocketEnginePrivate::BroadcastingInitFailedErrorString);
         close();
         return false;
      }

      // Set some extra flags that are interesting to us, but accept failure
      setOption(ReceivePacketInformation, 1);
      setOption(ReceiveHopLimit, 1);
   }

   // Make sure we receive out-of-band data
   if (socketType == QAbstractSocket::TcpSocket && !setOption(ReceiveOutOfBandData, 1)) {
      qWarning("QNativeSocketEngine::initialize() Unable to inline out-of-band data");
   }

   return true;
}

bool QNativeSocketEngine::initialize(qintptr socketDescriptor, QAbstractSocket::SocketState socketState)
{
   Q_D(QNativeSocketEngine);

   if (isValid()) {
      close();
   }

   d->socketDescriptor = socketDescriptor;

   // determine socket type and protocol
   if (! d->fetchConnectionParameters()) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug() << "QNativeSocketEngine::initialize(socketDescriptor) failed:"
               << socketDescriptor << d->socketErrorString;
#endif

      d->socketDescriptor = -1;
      return false;
   }

   if (d->socketType != QAbstractSocket::UnknownSocketType) {
      // Make the socket nonblocking

      if (! setOption(NonBlockingSocketOption, 1)) {
         d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                     QNativeSocketEnginePrivate::NonBlockingInitFailedErrorString);
         close();
         return false;
      }

      // Set the broadcasting flag if it's a UDP socket.
      if (d->socketType == QAbstractSocket::UdpSocket
            && !setOption(BroadcastSocketOption, 1)) {
         d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                     QNativeSocketEnginePrivate::BroadcastingInitFailedErrorString);
         close();
         return false;
      }
   }

   d->socketState = socketState;
   return true;
}

bool QNativeSocketEngine::isValid() const
{
   Q_D(const QNativeSocketEngine);
   return d->socketDescriptor != -1;
}

qintptr QNativeSocketEngine::socketDescriptor() const
{
   Q_D(const QNativeSocketEngine);
   return d->socketDescriptor;
}

bool QNativeSocketEngine::connectToHost(const QHostAddress &address, quint16 port)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::connectToHost()")) {
      return false;

   } else if (! d->checkProxy(address)) {
      return false;
   }

   Q_CHECK_STATES3(QNativeSocketEngine::connectToHost(), QAbstractSocket::BoundState,
                   QAbstractSocket::UnconnectedState, QAbstractSocket::ConnectingState, false);

   d->peerAddress = address;
   d->peerPort = port;
   bool connected = d->nativeConnect(d->adjustAddressProtocol(address), port);

   if (connected) {
      d->fetchConnectionParameters();
   }

   return connected;
}
void QNativeSocketEngine::connectionNotification()
{
   Q_D(QNativeSocketEngine);
   Q_ASSERT(state() == QAbstractSocket::ConnectingState);

   connectToHost(d->peerAddress, d->peerPort);
   if (state() != QAbstractSocket::ConnectingState) {
      // we changed states
      QAbstractSocketEngine::connectionNotification();
   }
}

bool QNativeSocketEngine::connectToHostByName(const QString &name, quint16 port)
{
   Q_D(QNativeSocketEngine);

   (void) name;
   (void) port;

   d->setError(QAbstractSocket::UnsupportedSocketOperationError,
               QNativeSocketEnginePrivate::OperationUnsupportedErrorString);

   return false;
}

bool QNativeSocketEngine::bind(const QHostAddress &address, quint16 port)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::bind()")) {
      return false;

   } else if (! d->checkProxy(address)) {
      return false;
   }

   Q_CHECK_STATE(QNativeSocketEngine::bind(), QAbstractSocket::UnconnectedState, false);

   if (! d->nativeBind(d->adjustAddressProtocol(address), port)) {
      return false;
   }

   d->fetchConnectionParameters();

   return true;
}

bool QNativeSocketEngine::listen()
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::listen()")) {
      return false;
   }

   Q_CHECK_STATE(QNativeSocketEngine::listen(), QAbstractSocket::BoundState, false);
   Q_CHECK_TYPE(QNativeSocketEngine::listen(), QAbstractSocket::TcpSocket, false);

   // Wevare using a backlog of 50. Most modern kernels support TCP
   // syncookies by default, and if they do, the backlog is ignored.
   // When there is no support for TCP syncookies, this value is fine.

   return d->nativeListen(50);
}

int QNativeSocketEngine::accept()
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::accept()")) {
      return -1;
   }

   Q_CHECK_STATE(QNativeSocketEngine::accept(), QAbstractSocket::ListeningState, -1);
   Q_CHECK_TYPE(QNativeSocketEngine::accept(), QAbstractSocket::TcpSocket, -1);

   return d->nativeAccept();
}

qint64 QNativeSocketEngine::bytesAvailable() const
{
   Q_D(const QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::bytesAvailable()")) {
      return -1;
   }

   Q_CHECK_NOT_STATE(QNativeSocketEngine::bytesAvailable(), QAbstractSocket::UnconnectedState, -1);

   return d->nativeBytesAvailable();
}

#ifndef QT_NO_UDPSOCKET
#ifndef QT_NO_NETWORKINTERFACE

bool QNativeSocketEngine::joinMulticastGroup(const QHostAddress &groupAddress,
      const QNetworkInterface &iface)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::joinMulticastGroup()")) {
      return false;
   }

   Q_CHECK_STATE(QNativeSocketEngine::joinMulticastGroup(), QAbstractSocket::BoundState, false);
   Q_CHECK_TYPE(QNativeSocketEngine::joinMulticastGroup(), QAbstractSocket::UdpSocket, false);

   // if the user binds a socket to an IPv6 address (or QHostAddress::Any) and
   // then attempts to join an IPv4 multicast group, this won't work on
   // Windows. In order to make this cross-platform, we warn & fail on all platforms.

   if (groupAddress.protocol() == QAbstractSocket::IPv4Protocol &&
         (d->socketProtocol == QAbstractSocket::IPv6Protocol ||
          d->socketProtocol == QAbstractSocket::AnyIPProtocol)) {
      qWarning("QNativeSocketEngine::joinMulticastGroup() Unable to bind to QHostAddress::Any (or IPv6 address) "
            "and join an IPv4 multicast group, bind to QHostAddress::AnyIPv4 instead");
      return false;
   }

   return d->nativeJoinMulticastGroup(groupAddress, iface);
}

bool QNativeSocketEngine::leaveMulticastGroup(const QHostAddress &groupAddress, const QNetworkInterface &iface)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::leaveMulticastGroup()")) {
      return false;
   }

   Q_CHECK_STATE(QNativeSocketEngine::leaveMulticastGroup(), QAbstractSocket::BoundState, false);
   Q_CHECK_TYPE(QNativeSocketEngine::leaveMulticastGroup(), QAbstractSocket::UdpSocket, false);

   return d->nativeLeaveMulticastGroup(groupAddress, iface);
}

QNetworkInterface QNativeSocketEngine::multicastInterface() const
{
   Q_D(const QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::multicastInterface()")) {
      return QNetworkInterface();
   }

   Q_CHECK_TYPE(QNativeSocketEngine::multicastInterface(), QAbstractSocket::UdpSocket, QNetworkInterface());

   return d->nativeMulticastInterface();
}

bool QNativeSocketEngine::setMulticastInterface(const QNetworkInterface &iface)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::setMulticastInterface()")) {
      return false;
   }

   Q_CHECK_TYPE(QNativeSocketEngine::setMulticastInterface(), QAbstractSocket::UdpSocket, false);

   return d->nativeSetMulticastInterface(iface);
}

#endif // QT_NO_NETWORKINTERFACE


bool QNativeSocketEngine::hasPendingDatagrams() const
{
   Q_D(const QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::hasPendingDatagrams()")) {
      return false;
   }

   Q_CHECK_NOT_STATE(QNativeSocketEngine::hasPendingDatagrams(), QAbstractSocket::UnconnectedState, false);
   Q_CHECK_TYPE(QNativeSocketEngine::hasPendingDatagrams(), QAbstractSocket::UdpSocket, false);

   return d->nativeHasPendingDatagrams();
}

qint64 QNativeSocketEngine::pendingDatagramSize() const
{
   Q_D(const QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::pendingDatagramSize()")) {
      return -1;
   }

   Q_CHECK_TYPE(QNativeSocketEngine::pendingDatagramSize(), QAbstractSocket::UdpSocket, -1);

   return d->nativePendingDatagramSize();
}

qint64 QNativeSocketEngine::readDatagram(char *data, qint64 maxSize, QIpPacketHeader *header, PacketHeaderOptions options)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::readDatagram()")) {
      return -1;
   }

   Q_CHECK_TYPE(QNativeSocketEngine::readDatagram(), QAbstractSocket::UdpSocket, -1);

   return d->nativeReceiveDatagram(data, maxSize, header, options);
}

qint64 QNativeSocketEngine::writeDatagram(const char *data, qint64 size, const QIpPacketHeader &header)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::writeDatagram()")) {
      return -1;
   }

   Q_CHECK_TYPE(QNativeSocketEngine::writeDatagram(), QAbstractSocket::UdpSocket, -1);

   return d->nativeSendDatagram(data, size, header);
}
#endif // QT_NO_UDPSOCKET

qint64 QNativeSocketEngine::write(const char *data, qint64 size)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::write()")) {
      return -1;
   }

   Q_CHECK_STATE(QNativeSocketEngine::write(), QAbstractSocket::ConnectedState, -1);

   return d->nativeWrite(data, size);
}

qint64 QNativeSocketEngine::bytesToWrite() const
{
   return 0;
}

qint64 QNativeSocketEngine::read(char *data, qint64 maxSize)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::read()")) {
      return -1;
   }

   Q_CHECK_STATES(QNativeSocketEngine::read(), QAbstractSocket::ConnectedState, QAbstractSocket::BoundState, -1);

   qint64 readBytes = d->nativeRead(data, maxSize);

   // Handle remote close
   if (readBytes == 0 && d->socketType == QAbstractSocket::TcpSocket) {
      d->setError(QAbstractSocket::RemoteHostClosedError,
                  QNativeSocketEnginePrivate::RemoteHostClosedErrorString);
      close();
      return -1;

   } else if (readBytes == -1) {
      if (!d->hasSetSocketError) {
         d->hasSetSocketError = true;
         d->socketError = QAbstractSocket::NetworkError;
         d->socketErrorString = qt_error_string();
      }
      close();

      return -1;
   }

   return readBytes;
}

void QNativeSocketEngine::close()
{
   Q_D(QNativeSocketEngine);
   if (d->readNotifier) {
      d->readNotifier->setEnabled(false);
   }

   if (d->writeNotifier) {
      d->writeNotifier->setEnabled(false);
   }

   if (d->exceptNotifier) {
      d->exceptNotifier->setEnabled(false);
   }

   if (d->socketDescriptor != -1) {
      d->nativeClose();
      d->socketDescriptor = -1;
   }

   d->socketState = QAbstractSocket::UnconnectedState;
   d->hasSetSocketError = false;
   d->localPort = 0;
   d->localAddress.clear();
   d->peerPort = 0;
   d->peerAddress.clear();

   if (d->readNotifier) {
      delete d->readNotifier;
      d->readNotifier = nullptr;
   }

   if (d->writeNotifier) {
      delete d->writeNotifier;
      d->writeNotifier = nullptr;
   }

   if (d->exceptNotifier) {
      delete d->exceptNotifier;
      d->exceptNotifier = nullptr;
   }
}

bool QNativeSocketEngine::waitForRead(int msecs, bool *timedOut)
{
   Q_D(const QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::waitForRead()")) {
      return false;
   }

   Q_CHECK_NOT_STATE(QNativeSocketEngine::waitForRead(), QAbstractSocket::UnconnectedState, false);

   if (timedOut) {
      *timedOut = false;
   }

   int ret = d->nativeSelect(msecs, true);
   if (ret == 0) {
      if (timedOut) {
         *timedOut = true;
      }
      d->setError(QAbstractSocket::SocketTimeoutError, QNativeSocketEnginePrivate::TimeOutErrorString);
      d->hasSetSocketError = false; // A timeout error is temporary in waitFor functions
      return false;

   } else if (state() == QAbstractSocket::ConnectingState) {
      connectToHost(d->peerAddress, d->peerPort);
   }

   return ret > 0;
}

bool QNativeSocketEngine::waitForWrite(int msecs, bool *timedOut)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::waitForWrite()")) {
      return false;
   }

   Q_CHECK_NOT_STATE(QNativeSocketEngine::waitForWrite(), QAbstractSocket::UnconnectedState, false);

   if (timedOut) {
      *timedOut = false;
   }

   int ret = d->nativeSelect(msecs, false);

   // On Windows, the socket is in connected state if a call to select(writable) is successful.
   // In this case we should not // issue a second call to WSAConnect()

#if defined (Q_OS_WIN)

   if (ret > 0) {
      setState(QAbstractSocket::ConnectedState);
      d_func()->fetchConnectionParameters();
      return true;

   } else {
      int value = 0;
      int valueSize = sizeof(value);

      if (::getsockopt(d->socketDescriptor, SOL_SOCKET, SO_ERROR, (char *) &value, &valueSize) == 0) {
         if (value == WSAECONNREFUSED) {
            d->setError(QAbstractSocket::ConnectionRefusedError, QNativeSocketEnginePrivate::ConnectionRefusedErrorString);
            d->socketState = QAbstractSocket::UnconnectedState;
            return false;

         } else if (value == WSAETIMEDOUT) {
            d->setError(QAbstractSocket::NetworkError, QNativeSocketEnginePrivate::ConnectionTimeOutErrorString);
            d->socketState = QAbstractSocket::UnconnectedState;
            return false;

         } else if (value == WSAEHOSTUNREACH) {
            d->setError(QAbstractSocket::NetworkError, QNativeSocketEnginePrivate::HostUnreachableErrorString);
            d->socketState = QAbstractSocket::UnconnectedState;
            return false;
         }
      }
   }
#endif

   if (ret == 0) {
      if (timedOut) {
         *timedOut = true;
      }

      d->setError(QAbstractSocket::SocketTimeoutError, QNativeSocketEnginePrivate::TimeOutErrorString);
      d->hasSetSocketError = false; // A timeout error is temporary in waitFor functions
      return false;

   } else if (state() == QAbstractSocket::ConnectingState || (state() == QAbstractSocket::BoundState && d->socketDescriptor != -1)) {
      connectToHost(d->peerAddress, d->peerPort);
   }

   return ret > 0;
}

bool QNativeSocketEngine::waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
      bool checkRead, bool checkWrite, int msecs, bool *timedOut)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::waitForReadOrWrite()")) {
      return false;
   }

   Q_CHECK_NOT_STATE(QNativeSocketEngine::waitForReadOrWrite(), QAbstractSocket::UnconnectedState, false);

   int ret = d->nativeSelect(msecs, checkRead, checkWrite, readyToRead, readyToWrite);
   // On Windows, the socket is in connected state if a call to
   // select(writable) is successful. In this case we should not
   // issue a second call to WSAConnect()

#if defined (Q_OS_WIN)

   if (checkWrite && ((readyToWrite && *readyToWrite) || !readyToWrite) && ret > 0) {
      setState(QAbstractSocket::ConnectedState);
      d_func()->fetchConnectionParameters();
      return true;

   } else {
      int value = 0;
      int valueSize = sizeof(value);

      if (::getsockopt(d->socketDescriptor, SOL_SOCKET, SO_ERROR, (char *) &value, &valueSize) == 0) {
         if (value == WSAECONNREFUSED) {
            d->setError(QAbstractSocket::ConnectionRefusedError, QNativeSocketEnginePrivate::ConnectionRefusedErrorString);
            d->socketState = QAbstractSocket::UnconnectedState;

            return false;

         } else if (value == WSAETIMEDOUT) {
            d->setError(QAbstractSocket::NetworkError, QNativeSocketEnginePrivate::ConnectionTimeOutErrorString);
            d->socketState = QAbstractSocket::UnconnectedState;

            return false;

         } else if (value == WSAEHOSTUNREACH) {
            d->setError(QAbstractSocket::NetworkError, QNativeSocketEnginePrivate::HostUnreachableErrorString);
            d->socketState = QAbstractSocket::UnconnectedState;

            return false;
         }
      }
   }
#endif

   if (ret == 0) {
      if (timedOut) {
         *timedOut = true;
      }

      d->setError(QAbstractSocket::SocketTimeoutError,  QNativeSocketEnginePrivate::TimeOutErrorString);
      d->hasSetSocketError = false; // A timeout error is temporary in waitFor functions
      return false;

   } else if (state() == QAbstractSocket::ConnectingState) {
      connectToHost(d->peerAddress, d->peerPort);
   }

   return ret > 0;
}

qint64 QNativeSocketEngine::receiveBufferSize() const
{
   Q_D(const QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::receiveBufferSize()")) {
      return -1;
   }

   return option(ReceiveBufferSocketOption);
}

void QNativeSocketEngine::setReceiveBufferSize(qint64 size)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::setReceiveBufferSize()")) {
      return;
   }

   setOption(ReceiveBufferSocketOption, size);
}

qint64 QNativeSocketEngine::sendBufferSize() const
{
   Q_D(const QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::sendBufferSize()")) {
      return -1;
   }

   return option(SendBufferSocketOption);
}

void QNativeSocketEngine::setSendBufferSize(qint64 size)
{
   Q_D(QNativeSocketEngine);

   if (! d->check_valid_socketlayer("QNativeSocketEngine::setSendBufferSize()")) {
      return;
   }

   setOption(SendBufferSocketOption, size);
}

bool QNativeSocketEngine::setOption(SocketOption option, int value)
{
   Q_D(QNativeSocketEngine);
   return d->setOption(option, value);
}

int QNativeSocketEngine::option(SocketOption socketOption) const
{
   Q_D(const QNativeSocketEngine);
   return d->option(socketOption);
}

bool QNativeSocketEngine::isReadNotificationEnabled() const
{
   Q_D(const QNativeSocketEngine);
   return d->readNotifier && d->readNotifier->isEnabled();
}

// internal
class QReadNotifier : public QSocketNotifier
{
 public:
   QReadNotifier(qintptr fd, QNativeSocketEngine *parent)
      : QSocketNotifier(fd, QSocketNotifier::Read, parent) {
      engine = parent;
   }

 protected:
   bool event(QEvent *) override;

   QNativeSocketEngine *engine;
};

bool QReadNotifier::event(QEvent *e)
{
   if (e->type() == QEvent::SockAct) {
      engine->readNotification();
      return true;

   } else if (e->type() == QEvent::SockClose) {
      engine->closeNotification();
      return true;
   }
   return QSocketNotifier::event(e);
}

/*
  \internal
  \class QWriteNotifier
  \brief The QWriteNotifer class is used to improve performance.

  QWriteNotifier is a private class used for performance reasons vs
  connecting to the QSocketNotifier activated() signal.
 */
class QWriteNotifier : public QSocketNotifier
{
 public:
   QWriteNotifier(int fd, QNativeSocketEngine *parent)
      : QSocketNotifier(fd, QSocketNotifier::Write, parent) {
      engine = parent;
   }

 protected:
   bool event(QEvent *) override;

   QNativeSocketEngine *engine;
};

bool QWriteNotifier::event(QEvent *e)
{
   if (e->type() == QEvent::SockAct) {
      if (engine->state() == QAbstractSocket::ConnectingState) {
         engine->connectionNotification();
      } else {
         engine->writeNotification();
      }
      return true;
   }
   return QSocketNotifier::event(e);
}

class QExceptionNotifier : public QSocketNotifier
{
 public:
   QExceptionNotifier(int fd, QNativeSocketEngine *parent)
      : QSocketNotifier(fd, QSocketNotifier::Exception, parent) {
      engine = parent;
   }

 protected:
   bool event(QEvent *) override;

   QNativeSocketEngine *engine;
};

bool QExceptionNotifier::event(QEvent *e)
{
   if (e->type() == QEvent::SockAct) {
      if (engine->state() == QAbstractSocket::ConnectingState) {
         engine->connectionNotification();
      } else {
         engine->exceptionNotification();
      }
      return true;
   }
   return QSocketNotifier::event(e);
}

void QNativeSocketEngine::setReadNotificationEnabled(bool enable)
{
   Q_D(QNativeSocketEngine);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (d->readNotifier) {
      d->readNotifier->setEnabled(enable);

   } else if (enable && threadData->eventDispatcher) {
      d->readNotifier = new QReadNotifier(d->socketDescriptor, this);
      d->readNotifier->setEnabled(true);
   }
}

bool QNativeSocketEngine::isWriteNotificationEnabled() const
{
   Q_D(const QNativeSocketEngine);
   return d->writeNotifier && d->writeNotifier->isEnabled();
}

void QNativeSocketEngine::setWriteNotificationEnabled(bool enable)
{
   Q_D(QNativeSocketEngine);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (d->writeNotifier) {
      d->writeNotifier->setEnabled(enable);

   } else if (enable && threadData->eventDispatcher) {
      d->writeNotifier = new QWriteNotifier(d->socketDescriptor, this);
      d->writeNotifier->setEnabled(true);
   }
}

bool QNativeSocketEngine::isExceptionNotificationEnabled() const
{
   Q_D(const QNativeSocketEngine);
   return d->exceptNotifier && d->exceptNotifier->isEnabled();
}

void QNativeSocketEngine::setExceptionNotificationEnabled(bool enable)
{
   Q_D(QNativeSocketEngine);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (d->exceptNotifier) {
      d->exceptNotifier->setEnabled(enable);

   } else if (enable && threadData->eventDispatcher) {
      d->exceptNotifier = new QExceptionNotifier(d->socketDescriptor, this);
      d->exceptNotifier->setEnabled(true);
   }
}

