/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

//#define QNATIVESOCKETENGINE_DEBUG

#include <qnativesocketengine_p.h>
#include <qnet_unix_p.h>
#include <qiodevice.h>
#include <qhostaddress.h>
#include <qelapsedtimer.h>
#include <qvarlengtharray.h>
#include <qnetworkinterface.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifndef QT_NO_IPV6IFNAME
#include <net/if.h>
#endif

#ifdef QT_LINUXBASE
#include <arpa/inet.h>
#endif

#ifdef Q_OS_BSD4
#include <net/if_dl.h>
#endif

#if defined QNATIVESOCKETENGINE_DEBUG
#include <qstring.h>
#include <ctype.h>
#endif

#include <netinet/tcp.h>

#if defined QNATIVESOCKETENGINE_DEBUG

// Returns a human readable representation of the first \a len characters in \a data.
static QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
   if (!data) {
      return "(null)";
   }

   QByteArray out;

   for (int i = 0; i < len; ++i) {
      char c = data[i];

      if (isprint(c)) {
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
               QString tmp;
               tmp.sprintf("\\%o", c);
               out += tmp.toLatin1();
         }
   }

   if (len < maxSize) {
      out += "...";
   }

   return out;
}
#endif

/*
    Extracts the port and address from a sockaddr, and stores them in
    \a port and \a addr if they are non-null.
*/
static inline void qt_socket_getPortAndAddress(const qt_sockaddr *s, quint16 *port, QHostAddress *addr)
{
   if (s->a.sa_family == AF_INET6) {
      Q_IPV6ADDR tmp;
      memcpy(&tmp, &s->a6.sin6_addr, sizeof(tmp));

      if (addr) {
         QHostAddress tmpAddress;
         tmpAddress.setAddress(tmp);
         *addr = tmpAddress;

         if (s->a6.sin6_scope_id) {
#ifndef QT_NO_IPV6IFNAME
            char scopeid[IFNAMSIZ];
            if (::if_indextoname(s->a6.sin6_scope_id, scopeid)) {
               addr->setScopeId(QLatin1String(scopeid));
            } else
#endif
               addr->setScopeId(QString::number(s->a6.sin6_scope_id));
         }
      }
      if (port) {
         *port = ntohs(s->a6.sin6_port);
      }
      return;
   }

   if (port) {
      *port = ntohs(s->a4.sin_port);
   }

   if (addr) {
      QHostAddress tmpAddress;
      tmpAddress.setAddress(ntohl(s->a4.sin_addr.s_addr));
      *addr = tmpAddress;
   }
}

// inline on purpose
inline uint QNativeSocketEnginePrivate::scopeIdFromString(const QString &scopeid)
{
   if (scopeid.isEmpty()) {
      return 0;
   }

   bool ok;
   uint id = scopeid.toInteger<uint>(&ok);

#ifndef QT_NO_IPV6IFNAME
   if (! ok) {
      id = ::if_nametoindex(scopeid.toLatin1().constData());
   }
#endif

   return id;
}

