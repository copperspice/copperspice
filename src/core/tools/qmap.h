/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QMAP_H
#define QMAP_H

#include <qcontainerfwd.h>

#include <qlist.h>
#include <qrefcount.h>

#include <map>
#include <initializer_list>

template <typename Key, typename Val, typename C>
class QMapIterator;

template <typename Key, typename Val, typename C>
class QMutableMapIterator;

template <typename Key> inline bool qMapLessThanKey(const Key &key1, const Key &key2)
{
   return key1 < key2;
}

template <typename Ptr> inline bool qMapLessThanKey(Ptr *key1, Ptr *key2)
{
   static_assert(sizeof(quintptr) == sizeof(Ptr *), "qMapLessThanKey: quintptr is not large enough to contain a ptr");
   return quintptr(key1) < quintptr(key2);
}

template <typename Ptr> inline bool qMapLessThanKey(const Ptr *key1, const Ptr *key2)
{
   static_assert(sizeof(quintptr) == sizeof(const Ptr *), "qMapLessThanKey: quintptr is not large enough to contain a ptr");
   return quintptr(key1) < quintptr(key2);
}

template <typename Key>
class qMapCompare
{
 public:
   bool operator()(const Key &a, const Key &b)  const {
      return qMapLessThanKey(a, b);
   }
};

template <typename Key, typename Val, typename C>
class QMap
{
 public:
   class iterator : private std::multimap<Key, Val, C>::iterator
   {
    public:
      using iterator_category = std::bidirectional_iterator_tag;

      using pointer           = Val *;
      using reference         = Val &;

      using difference_type   = typename std::multimap<Key, Val, C>::difference_type;
      using size_type         = typename std::multimap<Key, Val, C>::difference_type;
      using value_type        = Val;

      iterator() = default;

      iterator(typename std::multimap<Key, Val, C>::iterator iter)
         : std::multimap<Key, Val, C>::iterator(std::move(iter)) {
      }

      const Key &key() const {
         const Key &retval = std::multimap<Key, Val, C>::iterator::operator*().first;
         return retval;
      }

      Val &value() const {
         Val &retval = std::multimap<Key, Val, C>::iterator::operator*().second;
         return retval;
      }

      std::pair<const Key, Val> &pair() const {
         return std::multimap<Key, Val, C>::iterator::operator*();
      }

      // operators
      Val &operator*() const {
         return value();
      }

      Val *operator->() const {
         return &value();
      }

      bool operator==(iterator other) const {
         return std::multimap<Key, Val, C>::iterator::operator==(other);
      }

      bool operator!=(iterator other) const {
         return std::multimap<Key, Val, C>::iterator::operator!=(other);
      }

      iterator &operator+=(size_type n) {
         std::advance(*this, n);
         return *this;
      }

      iterator &operator-=(size_type n) {
         std::advance(*this, -n);
         return *this;
      }

      iterator &operator+(size_type n) {
         std::advance(*this, n);
         return *this;
      }

      iterator &operator-(size_type n) {
         std::advance(*this, -n);
         return *this;
      }

      iterator &operator++() {
         std::multimap<Key, Val, C>::iterator::operator++();
         return *this;
      }

      iterator operator++(int n) {
         return std::multimap<Key, Val, C>::iterator::operator++(n);
      }

      iterator &operator--() {
         std::multimap<Key, Val, C>::iterator::operator--();
         return *this;
      }

      iterator operator--(int n) {
         return std::multimap<Key, Val, C>::iterator::operator--(n);
      }

      friend class QMap<Key, Val, C>;
   };

   class const_iterator : private std::multimap<Key, Val, C>::const_iterator
   {
    public:
      using iterator_category = std::bidirectional_iterator_tag;

      using pointer         = const Val *;
      using reference       = const Val &;

      using difference_type = typename std::multimap<Key, Val, C>::difference_type;
      using size_type       = typename std::multimap<Key, Val, C>::difference_type;
      using value_type      = Val;

      const_iterator() = default;

      const_iterator(typename std::multimap<Key, Val, C>::const_iterator iter)
         : std::multimap<Key, Val, C>::const_iterator(std::move(iter)) {
      }

      const_iterator(iterator iter)
         : std::multimap<Key, Val, C>::const_iterator(std::move(iter)) {
      }

      const Key &key() const {
         const Key &retval = std::multimap<Key, Val, C>::const_iterator::operator*().first;
         return retval;
      }

      const Val &value() const {
         const Val &retval = std::multimap<Key, Val, C>::const_iterator::operator*().second;
         return retval;
      }

