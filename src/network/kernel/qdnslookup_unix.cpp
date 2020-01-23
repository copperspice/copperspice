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

#include <qdnslookup_p.h>

#include <qlibrary.h>
#include <qscopedpointer.h>
#include <qurl.h>
#include <qmutexpool_p.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/nameser_compat.h>
#include <resolv.h>

#if defined(__GNU_LIBRARY__) && !defined(__UCLIBC__)
#  include <gnu/lib-names.h>
#endif

QT_BEGIN_NAMESPACE

typedef int (*dn_expand_proto)(const unsigned char *, const unsigned char *, const unsigned char *, char *, int);
static dn_expand_proto local_dn_expand = 0;
typedef void (*res_nclose_proto)(res_state);
static res_nclose_proto local_res_nclose = 0;
typedef int (*res_ninit_proto)(res_state);
static res_ninit_proto local_res_ninit = 0;
typedef int (*res_nquery_proto)(res_state, const char *, int, int, unsigned char *, int);
static res_nquery_proto local_res_nquery = 0;

// Custom deleter to close resolver state.

struct QDnsLookupStateDeleter {
   static inline void cleanup(struct __res_state *pointer) {
      local_res_nclose(pointer);
   }
};

static void resolveLibraryInternal()
{
   QLibrary lib;

#ifdef LIBRESOLV_SO
   lib.setFileName(LIBRESOLV_SO);

   if (! lib.load())
#endif
   {
      lib.setFileName("resolv");
      if (!lib.load()) {
         return;
      }
   }

   local_dn_expand = dn_expand_proto(lib.resolve("__dn_expand"));
   if (!local_dn_expand) {
      local_dn_expand = dn_expand_proto(lib.resolve("dn_expand"));
   }

   local_res_nclose = res_nclose_proto(lib.resolve("__res_nclose"));
   if (!local_res_nclose) {
      local_res_nclose = res_nclose_proto(lib.resolve("res_9_nclose"));
   }
   if (!local_res_nclose) {
      local_res_nclose = res_nclose_proto(lib.resolve("res_nclose"));
   }

   local_res_ninit = res_ninit_proto(lib.resolve("__res_ninit"));
   if (!local_res_ninit) {
      local_res_ninit = res_ninit_proto(lib.resolve("res_9_ninit"));
   }
   if (!local_res_ninit) {
      local_res_ninit = res_ninit_proto(lib.resolve("res_ninit"));
   }

   local_res_nquery = res_nquery_proto(lib.resolve("__res_nquery"));
   if (!local_res_nquery) {
      local_res_nquery = res_nquery_proto(lib.resolve("res_9_nquery"));
   }
   if (!local_res_nquery) {
      local_res_nquery = res_nquery_proto(lib.resolve("res_nquery"));
   }
}

