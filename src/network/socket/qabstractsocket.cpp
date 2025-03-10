/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qabstractsocket.h>
#include <qabstractsocket_p.h>

#include <qabstracteventdispatcher.h>
#include <qdebug.h>
#include <qelapsedtimer.h>
#include <qhostaddress.h>
#include <qhostinfo.h>
#include <qmetaobject.h>
#include <qpointer.h>
#include <qscopedvaluerollback.h>
#include <qstring.h>
#include <qtimer.h>

#ifdef QT_SSL
#include <qsslsocket.h>
#endif

#include <qhostinfo_p.h>
#include <qnetworksession_p.h>
#include <qthread_p.h>

#include <time.h>

#define Q_CHECK_SOCKETENGINE(returnValue) do { \
    if (! d->socketEngine) { \
        return returnValue; \
    } } while (false)

#ifndef QABSTRACTSOCKET_BUFFERSIZE
#define QABSTRACTSOCKET_BUFFERSIZE 32768
#endif

#define QT_CONNECT_TIMEOUT 30000
#define QT_TRANSFER_TIMEOUT 120000

#if defined(CS_SHOW_DEBUG_NETWORK)

#include <ctype.h>

static QByteArray qt_prettyDebug(const char *data, int len, int maxLength)
{
   if (! data) {
      return "(null)";
   }

   QByteArray out;

   for (int i = 0; i < qMin(len, maxLength); ++i) {
      char c = data[i];

      if (isprint(int(uchar(c)))) {
         out += c;

      } else switch (c) {
            case '\n':
               out += "\\n";
               break;

            case '\r':
               out += "\\r";
               break;

            case '\t':
               out += "\\t";
               break;

            default:
               out += QString("\\%1").formatArgs(c, 8).toLatin1();
      }
   }

   if (len < maxLength) {
      out += "...";
   }

   return out;
}
#endif

static bool isProxyError(QAbstractSocket::SocketError error)
{
   switch (error) {
      case QAbstractSocket::ProxyAuthenticationRequiredError:
      case QAbstractSocket::ProxyConnectionRefusedError:
      case QAbstractSocket::ProxyConnectionClosedError:
      case QAbstractSocket::ProxyConnectionTimeoutError:
      case QAbstractSocket::ProxyNotFoundError:
      case QAbstractSocket::ProxyProtocolError:
         return true;

      default:
         return false;
   }
}

QAbstractSocketPrivate::QAbstractSocketPrivate()
   : readSocketNotifierCalled(false), readSocketNotifierState(false), readSocketNotifierStateSet(false),
     emittedReadyRead(false), emittedBytesWritten(false), abortCalled(false), pendingClose(false),
     pauseMode(QAbstractSocket::PauseNever), port(0), localPort(0), peerPort(0), socketEngine(nullptr),
     cachedSocketDescriptor(-1), readBufferMaxSize(0), writeBuffer(QABSTRACTSOCKET_BUFFERSIZE),
     isBuffered(false), connectTimer(nullptr), disconnectTimer(nullptr), connectTimeElapsed(0),
     hostLookupId(-1), socketType(QAbstractSocket::UnknownSocketType),
     state(QAbstractSocket::UnconnectedState), socketError(QAbstractSocket::UnknownSocketError),
     preferredNetworkLayerProtocol(QAbstractSocket::UnknownNetworkLayerProtocol)
{
}

QAbstractSocketPrivate::~QAbstractSocketPrivate()
{
}

void QAbstractSocketPrivate::resetSocketLayer()
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::resetSocketLayer()");
#endif

   if (socketEngine) {
      socketEngine->close();
      socketEngine->disconnect();
      delete socketEngine;
      socketEngine = nullptr;
      cachedSocketDescriptor = -1;
   }

   if (connectTimer) {
      connectTimer->stop();
   }

   if (disconnectTimer) {
      disconnectTimer->stop();
   }
}

bool QAbstractSocketPrivate::initSocketLayer(QAbstractSocket::NetworkLayerProtocol protocol)
{

#ifdef QT_NO_NETWORKPROXY
   // here to avoid a duplication of the call to createSocketEngine below
   static constexpr const QNetworkProxy &proxyInUse = *(QNetworkProxy *)0;
#endif

   Q_Q(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   QString typeStr;

   if (q->socketType() == QAbstractSocket::TcpSocket) {
      typeStr = "TcpSocket";

   } else if (q->socketType() == QAbstractSocket::UdpSocket) {
      typeStr = "UdpSocket";

   } else {
      typeStr = "UnknownSocketType";

   }

   QString protocolStr;

   if (protocol == QAbstractSocket::IPv4Protocol) {
      protocolStr = "IPv4Protocol";

   } else if (protocol == QAbstractSocket::IPv6Protocol) {
      protocolStr = "IPv6Protocol";

   } else {
      protocolStr = "UnknownNetworkLayerProtocol";
   }
#endif

   resetSocketLayer();
   socketEngine = QAbstractSocketEngine::createSocketEngine(q->socketType(), proxyInUse, q);

   if (! socketEngine) {
      socketError = QAbstractSocket::UnsupportedSocketOperationError;
      q->setErrorString(QAbstractSocket::tr("Operation on socket is not supported"));
      return false;
   }

#ifndef QT_NO_BEARERMANAGEMENT
   // copy network session down to the socket engine (if it has been set)
   socketEngine->setProperty("_q_networksession", q->property("_q_networksession"));
#endif

   if (! socketEngine->initialize(q->socketType(), protocol)) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocketPrivate::initSocketLayer(%s, %s) Failed (%s)",
            csPrintable(typeStr), csPrintable(protocolStr), csPrintable(socketEngine->errorString()));
#endif

      socketError = socketEngine->error();
      q->setErrorString(socketEngine->errorString());

      return false;
   }

   configureCreatedSocket();
   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);

   if (threadData->eventDispatcher) {
      socketEngine->setReceiver(this);
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::initSocketLayer(%s, %s) Success",
         csPrintable(typeStr), csPrintable(protocolStr));
#endif
   return true;
}

void QAbstractSocketPrivate::configureCreatedSocket()
{
}

bool QAbstractSocketPrivate::canReadNotification()
{
   Q_Q(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::canReadNotification()");
#endif

   // prevent recursive calls
   if (readSocketNotifierCalled) {
      if (! readSocketNotifierStateSet) {
         readSocketNotifierStateSet = true;
         readSocketNotifierState = socketEngine->isReadNotificationEnabled();
         socketEngine->setReadNotificationEnabled(false);
      }
   }

   QScopedValueRollback<bool> rsncrollback(readSocketNotifierCalled);
   readSocketNotifierCalled = true;

   if (!isBuffered) {
      socketEngine->setReadNotificationEnabled(false);
   }

   // If buffered, read data from the socket into the read buffer
   qint64 newBytes = 0;

   if (isBuffered) {
      // Return if there is no space in the buffer
      if (readBufferMaxSize && buffer.size() >= readBufferMaxSize) {

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocketPrivate::canReadNotification() Buffer is full");
#endif
         return false;
      }

      // If reading from the socket fails after getting a read notification, close the socket.
      newBytes = buffer.size();

      if (! readFromSocket()) {
#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocketPrivate::canReadNotification() Disconnecting socket");
#endif
         q->disconnectFromHost();
         return false;
      }

      newBytes = buffer.size() - newBytes;

      // if read buffer is full, disable the read socket notifier
      if (readBufferMaxSize && buffer.size() == readBufferMaxSize) {
         socketEngine->setReadNotificationEnabled(false);
      }
   }

   // only emit readyRead() when not recursing and only if there is data available
   bool hasData;

#ifndef QT_NO_UDPSOCKET
   hasData = newBytes > 0 || (! isBuffered && socketType != QAbstractSocket::TcpSocket &&
         socketEngine && socketEngine->hasPendingDatagrams()) ||
         (! isBuffered && socketType == QAbstractSocket::TcpSocket && socketEngine);
#else
   hasData = newBytes > 0 || (! isBuffered && socketType == QAbstractSocket::TcpSocket && socketEngine);

#endif

   if (! emittedReadyRead && hasData) {
      QScopedValueRollback<bool> r(emittedReadyRead);
      emittedReadyRead = true;
      emit q->readyRead();
   }

   // if we were closed as a result of the readyRead() signal, return.
   if (state == QAbstractSocket::UnconnectedState || state == QAbstractSocket::ClosingState) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocketPrivate::canReadNotification() Socket is closing, returning");
#endif

      return true;
   }

   if (socketEngine && isBuffered) {
      socketEngine->setReadNotificationEnabled(readBufferMaxSize == 0 || readBufferMaxSize > q->bytesAvailable());
   }

   // reset the read socket notifier state if we reentered inside the
   // readyRead() connected slot
   if (readSocketNotifierStateSet && socketEngine &&
         readSocketNotifierState != socketEngine->isReadNotificationEnabled()) {
      socketEngine->setReadNotificationEnabled(readSocketNotifierState);
      readSocketNotifierStateSet = false;
   }

   return true;
}

