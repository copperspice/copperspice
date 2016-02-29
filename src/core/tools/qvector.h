/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QVECTOR_H
#define QVECTOR_H

#include <QtCore/qalgorithms.h>
#include <QtCore/qlist.h>
#include <QtCore/qrefcount.h>
#include <QtCore/qarraydata.h>

#include <iterator>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <initializer_list>

QT_BEGIN_NAMESPACE

class QRegion;

template <typename T>
class QVector
{
   typedef QTypedArrayData<T> Data;
   Data *d;

 public:
   inline QVector() : d(Data::sharedNull()) { }
   explicit QVector(int size);
   QVector(int size, const T &t);
   inline QVector(const QVector<T> &v);

   inline ~QVector() {
      if (!d->ref.deref()) {
         free(d);
      }
   }

   QVector<T> &operator=(const QVector<T> &v);

   inline QVector<T> operator=(QVector<T> && other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QVector<T> &other) {
      qSwap(d, other.d);
   }

   inline QVector(std::initializer_list<T> args);

   bool operator==(const QVector<T> &v) const;
   inline bool operator!=(const QVector<T> &v) const {
      return !(*this == v);
   }

   inline int size() const {
      return d->size;
   }

   inline bool isEmpty() const {
      return d->size == 0;
   }

   void resize(int size);

   inline int capacity() const {
      return int(d->alloc);
   }

   void reserve(int size);

   inline void squeeze() {
      realloc(d->size, d->size);
      if (d->capacityReserved) {
         // capacity reserved in a read only memory would be useless
         // this checks avoid writing to such memory.
         d->capacityReserved = 0;
      }
   }

   inline void detach();
   inline bool isDetached() const {
      return !d->ref.isShared();
   }

   inline void setSharable(bool sharable) {
      if (sharable == d->ref.isSharable()) {
         return;
      }
      if (!sharable) {
         detach();
      }

      if (d == Data::unsharableEmpty()) {
         if (sharable) {
            d = Data::sharedNull();
         }
      } else {
         d->ref.setSharable(sharable);
      }
      Q_ASSERT(d->ref.isSharable() == sharable);
   }

   inline bool isSharedWith(const QVector<T> &other) const {
      return d == other.d;
   }

   inline T *data() {
      detach();
      return d->begin();
   }

   inline const T *data() const {
      return d->begin();
   }

   inline const T *constData() const {
      return d->begin();
   }
   void clear();

   const T &at(int i) const;
   T &operator[](int i);
   const T &operator[](int i) const;
   void append(const T &t);
   void prepend(const T &t);
   void insert(int i, const T &t);
   void insert(int i, int n, const T &t);
   void replace(int i, const T &t);
   void remove(int i);
   void remove(int i, int n);

   QVector<T> &fill(const T &t, int size = -1);

   int indexOf(const T &t, int from = 0) const;
   int lastIndexOf(const T &t, int from = -1) const;
   bool contains(const T &t) const;
   int count(const T &t) const;

#ifdef QT_STRICT_ITERATORS
   class iterator
   {
    public:
      T *i;
      typedef std::random_access_iterator_tag  iterator_category;
      typedef qptrdiff difference_type;
      typedef T value_type;
      typedef T *pointer;
      typedef T &reference;

      inline iterator() : i(0) {}
      inline iterator(T *n) : i(n) {}
      inline iterator(const iterator &o): i(o.i) {}
      inline T &operator*() const {
         return *i;
      }
      inline T *operator->() const {
         return i;
      }
      inline T &operator[](int j) const {
         return *(i + j);
      }
      inline bool operator==(const iterator &o) const {
         return i == o.i;
      }
      inline bool operator!=(const iterator &o) const {
         return i != o.i;
      }
      inline bool operator<(const iterator &other) const {
         return i < other.i;
      }
      inline bool operator<=(const iterator &other) const {
         return i <= other.i;
      }
      inline bool operator>(const iterator &other) const {
         return i > other.i;
      }
      inline bool operator>=(const iterator &other) const {
         return i >= other.i;
      }
      inline iterator &operator++() {
         ++i;
         return *this;
      }
      inline iterator operator++(int) {
         T *n = i;
         ++i;
         return n;
      }
      inline iterator &operator--() {
         i--;
         return *this;
      }
      inline iterator operator--(int) {
         T *n = i;
         i--;
         return n;
      }
      inline iterator &operator+=(int j) {
         i += j;
         return *this;
      }
      inline iterator &operator-=(int j) {
         i -= j;
         return *this;
      }
      inline iterator operator+(int j) const {
         return iterator(i + j);
      }
      inline iterator operator-(int j) const {
         return iterator(i - j);
      }
      inline int operator-(iterator j) const {
         return i - j.i;
      }
   };
   friend class iterator;

