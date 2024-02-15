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

#ifndef QVECTOR_H
#define QVECTOR_H

#include <qcontainerfwd.h>
#include <qlist.h>

#include <initializer_list>
#include <vector>

template <class T>
class QVectorIterator;

template <class T>
class QMutableVectorIterator;

template <typename T>
class QVector
{
 public:
   using difference_type = typename std::vector<T>::difference_type;
   using pointer         = typename std::vector<T>::pointer;
   using reference       = typename std::vector<T>::reference;
   using size_type       = typename std::vector<T>::difference_type;      // makes this signed instead of unsigned
   using value_type      = typename std::vector<T>::value_type;

   using allocator_type  = typename std::vector<T>::allocator_type;

   using iterator        = typename std::vector<T>::iterator;
   using const_iterator  = typename std::vector<T>::const_iterator;

   using const_pointer   = typename std::vector<T>::const_pointer;
   using const_reference = typename std::vector<T>::const_reference;

   using reverse_iterator       = typename std::vector<T>::reverse_iterator;
   using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

   // java
   using Java_Iterator          = QVectorIterator<T>;
   using Java_MutableIterator   = QMutableVectorIterator<T>;

   QVector() = default;
   explicit QVector(size_type size)
      : m_data(size)
   {
   }

   QVector(size_type size, const T &value)
      : m_data(size, value)
   {
   }

   QVector(const QVector<T> &other) = default;
   QVector(QVector<T> &&other) = default;

   QVector(std::initializer_list<T> args)
      : m_data(args)
   { }

   template <class Input_Iterator>
   QVector(Input_Iterator first, Input_Iterator last)
      : m_data(first, last) {}

   ~QVector() = default;

   // methods
   void append(const T &value) {
      m_data.push_back(value);
   }

   void append(T &&value) {
      m_data.push_back(std::move(value));
   }

   void append(const QVector<T> &other) {
      m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
   }

   template <typename Iter>
   void append(Iter begin, Iter end) {
      m_data.insert(m_data.end(), begin, end);
   }

   const_reference at(size_type pos) const;

   reference back() {
      return m_data.back();
   }

   const_reference back() const {
      return m_data.back();
   }

   size_type capacity() const {
      return m_data.capacity();
   }

   void clear() {
      m_data.clear();
   }

   bool contains(const T &value) const;
   size_type count(const T &value) const;

   size_type count() const {
      return size();
   }

   T *data() {
      return m_data.data();
   }

   const T *data() const {
      return m_data.data();
   }