static void convertToLevelAndOption(QNativeSocketEngine::SocketOption opt,
                                    QAbstractSocket::NetworkLayerProtocol socketProtocol, int &level, int &n)
{
   n = -1;
   level = SOL_SOCKET; // default

   switch (opt) {
      case QNativeSocketEngine::NonBlockingSocketOption:  // fcntl, not setsockopt
      case QNativeSocketEngine::BindExclusively:          // not handled on Unix
      // code not reachable

      case QNativeSocketEngine::BroadcastSocketOption:
         n = SO_BROADCAST;
         break;

      case QNativeSocketEngine::ReceiveBufferSocketOption:
         n = SO_RCVBUF;
         break;

      case QNativeSocketEngine::SendBufferSocketOption:
         n = SO_SNDBUF;
         break;

      case QNativeSocketEngine::AddressReusable:
         n = SO_REUSEADDR;
         break;

      case QNativeSocketEngine::ReceiveOutOfBandData:
         n = SO_OOBINLINE;
         break;
      case QNativeSocketEngine::LowDelayOption:
         level = IPPROTO_TCP;
         n = TCP_NODELAY;
         break;
      case QNativeSocketEngine::KeepAliveOption:
         n = SO_KEEPALIVE;
         break;
      case QNativeSocketEngine::MulticastTtlOption:
         if (socketProtocol == QAbstractSocket::IPv6Protocol || socketProtocol == QAbstractSocket::AnyIPProtocol) {
            level = IPPROTO_IPV6;
            n = IPV6_MULTICAST_HOPS;
         } else

         {
            level = IPPROTO_IP;
            n = IP_MULTICAST_TTL;
         }
         break;

      case QNativeSocketEngine::MulticastLoopbackOption:
         if (socketProtocol == QAbstractSocket::IPv6Protocol || socketProtocol == QAbstractSocket::AnyIPProtocol) {
            level = IPPROTO_IPV6;
            n = IPV6_MULTICAST_LOOP;
         } else

         {
            level = IPPROTO_IP;
            n = IP_MULTICAST_LOOP;
         }
         break;
      case QNativeSocketEngine::TypeOfServiceOption:
         if (socketProtocol == QAbstractSocket::IPv4Protocol) {
            level = IPPROTO_IP;
            n = IP_TOS;
         }
         break;
      case QNativeSocketEngine::ReceivePacketInformation:
         if (socketProtocol == QAbstractSocket::IPv6Protocol || socketProtocol == QAbstractSocket::AnyIPProtocol) {
            level = IPPROTO_IPV6;
            n = IPV6_RECVPKTINFO;

         } else if (socketProtocol == QAbstractSocket::IPv4Protocol) {
            level = IPPROTO_IP;

#ifdef IP_PKTINFO
            n = IP_PKTINFO;

#elif defined(IP_RECVDSTADDR)
            // variant found in QNX and FreeBSD; it will get us only the
            // destination address, not the interface; we need IP_RECVIF for that.
            n = IP_RECVDSTADDR;
#endif
         }
         break;

      case QNativeSocketEngine::ReceiveHopLimit:
         if (socketProtocol == QAbstractSocket::IPv6Protocol || socketProtocol == QAbstractSocket::AnyIPProtocol) {
            level = IPPROTO_IPV6;
            n = IPV6_RECVHOPLIMIT;

         } else if (socketProtocol == QAbstractSocket::IPv4Protocol) {
#ifdef IP_RECVTTL               // IP_RECVTTL is a non-standard extension supported on some OS
            level = IPPROTO_IP;
            n = IP_RECVTTL;
#endif
         }
         break;
   }
}
/*! \internal

    Creates and returns a new socket descriptor of type \a socketType
    and \a socketProtocol.  Returns -1 on failure.
*/
bool QNativeSocketEnginePrivate::createNewSocket(QAbstractSocket::SocketType socketType,
      QAbstractSocket::NetworkLayerProtocol &socketProtocol)
{
   int protocol = (socketProtocol == QAbstractSocket::IPv6Protocol ||
                   socketProtocol == QAbstractSocket::AnyIPProtocol) ? AF_INET6 : AF_INET;

   int type = (socketType == QAbstractSocket::UdpSocket) ? SOCK_DGRAM : SOCK_STREAM;

   int socket = qt_safe_socket(protocol, type, 0, O_NONBLOCK);
   if (socket < 0 && socketProtocol == QAbstractSocket::AnyIPProtocol && errno == EAFNOSUPPORT) {
      protocol = AF_INET;
      socket = qt_safe_socket(protocol, type, 0, O_NONBLOCK);
      socketProtocol = QAbstractSocket::IPv4Protocol;
   }

   if (socket < 0) {
      int ecopy = errno;
      switch (ecopy) {
         case EPROTONOSUPPORT:
         case EAFNOSUPPORT:
         case EINVAL:
            setError(QAbstractSocket::UnsupportedSocketOperationError, ProtocolUnsupportedErrorString);
            break;
         case ENFILE:
         case EMFILE:
         case ENOBUFS:
         case ENOMEM:
            setError(QAbstractSocket::SocketResourceError, ResourceErrorString);
            break;
         case EACCES:
            setError(QAbstractSocket::SocketAccessError, AccessErrorString);
            break;
         default:
            break;
      }

#if defined (QNATIVESOCKETENGINE_DEBUG)
      qDebug("QNativeSocketEnginePrivate::createNewSocket(%d, %d) == false (%s)",
             socketType, socketProtocol, strerror(ecopy));
#endif

      return false;
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::createNewSocket(%d, %d) == true", socketType, socketProtocol);
#endif

   socketDescriptor = socket;
   if (socket != -1) {
      this->socketProtocol = socketProtocol;
      this->socketType = socketType;
   }
   return true;
}

/*
    Returns the value of the socket option \a opt.
*/
int QNativeSocketEnginePrivate::option(QNativeSocketEngine::SocketOption opt) const
{
   Q_Q(const QNativeSocketEngine);
   if (!q->isValid()) {
      return -1;
   }

   // handle non-getsockopt cases first
   if (opt == QNativeSocketEngine::BindExclusively || opt == QNativeSocketEngine::NonBlockingSocketOption
         || opt == QNativeSocketEngine::BroadcastSocketOption)  {
      return -1;
   }

   int n;
   int level;
   int v = -1;

   QT_SOCKOPTLEN_T len = sizeof(v);

   convertToLevelAndOption(opt, socketProtocol, level, n);
   if (n != -1 && ::getsockopt(socketDescriptor, level, n, (char *) &v, &len) != -1) {
      return v;
   }

   return -1;
}


/*
    Sets the socket option \a opt to \a v.
*/
bool QNativeSocketEnginePrivate::setOption(QNativeSocketEngine::SocketOption opt, int v)
{
   Q_Q(QNativeSocketEngine);
   if (!q->isValid()) {
      return false;
   }

   // handle non-setsockopt cases first

   switch (opt) {
      case QNativeSocketEngine::NonBlockingSocketOption: {
         // Make the socket nonblocking

         int flags = ::fcntl(socketDescriptor, F_GETFL, 0);
         if (flags == -1) {
#ifdef QNATIVESOCKETENGINE_DEBUG
            perror("QNativeSocketEnginePrivate::setOption(): fcntl(F_GETFL) failed");
#endif
            return false;
         }
         if (::fcntl(socketDescriptor, F_SETFL, flags | O_NONBLOCK) == -1) {
#ifdef QNATIVESOCKETENGINE_DEBUG
            perror("QNativeSocketEnginePrivate::setOption(): fcntl(F_SETFL) failed");
#endif
            return false;
         }


         return true;
      }
      case QNativeSocketEngine::BindExclusively:
         return true;

      default:
         break;
   }

   int n, level;
   convertToLevelAndOption(opt, socketProtocol, level, n);
#if defined(SO_REUSEPORT) && !defined(Q_OS_LINUX)
   if (opt == QNativeSocketEngine::AddressReusable) {
      // on OS X, SO_REUSEADDR isn't sufficient to allow multiple binds to the
      // same port (which is useful for multicast UDP). SO_REUSEPORT is, but
      // we most definitely do not want to use this for TCP. See QTBUG-6305.
      if (socketType == QAbstractSocket::UdpSocket) {
         n = SO_REUSEPORT;
      }
   }
#endif

   return ::setsockopt(socketDescriptor, level, n, (char *) &v, sizeof(v)) == 0;
}

bool QNativeSocketEnginePrivate::nativeConnect(const QHostAddress &addr, quint16 port)
{
#ifdef QNATIVESOCKETENGINE_DEBUG
   qDebug() << "QNativeSocketEnginePrivate::nativeConnect() " << socketDescriptor;
#endif

   qt_sockaddr aa;
   QT_SOCKLEN_T sockAddrSize;
   setPortAndAddress(port, addr, &aa, &sockAddrSize);

   int connectResult = qt_safe_connect(socketDescriptor, &aa.a, sockAddrSize);
#if defined (QNATIVESOCKETENGINE_DEBUG)
   int ecopy = errno;
#endif

   if (connectResult == -1) {
      switch (errno) {
         case EISCONN:
            socketState = QAbstractSocket::ConnectedState;
            break;
         case ECONNREFUSED:
         case EINVAL:
            setError(QAbstractSocket::ConnectionRefusedError, ConnectionRefusedErrorString);
            socketState = QAbstractSocket::UnconnectedState;
            break;
         case ETIMEDOUT:
            setError(QAbstractSocket::NetworkError, ConnectionTimeOutErrorString);
            break;
         case EHOSTUNREACH:
            setError(QAbstractSocket::NetworkError, HostUnreachableErrorString);
            socketState = QAbstractSocket::UnconnectedState;
            break;
         case ENETUNREACH:
            setError(QAbstractSocket::NetworkError, NetworkUnreachableErrorString);
            socketState = QAbstractSocket::UnconnectedState;
            break;
         case EADDRINUSE:
            setError(QAbstractSocket::NetworkError, AddressInuseErrorString);
            break;
         case EINPROGRESS:
         case EALREADY:
            setError(QAbstractSocket::UnfinishedSocketOperationError, InvalidSocketErrorString);
            socketState = QAbstractSocket::ConnectingState;
            break;
         case EAGAIN:
            setError(QAbstractSocket::UnfinishedSocketOperationError, InvalidSocketErrorString);
            break;
         case EACCES:
         case EPERM:
            setError(QAbstractSocket::SocketAccessError, AccessErrorString);
            socketState = QAbstractSocket::UnconnectedState;
            break;
         case EAFNOSUPPORT:
         case EBADF:
         case EFAULT:
         case ENOTSOCK:
            socketState = QAbstractSocket::UnconnectedState;
         default:
            break;
      }

      if (socketState != QAbstractSocket::ConnectedState) {
#if defined (QNATIVESOCKETENGINE_DEBUG)
         qDebug("QNativeSocketEnginePrivate::nativeConnect(%s, %i) == false (%s)",
                addr.toString().toLatin1().constData(), port,
                socketState == QAbstractSocket::ConnectingState
                ? "Connection in progress" : strerror(ecopy));
#endif
         return false;
      }
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::nativeConnect(%s, %i) == true",
          addr.toString().toLatin1().constData(), port);
#endif

   socketState = QAbstractSocket::ConnectedState;
   return true;
}

bool QNativeSocketEnginePrivate::nativeBind(const QHostAddress &address, quint16 port)
{
   qt_sockaddr aa;
   QT_SOCKLEN_T sockAddrSize;
   setPortAndAddress(port, address, &aa, &sockAddrSize);




#ifdef IPV6_V6ONLY
   if (aa.a.sa_family == AF_INET6) {
      int ipv6only = 0;
      if (address.protocol() == QAbstractSocket::IPv6Protocol) {
         ipv6only = 1;
      }
      //default value of this socket option varies depending on unix variant (or system configuration on BSD), so always set it explicitly
      ::setsockopt(socketDescriptor, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only) );
   }
#endif

   int bindResult = QT_SOCKET_BIND(socketDescriptor, &aa.a, sockAddrSize);
   if (bindResult < 0 && errno == EAFNOSUPPORT && address.protocol() == QAbstractSocket::AnyIPProtocol) {
      // retry with v4
      aa.a4.sin_family = AF_INET;
      aa.a4.sin_port = htons(port);
      aa.a4.sin_addr.s_addr = htonl(address.toIPv4Address());
      sockAddrSize = sizeof(aa.a4);
      bindResult = QT_SOCKET_BIND(socketDescriptor, &aa.a, sockAddrSize);
   }




   if (bindResult < 0) {
#if defined (QNATIVESOCKETENGINE_DEBUG)
      int ecopy = errno;
#endif
      switch (errno) {
         case EADDRINUSE:
            setError(QAbstractSocket::AddressInUseError, AddressInuseErrorString);
            break;
         case EACCES:
            setError(QAbstractSocket::SocketAccessError, AddressProtectedErrorString);
            break;
         case EINVAL:
            setError(QAbstractSocket::UnsupportedSocketOperationError, OperationUnsupportedErrorString);
            break;
         case EADDRNOTAVAIL:
            setError(QAbstractSocket::SocketAddressNotAvailableError, AddressNotAvailableErrorString);
            break;
         default:
            break;
      }

#if defined (QNATIVESOCKETENGINE_DEBUG)
      qDebug("QNativeSocketEnginePrivate::nativeBind(%s, %i) == false (%s)",
             address.toString().toLatin1().constData(), port, strerror(ecopy));
#endif

      return false;
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::nativeBind(%s, %i) == true",
          address.toString().toLatin1().constData(), port);
#endif
   socketState = QAbstractSocket::BoundState;
   return true;
}

