/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qnetworkinterface.h>
#include <qnetworkinterface_p.h>

#ifndef QT_NO_NETWORKINTERFACE

#include <qhostinfo.h>
#include <qhash.h>
#include <qurl.h>

// order dependant include files
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <qt_windows.h>

#ifndef NETIO_STATUS
#define NETIO_STATUS DWORD
#endif

using PtrConvertInterfaceLuidToName = NETIO_STATUS (WINAPI *)(const NET_LUID *, PWSTR, SIZE_T);
static PtrConvertInterfaceLuidToName ptrConvertInterfaceLuidToName = 0;

static void resolveLibs()
{
   // try to find the functions we need from Iphlpapi.dll
   static bool done = false;
   if (! done) {
      HINSTANCE iphlpapiHnd = GetModuleHandle(L"iphlpapi");
      Q_ASSERT(iphlpapiHnd);

      // since Windows Vista
      ptrConvertInterfaceLuidToName = (PtrConvertInterfaceLuidToName)GetProcAddress(iphlpapiHnd, "ConvertInterfaceLuidToNameW");

      done = true;
   }
}

static QHostAddress addressFromSockaddr(sockaddr *sa)
{
   QHostAddress address;
   if (! sa) {
      return address;
   }

   if (sa->sa_family == AF_INET) {
      address.setAddress(htonl(((sockaddr_in *)sa)->sin_addr.s_addr));

   } else if (sa->sa_family == AF_INET6) {
      address.setAddress(((sockaddr_in6 *)sa)->sin6_addr.s6_addr);
      int scope = ((sockaddr_in6 *)sa)->sin6_scope_id;

      if (scope) {
         address.setScopeId(QString::number(scope));
      }

   } else {
      qWarning("Got unknown socket family %d", sa->sa_family);
   }

   return address;
}

static QHash<QHostAddress, QHostAddress> ipv4Netmasks()
{
   //Retrieve all the IPV4 addresses & netmasks
   IP_ADAPTER_INFO staticBuf[2]; // 2 is arbitrary
   PIP_ADAPTER_INFO pAdapter = staticBuf;
   ULONG bufSize = sizeof staticBuf;
   QHash<QHostAddress, QHostAddress> ipv4netmasks;

   DWORD retval = GetAdaptersInfo(pAdapter, &bufSize);

   if (retval == ERROR_BUFFER_OVERFLOW) {
      // need more memory
      pAdapter = (IP_ADAPTER_INFO *)malloc(bufSize);

      if (! pAdapter) {
         return ipv4netmasks;
      }

      // try again
      if (GetAdaptersInfo(pAdapter, &bufSize) != ERROR_SUCCESS) {
         free(pAdapter);
         return ipv4netmasks;
      }

   } else if (retval != ERROR_SUCCESS) {
      // error
      return ipv4netmasks;
   }

   // iterate over the list and add the entries to our listing
   for (PIP_ADAPTER_INFO ptr = pAdapter; ptr; ptr = ptr->Next) {
      for (PIP_ADDR_STRING addr = &ptr->IpAddressList; addr; addr = addr->Next) {
         QHostAddress address(QLatin1String(addr->IpAddress.String));
         QHostAddress mask(QLatin1String(addr->IpMask.String));
         ipv4netmasks[address] = mask;
      }
   }
   if (pAdapter != staticBuf) {
      free(pAdapter);
   }

   return ipv4netmasks;
}

