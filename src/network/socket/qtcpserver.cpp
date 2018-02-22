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

//#define QTCPSERVER_DEBUG

#include <qtcpserver.h>
#include <qtcpserver_p.h>

#include <qalgorithms.h>
#include <qhostaddress.h>
#include <qlist.h>
#include <qpointer.h>
#include <qabstractsocketengine_p.h>
#include <qtcpsocket.h>
#include <qnetworkproxy.h>

#define Q_CHECK_SOCKETENGINE(returnValue) do { \
    if (!d->socketEngine) { \
        return returnValue; \
    } } while (0)


/*! \internal
*/
QTcpServerPrivate::QTcpServerPrivate()
   : port(0), state(QAbstractSocket::UnconnectedState), socketEngine(0)
   , serverSocketError(QAbstractSocket::UnknownSocketError), maxConnections(30)
{
}

/*! \internal
*/
QTcpServerPrivate::~QTcpServerPrivate()
{
}

#ifndef QT_NO_NETWORKPROXY
/*! \internal

    Resolve the proxy to its final value.
*/
QNetworkProxy QTcpServerPrivate::resolveProxy(const QHostAddress &address, quint16 port)
{
   if (address.isLoopback()) {
      return QNetworkProxy::NoProxy;
   }

   QList<QNetworkProxy> proxies;
   if (proxy.type() != QNetworkProxy::DefaultProxy) {
      // a non-default proxy was set with setProxy
      proxies << proxy;
   } else {
      // try the application settings instead
      QNetworkProxyQuery query(port, QString(), QNetworkProxyQuery::TcpServer);
      proxies = QNetworkProxyFactory::proxyForQuery(query);
   }

   // return the first that we can use
   for (const QNetworkProxy &p : proxies) {
      if (p.capabilities() & QNetworkProxy::ListeningCapability) {
         return p;
      }
   }

   // no proxy found
   // DefaultProxy will raise an error
   return QNetworkProxy(QNetworkProxy::DefaultProxy);
}
#endif

// internal
void QTcpServerPrivate::configureCreatedSocket()
{
#if defined(Q_OS_UNIX)
   // Under Unix, we want to be able to bind to the port, even if a socket on
   // the same address-port is in TIME_WAIT. Under Windows this is possible
   // anyway -- furthermore, the meaning of reusable on Windows is different:
   // it means that you can use the same address-port for multiple listening
   // sockets.

   // Don't abort though if we can't set that option. For example the socks
   // engine doesn't support that option, but that shouldn't prevent us from
   // trying to bind/listen.

   socketEngine->setOption(QAbstractSocketEngine::AddressReusable, 1);
#endif
}

// internal
void QTcpServerPrivate::readNotification()
{
   Q_Q(QTcpServer);

   for (;;) {
      if (pendingConnections.count() >= maxConnections) {
#if defined (QTCPSERVER_DEBUG)
         qDebug("QTcpServerPrivate::_q_processIncomingConnection() too many connections");
#endif
         if (socketEngine->isReadNotificationEnabled()) {
            socketEngine->setReadNotificationEnabled(false);
         }
         return;
      }

      int descriptor = socketEngine->accept();
      if (descriptor == -1) {
         if (socketEngine->error() != QAbstractSocket::TemporaryError) {
            q->pauseAccepting();
            serverSocketError = socketEngine->error();
            serverSocketErrorString = socketEngine->errorString();
            emit q->acceptError(serverSocketError);
         }
         break;
      }
#if defined (QTCPSERVER_DEBUG)
      qDebug("QTcpServerPrivate::_q_processIncomingConnection() accepted socket %i", descriptor);
#endif
      q->incomingConnection(descriptor);

      QPointer<QTcpServer> that = q;
      emit q->newConnection();

      if (! that || ! q->isListening()) {
         return;
      }
   }
}

QTcpServer::QTcpServer(QObject *parent)
   : QObject(parent), d_ptr(new QTcpServerPrivate)
{
   d_ptr->q_ptr = this;

#if defined(QTCPSERVER_DEBUG)
   qDebug("QTcpServer::QTcpServer(%p)", parent);
#endif

}

// internal
QTcpServer::QTcpServer(QTcpServerPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;

#if defined(QTCPSERVER_DEBUG)
   qDebug("QTcpServer::QTcpServer(QTcpServerPrivate == %p, parent == %p)", &dd, parent);
#endif
}

QTcpServer::~QTcpServer()
{
   close();
}