      const std::pair<const Key, Val> &pair() const {
         return std::multimap<Key, Val, C>::const_iterator::operator*();
      }

      // operators
      const Val &operator*() const {
         return value();
      }

      const Val *operator->() const {
         return &value();
      }

      bool operator==(const_iterator other) const {
         return std::multimap<Key, Val, C>::const_iterator::operator==(other);
      }

      bool operator!=(const_iterator other) const {
         return std::multimap<Key, Val, C>::const_iterator::operator!=(other);
      }

      const_iterator &operator+=(size_type n) {
         std::advance(*this, n);
         return *this;
      }

      const_iterator &operator-=(size_type n) {
         std::advance(*this, -n);
         return *this;
      }

      const_iterator &operator+(size_type n) {
         std::advance(*this, n);
         return *this;
      }

      const_iterator &operator-(size_type n) {
         std::advance(*this, -n);
         return *this;
      }

      const_iterator &operator++() {
         std::multimap<Key, Val, C>::const_iterator::operator++();
         return *this;
      }

      const_iterator operator++(int n) {
         return std::multimap<Key, Val, C>::const_iterator::operator++(n);
      }

      const_iterator &operator--() {
         std::multimap<Key, Val, C>::const_iterator::operator--();
         return *this;
      }

      const_iterator operator--(int n) {
         return std::multimap<Key, Val, C>::const_iterator::operator--(n);
      }

      friend class QMap<Key, Val, C>;
   };

   // legacy
   using Iterator        = iterator;
   using ConstIterator   = const_iterator;

   using const_pointer   = const Val *;
   using const_reference = const Val &;
   using pointer         = Val *;
   using reference       = Val &;

   using difference_type = typename std::multimap<Key, Val, C>::difference_type;
   using size_type       = typename std::multimap<Key, Val, C>::difference_type;   // signed instead of unsigned
   using value_type      = Val;

   using key_type        = typename std::multimap<Key, Val, C>::key_type;
   using mapped_type     = typename std::multimap<Key, Val, C>::mapped_type;

   using key_compare     = typename std::multimap<Key, Val, C>::key_compare;

   // from std
   using allocator_type         = typename std::multimap<Key, Val, C>::allocator_type;
   using reverse_iterator       = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   // java
   using Java_Iterator          = QMapIterator<Key, Val, C>;
   using Java_MutableIterator   = QMutableMapIterator<Key, Val, C>;

   QMap() = default;

   QMap(const QMap<Key, Val, C> &other) = default;
   QMap(QMap<Key, Val, C> &&other)      = default;

   QMap(std::initializer_list<std::pair<Key, Val>> list, const C &compare = C())
      : m_data(list, compare) {}

   explicit QMap(C compare)
      : m_data(compare) {}

   explicit QMap(const std::map<Key, Val, C> &other)
      : m_data(other) {}

   template<typename Input_Iterator>
   QMap(Input_Iterator first, Input_Iterator last, const C &compare = C())
      : m_data(first, last, compare) {}

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

   iterator erase(const_iterator iter) {
      return m_data.erase(iter);
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

   iterator insert(const Key &key, const Val &value) {
      auto range = m_data.equal_range(key);

      if (range.first == range.second) {
         // add new element
         return m_data.emplace(key, value);
      }

      // get last key in the range, update value
      auto iter = --range.second;
      iter->second = value;

      return iter;
   }

   iterator insertMulti(const Key &key, const Val &value)  {
      return m_data.emplace(key, value);
   }

   const Key key(const Val &value) const;
   const Key key(const Val &value, const Key &defaultKey) const;

   QList<Key> keys() const;
   QList<Key> keys(const Val &value) const;

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

   const Val value(const Key &key) const {
      auto range = m_data.equal_range(key);

      if (range.first == range.second) {
         // key was not found
         return Val();
      }

      // get last key in the range
      auto iter = --range.second;

      return iter->second;
   }

   const Val value(const Key &key, const Val &defaultValue) const;
   QList<Val> values() const;
   QList<Val> values(const Key &key) const;

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
   const Val operator[](const Key &key) const;

 private:
   std::multimap<Key, Val, C> m_data;
};

// methods

template <class Key, class Val, class C>
const Key QMap<Key, Val, C>::key(const Val &value) const
{
   return key(value, Key());
}

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
   retval.reserve(size());

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
const Val QMap<Key, Val, C>::value(const Key &key, const Val &defaultValue) const
{
   auto range = m_data.equal_range(key);

   if (range.first == range.second) {
      // key was not found
      return defaultValue;
   }

   // get last key in the range
   auto iter = --range.second;

   return iter->second;
}

template <class Key, class Val, class C>
QList<Val> QMap<Key, Val, C>::values() const
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

template <class Key, class Val, class C>
QList<Val> QMap<Key, Val, C>::values(const Key &key) const
{
   QList<Val> retval;

   auto range = m_data.equal_range(key);

   for (auto iter = range.first; iter != range.second; ++iter) {
      retval.append(iter->second);
   }

   return retval;
}

// operators

template <class Key, class Val, class C>
const Val QMap<Key, Val, C>::operator[](const Key &key) const
{
   return value(key);
}

template <class Key, class Val, class C>
Val &QMap<Key, Val, C>::operator[](const Key &key)
{
   auto range = m_data.equal_range(key);

   if (range.first == range.second) {
      // default constructed element
      auto iter = m_data.emplace(key, Val());

      return iter->second;
   }

   // get last key in the range
   auto iter = --range.second;

   return iter->second;
}


// QMultiMap

template <typename Key, typename Val, typename C>
class QMultiMap : public QMap<Key, Val, C>
{
 public:
   using QMap<Key, Val, C>::contains;
   using QMap<Key, Val, C>::count;
   using QMap<Key, Val, C>::find;
   using QMap<Key, Val, C>::constFind;
   using QMap<Key, Val, C>::remove;