static QList<QNetworkInterfacePrivate *> interfaceListingWinXP()
{
   QList<QNetworkInterfacePrivate *> interfaces;

   IP_ADAPTER_ADDRESSES staticBuf[2];             // 2 is arbitrary
   PIP_ADAPTER_ADDRESSES pAdapter = staticBuf;

   ULONG bufSize = sizeof staticBuf;

   const QHash<QHostAddress, QHostAddress> &ipv4netmasks = ipv4Netmasks();
   ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_MULTICAST;

   ULONG retval = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize);

   if (retval == ERROR_BUFFER_OVERFLOW) {
      // need more memory
      pAdapter = (IP_ADAPTER_ADDRESSES *)malloc(bufSize);

      if (! pAdapter) {
         return interfaces;
      }

      // try again
      if (GetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAdapter, &bufSize) != ERROR_SUCCESS) {
         free(pAdapter);
         return interfaces;
      }

   } else if (retval != ERROR_SUCCESS) {
      // error
      return interfaces;
   }

   // iterate over the list and add the entries to our listing
   for (PIP_ADAPTER_ADDRESSES ptr = pAdapter; ptr; ptr = ptr->Next) {
      QNetworkInterfacePrivate *iface = new QNetworkInterfacePrivate;
      interfaces << iface;

      iface->index = 0;

      if (ptr->Length >= offsetof(IP_ADAPTER_ADDRESSES, Ipv6IfIndex) && ptr->Ipv6IfIndex != 0) {
         iface->index = ptr->Ipv6IfIndex;
      } else if (ptr->IfIndex != 0) {
         iface->index = ptr->IfIndex;
      }

      iface->flags = QNetworkInterface::CanBroadcast;
      if (ptr->OperStatus == IfOperStatusUp) {
         iface->flags |= QNetworkInterface::IsUp | QNetworkInterface::IsRunning;
      }

      if ((ptr->Flags & IP_ADAPTER_NO_MULTICAST) == 0) {
         iface->flags |= QNetworkInterface::CanMulticast;
      }

      if (ptr->IfType == IF_TYPE_PPP) {
         iface->flags |= QNetworkInterface::IsPointToPoint;
      }

      if (ptrConvertInterfaceLuidToName && ptr->Length >= offsetof(IP_ADAPTER_ADDRESSES_LH, Luid)) {
         IP_ADAPTER_ADDRESSES_LH tmp;
         memcpy(&tmp, ptr, sizeof(IP_ADAPTER_ADDRESSES_LH));

         std::wstring buffer(IF_MAX_STRING_SIZE + 1, L'\0');

         if (ptrConvertInterfaceLuidToName(&tmp.Luid, &buffer[0], buffer.size()) == NO_ERROR) {
            iface->name = QString::fromStdWString(buffer);
         }
      }

      if (iface->name.isEmpty()) {
         iface->name = QString::fromUtf8(ptr->AdapterName);
      }

      iface->friendlyName = QString::fromStdWString(std::wstring(ptr->FriendlyName));

      if (ptr->PhysicalAddressLength)
         iface->hardwareAddress = iface->makeHwAddress(ptr->PhysicalAddressLength, ptr->PhysicalAddress);

      else {
         // loopback if it has no address
         iface->flags |= QNetworkInterface::IsLoopBack;
      }

      // The GetAdaptersAddresses call has an interesting semantic:
      // It can return a number N of addresses and a number M of prefixes.
      // But if you have IPv6 addresses, generally N > M.
      // I cannot find a way to relate the Address to the Prefix, aside from stopping
      // the iteration at the last Prefix entry and assume that it applies to all addresses from that point on.
      PIP_ADAPTER_PREFIX pprefix = 0;

      if (ptr->Length >= offsetof(IP_ADAPTER_ADDRESSES_LH, FirstPrefix)) {
         pprefix = ptr->FirstPrefix;
      }

      for (PIP_ADAPTER_UNICAST_ADDRESS addr = ptr->FirstUnicastAddress; addr; addr = addr->Next) {
         QNetworkAddressEntry entry;
         entry.setIp(addressFromSockaddr(addr->Address.lpSockaddr));

         if (pprefix) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
               entry.setNetmask(ipv4netmasks[entry.ip()]);

               // broadcast address is set on postProcess()

            } else {
               //IPV6
               entry.setPrefixLength(pprefix->PrefixLength);
            }

            pprefix = pprefix->Next ? pprefix->Next : pprefix;
         }
         iface->addressEntries << entry;
      }
   }

   if (pAdapter != staticBuf) {
      free(pAdapter);
   }

   return interfaces;
}

QList<QNetworkInterfacePrivate *> QNetworkInterfaceManager::scan()
{
   resolveLibs();
   return interfaceListingWinXP();
}

QString QHostInfo::localDomainName()
{
   resolveLibs();

   FIXED_INFO info, *pinfo;
   ULONG bufSize = sizeof info;
   pinfo = &info;

   if (GetNetworkParams(pinfo, &bufSize) == ERROR_BUFFER_OVERFLOW) {
      pinfo = (FIXED_INFO *)malloc(bufSize);

      if (! pinfo) {
         return QString();
      }

      // try again
      if (GetNetworkParams(pinfo, &bufSize) != ERROR_SUCCESS) {
         free(pinfo);
         return QString();   // error
      }
   }

   QString domainName = QUrl::fromAce(QByteArray(pinfo->DomainName));

   if (pinfo != &info) {
      free(pinfo);
   }

   return domainName;
}

#endif // QT_NO_NETWORKINTERFACE