bool QTcpServer::listen(const QHostAddress &address, quint16 port)
{
   Q_D(QTcpServer);
   if (d->state == QAbstractSocket::ListeningState) {
      qWarning("QTcpServer::listen() called when already listening");
      return false;
   }

   QAbstractSocket::NetworkLayerProtocol proto = address.protocol();
   QHostAddress addr = address;

#ifdef QT_NO_NETWORKPROXY
   static const QNetworkProxy &proxy = *(QNetworkProxy *)0;
#else
   QNetworkProxy proxy = d->resolveProxy(addr, port);
#endif

   delete d->socketEngine;
   d->socketEngine = QAbstractSocketEngine::createSocketEngine(QAbstractSocket::TcpSocket, proxy, this);
   if (!d->socketEngine) {
      d->serverSocketError = QAbstractSocket::UnsupportedSocketOperationError;
      d->serverSocketErrorString = tr("Operation on socket is not supported");
      return false;
   }

#ifndef QT_NO_BEARERMANAGEMENT
   //copy network session down to the socket engine (if it has been set)
   d->socketEngine->setProperty("_q_networksession", property("_q_networksession"));
#endif

   if (!d->socketEngine->initialize(QAbstractSocket::TcpSocket, proto)) {
      d->serverSocketError = d->socketEngine->error();
      d->serverSocketErrorString = d->socketEngine->errorString();
      return false;
   }

   proto = d->socketEngine->protocol();
   if (addr.protocol() == QAbstractSocket::AnyIPProtocol && proto == QAbstractSocket::IPv4Protocol) {
      addr = QHostAddress::AnyIPv4;
   }

   d->configureCreatedSocket();

   if (! d->socketEngine->bind(addr, port)) {
      d->serverSocketError = d->socketEngine->error();
      d->serverSocketErrorString = d->socketEngine->errorString();
      return false;
   }

   if (!d->socketEngine->listen()) {
      d->serverSocketError = d->socketEngine->error();
      d->serverSocketErrorString = d->socketEngine->errorString();
      return false;
   }

   d->socketEngine->setReceiver(d);
   d->socketEngine->setReadNotificationEnabled(true);

   d->state = QAbstractSocket::ListeningState;
   d->address = d->socketEngine->localAddress();
   d->port = d->socketEngine->localPort();

#if defined (QTCPSERVER_DEBUG)
   qDebug("QTcpServer::listen(%i, \"%s\") == true (listening on port %i)", port,
          address.toString().toLatin1().constData(), d->socketEngine->localPort());
#endif
   return true;
}


bool QTcpServer::isListening() const
{
   Q_D(const QTcpServer);
   Q_CHECK_SOCKETENGINE(false);
   return d->socketEngine->state() == QAbstractSocket::ListeningState;
}

/*!
    Closes the server. The server will no longer listen for incoming
    connections.

    \sa listen()
*/
void QTcpServer::close()
{
   Q_D(QTcpServer);

   qDeleteAll(d->pendingConnections);
   d->pendingConnections.clear();

   if (d->socketEngine) {
      d->socketEngine->close();
      QT_TRY {
         d->socketEngine->deleteLater();
      } QT_CATCH(const std::bad_alloc &) {
         // in out of memory situations, the socketEngine
         // will be deleted in ~QTcpServer (it's a child-object of this)
      }
      d->socketEngine = 0;
   }

   d->state = QAbstractSocket::UnconnectedState;
}


qintptr QTcpServer::socketDescriptor() const
{
   Q_D(const QTcpServer);
   Q_CHECK_SOCKETENGINE(-1);
   return d->socketEngine->socketDescriptor();
}


