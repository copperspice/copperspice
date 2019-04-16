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

#ifndef QVARLENGTHARRAY_H
#define QVARLENGTHARRAY_H

#include <qcontainerfwd.h>
#include <qglobal.h>
#include <qassert.h>

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <new>
#include <stdlib.h>
#include <string.h>

template<class T, int Prealloc>
class QPodList;

// Prealloc = 256 by default, specified in qcontainerfwd.h
template<class T, int Prealloc>
class QVarLengthArray
{
 public:
   typedef int size_type;
   typedef T value_type;
   typedef value_type *pointer;
   typedef const value_type *const_pointer;
   typedef value_type &reference;
   typedef const value_type &const_reference;
   typedef qptrdiff difference_type;

   typedef T *iterator;
   typedef const T *const_iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;
   typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

   inline explicit QVarLengthArray(int size = 0);

   QVarLengthArray(const QVarLengthArray<T, Prealloc> &other)
      : a(Prealloc), s(0), ptr(reinterpret_cast<T *>(array)) {
      append(other.constData(), other.size());
   }

   QVarLengthArray(std::initializer_list<T> args)
        : a(Prealloc), s(0), ptr(reinterpret_cast<T *>(array))
   {
      if (args.size()) {
         append(args.begin(), args.size());
      }
   }

   ~QVarLengthArray() {
      if (QTypeInfo<T>::isComplex) {
         T *i = ptr + s;
         while (i-- != ptr) {
            i->~T();
         }
      }

      if (ptr != reinterpret_cast<T *>(array)) {
         free(ptr);
      }
   }

   // methods
   void append(const T *buf, int size);

   void append(const T &t) {
      if (s == a) {
         // for example, s != 0
         realloc(s, s << 1);
      }
      const int idx = s++;

      if (QTypeInfo<T>::isComplex) {
         new (ptr + idx) T(t);
      } else {
         ptr[idx] = t;
      }
   }

   const T &at(int idx) const {
      return operator[](idx);
   }

   T &back() {
      return last();
   }

   const T &back() const {
      return last();
   }

   int capacity() const {
      return a;
   }

   bool contains(const T &value) const {
      for (const auto &item : *this) {
         if (item == value) {
            return true;
         }
      }

      return false;
   }

   void clear() {
      resize(0);
   }

   int count() const {
      return s;
   }

   T *data() {
      return ptr;
   }

   const T *data() const {
      return ptr;
   }

   const T *constData() const {
      return ptr;
   }

   bool empty() const {
      return isEmpty();
   }

   iterator erase(const_iterator begin, const_iterator end);

   iterator erase(const_iterator pos) {
      return erase(pos, pos + 1);
   }

   T &first() {
      Q_ASSERT(!isEmpty());
      return *begin();
   }

   const T &first() const {
      Q_ASSERT(!isEmpty());
      return *begin();
   }

   T &front() {
      return first();
   }

   const T &front() const {
      return first();
   }

   T& last() {
      Q_ASSERT(! isEmpty());
      return *(end() - 1);
   }

   const T& last() const {
      Q_ASSERT(!isEmpty());
      return *(end() - 1);
   }

   int indexOf(const T &t, int from = 0) const;
   int lastIndexOf(const T &t, int from = -1) const;

   void insert(int i, const T &t);
   void insert(int i, int n, const T &t);

   iterator insert(const_iterator before, int n, const T &x);

   iterator insert(const_iterator before, const T &x) {
      return insert(before, 1, x);
   }

   bool isEmpty() const {
      return (s == 0);
   }

   int length() const {
      return s;
   }

   void prepend(const T &t);

   void push_back(const T &t) {
      append(t);
   }

   void pop_back() {
      removeLast();
   }

   void replace(int i, const T &t);
   void remove(int i);
   void remove(int i, int n);
   void removeLast() {
      Q_ASSERT(s > 0);
      realloc(s - 1, a);
   }

   void resize(int size);
   void reserve(int size);

   void squeeze();
   int size() const {
      return s;
   }

   T value(int i) const;
   T value(int i, const T &defaultValue) const;

   // iterators
   iterator begin() {
      return ptr;
   }

   const_iterator begin() const {
      return ptr;
   }

   const_iterator cbegin() const {
      return ptr;
   }

   const_iterator constBegin() const {
      return ptr;
   }

   iterator end() {
      return ptr + s;
   }

   const_iterator end() const {
      return ptr + s;
   }

