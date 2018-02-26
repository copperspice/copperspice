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

#include <qhostaddress.h>
#include <qhostaddress_p.h>
#include <qipaddress_p.h>
#include <qdebug.h>

#if defined(Q_OS_WIN)
# include <winsock2.h>
#else
# include <netinet/in.h>
#endif

#include <qplatformdefs.h>
#include <qstringlist.h>
#include <qendian.h>

#ifndef QT_NO_DATASTREAM
#include <qdatastream.h>
#endif

#ifdef __SSE2__
#  include <qsimd_p.h>
#endif

#ifdef QT_LINUXBASE
#  include <arpa/inet.h>
#endif

QT_BEGIN_NAMESPACE

#define QT_ENSURE_PARSED(a) \
    do { \
        if (! (a)->d->isParsed)  { \
           (a)->d->parse(); \
        } \
    } while (0)

#ifdef Q_OS_WIN

// sockaddr_in6 size changed between old and new SDK
// Only the new version is the correct one, so always
// use this structure.

struct qt_in6_addr {
   u_char qt_s6_addr[16];
};

typedef struct {
   short   sin6_family;            /* AF_INET6 */
   u_short sin6_port;              /* Transport level port number */
   u_long  sin6_flowinfo;          /* IPv6 flow information */
   struct  qt_in6_addr sin6_addr;  /* IPv6 address */
   u_long  sin6_scope_id;          /* set of interfaces for a scope */
} qt_sockaddr_in6;

#else
#define qt_sockaddr_in6 sockaddr_in6
#define qt_s6_addr s6_addr
#endif


class QHostAddressPrivate
{
 public:
   QHostAddressPrivate();

   void setAddress(quint32 a_ = 0);
   void setAddress(const quint8 *a_);
   void setAddress(const Q_IPV6ADDR &a_);

   bool parse();
   void clear();

   QString ipString;
   QString scopeId;

   quint32 a;    // IPv4 address

   union {
      Q_IPV6ADDR a6; // IPv6 address
      struct { quint64 c[2]; } a6_64;
      struct { quint32 c[4]; } a6_32;
   };

   QAbstractSocket::NetworkLayerProtocol protocol;

   bool isParsed;


   friend class QHostAddress;
};

QHostAddressPrivate::QHostAddressPrivate()
   : a(0), protocol(QAbstractSocket::UnknownNetworkLayerProtocol), isParsed(true)
{
   memset(&a6, 0, sizeof(a6));
}

void QHostAddressPrivate::setAddress(quint32 a_)
{
   a = a_;
   protocol = QAbstractSocket::IPv4Protocol;
   isParsed = true;

   //create mapped address, except for a_ == 0 (any)
   a6_64.c[0] = 0;

   if (a) {
      a6_32.c[2] = qToBigEndian(0xffff);
      a6_32.c[3] = qToBigEndian(a);
   } else {
      a6_64.c[1] = 0;
   }
}

/// parses v4-mapped addresses or the AnyIPv6 address and stores in \a a;
/// returns true if the address was one of those
static bool convertToIpv4(quint32 &a, const Q_IPV6ADDR &a6)
{
   const uchar *ptr = a6.c;
   if (qFromUnaligned<quint64>(ptr) != 0) {
      return false;
   }
   if (qFromBigEndian<quint32>(ptr + 8) == 0) {
      // is it AnyIPv6?
      a = 0;
      return qFromBigEndian<quint32>(ptr + 12) == 0;
   }
   if (qFromBigEndian<quint32>(ptr + 8) != 0xFFFF) {
      return false;
   }
   a = qFromBigEndian<quint32>(ptr + 12);
   return true;
}

void QHostAddressPrivate::setAddress(const quint8 *a_)
{
   protocol = QAbstractSocket::IPv6Protocol;
   isParsed = true;
   memcpy(a6.c, a_, sizeof(a6));
   a = 0;
   convertToIpv4(a, a6);
}