   class const_iterator
   {
    public:
      T *i;
      typedef std::random_access_iterator_tag  iterator_category;
      typedef qptrdiff difference_type;
      typedef T value_type;
      typedef const T *pointer;
      typedef const T &reference;

      inline const_iterator() : i(0) {}
      inline const_iterator(T *n) : i(n) {}
      inline const_iterator(const const_iterator &o): i(o.i) {}
      inline explicit const_iterator(const iterator &o): i(o.i) {}
      inline const T &operator*() const {
         return *i;
      }
      inline const T *operator->() const {
         return i;
      }
      inline const T &operator[](int j) const {
         return *(i + j);
      }
      inline bool operator==(const const_iterator &o) const {
         return i == o.i;
      }
      inline bool operator!=(const const_iterator &o) const {
         return i != o.i;
      }
      inline bool operator<(const const_iterator &other) const {
         return i < other.i;
      }
      inline bool operator<=(const const_iterator &other) const {
         return i <= other.i;
      }
      inline bool operator>(const const_iterator &other) const {
         return i > other.i;
      }
      inline bool operator>=(const const_iterator &other) const {
         return i >= other.i;
      }
      inline const_iterator &operator++() {
         ++i;
         return *this;
      }
      inline const_iterator operator++(int) {
         T *n = i;
         ++i;
         return n;
      }
      inline const_iterator &operator--() {
         i--;
         return *this;
      }
      inline const_iterator operator--(int) {
         T *n = i;
         i--;
         return n;
      }
      inline const_iterator &operator+=(int j) {
         i += j;
         return *this;
      }
      inline const_iterator &operator-=(int j) {
         i -= j;
         return *this;
      }
      inline const_iterator operator+(int j) const {
         return const_iterator(i + j);
      }
      inline const_iterator operator-(int j) const {
         return const_iterator(i - j);
      }
      inline int operator-(const_iterator j) const {
         return i - j.i;
      }
   };

   friend class const_iterator;
#else
   // STL-style
   typedef T *iterator;
   typedef const T *const_iterator;
#endif

   inline iterator begin() {
      detach();
      return d->begin();
   }
   inline const_iterator begin() const {
      return d->begin();
   }
   inline const_iterator constBegin() const {
      return d->begin();
   }
   inline iterator end() {
      detach();
      return d->end();
   }
   inline const_iterator end() const {
      return d->end();
   }
   inline const_iterator constEnd() const {
      return d->end();
   }
   iterator insert(iterator before, int n, const T &x);
   inline iterator insert(iterator before, const T &x) {
      return insert(before, 1, x);
   }
   iterator erase(iterator begin, iterator end);
   inline iterator erase(iterator pos) {
      return erase(pos, pos + 1);
   }

   inline int count() const {
      return d->size;
   }
   inline T &first() {
      Q_ASSERT(!isEmpty());
      return *begin();
   }
   inline const T &first() const {
      Q_ASSERT(!isEmpty());
      return *begin();
   }
   inline T &last() {
      Q_ASSERT(!isEmpty());
      return *(end() - 1);
   }
   inline const T &last() const {
      Q_ASSERT(!isEmpty());
      return *(end() - 1);
   }
   inline bool startsWith(const T &t) const {
      return !isEmpty() && first() == t;
   }
   inline bool endsWith(const T &t) const {
      return !isEmpty() && last() == t;
   }
   QVector<T> mid(int pos, int length = -1) const;

   T value(int i) const;
   T value(int i, const T &defaultValue) const;

   // STL compatibility
   typedef T value_type;
   typedef value_type *pointer;
   typedef const value_type *const_pointer;
   typedef value_type &reference;
   typedef const value_type &const_reference;
   typedef qptrdiff difference_type;
   typedef iterator Iterator;
   typedef const_iterator ConstIterator;
   typedef int size_type;

