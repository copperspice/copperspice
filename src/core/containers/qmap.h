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

#ifndef QMAP_H
#define QMAP_H

#include <qcontainerfwd.h>
#include <qlist.h>
#include <qmapfunc.h>

#include <initializer_list>
#include <map>

template <typename Key, typename Val, typename C>
class QMapIterator;

template <typename Key, typename Val, typename C>
class QMutableMapIterator;

template <typename Key, typename Val, typename C>
class QMap
{
 public:
   class iterator
   {
    public:
      using iterator_category = std::bidirectional_iterator_tag;

      using pointer           = Val *;
      using reference         = Val &;

      using difference_type   = typename std::map<Key, Val, C>::difference_type;
      using size_type         = typename std::map<Key, Val, C>::difference_type;
      using value_type        = Val;

      iterator() = default;

      iterator(typename std::map<Key, Val, C>::iterator iter)
         : m_iter(std::move(iter)) {
      }

      const Key &key() const {
         return m_iter->first;
      }

      Val &value() const {
         return m_iter->second;
      }

      std::pair<const Key, Val> &pair() const {
         return *m_iter;
      }

      // operators
      Val &operator*() const {
         return value();
      }

      Val *operator->() const {
         return &value();
      }

      bool operator==(iterator other) const {
         return m_iter == other.m_iter;
      }

      bool operator!=(iterator other) const {
         return m_iter != other.m_iter;
      }

      iterator &operator+=(size_type n) {
         std::advance(m_iter, n);
         return *this;
      }

      iterator &operator-=(size_type n) {
         std::advance(m_iter, -n);
         return *this;
      }

      iterator operator+(size_type n) const {
         auto tmp = m_iter;
         std::advance(tmp, n);
         return tmp;
      }

      iterator operator-(size_type n) const {
         auto tmp = m_iter;
         std::advance(tmp, -n);
         return tmp;
      }

      iterator &operator++() {
         ++m_iter;
         return *this;
      }

      iterator operator++(int) {
         return m_iter++;
      }

      iterator &operator--() {
         --m_iter;
         return *this;
      }

      iterator operator--(int) {
         return m_iter--;
      }

      friend class QMap<Key, Val, C>;

    private:
      typename std::map<Key, Val, C>::iterator m_iter;
   };

   class const_iterator
   {
    public:
      using iterator_category = std::bidirectional_iterator_tag;

      using pointer         = const Val *;
      using reference       = const Val &;

      using difference_type = typename std::map<Key, Val, C>::difference_type;
      using size_type       = typename std::map<Key, Val, C>::difference_type;
      using value_type      = Val;

      const_iterator() = default;

      const_iterator(typename std::map<Key, Val, C>::const_iterator iter)
         : m_iter(std::move(iter)) {
      }

      const_iterator(iterator iter)
         : m_iter(std::move(iter.m_iter)) {
      }

      const Key &key() const {
         return m_iter->first;
      }

      const Val &value() const {
         return m_iter->second;
      }

      const std::pair<const Key, Val> &pair() const {
         return *m_iter;
      }

      // operators
      const Val &operator*() const {
         return value();
      }

      const Val *operator->() const {
         return &value();
      }

      bool operator==(const_iterator other) const {
         return m_iter == other.m_iter;
      }

      bool operator!=(const_iterator other) const {
         return m_iter != other.m_iter;
      }

      const_iterator &operator+=(size_type n) {
         std::advance(m_iter, n);
         return *this;
      }

      const_iterator &operator-=(size_type n) {
         std::advance(m_iter, -n);
         return *this;
      }

      const_iterator operator+(size_type n) const {
         auto tmp = m_iter;
         std::advance(tmp, n);
         return tmp;
      }

      const_iterator operator-(size_type n) const {
         auto tmp = m_iter;
         std::advance(tmp, -n);
         return tmp;
      }

      const_iterator &operator++() {
         ++m_iter;
         return *this;
      }

      const_iterator operator++(int) {
         return m_iter++;
      }

      const_iterator &operator--() {
         --m_iter;
         return *this;
      }

      const_iterator operator--(int) {
         return m_iter--;
      }