void QHostAddressPrivate::setAddress(const Q_IPV6ADDR &a_)
{
   setAddress(a_.c);
}

static bool parseIp6(const QString &address, QIPAddressUtils::IPv6Address &addr, QString *scopeId)
{
   QString tmp = address;
   int scopeIdPos = tmp.lastIndexOf(QLatin1Char('%'));
   if (scopeIdPos != -1) {
      *scopeId = tmp.mid(scopeIdPos + 1);
      tmp.chop(tmp.size() - scopeIdPos);
   } else {
      scopeId->clear();
   }

   return QIPAddressUtils::parseIp6(addr, tmp.constBegin(), tmp.constEnd()) == 0;
}

bool QHostAddressPrivate::parse()
{
   isParsed = true;
   protocol = QAbstractSocket::UnknownNetworkLayerProtocol;
   QString a = ipString.simplified();

   if (a.isEmpty()) {
      return false;
   }

   // All IPv6 addresses contain a ':', and may contain a '.'.
   if (a.contains(QLatin1Char(':'))) {
      quint8 maybeIp6[16];
      if (parseIp6(a, maybeIp6, &scopeId)) {
         setAddress(maybeIp6);
         return true;
      }
   }

   quint32 maybeIp4 = 0;
   if (QIPAddressUtils::parseIp4(maybeIp4, a.constBegin(), a.constEnd())) {
      setAddress(maybeIp4);
      return true;
   }
   return false;
}

void QHostAddressPrivate::clear()
{
   a = 0;
   protocol = QAbstractSocket::UnknownNetworkLayerProtocol;
   isParsed = true;
   memset(&a6, 0, sizeof(a6));
}


bool QNetmaskAddress::setAddress(const QString &address)
{
   length = -1;
   QHostAddress other;
   return other.setAddress(address) && setAddress(other);
}

bool QNetmaskAddress::setAddress(const QHostAddress &address)
{
   static const quint8 zeroes[16] = { 0 };
   union {
      quint32 v4;
      quint8 v6[16];
   } ip;

   int netmask = 0;
   quint8 *ptr = ip.v6;
   quint8 *end;
   length = -1;

   QHostAddress::operator=(address);

   if (d->protocol == QAbstractSocket::IPv4Protocol) {
      ip.v4 = qToBigEndian(d->a);
      end = ptr + 4;
   } else if (d->protocol == QAbstractSocket::IPv6Protocol) {
      memcpy(ip.v6, d->a6.c, 16);
      end = ptr + 16;
   } else {
      d->clear();
      return false;
   }

   while (ptr < end) {
      switch (*ptr) {
         case 255:
            netmask += 8;
            ++ptr;
            continue;

         default:
            d->clear();
            return false;       // invalid IP-style netmask

         // the rest always falls through
         case 254:
            ++netmask;
         case 252:
            ++netmask;
         case 248:
            ++netmask;
         case 240:
            ++netmask;
         case 224:
            ++netmask;
         case 192:
            ++netmask;
         case 128:
            ++netmask;
         case 0:
            break;
      }
      break;
   }

   // confirm that the rest is only zeroes
   if (ptr < end && memcmp(ptr + 1, zeroes, end - ptr - 1) != 0) {
      d->clear();
      return false;
   }

   length = netmask;
   return true;
}

static void clearBits(quint8 *where, int start, int end)
{
   Q_ASSERT(end == 32 || end == 128);
   if (start == end) {
      return;
   }

   // for the byte where 'start' is, clear the lower bits only
   quint8 bytemask = 256 - (1 << (8 - (start & 7)));
   where[start / 8] &= bytemask;

   // for the tail part, clear everything
   memset(where + (start + 7) / 8, 0, end / 8 - (start + 7) / 8);
}

int QNetmaskAddress::prefixLength() const
{
   return length;
}

