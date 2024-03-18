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

#ifndef QBYTEARRAY_H
#define QBYTEARRAY_H

#include <qarraydata.h>
#include <qassert.h>
#include <qnamespace.h>
#include <qrefcount.h>

#include <string.h>
#include <stdarg.h>
#include <iterator>

class QByteRef;
class QDataStream;

template <typename T>
class QList;

using QByteArrayData = QArrayData;

#ifdef truncate
#error Header file qbytearray.h must be included before any header file that defines truncate
#endif

#ifdef Q_OS_DARWIN
using CFDataRef = const struct __CFData *;

#  ifdef __OBJC__
@class NSData;
#  endif
#endif

Q_CORE_EXPORT char *qstrdup(const char *);

inline uint qstrlen(const char *str)
{
   return str ? uint(strlen(str)) : 0;
}

inline uint qstrnlen(const char *str, uint maxlen)
{
   uint length = 0;

   if (str) {
      while (length < maxlen && *str++) {
         length++;
      }
   }

   return length;
}

Q_CORE_EXPORT char *qstrcpy(char *dst, const char *src);
Q_CORE_EXPORT char *qstrncpy(char *dst, const char *src, uint len);

Q_CORE_EXPORT int qstrcmp(const char *str1, const char *str2);
Q_CORE_EXPORT int qstrcmp(const QByteArray &str1, const QByteArray &str2);
Q_CORE_EXPORT int qstrcmp(const QByteArray &str1, const char *str2);

static inline int qstrcmp(const char *str1, const QByteArray &str2)
{
   return -qstrcmp(str2, str1);
}

inline int qstrncmp(const char *str1, const char *str2, uint len)
{
   return (str1 && str2) ? strncmp(str1, str2, len) : (str1 ? 1 : (str2 ? -1 : 0));
}

Q_CORE_EXPORT int qstricmp(const char *, const char *);
Q_CORE_EXPORT int qstrnicmp(const char *, const char *, uint len);

// qChecksum: Internet checksum
Q_CORE_EXPORT quint16 qChecksum(const char *s, uint len);

template <int N>
struct QStaticByteArrayData {
   QByteArrayData value;
   char data[N + 1];

   QByteArrayData *data_ptr() const {
      Q_ASSERT(value.ref.isStatic());
      return const_cast<QByteArrayData *>(&value);
   }
};

struct QByteArrayDataPtr {
   QByteArrayData *ptr;
};

#define Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(size, offset) \
   { Q_REFCOUNT_INITIALIZE_STATIC, size, 0, 0, offset }

#define Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER(size) \
   Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(size, sizeof(QByteArrayData))

#define QByteArrayLiteral(str) \
   ([]() -> QByteArrayDataPtr { \
      constexpr const int Size = sizeof(str) - 1; \
      static const QStaticByteArrayData<Size> qbytearray_literal = \
      { { Q_REFCOUNT_INITIALIZE_STATIC, Size, 0, 0, sizeof(QByteArrayData) }, str }; \
      QByteArrayDataPtr holder = { qbytearray_literal.data_ptr() }; \
      return holder; \
   }())

class Q_CORE_EXPORT QByteArray
{
   using Data = QTypedArrayData<char>;

 public:
   using iterator         = char *;
   using const_iterator   = const char *;

   using reference        = char &;
   using const_reference  = const char &;
   using value_type       = char;
   using DataPtr          = Data *;

   using reverse_iterator       = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   QByteArray()
      : d(Data::sharedNull())
   { }

   QByteArray(const char *str, int size = -1);
   QByteArray(int size, char ch);

   QByteArray(const QByteArray &other)
      : d(other.d)
   {
      d->ref.ref();
   }

   QByteArray(QByteArray &&other)
      : d(other.d)
   {
      other.d = Data::sharedNull();
   }

   // internal
   QByteArray(int size, Qt::NoDataOverload dummy);

   // internal
   QByteArray(QByteArrayDataPtr dd)
      : d(static_cast<Data *>(dd.ptr)) {
   }

   ~QByteArray() {
      if (! d->ref.deref()) {
         Data::deallocate(d);
      }
   }

   inline char at(int i) const;

   QByteArray &append(char ch);
   QByteArray &append(const char *str);
   QByteArray &append(const char *str, int len);
   QByteArray &append(const QByteArray &value);

   inline int capacity() const;
   void clear();
   inline const char *constData() const;

   void chop(int n);