void QAbstractSocketPrivate::canCloseNotification()
{
   Q_Q(QAbstractSocket);

   // this method is only called on Windows, other platforms close in the canReadNotification()

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::canCloseNotification()");
#endif

   qint64 newBytes = 0;

   if (isBuffered) {
      // Try to read to the buffer, if the read fail we can close the socket.
      newBytes = buffer.size();

      qint64 oldReadBufferMaxSize = readBufferMaxSize;
      readBufferMaxSize = 0; // temporarily disable max read buffer, we want to empty the OS buffer

      bool hadReadFromSocket = readFromSocket();
      readBufferMaxSize = oldReadBufferMaxSize;

      if (!hadReadFromSocket) {
         q->disconnectFromHost();
         return;
      }

      newBytes = buffer.size() - newBytes;

      if (newBytes) {
         // If there was still some data to be read from the socket
         // then we could get another FD_READ. The disconnect will
         // then occur when we read from the socket again and fail
         // in canReadNotification or by the manually created
         // closeNotification below.
         emit q->readyRead();

         QMetaObject::invokeMethod(socketEngine, "closeNotification", Qt::QueuedConnection);
      }

   } else if (socketType == QAbstractSocket::TcpSocket && socketEngine) {
      emit q->readyRead();
   }
}

bool QAbstractSocketPrivate::canWriteNotification()
{
#if defined (Q_OS_WIN)
   if (socketEngine && socketEngine->isWriteNotificationEnabled()) {
      socketEngine->setWriteNotificationEnabled(false);
   }
#endif

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::canWriteNotification() Flushing");
#endif

   qint64 tmp = writeBuffer.size();
   flush();

   if (socketEngine) {

#if defined (Q_OS_WIN)
      if (!writeBuffer.isEmpty()) {
         socketEngine->setWriteNotificationEnabled(true);
      }
#else
      if (writeBuffer.isEmpty() && socketEngine->bytesToWrite() == 0) {
         socketEngine->setWriteNotificationEnabled(false);
      }
#endif
   }

   return (writeBuffer.size() < tmp);
}

void QAbstractSocketPrivate::connectionNotification()
{
   // If in connecting state, check if the connection has been
   // established, otherwise flush pending data

   if (state == QAbstractSocket::ConnectingState) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocketPrivate::connectionNotification() Testing connection");
#endif

      _q_testConnection();
   }
}

bool QAbstractSocketPrivate::flush()
{
   Q_Q(QAbstractSocket);

   if (! socketEngine || ! socketEngine->isValid() || (writeBuffer.isEmpty()
         && socketEngine->bytesToWrite() == 0)) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocketPrivate::flush() Nothing to do: valid ? %s, writeBuffer.isEmpty() ? %s",
             (socketEngine && socketEngine->isValid()) ? "yes" : "no", writeBuffer.isEmpty() ? "yes" : "no");
#endif

      // this covers the case when the buffer was empty, but we had to wait for the socket engine to finish
      if (state == QAbstractSocket::ClosingState) {
         q->disconnectFromHost();
      }

      return false;
   }

   qint64 nextSize = writeBuffer.nextDataBlockSize();
   const char *ptr = writeBuffer.readPointer();

   // Attempt to write it all in one chunk.
   qint64 written = nextSize ? socketEngine->write(ptr, nextSize) : Q_INT64_C(0);

   if (written < 0) {
      socketError = socketEngine->error();
      q->setErrorString(socketEngine->errorString());

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug() << "QAbstractSocketPrivate::flush() Write error, aborting" << socketEngine->errorString();
#endif

      emit q->error(socketError);
      // an unexpected error so close the socket.
      q->abort();
      return false;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::flush() %lld bytes written to the network", written);
#endif

   // remove what we wrote so far
   writeBuffer.free(written);

   if (written > 0) {
      // Do not emit bytesWritten() recursively.
      if (! emittedBytesWritten) {
         QScopedValueRollback<bool> r(emittedBytesWritten);
         emittedBytesWritten = true;
         emit q->bytesWritten(written);
      }
   }

   if (writeBuffer.isEmpty() && socketEngine && socketEngine->isWriteNotificationEnabled()
         && ! socketEngine->bytesToWrite()) {
      socketEngine->setWriteNotificationEnabled(false);
   }

   if (state == QAbstractSocket::ClosingState) {
      q->disconnectFromHost();
   }

   return true;
}

#ifndef QT_NO_NETWORKPROXY