void QNetmaskAddress::setPrefixLength(QAbstractSocket::NetworkLayerProtocol proto, int newLength)
{
   length = newLength;
   if (length < 0 || length > (proto == QAbstractSocket::IPv4Protocol ? 32 :
                               proto == QAbstractSocket::IPv6Protocol ? 128 : -1)) {
      // invalid information, reject
      d->protocol = QAbstractSocket::UnknownNetworkLayerProtocol;
      length = -1;
      return;
   }

   d->protocol = proto;
   if (d->protocol == QAbstractSocket::IPv4Protocol) {
      if (length == 0) {
         d->a = 0;
      } else if (length == 32) {
         d->a = quint32(0xffffffff);
      } else {
         d->a = quint32(0xffffffff) >> (32 - length) << (32 - length);
      }
   } else {
      memset(d->a6.c, 0xFF, sizeof(d->a6));
      clearBits(d->a6.c, length, 128);
   }
}





QHostAddress::QHostAddress()
   : d(new QHostAddressPrivate)
{
}

/*!
    Constructs a host address object with the IPv4 address \a ip4Addr.
*/
QHostAddress::QHostAddress(quint32 ip4Addr)
   : d(new QHostAddressPrivate)
{
   setAddress(ip4Addr);
}


QHostAddress::QHostAddress(const quint8 *ip6Addr)
   : d(new QHostAddressPrivate)
{
   setAddress(ip6Addr);
}


QHostAddress::QHostAddress(const Q_IPV6ADDR &ip6Addr)
   : d(new QHostAddressPrivate)
{
   setAddress(ip6Addr);
}


QHostAddress::QHostAddress(const QString &address)
   : d(new QHostAddressPrivate)
{
   d->ipString = address;
   d->isParsed = false;
}


QHostAddress::QHostAddress(const struct sockaddr *sockaddr)
   : d(new QHostAddressPrivate)
{
   if (sockaddr->sa_family == AF_INET) {
      setAddress(htonl(((const sockaddr_in *)sockaddr)->sin_addr.s_addr));

   } else if (sockaddr->sa_family == AF_INET6) {
      setAddress(((const qt_sockaddr_in6 *)sockaddr)->sin6_addr.qt_s6_addr);
   }
}


QHostAddress::QHostAddress(const QHostAddress &address)
   : d(new QHostAddressPrivate(*address.d.data()))
{
}

/*!
    Constructs a QHostAddress object for \a address.
*/
QHostAddress::QHostAddress(SpecialAddress address)
   : d(new QHostAddressPrivate)
{
   Q_IPV6ADDR ip6;
   memset(&ip6, 0, sizeof ip6);
   quint32 ip4 = INADDR_ANY;
   switch (address) {
      case Null:
         return;
      case Broadcast:
         ip4 = INADDR_BROADCAST;
         break;

      case LocalHost:
         ip4 = INADDR_LOOPBACK;
         break;

      case AnyIPv4:
         break;

      case LocalHostIPv6:
         ip6[15] = 1;
      // fall through
      case AnyIPv6:
         d->setAddress(ip6);
         return;
      case Any:
         d->protocol = QAbstractSocket::AnyIPProtocol;
         return;
   }
   // common IPv4 part
   d->setAddress(ip4);
}

QHostAddress::~QHostAddress()
{
}

QHostAddress &QHostAddress::operator=(const QHostAddress &address)
{
   *d.data() = *address.d.data();
   return *this;
}

QHostAddress &QHostAddress::operator=(const QString &address)
{
   setAddress(address);
   return *this;
}

void QHostAddress::clear()
{
   d->clear();
}

void QHostAddress::setAddress(quint32 ip4Addr)
{
   d->setAddress(ip4Addr);
}

void QHostAddress::setAddress(const quint8 *ip6Addr)
{
   d->setAddress(ip6Addr);
}

void QHostAddress::setAddress(const Q_IPV6ADDR &ip6Addr)
{
   d->setAddress(ip6Addr);
}

bool QHostAddress::setAddress(const QString &address)
{
   d->ipString = address;
   return d->parse();
}

