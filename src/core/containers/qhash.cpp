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

// for rand_s, _CRT_RAND_S must be #defined before #including stdlib.h.
// put it at the beginning so some indirect inclusion does not break it
#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif

#ifdef truncate
#undef truncate
#endif

#include <qhash.h>

#include <qbitarray.h>
#include <qbytearray.h>
#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qglobal.h>
#include <qstring.h>

#ifdef Q_OS_UNIX
#include <stdio.h>
#include <qcore_unix_p.h>
#endif

#include <limits.h>
#include <stdlib.h>

static std::atomic<uint> cs_seed_value{0};

static inline uint hash(const uchar *p, int len, uint seed)
{
   uint h = seed;

   for (int i = 0; i < len; ++i) {
      h = 31 * h + p[i];
   }

   return h;
}

uint qHashBits(const void *p, size_t len, uint seed)
{
   return hash(static_cast<const uchar *>(p), int(len), seed);
}

uint qHash(const QByteArray &key, uint seed)
{
   return hash(reinterpret_cast<const uchar *>(key.constData()), key.size(), seed);
}

uint qHash(const QBitArray &bitArray, uint seed)
{
   int m = bitArray.d.size() - 1;
   uint result = hash(reinterpret_cast<const uchar *>(bitArray.d.constData()), qMax(0, m), seed);

   // deal with the last 0 to 7 bits manually, because we can't trust that
   // the padding is initialized to 0 in bitArray.d

   int n = bitArray.size();

   if (n & 0x7) {
      result = ((result << 4) + bitArray.d.at(m)) & ((1 << n) - 1);
   }

   return result;
}

uint cs_stable_hash(const QString &key)
{
   uint h = 0;

   for (auto item : key) {
      h  = (h << 4) + item.unicode();
      h ^= (h & 0xf0000000) >> 23;
      h &= 0x0fffffff;
   }

   return h;
}

static uint cs_create_seed()
{
   uint seed = 0;

#if defined(Q_OS_UNIX)
   int randomfd = qt_safe_open("/dev/urandom", O_RDONLY);

   if (randomfd == -1) {
      randomfd = qt_safe_open("/dev/random", O_RDONLY | O_NONBLOCK);
   }

   if (randomfd != -1) {
      if (qt_safe_read(randomfd, reinterpret_cast<char *>(&seed), sizeof(seed)) == sizeof(seed)) {
         qt_safe_close(randomfd);
         return seed;
      }

      qt_safe_close(randomfd);
   }

#endif

   quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
   seed ^= timestamp;
   seed ^= (timestamp >> 32);

   quint64 pid = QCoreApplication::applicationPid();
   seed ^= pid;
   seed ^= (pid >> 32);

   seed ^= std::hash<uint *> {}(&seed);

   return seed;
}

uint cs_getHashSeed()
{
   uint value = cs_seed_value.load(std::memory_order_relaxed);

   if (value != 0) {
      // already initialized
      return value;
   }

   value = cs_create_seed();

   if (value == 0) {
      // prevent 0 from ever being a valid seed value
      value = 1;
   }

   uint expectedValue = 0;

   if (cs_seed_value.compare_exchange_strong(expectedValue, value, std::memory_order_relaxed)) {
      // succeeded, value is correct
      return value;

   } else {
      // failed, someone else has filled in the value
      return expectedValue;
   }
}