   // makes this signed instead of unsigned
   using size_type = typename std::multimap<Key, Val, C>::difference_type;

   QMultiMap() = default;

   QMultiMap(const QMap<Key, Val, C> &other)
      : QMap<Key, Val, C>(other) {}

   QMultiMap(std::initializer_list<std::pair<Key, Val>> list) {
      this->reserve(list.size());

      for (typename std::initializer_list<std::pair<Key, Val>>::const_iterator it = list.begin(); it != list.end(); ++it) {
         insert(it->first, it->second);
      }
   }

   // methods
   bool contains(const Key &key, const Val &value)   const;
   size_type count(const Key &key, const Val &value) const;

   typename QMap<Key, Val, C>::const_iterator find(const Key &key, const Val &value) const {
      typename QMap<Key, Val, C>::const_iterator iter(constFind(key));
      typename QMap<Key, Val, C>::const_iterator end(QMap<Key, Val, C>::constEnd());

      while (iter != end && ! this->m_compare(key, iter.key())) {
         if (iter.value() == value) {
            return iter;
         }

         ++iter;
      }

      return end;
   }

   typename QMap<Key, Val, C>::iterator find(const Key &key, const Val &value) {
      typename QMap<Key, Val, C>::iterator iter(find(key));
      typename QMap<Key, Val, C>::iterator end(this->end());

      while (iter != end && ! this->m_compare(key, iter.key())) {
         if (iter.value() == value) {
            return iter;
         }

         ++iter;
      }

      return end;
   }

   typename QMap<Key, Val, C>::const_iterator constFind(const Key &key, const Val &value) const {
      return find(key, value);
   }

   typename QMap<Key, Val, C>::iterator insert(const Key &key, const Val &value) {
      return QMap<Key, Val, C>::insertMulti(key, value);
   }

   void swap(QMultiMap<Key, Val, C> &other) {
      QMap<Key, Val, C>::swap(other);
   }

   size_type remove(const Key &key, const Val &value);

   typename QMap<Key, Val, C>::iterator replace(const Key &key, const Val &value) {
      return QMap<Key, Val, C>::insert(key, value);
   }

   // operators
   QMultiMap &operator+=(const QMultiMap &other) {
      this->unite(other);
      return *this;
   }

   QMultiMap operator+(const QMultiMap &other) const {
      QMultiMap result = *this;
      result += other;
      return result;
   }

