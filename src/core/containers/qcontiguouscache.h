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

#ifndef QCONTIGUOUSCACHE_H
#define QCONTIGUOUSCACHE_H

#include <qassert.h>
#include <qatomic.h>
#include <qglobal.h>

#include <limits.h>
#include <new>

struct Q_CORE_EXPORT QContiguousCacheData {
   QAtomicInt ref;
   int alloc;
   int count;
   int start;
   int offset;
   uint sharable : 1;
   uint reserved : 31;

   // total is 24 bytes (HP-UX aCC: 40 bytes)
   // the next entry is already aligned to 8 bytes
   // there will be an 8 byte gap here if T requires 16-byte alignment
   // (such as long double on 64-bit platforms, __int128, __float128)

   static QContiguousCacheData *allocate(int size, int alignment);
   static void free(QContiguousCacheData *data);
};

template <typename T>
struct QContiguousCacheTypedData: private QContiguousCacheData {
   // private inheritance to avoid aliasing warningss
   T array[1];

   static void free(QContiguousCacheTypedData *data) {
      QContiguousCacheData::free(data);
   }
};

template <typename T>
class QContiguousCache
{
   using Data = QContiguousCacheTypedData<T>;

   union {
      QContiguousCacheData *d;
      QContiguousCacheTypedData<T> *p;
   };

 public:
   using value_type       = T;
   using pointer          = value_type *;
   using const_pointer    = const value_type *;
   using reference        = value_type &;
   using const_reference  = const value_type &;
   using difference_type  = qptrdiff;
   using size_type        = int;

   explicit QContiguousCache(int capacity = 0);

   QContiguousCache(const QContiguousCache<T> &other)
      : d(other.d)
   {
      d->ref.ref();

      if (! d->sharable) {
         detach_helper();
      }
   }

   ~QContiguousCache() {
      if (! d) {
         return;
      }

      if (! d->ref.deref()) {
         free(p);
      }
   }

   void detach() {
      if (d->ref.load() != 1) {
         detach_helper();
      }
   }

   bool isDetached() const {
      return d->ref.load() == 1;
   }

   void setSharable(bool sharable) {
      if (! sharable) {
         detach();
      }

      d->sharable = sharable;
   }

   QContiguousCache<T> &operator=(const QContiguousCache<T> &other);

   QContiguousCache<T> &operator=(QContiguousCache<T> && other) {
      qSwap(d, other.d);
      return *this;
   }

   void swap(QContiguousCache<T> &other) {
      qSwap(d, other.d);
   }

   bool operator==(const QContiguousCache<T> &other) const;

   bool operator!=(const QContiguousCache<T> &other) const {
      return !(*this == other);
   }

   int capacity() const {
      return d->alloc;
   }

   int count() const {
      return d->count;
   }

   int size() const {
      return d->count;
   }

   bool isEmpty() const {
      return d->count == 0;
   }

   bool isFull() const {
      return d->count == d->alloc;
   }

   int available() const {
      return d->alloc - d->count;
   }

   void clear();
   void setCapacity(int size);

   const T &at(int pos) const;
   T &operator[](int pos);
   const T &operator[](int pos) const;

   void append(const T &value);
   void prepend(const T &value);
   void insert(int pos, const T &value);

   bool containsIndex(int pos) const {
      return pos >= d->offset && pos - d->offset < d->count;
   }

   int firstIndex() const {
      return d->offset;
   }

   int lastIndex() const {
      return d->offset + d->count - 1;
   }

   const T &first() const {
      Q_ASSERT(!isEmpty());
      return p->array[d->start];
   }

   const T &last() const {
      Q_ASSERT(!isEmpty());
      return p->array[(d->start + d->count - 1) % d->alloc];
   }

   T &first() {
      Q_ASSERT(!isEmpty());
      detach();
      return p->array[d->start];
   }

   T &last() {
      Q_ASSERT(!isEmpty());
      detach();
      return p->array[(d->start + d->count - 1) % d->alloc];
   }

   void removeFirst();
   T takeFirst();
   void removeLast();
   T takeLast();

   bool areIndexesValid() const {
      return d->offset >= 0 && d->offset < INT_MAX - d->count && (d->offset % d->alloc) == d->start;
   }

   void normalizeIndexes() {
      d->offset = d->start;
   }

 private:
   void detach_helper();

   QContiguousCacheData *malloc(int aalloc);
   void free(Data *x);

