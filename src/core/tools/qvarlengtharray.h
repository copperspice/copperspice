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

#ifndef QVARLENGTHARRAY_H
#define QVARLENGTHARRAY_H

#include <qassert.h>
#include <qcontainerfwd.h>
#include <qglobal.h>

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <new>
#include <stdlib.h>
#include <string.h>

template <class T, int Prealloc>
class QPodList;

// Prealloc = 256 by default, specified in qcontainerfwd.h
template <class T, int Prealloc>
class QVarLengthArray
{
 public:
   using size_type        = int;
   using value_type       = T;
   using pointer          = value_type *;
   using const_pointer    = const value_type *;
   using reference        = value_type &;
   using const_reference  = const value_type &;
   using difference_type  = qptrdiff;
   using iterator         = T *;
   using const_iterator   = const T *;

   using reverse_iterator       = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   inline explicit QVarLengthArray(int size = 0);

   QVarLengthArray(const QVarLengthArray<T, Prealloc> &other)
      : a(Prealloc), s(0), ptr(reinterpret_cast<T *>(array))
   {
      append(other.constData(), other.size());
   }

   QVarLengthArray(std::initializer_list<T> args)
      : a(Prealloc), s(0), ptr(reinterpret_cast<T *>(array))
   {
      if (args.size()) {
         append(args.begin(), args.size());
      }
   }