void QAbstractSocketPrivate::resolveProxy(const QString &hostname, quint16 newPort)
{
   QList<QNetworkProxy> proxies;

   if (proxy.type() != QNetworkProxy::DefaultProxy) {
      // a non-default proxy was set with setProxy
      proxies << proxy;
   } else {
      // try the application settings instead
      QNetworkProxyQuery query(hostname, newPort, QString(),
            socketType == QAbstractSocket::TcpSocket ?
            QNetworkProxyQuery::TcpSocket : QNetworkProxyQuery::UdpSocket);

      proxies = QNetworkProxyFactory::proxyForQuery(query);
   }

   // return the first that we can use
   for (const QNetworkProxy &p : proxies) {
      if (socketType == QAbstractSocket::UdpSocket &&
            (p.capabilities() & QNetworkProxy::UdpTunnelingCapability) == 0) {
         continue;
      }

      if (socketType == QAbstractSocket::TcpSocket &&
            (p.capabilities() & QNetworkProxy::TunnelingCapability) == 0) {
         continue;
      }

      proxyInUse = p;
      return;
   }

   // no proxy found, DefaultProxy will raise an error
   proxyInUse = QNetworkProxy();
}

 void QAbstractSocketPrivate::startConnectingByName(const QString &newHost)
{
   Q_Q(QAbstractSocket);

   if (state == QAbstractSocket::ConnectingState || state == QAbstractSocket::ConnectedState) {
      return;
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::startConnectingByName(host == %s)", csPrintable(newHost));
#endif

   // ### Let the socket engine drive this?
   state = QAbstractSocket::ConnectingState;
   emit q->stateChanged(state);

   connectTimeElapsed = 0;

   if (cachedSocketDescriptor != -1 || initSocketLayer(QAbstractSocket::UnknownNetworkLayerProtocol)) {
      if (socketEngine->connectToHostByName(newHost, port) ||
            socketEngine->state() == QAbstractSocket::ConnectingState) {
         cachedSocketDescriptor = socketEngine->socketDescriptor();

         return;
      }

      // failed to connect
      socketError = socketEngine->error();
      q->setErrorString(socketEngine->errorString());
   }

   state = QAbstractSocket::UnconnectedState;
   emit q->error(socketError);
   emit q->stateChanged(state);
}

#endif

void QAbstractSocketPrivate::_q_startConnecting(const QHostInfo &hostInfo)
{
   Q_Q(QAbstractSocket);

   addresses.clear();

   if (state != QAbstractSocket::HostLookupState) {
      return;
   }

   if (hostLookupId != -1 && hostLookupId != hostInfo.lookupId()) {
      qWarning("QAbstractSocket::_q_startConnecting() Received hostInfo for lookup ID %d, expected %d",
            hostInfo.lookupId(), hostLookupId);
   }

   if (preferredNetworkLayerProtocol == QAbstractSocket::UnknownNetworkLayerProtocol ||
         preferredNetworkLayerProtocol == QAbstractSocket::AnyIPProtocol) {
      addresses = hostInfo.addresses();

   } else {
      for (const QHostAddress &address : hostInfo.addresses()) {
         if (address.protocol() == preferredNetworkLayerProtocol) {
            addresses += address;
         }
      }
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   QString s = "{";

   for (int i = 0; i < addresses.count(); ++i) {
      if (i != 0) {
         s += ", ";
      }

      s += addresses.at(i).toString();
   }

   s += '}';

   qDebug("QAbstractSocketPrivate::_q_startConnecting(hostInfo == %s)", csPrintable(s));
#endif

   // Try all addresses twice
   addresses += addresses;

   // If there are no addresses in the host list, report this to the user

   if (addresses.isEmpty()) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocketPrivate::_q_startConnecting() Host not found");
#endif

      state = QAbstractSocket::UnconnectedState;
      socketError = QAbstractSocket::HostNotFoundError;
      q->setErrorString(QAbstractSocket::tr("Host not found"));
      emit q->stateChanged(state);
      emit q->error(QAbstractSocket::HostNotFoundError);

      return;
   }

   // Enter Connecting state (see also sn_write, which is called by
   // the write socket notifier after connect())
   state = QAbstractSocket::ConnectingState;
   emit q->stateChanged(state);

   // Report the successful host lookup
   emit q->hostFound();

   // Reset the total time spent connecting.
   connectTimeElapsed = 0;

   // The addresses returned by the lookup will be tested one after
   // another by _q_connectToNextAddress().
   _q_connectToNextAddress();
}

void QAbstractSocketPrivate::_q_connectToNextAddress()
{
   Q_Q(QAbstractSocket);

   do {
      // Check for more pending addresses
      if (addresses.isEmpty()) {

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocketPrivate::_q_connectToNextAddress() All addresses failed");
#endif

         state = QAbstractSocket::UnconnectedState;

         if (socketEngine) {
            if ((socketEngine->error() == QAbstractSocket::UnknownSocketError) &&
                  (socketEngine->state() == QAbstractSocket::ConnectingState)) {

               socketError = QAbstractSocket::ConnectionRefusedError;
               q->setErrorString(QAbstractSocket::tr("Connection refused"));

            } else {
               socketError = socketEngine->error();
               q->setErrorString(socketEngine->errorString());
            }

         } else {
            //  socketError = QAbstractSocket::ConnectionRefusedError;
            //  q->setErrorString(QAbstractSocket::tr("Connection refused"));
         }

         emit q->stateChanged(state);
         emit q->error(socketError);
         return;
      }

      // Pick the first host address candidate
      host = addresses.takeFirst();

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocketPrivate::_q_connectToNextAddress() Connecting to %s:%i, %lld left to try",
             csPrintable(host.toString()), port, addresses.count());
#endif

      if (cachedSocketDescriptor == -1 && !initSocketLayer(host.protocol())) {
         // hope the next address is better

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocketPrivate::_q_connectToNextAddress() Failed to initialize sock layer");
#endif
         continue;
      }

      // Tries to connect to the address. If it succeeds immediately
      // (localhost address on BSD or any UDP connect), emit connected() and return

      if (socketEngine->connectToHost(host, port)) {
         //_q_testConnection();
         fetchConnectionParameters();
         return;
      }

      // Check that we are in delayed connection state. If not, try the next address
      if (socketEngine->state() != QAbstractSocket::ConnectingState) {

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocketPrivate::_q_connectToNextAddress() Connection failed (%s)",
                csPrintable(socketEngine->errorString()));
#endif
         continue;
      }

      // Start the connect timer
      QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);

      if (threadData->eventDispatcher) {
         if (! connectTimer) {
            connectTimer = new QTimer(q);
            QObject::connect(connectTimer, &QTimer::timeout,
                  q, &QAbstractSocket::_q_abortConnectionAttempt, Qt::DirectConnection);
         }

         connectTimer->start(QT_CONNECT_TIMEOUT);
      }

      // Wait for a write notification that will eventually call
      // _q_testConnection()
      socketEngine->setWriteNotificationEnabled(true);
      break;

   } while (state != QAbstractSocket::ConnectedState);
}

void QAbstractSocketPrivate::_q_testConnection()
{
   Q_Q(QAbstractSocket);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(q);

   if (socketEngine) {
      if (threadData->eventDispatcher) {
         if (connectTimer) {
            connectTimer->stop();
         }
      }

      if (socketEngine->state() == QAbstractSocket::ConnectedState) {
         // Fetch the parameters if our connection is completed;
         // otherwise, fall out and try the next address.
         fetchConnectionParameters();

         if (pendingClose) {
            q_func()->disconnectFromHost();
            pendingClose = false;
         }
         return;
      }

      // do not retry the other addresses if we had a proxy error
      if (isProxyError(socketEngine->error())) {
         addresses.clear();
      }
   }

   if (threadData->eventDispatcher) {
      if (connectTimer) {
         connectTimer->stop();
      }
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::_q_testConnection() Connection failed, checking for alternative addresses");
#endif

   _q_connectToNextAddress();
}

void QAbstractSocketPrivate::_q_abortConnectionAttempt()
{
   Q_Q(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::_q_abortConnectionAttempt() Timed out");
#endif

   if (socketEngine) {
      socketEngine->setWriteNotificationEnabled(false);
   }

   connectTimer->stop();

   if (addresses.isEmpty()) {
      state = QAbstractSocket::UnconnectedState;
      socketError = QAbstractSocket::SocketTimeoutError;
      q->setErrorString(QAbstractSocket::tr("Connection timed out"));
      emit q->stateChanged(state);
      emit q->error(socketError);
   } else {
      _q_connectToNextAddress();
   }
}

void QAbstractSocketPrivate::_q_forceDisconnect()
{
   Q_Q(QAbstractSocket);

   if (socketEngine && socketEngine->isValid() && state == QAbstractSocket::ClosingState) {
      socketEngine->close();
      q->disconnectFromHost();
   }
}

bool QAbstractSocketPrivate::readFromSocket()
{
   Q_Q(QAbstractSocket);

   // Find how many bytes we can read from the socket layer.
   qint64 bytesToRead = socketEngine->bytesAvailable();

   if (bytesToRead == 0) {
      // Under heavy load, certain conditions can trigger read notifications
      // for socket notifiers on which there is no activity. If we continue
      // to read 0 bytes from the socket, we will trigger behavior similar
      // to that which signals a remote close. When we hit this condition,
      // we try to read 4k of data from the socket, which will give us either
      // an EAGAIN/EWOULDBLOCK if the connection is alive (i.e., the remote
      // host has _not_ disappeared).
      bytesToRead = 4096;
   }

   if (readBufferMaxSize && bytesToRead > (readBufferMaxSize - buffer.size())) {
      bytesToRead = readBufferMaxSize - buffer.size();
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::readFromSocket() About to read %d bytes", int(bytesToRead));
#endif

   // Read from the socket, store data in the read buffer.
   char *ptr = buffer.reserve(bytesToRead);
   qint64 readBytes = socketEngine->read(ptr, bytesToRead);

   if (readBytes == -2) {
      // No bytes currently available for reading.
      buffer.chop(bytesToRead);
      return true;
   }

   buffer.chop(bytesToRead - (readBytes < 0 ? qint64(0) : readBytes));

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::readFromSocket() Read %lld bytes, buffer size = %d",
         readBytes, buffer.size());
#endif

   if (! socketEngine->isValid()) {
      socketError = socketEngine->error();
      q->setErrorString(socketEngine->errorString());
      emit q->error(socketError);

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocketPrivate::readFromSocket() Fead failed: %s",
            q->errorString().toLatin1().constData());
#endif
      resetSocketLayer();
      return false;
   }

   return true;
}

void QAbstractSocketPrivate::fetchConnectionParameters()
{
   Q_Q(QAbstractSocket);

   peerName = hostName;

   if (socketEngine) {
      socketEngine->setReadNotificationEnabled(true);
      socketEngine->setWriteNotificationEnabled(true);
      localPort = socketEngine->localPort();
      peerPort = socketEngine->peerPort();
      localAddress = socketEngine->localAddress();
      peerAddress = socketEngine->peerAddress();
      cachedSocketDescriptor = socketEngine->socketDescriptor();
   }

   state = QAbstractSocket::ConnectedState;
   emit q->stateChanged(state);
   emit q->connected();

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocketPrivate::fetchConnectionParameters() Connection to %s:%i established",
          csPrintable(host.toString()), port);
#endif
}