   inline void push_back(const T &t) {
      append(t);
   }

   inline void push_front(const T &t) {
      prepend(t);
   }

   void pop_back() {
      Q_ASSERT(!isEmpty());
      erase(d->end() - 1);
   }

   void pop_front() {
      Q_ASSERT(!isEmpty());
      erase(d->begin());
   }

   inline bool empty() const {
      return d->size == 0;
   }

   inline T &front() {
      return first();
   }
   inline const_reference front() const {
      return first();
   }
   inline reference back() {
      return last();
   }
   inline const_reference back() const {
      return last();
   }

   //
   QVector<T> &operator+=(const QVector<T> &l);

   inline QVector<T> operator+(const QVector<T> &l) const {
      QVector n = *this;
      n += l;
      return n;
   }

   inline QVector<T> &operator+=(const T &t) {
      append(t);
      return *this;
   }

   inline QVector<T> &operator<< (const T &t) {
      append(t);
      return *this;
   }

   inline QVector<T> &operator<<(const QVector<T> &l) {
      *this += l;
      return *this;
   }

   QList<T> toList() const;

   static QVector<T> fromList(const QList<T> &list);

   static inline QVector<T> fromStdVector(const std::vector<T> &vector) {
      QVector<T> tmp;
      tmp.reserve(int(vector.size()));
      qCopy(vector.begin(), vector.end(), std::back_inserter(tmp));
      return tmp;
   }

   inline std::vector<T> toStdVector() const {
      std::vector<T> tmp;
      tmp.reserve(size());
      qCopy(constBegin(), constEnd(), std::back_inserter(tmp));
      return tmp;
   }

 private:
   friend class QRegion; // Optimization for QRegion::rects()

   void realloc(const int size, const int alloc, QArrayData::AllocationOptions options = QArrayData::Default);
   void free(Data *d);
   void defaultConstruct(T *from, T *to);
   void copyConstruct(const T *srcFrom, const T *srcTo, T *dstFrom);
   void destruct(T *from, T *to);

   class AlignmentDummy
   {
      Data header;
      T array[1];
   };
};

template <typename T>
void QVector<T>::defaultConstruct(T *from, T *to)
{
   if (QTypeInfo<T>::isComplex) {
      while (from != to) {
         new (from++) T();
      }
   } else {
      ::memset(from, 0, (to - from) * sizeof(T));
   }
}

template <typename T>
void QVector<T>::copyConstruct(const T *srcFrom, const T *srcTo, T *dstFrom)
{
   if (QTypeInfo<T>::isComplex) {
      while (srcFrom != srcTo) {
         new (dstFrom++) T(*srcFrom++);
      }
   } else {
      ::memcpy(dstFrom, srcFrom, (srcTo - srcFrom) * sizeof(T));
   }
}

template <typename T>
void QVector<T>::destruct(T *from, T *to)
{
   if (QTypeInfo<T>::isComplex) {
      while (from != to) {
         from++->~T();
      }
   }
}

template <typename T>
inline QVector<T>::QVector(const QVector<T> &v)
{
   if (v.d->ref.ref()) {
      d = v.d;
   } else {
      if (v.d->capacityReserved) {
         d = Data::allocate(v.d->alloc);
         d->capacityReserved = true;
      } else {
         d = Data::allocate(v.d->size);
      }
      if (d->alloc) {
         copyConstruct(v.d->begin(), v.d->end(), d->begin());
         d->size = v.d->size;
      }
   }
}

template <typename T>
void QVector<T>::detach()
{
   if (!isDetached()) {
      if (d->alloc) {
         realloc(d->size, int(d->alloc));
      } else {
         d = Data::unsharableEmpty();
      }
   }
   Q_ASSERT(isDetached());
}

template <typename T>
void QVector<T>::reserve(int asize)
{
   if (asize > int(d->alloc)) {
      realloc(d->size, asize);
   }

   if (isDetached()) {
      d->capacityReserved = 1;
   }

   Q_ASSERT(capacity() >= asize);
}

