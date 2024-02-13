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

#ifndef QFLATMAP_H
#define QFLATMAP_H

#include <qcontainerfwd.h>
#include <qlist.h>
#include <qmap.h>
#include <qmapfunc.h>
#include <qpair.h>

#include <initializer_list>
#include <vector>

template <typename Key, typename Val, typename C>
class QFlatMapIterator;

template <typename Key, typename Val, typename C>
class QMutableFlatMapIterator;

template <typename Key, typename Val, typename C>
class QFlatMap
{
 public:
   class iterator
   {

    public:
      using iterator_category = std::bidirectional_iterator_tag;

      using pointer           = Val *;
      using reference         = Val &;

      using difference_type   = typename std::vector<std::pair<Key, Val>>::difference_type;
      using size_type         = typename std::vector<std::pair<Key, Val>>::difference_type;
      using value_type        = Val;

      iterator() = default;

      iterator(typename std::vector<std::pair<Key, Val>>::iterator iter)
         : m_iter(std::move(iter)) {
      }

      const Key &key() const {
         return m_iter->first;
      }

      Val &value() const {
         return m_iter->second;
      }

      std::pair<Key, Val> &pair() const {
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

      friend class QFlatMap<Key, Val, C>;

    private:
      typename std::vector<std::pair<Key, Val>>::iterator m_iter;
   };

   class const_iterator
   {

    public:
      using iterator_category = std::bidirectional_iterator_tag;

      using pointer         = const Val *;
      using reference       = const Val &;

      using difference_type = typename std::vector<std::pair<Key, Val>>::difference_type;
      using size_type       = typename std::vector<std::pair<Key, Val>>::difference_type;
      using value_type      = Val;

      const_iterator() = default;

      const_iterator(typename std::vector<std::pair<Key, Val>>::const_iterator iter)
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

      const std::pair<Key, Val> &pair() const {
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

      friend class QFlatMap<Key, Val, C>;

    private:
      typename std::vector<std::pair<Key, Val>>::const_iterator m_iter;
   };

   class CompareFilter
   {
    public:
      using Element = std::pair<Key, Val>;

      CompareFilter(const C &compare) : m_compare(compare) {}

      bool operator()(const Element &x, const Key &y) const {
         return m_compare(x.first, y);
      }

      bool operator()(const Key &x, const Element &y) const {
         return m_compare(x, y.first);
      }

      bool operator()(const Key &x, const Key &y) const {
         return m_compare(x, y);
      }

      bool operator()(const Element &x, const Element &y) const {
         return m_compare(x.first, y.first);
      }

    private:
      const C &m_compare;
   };

   using difference_type = typename std::vector<std::pair<Key, Val>>::difference_type;
   using pointer         = Val *;
   using reference       = Val &;
   using size_type       = typename std::vector<std::pair<Key, Val>>::difference_type;   // signed instead of unsigned
   using value_type      = Val;

   using key_type        = typename std::map<Key, Val, C>::key_type;
   using mapped_type     = typename std::map<Key, Val, C>::mapped_type;
   using key_compare     = typename std::map<Key, Val, C>::key_compare;

   using allocator_type  = typename std::vector<std::pair<Key, Val>>::allocator_type;

   // iterator and const_iterator are classes

   using const_pointer   = const Val *;
   using const_reference = const Val &;

   using reverse_iterator       = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   // java
   using Java_Iterator          = QFlatMapIterator<Key, Val, C>;
   using Java_MutableIterator   = QMutableFlatMapIterator<Key, Val, C>;

   QFlatMap() = default;

   QFlatMap(const QFlatMap<Key, Val, C> &other) = default;
   QFlatMap(QFlatMap<Key, Val, C> &&other)      = default;

   QFlatMap(std::initializer_list<std::pair<const Key, Val>> list, const C &compare = C())
      : m_compare(compare) {

      for (auto &item : list) {
         insert(item.first, item.second);
      }
   }