bool QNativeSocketEnginePrivate::nativeListen(int backlog)
{
   if (qt_safe_listen(socketDescriptor, backlog) < 0) {
#if defined (QNATIVESOCKETENGINE_DEBUG)
      int ecopy = errno;
#endif
      switch (errno) {
         case EADDRINUSE:
            setError(QAbstractSocket::AddressInUseError,
                     PortInuseErrorString);
            break;
         default:
            break;
      }

#if defined (QNATIVESOCKETENGINE_DEBUG)
      qDebug("QNativeSocketEnginePrivate::nativeListen(%i) == false (%s)",
             backlog, strerror(ecopy));
#endif
      return false;
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::nativeListen(%i) == true", backlog);
#endif

   socketState = QAbstractSocket::ListeningState;
   return true;
}

int QNativeSocketEnginePrivate::nativeAccept()
{
   int acceptedDescriptor = qt_safe_accept(socketDescriptor, 0, 0);
   if (acceptedDescriptor == -1) {
      switch (errno) {
         case EBADF:
         case EOPNOTSUPP:
            setError(QAbstractSocket::UnsupportedSocketOperationError, InvalidSocketErrorString);
            break;
         case ECONNABORTED:
            setError(QAbstractSocket::NetworkError, RemoteHostClosedErrorString);
            break;
         case EFAULT:
         case ENOTSOCK:
            setError(QAbstractSocket::SocketResourceError, NotSocketErrorString);
            break;
         case EPROTONOSUPPORT:
         case EPROTO:
         case EAFNOSUPPORT:
         case EINVAL:
            setError(QAbstractSocket::UnsupportedSocketOperationError, ProtocolUnsupportedErrorString);
            break;
         case ENFILE:
         case EMFILE:
         case ENOBUFS:
         case ENOMEM:
            setError(QAbstractSocket::SocketResourceError, ResourceErrorString);
            break;
         case EACCES:
         case EPERM:
            setError(QAbstractSocket::SocketAccessError, AccessErrorString);
            break;
#if EAGAIN != EWOULDBLOCK
         case EWOULDBLOCK:
#endif
         case EAGAIN:
            setError(QAbstractSocket::TemporaryError, TemporaryErrorString);
            break;
         default:
            setError(QAbstractSocket::UnknownSocketError, UnknownSocketErrorString);
            break;
      }
   }

   return acceptedDescriptor;
}

#ifndef QT_NO_NETWORKINTERFACE

static bool multicastMembershipHelper(QNativeSocketEnginePrivate *d,
                                      int how6,
                                      int how4,
                                      const QHostAddress &groupAddress,
                                      const QNetworkInterface &interface)
{
   int level = 0;
   int sockOpt = 0;
   void *sockArg;
   int sockArgSize;

   ip_mreq mreq4;
   ipv6_mreq mreq6;

   if (groupAddress.protocol() == QAbstractSocket::IPv6Protocol) {
      level = IPPROTO_IPV6;
      sockOpt = how6;
      sockArg = &mreq6;
      sockArgSize = sizeof(mreq6);
      memset(&mreq6, 0, sizeof(mreq6));
      Q_IPV6ADDR ip6 = groupAddress.toIPv6Address();
      memcpy(&mreq6.ipv6mr_multiaddr, &ip6, sizeof(ip6));
      mreq6.ipv6mr_interface = interface.index();
   } else

      if (groupAddress.protocol() == QAbstractSocket::IPv4Protocol) {
         level = IPPROTO_IP;
         sockOpt = how4;
         sockArg = &mreq4;
         sockArgSize = sizeof(mreq4);
         memset(&mreq4, 0, sizeof(mreq4));
         mreq4.imr_multiaddr.s_addr = htonl(groupAddress.toIPv4Address());

         if (interface.isValid()) {
            QList<QNetworkAddressEntry> addressEntries = interface.addressEntries();
            if (!addressEntries.isEmpty()) {
               QHostAddress firstIP = addressEntries.first().ip();
               mreq4.imr_interface.s_addr = htonl(firstIP.toIPv4Address());
            } else {
               d->setError(QAbstractSocket::NetworkError,
                           QNativeSocketEnginePrivate::NetworkUnreachableErrorString);
               return false;
            }
         } else {
            mreq4.imr_interface.s_addr = INADDR_ANY;
         }
      } else {
         // unreachable
         d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                     QNativeSocketEnginePrivate::ProtocolUnsupportedErrorString);
         return false;
      }

   int res = setsockopt(d->socketDescriptor, level, sockOpt, sockArg, sockArgSize);
   if (res == -1) {
      switch (errno) {
         case ENOPROTOOPT:
            d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                        QNativeSocketEnginePrivate::OperationUnsupportedErrorString);
            break;
         case EADDRNOTAVAIL:
            d->setError(QAbstractSocket::SocketAddressNotAvailableError,
                        QNativeSocketEnginePrivate::AddressNotAvailableErrorString);
            break;
         default:
            d->setError(QAbstractSocket::UnknownSocketError,
                        QNativeSocketEnginePrivate::UnknownSocketErrorString);
            break;
      }
      return false;
   }
   return true;
}