template <typename T>
void QVector<T>::resize(int asize)
{
   int newAlloc;
   const int oldAlloc = int(d->alloc);
   QArrayData::AllocationOptions opt;

   if (asize > oldAlloc) { // there is not enough space
      newAlloc = asize;
      opt = QArrayData::Grow;
   } else if (!d->capacityReserved && asize < d->size && asize < (oldAlloc >> 1)) { // we want to shrink
      newAlloc = asize;
      opt = QArrayData::Grow;
   } else {
      newAlloc = oldAlloc;
   }
   realloc(asize, newAlloc, opt);
}

template <typename T>
inline void QVector<T>::clear()
{
   *this = QVector<T>();
}

template <typename T>
inline const T &QVector<T>::at(int i) const
{
   Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::at", "index out of range");
   return d->begin()[i];
}

template <typename T>
inline const T &QVector<T>::operator[](int i) const
{
   Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::operator[]", "index out of range");
   return d->begin()[i];
}

template <typename T>
inline T &QVector<T>::operator[](int i)
{
   Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::operator[]", "index out of range");
   return data()[i];
}

template <typename T>
inline void QVector<T>::insert(int i, const T &t)
{
   Q_ASSERT_X(i >= 0 && i <= d->size, "QVector<T>::insert", "index out of range");
   insert(begin() + i, 1, t);
}

template <typename T>
inline void QVector<T>::insert(int i, int n, const T &t)
{
   Q_ASSERT_X(i >= 0 && i <= d->size, "QVector<T>::insert", "index out of range");
   insert(begin() + i, n, t);
}

template <typename T>
inline void QVector<T>::remove(int i, int n)
{
   Q_ASSERT_X(i >= 0 && n >= 0 && i + n <= d->size, "QVector<T>::remove", "index out of range");
   erase(d->begin() + i, d->begin() + i + n);
}

template <typename T>
inline void QVector<T>::remove(int i)
{
   Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::remove", "index out of range");
   erase(d->begin() + i, d->begin() + i + 1);
}

template <typename T>
inline void QVector<T>::prepend(const T &t)
{
   insert(begin(), 1, t);
}

template <typename T>
inline void QVector<T>::replace(int i, const T &t)
{
   Q_ASSERT_X(i >= 0 && i < d->size, "QVector<T>::replace", "index out of range");
   const T copy(t);
   data()[i] = copy;
}

template <typename T>
QVector<T> &QVector<T>::operator=(const QVector<T> &v)
{
   if (v.d != d) {
      QVector<T> tmp(v);
      tmp.swap(*this);
   }
   return *this;
}

template <typename T>
QVector<T>::QVector(int asize)
{
   if (Q_LIKELY(asize)) {
      d = Data::allocate(asize);
      d->size = asize;
      defaultConstruct(d->begin(), d->end());
   } else {
      d = Data::sharedNull();
   }
}

template <typename T>
QVector<T>::QVector(int asize, const T &t)
{
   if (asize) {
      d = Data::allocate(asize);
      d->size = asize;
      T *i = d->end();
      while (i != d->begin()) {
         new (--i) T(t);
      }
   } else {
      d = Data::sharedNull();
   }
}

template <typename T>
QVector<T>::QVector(std::initializer_list<T> args)
{
   d = Data::allocate(int(args.size()));
   // std::initializer_list<T>::iterator is guaranteed to be
   // const T* ([support.initlist]/1), so can be memcpy'ed away from by copyConstruct
   copyConstruct(args.begin(), args.end(), d->begin());
   d->size = int(args.size());
}

template <typename T>
void QVector<T>::free(Data *x)
{
   destruct(x->begin(), x->end());
   Data::deallocate(x);
}