void QHostAddress::setAddress(const struct sockaddr *sockaddr)
{
   clear();
   if (sockaddr->sa_family == AF_INET) {
      setAddress(htonl(((const sockaddr_in *)sockaddr)->sin_addr.s_addr));


   } else if (sockaddr->sa_family == AF_INET6) {
      setAddress(((const qt_sockaddr_in6 *)sockaddr)->sin6_addr.qt_s6_addr);
   }

}

quint32 QHostAddress::toIPv4Address(bool *ok) const
{
   QT_ENSURE_PARSED(this);
   quint32 dummy;

   if (ok) {
      *ok = d->protocol == QAbstractSocket::IPv4Protocol || d->protocol == QAbstractSocket::AnyIPProtocol ||
            (d->protocol == QAbstractSocket::IPv6Protocol && convertToIpv4(dummy, d->a6));
   }

   return d->a;
}

QAbstractSocket::NetworkLayerProtocol QHostAddress::protocol() const
{
   QT_ENSURE_PARSED(this);
   return d->protocol;
}

Q_IPV6ADDR QHostAddress::toIPv6Address() const
{
   QT_ENSURE_PARSED(this);
   return d->a6;
}

QString QHostAddress::toString() const
{
   QT_ENSURE_PARSED(this);
   if (d->protocol == QAbstractSocket::IPv4Protocol
         || d->protocol == QAbstractSocket::AnyIPProtocol) {

      quint32 i = toIPv4Address();
      QString s;
      QIPAddressUtils::toString(s, i);
      return s;
   }

   if (d->protocol == QAbstractSocket::IPv6Protocol) {
      QString s;
      QIPAddressUtils::toString(s, d->a6.c);

      if (! d->scopeId.isEmpty()) {
         s.append(QLatin1Char('%') + d->scopeId);
      }

      return s;
   }

   return QString();
}

QString QHostAddress::scopeId() const
{
   QT_ENSURE_PARSED(this);
   return (d->protocol == QAbstractSocket::IPv6Protocol) ? d->scopeId : QString();
}

void QHostAddress::setScopeId(const QString &id)
{
   QT_ENSURE_PARSED(this);
   if (d->protocol == QAbstractSocket::IPv6Protocol) {
      d->scopeId = id;
   }
}

bool QHostAddress::operator==(const QHostAddress &other) const
{
   QT_ENSURE_PARSED(this);
   QT_ENSURE_PARSED(&other);

   if (d->protocol == QAbstractSocket::IPv4Protocol) {
      return other.d->protocol == QAbstractSocket::IPv4Protocol && d->a == other.d->a;
   }

   if (d->protocol == QAbstractSocket::IPv6Protocol) {
      return other.d->protocol == QAbstractSocket::IPv6Protocol
             && memcmp(&d->a6, &other.d->a6, sizeof(Q_IPV6ADDR)) == 0;
   }
   return d->protocol == other.d->protocol;
}

bool QHostAddress::operator ==(SpecialAddress other) const
{
   QT_ENSURE_PARSED(this);

   quint32 ip4 = INADDR_ANY;
   switch (other) {
      case Null:
         return d->protocol == QAbstractSocket::UnknownNetworkLayerProtocol;

      case Broadcast:
         ip4 = INADDR_BROADCAST;
         break;

      case LocalHost:
         ip4 = INADDR_LOOPBACK;
         break;

      case Any:
         return d->protocol == QAbstractSocket::AnyIPProtocol;

      case AnyIPv4:
         break;

      case LocalHostIPv6:
      case AnyIPv6:
         if (d->protocol == QAbstractSocket::IPv6Protocol) {
            quint64 second = quint8(other == LocalHostIPv6);  // 1 for localhost, 0 for any
            return d->a6_64.c[0] == 0 && d->a6_64.c[1] == qToBigEndian(second);
         }
         return false;
   }

   // common IPv4 part
   return d->protocol == QAbstractSocket::IPv4Protocol && d->a == ip4;
}

bool QHostAddress::isNull() const
{
   QT_ENSURE_PARSED(this);
   return d->protocol == QAbstractSocket::UnknownNetworkLayerProtocol;
}