   const_iterator cend() const {
      return ptr + s;
   }

   const_iterator constEnd() const {
      return ptr + s;
   }

   reverse_iterator rbegin() {
      return reverse_iterator(end());
   }

   reverse_iterator rend() {
      return reverse_iterator(begin());
   }

   const_reverse_iterator rbegin() const {
      return const_reverse_iterator(end());
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
   QVarLengthArray<T, Prealloc> &operator=(const QVarLengthArray<T, Prealloc> &other) {
      if (this != &other) {
         clear();
         append(other.constData(), other.size());
      }
      return *this;
   }

   QVarLengthArray<T, Prealloc> &operator=(std::initializer_list<T> list)
   {
      resize(list.size());
      std::copy(list.begin(), list.end(), this->begin());

      return *this;
   }

   // operators
   T &operator[](int idx) {
      Q_ASSERT(idx >= 0 && idx < s);
      return ptr[idx];
   }

   const T &operator[](int idx) const {
      Q_ASSERT(idx >= 0 && idx < s);
      return ptr[idx];
   }

   QVarLengthArray<T, Prealloc> &operator<<(const T &t) {
      append(t);
      return *this;
   }

   QVarLengthArray<T, Prealloc> &operator+=(const T &t) {
      append(t);
      return *this;
   }

 private:
   friend class QPodList<T, Prealloc>;
   void realloc(int size, int alloc);

   int a;      // capacity
   int s;      // size
   T *ptr;     // data

   union {
      char array[Prealloc * sizeof(T)];
      qint64 q_for_alignment_1;
      double q_for_alignment_2;
   };
};

template <class T, int Prealloc>
QVarLengthArray<T, Prealloc>::QVarLengthArray(int asize)
   : s(asize)
{
   if (s > Prealloc) {
      ptr = reinterpret_cast<T *>(malloc(s * sizeof(T)));
      Q_CHECK_PTR(ptr);
      a = s;
   } else {
      ptr = reinterpret_cast<T *>(array);
      a = Prealloc;
   }

   if (QTypeInfo<T>::isComplex) {
      T *i = ptr + s;
      while (i != ptr) {
         new (--i) T;
      }
   }
}

template <class T, int Prealloc>
int QVarLengthArray<T, Prealloc>::indexOf(const T &t, int from) const
{
    if (from < 0)
        from = qMax(from + s, 0);
    if (from < s) {
        T *n = ptr + from - 1;
        T *e = ptr + s;
        while (++n != e)
            if (*n == t)
                return n - ptr;
    }
    return -1;
}

template <class T, int Prealloc>
int QVarLengthArray<T, Prealloc>::lastIndexOf(const T &t, int from) const
{
    if (from < 0)
        from += s;
    else if (from >= s)
        from = s - 1;
    if (from >= 0) {
        T *b = ptr;
        T *n = ptr + from + 1;
        while (n != b) {
            if (*--n == t)
                return n - b;
        }
    }
    return -1;
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::append(const T *abuf, int increment)
{
   Q_ASSERT(abuf);
   if (increment <= 0) {
      return;
   }

   const int asize = s + increment;

   if (asize >= a) {
      realloc(s, qMax(s * 2, asize));
   }

   if (QTypeInfo<T>::isComplex) {
      // call constructor for new objects (which can throw)
      while (s < asize) {
         new (ptr + (s++)) T(*abuf++);
      }

   } else {
      memcpy(&ptr[s], abuf, increment * sizeof(T));
      s = asize;
   }
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::resize(int asize)
{
   realloc(asize, qMax(asize, a));
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::reserve(int asize)
{
   if (asize > a) {
      realloc(s, asize);
   }
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::realloc(int asize, int aalloc)
{
   Q_ASSERT(aalloc >= asize);

   T *oldPtr = ptr;
   int osize = s;

   const int copySize = qMin(asize, osize);
   if (aalloc != a) {
      ptr = reinterpret_cast<T *>(malloc(aalloc * sizeof(T)));
      Q_CHECK_PTR(ptr);

      if (ptr) {
         s = 0;
         a = aalloc;

         if (QTypeInfo<T>::isStatic) {
            try {
               // copy all the old elements
               while (s < copySize) {
                  new (ptr + s) T(*(oldPtr + s));
                  (oldPtr + s)->~T();
                  s++;
               }

            } catch(...) {
               // clean up all the old objects and then free the old ptr
               int sClean = s;

               while (sClean < osize) {
                  (oldPtr + (sClean++))->~T();
               }

               if (oldPtr != reinterpret_cast<T *>(array) && oldPtr != ptr) {
                  free(oldPtr);
               }

               throw;
            }

         } else {
            memcpy(ptr, oldPtr, copySize * sizeof(T));
         }

      } else {
         ptr = oldPtr;
         return;
      }
   }
   s = copySize;

   if (QTypeInfo<T>::isComplex) {
      // destroy remaining old objects
      while (osize > asize) {
         (oldPtr + (--osize))->~T();
      }
   }

   if (oldPtr != reinterpret_cast<T *>(array) && oldPtr != ptr) {
      free(oldPtr);
   }

   if (QTypeInfo<T>::isComplex) {
      // call default constructor for new objects (which can throw)
      while (s < asize) {
         new (ptr + (s++)) T;
      }
   } else {
      s = asize;
   }
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::squeeze()
{
   realloc(s, s);
}

template <class T, int Prealloc>
T QVarLengthArray<T, Prealloc>::value(int i) const
{
   if (i < 0 || i >= size()) {
      return T();
   }
   return at(i);
}

template <class T, int Prealloc>
T QVarLengthArray<T, Prealloc>::value(int i, const T &defaultValue) const
{
   return (i < 0 || i >= size()) ? defaultValue : at(i);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::insert(int i, const T &t)
{
   Q_ASSERT_X(i >= 0 && i <= s, "QVarLengthArray::insert", "index out of range");
   insert(begin() + i, 1, t);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::insert(int i, int n, const T &t)
{
   Q_ASSERT_X(i >= 0 && i <= s, "QVarLengthArray::insert", "index out of range");
   insert(begin() + i, n, t);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::remove(int i, int n)
{
   Q_ASSERT_X(i >= 0 && n >= 0 && i + n <= s, "QVarLengthArray::remove", "index out of range");
   erase(begin() + i, begin() + i + n);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::remove(int i)
{
   Q_ASSERT_X(i >= 0 && i < s, "QVarLengthArray::remove", "index out of range");
   erase(begin() + i, begin() + i + 1);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::prepend(const T &t)
{
   insert(begin(), 1, t);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::replace(int i, const T &t)
{
   Q_ASSERT_X(i >= 0 && i < s, "QVarLengthArray::replace", "index out of range");
   const T copy(t);
   data()[i] = copy;
}

template <class T, int Prealloc>
typename QVarLengthArray<T, Prealloc>::iterator QVarLengthArray<T, Prealloc>::insert(const_iterator before, size_type n, const T &t)
{
   int offset = int(before - ptr);
   if (n != 0) {
      resize(s + n);
      const T copy(t);

      if (QTypeInfo<T>::isStatic) {
         T *b = ptr + offset;
         T *j = ptr + s;
         T *i = j - n;

         while (i != b) {
            *--j = *--i;
         }

         i = b + n;

         while (i != b) {
            *--i = copy;
         }

      } else {
         T *b = ptr + offset;
         T *i = b + n;
         memmove(i, b, (s - offset - n) * sizeof(T));
         while (i != b) {
            new (--i) T(copy);
         }
      }
   }
   return ptr + offset;
}

template <class T, int Prealloc>
typename QVarLengthArray<T, Prealloc>::iterator QVarLengthArray<T, Prealloc>::erase(const_iterator abegin, const_iterator aend)
{
   int f = int(abegin - ptr);
   int l = int(aend - ptr);
   int n = l - f;

   if (QTypeInfo<T>::isComplex) {
      std::copy(ptr + l, ptr + s, ptr + f);

      T *i = ptr + s;
      T *b = ptr + s - n;

      while (i != b) {
         --i;
         i->~T();
      }

   } else {
      memmove(ptr + f, ptr + l, (s - l) * sizeof(T));
   }

   s -= n;

   return ptr + f;
}

template <typename T, int Prealloc1, int Prealloc2>
bool operator==(const QVarLengthArray<T, Prealloc1> &l, const QVarLengthArray<T, Prealloc2> &r)
{
   if (l.size() != r.size()) {
      return false;
   }

   for (int i = 0; i < l.size(); i++) {
      if (l.at(i) != r.at(i)) {
         return false;
      }
   }

   return true;
}


template <typename T, int Prealloc1, int Prealloc2>
bool operator!=(const QVarLengthArray<T, Prealloc1> &l, const QVarLengthArray<T, Prealloc2> &r)
{
   return !(l == r);
}

#endif