template <typename T>
void QVector<T>::realloc(const int asize, const int aalloc, QArrayData::AllocationOptions options)
{
   Q_ASSERT(asize >= 0 && asize <= aalloc);
   Data *x = d;

   const bool isShared = d->ref.isShared();

#ifndef QT_NO_DEBUG
   bool moved = false;
   int oldSize = d->size;
#endif

   if (aalloc != 0) {
      if (aalloc != int(d->alloc) || isShared) {
         QT_TRY {
            // allocate memory
            x = Data::allocate(aalloc, options);
            Q_CHECK_PTR(x);

            // aalloc is bigger then 0 so it is not [un]sharedEmpty
            Q_ASSERT(x->ref.isSharable() || options.testFlag(QArrayData::Unsharable));
            Q_ASSERT(!x->ref.isStatic());
            x->size = asize;

            T *srcBegin = d->begin();
            T *srcEnd = asize > d->size ? d->end() : d->begin() + asize;
            T *dst = x->begin();

            if (QTypeInfo<T>::isStatic || (isShared && QTypeInfo<T>::isComplex))
            {
               // we can not move the data, we need to copy construct it
               while (srcBegin != srcEnd) {
                  new (dst++) T(*srcBegin++);
               }

            } else {
               ::memcpy(dst, srcBegin, (srcEnd - srcBegin) * sizeof(T));
               dst += srcEnd - srcBegin;

               // destruct unused / not moved data
               if (asize < d->size)
               {
                  destruct(d->begin() + asize, d->end());
               }
#ifndef QT_NO_DEBUG
               moved = true;
#endif
            }

            if (asize > d->size)
            {
               // construct all new objects when growing
               QT_TRY {
                  defaultConstruct(dst, x->end());
               } QT_CATCH (...) {
                  // destruct already copied objects
                  destruct(x->begin(), dst);
                  QT_RETHROW;
               }
            }
         } QT_CATCH (...) {
            Data::deallocate(x);
            QT_RETHROW;
         }
         x->capacityReserved = d->capacityReserved;
      } else {
         Q_ASSERT(d->alloc == aalloc); // resize, without changing allocation size
         Q_ASSERT(isDetached());       // can be done only on detached d
         Q_ASSERT(x == d);             // in this case we do not need to allocate anything
         if (asize <= d->size) {
            destruct(x->begin() + asize, x->end()); // from future end to current end
         } else {
            defaultConstruct(x->end(), x->begin() + asize); // from current end to future end
         }
         x->size = asize;
      }
   } else {
      x = Data::sharedNull();
   }
   if (d != x) {
      if (!d->ref.deref()) {
         Q_ASSERT(!isShared);
         Q_ASSERT(d->size == oldSize);
         if (QTypeInfo<T>::isStatic || !aalloc) {
            // data was copy constructed, we need to call destructors
            // or if !alloc we did nothing to the old 'd'.
            Q_ASSERT(!moved);
            free(d);
         } else {
            Data::deallocate(d);
         }
      }
      d = x;
   }

   Q_ASSERT(d->data());
   Q_ASSERT(d->size >= 0 && static_cast<uint>(d->size) <= d->alloc);
   Q_ASSERT(d != Data::unsharableEmpty());
   Q_ASSERT(aalloc ? d != Data::sharedNull() : d == Data::sharedNull());
   Q_ASSERT(aalloc >= 0 && d->alloc >= static_cast<uint>(aalloc));
   Q_ASSERT(d->size == asize);
}

template<typename T>
Q_OUTOFLINE_TEMPLATE T QVector<T>::value(int i) const
{
   if (i < 0 || i >= d->size) {
      return T();
   }
   return d->begin()[i];
}
template<typename T>
Q_OUTOFLINE_TEMPLATE T QVector<T>::value(int i, const T &defaultValue) const
{
   return ((i < 0 || i >= d->size) ? defaultValue : d->begin()[i]);
}

template <typename T>
void QVector<T>::append(const T &t)
{
   const T copy(t);
   const bool isTooSmall = uint(d->size + 1) > d->alloc;

   if (! isDetached() || isTooSmall) {
      QArrayData::AllocationOptions opt(isTooSmall ? QArrayData::Grow : QArrayData::Default);
      realloc(d->size, isTooSmall ? d->size + 1 : d->alloc, opt);
   }

   if (QTypeInfo<T>::isComplex) {
      new (d->end()) T(copy);
   } else {
      *d->end() = copy;
   }

   ++d->size;
}

template <typename T>
typename QVector<T>::iterator QVector<T>::insert(iterator before, size_type n, const T &t)
{
   int offset = int(before - d->begin());
   if (n != 0) {
      const T copy(t);
      if (!isDetached() || d->size + n > int(d->alloc)) {
         realloc(d->size, d->size + n, QArrayData::Grow);
      }
      if (QTypeInfo<T>::isStatic) {
         T *b = d->end();
         T *i = d->end() + n;
         while (i != b) {
            new (--i) T;
         }
         i = d->end();
         T *j = i + n;
         b = d->begin() + offset;
         while (i != b) {
            *--j = *--i;
         }
         i = b + n;
         while (i != b) {
            *--i = copy;
         }
      } else {
         T *b = d->begin() + offset;
         T *i = b + n;
         memmove(i, b, (d->size - offset) * sizeof(T));
         while (i != b) {
            new (--i) T(copy);
         }
      }
      d->size += n;
   }
   return d->begin() + offset;
}

