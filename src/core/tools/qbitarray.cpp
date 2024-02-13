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

#include <qbitarray.h>

#include <qdatastream.h>
#include <qdebug.h>
#include <string.h>

QBitArray::QBitArray(int size, bool value)
{
   if (! size) {
      d.resize(0);
      return;
   }

   d.resize(1 + (size + 7) / 8);
   uchar *c = reinterpret_cast<uchar *>(d.data());
   memset(c, value ? 0xff : 0, d.size());

   *c = d.size() * 8 - size;

   if (value && size && size % 8) {
      *(c + 1 + size / 8) &= (1 << (size % 8)) - 1;
   }
}

int QBitArray::count(bool on) const
{
   int numBits = 0;
   int len = size();

   // See http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
   const quint8 *bits = reinterpret_cast<const quint8 *>(d.data()) + 1;

   while (len >= 32) {
      quint32 v = quint32(bits[0]) | (quint32(bits[1]) << 8) | (quint32(bits[2]) << 16) | (quint32(bits[3]) << 24);
      quint32 c = ((v & 0xfff) * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f;
      c += (((v & 0xfff000) >> 12) * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f;
      c += ((v >> 24) * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f;
      len -= 32;
      bits += 4;
      numBits += int(c);
   }

   while (len >= 24) {
      quint32 v = quint32(bits[0]) | (quint32(bits[1]) << 8) | (quint32(bits[2]) << 16);
      quint32 c =  ((v & 0xfff) * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f;
      c += (((v & 0xfff000) >> 12) * Q_UINT64_C(0x1001001001001) & Q_UINT64_C(0x84210842108421)) % 0x1f;
      len -= 24;
      bits += 3;
      numBits += int(c);
   }

   while (len >= 0) {
      if (bits[len / 8] & (1 << ((len - 1) & 7))) {
         ++numBits;
      }

      --len;
   }

   return on ? numBits : size() - numBits;
}

void QBitArray::resize(int size)
{
   if (!size) {
      d.resize(0);
   } else {
      int s = d.size();
      d.resize(1 + (size + 7) / 8);
      uchar *c = reinterpret_cast<uchar *>(d.data());

      if (size > (s << 3)) {
         memset(c + s, 0, d.size() - s);
      } else if ( size % 8) {
         *(c + 1 + size / 8) &= (1 << (size % 8)) - 1;
      }

      *c = d.size() * 8 - size;
   }
}

void QBitArray::fill(bool value, int begin, int end)
{
   while (begin < end && begin & 0x7) {
      setBit(begin++, value);
   }

   int len = end - begin;

   if (len <= 0) {
      return;
   }

   int s = len & ~0x7;
   uchar *c = reinterpret_cast<uchar *>(d.data());
   memset(c + (begin >> 3) + 1, value ? 0xff : 0, s >> 3);
   begin += s;

   while (begin < end) {
      setBit(begin++, value);
   }
}

QBitArray &QBitArray::operator&=(const QBitArray &other)
{
   resize(qMax(size(), other.size()));
   uchar *a1 = reinterpret_cast<uchar *>(d.data()) + 1;
   const uchar *a2 = reinterpret_cast<const uchar *>(other.d.constData()) + 1;
   int n = other.d.size() - 1 ;
   int p = d.size() - 1 - n;

   while (n-- > 0) {
      *a1++ &= *a2++;
   }

   while (p-- > 0) {
      *a1++ = 0;
   }

   return *this;
}

QBitArray &QBitArray::operator|=(const QBitArray &other)
{
   resize(qMax(size(), other.size()));
   uchar *a1 = reinterpret_cast<uchar *>(d.data()) + 1;
   const uchar *a2 = reinterpret_cast<const uchar *>(other.d.constData()) + 1;
   int n = other.d.size() - 1;

   while (n-- > 0) {
      *a1++ |= *a2++;
   }

   return *this;
}

QBitArray &QBitArray::operator^=(const QBitArray &other)
{
   resize(qMax(size(), other.size()));
   uchar *a1 = reinterpret_cast<uchar *>(d.data()) + 1;
   const uchar *a2 = reinterpret_cast<const uchar *>(other.d.constData()) + 1;
   int n = other.d.size() - 1;

   while (n-- > 0) {
      *a1++ ^= *a2++;
   }

   return *this;
}

QBitArray QBitArray::operator~() const
{
   int sz = size();
   QBitArray a(sz);
   const uchar *a1 = reinterpret_cast<const uchar *>(d.constData()) + 1;
   uchar *a2 = reinterpret_cast<uchar *>(a.d.data()) + 1;
   int n = d.size() - 1;

   while (n-- > 0) {
      *a2++ = ~*a1++;
   }

   if (sz && sz % 8) {
      *(a2 - 1) &= (1 << (sz % 8)) - 1;
   }

   return a;
}

QBitArray operator&(const QBitArray &a1, const QBitArray &a2)
{
   QBitArray tmp = a1;
   tmp &= a2;
   return tmp;
}

QBitArray operator|(const QBitArray &bitArray_1, const QBitArray &bitArray_2)
{
   QBitArray tmp = bitArray_1;
   tmp |= bitArray_2;

   return tmp;
}

QBitArray operator^(const QBitArray &bitArray_1, const QBitArray &bitArray_2)
{
   QBitArray tmp = bitArray_1;
   tmp ^= bitArray_2;

   return tmp;
}

QDataStream &operator<<(QDataStream &stream, const QBitArray &bitArray)
{
   quint32 len = bitArray.size();
   stream << len;

   if (len > 0) {
      stream.writeRawData(bitArray.d.constData() + 1, bitArray.d.size() - 1);
   }

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QBitArray &bitArray)
{
   bitArray.clear();
   quint32 len;

   stream >> len;

   if (len == 0) {
      bitArray.clear();
      return stream;
   }

   const quint32 Step = 8 * 1024 * 1024;
   quint32 totalBytes = (len + 7) / 8;
   quint32 allocated  = 0;

   while (allocated < totalBytes) {
      int blockSize = qMin(Step, totalBytes - allocated);
      bitArray.d.resize(allocated + blockSize + 1);

      if (stream.readRawData(bitArray.d.data() + 1 + allocated, blockSize) != blockSize) {
         bitArray.clear();
         stream.setStatus(QDataStream::ReadPastEnd);
         return stream;
      }

      allocated += blockSize;
   }

   int paddingMask = ~((0x1 << (len & 0x7)) - 1);

   if (paddingMask != ~0x0 && (bitArray.d.constData()[bitArray.d.size() - 1] & paddingMask)) {
      bitArray.clear();
      stream.setStatus(QDataStream::ReadCorruptData);
      return stream;
   }

   *bitArray.d.data() = bitArray.d.size() * 8 - len;

   return stream;
}