void QAbstractSocketPrivate::pauseSocketNotifiers(QAbstractSocket *socket)
{
   QAbstractSocketEngine *socketEngine = socket->d_func()->socketEngine;

   if (! socketEngine) {
      return;
   }

   socket->d_func()->prePauseReadSocketNotifierState      = socketEngine->isReadNotificationEnabled();
   socket->d_func()->prePauseWriteSocketNotifierState     = socketEngine->isWriteNotificationEnabled();
   socket->d_func()->prePauseExceptionSocketNotifierState = socketEngine->isExceptionNotificationEnabled();

   socketEngine->setReadNotificationEnabled(false);
   socketEngine->setWriteNotificationEnabled(false);
   socketEngine->setExceptionNotificationEnabled(false);
}

void QAbstractSocketPrivate::resumeSocketNotifiers(QAbstractSocket *socket)
{
   QAbstractSocketEngine *socketEngine = socket->d_func()->socketEngine;

   if (! socketEngine) {
      return;
   }

   socketEngine->setReadNotificationEnabled(socket->d_func()->prePauseReadSocketNotifierState);
   socketEngine->setWriteNotificationEnabled(socket->d_func()->prePauseWriteSocketNotifierState);
   socketEngine->setExceptionNotificationEnabled(socket->d_func()->prePauseExceptionSocketNotifierState);
}

QAbstractSocketEngine *QAbstractSocketPrivate::getSocketEngine(QAbstractSocket *socket)
{
   return socket->d_func()->socketEngine;
}

void QAbstractSocketPrivate::setError(QAbstractSocket::SocketError errorCode, const QString &errStr)
{
   socketError = errorCode;
   errorString = errStr;
}

void QAbstractSocketPrivate::setErrorAndEmit(QAbstractSocket::SocketError errorCode, const QString &errorMsg)
{
   Q_Q(QAbstractSocket);
   setError(errorCode, errorMsg);
   emit q->error(errorCode);
}

QAbstractSocket::QAbstractSocket(SocketType socketType, QAbstractSocketPrivate &dd, QObject *parent)
   : QIODevice(dd, parent)
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::QAbstractSocket(%sSocket, QAbstractSocketPrivate == %p, parent == %p)",
          socketType == TcpSocket ? "Tcp" : socketType == UdpSocket ? "Udp" : "Unknown", &dd, parent);
#endif

   d->socketType = socketType;
}

QAbstractSocket::QAbstractSocket(SocketType socketType, QObject *parent)
   : QIODevice(*new QAbstractSocketPrivate, parent)
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::QAbstractSocket(%p)", parent);
#endif

   d->socketType = socketType;
}

QAbstractSocket::~QAbstractSocket()
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::~QAbstractSocket()");
#endif

   if (d->state != UnconnectedState) {
      abort();
   }
}

void QAbstractSocket::resume()
{
   QAbstractSocketPrivate::resumeSocketNotifiers(this);
}

QAbstractSocket::PauseModes QAbstractSocket::pauseMode() const
{
   return d_func()->pauseMode;
}

void QAbstractSocket::setPauseMode(PauseModes pauseMode)
{
   d_func()->pauseMode = pauseMode;
}

bool QAbstractSocket::bind(const QHostAddress &address, quint16 port, BindMode mode)
{
   Q_D(QAbstractSocket);
   return d->bind(address, port, mode);
}

bool QAbstractSocketPrivate::bind(const QHostAddress &address, quint16 newPort, QAbstractSocket::BindMode mode)
{
   Q_Q(QAbstractSocket);

   // now check if the socket engine is initialized and to the right type
   if (! socketEngine || ! socketEngine->isValid()) {
      QHostAddress nullAddress;
      resolveProxy(nullAddress.toString(), newPort);

      QAbstractSocket::NetworkLayerProtocol protocol = address.protocol();
      if (protocol == QAbstractSocket::UnknownNetworkLayerProtocol) {
         protocol = nullAddress.protocol();
      }

      if (!initSocketLayer(protocol)) {
         return false;
      }
   }

   if (mode != QAbstractSocket::DefaultForPlatform) {

#ifdef Q_OS_UNIX
      if ((mode & QAbstractSocket::ShareAddress) || (mode & QAbstractSocket::ReuseAddressHint)) {
         socketEngine->setOption(QAbstractSocketEngine::AddressReusable, 1);
      } else {
         socketEngine->setOption(QAbstractSocketEngine::AddressReusable, 0);
      }
#endif

#ifdef Q_OS_WIN
      if (mode & QAbstractSocket::ReuseAddressHint) {
         socketEngine->setOption(QAbstractSocketEngine::AddressReusable, 1);
      } else {
         socketEngine->setOption(QAbstractSocketEngine::AddressReusable, 0);
      }
      if (mode & QAbstractSocket::DontShareAddress) {
         socketEngine->setOption(QAbstractSocketEngine::BindExclusively, 1);
      } else {
         socketEngine->setOption(QAbstractSocketEngine::BindExclusively, 0);
      }
#endif

   }

   bool result = socketEngine->bind(address, newPort);
   cachedSocketDescriptor = socketEngine->socketDescriptor();

   if (! result) {
      setErrorAndEmit(socketEngine->error(), socketEngine->errorString());
      return false;
   }

   state = QAbstractSocket::BoundState;
   localAddress = socketEngine->localAddress();
   localPort = socketEngine->localPort();

   emit q->stateChanged(state);
   if (socketType == QAbstractSocket::UdpSocket) {
      socketEngine->setReadNotificationEnabled(true);
   }

   return true;
}

bool QAbstractSocket::bind(quint16 port, BindMode mode)
{
   return bind(QHostAddress::Any, port, mode);
}

bool QAbstractSocket::isValid() const
{
   return d_func()->socketEngine ? d_func()->socketEngine->isValid() : isOpen();
}

void QAbstractSocket::connectToHost(const QString &hostName, quint16 port,
      OpenMode openMode, NetworkLayerProtocol protocol)
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::connectToHost(\"%s\", %i, %i)", csPrintable(hostName), port, (int) openMode);
#endif

   if (d->state == ConnectedState || d->state == ConnectingState
         || d->state == ClosingState || d->state == HostLookupState) {

      qWarning("QAbstractSocket::connectToHost() Called when looking up or connecting to \"%s\"", csPrintable(hostName));

      d->setErrorAndEmit(OperationError, tr("Trying to connect while connection is in progress"));
      return;
   }

   d->preferredNetworkLayerProtocol = protocol;
   d->hostName = hostName;
   d->port = port;
   d->buffer.clear();
   d->writeBuffer.clear();
   d->abortCalled = false;
   d->pendingClose = false;

   if (d->state != BoundState) {
      d->state = UnconnectedState;
      d->localPort = 0;
      d->localAddress.clear();
   }

   d->peerPort = 0;
   d->peerAddress.clear();
   d->peerName = hostName;

   if (d->hostLookupId != -1) {
      QHostInfo::abortHostLookup(d->hostLookupId);
      d->hostLookupId = -1;
   }