bool QNativeSocketEnginePrivate::nativeJoinMulticastGroup(const QHostAddress &groupAddress,
      const QNetworkInterface &interface)
{
   return multicastMembershipHelper(this,
                                    IPV6_JOIN_GROUP,
                                    IP_ADD_MEMBERSHIP,
                                    groupAddress,
                                    interface);
}

bool QNativeSocketEnginePrivate::nativeLeaveMulticastGroup(const QHostAddress &groupAddress,
      const QNetworkInterface &interface)
{
   return multicastMembershipHelper(this,
                                    IPV6_LEAVE_GROUP,
                                    IP_DROP_MEMBERSHIP,
                                    groupAddress,
                                    interface);
}

QNetworkInterface QNativeSocketEnginePrivate::nativeMulticastInterface() const
{
   if (socketProtocol == QAbstractSocket::IPv6Protocol || socketProtocol == QAbstractSocket::AnyIPProtocol) {
      uint v;
      QT_SOCKOPTLEN_T sizeofv = sizeof(v);
      if (::getsockopt(socketDescriptor, IPPROTO_IPV6, IPV6_MULTICAST_IF, &v, &sizeofv) == -1) {
         return QNetworkInterface();
      }
      return QNetworkInterface::interfaceFromIndex(v);
   }

   struct in_addr v = { 0 };
   QT_SOCKOPTLEN_T sizeofv = sizeof(v);
   if (::getsockopt(socketDescriptor, IPPROTO_IP, IP_MULTICAST_IF, &v, &sizeofv) == -1) {
      return QNetworkInterface();
   }

   if (v.s_addr != 0 && sizeofv >= QT_SOCKOPTLEN_T(sizeof(v))) {
      QHostAddress ipv4(ntohl(v.s_addr));
      QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
      for (int i = 0; i < ifaces.count(); ++i) {
         const QNetworkInterface &iface = ifaces.at(i);
         QList<QNetworkAddressEntry> entries = iface.addressEntries();
         for (int j = 0; j < entries.count(); ++j) {
            const QNetworkAddressEntry &entry = entries.at(j);
            if (entry.ip() == ipv4) {
               return iface;
            }
         }
      }
   }
   return QNetworkInterface();
}