   explicit QFlatMap(C compare)
      : m_compare(compare) {
   }

   explicit QFlatMap(const std::map<Key, Val, C> &other)
      : m_data(other.begin(), other.end()), m_compare(other.key_comp()) {}

   template <typename Input_Iterator>
   QFlatMap(Input_Iterator first, Input_Iterator last, const C &compare = C())
      : m_data(first, last), m_compare(compare) {

      std::sort(m_data.begin(), m_data.end(), CompareFilter{m_compare} );
   }

   QFlatMap(const_iterator first, const_iterator last, const C &compare = C())
      : m_data(first.m_iter, last.m_iter), m_compare(compare) {

      std::sort(m_data.begin(), m_data.end(), CompareFilter{m_compare} );
   }

   ~QFlatMap() = default;

   // methods
   void clear() {
      m_data.clear();
   }

   bool contains(const Key &key) const {
      return std::binary_search(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );
   }

   size_type count(const Key &key) const {
      return contains(key);
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
      return std::equal_range(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );
   }

   QPair<const_iterator, const_iterator> equal_range(const Key &key) const {
      return std::equal_range(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );
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
      auto iter = std::lower_bound(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );

      if (iter == m_data.end() || m_compare(key, iter->first)) {
         iter = m_data.end();
      }

      return iter;
   }

   const_iterator find(const Key &key) const {
      auto iter = std::lower_bound(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );

      if (iter == m_data.end() || m_compare(key, iter->first)) {
         iter = m_data.end();
      }

      return iter;
   }

   const_iterator constFind(const Key &key) const {
      auto iter = std::lower_bound(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );

      if (iter == m_data.end() || m_compare(key, iter->first)) {
         iter = m_data.end();
      }

      return iter;
   }

   iterator insert(const Key &key, const Val &value) {
      auto iter = std::lower_bound(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );

      if (iter == m_data.end() || m_compare(key, iter->first)) {
         // add new element, emplace returns an std::pair, first is the iterator

         return m_data.emplace(iter, key, value);
      }

      // update value
      iter->second = value;

      return iter;
   }

   iterator insert(const_iterator hint, const Key &key, const Val &value) {
      if (hint == end() || m_compare(key, hint.key()))  {

         if (hint == begin()) {
            return m_data.emplace(hint.m_iter, key, value);

         }  else {
            auto previous = hint - 1;

            if (m_compare(previous.key(), key)) {
               return m_data.emplace(hint.m_iter, key, value);
            }
         }
      }

      return insert(key, value);
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
      return std::lower_bound(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );
   }

   const_iterator lowerBound(const Key &key) const  {
      return std::lower_bound(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );
   }

   size_type remove(const Key &key)  {
      auto iter = find(key);

      if (iter != end()) {
         erase(iter);
         return 1;
      }

      return 0;
   }

   size_type size() const {
      // returns unsigned, must convert to signed
      return static_cast<size_type>(m_data.size());
   }

   void swap(QFlatMap<Key, Val, C> &other) {
      qSwap(m_data, other.m_data);
   }

   Val take(const Key &key)  {
      auto iter = find(key);

      if (iter == end()) {
         return Val();
      }

      Val retval = std::move(iter.value());
      erase(iter);

      return retval;
   }

   iterator upperBound(const Key &key)  {
      return std::upper_bound(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );
   }

   const_iterator upperBound(const Key &key) const {
      return std::upper_bound(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );
   }

   QList<Key> uniqueKeys() const;

   QFlatMap<Key, Val, C> &unite(const QFlatMap<Key, Val, C> &other) {

      std::vector<std::pair<Key, Val>> tmp;
      std::set_union(m_data.begin(), m_data.end(), other.m_data.begin(), other.m_data.end(),
            std::back_inserter(tmp), CompareFilter{m_compare} );

      m_data = std::move(tmp);

      return *this;
   }

   const Val value(const Key &key, const Val &defaultValue = Val()) const;

