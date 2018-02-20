/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QLIST_H
#define QLIST_H

#include <algorithm>
#include <deque>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <list>
#include <string>
#include <stdexcept>

#include <limits.h>
#include <string.h>

#include <qassert.h>
#include <qglobal.h>

template <typename T>
class QSet;

template <typename T>
class QVector;

template <class T>
class QListIterator;

template <class T>
class QMutableListIterator;

template <typename T>
class QList
{

 public:
   using iterator        = typename std::deque<T>::iterator;
   using const_iterator  = typename std::deque<T>::const_iterator;

   // legacy
   using Iterator        = iterator;
   using ConstIterator   = const_iterator;

   using const_pointer   = typename std::deque<T>::const_pointer;
   using const_reference = typename std::deque<T>::const_reference;
   using difference_type = typename std::deque<T>::difference_type;
   using pointer         = typename std::deque<T>::pointer;
   using reference       = typename std::deque<T>::reference;
   using size_type       = typename std::deque<T>::difference_type;      // makes this signed instead of unsigned
   using value_type      = typename std::deque<T>::value_type;

   // from std
   using allocator_type         = typename std::deque<T>::allocator_type;
   using reverse_iterator       = typename std::deque<T>::reverse_iterator;
   using const_reverse_iterator = typename std::deque<T>::const_reverse_iterator;

   // java
   using Java_Iterator          = QListIterator<T>;
   using Java_MutableIterator   = QMutableListIterator<T>;

   QList() = default;
   QList(const QList<T> &other) = default;
   QList(QList<T> &&other)      = default;

   QList(std::initializer_list<T> args)
      :  m_data(args) { }

   template<class Input_Iterator>
   QList(Input_Iterator first, Input_Iterator last)
      : m_data(first, last) {}

   ~QList() = default;

   // methods
   void append(const T &value) {
      m_data.push_back(value);
   }

   void append(const QList<T> &other) {
      if (this != &other) {
        m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
        return;
      }

      auto numElements = this->size();
      for (QList<T>::size_type i = 0; i < numElements; ++i) {
          m_data.push_back(m_data[i]);
      }
   }

   const T &at(size_type i) const;

   T &back() {
      return m_data.back();
   }

   const T &back() const {
      return m_data.back();
   }

   void clear(){
      return m_data.clear();
   }

   bool contains(const T &value) const;

   size_type count(const T &value) const;

   size_type count() const {
      return size();
   }

   bool empty() const {
      return m_data.empty();
   }

   bool isEmpty() const {
      return m_data.empty();
   }

   bool endsWith(const T &value) const {
      return ! isEmpty() && m_data.back() == value;
   }

   T &first() {
      Q_ASSERT(! isEmpty());
      return m_data.front();
   }

   const T &first() const {
      Q_ASSERT(! isEmpty());
      return m_data.front();
   }

   const_reference constFirst() const {
      Q_ASSERT(! isEmpty());
      return m_data.front();
   }

   T &front() {
      return m_data.front();
   }

   const T &front() const {
      return m_data.front();
   }

   size_type indexOf(const T &value, size_type from = 0) const {
      size_type retval = -1;

      auto iter = std::find(m_data.begin() + from, m_data.end(), value);

      if (iter != m_data.end()) {
         retval = iter - m_data.begin();
      }

      return retval;
   }

   T &last() {
      Q_ASSERT(! isEmpty());
      return m_data.back();
   }

   const T &last() const {
      Q_ASSERT(! isEmpty());
      return m_data.back();
   }

   const_reference constLast() const {
      Q_ASSERT(! isEmpty());
      return m_data.back();
   }

   void insert(size_type i, const T &value);

   size_type length() const {
     return size();
   }

   size_type lastIndexOf(const T &value, size_type from = -1) const {
      size_type retval = -1;
      size_type from_reverse = 0;

      if (from >= 0)  {
         from_reverse = size() - from;
      }

      auto iter = std::find(m_data.rbegin() + from_reverse, m_data.rend(), value);

      if (iter != m_data.rend()) {
         retval = m_data.rend() - iter - 1;
      }

      return retval;
   }

   QList<T> mid(size_type pos, size_type length = -1) const;
   void move(size_type from, size_type to);

   void pop_back() {
      removeLast();
   }

   void pop_front() {
      removeFirst();
   }

   void prepend(const T &value) {
      m_data.push_front(value);
   }

   void push_back(const T &value) {
      m_data.push_back(value);
   }

   void push_front(const T &value) {
      m_data.push_front(value);
   }

   size_type removeAll(const T &value);

   void removeAt(size_type i) {
      Q_ASSERT_X(i >= 0 && i < size(), "QList<T>::removeAt", "index out of range");
      m_data.erase(m_data.begin() + i);
   }

   void removeFirst() {
      Q_ASSERT(!isEmpty());
      m_data.pop_front();
   }

   void removeLast() {
     Q_ASSERT(! isEmpty());
     m_data.pop_back();
   }

   bool removeOne(const T &value);
   void replace(size_type i, const T &value);

   void reserve(size_type size) {
      // this method should do nothing
   }

