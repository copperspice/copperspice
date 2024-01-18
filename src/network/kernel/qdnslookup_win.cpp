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

#include <winsock2.h>
#include <qdnslookup_p.h>

#include <qurl.h>
#include <qmutexpool_p.h>
#include <qsystemerror_p.h>

#include <qt_windows.h>
#include <windns.h>
#include <memory.h>

void QDnsLookupRunnable::query(const int requestType, const QByteArray &name,
                               const QHostAddress &nameserver, QDnsLookupReply *reply)
{
   // Perform DNS query
   const QString requestName = QString::fromUtf8(name.data(), name.size());

   IP4_ARRAY srvList;

   memset(&srvList, 0, sizeof(IP4_ARRAY));

   if (!nameserver.isNull()) {
      if (nameserver.protocol() == QAbstractSocket::IPv4Protocol) {
         // code below is referenced from: http://support.microsoft.com/kb/831226
         srvList.AddrCount = 1;
         srvList.AddrArray[0] = htonl(nameserver.toIPv4Address());

      } else if (nameserver.protocol() == QAbstractSocket::IPv6Protocol) {
         // For supoprting IPv6 nameserver addresses, we will need to switch from DnsQuey() to DnsQueryEx()
         // as it supports passing an IPv6 address in the nameserver list

         qWarning("QDnsLookupRunnable::query() %s", QDnsLookupPrivate::msgNoIpV6NameServerAdresses);
         reply->error = QDnsLookup::ResolverError;
         reply->errorString = tr(QDnsLookupPrivate::msgNoIpV6NameServerAdresses);

         return;
      }
   }

#ifdef Q_CC_MSVC
   using DNS_RECORD_PTR = PDNS_RECORD;

#else
   using DNS_RECORD_PTR = _DnsRecordA *;

#endif

   DNS_RECORD_PTR dns_records = nullptr;

   const DNS_STATUS status = DnsQuery_UTF8(requestName.constData(), requestType, DNS_QUERY_STANDARD, &srvList, &dns_records, nullptr);

   switch (status) {
      case ERROR_SUCCESS:
         break;

      case DNS_ERROR_RCODE_FORMAT_ERROR:
         reply->error = QDnsLookup::InvalidRequestError;
         reply->errorString = tr("Server was unable to process query");
         return;

      case DNS_ERROR_RCODE_SERVER_FAILURE:
         reply->error = QDnsLookup::ServerFailureError;
         reply->errorString = tr("Server failure");
         return;

      case DNS_ERROR_RCODE_NAME_ERROR:
         reply->error = QDnsLookup::NotFoundError;
         reply->errorString = tr("Non existent domain");
         return;

      case DNS_ERROR_RCODE_REFUSED:
         reply->error = QDnsLookup::ServerRefusedError;
         reply->errorString = tr("Server refused to answer");
         return;

      default:
         reply->error = QDnsLookup::InvalidReplyError;
         reply->errorString = QSystemError(status, QSystemError::NativeError).toString();
         return;
   }

   // Extract results
   for (DNS_RECORD_PTR ptr = dns_records; ptr != nullptr; ptr = ptr->pNext) {

#ifdef Q_CC_MSVC
      const QString name = QUrl::fromAce(QString::fromUtf8(reinterpret_cast<const char *>(ptr->pName)));
#else
      const QString name = QUrl::fromAce(QString::fromUtf8(ptr->pName));
#endif

      if (ptr->wType == QDnsLookup::A) {
         QDnsHostAddressRecord record;
         record.d->name       = name;
         record.d->timeToLive = ptr->dwTtl;
         record.d->value      = QHostAddress(ntohl(ptr->Data.A.IpAddress));
         reply->hostAddressRecords.append(record);

      } else if (ptr->wType == QDnsLookup::AAAA) {
         Q_IPV6ADDR addr;
         memcpy(&addr, &ptr->Data.AAAA.Ip6Address, sizeof(Q_IPV6ADDR));

         QDnsHostAddressRecord record;
         record.d->name       = name;
         record.d->timeToLive = ptr->dwTtl;
         record.d->value      = QHostAddress(addr);
         reply->hostAddressRecords.append(record);

      } else if (ptr->wType == QDnsLookup::CNAME) {
         QDnsDomainNameRecord record;
         record.d->name       = name;
         record.d->timeToLive = ptr->dwTtl;

#ifdef Q_CC_MSVC
         record.d->value = QUrl::fromAce(QString::fromUtf8(reinterpret_cast<const char *>(ptr->Data.Cname.pNameHost)));
#else
         record.d->value = QUrl::fromAce(QString::fromUtf8(ptr->Data.Cname.pNameHost));
#endif

         reply->canonicalNameRecords.append(record);

      } else if (ptr->wType == QDnsLookup::MX) {
         QDnsMailExchangeRecord record;
         record.d->name       = name;

#ifdef Q_CC_MSVC
         record.d->exchange   = QUrl::fromAce(QString::fromUtf8(reinterpret_cast<const char *>(ptr->Data.Mx.pNameExchange)));
#else
         record.d->exchange   = QUrl::fromAce(QString::fromUtf8(ptr->Data.Mx.pNameExchange));
#endif

         record.d->preference = ptr->Data.Mx.wPreference;
         record.d->timeToLive = ptr->dwTtl;
         reply->mailExchangeRecords.append(record);

      } else if (ptr->wType == QDnsLookup::NS) {
         QDnsDomainNameRecord record;
         record.d->name       = name;
         record.d->timeToLive = ptr->dwTtl;

#ifdef Q_CC_MSVC
         record.d->value      = QUrl::fromAce(QString::fromUtf8(reinterpret_cast<const char *>(ptr->Data.Ns.pNameHost)));
#else
         record.d->value      = QUrl::fromAce(QString::fromUtf8(ptr->Data.Ns.pNameHost));
#endif

         reply->nameServerRecords.append(record);

      } else if (ptr->wType == QDnsLookup::PTR) {
         QDnsDomainNameRecord record;
         record.d->name       = name;
         record.d->timeToLive = ptr->dwTtl;


#ifdef Q_CC_MSVC
         record.d->value      = QUrl::fromAce(QString::fromUtf8(reinterpret_cast<const char *>(ptr->Data.Ptr.pNameHost)));
#else
         record.d->value      = QUrl::fromAce(QString::fromUtf8(ptr->Data.Ptr.pNameHost));
#endif

         reply->pointerRecords.append(record);

      } else if (ptr->wType == QDnsLookup::SRV) {
         QDnsServiceRecord record;
         record.d->name       = name;

#ifdef Q_CC_MSVC
         record.d->target     = QUrl::fromAce(QString::fromUtf8(reinterpret_cast<const char *>(ptr->Data.Srv.pNameTarget)));
#else
         record.d->target     = QUrl::fromAce(QString::fromUtf8(ptr->Data.Srv.pNameTarget));
#endif

         record.d->port       = ptr->Data.Srv.wPort;
         record.d->priority   = ptr->Data.Srv.wPriority;
         record.d->timeToLive = ptr->dwTtl;
         record.d->weight     = ptr->Data.Srv.wWeight;
         reply->serviceRecords.append(record);

      } else if (ptr->wType == QDnsLookup::TXT) {
         QDnsTextRecord record;
         record.d->name = name;
         record.d->timeToLive = ptr->dwTtl;

         for (unsigned int i = 0; i < ptr->Data.Txt.dwStringCount; ++i) {

#ifdef Q_CC_MSVC
            record.d->values << QString::fromUtf8(reinterpret_cast<const char *>(ptr->Data.Txt.pStringArray[i])).toLatin1();
#else
            record.d->values << QString::fromUtf8(ptr->Data.Txt.pStringArray[i]).toLatin1();
#endif

         }
         reply->textRecords.append(record);
      }
   }

   DnsRecordListFree(dns_records, DnsFreeRecordList);
}