      friend class QMap<Key, Val, C>;

      // free functions
      friend bool operator==(iterator iter1, const_iterator iter2) {
         return iter2.operator==(iter1);
      }

      friend bool operator!=(iterator iter1, const_iterator iter2) {
         return iter2.operator!=(iter1);
      }

    private:
      typename std::map<Key, Val, C>::const_iterator m_iter;
   };

   using difference_type = typename std::map<Key, Val, C>::difference_type;
   using pointer         = Val *;
   using reference       = Val &;
   using size_type       = typename std::map<Key, Val, C>::difference_type;   // signed instead of unsigned
   using value_type      = Val;

   using key_type        = typename std::map<Key, Val, C>::key_type;
   using mapped_type     = typename std::map<Key, Val, C>::mapped_type;
   using key_compare     = typename std::map<Key, Val, C>::key_compare;

   using allocator_type  = typename std::map<Key, Val, C>::allocator_type;

   // iterator and const_iterator are classes

   using const_pointer   = const Val *;
   using const_reference = const Val &;

   using reverse_iterator       = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   // java
   using Java_Iterator          = QMapIterator<Key, Val, C>;
   using Java_MutableIterator   = QMutableMapIterator<Key, Val, C>;

   QMap() = default;

   QMap(const QMap<Key, Val, C> &other) = default;
   QMap(QMap<Key, Val, C> &&other)      = default;

   QMap(std::initializer_list<std::pair<const Key, Val>> list, const C &compare = C())
      : m_data(list, compare)
   { }

   explicit QMap(C compare)
      : m_data(compare)
   { }

   explicit QMap(const std::map<Key, Val, C> &other)
      : m_data(other)
   { }

   template <typename Input_Iterator>
   QMap(Input_Iterator first, Input_Iterator last, const C &compare = C())
      : m_data(first, last, compare)
   { }

   ~QMap() = default;

   // methods
   void clear() {
      m_data.clear();
   }

   bool contains(const Key &key) const {
      return m_data.find(key) != m_data.end();
   }