template <typename T>
typename QVector<T>::iterator QVector<T>::erase(iterator abegin, iterator aend)
{
   abegin = qMax(abegin, d->begin());
   aend = qMin(aend, d->end());

   Q_ASSERT(abegin <= aend);

   const int itemsToErase = aend - abegin;
   const int itemsUntouched = abegin - d->begin();

   // FIXME we could do a proper realloc, which copy constructs only needed data.
   // FIXME we ara about to delete data maybe it is good time to shrink?
   if (d->alloc) {
      detach();
      abegin = d->begin() + itemsUntouched;
      aend = abegin + itemsToErase;
      if (QTypeInfo<T>::isStatic) {
         iterator moveBegin = abegin + itemsToErase;
         iterator moveEnd = d->end();
         while (moveBegin != moveEnd) {
            if (QTypeInfo<T>::isComplex) {
               abegin->~T();
            }
            new (abegin++) T(*moveBegin++);
         }
         if (abegin < d->end()) {
            // destroy rest of instances
            destruct(abegin, d->end());
         }
      } else {
         destruct(abegin, aend);
         memmove(abegin, aend, (d->size - itemsToErase - itemsUntouched) * sizeof(T));
      }
      d->size -= itemsToErase;
   }
   return d->begin() + itemsUntouched;
}

template <typename T>
bool QVector<T>::operator==(const QVector<T> &v) const
{
   if (d->size != v.d->size) {
      return false;
   }
   if (d == v.d) {
      return true;
   }
   T *b = d->begin();
   T *i = b + d->size;
   T *j = v.d->end();
   while (i != b)
      if (!(*--i == *--j)) {
         return false;
      }
   return true;
}

template <typename T>
QVector<T> &QVector<T>::fill(const T &from, int asize)
{
   const T copy(from);
   resize(asize < 0 ? d->size : asize);
   if (d->size) {
      T *i = d->end();
      T *b = d->begin();
      while (i != b) {
         *--i = copy;
      }
   }
   return *this;
}

template <typename T>
QVector<T> &QVector<T>::operator+=(const QVector &l)
{
   int newSize = d->size + l.d->size;
   realloc(d->size, newSize);

   if (d->alloc) {
      T *w = d->begin() + newSize;
      T *i = l.d->end();
      T *b = l.d->begin();
      while (i != b) {
         if (QTypeInfo<T>::isComplex) {
            new (--w) T(*--i);
         } else {
            *--w = *--i;
         }
      }
      d->size = newSize;
   }
   return *this;
}

template <typename T>
int QVector<T>::indexOf(const T &t, int from) const
{
   if (from < 0) {
      from = qMax(from + d->size, 0);
   }
   if (from < d->size) {
      T *n = d->begin() + from - 1;
      T *e = d->end();
      while (++n != e)
         if (*n == t) {
            return n - d->begin();
         }
   }
   return -1;
}

template <typename T>
int QVector<T>::lastIndexOf(const T &t, int from) const
{
   if (from < 0) {
      from += d->size;
   } else if (from >= d->size) {
      from = d->size - 1;
   }
   if (from >= 0) {
      T *b = d->begin();
      T *n = d->begin() + from + 1;
      while (n != b) {
         if (*--n == t) {
            return n - b;
         }
      }
   }
   return -1;
}

template <typename T>
bool QVector<T>::contains(const T &t) const
{
   T *b = d->begin();
   T *i = d->end();
   while (i != b)
      if (*--i == t) {
         return true;
      }
   return false;
}