   size_type size() const {
      // returns unsigned, must convert to signed
      return static_cast<size_type>(m_data.size());
   }

   bool startsWith(const T &value) const {
      return ! isEmpty() && m_data.front() == value;
   }

   void swap(QList<T> &other) {
      qSwap(m_data, other.m_data);
   }

   void swap(size_type i, size_type j);

   T takeAt(size_type i);
   T takeFirst();
   T takeLast();

   T value(size_type i) const;
   T value(size_type i, const T &defaultValue) const;

   // to from
   static QList<T> fromSet(const QSet<T> &set);
   static QList<T> fromVector(const QVector<T> &vector);

   static QList<T> fromStdList(const std::list<T> &list) {
      QList<T> tmp;
      std::copy(list.begin(), list.end(), std::back_inserter(tmp));
      return tmp;
   }

   QSet<T> toSet() const;
   QVector<T> toVector() const;

   std::list<T> toStdList() const {
      std::list<T> tmp(m_data.begin(), m_data.end());
      return tmp;
   }

   // iterators
   iterator begin() {
      return m_data.begin();
   }

   const_iterator begin() const {
      return m_data.begin();
   }

   const_iterator constBegin() const {
      return m_data.begin();
   }

   const_iterator cbegin() const {
      return m_data.begin();
   }

   iterator end() {
      return m_data.end();
   }

   const_iterator end() const {
      return m_data.end();
   }

   const_iterator constEnd() const {
      return m_data.end();
   }

   const_iterator cend() const {
      return m_data.end();
   }

   // reverse iterators
   reverse_iterator rbegin() {
      return m_data.rbegin();
   }

   const_reverse_iterator rbegin() const {
      return m_data.rbegin();
   }

   const_reverse_iterator crbegin() const {
      return m_data.rbegin();
   }

   reverse_iterator rend() {
      return m_data.rend();
   }

   const_reverse_iterator rend() const {
      return m_data.rend();
   }

   const_reverse_iterator crend() const {
      return m_data.rend();
   }

   iterator erase(iterator begin, iterator end) {
      return m_data.erase(begin, end);
   }

   iterator erase(iterator pos) {
      return m_data.erase(pos);
   }

   iterator insert(iterator before, const T &value) {
      return m_data.insert(before, value);
   }

   // operators
   QList<T> &operator=(const QList<T> &other) = default;
   QList<T> &operator=(QList<T> && other) = default;

   bool operator==(const QList<T> &other) const {
      return (m_data == other.m_data);
   }

   bool operator!=(const QList<T> &other) const {
      return (m_data != other.m_data);
   }

   QList<T> operator+(const QList<T> &other) const {
      QList n = *this;
      n += other;
      return n;
   }

   QList<T> &operator+=(const QList<T> &other) {
      append(other);
      return *this;
   }

   QList<T> &operator+=(const T &value) {
      append(value);
      return *this;
   }

   QList<T> &operator<< (const T &value) {
      append(value);
      return *this;
   }

   QList<T> &operator<<(const QList<T> &other) {
      *this += other;
      return *this;
   }

   const T &operator[](size_type i) const;
   T &operator[](size_type i);

   private:
      std::deque<T> m_data;
};

// methods
template <typename T>
inline const T &QList<T>::at(size_type i) const
{
   if (i < 0 || i >= size()) {

      std::string msg = "QList<T>::at() Index is out of Range. (Index is " + std::to_string(i) +
                  ", Size is " + std::to_string(size()) + ")";
      throw std::logic_error(msg);
   }

   return m_data[i];
}

template <typename T>
bool QList<T>::contains(const T &value) const
{
   for (const auto &item : m_data) {
      if (item == value) {
         return true;
      }
   }

   return false;
}

template <typename T>
typename QList<T>::size_type QList<T>::count(const T &value) const
{
   size_type retval = 0;

   for (const auto &item : m_data) {
      if (item == value) {
         ++retval;
      }
   }

   return retval;
}

template <typename T>
inline void QList<T>::insert(size_type i, const T &value)
{
   Q_ASSERT_X(i >= 0 && i <= size(), "QList<T>::insert", "index out of range");
   m_data.insert(m_data.begin() + i, value);
}

template<typename T>
QList<T> QList<T>::mid(size_type pos, size_type alength) const
{
   Q_ASSERT_X(pos < size(), "QList<T>::mid", "pos out of range");

   if (alength < 0 || pos + alength > size()) {
      alength = size() - pos;
   }

   if (pos == 0 && alength == size()) {
      return *this;
   }

   QList<T> retval(m_data.begin() + pos, m_data.begin() + pos + alength);

   return retval;
}

template <typename T>
inline void QList<T>::move(size_type from, size_type to)
{
   Q_ASSERT_X(from >= 0 && from < size(), "QList<T>::move", "from index out of range");
   Q_ASSERT_X(to   >= 0 && to   < size(), "QList<T>::move", "to index out of range");

   if (to == from) {
      // do nothing

   } else if (to > from) {
      // forward
      std::rotate(m_data.begin() + from, m_data.begin() + from + 1, m_data.begin() + to + 1);

   } else {
      // reverse
      std::rotate(m_data.rend() - from - 1, m_data.rend() - from, m_data.rend() - to);

   }
}