bool QHostAddress::isInSubnet(const QHostAddress &subnet, int netmask) const
{
   QT_ENSURE_PARSED(this);
   if (subnet.protocol() != d->protocol || netmask < 0) {
      return false;
   }

   union {
      quint32 ip;
      quint8 data[4];
   } ip4, net4;
   const quint8 *ip;
   const quint8 *net;

   if (d->protocol == QAbstractSocket::IPv4Protocol) {
      if (netmask > 32) {
         netmask = 32;
      }
      ip4.ip = qToBigEndian(d->a);
      net4.ip = qToBigEndian(subnet.d->a);
      ip = ip4.data;
      net = net4.data;

   } else if (d->protocol == QAbstractSocket::IPv6Protocol) {
      if (netmask > 128) {
         netmask = 128;
      }
      ip = d->a6.c;
      net = subnet.d->a6.c;
   } else {
      return false;
   }

   if (netmask >= 8 && memcmp(ip, net, netmask / 8) != 0) {
      return false;
   }
   if ((netmask & 7) == 0) {
      return true;
   }

   // compare the last octet now
   quint8 bytemask = 256 - (1 << (8 - (netmask & 7)));
   quint8 ipbyte = ip[netmask / 8];
   quint8 netbyte = net[netmask / 8];
   return (ipbyte & bytemask) == (netbyte & bytemask);
}

bool QHostAddress::isInSubnet(const QPair<QHostAddress, int> &subnet) const
{
   return isInSubnet(subnet.first, subnet.second);
}

QPair<QHostAddress, int> QHostAddress::parseSubnet(const QString &subnet)
{
   // We support subnets in the form:
   //   ddd.ddd.ddd.ddd/nn
   //   ddd.ddd.ddd/nn
   //   ddd.ddd/nn
   //   ddd/nn
   //   ddd.ddd.ddd.
   //   ddd.ddd.ddd
   //   ddd.ddd.
   //   ddd.ddd
   //   ddd.
   //   ddd
   //   <ipv6-address>/nn
   //
   //  where nn can be an IPv4-style netmask for the IPv4 forms

   const QPair<QHostAddress, int> invalid = qMakePair(QHostAddress(), -1);
   if (subnet.isEmpty()) {
      return invalid;
   }

   int slash = subnet.indexOf(QLatin1Char('/'));
   QString netStr = subnet;
   if (slash != -1) {
      netStr.truncate(slash);
   }

   int netmask = -1;
   bool isIpv6 = netStr.contains(QLatin1Char(':'));

   if (slash != -1) {
      // is the netmask given in IP-form or in bit-count form?
      if (!isIpv6 && subnet.indexOf(QLatin1Char('.'), slash + 1) != -1) {
         // IP-style, convert it to bit-count form
         QNetmaskAddress parser;
         if (!parser.setAddress(subnet.mid(slash + 1))) {
            return invalid;
         }
         netmask = parser.prefixLength();
      } else {
         bool ok;
         netmask = subnet.mid(slash + 1).toUInt(&ok);
         if (!ok) {
            return invalid;   // failed to parse the subnet
         }
      }
   }

   if (isIpv6) {
      // looks like it's an IPv6 address
      if (netmask > 128) {
         return invalid;   // invalid netmask
      }
      if (netmask < 0) {
         netmask = 128;
      }

      QHostAddress net;
      if (!net.setAddress(netStr)) {
         return invalid;   // failed to parse the IP
      }

      clearBits(net.d->a6.c, netmask, 128);
      return qMakePair(net, netmask);
   }

   if (netmask > 32) {
      return invalid;   // invalid netmask
   }

   // parse the address manually
   QStringList parts = netStr.split(QLatin1Char('.'));
   if (parts.isEmpty() || parts.count() > 4) {
      return invalid;   // invalid IPv4 address
   }

   if (parts.last().isEmpty()) {
      parts.removeLast();
   }

   quint32 addr = 0;
   for (int i = 0; i < parts.count(); ++i) {
      bool ok;
      uint byteValue = parts.at(i).toUInt(&ok);
      if (!ok || byteValue > 255) {
         return invalid;   // invalid IPv4 address
      }

      addr <<= 8;
      addr += byteValue;
   }
   addr <<= 8 * (4 - parts.count());
   if (netmask == -1) {
      netmask = 8 * parts.count();
   } else if (netmask == 0) {
      // special case here
      // x86's instructions "shr" and "shl" do not operate when
      // their argument is 32, so the code below doesn't work as expected
      addr = 0;
   } else if (netmask != 32) {
      // clear remaining bits
      quint32 mask = quint32(0xffffffff) >> (32 - netmask) << (32 - netmask);
      addr &= mask;
   }

   return qMakePair(QHostAddress(addr), netmask);
}