   inline bool contains(char ch) const;
   inline bool contains(const char *str) const;
   inline bool contains(const QByteArray &value) const;

   int count(char ch) const;
   int count(const char *str) const;
   int count(const QByteArray &value) const;

   int count() const {
      return d->size;
   }

   inline char *data();
   inline const char *data() const;

   inline void detach();

   DataPtr &data_ptr() {
      return d;
   }

   bool endsWith(const QByteArray &value) const;
   bool endsWith(char ch) const;
   bool endsWith(const char *str) const;

   QByteArray &fill(char ch, int size = -1);

   inline bool isEmpty() const;
   inline bool isDetached() const;

   bool isSharedWith(const QByteArray &value) const {
      return d == value.d;
   }

   int indexOf(char ch, int from = 0) const;
   int indexOf(const char *str, int from = 0) const;
   int indexOf(const QByteArray &value, int from = 0) const;

   QByteArray &insert(int i, char ch);
   QByteArray &insert(int i, const char *str);
   QByteArray &insert(int i, const char *str, int len);
   QByteArray &insert(int i, const QByteArray &value);

   bool isNull() const;

   int lastIndexOf(char ch, int from = -1) const;
   int lastIndexOf(const char *str, int from = -1) const;
   int lastIndexOf(const QByteArray &value, int from = -1) const;

   int length() const {
      return d->size;
   }

   QByteArray leftJustified(int width, char fill = ' ', bool truncate = false) const;
   QByteArray left(int len) const;

   QByteArray mid(int pos, int len = -1) const;

   inline void push_back(char ch);
   inline void push_back(const char *str);
   inline void push_back(const QByteArray &value);

   inline void push_front(char ch);
   inline void push_front(const char *str);
   inline void push_front(const QByteArray &value);

   QByteArray &prepend(char ch);
   QByteArray &prepend(const char *str);
   QByteArray &prepend(const char *str, int len);
   QByteArray &prepend(const QByteArray &value);

   QByteArray rightJustified(int width, char fill = ' ', bool truncate = false) const;
   QByteArray right(int len) const;
   void resize(int size);
   inline void reserve(int size);

   QByteArray &remove(char ch);
   QByteArray &remove(int pos, int len);

   QByteArray &replace(int pos, int len, const char *str);
   QByteArray &replace(int pos, int len, const char *str, int size);
   QByteArray &replace(int pos, int len, const QByteArray &value);

   inline QByteArray &replace(char before, const char *after);
   QByteArray &replace(char before, const QByteArray &after);
   inline QByteArray &replace(const char *before, const char *after);
   QByteArray &replace(const char *before, int bsize, const char *after, int asize);
   QByteArray &replace(const QByteArray &before, const QByteArray &after);
   inline QByteArray &replace(const QByteArray &before, const char *after);
   QByteArray &replace(const char *before, const QByteArray &after);
   QByteArray &replace(char before, char after);

   QByteArray repeated(int times) const;

   bool startsWith(const QByteArray &value) const;
   bool startsWith(char ch) const;
   bool startsWith(const char *str) const;

   QByteArray simplified() const;

   inline int size() const;
   inline void squeeze();

   void swap(QByteArray &other) {
      qSwap(d, other.d);
   }

   QList<QByteArray> split(char sep) const;

   void truncate(int pos);
   QByteArray trimmed() const;

   QByteArray toLower() const;
   QByteArray toUpper() const;

   // iterators
   inline iterator begin();
   inline const_iterator begin() const;
   inline const_iterator cbegin() const;
   inline const_iterator constBegin() const;

   inline iterator end();
   inline const_iterator end() const;
   inline const_iterator cend() const;
   inline const_iterator constEnd() const;

   reverse_iterator rbegin()  {
      return reverse_iterator(end());
   }

   const_reverse_iterator rbegin() const {
      return const_reverse_iterator(end());
   }

   reverse_iterator rend()  {
      return reverse_iterator(begin());
   }

   const_reverse_iterator rend() const {
      return const_reverse_iterator(begin());
   }

   const_reverse_iterator crbegin() const {
      return const_reverse_iterator(end());
   }

   const_reverse_iterator crend() const {
      return const_reverse_iterator(begin());
   }

   // operators
   QByteArray &operator=(const QByteArray &other);
   QByteArray &operator=(const char *str);

   QByteArray &operator=(QByteArray && other) {
      qSwap(d, other.d);
      return *this;
   }

