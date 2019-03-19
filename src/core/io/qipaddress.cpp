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

/*****************************************************
** Copyright (C) 2012 Intel Corporation
*****************************************************/

#include <qipaddress_p.h>
#include <qlocale_tools_p.h>
#include <qtools_p.h>
#include <qvarlengtharray.h>

namespace QIPAddressUtils {

static QString number(quint8 val, int base = 10)
{
   QChar zero(0x30);
   return val ? qulltoa(val, base, zero) : zero;
}

typedef QVarLengthArray<char, 64> Buffer;

static const QString::const_iterator checkedToAscii(Buffer &buffer, const QString::const_iterator begin, const QString::const_iterator end)
{
   QString::const_iterator iter = begin;

   buffer.resize(end - begin + 1);
   char *dst = buffer.data();

   while (iter != end) {

      if (*iter >= 0x7f) {
         return iter;
      }

      *dst = iter->unicode() & 0xFF;

      ++dst;
      ++iter;
   }

   *dst = '\0';

   return end;
}

static bool parseIp4Internal(IPv4Address &address, Buffer::const_iterator iter, bool acceptLeadingZero);

bool parseIp4(IPv4Address &address, const QString::const_iterator begin, QString::const_iterator end)
{
   Q_ASSERT(begin != end);

   Buffer buffer;

   if (checkedToAscii(buffer, begin, end) != end) {
      return false;
   }

   Buffer::const_iterator iter = buffer.begin();

   return parseIp4Internal(address, iter, true);
}

static bool parseIp4Internal(IPv4Address &address, Buffer::const_iterator iter, bool acceptLeadingZero)
{
   address      = 0;
   int dotCount = 0;

   while (dotCount < 4) {
      if (! acceptLeadingZero && *iter == '0' && iter[1] != '.' && iter[1] != '\0') {
         return false;
      }

      Buffer::const_iterator end;

      bool ok;
      quint64 value64 = qstrtoull(iter, &end, 0, &ok);
      quint32 value32 = value64;

      if (! ok || iter == end || value64 != value32) {
         return false;
      }

      if (*end == '.' || dotCount == 3) {
         if (value32 & ~0xff) {
            return false;
         }
         address <<= 8;

      } else if (dotCount == 2) {
         if (value32 & ~0xffff) {
            return false;
         }
         address <<= 16;

      } else if (dotCount == 1) {
         if (value32 & ~0xffffff) {
            return false;
         }
         address <<= 24;
      }
      address |= value32;

      if (dotCount == 3 && *end != '\0') {
         return false;

      } else if (dotCount == 3 || *end == '\0') {
         return true;
      }

      if (*end != '.') {
         return false;
      }

      ++dotCount;
      iter = end + 1;
   }

   return false;
}

void toString(QString &appendTo, IPv4Address address)
{
   // reconstructing is easy
   // use the fast operator% that pre-calculates the size
   appendTo += number(address >> 24)
               + '.'
               + number(address >> 16)
               + '.'
               + number(address >> 8)
               + '.'
               + number(address);
}

const QString::const_iterator parseIp6(IPv6Address &address, const QString::const_iterator begin, QString::const_iterator end)
{
   Q_ASSERT(begin != end);

   Buffer buffer;
   QString::const_iterator iter = checkedToAscii(buffer, begin, end);

   if (iter != end) {
      return iter;
   }

   const char *ptr = buffer.data();

   // count the colons
   int colonCount = 0;
   int dotCount   = 0;

   while (*ptr) {
      if (*ptr == ':') {
         ++colonCount;
      }
      if (*ptr == '.') {
         ++dotCount;
      }
      ++ptr;
   }

   // IPv4-in-IPv6 addresses are stricter in what they accept
   if (dotCount != 0 && dotCount != 3) {
      return begin;
   }

   memset(address, 0, sizeof address);
   if (colonCount == 2 && end - begin == 2) {
      // "::"
      return begin;
   }

   // if there's a double colon ("::"), this is how many zeroes it means
   int zeroWordsToFill;
   ptr = buffer.data();

   // there are two cases where 8 colons are allowed: at the ends
   // so test that before the colon-count test
   if ((ptr[0] == ':' && ptr[1] == ':') || (ptr[end - begin - 2] == ':' && ptr[end - begin - 1] == ':')) {
      zeroWordsToFill = 9 - colonCount;

   } else if (colonCount < 2 || colonCount > 7) {
      return begin;

   } else {
      zeroWordsToFill = 8 - colonCount;
   }

   if (dotCount) {
      --zeroWordsToFill;
   }

   int pos = 0;
   while (pos < 15) {
      if (*ptr == ':') {
         // empty field, we hope it's "::"

         if (zeroWordsToFill < 1) {
            return begin + (ptr - buffer.data());
         }

         if (pos == 0 || pos == colonCount * 2) {
            if (ptr[0] == '\0' || ptr[1] != ':') {
               return begin + (ptr - buffer.data());
            }
            ++ptr;
         }

         pos += zeroWordsToFill * 2;
         zeroWordsToFill = 0;
         ++ptr;

         continue;
      }

      const char *endptr;
      bool ok;
      quint64 ll = qstrtoull(ptr, &endptr, 16, &ok);
      quint16 x = ll;

      if (!ok || ll != x) {
         return begin + (ptr - buffer.data());
      }

      if (*endptr == '.') {
         // this could be an IPv4 address
         // it's only valid in the last element

         if (pos != 12) {
            return begin + (ptr - buffer.data());
         }

         IPv4Address ip4;

         if (! parseIp4Internal(ip4, ptr, false)) {
            return begin + (ptr - buffer.data());
         }

         address[12] = ip4 >> 24;
         address[13] = ip4 >> 16;
         address[14] = ip4 >> 8;
         address[15] = ip4;

         return end;
      }

      address[pos++] = x >> 8;
      address[pos++] = x & 0xff;

      if (*endptr == '\0') {
         break;
      }

      if (*endptr != ':') {
         return begin + (endptr - buffer.data());
      }

      ptr = endptr + 1;
   }

   return end;
}

static inline QChar toHex(uchar c)
{
   return QtMiscUtils::toHexLower(c);
}

void toString(QString &appendTo, IPv6Address address)
{
   // the longest IPv6 address possible is:
   //   "1111:2222:3333:4444:5555:6666:255.255.255.255"
   // however, this function never generates that. The longest it does
   // generate without an IPv4 address is:
   //   "1111:2222:3333:4444:5555:6666:7777:8888"
   // and the longest with an IPv4 address is:
   //   "::ffff:255.255.255.255"

   static const int Ip6AddressMaxLen = sizeof "1111:2222:3333:4444:5555:6666:7777:8888";
   static const int Ip6WithIp4AddressMaxLen = sizeof "::ffff:255.255.255.255";

   // check for the special cases
   const quint64 zeroes[] = { 0, 0 };
   bool embeddedIp4 = false;

   // we consider embedded IPv4 for:
   //  ::ffff:x.x.x.x
   //  ::x.x.x.y  except if the x are 0 too
   if (memcmp(address, zeroes, 10) == 0) {
      if (address[10] == 0xff && address[11] == 0xff) {
         embeddedIp4 = true;

      } else if (address[10] == 0 && address[11] == 0) {
         if (address[12] != 0 || address[13] != 0 || address[14] != 0) {
            embeddedIp4 = true;
         } else if (address[15] == 0) {
            appendTo.append(QLatin1String("::"));
            return;
         }
      }
   }


   // for finding where to place the "::"
   int zeroRunLength = 0; // in octets
   int zeroRunOffset = 0; // in octets

   for (int i = 0; i < 16; i += 2) {
      if (address[i] == 0 && address[i + 1] == 0) {
         // found a zero, scan forward to see how many more there are
         int j;

         for (j = i; j < 16; j += 2) {
            if (address[j] != 0 || address[j + 1] != 0) {
               break;
            }
         }

         if (j - i > zeroRunLength) {
            zeroRunLength = j - i;
            zeroRunOffset = i;
            i = j;
         }
      }
   }

   const QChar colon = ushort(':');
   if (zeroRunLength < 4) {
      zeroRunOffset = -1;

   } else if (zeroRunOffset == 0) {
      appendTo.append(colon);
   }

   for (int i = 0; i < 16; i += 2) {
      if (i == zeroRunOffset) {
         appendTo.append(colon);
         i += zeroRunLength - 2;
         continue;
      }

      if (i == 12 && embeddedIp4) {
         IPv4Address ip4 = address[12] << 24 |
                           address[13] << 16 |
                           address[14] << 8 |
                           address[15];
         toString(appendTo, ip4);
         return;
      }

      if (address[i]) {
         if (address[i] >> 4) {
            appendTo.append(toHex(address[i] >> 4));
            appendTo.append(toHex(address[i] & 0xf));
            appendTo.append(toHex(address[i + 1] >> 4));
            appendTo.append(toHex(address[i + 1] & 0xf));
         } else if (address[i] & 0xf) {
            appendTo.append(toHex(address[i] & 0xf));
            appendTo.append(toHex(address[i + 1] >> 4));
            appendTo.append(toHex(address[i + 1] & 0xf));
         }
      } else if (address[i + 1] >> 4) {
         appendTo.append(toHex(address[i + 1] >> 4));
         appendTo.append(toHex(address[i + 1] & 0xf));
      } else {
         appendTo.append(toHex(address[i + 1] & 0xf));
      }

      if (i != 14) {
         appendTo.append(colon);
      }
   }
}

}