   size_type count(const Key &key) const {
      return m_data.count(key);
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

   QPair<iterator, iterator> equal_range(const Key &key) {
      return m_data.equal_range(key);
   }

   QPair<const_iterator, const_iterator> equal_range(const Key &key) const {
      return m_data.equal_range(key);
   }

   iterator erase(const_iterator iter) {
      return m_data.erase(iter.m_iter);
   }

   Val &first()  {
      return begin().value();
   }

   const Val &first() const  {
      return begin().value();
   }

   const Key &firstKey() const  {
      return begin().key();
   }

   iterator find(const Key &key) {
      // find returns an std::map::iterator, constructor will convert to QMap::iterator
      return m_data.find(key);
   }

   const_iterator find(const Key &key) const {
      return m_data.find(key);
   }

   const_iterator constFind(const Key &key) const {
      return m_data.find(key);
   }

   iterator insert(const std::pair<const Key, Val> &data) {
      return m_data.insert_or_assign(data.first, data.second).first;
   }

   iterator insert(const Key &key, const Val &value) {
      return m_data.insert_or_assign(key, value).first;
   }

   iterator insert(const_iterator hint, const Key &key, const Val &value) {
      auto oldSize = m_data.size();
      auto iter    = m_data.emplace_hint(hint.m_iter, key, value);

      if (m_data.size() == oldSize) {
         // add new element
         iter->second = value;
      }

      return iter;
   }

   const Key key(const Val &value, const Key &defaultKey = Key()) const;

   QList<Key> keys() const;
   QList<Key> keys(const Val &value) const;

   Val &last()  {
      return (end() - 1).value();
   }

   const Val &last() const  {
      return (end() - 1).value();
   }

   const Key &lastKey() const  {
      return (end() - 1).key();
   }

   iterator lowerBound(const Key &key) {
      return m_data.lower_bound(key);
   }

   const_iterator lowerBound(const Key &key) const  {
      return m_data.lower_bound(key);
   }

   size_type remove(const Key &key)  {
      return m_data.erase(key);
   }

   size_type size() const {
      // returns unsigned, must convert to signed
      return static_cast<size_type>(m_data.size());
   }

   void swap(QMap<Key, Val, C> &other) {
      qSwap(m_data, other.m_data);
   }

   Val take(const Key &key)  {
      auto iter = m_data.find(key);

      if (iter == m_data.end()) {
         return Val();
      }

      Val retval = std::move(iter->second);
      m_data.erase(iter);

      return retval;
   }

   iterator upperBound(const Key &key)  {
      return m_data.upper_bound(key);
   }

   const_iterator upperBound(const Key &key) const {
      return m_data.upper_bound(key);
   }

   QList<Key> uniqueKeys() const;

   QMap<Key, Val, C> &unite(const QMap<Key, Val, C> &other) {
      m_data.insert(other.m_data.begin(), other.m_data.end());
      return *this;
   }

   Val value(const Key &key) const;
   Val value(const Key &key, const Val &defaultValue) const;

   QList<Val> values() const;

   // to from
   std::map<Key, Val, C> toStdMap() const;

   // iterators
   iterator begin() {
      // m_data.begin is an stl iterator
      // return calls a conversion constructor since the return type is iterator, returns a QMap iterator

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
   QMap<Key, Val, C> &operator=(const QMap<Key, Val, C> &other) = default;
   QMap<Key, Val, C> &operator=(QMap<Key, Val, C> &&other)      = default;

   bool operator==(const QMap<Key, Val, C> &other) const {
      return m_data == other.m_data;
   }

   bool operator!=(const QMap<Key, Val, C> &other) const {
      return m_data != other.m_data;
   }

   Val &operator[](const Key &key);

   Val operator[](const Key &key) const {
      return value(key);
   }

 private:
   std::map<Key, Val, C> m_data;
};

// methods

template <class Key, class Val, class C>
const Key QMap<Key, Val, C>::key(const Val &value, const Key &defaultKey) const
{
   const_iterator iter = begin();

   while (iter != end()) {
      if (iter.value() == value) {
         return iter.key();
      }

      ++iter;
   }

   return defaultKey;
}

template <class Key, class Val, class C>
QList<Key> QMap<Key, Val, C>::keys() const
{
   QList<Key> retval;
   const_iterator iter = begin();

   while (iter != end()) {
      retval.append(iter.key());
      ++iter;
   }

   return retval;
}

template <class Key, class Val, class C>
QList<Key> QMap<Key, Val, C>::keys(const Val &value) const
{
   QList<Key> retval;
   const_iterator iter = begin();

   while (iter != end()) {
      if (iter.value() == value) {
         retval.append(iter.key());
      }

      ++iter;
   }

   return retval;
}

template <class Key, class Val, class C>
QList<Key> QMap<Key, Val, C>::uniqueKeys() const
{
   QList<Key> retval;
   retval.reserve(size());

   for (const auto &item : m_data) {

      if (! retval.empty() && retval.last() == item.first) {
         continue;
      }

      retval.append(item.first);
   }

   return retval;
}

template <class Key, class Val, class C>
Val QMap<Key, Val, C>::value(const Key &key) const
{
   auto iter = m_data.find(key);

   if (iter == m_data.end()) {
      // key was not found
      return Val();
   }

   return iter->second;
}

template <class Key, class Val, class C>
Val QMap<Key, Val, C>::value(const Key &key, const Val &defaultValue) const
{
   auto iter = m_data.find(key);

   if (iter == m_data.end()) {
      // key was not found
      return defaultValue;
   }

   return iter->second;
}

template <class Key, class Val, class C>
QList<Val> QMap<Key, Val, C>::values() const
{
   QList<Val> retval;
   const_iterator iter = begin();

   while (iter != end()) {
      retval.append(iter.value());
      ++iter;
   }

   return retval;
}

// operators

template <class Key, class Val, class C>
Val &QMap<Key, Val, C>::operator[](const Key &key)
{
   auto range = m_data.equal_range(key);

   if (range.first == range.second) {
      // default constructed element, emplace returns an std::pair where first is the iterator
      auto iter = m_data.emplace(key, Val()).first;

      return iter->second;
   }

   // get last key in the range
   auto iter = --range.second;

   return iter->second;
}

// to from

template <class Key, class Val, class C>
std::map<Key, Val, C> QMap<Key, Val, C>::toStdMap() const
{
   std::map<Key, Val, C> map;
   const_iterator iter = end();

   while (iter != begin()) {
      --iter;
      map.insert(std::pair<Key, Val>(iter.key(), iter.value()));
   }

   return map;
}

// java style iterators

template <class Key, class Val, class C = qMapCompare<Key>>
class QMapIterator
{
   using const_iterator = typename QMap<Key, Val, C>::const_iterator;
   using Item           = const_iterator;

 public:
   QMapIterator(const QMap<Key, Val, C> &map)
      : c(&map), i(c->constBegin()), n(c->constEnd()) {}

   ~QMapIterator() {
   }

   QMapIterator &operator=(const QMap<Key, Val, C> &map) {
      c = map;
      i = c->constBegin();
      n = c->constEnd();

      return *this;
   }

   void toFront() {
      i = c->constBegin();
      n = c->constEnd();
   }

   void toBack() {
      i = c->constEnd();
      n = c->constEnd();
   }

   bool hasNext() const {
      return i != c->constEnd();
   }

   Item next() {
      n = i++;
      return n;
   }

   Item peekNext() const {
      return i;
   }

   bool hasPrevious() const {
      return i != c->constBegin();
   }

   Item previous() {
      n = --i;
      return n;
   }

   Item peekPrevious() const {
      const_iterator p = i;
      return --p;
   }

   const Val &value() const {
      Q_ASSERT(item_exists());
      return *n;
   }

   const Key &key() const {
      Q_ASSERT(item_exists());
      return n.key();
   }

   bool findNext(const Val &value) {
      while ((n = i) != c->constEnd()) {
         if (*i++ == value) {
            return true;
         }
      }

      return false;
   }

   bool findPrevious(const Val &value) {
      while (i != c->constBegin()) {
         if (*(n = --i) == value) {
            return true;
         }
      }

      n = c->constEnd();
      return false;
   }

 private:
   const QMap<Key, Val, C> *c;
   const_iterator i;
   const_iterator n;

   bool item_exists() const {
      return n != c->constEnd();
   }
};

template <class Key, class Val, class C = qMapCompare<Key>>
class QMutableMapIterator
{
   using iterator       = typename QMap<Key, Val, C>::iterator;
   using const_iterator = typename QMap<Key, Val, C>::const_iterator;
   using Item           = iterator;

 public:
   QMutableMapIterator(QMap<Key, Val, C> &map)
      : c(&map), i(c->begin()), n(c->end()) {}

   ~QMutableMapIterator() {
   }

   QMutableMapIterator &operator=(QMap<Key, Val, C> &map) {
      c = &map;
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

   Item next() {
      n = i++;
      return n;
   }

   Item peekNext() const {
      return i;
   }

   bool hasPrevious() const {
      return c->constBegin() != const_iterator(i);
   }

   Item previous() {
      n = --i;
      return n;
   }

   Item peekPrevious() const {
      iterator p = i;
      return --p;
   }

   void remove() {
      if (c->constEnd() != const_iterator(n)) {
         i = c->erase(n);
         n = c->end();
      }
   }

   void setValue(const Val &value) const {
      if (c->constEnd() != const_iterator(n)) {
         *n = value;
      }
   }

   Val &value() {
      Q_ASSERT(item_exists());
      return *n;
   }

   const Val &value() const {
      Q_ASSERT(item_exists());
      return *n;
   }

   const Key &key() const {
      Q_ASSERT(item_exists());
      return n.key();
   }

   bool findNext(const Val &value) {
      while (c->constEnd() != const_iterator(n = i)) {
         if (*i++ == value)  {
            return true;
         }
      }

      return false;
   }

   bool findPrevious(const Val &value) {
      while (c->constBegin() != const_iterator(i)) {
         if (*(n = --i) == value) {
            return true;
         }
      }

      n = c->end();
      return false;
   }

 private:
   QMap<Key, Val, C> *c;
   iterator i;
   iterator n;

   bool item_exists() const {
      return const_iterator(n) != c->constEnd();
   }
};

#endif