   int sizeOfTypedData() {
      // this is more or less the same as sizeof(Data), except that it doesn't
      // count the padding at the end
      return reinterpret_cast<const char *>(&(reinterpret_cast<const Data *>(this))->array[1]) -
            reinterpret_cast<const char *>(this);
   }

   int alignOfTypedData() const {
      return qMax(sizeof(void *), alignof(Data));
   }
};

template <typename T>
void QContiguousCache<T>::detach_helper()
{
   union {
      QContiguousCacheData *d;
      QContiguousCacheTypedData<T> *p;
   } x;

   x.d = malloc(d->alloc);
   x.d->ref.store(1);
   x.d->count    = d->count;
   x.d->start    = d->start;
   x.d->offset   = d->offset;
   x.d->alloc    = d->alloc;
   x.d->sharable = true;
   x.d->reserved = 0;

   T *dest = x.p->array + x.d->start;
   T *src  = p->array + d->start;
   int oldcount = x.d->count;

   while (oldcount--) {
      if (std::is_trivially_constructible_v<T>) {
         *dest = *src;
      } else {
         new (dest) T(*src);
      }

      ++dest;

      if (dest == x.p->array + x.d->alloc) {
         dest = x.p->array;
      }

      ++src;

      if (src == p->array + d->alloc) {
         src = p->array;
      }
   }

   if (! d->ref.deref()) {
      free(p);
   }

   d = x.d;
}

template <typename T>
void QContiguousCache<T>::setCapacity(int size)
{
   if (size == d->alloc) {
      return;
   }

   detach();

   union {
      QContiguousCacheData *d;
      QContiguousCacheTypedData<T> *p;
   } x;

   x.d = malloc(size);
   x.d->alloc  = size;
   x.d->count  = qMin(d->count, size);
   x.d->offset = d->offset + d->count - x.d->count;

   if (size) {
      x.d->start = x.d->offset % x.d->alloc;
   } else {
      x.d->start = 0;
   }

   int oldcount = x.d->count;

   if (oldcount) {
      T *dest = x.p->array + (x.d->start + x.d->count - 1) % x.d->alloc;
      T *src = p->array + (d->start + d->count - 1) % d->alloc;

      while (oldcount--) {

         if (std::is_trivially_constructible_v<T>) {
            *dest = *src;
         } else {
            new (dest) T(*src);
         }

         if (dest == x.p->array) {
            dest = x.p->array + x.d->alloc;
         }

         dest--;

         if (src == p->array) {
            src = p->array + d->alloc;
         }

         src--;
      }
   }

   /* free old */
   free(p);
   d = x.d;
}

template <typename T>
void QContiguousCache<T>::clear()
{
   if (d->ref.load() == 1) {
      if (! std::is_trivially_destructible_v<T>) {
         int oldcount = d->count;
         T *i = p->array + d->start;
         T *e = p->array + d->alloc;

         while (oldcount--) {
            i->~T();
            i++;

            if (i == e) {
               i = p->array;
            }
         }
      }

      d->count = d->start = d->offset = 0;

   } else {
      union {
         QContiguousCacheData *d;
         QContiguousCacheTypedData<T> *p;
      } x;

      x.d = malloc(d->alloc);
      x.d->ref.store(1);
      x.d->alloc = d->alloc;
      x.d->count = x.d->start = x.d->offset = 0;
      x.d->sharable = true;

      if (!d->ref.deref()) {
         free(p);
      }

      d = x.d;
   }
}

template <typename T>
inline QContiguousCacheData *QContiguousCache<T>::malloc(int aalloc)
{
   return QContiguousCacheData::allocate(sizeOfTypedData() + (aalloc - 1) * sizeof(T), alignOfTypedData());
}

template <typename T>
QContiguousCache<T>::QContiguousCache(int capacity)
{
   d = malloc(capacity);
   d->ref.store(1);

   d->alloc    = capacity;
   d->count    = 0;
   d->start    = 0;
   d->offset   = 0;
   d->sharable = true;
}

template <typename T>
QContiguousCache<T> &QContiguousCache<T>::operator=(const QContiguousCache<T> &other)
{
   other.d->ref.ref();

   if (! d->ref.deref()) {
      free(d);
   }

   d = other.d;

   if (! d->sharable) {
      detach_helper();
   }

   return *this;
}

template <typename T>
bool QContiguousCache<T>::operator==(const QContiguousCache<T> &other) const
{
   if (other.d == d) {
      return true;
   }

   if (other.d->start != d->start
         || other.d->count != d->count
         || other.d->offset != d->offset
         || other.d->alloc != d->alloc) {
      return false;
   }

   for (int i = firstIndex(); i <= lastIndex(); ++i)
      if (!(at(i) == other.at(i))) {
         return false;
      }

   return true;
}

