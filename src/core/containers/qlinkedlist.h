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

#ifndef QLINKEDLIST_H
#define QLINKEDLIST_H

#include <qcontainerfwd.h>

#include <qassert.h>
#include <iterator>
#include <list>

template <class T>
class QLinkedListIterator;

template <class T>
class QMutableLinkedListIterator;

template <class T>
class QLinkedList
{
 public:
   using difference_type = typename std::list<T>::difference_type;
   using pointer         = typename std::list<T>::pointer;
   using reference       = typename std::list<T>::reference;
   using size_type       = typename std::list<T>::difference_type;      // makes this signed instead of unsigned
   using value_type      = typename std::list<T>::value_type;

   using allocator_type  = typename std::list<T>::allocator_type;

   using iterator        = typename std::list<T>::iterator;
   using const_iterator  = typename std::list<T>::const_iterator;

   using const_pointer   = typename std::list<T>::const_pointer;
   using const_reference = typename std::list<T>::const_reference;

   using reverse_iterator       = typename std::list<T>::reverse_iterator;
   using const_reverse_iterator = typename std::list<T>::const_reverse_iterator;

   // java
   using Java_Iterator          = QLinkedListIterator<T>;
   using Java_MutableIterator   = QMutableLinkedListIterator<T>;

   QLinkedList() = default;
   QLinkedList(const QLinkedList<T> &other) = default;
   QLinkedList(QLinkedList<T> &&other)      = default;

   QLinkedList(std::initializer_list<T> args)
      : m_data(args)
   { }

   template <class Input_Iterator>
   QLinkedList(Input_Iterator first, Input_Iterator last)
      : m_data(first, last)
   { }

   ~QLinkedList() = default;

   // methods
   void append(const T &value) {
      m_data.push_back(value);
   }

   void append(T &&value) {
      m_data.push_back(std::move(value));
   }

   void append(const QLinkedList<T> &other) {
      m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
   }

   reference back() {
      return m_data.back();
   }

   const_reference back() const {
      return m_data.back();
   }

   void clear() {
      m_data.clear();
   }

   bool contains(const T &value) const;

   size_type count() const {
      return size();
   }

   size_type count(const T &value) const;

   bool empty() const {
      return m_data.empty();
   }

   bool isEmpty() const {
      return m_data.empty();
   }

   bool endsWith(const T &value) const {
      return ! isEmpty() && m_data.back() == value;
   }

   reference first() {
      Q_ASSERT(! isEmpty());
      return *begin();
   }

   const_reference first() const {
      Q_ASSERT(! isEmpty());
      return *begin();
   }

   reference front() {
      return m_data.front();
   }

   const_reference front() const {
      return m_data.front();
   }

   reference last() {
      Q_ASSERT(! isEmpty());
      return *(--m_data.end());
   }

   const_reference last() const {
      Q_ASSERT(! isEmpty());
      return *(--m_data.end());
   }

   size_type length() const {
      return size();
   }

   void pop_front() {
      Q_ASSERT(! isEmpty());
      m_data.pop_front();
   }

   void pop_back() {
      Q_ASSERT(! isEmpty());
      m_data.pop_back();
   }

   void prepend(const T &value) {
      insert(m_data.begin(), 1, value);
   }

   void push_back(const T &value) {
      m_data.push_back(value);
   }

   void push_front(const T &value) {
      m_data.push_front(value);
   }

   size_type removeAll(const T &value);
   bool removeOne(const T &value);

   void removeFirst() {
      Q_ASSERT(! isEmpty());
      erase(begin());
   }

   void removeLast() {
      Q_ASSERT(! isEmpty());
      erase(--end());
   }

   size_type size() const {
      // returns unsigned, must convert to signed
      return static_cast<size_type>(m_data.size());
   }

   bool startsWith(const T &value) const {
      return !isEmpty() && m_data.first() == value;
   }

   void swap(QLinkedList<T> &other) {
      qSwap(m_data, other.m_data);
   }