#ifndef QT_NO_NETWORKPROXY
   // Get the proxy information
   d->resolveProxy(hostName, port);

   if (d->proxyInUse.type() == QNetworkProxy::DefaultProxy) {
      // failed to setup the proxy
      d->socketError = QAbstractSocket::UnsupportedSocketOperationError;
      setErrorString(QAbstractSocket::tr("Operation on socket is not supported"));
      emit error(d->socketError);

      return;
   }
#endif

   if (openMode & QIODevice::Unbuffered) {
      d->isBuffered = false;   // Unbuffered QTcpSocket

   } else if (!d_func()->isBuffered) {
      openMode |= QAbstractSocket::Unbuffered;   // QUdpSocket

   }

   QIODevice::open(openMode);
   d->state = HostLookupState;
   emit stateChanged(d->state);

   QHostAddress temp;
   if (temp.setAddress(hostName)) {
      QHostInfo info;
      info.setAddresses(QList<QHostAddress>() << temp);
      d->_q_startConnecting(info);

#ifndef QT_NO_NETWORKPROXY
   } else if (d->proxyInUse.capabilities() & QNetworkProxy::HostNameLookupCapability) {
      // proxy supports connection by name, so use it
      d->startConnectingByName(hostName);
      return;
#endif

   } else {
      QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

      if (threadData->eventDispatcher) {
         // QHostInfo either returns requestedd QHostInfo from cache or calls q_startConnecting() slot later on
         bool immediateResultValid = false;

         QHostInfo hostInfo = qt_qhostinfo_lookup(hostName, this, SLOT(_q_startConnecting(const QHostInfo &)),
               &immediateResultValid, &d->hostLookupId);

         if (immediateResultValid) {
            d->hostLookupId = -1;
            d->_q_startConnecting(hostInfo);
         }
      }
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::connectToHost(\"%s\", %i) == %s%s", csPrintable(hostName), port,
          (d->state == ConnectedState) ? "true" : "false",
          (d->state == ConnectingState || d->state == HostLookupState)
          ? " (connection in progress)" : "");
#endif
}

void QAbstractSocket::connectToHost(const QHostAddress &address, quint16 port, OpenMode openMode)
{
#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::connectToHost([%s], %i, %i)",
          csPrintable(address.toString()), port, (int) openMode);
#endif

   connectToHost(address.toString(), port, openMode);
}

qint64 QAbstractSocket::bytesToWrite() const
{
   Q_D(const QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::bytesToWrite() == %d", d->writeBuffer.size());
#endif

   return d->writeBuffer.size();
}

qint64 QAbstractSocket::bytesAvailable() const
{
   Q_D(const QAbstractSocket);
   qint64 available = QIODevice::bytesAvailable();

   if (! d->isBuffered && d->socketEngine && d->socketEngine->isValid()) {
      available += d->socketEngine->bytesAvailable();
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::bytesAvailable() == %lld", available);
#endif

   return available;
}

quint16 QAbstractSocket::localPort() const
{
   Q_D(const QAbstractSocket);
   return d->localPort;
}

QHostAddress QAbstractSocket::localAddress() const
{
   Q_D(const QAbstractSocket);
   return d->localAddress;
}

quint16 QAbstractSocket::peerPort() const
{
   Q_D(const QAbstractSocket);
   return d->peerPort;
}

QHostAddress QAbstractSocket::peerAddress() const
{
   Q_D(const QAbstractSocket);
   return d->peerAddress;
}

QString QAbstractSocket::peerName() const
{
   Q_D(const QAbstractSocket);
   return d->peerName.isEmpty() ? d->hostName : d->peerName;
}

bool QAbstractSocket::canReadLine() const
{
   bool hasLine = QIODevice::canReadLine();

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::canReadLine() == %s, buffer size = %d, size = %d",
          hasLine ? "true" : "false", d_func()->buffer.size(), d_func()->buffer.size());
#endif

   return hasLine;
}

qintptr QAbstractSocket::socketDescriptor() const
{
   Q_D(const QAbstractSocket);
   return d->cachedSocketDescriptor;
}

bool QAbstractSocket::setSocketDescriptor(qintptr socketDescriptor, SocketState socketState, OpenMode openMode)
{
   Q_D(QAbstractSocket);

   d->resetSocketLayer();
   d->writeBuffer.clear();
   d->buffer.clear();

   d->socketEngine = QAbstractSocketEngine::createSocketEngine(socketDescriptor, this);

   if (! d->socketEngine) {
      d->socketError = UnsupportedSocketOperationError;
      setErrorString(tr("Operation on socket is not supported"));
      return false;
   }

#ifndef QT_NO_BEARERMANAGEMENT
   // copy network session down to the socket engine (if it has been set)
   d->socketEngine->setProperty("_q_networksession", property("_q_networksession"));
#endif

   bool result = d->socketEngine->initialize(socketDescriptor, socketState);
   if (!result) {
      d->socketError = d->socketEngine->error();
      setErrorString(d->socketEngine->errorString());
      return false;
   }

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (threadData->eventDispatcher) {
      d->socketEngine->setReceiver(d);
   }

   QIODevice::open(openMode);

   if (d->state != socketState) {
      d->state = socketState;
      emit stateChanged(d->state);
   }

   d->pendingClose = false;
   d->socketEngine->setReadNotificationEnabled(true);
   d->localPort = d->socketEngine->localPort();
   d->peerPort = d->socketEngine->peerPort();
   d->localAddress = d->socketEngine->localAddress();
   d->peerAddress = d->socketEngine->peerAddress();
   d->cachedSocketDescriptor = socketDescriptor;

   return true;
}

void QAbstractSocket::setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)
{
   if (! d_func()->socketEngine) {
      return;
   }

   switch (option) {
      case LowDelayOption:
         d_func()->socketEngine->setOption(QAbstractSocketEngine::LowDelayOption, value.toInt());
         break;

      case KeepAliveOption:
         d_func()->socketEngine->setOption(QAbstractSocketEngine::KeepAliveOption, value.toInt());
         break;

      case MulticastTtlOption:
         d_func()->socketEngine->setOption(QAbstractSocketEngine::MulticastTtlOption, value.toInt());
         break;

      case MulticastLoopbackOption:
         d_func()->socketEngine->setOption(QAbstractSocketEngine::MulticastLoopbackOption, value.toInt());
         break;

      case TypeOfServiceOption:
         d_func()->socketEngine->setOption(QAbstractSocketEngine::TypeOfServiceOption, value.toInt());
         break;

      case SendBufferSizeSocketOption:
         d_func()->socketEngine->setOption(QAbstractSocketEngine::SendBufferSocketOption, value.toInt());
         break;

      case ReceiveBufferSizeSocketOption:
         d_func()->socketEngine->setOption(QAbstractSocketEngine::ReceiveBufferSocketOption, value.toInt());
         break;
   }
}

QVariant QAbstractSocket::socketOption(QAbstractSocket::SocketOption option)
{
   if (! d_func()->socketEngine) {
      return QVariant();
   }

   int ret = -1;

   switch (option) {
      case LowDelayOption:
         ret = d_func()->socketEngine->option(QAbstractSocketEngine::LowDelayOption);
         break;

      case KeepAliveOption:
         ret = d_func()->socketEngine->option(QAbstractSocketEngine::KeepAliveOption);
         break;

      case MulticastTtlOption:
         ret = d_func()->socketEngine->option(QAbstractSocketEngine::MulticastTtlOption);
         break;

      case MulticastLoopbackOption:
         ret = d_func()->socketEngine->option(QAbstractSocketEngine::MulticastLoopbackOption);
         break;

      case TypeOfServiceOption:
         ret = d_func()->socketEngine->option(QAbstractSocketEngine::TypeOfServiceOption);
         break;

      case SendBufferSizeSocketOption:
         ret = d_func()->socketEngine->option(QAbstractSocketEngine::SendBufferSocketOption);
         break;

      case ReceiveBufferSizeSocketOption:
         ret = d_func()->socketEngine->option(QAbstractSocketEngine::ReceiveBufferSocketOption);
         break;
   }

   if (ret == -1) {
      return QVariant();
   } else {
      return QVariant(ret);
   }
}