   QList<Val> values() const;

   // iterators
   iterator begin() {
      // m_data.begin() returns std::vector<T>::iterator
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
   QFlatMap<Key, Val, C> &operator=(const QFlatMap<Key, Val, C> &other) = default;
   QFlatMap<Key, Val, C> &operator=(QFlatMap<Key, Val, C> &&other)      = default;

   bool operator==(const QFlatMap<Key, Val, C> &other) const {
      return m_data == other.m_data;
   }

   bool operator!=(const QFlatMap<Key, Val, C> &other) const {
      return m_data != other.m_data;
   }

   Val &operator[](const Key &key);
   const Val operator[](const Key &key) const;

 private:
   std::vector<std::pair<Key, Val>> m_data;
   C m_compare;
};

// methods

template <class Key, class Val, class C>
const Key QFlatMap<Key, Val, C>::key(const Val &value, const Key &defaultKey) const
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
QList<Key> QFlatMap<Key, Val, C>::keys() const
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
QList<Key> QFlatMap<Key, Val, C>::keys(const Val &value) const
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
QList<Key> QFlatMap<Key, Val, C>::uniqueKeys() const
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
const Val QFlatMap<Key, Val, C>::value(const Key &key, const Val &defaultValue) const
{
   auto range = equal_range(key);

   if (range.first == range.second) {
      // key was not found
      return defaultValue;
   }

   // get last key in the range
   auto iter = --range.second;

   return iter.value();
}

template <class Key, class Val, class C>
QList<Val> QFlatMap<Key, Val, C>::values() const
{
   QList<Val> retval;
   retval.reserve(size());

   const_iterator iter = begin();

   while (iter != end()) {
      retval.append(iter.value());
      ++iter;
   }

   return retval;
}

// operators

template <class Key, class Val, class C>
const Val QFlatMap<Key, Val, C>::operator[](const Key &key) const
{
   return value(key);
}

template <class Key, class Val, class C>
Val &QFlatMap<Key, Val, C>::operator[](const Key &key)
{
   auto iter = std::lower_bound(m_data.begin(), m_data.end(), key, CompareFilter{m_compare} );

   if (iter == m_data.end() || m_compare(key, iter->first)) {

      iter = m_data.emplace(iter, key, Val());
   }

   return iter->second;
}

// java style iterators

template <class Key, class Val, class C = qMapCompare<Key>>
class QFlatMapIterator
{
   using const_iterator = typename QFlatMap<Key, Val, C>::const_iterator;
   using Item           = const_iterator;

 public:
   QFlatMapIterator(const QFlatMap<Key, Val, C> &flatmap)
      : c(&flatmap), i(c->constBegin()), n(c->constEnd()) {}

   ~QFlatMapIterator() {
   }

   QFlatMapIterator &operator=(const QFlatMap<Key, Val, C> &flatmap) {
      c = flatmap;
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
   const QFlatMap<Key, Val, C> *c;
   const_iterator i;
   const_iterator n;

   bool item_exists() const {
      return n != c->constEnd();
   }
};

template <class Key, class Val, class C = qMapCompare<Key>>
class QMutableFlatMapIterator
{
   using iterator       = typename QFlatMap<Key, Val, C>::iterator;
   using const_iterator = typename QFlatMap<Key, Val, C>::const_iterator;
   using Item           = iterator;

 public:
   QMutableFlatMapIterator(QFlatMap<Key, Val, C> &flatmap)
      : c(&flatmap), i(c->begin()), n(c->end())
   {
   }

   ~QMutableFlatMapIterator()
   {
   }

   QMutableFlatMapIterator &operator=(QFlatMap<Key, Val, C> &flatmap) {
      c = &flatmap;
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
   QFlatMap<Key, Val, C> *c;
   iterator i;
   iterator n;

   bool item_exists() const {
      return const_iterator(n) != c->constEnd();
   }
};

#endif