template <typename T>
typename QList<T>::size_type QList<T>::removeAll(const T &value)
{
   auto iter  = std::remove(m_data.begin(), m_data.end(), value);
   int retval = m_data.end() - iter;

   m_data.erase(iter, m_data.end());

   return retval;
}

template <typename T>
inline void QList<T>::replace(size_type i, const T &value)
{
   Q_ASSERT_X(i >= 0 && i < size(), "QList<T>::replace", "index out of range");
   m_data[i] = value;
}

template <typename T>
bool QList<T>::removeOne(const T &value)
{
   size_type index = indexOf(value);

   if (index != -1) {
      removeAt(index);
      return true;
   }

   return false;
}

template <typename T>
inline void QList<T>::swap(size_type i, size_type j)
{
   Q_ASSERT_X(i >= 0 && i < size() && j >= 0 && j < size(), "QList<T>::swap", "index out of range");
   qSwap(m_data[i], m_data[j]);
}

template <typename T>
inline T QList<T>::takeAt(size_type i)
{
   if (i < 0 || i >= size()) {

      std::string msg = "QList<T>::takeAt() Index is out of Range. (Index is " + std::to_string(i) +
                        ", Size is " + std::to_string(size()) + ")";
      throw std::logic_error(msg);
   }

   T value = std::move(m_data[i]);
   removeAt(i);

   return value;
}

template <typename T>
inline T QList<T>::takeFirst()
{
   T value = first();
   removeFirst();

   return value;
}

template <typename T>
inline T QList<T>::takeLast()
{
   T value = last();
   removeLast();

   return value;
}

template<typename T>
T QList<T>::value(size_type i) const
{
   if (i < 0 || i >= size()) {
      return T();
   }

   return m_data[i];
}

template<typename T>
T QList<T>::value(size_type i, const T &defaultValue) const
{
   return ((i < 0 || i >= size()) ? defaultValue : m_data[i]);
}

// operators
template <typename T>
inline const T &QList<T>::operator[](size_type i) const
{
   if (i < 0 || i >= size()) {

      std::string msg = "QList<T>::operator[] Index is out of Range. (Index is " + std::to_string(i) +
                        ", Size is " + std::to_string(size()) + ")";
      throw std::logic_error(msg);
   }

   return m_data[i];
}

template <typename T>
inline T &QList<T>::operator[](size_type i)
{
   if (i < 0 || i >= size()) {

      std::string msg = "QList<T>::operator[] Index is out of Range. (Index is " + std::to_string(i) +
                        ", Size is " + std::to_string(size()) + ")";
      throw std::logic_error(msg);
   }

   return m_data[i];
}

template <typename T>
uint qHash(const QList<T> &list, uint seed = 0)
{
   for (const auto &item : list)  {
      seed = qHash(item, seed);
   }

   return seed;
}

template <class T>
class QListIterator
{
   typedef typename QList<T>::const_iterator const_iterator;
   QList<T> c;
   const_iterator i;

   public:
      inline QListIterator(const QList<T> &container)
         : c(container), i(c.constBegin()) {}

      inline QListIterator &operator=(const QList<T> &container)
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
class QMutableListIterator
{
   typedef typename QList<T>::iterator iterator;
   typedef typename QList<T>::const_iterator const_iterator;

   QList<T> *c;
   iterator i, n;
   inline bool item_exists() const { return const_iterator(n) != c->constEnd(); }

   public:
      inline QMutableListIterator(QList<T> &container)
         : c(&container)
      {
         i = c->begin();
         n = c->end();
      }

      inline ~QMutableListIterator()
         {  }

      inline QMutableListIterator &operator=(QList<T> &container)
      {
         c = &container;
         i = c->begin();
         n = c->end();

         return *this;
      }

      inline void toFront() { i = c->begin(); n = c->end(); }
      inline void toBack() { i = c->end(); n = i; }
      inline bool hasNext() const { return c->constEnd() != const_iterator(i); }
      inline T &next() { n = i++; return *n; }
      inline T &peekNext() const { return *i; }
      inline bool hasPrevious() const { return c->constBegin() != const_iterator(i); }
      inline T &previous() { n = --i; return *n; }
      inline T &peekPrevious() const { iterator p = i; return *--p; }

      inline void remove()
      {
         if (c->constEnd() != const_iterator(n)) {
            i = c->erase(n);
            n = c->end();
         }
      }

      inline void setValue(const T &t) const { if (c->constEnd() != const_iterator(n)) *n = t; }
      inline T &value() { Q_ASSERT(item_exists()); return *n; }
      inline const T &value() const { Q_ASSERT(item_exists()); return *n; }
      inline void insert(const T &t) { n = i = c->insert(i, t); ++i; }

      inline bool findNext(const T &t)
         { while (c->constEnd() != const_iterator(n = i)) if (*i++ == t) return true; return false; }

      inline bool findPrevious(const T &t)
      {
         while (c->constBegin() != const_iterator(i)) {
            if (*(n = --i) == t) {
               return true;
            }
         }

         n = c->end();
         return false;
      }
};

#endif