bool QAbstractSocket::waitForConnected(int msecs)
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::waitForConnected(%i)", msecs);
#endif

   if (state() == ConnectedState) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocket::waitForConnected(%i) Already connected", msecs);
#endif
      return true;
   }

   bool wasPendingClose = d->pendingClose;
   d->pendingClose = false;
   QElapsedTimer stopWatch;
   stopWatch.start();

   if (d->state == HostLookupState) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocket::waitForConnected(%i) Doing a host name lookup", msecs);
#endif

      QHostInfo::abortHostLookup(d->hostLookupId);
      d->hostLookupId = -1;

#ifndef QT_NO_BEARERMANAGEMENT
      QSharedPointer<QNetworkSession> networkSession;
      QVariant v(property("_q_networksession"));

      if (v.isValid()) {
         networkSession = v.value<QSharedPointer<QNetworkSession>>();
         d->_q_startConnecting(QHostInfoPrivate::fromName(d->hostName, networkSession));
      } else
#endif

      {
         QHostAddress temp;
         if (temp.setAddress(d->hostName)) {
            QHostInfo info;
            info.setAddresses(QList<QHostAddress>() << temp);
            d->_q_startConnecting(info);

         } else {
            d->_q_startConnecting(QHostInfo::fromName(d->hostName));
         }
      }
   }

   if (state() == UnconnectedState) {
      return false;   // connect not im progress anymore!
   }

   bool timedOut = true;

#if defined(CS_SHOW_DEBUG_NETWORK)
   int attempt = 1;
#endif

   while (state() == ConnectingState && (msecs == -1 || stopWatch.elapsed() < msecs)) {
      int timeout = qt_subtract_from_timeout(msecs, stopWatch.elapsed());

      if (msecs != -1 && timeout > QT_CONNECT_TIMEOUT) {
         timeout = QT_CONNECT_TIMEOUT;
      }

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocket::waitForConnected(%i) Waiting %.2f secs for connection attempt #%i",
             msecs, timeout / 1000.0, attempt++);
#endif

      timedOut = false;

      if (d->socketEngine && d->socketEngine->waitForWrite(timeout, &timedOut) && ! timedOut) {
         d->_q_testConnection();
      } else {
         d->_q_connectToNextAddress();
      }
   }

   if ((timedOut && state() != ConnectedState) || state() == ConnectingState) {
      setErrorString(tr("Socket operation timed out"));
      d->socketError = SocketTimeoutError;

      d->state = UnconnectedState;

      emit stateChanged(d->state);
      d->resetSocketLayer();

   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::waitForConnected(%i) == %s", msecs,
          state() == ConnectedState ? "true" : "false");
#endif
   if (state() != ConnectedState) {
      return false;
   }

   if (wasPendingClose) {
      disconnectFromHost();
   }

   return true;
}

bool QAbstractSocket::waitForReadyRead(int msecs)
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::waitForReadyRead(%i)", msecs);
#endif

   // require calling connectToHost() before waitForReadyRead()

   if (state() == UnconnectedState) {
      // If all you have is a QIODevice pointer to an abstractsocket, you can not check
      // this nor can not avoid this warning

      // qWarning("QAbstractSocket::waitForReadyRead() Not allowed in UnconnectedState");
      return false;
   }

   QElapsedTimer stopWatch;
   stopWatch.start();

   // handle a socket in connecting state
   if (state() == HostLookupState || state() == ConnectingState) {
      if (! waitForConnected(msecs)) {
         return false;
      }
   }

   do {
      if (state() != ConnectedState && state() != BoundState) {
         return false;
      }

      bool readyToRead  = false;
      bool readyToWrite = false;

      if (! d->socketEngine->waitForReadOrWrite(&readyToRead, &readyToWrite, true, !d->writeBuffer.isEmpty(),
            qt_subtract_from_timeout(msecs, stopWatch.elapsed()))) {

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocket::waitForReadyRead(%i) Failed (%i, %s)",
                msecs, d->socketEngine->error(), d->socketEngine->errorString().toLatin1().constData());
#endif

         d->socketError = d->socketEngine->error();
         setErrorString(d->socketEngine->errorString());

         emit error(d->socketError);

         if (d->socketError != SocketTimeoutError) {
            close();
         }
         return false;
      }

      if (readyToRead) {
         if (d->canReadNotification()) {
            return true;
         }
      }

      if (readyToWrite) {
         d->canWriteNotification();
      }

   } while (msecs == -1 || qt_subtract_from_timeout(msecs, stopWatch.elapsed()) > 0);
   return false;
}

bool QAbstractSocket::waitForBytesWritten(int msecs)
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::waitForBytesWritten(%i)", msecs);
#endif

   // require calling connectToHost() before waitForBytesWritten()
   if (state() == UnconnectedState) {
      qWarning("QAbstractSocket::waitForBytesWritten() Not allowed in UnconnectedState");
      return false;
   }

   if (d->writeBuffer.isEmpty()) {
      return false;
   }

   QElapsedTimer stopWatch;
   stopWatch.start();

   // handle a socket in connecting state
   if (state() == HostLookupState || state() == ConnectingState) {
      if (! waitForConnected(msecs)) {
         return false;
      }
   }

   while(true) {
      bool readyToRead  = false;
      bool readyToWrite = false;

      if (! d->socketEngine->waitForReadOrWrite(&readyToRead, &readyToWrite, true, ! d->writeBuffer.isEmpty(),
            qt_subtract_from_timeout(msecs, stopWatch.elapsed()))) {

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocket::waitForBytesWritten(%i) Failed (%i, %s)",
                msecs, d->socketEngine->error(), d->socketEngine->errorString().toLatin1().constData());
#endif

         d->socketError = d->socketEngine->error();
         setErrorString(d->socketEngine->errorString());
         emit error(d->socketError);

         if (d->socketError != SocketTimeoutError) {
            close();
         }

         return false;
      }

      if (readyToRead) {
         if (! d->canReadNotification()) {
            return false;
         }
      }

      if (readyToWrite) {
         if (d->canWriteNotification()) {
            return true;
         }
      }

      if (state() != ConnectedState) {
         return false;
      }
   }
   return false;
}

bool QAbstractSocket::waitForDisconnected(int msecs)
{
   Q_D(QAbstractSocket);

   // require calling connectToHost() before waitForDisconnected()
   if (state() == UnconnectedState) {
      qWarning("QAbstractSocket::waitForDisconnected() Not allowed in UnconnectedState");
      return false;
   }

   QElapsedTimer stopWatch;
   stopWatch.start();

   // handle a socket in connecting state
   if (state() == HostLookupState || state() == ConnectingState) {
      if (!waitForConnected(msecs)) {
         return false;
      }

      if (state() == UnconnectedState) {
         return true;
      }
   }

   while(true) {
      bool readyToRead  = false;
      bool readyToWrite = false;

      if (! d->socketEngine->waitForReadOrWrite(&readyToRead, &readyToWrite, state() == ConnectedState,
            ! d->writeBuffer.isEmpty(), qt_subtract_from_timeout(msecs, stopWatch.elapsed()))) {

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocket::waitForReadyRead(%i) Failed (%i, %s)",
                msecs, d->socketEngine->error(), csPrintable(d->socketEngine->errorString()));
#endif

         d->socketError = d->socketEngine->error();
         setErrorString(d->socketEngine->errorString());

         emit error(d->socketError);
         if (d->socketError != SocketTimeoutError) {
            close();
         }

         return false;
      }

      if (readyToRead) {
         d->canReadNotification();
      }

      if (readyToWrite) {
         d->canWriteNotification();
      }

      if (state() == UnconnectedState) {
         return true;
      }
   }
   return false;
}