template <typename T>
void QContiguousCache<T>::free(Data *x)
{
   if (! std::is_trivially_destructible_v<T>) {
      int oldcount = d->count;
      T *i = p->array + d->start;
      T *e = p->array + d->alloc;

      while (oldcount--) {
         i->~T();
         i++;

         if (i == e) {
            i = p->array;
         }
      }
   }

   x->free(x);
}

template <typename T>
void QContiguousCache<T>::append(const T &value)
{
   if (! d->alloc) {
      return;   // zero capacity
   }

   detach();

   if (std::is_trivially_constructible_v<T> && std::is_trivially_destructible_v<T>) {
      p->array[(d->start + d->count) % d->alloc] = value;

   } else {
      if (d->count == d->alloc) {
         (p->array + (d->start + d->count) % d->alloc)->~T();
      }

      new (p->array + (d->start + d->count) % d->alloc) T(value);
   }

   if (d->count == d->alloc) {
      d->start++;
      d->start %= d->alloc;
      d->offset++;
   } else {
      d->count++;
   }
}

template <typename T>
void QContiguousCache<T>::prepend(const T &value)
{
   if (!d->alloc) {
      return;   // zero capacity
   }

   detach();

   if (d->start) {
      d->start--;
   } else {
      d->start = d->alloc - 1;
   }

   d->offset--;

   if (d->count != d->alloc) {
      d->count++;
   } else if (d->count == d->alloc) {
      (p->array + d->start)->~T();
   }

   if (std::is_trivially_constructible_v<T>) {
      p->array[d->start] = value;
   } else {
      new (p->array + d->start) T(value);
   }
}

template <typename T>
void QContiguousCache<T>::insert(int pos, const T &value)
{
   Q_ASSERT_X(pos >= 0 && pos < INT_MAX, "QContiguousCache<T>::insert", "index out of range");

   if (! d->alloc) {
      return;   // zero capacity
   }

   detach();

   if (containsIndex(pos)) {

      if (std::is_trivially_constructible_v<T> && std::is_trivially_destructible_v<T>) {
         p->array[pos % d->alloc] = value;
      } else {
         (p->array + pos % d->alloc)->~T();
         new (p->array + pos % d->alloc) T(value);
      }

   } else if (pos == d->offset - 1) {
      prepend(value);

   } else if (pos == d->offset + d->count) {
      append(value);

   } else {
      // we do not leave gaps
      clear();
      d->offset = pos;
      d->start  = pos % d->alloc;
      d->count  = 1;

      if (std::is_trivially_constructible_v<T>) {
         p->array[d->start] = value;
      } else {
         new (p->array + d->start) T(value);
      }
   }
}

template <typename T>
inline const T &QContiguousCache<T>::at(int pos) const
{
   Q_ASSERT_X(pos >= d->offset && pos - d->offset < d->count, "QContiguousCache<T>::at", "index out of range");
   return p->array[pos % d->alloc];
}

template <typename T>
inline const T &QContiguousCache<T>::operator[](int pos) const
{
   Q_ASSERT_X(pos >= d->offset && pos - d->offset < d->count, "QContiguousCache<T>::at", "index out of range");
   return p->array[pos % d->alloc];
}

template <typename T>
inline T &QContiguousCache<T>::operator[](int pos)
{
   detach();

   if (! containsIndex(pos)) {
      insert(pos, T());
   }

   return p->array[pos % d->alloc];
}

template <typename T>
inline void QContiguousCache<T>::removeFirst()
{
   Q_ASSERT(d->count > 0);
   detach();

   --d->count;

   if (! std::is_trivially_destructible_v<T>) {
      (p->array + d->start)->~T();
   }

   d->start = (d->start + 1) % d->alloc;
   ++d->offset;
}

template <typename T>
inline void QContiguousCache<T>::removeLast()
{
   Q_ASSERT(d->count > 0);
   detach();
   d->count--;

   if (! std::is_trivially_destructible_v<T>) {
      (p->array + (d->start + d->count) % d->alloc)->~T();
   }
}

template <typename T>
inline T QContiguousCache<T>::takeFirst()
{
   T t = first();
   removeFirst();

   return t;
}

template <typename T>
inline T QContiguousCache<T>::takeLast()
{
   T t = last();
   removeLast();
   return t;
}

#endif