bool QNativeSocketEnginePrivate::nativeSetMulticastInterface(const QNetworkInterface &iface)
{
   if (socketProtocol == QAbstractSocket::IPv6Protocol || socketProtocol == QAbstractSocket::AnyIPProtocol) {
      uint v = iface.index();
      return (::setsockopt(socketDescriptor, IPPROTO_IPV6, IPV6_MULTICAST_IF, &v, sizeof(v)) != -1);
   }

   struct in_addr v;
   if (iface.isValid()) {
      QList<QNetworkAddressEntry> entries = iface.addressEntries();
      for (int i = 0; i < entries.count(); ++i) {
         const QNetworkAddressEntry &entry = entries.at(i);
         const QHostAddress &ip = entry.ip();
         if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
            v.s_addr = htonl(ip.toIPv4Address());
            int r = ::setsockopt(socketDescriptor, IPPROTO_IP, IP_MULTICAST_IF, &v, sizeof(v));
            if (r != -1) {
               return true;
            }
         }
      }
      return false;
   }

   v.s_addr = INADDR_ANY;
   return (::setsockopt(socketDescriptor, IPPROTO_IP, IP_MULTICAST_IF, &v, sizeof(v)) != -1);
}

#endif // QT_NO_NETWORKINTERFACE

qint64 QNativeSocketEnginePrivate::nativeBytesAvailable() const
{
   int nbytes = 0;
   // gives shorter than true amounts on Unix domain sockets.
   qint64 available = 0;
   if (qt_safe_ioctl(socketDescriptor, FIONREAD, (char *) &nbytes) >= 0) {
      available = (qint64) nbytes;
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::nativeBytesAvailable() == %lli", available);
#endif
   return available;
}

bool QNativeSocketEnginePrivate::nativeHasPendingDatagrams() const
{
   // Create a sockaddr struct and reset its port number.
   qt_sockaddr storage;
   QT_SOCKLEN_T storageSize = sizeof(storage);
   memset(&storage, 0, storageSize);

   // Peek 0 bytes into the next message. The size of the message may
   // well be 0, so we can't check recvfrom's return value.
   ssize_t readBytes;
   do {
      char c;
      readBytes = ::recvfrom(socketDescriptor, &c, 1, MSG_PEEK, &storage.a, &storageSize);
   } while (readBytes == -1 && errno == EINTR);

   // If there's no error, or if our buffer was too small, there must be a
   // pending datagram.
   bool result = (readBytes != -1) || errno == EMSGSIZE;

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::nativeHasPendingDatagrams() == %s",
          result ? "true" : "false");
#endif
   return result;
}