   ~QVarLengthArray()
   {
      if constexpr (! std::is_trivially_destructible_v<T>) {
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
   void append(const T *buffer, int length);

   void append(const T &value) {
      if (s == a) {
         realloc(s, s << 1);
      }

      const int index = s++;

      if constexpr (std::is_trivially_copy_assignable_v<T>) {
         ptr[index] = value;
      } else {
         new (ptr + index) T(value);
      }
   }

   const T &at(int index) const {
      return operator[](index);
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

   T &last() {
      Q_ASSERT(! isEmpty());
      return *(end() - 1);
   }

   const T &last() const {
      Q_ASSERT(!isEmpty());
      return *(end() - 1);
   }

   int indexOf(const T &value, int from = 0) const;
   int lastIndexOf(const T &value, int from = -1) const;

   void insert(int index, const T &value);
   void insert(int index, int count, const T &value);

   iterator insert(const_iterator before, int count, const T &value);

   iterator insert(const_iterator before, const T &value) {
      return insert(before, 1, value);
   }

   bool isEmpty() const {
      return (s == 0);
   }

   int length() const {
      return s;
   }

   void prepend(const T &value);

   void push_back(const T &value) {
      append(value);
   }

   void pop_back() {
      removeLast();
   }

   void replace(int index, const T &value);
   void remove(int index);
   void remove(int index, int count);

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

   T value(int index) const;
   T value(int index, const T &defaultValue) const;

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

   QVarLengthArray<T, Prealloc> &operator=(std::initializer_list<T> list) {
      resize(list.size());
      std::copy(list.begin(), list.end(), this->begin());

      return *this;
   }

   // operators
   T &operator[](int index) {
      Q_ASSERT(index >= 0 && index < s);
      return ptr[index];
   }

   const T &operator[](int index) const {
      Q_ASSERT(index >= 0 && index < s);
      return ptr[index];
   }

   QVarLengthArray<T, Prealloc> &operator<<(const T &value) {
      append(value);
      return *this;
   }

   QVarLengthArray<T, Prealloc> &operator+=(const T &value) {
      append(value);
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
QVarLengthArray<T, Prealloc>::QVarLengthArray(int size)
   : s(size)
{
   if (s > Prealloc) {
      ptr = reinterpret_cast<T *>(malloc(s * sizeof(T)));
      Q_CHECK_PTR(ptr);
      a = s;

   } else {
      ptr = reinterpret_cast<T *>(array);
      a = Prealloc;
   }

   if constexpr (! std::is_trivially_constructible_v<T>) {
      T *i = ptr + s;

      while (i != ptr) {
         new (--i) T;
      }
   }
}

template <class T, int Prealloc>
int QVarLengthArray<T, Prealloc>::indexOf(const T &value, int from) const
{
   if (from < 0) {
      from = qMax(from + s, 0);
   }

   if (from < s) {
      T *n = ptr + from - 1;
      T *e = ptr + s;

      while (++n != e) {
         if (*n == value) {
            return n - ptr;
         }
      }
   }

   return -1;
}

template <class T, int Prealloc>
int QVarLengthArray<T, Prealloc>::lastIndexOf(const T &value, int from) const
{
   if (from < 0) {
      from += s;

   } else if (from >= s) {
      from = s - 1;
   }

   if (from >= 0) {
      T *b = ptr;
      T *n = ptr + from + 1;

      while (n != b) {
         if (*--n == value) {
            return n - b;
         }
      }
   }

   return -1;
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::append(const T *buffer, int length)
{
   Q_ASSERT(buffer);

   if (length <= 0) {
      return;
   }

   const int asize = s + length;

   if (asize >= a) {
      realloc(s, qMax(s * 2, asize));
   }

   if constexpr (std::is_trivially_copy_constructible_v<T>) {
      memcpy(&ptr[s], buffer, length * sizeof(T));
      s = asize;

   } else {
      // call constructor for new objects (which can throw)

      while (s < asize) {
         new (ptr + (s++)) T(*buffer++);
      }
   }
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::resize(int size)
{
   realloc(size, qMax(size, a));
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::reserve(int size)
{
   if (size > a) {
      realloc(s, size);
   }
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::realloc(int size, int allocSize)
{
   Q_ASSERT(allocSize >= size);

   T *oldPtr = ptr;
   int osize = s;

   const int copySize = qMin(size, osize);

   if (allocSize != a) {
      ptr = reinterpret_cast<T *>(malloc(allocSize * sizeof(T)));
      Q_CHECK_PTR(ptr);

      if (ptr) {
         s = 0;
         a = allocSize;

         if constexpr (std::is_trivially_copy_constructible_v<T>) {
            memcpy(ptr, oldPtr, copySize * sizeof(T));

         } else {
            try {
               // copy all the old elements
               while (s < copySize) {
                  new (ptr + s) T(*(oldPtr + s));
                  (oldPtr + s)->~T();
                  s++;
               }

            } catch (...) {
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
         }

      } else {
         ptr = oldPtr;
         return;
      }
   }

   s = copySize;

   if constexpr (! std::is_trivially_destructible_v<T>) {
      // destroy remaining old objects
      while (osize > size) {
         (oldPtr + (--osize))->~T();
      }
   }

   if (oldPtr != reinterpret_cast<T *>(array) && oldPtr != ptr) {
      free(oldPtr);
   }

   if constexpr (std::is_trivially_constructible_v<T>) {
      s = size;

   } else {
      // call default constructor for new objects (which can throw)
      while (s < size) {
         new (ptr + (s++)) T;
      }
   }
}

template <class T, int Prealloc>
void QVarLengthArray<T, Prealloc>::squeeze()
{
   realloc(s, s);
}

template <class T, int Prealloc>
T QVarLengthArray<T, Prealloc>::value(int index) const
{
   if (index < 0 || index >= size()) {
      return T();
   }

   return at(index);
}

template <class T, int Prealloc>
T QVarLengthArray<T, Prealloc>::value(int index, const T &defaultValue) const
{
   return (index < 0 || index >= size()) ? defaultValue : at(index);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::insert(int index, const T &value)
{
   Q_ASSERT_X(index >= 0 && index <= s, "QVarLengthArray::insert", "index out of range");
   insert(begin() + index, 1, value);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::insert(int index, int count, const T &value)
{
   Q_ASSERT_X(index >= 0 && index <= s, "QVarLengthArray::insert", "index out of range");
   insert(begin() + index, count, value);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::remove(int index, int count)
{
   Q_ASSERT_X(index >= 0 && count >= 0 && index + count <= s, "QVarLengthArray::remove", "index out of range");
   erase(begin() + index, begin() + index + count);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::remove(int index)
{
   Q_ASSERT_X(index >= 0 && index < s, "QVarLengthArray::remove", "index out of range");
   erase(begin() + index, begin() + index + 1);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::prepend(const T &value)
{
   insert(begin(), 1, value);
}

template <class T, int Prealloc>
inline void QVarLengthArray<T, Prealloc>::replace(int index, const T &value)
{
   Q_ASSERT_X(index >= 0 && index < s, "QVarLengthArray::replace", "index out of range");

   const T copy(value);
   data()[index] = copy;
}

template <class T, int Prealloc>
typename QVarLengthArray<T, Prealloc>::iterator QVarLengthArray<T, Prealloc>::insert(const_iterator before,
      size_type count, const T &value)
{
   int offset = int(before - ptr);

   if (count != 0) {
      resize(s + count);
      const T copy(value);

      if constexpr (std::is_trivially_constructible_v<T> && std::is_trivially_destructible_v<T> &&
            std::is_trivially_copyable_v<T> ) {

         T *b = ptr + offset;
         T *i = b + count;
         memmove(i, b, (s - offset - count) * sizeof(T));

         while (i != b) {
            new (--i) T(copy);
         }

      } else {

         T *b = ptr + offset;
         T *j = ptr + s;
         T *i = j - count;

         while (i != b) {
            *--j = *--i;
         }

         i = b + count;

         while (i != b) {
            *--i = copy;
         }
      }
   }

   return ptr + offset;
}

template <class T, int Prealloc>
typename QVarLengthArray<T, Prealloc>::iterator QVarLengthArray<T, Prealloc>::erase(const_iterator begin,
      const_iterator end)
{
   int f = int(begin - ptr);
   int l = int(end - ptr);
   int n = l - f;

   if constexpr (std::is_trivially_constructible_v<T> && std::is_trivially_destructible_v<T> &&
         std::is_trivially_copyable_v<T> ) {

      memmove(ptr + f, ptr + l, (s - l) * sizeof(T));

   } else {

      std::copy(ptr + l, ptr + s, ptr + f);

      T *i = ptr + s;
      T *b = ptr + s - n;

      while (i != b) {
         --i;
         i->~T();
      }
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