 private:
   Val &operator[](const Key &key);
   const Val operator[](const Key &key) const;
};


template <class Key, class Val, class C>
bool QMultiMap<Key, Val, C>::contains(const Key &key, const Val &value) const
{
   return constFind(key, value) != QMap<Key, Val, C>::constEnd();
}

template <class Key, class Val, class C>
typename QMultiMap<Key, Val, C>::size_type QMultiMap<Key, Val, C>::count(const Key &key, const Val &value) const
{
   int retval = 0;

   typename QMap<Key, Val, C>::const_iterator iter(constFind(key));
   typename QMap<Key, Val, C>::const_iterator end(QMap<Key, Val, C>::constEnd());

   while (iter != end && ! this->m_compare(key, iter.key())) {
      if (iter.value() == value) {
         ++retval;
      }

      ++iter;
   }

   return retval;
}

template <class Key, class Val, class C>
typename QMultiMap<Key, Val, C>::size_type QMultiMap<Key, Val, C>::remove(const Key &key, const Val &value)
{
   int retval = 0;

   typename QMap<Key, Val, C>::iterator iter(find(key));
   typename QMap<Key, Val, C>::iterator end(QMap<Key, Val, C>::end());

   while (iter != end && ! this->m_compare(key, iter.key())) {
      if (iter.value() == value) {
         iter = this->erase(iter);
         ++retval;
      } else {
         ++iter;
      }
   }

   return retval;
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
   typedef typename QMap<Key, Val, C>::const_iterator const_iterator;
   typedef const_iterator Item;

   QMap<Key, Val, C> c;

   const_iterator i;
   const_iterator n;

   inline bool item_exists() const {
      return n != c.constEnd();
   }

 public:
   inline QMapIterator(const QMap<Key, Val, C> &container)
      : c(container), i(c.constBegin()), n(c.constEnd()) {}

   inline QMapIterator &operator=(const QMap<Key, Val, C> &container) {
      c = container;
      i = c.constBegin();
      n = c.constEnd();
      return *this;
   }

   inline void toFront() {
      i = c.constBegin();
      n = c.constEnd();
   }
   inline void toBack() {
      i = c.constEnd();
      n = c.constEnd();
   }
   inline bool hasNext() const {
      return i != c.constEnd();
   }
   inline Item next() {
      n = i++;
      return n;
   }
   inline Item peekNext() const {
      return i;
   }
   inline bool hasPrevious() const {
      return i != c.constBegin();
   }
   inline Item previous() {
      n = --i;
      return n;
   }
   inline Item peekPrevious() const {
      const_iterator p = i;
      return --p;
   }
   inline const Val &value() const {
      Q_ASSERT(item_exists());
      return *n;
   }
   inline const Key &key() const {
      Q_ASSERT(item_exists());
      return n.key();
   }

   inline bool findNext(const Val &t) {
      while ((n = i) != c.constEnd()) if (*i++ == t) {
            return true;
         }
      return false;
   }

   inline bool findPrevious(const Val &t) {
      while (i != c.constBegin()) if (*(n = --i) == t) {
            return true;
         }
      n = c.constEnd();
      return false;
   }
};

template <class Key, class Val, class C = qMapCompare<Key>>
class QMutableMapIterator
{
   typedef typename QMap<Key, Val, C>::iterator iterator;
   typedef typename QMap<Key, Val, C>::const_iterator const_iterator;
   typedef iterator Item;

   QMap<Key, Val, C> *c;

   iterator i, n;
   bool item_exists() const {
      return const_iterator(n) != c->constEnd();
   }

 public:
   inline QMutableMapIterator(QMap<Key, Val, C> &container)
      : c(&container) {
      i = c->begin();
      n = c->end();
   }

   inline ~QMutableMapIterator() {
   }

   inline QMutableMapIterator &operator=(QMap<Key, Val, C> &container) {
      c = &container;
      i = c->begin();
      n = c->end();

      return *this;
   }

   inline void toFront() {
      i = c->begin();
      n = c->end();
   }
   inline void toBack() {
      i = c->end();
      n = i;
   }
   inline bool hasNext() const {
      return c->constEnd() != const_iterator(i);
   }
   inline Item next() {
      n = i++;
      return n;
   }
   inline Item peekNext() const {
      return i;
   }
   inline bool hasPrevious() const {
      return c->constBegin() != const_iterator(i);
   }
   inline Item previous() {
      n = --i;
      return n;
   }
   inline Item peekPrevious() const {
      iterator p = i;
      return --p;
   }

   inline void remove() {
      if (c->constEnd() != const_iterator(n)) {
         i = c->erase(n);
         n = c->end();
      }
   }

   inline void setValue(const Val &t) const {
      if (c->constEnd() != const_iterator(n)) {
         *n = t;
      }
   }
   inline Val &value() {
      Q_ASSERT(item_exists());
      return *n;
   }
   inline const Val &value() const {
      Q_ASSERT(item_exists());
      return *n;
   }
   inline const Key &key() const {
      Q_ASSERT(item_exists());
      return n.key();
   }

   inline bool findNext(const Val &t) {
      while (c->constEnd() != const_iterator(n = i)) {
         if (*i++ == t)  {
            return true;
         }
      }

      return false;
   }

   inline bool findPrevious(const Val &t) {
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