void QAbstractSocket::abort()
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::abort()");
#endif

   d->writeBuffer.clear();

   if (d->state == UnconnectedState) {
      return;
   }

#ifdef QT_SSL
   if (QSslSocket *socket = dynamic_cast<QSslSocket *>(this)) {
      socket->abort();
      return;
   }
#endif

   if (d->connectTimer) {
      d->connectTimer->stop();
      delete d->connectTimer;
      d->connectTimer = nullptr;
   }

   d->abortCalled = true;
   close();
}

bool QAbstractSocket::isSequential() const
{
   return true;
}

bool QAbstractSocket::atEnd() const
{
   return QIODevice::atEnd() && (!isOpen() || d_func()->buffer.isEmpty());
}

bool QAbstractSocket::flush()
{
   Q_D(QAbstractSocket);

#ifdef QT_SSL
   // Manual polymorphism, flush() is not virtual, QSslSocket overloads it
   if (QSslSocket *socket = dynamic_cast<QSslSocket *>(this)) {
      return socket->flush();
   }
#endif

   Q_CHECK_SOCKETENGINE(false);

   return d->flush();
}

qint64 QAbstractSocket::readData(char *data, qint64 maxSize)
{
   Q_D(QAbstractSocket);

   // Check if the read notifier can be enabled again.
   if (d->socketEngine && !d->socketEngine->isReadNotificationEnabled() && d->socketEngine->isValid()) {
      d->socketEngine->setReadNotificationEnabled(true);
   }

   if (! maxSize) {
      return 0;
   }

   // for a buffered QTcpSocket
   if (d->isBuffered) {
      // if we're still connected, return 0 indicating there may be more data in the future
      // if we're not connected, return -1 indicating EOF

      return d->state == QAbstractSocket::ConnectedState ? qint64(0) : qint64(-1);
   }

   if (! d->socketEngine)  {
      return -1;   // no socket engine is probably EOF
   }

   if (! d->socketEngine->isValid()) {
      return -1;   // unbuffered TCP when we already had been disconnected
   }

   if (d->state != QAbstractSocket::ConnectedState) {
      return -1;   // This is for unbuffered TCP if we are not connected yet
   }

   qint64 readBytes = d->socketEngine->read(data, maxSize);
   if (readBytes == -2) {
      // -2 from the engine means no bytes available (EAGAIN) so read more later
      return 0;

   } else if (readBytes < 0) {
      d->socketError = d->socketEngine->error();
      setErrorString(d->socketEngine->errorString());

      d->resetSocketLayer();
      d->state = QAbstractSocket::UnconnectedState;

   } else if (!d->socketEngine->isReadNotificationEnabled()) {
      // Only do this when there was no error
      d->socketEngine->setReadNotificationEnabled(true);
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::readData(%p \"%s\", %lli) == %lld [engine]",
          data, qt_prettyDebug(data, 32, readBytes).data(), maxSize, readBytes);
#endif

   return readBytes;
}

qint64 QAbstractSocket::readLineData(char *data, qint64 maxlen)
{
   return QIODevice::readLineData(data, maxlen);
}

qint64 QAbstractSocket::writeData(const char *data, qint64 size)
{
   Q_D(QAbstractSocket);

   if (d->state == QAbstractSocket::UnconnectedState || (! d->socketEngine &&
         d->socketType != TcpSocket && ! d->isBuffered)) {

      d->socketError = QAbstractSocket::UnknownSocketError;
      setErrorString(tr("Socket is not connected"));
      return -1;
   }

   if (! d->isBuffered && d->socketType == TcpSocket && d->socketEngine && d->writeBuffer.isEmpty()) {
      // This code is for the new Unbuffered QTcpSocket use case
      qint64 written = size ? d->socketEngine->write(data, size) : Q_INT64_C(0);

      if (written < 0) {
         d->socketError = d->socketEngine->error();
         setErrorString(d->socketEngine->errorString());
         return written;

      } else if (written < size) {
         // Buffer what was not written yet
         char *ptr = d->writeBuffer.reserve(size - written);
         memcpy(ptr, data + written, size - written);
         d->socketEngine->setWriteNotificationEnabled(true);
      }

      return size; // size=actually written + what has been buffered

   } else if (! d->isBuffered && d->socketType != TcpSocket) {
      // This is for a QUdpSocket that was connect()ed
      qint64 written = d->socketEngine->write(data, size);

      if (written < 0) {
         d->socketError = d->socketEngine->error();
         setErrorString(d->socketEngine->errorString());
      } else if (!d->writeBuffer.isEmpty()) {
         d->socketEngine->setWriteNotificationEnabled(true);
      }

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocket::writeData(%p \"%s\", %lli) == %lli", data,
             qt_prettyDebug(data, qMin((int)size, 32), size).data(), size, written);
#endif

      if (written >= 0) {
         emit bytesWritten(written);
      }

      return written;
   }

   // This is the code path for normal buffered QTcpSocket or
   // unbuffered QTcpSocket when there was already something in the
   // write buffer and therefore we could not do a direct engine write.
   // We just write to our write buffer and enable the write notifier
   // The write notifier then flush()es the buffer.

   char *ptr = d->writeBuffer.reserve(size);

   if (size == 1) {
      *ptr = *data;
   } else {
      memcpy(ptr, data, size);
   }

   qint64 written = size;

   if (d->socketEngine && !d->writeBuffer.isEmpty()) {
      d->socketEngine->setWriteNotificationEnabled(true);
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::writeData(%p \"%s\", %lli) == %lli", data,
          qt_prettyDebug(data, qMin((int)size, 32), size).data(), size, written);
#endif

   return written;
}

void QAbstractSocket::setLocalPort(quint16 port)
{
   Q_D(QAbstractSocket);
   d->localPort = port;
}

void QAbstractSocket::setLocalAddress(const QHostAddress &address)
{
   Q_D(QAbstractSocket);
   d->localAddress = address;
}

void QAbstractSocket::setPeerPort(quint16 port)
{
   Q_D(QAbstractSocket);
   d->peerPort = port;
}

void QAbstractSocket::setPeerAddress(const QHostAddress &address)
{
   Q_D(QAbstractSocket);
   d->peerAddress = address;
}

void QAbstractSocket::setPeerName(const QString &name)
{
   Q_D(QAbstractSocket);
   d->peerName = name;
}

void QAbstractSocket::close()
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::close()");
#endif

   QIODevice::close();

   if (d->state != UnconnectedState) {
      disconnectFromHost();
   }

   d->localPort = 0;
   d->peerPort = 0;
   d->localAddress.clear();
   d->peerAddress.clear();
   d->peerName.clear();
   d->cachedSocketDescriptor = -1;
}

void QAbstractSocket::disconnectFromHost()
{
   Q_D(QAbstractSocket);

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::disconnectFromHost()");
#endif

   if (d->state == UnconnectedState) {
#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocket::disconnectFromHost() Called on an unconnected socket");
#endif

      return;
   }

   if (! d->abortCalled && (d->state == ConnectingState || d->state == HostLookupState)) {
#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocket::disconnectFromHost() Still connecting");
#endif

      d->pendingClose = true;

      return;
   }

   // Disable and delete read notification
   if (d->socketEngine) {
      d->socketEngine->setReadNotificationEnabled(false);
   }

   if (d->abortCalled) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("QAbstractSocket::disconnectFromHost() Aborting immediately");
#endif

      if (d->state == HostLookupState) {
         QHostInfo::abortHostLookup(d->hostLookupId);
         d->hostLookupId = -1;
      }

   } else {
      // Perhaps emit closing()

      if (d->state != ClosingState) {
         d->state = ClosingState;

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocket::disconnectFromHost() Emiting stateChanged()- ClosingState");
#endif

         emit stateChanged(d->state);

      } else {

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocket::disconnectFromHost() Return from delayed close");
#endif

      }

      // Wait for pending data to be written
      if (d->socketEngine && d->socketEngine->isValid() && (d->writeBuffer.size() > 0
            || d->socketEngine->bytesToWrite() > 0)) {

         // when we are waiting for the socket engine to write bytes, close after 2 seconds

         // prevents a timeout on Mac, wen the write notifier is lost and no progress was made
         // only possible when using Socks5 or HTTP socket engine

         if (d->writeBuffer.size() == 0 && d->socketEngine->bytesToWrite() > 0) {
            if (! d->disconnectTimer) {
               d->disconnectTimer = new QTimer(this);

               connect(d->disconnectTimer, &QTimer::timeout, this,
                     &QAbstractSocket::_q_forceDisconnect, Qt::DirectConnection);

            }

            if (! d->disconnectTimer->isActive()) {
               d->disconnectTimer->start(2000);
            }
         }

         d->socketEngine->setWriteNotificationEnabled(true);

#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocket::disconnectFromHost() Delaying disconnect");
#endif
         return;

      } else {
#if defined(CS_SHOW_DEBUG_NETWORK)
         qDebug("QAbstractSocket::disconnectFromHost() Disconnecting immediately");
#endif
      }
   }

   SocketState previousState = d->state;
   d->resetSocketLayer();
   d->state = UnconnectedState;
   emit stateChanged(d->state);
   emit readChannelFinished();       // we got an EOF

   // only emit disconnected if we were connected before
   if (previousState == ConnectedState || previousState == ClosingState) {
      emit disconnected();
   }

   d->localPort = 0;
   d->peerPort  = 0;
   d->localAddress.clear();
   d->peerAddress.clear();
   d->writeBuffer.clear();

#if defined(CS_SHOW_DEBUG_NETWORK)
   qDebug("QAbstractSocket::disconnectFromHost() Disconnected");
#endif

}