bool QHostAddress::isLoopback() const
{
   QT_ENSURE_PARSED(this);

   if ((d->a & 0xFF000000) == 0x7F000000)  {
      return true; // v4 range (including IPv6 wrapped IPv4 addresses)
   }

   if (d->protocol == QAbstractSocket::IPv6Protocol) {

      if (d->a6_64.c[0] != 0 || qFromBigEndian(d->a6_64.c[1]) != 1) {
         return false;
      }

      return true;
   }

   return false;
}
bool QHostAddress::isMulticast() const
{
   QT_ENSURE_PARSED(this);
   if ((d->a & 0xF0000000) == 0xE0000000) {
      return true; // 224.0.0.0-239.255.255.255 (including v4-mapped IPv6 addresses)
   }

   if (d->protocol == QAbstractSocket::IPv6Protocol) {
      return d->a6.c[0] == 0xff;
   }

   return false;
}
QDebug operator<<(QDebug d, const QHostAddress &address)
{
   // QDebugStateSaver saver(d);
   // d.resetFormat().nospace();

   if (address == QHostAddress::Any) {
      d << "QHostAddress(QHostAddress::Any)";
   } else {
      d << "QHostAddress(" << address.toString() << ')';
   }

   return d;
}

uint qHash(const QHostAddress &key, uint seed)
{
   QT_ENSURE_PARSED(&key);
   return qHashBits(key.d->a6.c, 16, seed);
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &out, const QHostAddress &address)
{
   qint8 prot;
   prot = qint8(address.protocol());
   out << prot;
   switch (address.protocol()) {
      case QAbstractSocket::UnknownNetworkLayerProtocol:
      case QAbstractSocket::AnyIPProtocol:
         break;

      case QAbstractSocket::IPv4Protocol:
         out << address.toIPv4Address();
         break;

      case QAbstractSocket::IPv6Protocol: {
         Q_IPV6ADDR ipv6 = address.toIPv6Address();
         for (int i = 0; i < 16; ++i) {
            out << ipv6[i];
         }
         out << address.scopeId();
      }
      break;
   }
   return out;
}

QDataStream &operator>>(QDataStream &in, QHostAddress &address)
{
   qint8 prot;
   in >> prot;
   switch (QAbstractSocket::NetworkLayerProtocol(prot)) {
      case QAbstractSocket::UnknownNetworkLayerProtocol:
         address.clear();
         break;

      case QAbstractSocket::IPv4Protocol: {
         quint32 ipv4;
         in >> ipv4;
         address.setAddress(ipv4);
      }
      break;

      case QAbstractSocket::IPv6Protocol: {
         Q_IPV6ADDR ipv6;
         for (int i = 0; i < 16; ++i) {
            in >> ipv6[i];
         }
         address.setAddress(ipv6);

         QString scope;
         in >> scope;
         address.setScopeId(scope);
      }
      break;
      case QAbstractSocket::AnyIPProtocol:
         address = QHostAddress::Any;
         break;
      default:
         address.clear();
         in.setStatus(QDataStream::ReadCorruptData);
   }
   return in;
}

#endif //QT_NO_DATASTREAM

QT_END_NAMESPACE