   T takeFirst();
   T takeLast();

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
      return  m_data.erase(pos);
   }

   iterator insert(iterator before, size_type count, const T &value) {
      return m_data.insert(before, count, value);
   }

   iterator insert(iterator before, const T &value) {
      return m_data.insert(before, value);
   }

   // operators
   QLinkedList<T> &operator=(const QLinkedList<T> &other) = default;
   QLinkedList<T> &operator=(QLinkedList<T> && other) = default;

   bool operator==(const QLinkedList<T> &other) const {
      return (m_data == other.m_data);
   }

   bool operator!=(const QLinkedList<T> &other) const {
      return (m_data != other.m_data);
   }

   QLinkedList<T> operator+(const QLinkedList<T> &other) const {
      QLinkedList n = *this;
      n += other;
      return n;
   }

   QLinkedList<T> &operator+=(const QLinkedList<T> &other);

   QLinkedList<T> &operator+=(const T &value) {
      m_data.push_back(value);
      return *this;
   }

   QLinkedList<T> &operator<< (const T &value) {
      m_data.push_back(value);
      return *this;
   }

   QLinkedList<T> &operator<<(const QLinkedList<T> &other) {
      *this += other;
      return *this;
   }

   // to from
   static QLinkedList<T> fromStdList(const std::list<T> &other) {
      QLinkedList<T> tmp(other.begin(), other.end());
      return tmp;
   }

   std::list<T> toStdList() const {
      return m_data;
   }

 private:
   std::list<T> m_data;

};

// methods
template <typename T>
bool QLinkedList<T>::contains(const T &value) const
{
   for (const auto &item : m_data) {
      if (item == value) {
         return true;
      }
   }

   return false;
}

template <typename T>
typename QLinkedList<T>::size_type QLinkedList<T>::count(const T &value) const
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
typename QLinkedList<T>::size_type QLinkedList<T>::removeAll(const T &value)
{
   auto iter = std::remove(m_data.begin(), m_data.end(), value);

   size_type retval = m_data.end() - iter;
   m_data.erase(iter, m_data.end());

   return retval;
}

template <typename T>
bool QLinkedList<T>::removeOne(const T &value)
{
   auto iter = m_data.begin();

   while (iter != m_data.end()) {

      if (*iter == value) {
         m_data.erase(iter);
         return true;
      }

      ++iter;
   }

   return false;
}

template <typename T>
inline T QLinkedList<T>::takeFirst()
{
   T t = first();
   removeFirst();
   return t;
}

template <typename T>
inline T QLinkedList<T>::takeLast()
{
   T t = last();
   removeLast();
   return t;
}

// operators
template <typename T>
QLinkedList<T> &QLinkedList<T>::operator+=(const QLinkedList<T> &other)
{
   m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
   return *this;
}

template <class T>
class QLinkedListIterator
{
   using const_iterator = typename QLinkedList<T>::const_iterator;

   QLinkedList<T> c;
   const_iterator i;

 public:
   QLinkedListIterator(const QLinkedList<T> &list)
      : c(list), i(c.constBegin())
   {
   }

   QLinkedListIterator &operator=(const QLinkedList<T> &list) {
      c = list;
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
      return *i++;
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

   bool findNext(const T &value)  {
      while (i != c.constEnd()) {
         if (*(i++) == value) {
            return true;
         }
      }

      return false;
   }

   bool findPrevious(const T &value) {
      while (i != c.constBegin()) {
         if (*(--i) == value) {
            return true;
         }
      }

      return false;
   }
};

template <class T>
class QMutableLinkedListIterator
{
   using iterator       = typename QLinkedList<T>::iterator;
   using const_iterator = typename QLinkedList<T>::const_iterator;

   QLinkedList<T> *c;
   iterator i, n;

   bool item_exists() const {
      return const_iterator(n) != c->constEnd();
   }

 public:
   QMutableLinkedListIterator(QLinkedList<T> &list)
      : c(&list)
   {
      i = c->begin();
      n = c->end();
   }

   ~QMutableLinkedListIterator()
   {
   }

   QMutableLinkedListIterator &operator=(QLinkedList<T> &list) {
      c = &list;
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
      return *--p;
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
