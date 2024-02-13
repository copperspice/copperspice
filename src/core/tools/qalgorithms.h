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

#ifndef QALGORITHMS_H
#define QALGORITHMS_H

#include <qglobal.h>

template <typename ForwardIterator>
void qDeleteAll(ForwardIterator begin, ForwardIterator end)
{
   while (begin != end) {
      delete *begin;
      ++begin;
   }
}

template <typename Container>
inline void qDeleteAll(const Container &c)
{
   qDeleteAll(c.begin(), c.end());
}

// Use __builtin_popcount on gcc.
// clang claims to be gcc but has a bug where __builtin_popcount is not marked as constexpr.

#if defined(Q_CC_GNU) && ! defined(Q_CC_CLANG)
#define QALGORITHMS_USE_BUILTIN_POPCOUNT
#endif

constexpr inline uint qPopulationCount(quint32 v)
{
#ifdef QALGORITHMS_USE_BUILTIN_POPCOUNT
   return __builtin_popcount(v);

#else
   // refer to http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
   return
         (((v      ) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f +
         (((v >> 12) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f +
         (((v >> 24) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f;
#endif
}

constexpr inline uint qPopulationCount(quint8 v)
{
#ifdef QALGORITHMS_USE_BUILTIN_POPCOUNT
   return __builtin_popcount(v);

#else
   return
         (((v      ) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f;
#endif
}

constexpr inline uint qPopulationCount(quint16 v)
{
#ifdef QALGORITHMS_USE_BUILTIN_POPCOUNT
   return __builtin_popcount(v);
#else
   return
         (((v      ) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f +
         (((v >> 12) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f;
#endif
}

constexpr inline uint qPopulationCount(quint64 v)
{
#ifdef QALGORITHMS_USE_BUILTIN_POPCOUNT
   return __builtin_popcountll(v);
#else
   return
         (((v      ) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f +
         (((v >> 12) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f +
         (((v >> 24) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f +
         (((v >> 36) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f +
         (((v >> 48) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f +
         (((v >> 60) & 0xfff)    * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f;
#endif
}

constexpr inline uint qPopulationCount(long unsigned int v)
{
   return qPopulationCount(static_cast<quint64>(v));
}

#if defined(Q_CC_GNU) && ! defined(Q_CC_CLANG)
#undef QALGORITHMS_USE_BUILTIN_POPCOUNT
#endif

#endif // QALGORITHMS_H