   inline char operator[](int i) const;
   inline char operator[](uint i) const;

   inline QByteRef operator[](int i);
   inline QByteRef operator[](uint i);

   inline QByteArray &operator+=(char ch);
   inline QByteArray &operator+=(const char *str);
   inline QByteArray &operator+=(const QByteArray &value);

   short toShort(bool *ok = nullptr, int base = 10) const;
   ushort toUShort(bool *ok = nullptr, int base = 10) const;
   int toInt(bool *ok = nullptr, int base = 10) const;
   uint toUInt(bool *ok = nullptr, int base = 10) const;
   long toLong(bool *ok = nullptr, int base = 10) const;
   ulong toULong(bool *ok = nullptr, int base = 10) const;
   qint64 toLongLong(bool *ok = nullptr, int base = 10) const;
   quint64 toULongLong(bool *ok = nullptr, int base = 10) const;
   float toFloat(bool *ok = nullptr) const;
   double toDouble(bool *ok = nullptr) const;
   QByteArray toBase64() const;
   QByteArray toHex() const;

   QByteArray toPercentEncoding(const QByteArray &exclude = QByteArray(),
         const QByteArray &include = QByteArray(), char percent = '%') const;

   inline QByteArray &setNum(short n, int base = 10);
   inline QByteArray &setNum(ushort n, int base = 10);
   inline QByteArray &setNum(int n, int base = 10);
   inline QByteArray &setNum(uint n, int base = 10);
   QByteArray &setNum(qint64 n, int base = 10);
   QByteArray &setNum(quint64 n, int base = 10);
   inline QByteArray &setNum(float n, char f = 'g', int prec = 6);
   QByteArray &setNum(double n, char f = 'g', int prec = 6);
   QByteArray &setRawData(const char *str, int size);

   static QByteArray number(int n, int base = 10);
   static QByteArray number(uint n, int base = 10);
   static QByteArray number(qint64 n, int base = 10);
   static QByteArray number(quint64 n, int base = 10);
   static QByteArray number(double n, char f = 'g', int prec = 6);
   static QByteArray fromRawData(const char *str, int size);
   static QByteArray fromBase64(const QByteArray &value);
   static QByteArray fromHex(const QByteArray &value);
   static QByteArray fromPercentEncoding(const QByteArray &value, char percent = '%');

#if defined(Q_OS_DARWIN)
   static QByteArray fromCFData(CFDataRef data);
   static QByteArray fromRawCFData(CFDataRef data);
   CFDataRef toCFData() const;
   CFDataRef toRawCFData() const;

#if defined(__OBJC__)
   static QByteArray fromNSData(const NSData *data);
   static QByteArray fromRawNSData(const NSData *data);
   NSData *toNSData() const;
   NSData *toRawNSData() const;
#endif

#endif

 private:
   void reallocData(uint alloc, Data::AllocationOptions options);
   void expand(int i);
   QByteArray nulTerminated() const;

   Data *d;

   friend class QByteRef;
   friend Q_CORE_EXPORT QByteArray qUncompress(const uchar *data, int nbytes);
};

inline int QByteArray::size() const
{
   return d->size;
}

inline char QByteArray::at(int i) const
{
   Q_ASSERT(i >= 0 && i < size());
   return d->data()[i];
}

inline char QByteArray::operator[](int i) const
{
   Q_ASSERT(i >= 0 && i < size());
   return d->data()[i];
}

inline char QByteArray::operator[](uint i) const
{
   Q_ASSERT(i < uint(size()));
   return d->data()[i];
}

inline bool QByteArray::isEmpty() const
{
   return d->size == 0;
}

inline char *QByteArray::data()
{
   detach();
   return d->data();
}

inline const char *QByteArray::data() const
{
   return d->data();
}

inline const char *QByteArray::constData() const
{
   return d->data();
}

inline void QByteArray::detach()
{
   if (d->ref.isShared() || (d->offset != sizeof(QByteArrayData))) {
      reallocData(uint(d->size) + 1u, d->detachFlags());
   }
}

inline bool QByteArray::isDetached() const
{
   return !d->ref.isShared();
}

inline int QByteArray::capacity() const
{
   return d->alloc ? d->alloc - 1 : 0;
}

inline void QByteArray::reserve(int size)
{
   if (d->ref.isShared() || uint(size) + 1u > d->alloc) {
      reallocData(uint(size) + 1u, d->detachFlags() | Data::CapacityReserved);
   } else {
      // cannot set unconditionally, since d could be the shared_null or
      // otherwise static
      d->capacityReserved = true;
   }
}