qint64 QNativeSocketEnginePrivate::nativePendingDatagramSize() const
{
   QVarLengthArray<char, 8192> udpMessagePeekBuffer(8192);
   ssize_t recvResult = -1;

   for (;;) {
      // the data written to udpMessagePeekBuffer is discarded, so
      // this function is still reentrant although it might not look
      // so.
      recvResult = ::recv(socketDescriptor, udpMessagePeekBuffer.data(),
                          udpMessagePeekBuffer.size(), MSG_PEEK);
      if (recvResult == -1 && errno == EINTR) {
         continue;
      }

      if (recvResult != (ssize_t) udpMessagePeekBuffer.size()) {
         break;
      }

      udpMessagePeekBuffer.resize(udpMessagePeekBuffer.size() * 2);
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::nativePendingDatagramSize() == %zd", recvResult);
#endif

   return qint64(recvResult);
}

qint64 QNativeSocketEnginePrivate::nativeReceiveDatagram(char *data, qint64 maxSize, QIpPacketHeader *header,
      QAbstractSocketEngine::PacketHeaderOptions options)
{
   // we use quintptr to force the alignment
   quintptr cbuf[(CMSG_SPACE(sizeof(struct in6_pktinfo)) + CMSG_SPACE(sizeof(int))
#if !defined(IP_PKTINFO) && defined(IP_RECVIF) && defined(Q_OS_BSD4)
                                                          + CMSG_SPACE(sizeof(sockaddr_dl))
#endif
                                                          + sizeof(quintptr) - 1) / sizeof(quintptr)];

   struct msghdr msg;
   struct iovec vec;
   qt_sockaddr aa;
   char c;
   memset(&msg, 0, sizeof(msg));
   memset(&aa, 0, sizeof(aa));

   // we need to receive at least one byte, even if our user isn't interested in it
   vec.iov_base = maxSize ? data : &c;
   vec.iov_len = maxSize ? maxSize : 1;
   msg.msg_iov = &vec;
   msg.msg_iovlen = 1;
   if (options & QAbstractSocketEngine::WantDatagramSender) {
      msg.msg_name = &aa;
      msg.msg_namelen = sizeof(aa);
   }
   if (options & (QAbstractSocketEngine::WantDatagramHopLimit | QAbstractSocketEngine::WantDatagramDestination)) {
      msg.msg_control = cbuf;
      msg.msg_controllen = sizeof(cbuf);
   }

   ssize_t recvResult = 0;
   do {
      recvResult = ::recvmsg(socketDescriptor, &msg, 0);
   } while (recvResult == -1 && errno == EINTR);

   if (recvResult == -1) {
      setError(QAbstractSocket::NetworkError, ReceiveDatagramErrorString);
      if (header) {
         header->clear();
      }
   } else if (options != QAbstractSocketEngine::WantNone) {
      Q_ASSERT(header);
      qt_socket_getPortAndAddress(&aa, &header->senderPort, &header->senderAddress);
      header->destinationPort = localPort;

      // parse the ancillary data
      struct cmsghdr *cmsgptr;
      for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL;
            cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) {
         if (cmsgptr->cmsg_level == IPPROTO_IPV6 && cmsgptr->cmsg_type == IPV6_PKTINFO
               && cmsgptr->cmsg_len >= CMSG_LEN(sizeof(in6_pktinfo))) {
            in6_pktinfo *info = reinterpret_cast<in6_pktinfo *>(CMSG_DATA(cmsgptr));

            header->destinationAddress.setAddress(reinterpret_cast<quint8 *>(&info->ipi6_addr));
            header->ifindex = info->ipi6_ifindex;
            if (header->ifindex) {
               header->destinationAddress.setScopeId(QString::number(info->ipi6_ifindex));
            }
         }

#ifdef IP_PKTINFO
         if (cmsgptr->cmsg_level == IPPROTO_IP && cmsgptr->cmsg_type == IP_PKTINFO
               && cmsgptr->cmsg_len >= CMSG_LEN(sizeof(in_pktinfo))) {
            in_pktinfo *info = reinterpret_cast<in_pktinfo *>(CMSG_DATA(cmsgptr));

            header->destinationAddress.setAddress(ntohl(info->ipi_addr.s_addr));
            header->ifindex = info->ipi_ifindex;
         }
#else

#  ifdef IP_RECVDSTADDR
         if (cmsgptr->cmsg_level == IPPROTO_IP && cmsgptr->cmsg_type == IP_RECVDSTADDR
               && cmsgptr->cmsg_len >= CMSG_LEN(sizeof(in_addr))) {
            in_addr *addr = reinterpret_cast<in_addr *>(CMSG_DATA(cmsgptr));

            header->destinationAddress.setAddress(ntohl(addr->s_addr));
         }
#  endif

#  if defined(IP_RECVIF) && defined(Q_OS_BSD4)
         if (cmsgptr->cmsg_level == IPPROTO_IP && cmsgptr->cmsg_type == IP_RECVIF
               && cmsgptr->cmsg_len >= CMSG_LEN(sizeof(sockaddr_dl))) {
            sockaddr_dl *sdl = reinterpret_cast<sockaddr_dl *>(CMSG_DATA(cmsgptr));

            header->ifindex = sdl->sdl_index;
         }
#  endif
#endif

         if (cmsgptr->cmsg_len == CMSG_LEN(sizeof(int))
               && ((cmsgptr->cmsg_level == IPPROTO_IPV6 && cmsgptr->cmsg_type == IPV6_HOPLIMIT)
                   || (cmsgptr->cmsg_level == IPPROTO_IP && cmsgptr->cmsg_type == IP_TTL))) {
            header->hopLimit = *reinterpret_cast<int *>(CMSG_DATA(cmsgptr));
         }
      }
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::nativeReceiveDatagram(%p \"%s\", %lli, %s, %i) == %lli",
          data, qt_prettyDebug(data, qMin(recvResult, ssize_t(16)), recvResult).data(), maxSize,
          (recvResult != -1 && options != QAbstractSocketEngine::WantNone)
          ? header->senderAddress.toString().toLatin1().constData() : "(unknown)",
          (recvResult != -1 && options != QAbstractSocketEngine::WantNone)
          ? header->senderPort : 0, (qint64) recvResult);
#endif

   return qint64(maxSize ? recvResult : recvResult == -1 ? -1 : 0);
}

qint64 QNativeSocketEnginePrivate::nativeSendDatagram(const char *data, qint64 len, const QIpPacketHeader &header)
{
   // use quintptr to force the alignment
   quintptr cbuf[(CMSG_SPACE(sizeof(struct in6_pktinfo)) + CMSG_SPACE(sizeof(int)) + sizeof(quintptr) - 1) / sizeof(quintptr)];

   struct cmsghdr *cmsgptr = reinterpret_cast<struct cmsghdr *>(cbuf);
   struct msghdr msg;
   struct iovec vec;
   qt_sockaddr aa;

   memset(&msg, 0, sizeof(msg));
   memset(&aa, 0, sizeof(aa));
   vec.iov_base = const_cast<char *>(data);
   vec.iov_len = len;
   msg.msg_iov = &vec;
   msg.msg_iovlen = 1;
   msg.msg_name = &aa.a;
   msg.msg_control = &cbuf;

   setPortAndAddress(header.destinationPort, header.destinationAddress, &aa, &msg.msg_namelen);

   if (msg.msg_namelen == sizeof(aa.a6)) {
      if (header.hopLimit != -1) {
         msg.msg_controllen += CMSG_SPACE(sizeof(int));
         cmsgptr->cmsg_len = CMSG_LEN(sizeof(int));
         cmsgptr->cmsg_level = IPPROTO_IPV6;
         cmsgptr->cmsg_type = IPV6_HOPLIMIT;
         memcpy(CMSG_DATA(cmsgptr), &header.hopLimit, sizeof(int));
         cmsgptr = reinterpret_cast<cmsghdr *>(reinterpret_cast<char *>(cmsgptr) + CMSG_SPACE(sizeof(int)));
      }

      if (header.ifindex != 0 || !header.senderAddress.isNull()) {
         struct in6_pktinfo *data = reinterpret_cast<in6_pktinfo *>(CMSG_DATA(cmsgptr));
         memset(data, 0, sizeof(*data));
         msg.msg_controllen += CMSG_SPACE(sizeof(*data));
         cmsgptr->cmsg_len = CMSG_LEN(sizeof(*data));
         cmsgptr->cmsg_level = IPPROTO_IPV6;
         cmsgptr->cmsg_type = IPV6_PKTINFO;
         data->ipi6_ifindex = header.ifindex;

         QIPv6Address tmp = header.senderAddress.toIPv6Address();
         memcpy(&data->ipi6_addr, &tmp, sizeof(tmp));
         cmsgptr = reinterpret_cast<cmsghdr *>(reinterpret_cast<char *>(cmsgptr) + CMSG_SPACE(sizeof(*data)));
      }

   } else {
      if (header.hopLimit != -1) {
         msg.msg_controllen += CMSG_SPACE(sizeof(int));
         cmsgptr->cmsg_len = CMSG_LEN(sizeof(int));
         cmsgptr->cmsg_level = IPPROTO_IP;
         cmsgptr->cmsg_type = IP_TTL;
         memcpy(CMSG_DATA(cmsgptr), &header.hopLimit, sizeof(int));
         cmsgptr = reinterpret_cast<cmsghdr *>(reinterpret_cast<char *>(cmsgptr) + CMSG_SPACE(sizeof(int)));
      }

#if defined(IP_PKTINFO) || defined(IP_SENDSRCADDR)
      if (header.ifindex != 0 || !header.senderAddress.isNull()) {
#  ifdef IP_PKTINFO
         struct in_pktinfo *data = reinterpret_cast<in_pktinfo *>(CMSG_DATA(cmsgptr));
         memset(data, 0, sizeof(*data));
         cmsgptr->cmsg_type = IP_PKTINFO;
         data->ipi_ifindex = header.ifindex;
         data->ipi_addr.s_addr = htonl(header.senderAddress.toIPv4Address());
#  elif defined(IP_SENDSRCADDR)
         struct in_addr *data = reinterpret_cast<in_addr *>(CMSG_DATA(cmsgptr));
         cmsgptr->cmsg_type = IP_SENDSRCADDR;
         data->s_addr = htonl(header.senderAddress.toIPv4Address());
#  endif
         cmsgptr->cmsg_level = IPPROTO_IP;
         msg.msg_controllen += CMSG_SPACE(sizeof(*data));
         cmsgptr->cmsg_len = CMSG_LEN(sizeof(*data));
         cmsgptr = reinterpret_cast<cmsghdr *>(reinterpret_cast<char *>(cmsgptr) + CMSG_SPACE(sizeof(*data)));
      }
#endif

   }

   if (msg.msg_controllen == 0) {
      msg.msg_control = 0;
   }

   ssize_t sentBytes = qt_safe_sendmsg(socketDescriptor, &msg, 0);

   if (sentBytes < 0) {
      switch (errno) {
         case EMSGSIZE:
            setError(QAbstractSocket::DatagramTooLargeError, DatagramTooLargeErrorString);
            break;

         default:
            setError(QAbstractSocket::NetworkError, SendDatagramErrorString);
      }
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEngine::sendDatagram(%p \"%s\", %lli, \"%s\", %i) == %lli", data,
          qt_prettyDebug(data, qMin<int>(len, 16), len).data(), len,
          header.destinationAddress.toString().toLatin1().constData(),
          header.destinationPort, (qint64) sentBytes);
#endif

   return qint64(sentBytes);
}

bool QNativeSocketEnginePrivate::fetchConnectionParameters()
{
   localPort = 0;
   localAddress.clear();
   peerPort = 0;
   peerAddress.clear();

   if (socketDescriptor == -1) {
      return false;
   }

   qt_sockaddr sa;
   QT_SOCKLEN_T sockAddrSize = sizeof(sa);

   // Determine local address
   memset(&sa, 0, sizeof(sa));
   if (::getsockname(socketDescriptor, &sa.a, &sockAddrSize) == 0) {
      qt_socket_getPortAndAddress(&sa, &localPort, &localAddress);

      // Determine protocol family
      switch (sa.a.sa_family) {
         case AF_INET:
            socketProtocol = QAbstractSocket::IPv4Protocol;
            break;

         case AF_INET6:
            socketProtocol = QAbstractSocket::IPv6Protocol;
            break;

         default:
            socketProtocol = QAbstractSocket::UnknownNetworkLayerProtocol;
            break;
      }

   } else if (errno == EBADF) {
      setError(QAbstractSocket::UnsupportedSocketOperationError, InvalidSocketErrorString);
      return false;
   }

#if defined (IPV6_V6ONLY)
   // determine if local address is dual mode
   // On linux, these are returned as "::" (==AnyIPv6)
   // On OSX, these are returned as "::FFFF:0.0.0.0" (==AnyIPv4)
   // in either case, the IPV6_V6ONLY option is cleared
   int ipv6only = 0;
   socklen_t optlen = sizeof(ipv6only);
   if (socketProtocol == QAbstractSocket::IPv6Protocol
         && (localAddress == QHostAddress::AnyIPv4 || localAddress == QHostAddress::AnyIPv6)
         && !getsockopt(socketDescriptor, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, &optlen )) {
      if (optlen != sizeof(ipv6only)) {
         qWarning("unexpected size of IPV6_V6ONLY socket option");
      }
      if (!ipv6only) {
         socketProtocol = QAbstractSocket::AnyIPProtocol;
         localAddress = QHostAddress::Any;
      }
   }
#endif

   // Determine the remote address
   if (!::getpeername(socketDescriptor, &sa.a, &sockAddrSize)) {
      qt_socket_getPortAndAddress(&sa, &peerPort, &peerAddress);
   }

   // Determine the socket type (UDP/TCP)
   int value = 0;
   QT_SOCKOPTLEN_T valueSize = sizeof(int);
   if (::getsockopt(socketDescriptor, SOL_SOCKET, SO_TYPE, &value, &valueSize) == 0) {
      if (value == SOCK_STREAM) {
         socketType = QAbstractSocket::TcpSocket;
      } else if (value == SOCK_DGRAM) {
         socketType = QAbstractSocket::UdpSocket;
      } else {
         socketType = QAbstractSocket::UnknownSocketType;
      }
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   QString socketProtocolStr = "UnknownProtocol";

   if (socketProtocol == QAbstractSocket::IPv4Protocol) {
      socketProtocolStr = "IPv4Protocol";

   } else if (socketProtocol == QAbstractSocket::IPv6Protocol || socketProtocol == QAbstractSocket::AnyIPProtocol) {
      socketProtocolStr = "IPv6Protocol";
   }

   QString socketTypeStr = "UnknownSocketType";
   if (socketType == QAbstractSocket::TcpSocket) {
      socketTypeStr = "TcpSocket";

   } else if (socketType == QAbstractSocket::UdpSocket) {
      socketTypeStr = "UdpSocket";
   }

   qDebug("QNativeSocketEnginePrivate::fetchConnectionParameters() local == %s:%i,"
          " peer == %s:%i, socket == %s - %s",
          localAddress.toString().toLatin1().constData(), localPort,
          peerAddress.toString().toLatin1().constData(), peerPort, socketTypeStr.toLatin1().constData(),
          socketProtocolStr.toLatin1().constData());
#endif
   return true;
}

void QNativeSocketEnginePrivate::nativeClose()
{
#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEngine::nativeClose()");
#endif

   qt_safe_close(socketDescriptor);
}

qint64 QNativeSocketEnginePrivate::nativeWrite(const char *data, qint64 len)
{
   Q_Q(QNativeSocketEngine);

   ssize_t writtenBytes;
   writtenBytes = qt_safe_write_nosignal(socketDescriptor, data, len);

   if (writtenBytes < 0) {
      switch (errno) {
         case EPIPE:
         case ECONNRESET:
            writtenBytes = -1;
            setError(QAbstractSocket::RemoteHostClosedError, RemoteHostClosedErrorString);
            q->close();
            break;
         case EAGAIN:
            writtenBytes = 0;
            break;
         case EMSGSIZE:
            setError(QAbstractSocket::DatagramTooLargeError, DatagramTooLargeErrorString);
            break;
         default:
            break;
      }
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::nativeWrite(%p \"%s\", %llu) == %i",
          data, qt_prettyDebug(data, qMin((int) len, 16),
                               (int) len).data(), len, (int) writtenBytes);
#endif

   return qint64(writtenBytes);
}
/*
*/
qint64 QNativeSocketEnginePrivate::nativeRead(char *data, qint64 maxSize)
{
   Q_Q(QNativeSocketEngine);
   if (!q->isValid()) {
      qWarning("QNativeSocketEngine::nativeRead: Invalid socket");
      return -1;
   }

   ssize_t r = 0;
   r = qt_safe_read(socketDescriptor, data, maxSize);

   if (r < 0) {
      r = -1;
      switch (errno) {
#if EWOULDBLOCK-0 && EWOULDBLOCK != EAGAIN
         case EWOULDBLOCK:
#endif
         case EAGAIN:
            // No data was available for reading
            r = -2;
            break;
         case EBADF:
         case EINVAL:
         case EIO:
            //error string is now set in read(), not here in nativeRead()
            break;

         case ECONNRESET:
            r = 0;
            break;

         default:
            break;
      }
   }

#if defined (QNATIVESOCKETENGINE_DEBUG)
   qDebug("QNativeSocketEnginePrivate::nativeRead(%p \"%s\", %llu) == %zd",
          data, qt_prettyDebug(data, qMin(r, ssize_t(16)), r).data(),
          maxSize, r);
#endif

   return qint64(r);
}

int QNativeSocketEnginePrivate::nativeSelect(int timeout, bool selectForRead) const
{
   fd_set fds;
   FD_ZERO(&fds);
   FD_SET(socketDescriptor, &fds);

   struct timespec tv;
   tv.tv_sec = timeout / 1000;
   tv.tv_nsec = (timeout % 1000) * 1000 * 1000;

   int retval;
   if (selectForRead) {
      retval = qt_safe_select(socketDescriptor + 1, &fds, 0, 0, timeout < 0 ? 0 : &tv);
   } else {
      retval = qt_safe_select(socketDescriptor + 1, 0, &fds, 0, timeout < 0 ? 0 : &tv);
   }

   return retval;
}

int QNativeSocketEnginePrivate::nativeSelect(int timeout, bool checkRead, bool checkWrite,
      bool *selectForRead, bool *selectForWrite) const
{
   fd_set fdread;
   FD_ZERO(&fdread);
   if (checkRead) {
      FD_SET(socketDescriptor, &fdread);
   }

   fd_set fdwrite;
   FD_ZERO(&fdwrite);
   if (checkWrite) {
      FD_SET(socketDescriptor, &fdwrite);
   }

   struct timespec tv;
   tv.tv_sec  = timeout / 1000;
   tv.tv_nsec = (timeout % 1000) * 1000 * 1000;

   int ret;
   ret = qt_safe_select(socketDescriptor + 1, &fdread, &fdwrite, 0, timeout < 0 ? 0 : &tv);

   if (ret <= 0) {
      return ret;
   }

   *selectForRead  = FD_ISSET(socketDescriptor, &fdread);
   *selectForWrite = FD_ISSET(socketDescriptor, &fdwrite);

   return ret;
}

