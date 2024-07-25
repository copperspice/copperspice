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

#include <qhostinfo_p.h>

#include <qurl.h>

#include <qmutexpool_p.h>
#include <qnativesocketengine_p.h>
#include <qsystemlibrary_p.h>

#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>

// Older SDKs do not include the addrinfo struct declaration, so we
// include a copy of it here.
struct qt_addrinfo {
   int ai_flags;
   int ai_family;
   int ai_socktype;
   int ai_protocol;
   size_t ai_addrlen;
   char *ai_canonname;
   sockaddr *ai_addr;
   qt_addrinfo *ai_next;
};

#ifndef NI_MAXHOST // already defined to 1025 in ws2tcpip.h?
#define NI_MAXHOST 1024
#endif

typedef int (__stdcall *getnameinfoProto)(const sockaddr *, int, const char *, DWORD, const char *, DWORD, int);
typedef int (__stdcall *getaddrinfoProto)(const char *, const char *, const qt_addrinfo *, qt_addrinfo **);
typedef int (__stdcall *freeaddrinfoProto)(qt_addrinfo *);

static getnameinfoProto local_getnameinfo   = nullptr;
static getaddrinfoProto local_getaddrinfo   = nullptr;
static freeaddrinfoProto local_freeaddrinfo = nullptr;

static void resolveLibraryInternal()
{
   // Attempt to resolve getaddrinfo(); without it we'll have to fall
   // back to gethostbyname(), which has no IPv6 support.

   local_getaddrinfo  = (getaddrinfoProto) QSystemLibrary::resolve(QLatin1String("ws2_32"), "getaddrinfo");
   local_freeaddrinfo = (freeaddrinfoProto) QSystemLibrary::resolve(QLatin1String("ws2_32"), "freeaddrinfo");
   local_getnameinfo  = (getnameinfoProto) QSystemLibrary::resolve(QLatin1String("ws2_32"), "getnameinfo");

}

static void translateWSAError(int error, QHostInfo *results)
{
   switch (error) {
      case WSAHOST_NOT_FOUND: // authoritative not found
      case WSATRY_AGAIN:      // non authoritative not found
      case WSANO_DATA:        // valid name, no associated address
         results->setError(QHostInfo::HostNotFound);
         results->setErrorString(QHostInfoAgent::tr("Host not found"));
         return;

      default:
         results->setError(QHostInfo::UnknownError);
         results->setErrorString(QHostInfoAgent::tr("Unknown error (%1)").formatArg(error));
         return;
   }
}

QHostInfo QHostInfoAgent::fromName(const QString &hostName)
{
   QSysInfo::machineHostName();        // this initializes ws2_32.dll

   // Load res_init on demand
   static std::atomic<bool> triedResolve(false);

   if (! triedResolve.load()) {
      QRecursiveMutexLocker locker(QMutexPool::globalInstanceGet(&local_getaddrinfo));

      if (! triedResolve.load()) {
         resolveLibraryInternal();
         triedResolve.store(true);
      }
   }

   QHostInfo results;
   QHostAddress address;

   if (address.setAddress(hostName)) {
      // Reverse lookup
      if (local_getnameinfo) {
         sockaddr_in  sa4;
         sockaddr_in6 sa6;
         sockaddr *sa;

         int saSize;

         if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            sa     = (sockaddr *)&sa4;
            saSize = sizeof(sa4);
            memset(&sa4, 0, sizeof(sa4));

            sa4.sin_family = AF_INET;
            sa4.sin_addr.s_addr = htonl(address.toIPv4Address());

         } else {
            sa     = (sockaddr *)&sa6;
            saSize = sizeof(sa6);
            memset(&sa6, 0, sizeof(sa6));

            sa6.sin6_family = AF_INET6;
            memcpy(&sa6.sin6_addr, address.toIPv6Address().c, sizeof(sa6.sin6_addr));
         }

         char hbuf[NI_MAXHOST];
         if (local_getnameinfo(sa, saSize, hbuf, sizeof(hbuf), nullptr, 0, 0) == 0) {
            results.setHostName(QString::fromLatin1(hbuf));
         }

      } else {
         unsigned long addr = inet_addr(hostName.toLatin1().constData());
         struct hostent *ent = gethostbyaddr((const char *)&addr, sizeof(addr), AF_INET);

         if (ent) {
            results.setHostName(QString::fromLatin1(ent->h_name));
         }
      }

      if (results.hostName().isEmpty()) {
         results.setHostName(address.toString());
      }

      results.setAddresses(QList<QHostAddress>() << address);

      return results;
   }

   // IDN support
   QByteArray aceHostname = QUrl::toAce(hostName);
   results.setHostName(hostName);

   if (aceHostname.isEmpty()) {
      results.setError(QHostInfo::HostNotFound);
      results.setErrorString(hostName.isEmpty() ? tr("No host name given") : tr("Invalid hostname"));
      return results;
   }

   if (local_getaddrinfo && local_freeaddrinfo) {
      // Call getaddrinfo, and place all IPv4 addresses at the start
      // and the IPv6 addresses at the end of the address list in results.
      qt_addrinfo *res;
      int err = local_getaddrinfo(aceHostname.constData(), nullptr, nullptr, &res);

      if (err == 0) {
         QList<QHostAddress> addresses;

         for (qt_addrinfo *p = res; p != nullptr; p = p->ai_next) {
            switch (p->ai_family) {
               case AF_INET: {
                  QHostAddress addr;
                  addr.setAddress(ntohl(((sockaddr_in *) p->ai_addr)->sin_addr.s_addr));

                  if (! addresses.contains(addr)) {
                     addresses.append(addr);
                  }
               }

               break;

               case AF_INET6: {
                  QHostAddress addr;
                  addr.setAddress(((sockaddr_in6 *) p->ai_addr)->sin6_addr.s6_addr);

                  if (!addresses.contains(addr)) {
                     addresses.append(addr);
                  }
               }
               break;

               default:
                  results.setError(QHostInfo::UnknownError);
                  results.setErrorString(tr("Unknown address type"));
            }
         }

         results.setAddresses(addresses);
         local_freeaddrinfo(res);

      } else {
         translateWSAError(WSAGetLastError(), &results);

      }

   } else {
      // Fall back to gethostbyname, which only supports IPv4
      hostent *ent = gethostbyname(aceHostname.constData());

      if (ent) {
         char **p;
         QList<QHostAddress> addresses;

         switch (ent->h_addrtype) {
            case AF_INET:
               for (p = ent->h_addr_list; *p != nullptr; p++) {
                  long *ip4Addr = (long *) *p;
                  QHostAddress temp;
                  temp.setAddress(ntohl(*ip4Addr));
                  addresses << temp;
               }
               break;

            default:
               results.setError(QHostInfo::UnknownError);
               results.setErrorString(tr("Unknown address type"));
               break;
         }

         results.setAddresses(addresses);

      } else {
         translateWSAError(WSAGetLastError(), &results);
      }
   }

#if defined(CS_SHOW_DEBUG_NETWORK)
   if (results.error() != QHostInfo::NoError) {
      qDebug("QHostInfoAgent::run() Error (%s)", csPrintable(results.errorString()));

   } else {
      QString tmp;
      QList<QHostAddress> addresses = results.addresses();

      for (int i = 0; i < addresses.count(); ++i) {
         if (i != 0) { tmp += ", "; }
         tmp += addresses.at(i).toString();
      }

      qDebug("QHostInfoAgent::run() Found %lli entries for: %s", addresses.count(), csPrintable(tmp));
   }
#endif

   return results;
}