inline void QByteArray::squeeze()
{
   if (d->ref.isShared() || uint(d->size) + 1u < d->alloc) {
      reallocData(uint(d->size) + 1u, d->detachFlags() & ~Data::CapacityReserved);
   } else {
      // cannot set unconditionally, since d could be shared_null or static
      d->capacityReserved = false;
   }
}

class Q_CORE_EXPORT QByteRef
{
 public:
   operator char() const {
      return i < a.d->size ? a.d->data()[i] : char(0);
   }

   QByteRef &operator=(char c) {
      if (i >= a.d->size) {
         a.expand(i);
      } else {
         a.detach();
      }

      a.d->data()[i] = c;
      return *this;
   }

   QByteRef &operator=(const QByteRef &c) {
      if (i >= a.d->size) {
         a.expand(i);
      } else {
         a.detach();
      }

      a.d->data()[i] = c.a.d->data()[c.i];
      return *this;
   }

   bool operator==(char c) const {
      return a.d->data()[i] == c;
   }

   bool operator!=(char c) const {
      return a.d->data()[i] != c;
   }

   bool operator>(char c) const {
      return a.d->data()[i] > c;
   }

   bool operator>=(char c) const {
      return a.d->data()[i] >= c;
   }

   bool operator<(char c) const {
      return a.d->data()[i] < c;
   }

   bool operator<=(char c) const {
      return a.d->data()[i] <= c;
   }

 private:
   QByteRef(QByteArray &array, int idx): a(array), i(idx) {
   }

   QByteArray &a;
   int i;

   friend class QByteArray;
};

inline QByteRef QByteArray::operator[](int i)
{
   Q_ASSERT(i >= 0);
   return QByteRef(*this, i);
}

inline QByteRef QByteArray::operator[](uint i)
{
   return QByteRef(*this, i);
}

inline QByteArray::iterator QByteArray::begin()
{
   detach();
   return d->data();
}

inline QByteArray::const_iterator QByteArray::begin() const
{
   return d->data();
}

inline QByteArray::const_iterator QByteArray::cbegin() const
{
   return d->data();
}

inline QByteArray::const_iterator QByteArray::constBegin() const
{
   return d->data();
}

inline QByteArray::iterator QByteArray::end()
{
   detach();
   return d->data() + d->size;
}

inline QByteArray::const_iterator QByteArray::end() const
{
   return d->data() + d->size;
}

inline QByteArray::const_iterator QByteArray::cend() const
{
   return d->data() + d->size;
}

inline QByteArray::const_iterator QByteArray::constEnd() const
{
   return d->data() + d->size;
}

inline QByteArray &QByteArray::operator+=(char ch)
{
   return append(ch);
}

inline QByteArray &QByteArray::operator+=(const char *str)
{
   return append(str);
}

inline QByteArray &QByteArray::operator+=(const QByteArray &value)
{
   return append(value);
}

inline void QByteArray::push_back(char ch)
{
   append(ch);
}

inline void QByteArray::push_back(const char *str)
{
   append(str);
}

inline void QByteArray::push_back(const QByteArray &value)
{
   append(value);
}

inline void QByteArray::push_front(char ch)
{
   prepend(ch);
}

inline void QByteArray::push_front(const char *str)
{
   prepend(str);
}

inline void QByteArray::push_front(const QByteArray &value)
{
   prepend(value);
}

inline bool QByteArray::contains(const QByteArray &value) const
{
   return bool(indexOf(value) != -1);
}

inline bool QByteArray::contains(char ch) const
{
   return bool(indexOf(ch) != -1);
}

inline bool operator==(const QByteArray &a1, const QByteArray &a2)
{
   return (a1.size() == a2.size()) && (memcmp(a1.constData(), a2.constData(), a1.size()) == 0);
}

inline bool operator==(const QByteArray &a1, const char *a2)
{
   return a2 ? qstrcmp(a1, a2) == 0 : a1.isEmpty();
}

inline bool operator==(const char *a1, const QByteArray &a2)
{
   return a1 ? qstrcmp(a1, a2) == 0 : a2.isEmpty();
}

inline bool operator!=(const QByteArray &a1, const QByteArray &a2)
{
   return !(a1 == a2);
}