bool QTcpServer::setSocketDescriptor(qintptr socketDescriptor)
{
   Q_D(QTcpServer);
   if (isListening()) {
      qWarning("QTcpServer::setSocketDescriptor() called when already listening");
      return false;
   }

   if (d->socketEngine) {
      delete d->socketEngine;
   }

   d->socketEngine = QAbstractSocketEngine::createSocketEngine(socketDescriptor, this);
   if (!d->socketEngine) {
      d->serverSocketError = QAbstractSocket::UnsupportedSocketOperationError;
      d->serverSocketErrorString = tr("Operation on socket is not supported");
      return false;
   }

#ifndef QT_NO_BEARERMANAGEMENT
   //copy network session down to the socket engine (if it has been set)
   d->socketEngine->setProperty("_q_networksession", property("_q_networksession"));
#endif

   if (!d->socketEngine->initialize(socketDescriptor, QAbstractSocket::ListeningState)) {
      d->serverSocketError = d->socketEngine->error();
      d->serverSocketErrorString = d->socketEngine->errorString();

#if defined (QTCPSERVER_DEBUG)
      qDebug("QTcpServer::setSocketDescriptor(%i) failed (%s)", socketDescriptor,
             d->serverSocketErrorString.toLatin1().constData());
#endif

      return false;
   }

   d->socketEngine->setReceiver(d);
   d->socketEngine->setReadNotificationEnabled(true);

   d->state = d->socketEngine->state();
   d->address = d->socketEngine->localAddress();
   d->port = d->socketEngine->localPort();

#if defined (QTCPSERVER_DEBUG)
   qDebug("QTcpServer::setSocketDescriptor(%i) succeeded.", socketDescriptor);
#endif
   return true;
}

/*!
    Returns the server's port if the server is listening for
    connections; otherwise returns 0.

    \sa serverAddress(), listen()
*/
quint16 QTcpServer::serverPort() const
{
   Q_D(const QTcpServer);
   Q_CHECK_SOCKETENGINE(0);
   return d->socketEngine->localPort();
}

/*!
    Returns the server's address if the server is listening for
    connections; otherwise returns QHostAddress::Null.

    \sa serverPort(), listen()
*/
QHostAddress QTcpServer::serverAddress() const
{
   Q_D(const QTcpServer);
   Q_CHECK_SOCKETENGINE(QHostAddress(QHostAddress::Null));
   return d->socketEngine->localAddress();
}


bool QTcpServer::waitForNewConnection(int msec, bool *timedOut)
{
   Q_D(QTcpServer);
   if (d->state != QAbstractSocket::ListeningState) {
      return false;
   }

   if (!d->socketEngine->waitForRead(msec, timedOut)) {
      d->serverSocketError = d->socketEngine->error();
      d->serverSocketErrorString = d->socketEngine->errorString();
      return false;
   }

   if (timedOut && *timedOut) {
      return false;
   }

   d->readNotification();

   return true;
}


bool QTcpServer::hasPendingConnections() const
{
   return !d_func()->pendingConnections.isEmpty();
}


QTcpSocket *QTcpServer::nextPendingConnection()
{
   Q_D(QTcpServer);
   if (d->pendingConnections.isEmpty()) {
      return 0;
   }

   if (! d->socketEngine->isReadNotificationEnabled()) {
      d->socketEngine->setReadNotificationEnabled(true);
   }

   return d->pendingConnections.takeFirst();
}


void QTcpServer::incomingConnection(qintptr socketDescriptor)
{
#if defined (QTCPSERVER_DEBUG)
   qDebug("QTcpServer::incomingConnection(%i)", socketDescriptor);
#endif

   QTcpSocket *socket = new QTcpSocket(this);
   socket->setSocketDescriptor(socketDescriptor);
   addPendingConnection(socket);
}

/*!
    This function is called by QTcpServer::incomingConnection()
    to add the \a socket to the list of pending incoming connections.

    \note Don't forget to call this member from reimplemented
    incomingConnection() if you do not want to break the
    Pending Connections mechanism.

    \sa incomingConnection()
    \since 4.7
*/
void QTcpServer::addPendingConnection(QTcpSocket *socket)
{
   d_func()->pendingConnections.append(socket);
}


void QTcpServer::setMaxPendingConnections(int numConnections)
{
   d_func()->maxConnections = numConnections;
}


int QTcpServer::maxPendingConnections() const
{
   return d_func()->maxConnections;
}


QAbstractSocket::SocketError QTcpServer::serverError() const
{
   return d_func()->serverSocketError;
}

QString QTcpServer::errorString() const
{
   return d_func()->serverSocketErrorString;
}

void QTcpServer::pauseAccepting()
{
   d_func()->socketEngine->setReadNotificationEnabled(false);
}

void QTcpServer::resumeAccepting()
{
   d_func()->socketEngine->setReadNotificationEnabled(true);
}

#ifndef QT_NO_NETWORKPROXY

void QTcpServer::setProxy(const QNetworkProxy &networkProxy)
{
   Q_D(QTcpServer);
   d->proxy = networkProxy;
}


QNetworkProxy QTcpServer::proxy() const
{
   Q_D(const QTcpServer);
   return d->proxy;
}
#endif // QT_NO_NETWORKPROXY