qint64 QAbstractSocket::readBufferSize() const
{
   return d_func()->readBufferMaxSize;
}

void QAbstractSocket::setReadBufferSize(qint64 size)
{
   Q_D(QAbstractSocket);

   if (d->readBufferMaxSize == size) {
      return;
   }

   d->readBufferMaxSize = size;

   if (! d->readSocketNotifierCalled && d->socketEngine) {
      // ensure that the read notification is enabled if we've now got
      // room in the read buffer
      // but only if we're not inside canReadNotification -- that will take care on its own

      if ((size == 0 || d->buffer.size() < size) && d->state == QAbstractSocket::ConnectedState) {
         // Do not change the notifier unless we are connected.
         d->socketEngine->setReadNotificationEnabled(true);
      }
   }
}

QAbstractSocket::SocketState QAbstractSocket::state() const
{
   return d_func()->state;
}

void QAbstractSocket::setSocketState(SocketState state)
{
   d_func()->state = state;
}

QAbstractSocket::SocketType QAbstractSocket::socketType() const
{
   return d_func()->socketType;
}

QAbstractSocket::SocketError QAbstractSocket::error() const
{
   return d_func()->socketError;
}

void QAbstractSocket::setSocketError(SocketError socketError)
{
   d_func()->socketError = socketError;
}

#ifndef QT_NO_NETWORKPROXY

void QAbstractSocket::setProxy(const QNetworkProxy &networkProxy)
{
   Q_D(QAbstractSocket);
   d->proxy = networkProxy;
}

QNetworkProxy QAbstractSocket::proxy() const
{
   Q_D(const QAbstractSocket);
   return d->proxy;
}
#endif

Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, QAbstractSocket::SocketError error)
{
   // QDebugStateSaver saver(debug);
   // debug.resetFormat().nospace();

   switch (error) {
      case QAbstractSocket::ConnectionRefusedError:
         debug << "QAbstractSocket::ConnectionRefusedError";
         break;

      case QAbstractSocket::RemoteHostClosedError:
         debug << "QAbstractSocket::RemoteHostClosedError";
         break;

      case QAbstractSocket::HostNotFoundError:
         debug << "QAbstractSocket::HostNotFoundError";
         break;

      case QAbstractSocket::SocketAccessError:
         debug << "QAbstractSocket::SocketAccessError";
         break;

      case QAbstractSocket::SocketResourceError:
         debug << "QAbstractSocket::SocketResourceError";
         break;

      case QAbstractSocket::SocketTimeoutError:
         debug << "QAbstractSocket::SocketTimeoutError";
         break;

      case QAbstractSocket::DatagramTooLargeError:
         debug << "QAbstractSocket::DatagramTooLargeError";
         break;

      case QAbstractSocket::NetworkError:
         debug << "QAbstractSocket::NetworkError";
         break;

      case QAbstractSocket::AddressInUseError:
         debug << "QAbstractSocket::AddressInUseError";
         break;

      case QAbstractSocket::SocketAddressNotAvailableError:
         debug << "QAbstractSocket::SocketAddressNotAvailableError";
         break;

      case QAbstractSocket::UnsupportedSocketOperationError:
         debug << "QAbstractSocket::UnsupportedSocketOperationError";
         break;

      case QAbstractSocket::UnfinishedSocketOperationError:
         debug << "QAbstractSocket::UnfinishedSocketOperationError";
         break;

      case QAbstractSocket::ProxyAuthenticationRequiredError:
         debug << "QAbstractSocket::ProxyAuthenticationRequiredError";
         break;

      case QAbstractSocket::UnknownSocketError:
         debug << "QAbstractSocket::UnknownSocketError";
         break;

      case QAbstractSocket::ProxyConnectionRefusedError:
         debug << "QAbstractSocket::ProxyConnectionRefusedError";
         break;

      case QAbstractSocket::ProxyConnectionClosedError:
         debug << "QAbstractSocket::ProxyConnectionClosedError";
         break;

      case QAbstractSocket::ProxyConnectionTimeoutError:
         debug << "QAbstractSocket::ProxyConnectionTimeoutError";
         break;

      case QAbstractSocket::ProxyNotFoundError:
         debug << "QAbstractSocket::ProxyNotFoundError";
         break;

      case QAbstractSocket::ProxyProtocolError:
         debug << "QAbstractSocket::ProxyProtocolError";
         break;

      default:
         debug << "QAbstractSocket::SocketError(" << int(error) << ')';
         break;
   }

   return debug;
}

Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, QAbstractSocket::SocketState state)
{
   // QDebugStateSaver saver(debug);
   // debug.resetFormat().nospace();

   switch (state) {
      case QAbstractSocket::UnconnectedState:
         debug << "QAbstractSocket::UnconnectedState";
         break;

      case QAbstractSocket::HostLookupState:
         debug << "QAbstractSocket::HostLookupState";
         break;

      case QAbstractSocket::ConnectingState:
         debug << "QAbstractSocket::ConnectingState";
         break;

      case QAbstractSocket::ConnectedState:
         debug << "QAbstractSocket::ConnectedState";
         break;

      case QAbstractSocket::BoundState:
         debug << "QAbstractSocket::BoundState";
         break;

      case QAbstractSocket::ListeningState:
         debug << "QAbstractSocket::ListeningState";
         break;

      case QAbstractSocket::ClosingState:
         debug << "QAbstractSocket::ClosingState";
         break;

      default:
         debug << "QAbstractSocket::SocketState(" << int(state) << ')';
         break;
   }

   return debug;
}

void QAbstractSocket::_q_connectToNextAddress()
{
   Q_D(QAbstractSocket);
   d->_q_connectToNextAddress();
}

void QAbstractSocket::_q_startConnecting(const QHostInfo &hostInfo)
{
   Q_D(QAbstractSocket);
   d->_q_startConnecting(hostInfo);
}

void QAbstractSocket::_q_abortConnectionAttempt()
{
   Q_D(QAbstractSocket);
   d->_q_abortConnectionAttempt();
}

void QAbstractSocket::_q_testConnection()
{
   Q_D(QAbstractSocket);
   d->_q_testConnection();
}

void QAbstractSocket::_q_forceDisconnect()
{
   Q_D(QAbstractSocket);
   d->_q_forceDisconnect();
}
