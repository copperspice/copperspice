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

#ifndef Q_HASHFUNC_H
#define Q_HASHFUNC_H

#include <qchar.h>
#include <qhashfwd.h>
#include <qpair.h>
#include <qstring.h>
#include <qstring16.h>

class QBitArray;
class QByteArray;

Q_CORE_EXPORT uint cs_getHashSeed();
Q_CORE_EXPORT uint cs_stable_hash(const QString &key);

Q_CORE_EXPORT uint qHashBits(const void *p, size_t len, uint seed);

Q_CORE_EXPORT uint qHash(const QBitArray  &key, uint seed = 0);
Q_CORE_EXPORT uint qHash(const QByteArray &key, uint seed = 0);

inline uint qHash(char key, uint seed = 0)
{
   return uint(key) ^ seed;
}

inline uint qHash(uchar key, uint seed = 0)
{
   return uint(key) ^ seed;
}

inline uint qHash(signed char key, uint seed = 0)
{
   return uint(key) ^ seed;
}

inline uint qHash(ushort key, uint seed = 0)
{
   return uint(key) ^ seed;
}

inline uint qHash(short key, uint seed = 0)
{
   return uint(key) ^ seed;
}

inline uint qHash(uint key, uint seed = 0)
{
   return key ^ seed;
}

inline uint qHash(int key, uint seed = 0)
{
   return uint(key) ^ seed;
}

inline uint qHash(double key, uint seed = 0)
{
   return uint(key) ^ seed;
}

inline uint qHash(ulong key, uint seed = 0)
{
   if (sizeof(ulong) > sizeof(uint)) {
      return uint(((key >> (8 * sizeof(uint) - 1)) ^ key) & (~0U)) ^ seed;
   } else {
      return uint(key & (~0U)) ^ seed;
   }
}

inline uint qHash(long key, uint seed = 0)
{
   return qHash(ulong(key), seed);
}

inline uint qHash(quint64 key, uint seed = 0)
{
   if (sizeof(quint64) > sizeof(uint)) {
      return uint(((key >> (8 * sizeof(uint) - 1)) ^ key) & (~0U)) ^ seed;
   } else {
      return uint(key & (~0U)) ^ seed;
   }
}

inline uint qHash(qint64 key, uint seed = 0)
{
   return qHash(quint64(key), seed);
}

inline uint qHash(const QString &key, uint seed = 0)
{
   uint h = seed;

   for (QChar c : key) {
      h = 31 * h + c.unicode();
   }

   return h;
}

inline uint qHash(const QString16 &key, uint seed = 0)
{
   uint h = seed;

   for (QChar c : key) {
      h = 31 * h + c.unicode();
   }

   return h;
}

inline uint qHash(QStringView key, uint seed = 0)
{
   uint h = seed;

   for (QChar c : key) {
      h = 31 * h + c.unicode();
   }

   return h;
}

inline uint qHash(const QStringView16 &key, uint seed = 0)
{
   uint h = seed;

   for (QChar c : key) {
      h = 31 * h + c.unicode();
   }

   return h;
}

template <typename Key>
uint qHash(const Key *key, uint seed)
{
   return qHash(reinterpret_cast<quintptr>(key), seed);
}

template <typename T1, typename T2>
uint qHash(const QPair<T1, T2> &key)
{
   uint h1 = qHash(key.first);
   uint h2 = qHash(key.second);

   return ((h1 << 16) | (h1 >> 16)) ^ h2;
}

template <typename T>
uint qHash(const T &t, uint seed)
{
   return qHash(t) ^ seed;
}

// **
template <typename Key>
class qHashFunc
{
 public:
   uint operator()(const Key &key) const {
      return qHash(key, cs_getHashSeed());
   }
};

template <typename Key>
class qHashEqual
{
 public:
   using result_type = bool;

   bool operator()(const Key &a, const Key &b) const {
      return a == b;
   }
};

#endif