inline bool operator!=(const QByteArray &a1, const char *a2)
{
   return a2 ? qstrcmp(a1, a2) != 0 : !a1.isEmpty();
}

inline bool operator!=(const char *a1, const QByteArray &a2)
{
   return a1 ? qstrcmp(a1, a2) != 0 : !a2.isEmpty();
}

inline bool operator<(const QByteArray &a1, const QByteArray &a2)
{
   return qstrcmp(a1, a2) < 0;
}

inline bool operator<(const QByteArray &a1, const char *a2)
{
   return qstrcmp(a1, a2) < 0;
}

inline bool operator<(const char *a1, const QByteArray &a2)
{
   return qstrcmp(a1, a2) < 0;
}

inline bool operator<=(const QByteArray &a1, const QByteArray &a2)
{
   return qstrcmp(a1, a2) <= 0;
}

inline bool operator<=(const QByteArray &a1, const char *a2)
{
   return qstrcmp(a1, a2) <= 0;
}

inline bool operator<=(const char *a1, const QByteArray &a2)
{
   return qstrcmp(a1, a2) <= 0;
}

inline bool operator>(const QByteArray &a1, const QByteArray &a2)
{
   return qstrcmp(a1, a2) > 0;
}

inline bool operator>(const QByteArray &a1, const char *a2)
{
   return qstrcmp(a1, a2) > 0;
}

inline bool operator>(const char *a1, const QByteArray &a2)
{
   return qstrcmp(a1, a2) > 0;
}

inline bool operator>=(const QByteArray &a1, const QByteArray &a2)
{
   return qstrcmp(a1, a2) >= 0;
}

inline bool operator>=(const QByteArray &a1, const char *a2)
{
   return qstrcmp(a1, a2) >= 0;
}

inline bool operator>=(const char *a1, const QByteArray &a2)
{
   return qstrcmp(a1, a2) >= 0;
}

inline const QByteArray operator+(const QByteArray &a1, const QByteArray &a2)
{
   return QByteArray(a1) += a2;
}

inline const QByteArray operator+(const QByteArray &a1, const char *a2)
{
   return QByteArray(a1) += a2;
}

inline const QByteArray operator+(const QByteArray &a1, char a2)
{
   return QByteArray(a1) += a2;
}

inline const QByteArray operator+(const char *a1, const QByteArray &a2)
{
   return QByteArray(a1) += a2;
}

inline const QByteArray operator+(char a1, const QByteArray &a2)
{
   return QByteArray(&a1, 1) += a2;
}

inline bool QByteArray::contains(const char *str) const
{
   return bool(indexOf(str) != -1);
}

inline QByteArray &QByteArray::replace(char before, const char *after)
{
   return replace(&before, 1, after, qstrlen(after));
}

inline QByteArray &QByteArray::replace(const QByteArray &before, const char *after)
{
   return replace(before.constData(), before.size(), after, qstrlen(after));
}

inline QByteArray &QByteArray::replace(const char *before, const char *after)
{
   return replace(before, qstrlen(before), after, qstrlen(after));
}

inline QByteArray &QByteArray::setNum(short n, int base)
{
   return base == 10 ? setNum(qint64(n), base) : setNum(quint64(ushort(n)), base);
}

inline QByteArray &QByteArray::setNum(ushort n, int base)
{
   return setNum(quint64(n), base);
}

inline QByteArray &QByteArray::setNum(int n, int base)
{
   return base == 10 ? setNum(qint64(n), base) : setNum(quint64(uint(n)), base);
}

inline QByteArray &QByteArray::setNum(uint n, int base)
{
   return setNum(quint64(n), base);
}

inline QByteArray &QByteArray::setNum(float n, char f, int prec)
{
   return setNum(double(n), f, prec);
}

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QByteArray &byteArray);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QByteArray &byteArray);

Q_CORE_EXPORT QByteArray qCompress(const uchar *data, int nbytes, int compressionLevel = -1);
Q_CORE_EXPORT QByteArray qUncompress(const uchar *data, int nbytes);

inline QByteArray qCompress(const QByteArray &value, int compressionLevel = -1)
{
   return qCompress(reinterpret_cast<const uchar *>(value.constData()), value.size(), compressionLevel);
}

inline QByteArray qUncompress(const QByteArray &value)
{
   return qUncompress(reinterpret_cast<const uchar *>(value.constData()), value.size());
}

Q_DECLARE_SHARED(QByteArray)

#endif