void QDnsLookupRunnable::query(const int requestType, const QByteArray &requestName,
                               const QHostAddress &nameserver, QDnsLookupReply *reply)
{
   // Load dn_expand, res_ninit and res_nquery on demand.
   static std::atomic<bool> triedResolve(false);

   if (! triedResolve.load()) {
      QMutexLocker locker(QMutexPool::globalInstanceGet(&local_res_ninit));
      if (!triedResolve.load()) {
         resolveLibraryInternal();
         triedResolve.store(true);
      }
   }

   // If dn_expand, res_ninit or res_nquery is missing, fail.
   if (!local_dn_expand || !local_res_nclose || !local_res_ninit || !local_res_nquery) {
      reply->error = QDnsLookup::ResolverError;
      reply->errorString = tr("Resolver functions not found");
      return;
   }

   // Initialize state.
   struct __res_state state;
   memset(&state, 0, sizeof(state));
   if (local_res_ninit(&state) < 0) {
      reply->error = QDnsLookup::ResolverError;
      reply->errorString = tr("Resolver initialization failed");
      return;
   }

   //Check if a nameserver was set. If so, use it
   if (!nameserver.isNull()) {
      if (nameserver.protocol() == QAbstractSocket::IPv4Protocol) {
         state.nsaddr_list[0].sin_addr.s_addr = htonl(nameserver.toIPv4Address());
         state.nscount = 1;
      } else if (nameserver.protocol() == QAbstractSocket::IPv6Protocol) {

#if defined(Q_OS_LINUX)
         struct sockaddr_in6 *ns;
         ns = state._u._ext.nsaddrs[0];
         // nsaddrs will be NULL if no nameserver is set in /etc/resolv.conf
         if (!ns) {
            // Memory allocated here will be free'd in res_close() as we
            // have done res_init() above.
            ns = (struct sockaddr_in6 *) calloc(1, sizeof(struct sockaddr_in6));
            Q_CHECK_PTR(ns);
            state._u._ext.nsaddrs[0] = ns;
         }

#ifndef __UCLIBC__
         // Set nsmap[] to indicate that nsaddrs[0] is an IPv6 address
         // See: https://sourceware.org/ml/libc-hacker/2002-05/msg00035.html
         state._u._ext.nsmap[0] = MAXNS + 1;
#endif
         state._u._ext.nscount6 = 1;
         ns->sin6_family = AF_INET6;
         ns->sin6_port = htons(53);

         Q_IPV6ADDR ipv6Address = nameserver.toIPv6Address();
         for (int i = 0; i < 16; i++) {
            ns->sin6_addr.s6_addr[i] = ipv6Address[i];
         }
#else
         qWarning("%s", QDnsLookupPrivate::msgNoIpV6NameServerAdresses);
         reply->error = QDnsLookup::ResolverError;
         reply->errorString = tr(QDnsLookupPrivate::msgNoIpV6NameServerAdresses);
         return;
#endif
      }
   }

#ifdef QDNSLOOKUP_DEBUG
   state.options |= RES_DEBUG;
#endif

   QScopedPointer<struct __res_state, QDnsLookupStateDeleter> state_ptr(&state);

   // Perform DNS query.
   unsigned char response[PACKETSZ];
   memset(response, 0, sizeof(response));
   const int responseLength = local_res_nquery(&state, requestName.constData(), C_IN, requestType, response, sizeof(response));

   // Check the response header.
   HEADER *header = (HEADER *)response;
   const int answerCount = ntohs(header->ancount);
   switch (header->rcode) {
      case NOERROR:
         break;
      case FORMERR:
         reply->error = QDnsLookup::InvalidRequestError;
         reply->errorString = tr("Server could not process query");
         return;
      case SERVFAIL:
         reply->error = QDnsLookup::ServerFailureError;
         reply->errorString = tr("Server failure");
         return;
      case NXDOMAIN:
         reply->error = QDnsLookup::NotFoundError;
         reply->errorString = tr("Non existent domain");
         return;
      case REFUSED:
         reply->error = QDnsLookup::ServerRefusedError;
         reply->errorString = tr("Server refused to answer");
         return;
      default:
         reply->error = QDnsLookup::InvalidReplyError;
         reply->errorString = tr("Invalid reply received");
         return;
   }

   // Check the reply is valid.
   if (responseLength < int(sizeof(HEADER))) {
      reply->error = QDnsLookup::InvalidReplyError;
      reply->errorString = tr("Invalid reply received");
      return;
   }

   // Skip the query host, type (2 bytes) and class (2 bytes).
   char host[PACKETSZ];
   char answer[PACKETSZ];
   unsigned char *p = response + sizeof(HEADER);
   int status = local_dn_expand(response, response + responseLength, p, host, sizeof(host));

   if (status < 0) {
      reply->error = QDnsLookup::InvalidReplyError;
      reply->errorString = tr("Could not expand domain name");
      return;
   }
   p += status + 4;

   // Extract results.
   int answerIndex = 0;

   while ((p < response + responseLength) && (answerIndex < answerCount)) {
      status = local_dn_expand(response, response + responseLength, p, host, sizeof(host));

      if (status < 0) {
         reply->error = QDnsLookup::InvalidReplyError;
         reply->errorString = tr("Could not expand domain name");
         return;
      }
      const QString name = QUrl::fromAce(QString::fromUtf8(host));

      p += status;
      const quint16 type = (p[0] << 8) | p[1];
      p += 2; // RR type
      p += 2; // RR class
      const quint32 ttl = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
      p += 4;
      const quint16 size = (p[0] << 8) | p[1];
      p += 2;

      if (type == QDnsLookup::A) {
         if (size != 4) {
            reply->error = QDnsLookup::InvalidReplyError;
            reply->errorString = tr("Invalid IPv4 address record");
            return;
         }
         const quint32 addr = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         QDnsHostAddressRecord record;
         record.d->name = name;
         record.d->timeToLive = ttl;
         record.d->value = QHostAddress(addr);
         reply->hostAddressRecords.append(record);
      } else if (type == QDnsLookup::AAAA) {
         if (size != 16) {
            reply->error = QDnsLookup::InvalidReplyError;
            reply->errorString = tr("Invalid IPv6 address record");
            return;
         }
         QDnsHostAddressRecord record;
         record.d->name = name;
         record.d->timeToLive = ttl;
         record.d->value = QHostAddress(p);
         reply->hostAddressRecords.append(record);
      } else if (type == QDnsLookup::CNAME) {
         status = local_dn_expand(response, response + responseLength, p, answer, sizeof(answer));
         if (status < 0) {
            reply->error = QDnsLookup::InvalidReplyError;
            reply->errorString = tr("Invalid canonical name record");
            return;
         }
         QDnsDomainNameRecord record;
         record.d->name = name;
         record.d->timeToLive = ttl;
         record.d->value = QUrl::fromAce(QString::fromUtf8(answer));
         reply->canonicalNameRecords.append(record);

      } else if (type == QDnsLookup::NS) {
         status = local_dn_expand(response, response + responseLength, p, answer, sizeof(answer));
         if (status < 0) {
            reply->error = QDnsLookup::InvalidReplyError;
            reply->errorString = tr("Invalid name server record");
            return;
         }
         QDnsDomainNameRecord record;
         record.d->name = name;
         record.d->timeToLive = ttl;
         record.d->value = QUrl::fromAce(QString::fromUtf8(answer));
         reply->nameServerRecords.append(record);

      } else if (type == QDnsLookup::PTR) {
         status = local_dn_expand(response, response + responseLength, p, answer, sizeof(answer));
         if (status < 0) {
            reply->error = QDnsLookup::InvalidReplyError;
            reply->errorString = tr("Invalid pointer record");
            return;
         }
         QDnsDomainNameRecord record;
         record.d->name = name;
         record.d->timeToLive = ttl;
         record.d->value = QUrl::fromAce(QString::fromUtf8(answer));
         reply->pointerRecords.append(record);

      } else if (type == QDnsLookup::MX) {
         const quint16 preference = (p[0] << 8) | p[1];
         status = local_dn_expand(response, response + responseLength, p + 2, answer, sizeof(answer));
         if (status < 0) {
            reply->error = QDnsLookup::InvalidReplyError;
            reply->errorString = tr("Invalid mail exchange record");
            return;
         }
         QDnsMailExchangeRecord record;
         record.d->exchange = QUrl::fromAce(QString::fromUtf8(answer));
         record.d->name = name;
         record.d->preference = preference;
         record.d->timeToLive = ttl;
         reply->mailExchangeRecords.append(record);

      } else if (type == QDnsLookup::SRV) {
         const quint16 priority = (p[0] << 8) | p[1];
         const quint16 weight = (p[2] << 8) | p[3];
         const quint16 port = (p[4] << 8) | p[5];
         status = local_dn_expand(response, response + responseLength, p + 6, answer, sizeof(answer));
         if (status < 0) {
            reply->error = QDnsLookup::InvalidReplyError;
            reply->errorString = tr("Invalid service record");
            return;
         }
         QDnsServiceRecord record;
         record.d->name = name;
         record.d->target = QUrl::fromAce(QString::fromUtf8(answer));
         record.d->port = port;
         record.d->priority = priority;
         record.d->timeToLive = ttl;
         record.d->weight = weight;
         reply->serviceRecords.append(record);
      } else if (type == QDnsLookup::TXT) {
         unsigned char *txt = p;
         QDnsTextRecord record;
         record.d->name = name;
         record.d->timeToLive = ttl;
         while (txt < p + size) {
            const unsigned char length = *txt;
            txt++;
            if (txt + length > p + size) {
               reply->error = QDnsLookup::InvalidReplyError;
               reply->errorString = tr("Invalid text record");
               return;
            }
            record.d->values << QByteArray((char *)txt, length);
            txt += length;
         }
         reply->textRecords.append(record);
      }
      p += size;
      answerIndex++;
   }
}
