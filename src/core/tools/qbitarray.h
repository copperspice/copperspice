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

#ifndef QBITARRAY_H
#define QBITARRAY_H

#include <qbytearray.h>

class QBitRef;

class Q_CORE_EXPORT QBitArray
{
 public:
   QBitArray()
   { }

   explicit QBitArray(int size, bool value = false);

   QBitArray(const QBitArray &other)
      : d(other.d)
   {
   }

   QBitArray &operator=(const QBitArray &other) {
      d = other.d;
      return *this;
   }

   QBitArray &operator=(QBitArray && other) {
      qSwap(d, other.d);
      return *this;
   }

   void swap(QBitArray &other) {
      qSwap(d, other.d);
   }

   int size() const {
      return (d.size() << 3) - *d.constData();
   }

   int count() const {
      return (d.size() << 3) - *d.constData();
   }

   int count(bool on) const;
   // TODO: Store the number of set bits separately

   bool isEmpty() const {
      return d.isEmpty();
   }

   bool isNull() const {
      return d.isNull();
   }

   void resize(int size);

   void detach() {
      d.detach();
   }

   bool isDetached() const {
      return d.isDetached();
   }

   void clear() {
      d.clear();
   }

   bool testBit(int i) const;
   void setBit(int i);
   void setBit(int i, bool value);
   void clearBit(int i);
   bool toggleBit(int i);

   bool at(int i) const;
   QBitRef operator[](int i);
   bool operator[](int i) const;
   QBitRef operator[](uint i);
   bool operator[](uint i) const;

   QBitArray &operator&=(const QBitArray &other);
   QBitArray &operator|=(const QBitArray &other);
   QBitArray &operator^=(const QBitArray &other);
   QBitArray  operator~() const;

   bool operator==(const QBitArray &other) const {
      return d == other.d;
   }

   bool operator!=(const QBitArray &other) const {
      return d != other.d;
   }

   inline bool fill(bool value, int size = -1);
   void fill(bool value, int begin, int end);

   void truncate(int pos) {
      if (pos < size()) {
         resize(pos);
      }
   }

   QByteArray::DataPtr &data_ptr() {
      return d.data_ptr();
   }

 private:
   QByteArray d;

   friend Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QBitArray &bitArray);
   friend Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QBitArray &bitArray);
   friend Q_CORE_EXPORT uint qHash(const QBitArray &key, uint seed);
};

inline bool QBitArray::fill(bool value, int size)
{
   *this = QBitArray((size < 0 ? this->size() : size), value);

   return true;
}

Q_CORE_EXPORT QBitArray operator&(const QBitArray &, const QBitArray &);
Q_CORE_EXPORT QBitArray operator|(const QBitArray &, const QBitArray &);
Q_CORE_EXPORT QBitArray operator^(const QBitArray &, const QBitArray &);

inline bool QBitArray::testBit(int i) const
{
   Q_ASSERT(uint(i) < uint(size()));
   return (*(reinterpret_cast<const uchar *>(d.constData()) + 1 + (i >> 3)) & (1 << (i & 7))) != 0;
}

inline void QBitArray::setBit(int i)
{
   Q_ASSERT(uint(i) < uint(size()));
   *(reinterpret_cast<uchar *>(d.data()) + 1 + (i >> 3)) |= uchar(1 << (i & 7));
}

inline void QBitArray::clearBit(int i)
{
   Q_ASSERT(uint(i) < uint(size()));
   *(reinterpret_cast<uchar *>(d.data()) + 1 + (i >> 3)) &= ~uchar(1 << (i & 7));
}

inline void QBitArray::setBit(int i, bool value)
{
   if (value) {
      setBit(i);
   } else {
      clearBit(i);
   }
}

inline bool QBitArray::toggleBit(int i)
{
   Q_ASSERT(uint(i) < uint(size()));
   uchar b = uchar(1 << (i & 7));
   uchar *p = reinterpret_cast<uchar *>(d.data()) + 1 + (i >> 3);
   uchar c = uchar(*p & b);
   *p ^= b;

   return c != 0;
}

inline bool QBitArray::operator[](int i) const
{
   return testBit(i);
}

inline bool QBitArray::operator[](uint i) const
{
   return testBit(i);
}

inline bool QBitArray::at(int i) const
{
   return testBit(i);
}

class Q_CORE_EXPORT QBitRef
{
 public:
   operator bool() const {
      return a.testBit(i);
   }

   bool operator!() const {
      return !a.testBit(i);
   }

   QBitRef &operator=(const QBitRef &val) {
      a.setBit(i, val);
      return *this;
   }

   QBitRef &operator=(bool val) {
      a.setBit(i, val);
      return *this;
   }

 private:
   inline QBitRef(QBitArray &array, int idx)
      : a(array), i(idx)
   {}

   QBitArray &a;
   int i;

   friend class QBitArray;
};

inline QBitRef QBitArray::operator[](int i)
{
   Q_ASSERT(i >= 0);
   return QBitRef(*this, i);
}

inline QBitRef QBitArray::operator[](uint i)
{
   return QBitRef(*this, i);
}

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QBitArray &bitArray);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QBitArray &bitArray);

Q_DECLARE_SHARED(QBitArray)

#endif