   const T *constData() const {
      return m_data.data();
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

   QVector<T> &fill(const T &value, size_type size = -1);

   const_reference constFirst() const {
      Q_ASSERT(! isEmpty());
      return m_data.front();
   }

   reference first() {
      Q_ASSERT(! isEmpty());
      return m_data.front();
   }

   const_reference first() const {
      Q_ASSERT(! isEmpty());
      return m_data.front();
   }

   reference front() {
      return m_data.front();
   }

   const_reference front() const {
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

   void insert(size_type pos, const T &value);
   void insert(size_type pos, size_type count, const T &value);

   const_reference constLast() const {
      Q_ASSERT(! isEmpty());
      return m_data.back();
   }

   reference last() {
      Q_ASSERT(! isEmpty());
      return m_data.back();
   }

   const_reference last() const {
      Q_ASSERT(!isEmpty());
      return m_data.back();
   }

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

   QVector<T> mid(size_type pos, size_type length = -1) const;
   void move(size_type from, size_type to);

   void pop_back() {
      Q_ASSERT(! isEmpty());
      m_data.pop_back();
   }

   void pop_front() {
      Q_ASSERT(! isEmpty());
      m_data.erase(m_data.begin());
   }

   void prepend(const T &value) {
      insert(m_data.begin(), 1, value);
   }

   void push_back(const T &value) {
      m_data.push_back(value);
   }

   void push_back(T &&value) {
      m_data.push_back(std::move(value));
   }

   void push_front(const T &value) {
      m_data.insert(m_data.begin(), value);
   }

   void push_front(T &&value) {
      m_data.insert(m_data.begin(), std::move(value));
   }

   void remove(size_type pos) {
      Q_ASSERT_X(pos >= 0 && pos < size(), "QVector<T>::remove", "index out of range");
      erase(begin() + pos, begin() + pos + 1);
   }

   void remove(size_type pos, size_type count)  {
      Q_ASSERT_X(pos >= 0 && count >= 0 && pos + count <= size(), "QVector<T>::remove", "index out of range");
      m_data.erase(m_data.begin() + pos, m_data.begin() + pos + count);
   }

   size_type removeAll(const T &value);

   void removeAt(size_type pos) {
      Q_ASSERT_X(pos >= 0 && pos < size(), "QVector<T>::removeAt", "index out of range");
      m_data.erase(m_data.begin() + pos);
   }

   void removeFirst() {
      Q_ASSERT(! isEmpty());
      m_data.erase(m_data.begin());
   }

   void removeLast() {
      Q_ASSERT(! isEmpty());
      m_data.pop_back();
   }

   bool removeOne(const T &value) {
      auto iter = std::find(m_data.begin(), m_data.end(), value);

      if (iter != m_data.end()) {
         m_data.erase(iter);
         return true;
      }

      return false;
   }

   void replace(size_type i, const T &value);

   void reserve(size_type size) {
      m_data.reserve(size);
   }

   void resize(size_type size) {
      m_data.resize(size);
   }

   size_type size() const {
      // returns unsigned, must convert to signed
      return static_cast<size_type>(m_data.size());
   }

   void squeeze() {
      m_data.shrink_to_fit();
   }

   bool startsWith(const T &value) const {
      return ! isEmpty() && m_data.front() == value;
   }

   void swap(QVector<T> &other) {
      qSwap(m_data, other.m_data);
   }

   T takeAt(size_type pos);
   T takeFirst();
   T takeLast();

   T value(size_type pos) const;
   T value(size_type pos, const T &defaultValue) const;

   // to from
   static QVector<T> fromList(const QList<T> &list) {
      return list.toVector();
   }

   static QVector<T> fromStdVector(const std::vector<T> &vector) {
      QVector<T> tmp;
      tmp.m_data = vector;
      return tmp;
   }

   QList<T> toList() const;

   std::vector<T> toStdVector() const {
      return m_data;
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

   iterator erase(const_iterator begin, const_iterator end) {
      return m_data.erase(begin, end);
   }

   iterator erase(const_iterator pos) {
      return m_data.erase(pos);
   }

   iterator insert(iterator before, size_type count, const T &value) {
      return m_data.insert(before, count, value);
   }

   iterator insert(iterator before, const T &value) {
      return m_data.insert(before, value);
   }

   template <typename Iter>
   iterator insert(iterator pos, Iter begin, Iter end) {
      return m_data.insert(pos, begin, end);
   }

   // operators
   QVector<T> &operator=(const QVector<T> &other) = default;
   QVector<T> &operator=(QVector<T> &&other)      = default;

   bool operator==(const QVector<T> &other) const {
      return (m_data == other.m_data);
   }

   bool operator!=(const QVector<T> &other) const {
      return (m_data != other.m_data);
   }

   reference operator[](size_type i);
   const_reference operator[](size_type i) const;

   QVector<T> &operator+=(const QVector<T> &other);

   QVector<T> operator+(const QVector<T> &other) const {
      QVector n = *this;
      n += other;
      return n;
   }

   QVector<T> &operator+=(const T &value) {
      append(value);
      return *this;
   }

   QVector<T> &operator<< (const T &value) {
      append(value);
      return *this;
   }

   QVector<T> &operator<<(const QVector<T> &other) {
      *this += other;
      return *this;
   }

 private:
   std::vector<T> m_data;
};

// constructors
template <typename T>
inline typename QVector<T>::const_reference QVector<T>::at(size_type i) const
{
   Q_ASSERT_X(i < size(), "QVector<T>::at", "index out of range");
   return m_data[i];
}

template <typename T>
bool QVector<T>::contains(const T &value) const
{
   for (const auto &item : m_data) {
      if (item == value) {
         return true;
      }
   }

   return false;
}

template <typename T>
typename QVector<T>::size_type QVector<T>::count(const T &value) const
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
QVector<T> &QVector<T>::fill(const T &value, size_type newSize)
{
   if (newSize != -1) {
      m_data.resize(newSize);
   }

   for (auto &item : m_data) {
      item = value;
   }

   return *this;
}

template <typename T>
inline void QVector<T>::insert(size_type pos, const T &value)
{
   Q_ASSERT_X(pos >= 0 && pos <= size(), "QVector<T>::insert", "index out of range");
   m_data.insert(m_data.begin() + pos, value);
}

template <typename T>
inline void QVector<T>::insert(size_type pos, size_type count, const T &value)
{
   Q_ASSERT_X(pos >= 0 && pos <= size(), "QVector<T>::insert", "index out of range");
   m_data.insert(m_data.begin() + pos, count, value);
}

template <typename T>
QVector<T> QVector<T>::mid(size_type pos, size_type length) const
{
   if (length < 0 || pos + length > size()) {
      length = size() - pos;
   }

   if (pos == 0 && length == size()) {
      return *this;
   }

   QVector<T> retval(m_data.begin() + pos, m_data.begin() + pos + length);

   return retval;
}

template <typename T>
inline void QVector<T>::move(size_type from, size_type to)
{
   Q_ASSERT_X(from >= 0 && from < size(), "QVector<T>::move", "from index out of range");
   Q_ASSERT_X(to   >= 0 && to   < size(), "QVector<T>::move", "to index out of range");

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
typename QVector<T>::size_type QVector<T>::removeAll(const T &value)
{
   auto iter  = std::remove(m_data.begin(), m_data.end(), value);
   int retval = m_data.end() - iter;

   m_data.erase(iter, m_data.end());

   return retval;
}

template <typename T>
inline void QVector<T>::replace(size_type i, const T &value)
{
   Q_ASSERT_X(i >= 0 && i < size(), "QVector<T>::replace", "index out of range");
   m_data[i] = value;
}

template <typename T>
inline T QVector<T>::takeAt(size_type i)
{
   if (i < 0 || i >= size()) {

      std::string msg = "QVector<T>::takeAt() Index is out of Range. (Index is " + std::to_string(i) +
                        ", Size is " + std::to_string(size()) + ")";
      throw std::logic_error(msg);
   }

   T value = std::move(m_data[i]);
   removeAt(i);

   return value;
}

template <typename T>
inline T QVector<T>::takeFirst()
{
   T value = first();
   removeFirst();

   return value;
}

template <typename T>
inline T QVector<T>::takeLast()
{
   T value = last();
   removeLast();

   return value;
}

template <typename T>
T QVector<T>::value(size_type i) const
{
   if (i < 0 || i >= size()) {
      return T();
   }

   return m_data.begin()[i];
}

template <typename T>
T QVector<T>::value(size_type i, const T &defaultValue) const
{
   return ((i < 0 || i >= size()) ? defaultValue : m_data[i]);
}

// operators
template <typename T>
inline typename QVector<T>::const_reference QVector<T>::operator[](size_type i) const
{
   Q_ASSERT_X(i >= 0 && i < size(), "QVector<T>::operator[]", "index out of range");
   return m_data[i];
}

template <typename T>
inline typename QVector<T>::reference QVector<T>::operator[](size_type i)
{
   Q_ASSERT_X(i < size(), "QVector<T>::operator[]", "index out of range");
   return m_data[i];
}

template <typename T>
QVector<T> &QVector<T>::operator+=(const QVector &other)
{
   m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
   return *this;
}

template <typename T>
QList<T> QVector<T>::toList() const
{
   QList<T> result;

   for (const auto &pos : m_data) {
      result.append(pos);
   }

   return result;
}

// methods declared in QList
template <typename T>
QVector<T> QList<T>::toVector() const
{
   QVector<T> result( m_data.begin(), m_data.end() );
   return result;
}

template <typename T>
QList<T> QList<T>::fromVector(const QVector<T> &vector)
{
   return vector.toList();
}

// java style iterators
template <class T>
class QVectorIterator
{
   using const_iterator = typename QVector<T>::const_iterator;

   QVector<T> c;
   const_iterator i;

 public:
   QVectorIterator(const QVector<T> &vector)
      : c(vector), i(c.constBegin())
   {
   }

   QVectorIterator &operator=(const QVector<T> &vector) {
      c = vector;
      i = c.constBegin();
      return *this;
   }

   void toFront() {
      i = c.constBegin();
   }

   void toBack() {
      i = c.constEnd();
   }

   bool hasNext() const {
      return i != c.constEnd();
   }

   const T &next() {
      return *(i++);
   }

   const T &peekNext() const {
      return *i;
   }

   bool hasPrevious() const {
      return i != c.constBegin();
   }

   const T &previous() {
      return *(--i);
   }

   const T &peekPrevious() const {
      const_iterator p = i;
      return *(--p);
   }

   bool findNext(const T &value) {
      while (i != c.constEnd()) {
         if (*(i++) == value) {
            return true;
         }
      }

      return false;
   }

   bool findPrevious(const T &value) {
      while (i != c.constBegin()) {
         if (*(--i) == value)  {
            return true;
         }
      }

      return false;
   }
};

template <class T>
class QMutableVectorIterator
{
   using iterator       = typename QVector<T>::iterator;
   using const_iterator = typename QVector<T>::const_iterator;

   QVector<T> *c;
   iterator i, n;

   bool item_exists() const {
      return const_iterator(n) != c->constEnd();
   }

 public:
   QMutableVectorIterator(QVector<T> &vector)
      : c(&vector)
   {
      i = c->begin();
      n = c->end();
   }

   QMutableVectorIterator &operator=(QVector<T> &vector) {
      c = &vector;
      i = c->begin();
      n = c->end();
      return *this;
   }

   void toFront() {
      i = c->begin();
      n = c->end();
   }

   void toBack() {
      i = c->end();
      n = i;
   }

   bool hasNext() const {
      return c->constEnd() != const_iterator(i);
   }

   T &next() {
      n = i++;
      return *n;
   }

   T &peekNext() const {
      return *i;
   }

   bool hasPrevious() const {
      return c->constBegin() != const_iterator(i);
   }

   T &previous() {
      n = --i;
      return *n;
   }

   T &peekPrevious() const {
      iterator p = i;
      return *(--p);
   }

   void remove() {
      if (c->constEnd() != const_iterator(n)) {
         i = c->erase(n);
         n = c->end();
      }
   }

   void setValue(const T &value) const {
      if (c->constEnd() != const_iterator(n)) {
         *n = value;
      }
   }

   T &value() {
      Q_ASSERT(item_exists());
      return *n;
   }

   const T &value() const {
      Q_ASSERT(item_exists());
      return *n;
   }

   void insert(const T &value) {
      n = c->insert(i, value);
      i = n;
      ++i;
   }

   bool findNext(const T &value) {
      while (c->constEnd() != const_iterator(n = i)) {

         if (*(i++) == value) {
            return true;
         }
      }

      return false;
   }

   bool findPrevious(const T &value) {
      while (c->constBegin() != const_iterator(i)) {
         n = --i;

         if (*n == value) {
            return true;
         }
      }

      n = c->end();

      return false;
   }
};

#endif
