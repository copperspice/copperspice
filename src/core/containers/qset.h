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

#ifndef QSET_H
#define QSET_H

#include <qhash.h>

#include <initializer_list>
#include <unordered_set>

template <class T>
class QSetIterator;

template <class T>
class QMutableSetIterator;

template <class T>
class QSet
{
   class Hash
   {
    public:
      size_t operator()(const T &data) const {
         return qHash(data);
      }
   };

 public:
   using difference_type = typename std::unordered_set<T, Hash>::difference_type;
   using pointer         = typename std::unordered_set<T, Hash>::pointer;
   using reference       = typename std::unordered_set<T, Hash>::reference;
   using size_type       = typename std::unordered_set<T, Hash>::difference_type;      // makes this signed instead of unsigned
   using value_type      = typename std::unordered_set<T, Hash>::value_type;

   using key_type        = typename std::unordered_set<T, Hash>::key_type;
   using key_equal       = typename std::unordered_set<T, Hash>::key_equal;
   using hasher          = typename std::unordered_set<T, Hash>::hasher;

   using allocator_type  = typename std::unordered_set<T, Hash>::allocator_type;

   using iterator        = typename std::unordered_set<T, Hash>::iterator;
   using const_iterator  = typename std::unordered_set<T, Hash>::const_iterator;

   using const_pointer   = typename std::unordered_set<T, Hash>::const_pointer;
   using const_reference = typename std::unordered_set<T, Hash>::const_reference;

   // java
   using Java_Iterator        = QSetIterator<T>;
   using Java_MutableIterator = QMutableSetIterator<T>;

   QSet() = default;
   QSet(const QSet<T> &other) = default;
   QSet(QSet<T> &&other) = default;

   QSet(std::initializer_list<T> args)
      : m_data(args)  {}

   template <class Input_Iterator>
   QSet(Input_Iterator first, Input_Iterator last);

   // methods
   size_type capacity() const {
      return m_data.capacity();
   }

   void clear() {
      m_data.clear();
   }

   bool contains(const QSet<T> &other) const;

   bool contains(const T &value) const {
      return m_data.count(value);
   }

   size_type count() const {
      return size();
   }

   bool empty() const {
      return m_data.empty();
   }

   bool isEmpty() const {
      return m_data.empty();
   }

   size_type erase(const key_type &key) {
      return m_data.erase(key);
   }

   QSet<T> &intersect(const QSet<T> &other);
   bool intersects(const QSet<T> &other) const;

   bool remove(const T &value) {
      return m_data.erase(value) != 0;
   }

   void reserve(size_type size) {
      m_data.reserve(size);
   }

   size_type size() const {
      // returns unsigned, must convert to signed
      return static_cast<size_type>(m_data.size());
   }

   void squeeze() {
      m_data.reserve(size());
   }

   QSet<T> &subtract(const QSet<T> &other);

   void swap(QSet<T> &other) {
      m_data.swap(other.m_data);
   }

   QSet<T> &unite(const QSet<T> &other);

   QList<T> values() const {
      return toList();
   }

   // iterators
   iterator begin() {
      return m_data.begin();
   }

   const_iterator begin() const {
      return m_data.begin();
   }

   const_iterator cbegin() const {
      return m_data.begin();
   }

   const_iterator constBegin() const {
      return m_data.begin();
   }

   iterator end() {
      return m_data.end();
   }

   const_iterator end() const {
      return m_data.end();
   }

   const_iterator cend() const {
      return m_data.end();
   }

   const_iterator constEnd() const {
      return m_data.end();
   }

   iterator erase(const_iterator pos) {
      return m_data.erase(pos);
   }

   iterator erase(const_iterator first, const_iterator last) {
      return m_data.erase(first, last);
   }

   iterator find(const T &value) {
      return m_data.find(value);
   }

   const_iterator find(const T &value) const {
      return m_data.find(value);
   }

   const_iterator constFind(const T &value) const {
      return find(value);
   }

   iterator insert(const T &value) {
      return m_data.insert(value).first;
   }

   // reverse iterators - do not apply

   // operators
   QSet<T> &operator=(const QSet<T> &other)  = default;
   QSet<T> &operator=(QSet<T> && other)  = default;

   bool operator==(const QSet<T> &other) const {
      return m_data == other.m_data;
   }

   bool operator!=(const QSet<T> &other) const {
      return m_data != other.m_data;
   }

   QSet<T> &operator<<(const T &value) {
      insert(value);
      return *this;
   }

   QSet<T> &operator|=(const QSet<T> &other) {
      unite(other);
      return *this;
   }

   QSet<T> &operator|=(const T &value) {
      insert(value);
      return *this;
   }

   QSet<T> &operator&=(const QSet<T> &other) {
      intersect(other);
      return *this;
   }

   QSet<T> &operator&=(const T &value) {
      QSet<T> result;

      if (contains(value)) {
         result.insert(value);
      }

      return (*this = result);
   }