template <typename T>
int QVector<T>::count(const T &t) const
{
   int c = 0;
   T *b = d->begin();
   T *i = d->end();
   while (i != b)
      if (*--i == t) {
         ++c;
      }
   return c;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QVector<T> QVector<T>::mid(int pos, int length) const
{
   if (length < 0) {
      length = size() - pos;
   }
   if (pos == 0 && length == size()) {
      return *this;
   }
   if (pos + length > size()) {
      length = size() - pos;
   }
   QVector<T> copy;
   copy.reserve(length);
   for (int i = pos; i < pos + length; ++i) {
      copy += at(i);
   }
   return copy;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QList<T> QVector<T>::toList() const
{
   QList<T> result;
   result.reserve(size());
   for (int i = 0; i < size(); ++i) {
      result.append(at(i));
   }
   return result;
}

template <typename T>
Q_OUTOFLINE_TEMPLATE QVector<T> QList<T>::toVector() const
{
   QVector<T> result(size());
   for (int i = 0; i < size(); ++i) {
      result[i] = at(i);
   }
   return result;
}

template <typename T>
QVector<T> QVector<T>::fromList(const QList<T> &list)
{
   return list.toVector();
}

template <typename T>
QList<T> QList<T>::fromVector(const QVector<T> &vector)
{
   return vector.toList();
}

template <class T>
class QVectorIterator
{ 
   typedef typename QVector<T>::const_iterator const_iterator;
   QVector<T> c;
   const_iterator i;
   
   public:
      inline QVectorIterator(const QVector<T> &container)
         : c(container), i(c.constBegin()) {}
   
      inline QVectorIterator &operator=(const QVector<T> &container)
         { c = container; i = c.constBegin(); return *this; }
      
      inline void toFront() { i = c.constBegin(); } 
      inline void toBack() { i = c.constEnd(); } 
      inline bool hasNext() const { return i != c.constEnd(); } 
      inline const T &next() { return *i++; } 
      inline const T &peekNext() const { return *i; } 
      inline bool hasPrevious() const { return i != c.constBegin(); } 
      inline const T &previous() { return *--i; } 
      inline const T &peekPrevious() const { const_iterator p = i; return *--p; } 
      
      inline bool findNext(const T &t)  { 
         while (i != c.constEnd()) {
            if (*i++ == t) {
               return true; 
            }
         }
         return false;           
      }
      
      inline bool findPrevious(const T &t)   { 
         while (i != c.constBegin()) {
            if (*(--i) == t)  {
               return true; 
            }
         }  
         return false;                 
      }
};

template <class T>
class QMutableVectorIterator
{ 
   typedef typename QVector<T>::iterator iterator;
   typedef typename QVector<T>::const_iterator const_iterator;
   QVector<T> *c;
   iterator i, n;
   inline bool item_exists() const { return const_iterator(n) != c->constEnd(); } 
   
   public:
      inline QMutableVectorIterator(QVector<T> &container)
         : c(&container)
      { c->setSharable(false); i = c->begin(); n = c->end(); } 
      
      inline ~QMutableVectorIterator()
         { c->setSharable(true); }

      inline QMutableVectorIterator &operator=(QVector<T> &container)
         { c->setSharable(true); c = &container; c->setSharable(false);

      i = c->begin(); n = c->end(); return *this; }
      inline void toFront() { i = c->begin(); n = c->end(); }
      inline void toBack() { i = c->end(); n = i; }
      inline bool hasNext() const { return c->constEnd() != const_iterator(i); }
      inline T &next() { n = i++; return *n; }
      inline T &peekNext() const { return *i; }
      inline bool hasPrevious() const { return c->constBegin() != const_iterator(i); }
      inline T &previous() { n = --i; return *n; }
      inline T &peekPrevious() const { iterator p = i; return *--p; }

      inline void remove()
         { if (c->constEnd() != const_iterator(n)) { i = c->erase(n); n = c->end(); } }

      inline void setValue(const T &t) const { if (c->constEnd() != const_iterator(n)) *n = t; }
      inline T &value() { Q_ASSERT(item_exists()); return *n; }
      inline const T &value() const { Q_ASSERT(item_exists()); return *n; }
      inline void insert(const T &t) { n = i = c->insert(i, t); ++i; } 

      inline bool findNext(const T &t) 
         { while (c->constEnd() != const_iterator(n = i)) if (*i++ == t) return true; return false; } 

      inline bool findPrevious(const T &t) 
         { while (c->constBegin() != const_iterator(i)) if (*(n = --i) == t) return true; 

      n = c->end(); return false;  } 
};

QT_END_NAMESPACE

#endif // QVECTOR_H