   QSet<T> &operator+=(const QSet<T> &other) {
      unite(other);
      return *this;
   }

   QSet<T> &operator+=(const T &value) {
      insert(value);
      return *this;
   }

   QSet<T> &operator-=(const QSet<T> &other) {
      subtract(other);
      return *this;
   }

   QSet<T> &operator-=(const T &value) {
      remove(value);
      return *this;
   }

   QSet<T> operator|(const QSet<T> &other) const {
      QSet<T> result = *this;
      result |= other;
      return result;
   }

   QSet<T> operator&(const QSet<T> &other) const {
      QSet<T> result = *this;
      result &= other;
      return result;
   }

   QSet<T> operator+(const QSet<T> &other) const {
      QSet<T> result = *this;
      result += other;
      return result;
   }

   QSet<T> operator-(const QSet<T> &other) const {
      QSet<T> result = *this;
      result -= other;
      return result;
   }

   // to from
   QList<T> toList() const;

   static QSet<T> fromList(const QList<T> &list);

 private:
   std::unordered_set<T, Hash> m_data;
};

template <class T>
inline QSet<T> &QSet<T>::unite(const QSet<T> &other)
{
   m_data.insert(other.m_data.begin(), other.m_data.end());
   return *this;
}

template <class T>
inline QSet<T> &QSet<T>::intersect(const QSet<T> &other)
{
   auto iter = m_data.cbegin();

   while (iter != m_data.cend()) {

      if (! other.contains(*iter)) {
         iter = m_data.erase(iter);

      } else  {
         ++iter;
      }
   }

   return *this;
}

template <class T>
inline bool QSet<T>::intersects(const QSet<T> &other) const
{
   for (const auto &item : m_data) {
      if (other.contains(item)) {
         return true;
      }
   }

   return false;
}

template <class T>
inline QSet<T> &QSet<T>::subtract(const QSet<T> &other)
{
   auto iter = m_data.cbegin();

   while (iter != m_data.cend()) {

      if (other.contains(*iter)) {
         iter = m_data.erase(iter);

      } else  {
         ++iter;
      }
   }

   return *this;
}

template <class T>
inline bool QSet<T>::contains(const QSet<T> &other) const
{
   for (const auto &item : other.m_data) {
      if (! contains(item)) {
         return false;
      }
   }

   return true;
}

template <typename T>
QList<T> QSet<T>::toList() const
{
   QList<T> result;

   for (const auto &item : m_data) {
      result.append(item);
   }

   return result;
}

template <typename T>
QSet<T> QList<T>::toSet() const
{
   QSet<T> result;
   result.reserve(size());

   for (const auto &item : m_data) {
      result.insert(item);
   }

   return result;
}

template <typename T>
QSet<T> QSet<T>::fromList(const QList<T> &list)
{
   return list.toSet();
}

template <typename T>
QList<T> QList<T>::fromSet(const QSet<T> &set)
{
   return set.toList();
}

template <class T>
class QSetIterator
{
   using const_iterator = typename QSet<T>::const_iterator;

   QSet<T> c;
   const_iterator i;

 public:
   QSetIterator(const QSet<T> &set)
      : c(set), i(c.constBegin())
   { }

   QSetIterator &operator=(const QSet<T> &set) {
      c = set;
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
      return *--p;
   }

   bool findNext(const T &value)  {
      while (i != c.constEnd()) {
         if (*(i++) == value) {
            return true;
         }
      }

      return false;
   }

   bool findPrevious(const T &value)   {
      while (i != c.constBegin()) {
         if (*(--i) == value)  {
            return true;
         }
      }

      return false;
   }
};

template <typename T>
class QMutableSetIterator
{
   using iterator = typename QSet<T>::iterator;

   QSet<T> *c;
   iterator i, n;

   bool item_exists() const {
      return c->constEnd() != n;
   }

 public:
   QMutableSetIterator(QSet<T> &set)
      : c(&set)
   {
      i = c->begin();
      n = c->end();
   }

   ~QMutableSetIterator() {
   }

   QMutableSetIterator &operator=(QSet<T> &set) {
      c = &set;
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
      return c->constEnd() != i;
   }

   const T &next() {
      n = i++;
      return *n;
   }

   const T &peekNext() const {
      return *i;
   }

   bool hasPrevious() const {
      return c->constBegin() != i;
   }

   const T &previous() {
      n = --i;
      return *n;
   }

   const T &peekPrevious() const {
      iterator p = i;
      return *(--p);
   }

   void remove() {
      if (c->constEnd() != n) {
         i = c->erase(n);
         n = c->end();
      }
   }

   const T &value() const {
      Q_ASSERT(item_exists());
      return *n;
   }

   bool findNext(const T &value) {
      while (c->constEnd() != (n = i)) {
         if (*(i++) == value) {
            return true;
         }
      }

      return false;
   }

   bool findPrevious(const T &value) {
      while (c->constBegin() != i) {
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