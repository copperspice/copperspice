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

#ifndef QHASH_H
#define QHASH_H

#include <qcontainerfwd.h>
#include <qhashfunc.h>

#include <qhashfwd.h>
#include <qlist.h>
#include <qrefcount.h>

#include <unordered_map>
#include <initializer_list>

template <typename Key, typename Val, typename Hash = qHashFunc<Key>, typename KeyEqual = qHashEqual<Key>>
class QHashIterator;

template <typename Key, typename Val, typename Hash = qHashFunc<Key>, typename KeyEqual = qHashEqual<Key>>
class QMutableHashIterator;

// **
template <typename Key, typename Val, typename Hash, typename KeyEqual>
class QHash
{
 public:

   class iterator : private std::unordered_map<Key, Val, Hash, KeyEqual>::iterator
   {
    public:
      using iterator_category = std::forward_iterator_tag;

      using pointer           = Val *;
      using reference         = Val &;

      using difference_type   = typename std::unordered_map<Key, Val, Hash, KeyEqual>::difference_type;
      using size_type         = typename std::unordered_map<Key, Val, Hash, KeyEqual>::difference_type;
      using value_type        = Val;

      iterator() = default;

      iterator(typename std::unordered_map<Key, Val, Hash, KeyEqual>::iterator iter)
         : std::unordered_map<Key, Val, Hash, KeyEqual>::iterator(std::move(iter)) {
      }

      const Key &key() const {
         const Key &retval = std::unordered_map<Key, Val, Hash, KeyEqual>::iterator::operator*().first;
         return retval;
      }

      Val &value() const {
         Val &retval = std::unordered_map<Key, Val, Hash, KeyEqual>::iterator::operator*().second;
         return retval;
      }

      std::pair<const Key, Val> &pair() const {
         return std::unordered_map<Key, Val, Hash, KeyEqual>::iterator::operator*();
      }

      // operators
      Val &operator*() const {
         return value();
      }

      Val *operator->() const {
         return &value();
      }

      bool operator==(iterator other) const {
         return static_cast<typename std::unordered_map<Key, Val, Hash, KeyEqual>::iterator>(*this) == other;
      }

      bool operator!=(iterator other) const {
         return static_cast<typename std::unordered_map<Key, Val, Hash, KeyEqual>::iterator>(*this) != other;
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
         std::unordered_map<Key, Val, Hash, KeyEqual>::iterator::operator++();
         return *this;
      }

      iterator operator++(int n) {
         return std::unordered_map<Key, Val, Hash, KeyEqual>::iterator::operator++(n);
      }

      friend class QHash<Key, Val, Hash, KeyEqual>;
   };

   class const_iterator : private std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator
   {
    public:
      using iterator_category = std::forward_iterator_tag;

      using pointer           = const Val *;
      using reference         = const Val &;

      using difference_type   = typename std::unordered_map<Key, Val, Hash, KeyEqual>::difference_type;
      using size_type         = typename std::unordered_map<Key, Val, Hash, KeyEqual>::difference_type;
      using value_type        = Val;

      const_iterator() = default;

      const_iterator(typename std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator iter)
         : std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator(std::move(iter)) {
      }

      const_iterator(iterator iter)
         : std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator(std::move(iter)) {
      }

      const Key &key() const {
         const Key &retval = std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator::operator*().first;
         return retval;
      }

      const Val &value() const {
         const Val &retval = std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator::operator*().second;
         return retval;
      }

      const std::pair<const Key, Val> &pair() const {
         return std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator::operator*();
      }

      // operators
      const Val &operator*() const {
         return value();
      }

      const Val *operator->() const {
         return &value();
      }

      bool operator==(const iterator &other) const {
         return static_cast<typename std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator>(*this) == other;
      }

      bool operator==(const const_iterator &other) const {
         return static_cast<typename std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator>(*this) == other;
      }

      bool operator!=(const iterator & other) const {
         return static_cast<typename std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator>(*this) != other;
      }

      bool operator!=(const const_iterator &other) const {
         return static_cast<typename std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator>(*this) != other;
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
         std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator::operator++();
         return *this;
      }

      const_iterator operator++(int n) {
         return std::unordered_map<Key, Val, Hash, KeyEqual>::const_iterator::operator++(n);
      }

      friend class QHash<Key, Val, Hash, KeyEqual>;
   };

   // legacy
   using Iterator        = iterator;
   using ConstIterator   = const_iterator;

   using const_pointer   = const Val *;
   using const_reference = const Val &;
   using pointer         = Val *;
   using reference       = Val &;

   using difference_type = typename std::unordered_map<Key, Val, Hash, KeyEqual>::difference_type;
   using size_type       = typename std::unordered_map<Key, Val, Hash, KeyEqual>::difference_type;
   using value_type      = Val;

   using key_type        = typename std::unordered_map<Key, Val, Hash, KeyEqual>::key_type;
   using mapped_type     = typename std::unordered_map<Key, Val, Hash, KeyEqual>::mapped_type;

   using hasher          = typename std::unordered_map<Key, Val, Hash, KeyEqual>::hasher;
   using key_equal       = typename std::unordered_map<Key, Val, Hash, KeyEqual>::key_equal;

   // from std
   using allocator_type         = typename std::unordered_map<Key, Val, Hash, KeyEqual>::allocator_type;

   // java
   using Java_Iterator          = QHashIterator<Key, Val, Hash, KeyEqual>;
   using Java_MutableIterator   = QMutableHashIterator<Key, Val, Hash, KeyEqual>;

   QHash() = default;

   QHash(const QHash<Key, Val, Hash, KeyEqual> &other) = default;
   QHash(QHash<Key, Val, Hash, KeyEqual> &&other) = default;

   QHash(std::initializer_list<std::pair<Key, Val> > list, const Hash & hash = Hash(), const KeyEqual &key_equal = KeyEqual())
      : m_data(list, hash, key_equal) {}

   explicit QHash(const Hash & hash, const KeyEqual &key_equal = KeyEqual())
      : m_data(hash, key_equal) {}

   explicit QHash(const std::unordered_map<Key, Val, Hash, KeyEqual> &other)
       : m_data(other) {}

   explicit QHash(std::unordered_map<Key, Val, Hash, KeyEqual> &&other)
       : m_data(std::move(other)) {}

   template<typename Input_Iterator>
   QHash(Input_Iterator first, Input_Iterator last, const Hash & hash = Hash(), const KeyEqual &key_equal = KeyEqual())
      : m_data(first, last, hash, key_equal) {}

   ~QHash() = default;

   // methods
   void clear() {
      m_data.clear();
   }

   size_type capacity() const {
      return m_data.bucket_count();
   }

   bool contains(const Key &key) const {
      auto iter = m_data.find(key);

      if (iter == m_data.end()) {
         return false;
      }

      return true;
   }

   size_type count(const Key &key) const {
      return m_data.count(key);
   }

   size_type count() const {
      return m_data.size();
   }

   bool empty() const {
      return isEmpty();
   }

   bool isEmpty() const {
      return m_data.empty();
   }

   iterator erase(const_iterator iter) {
      return m_data.erase(iter);
   }

   iterator find(const Key &key) {
      // find returns an std::map::iterator, default iterator constructor will convert to QMap::iterator
      return m_data.find(key);
   }

   const_iterator find(const Key &key) const {
      return m_data.find(key);
   }

   const_iterator constFind(const Key &key) const {
      return m_data.find(key);
   }

   iterator insert(const Key &key, const Val &value) {
      auto iter = m_data.find(key);

      if (iter != m_data.end()) {
         // update key with new value
         iter->second = value;
         return iter;
      }

      return m_data.emplace(key, value).first;
   }

   const Key key(const Val &value) const;
   const Key key(const Val &value, const Key &defaultKey) const;

   QList<Key> keys() const;
   QList<Key> keys(const Val &value) const;

   void reserve(size_type size) {
      m_data.reserve(size);
   }

   size_type remove(const Key &key)  {
      return m_data.erase(key);
   }

   size_type size() const {
      // returns unsigned, must convert to signed
      return static_cast<size_type>(m_data.size());
   }

   void squeeze() {
      m_data.rehash(0);
   }

   void swap(QHash<Key, Val, Hash, KeyEqual> &other) {
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

   QList<Key> uniqueKeys() const;

   QHash<Key, Val, Hash, KeyEqual> &unite(const QHash<Key, Val, Hash, KeyEqual> &other) {
      m_data.insert(other.m_data.begin(), other.m_data.end());
      return *this;
   }

   Val value(const Key &key) const {
      auto iter = m_data.find(key);

      if (iter == m_data.end()) {
         // key was not found
         return Val();
      }

      return iter->second;
   }

   Val value(const Key &key, const Val &defaultValue) const {
      auto iter = m_data.find(key);

      if (iter == m_data.end()) {
         // key was not found
         return defaultValue;
      }

      return iter->second;
   }

   QList<Val> values() const;

   // operators
   QHash<Key, Val, Hash, KeyEqual> &operator=(const QHash<Key, Val, Hash, KeyEqual> &other) = default;
   QHash<Key, Val, Hash, KeyEqual> &operator=(QHash<Key, Val, Hash, KeyEqual> &&other) = default;

   bool operator==(const QHash<Key, Val, Hash, KeyEqual> &other) const {
      return m_data == other.m_data;
   }

   inline bool operator!=(const QHash<Key, Val, Hash, KeyEqual> &other) const {
      return ! (*this == other);
   }

   Val &operator[](const Key &key) {
      return m_data[key];
   }

   const Val operator[](const Key &key) const  {
      return value(key);
   }

   // iterators
   inline iterator begin() {
      return m_data.begin();
   }

   inline const_iterator begin() const {
      return m_data.begin();
   }

   inline const_iterator cbegin() const {
      return m_data.begin();;
   }

   inline const_iterator constBegin() const {
      return m_data.begin();
   }

   inline iterator end() {
      return m_data.end();
   }

   inline const_iterator end() const {
      return m_data.end();
   }

   inline const_iterator cend() const {
      return m_data.end();
   }

   inline const_iterator constEnd() const {
      return m_data.end();
   }

 private:
   std::unordered_map<Key, Val, Hash, KeyEqual> m_data;
};

// methods

template <typename Key, typename Val, typename Hash, typename KeyEqual>
const Key QHash<Key, Val, Hash, KeyEqual>::key(const Val &value) const
{
   return key(value, Key());
}

template <typename Key, typename Val, typename Hash, typename KeyEqual>
const Key QHash<Key, Val, Hash, KeyEqual>::key(const Val &value, const Key &defaultValue) const
{
   const_iterator iter = begin();

   while (iter != end()) {
      if (iter.value() == value) {
         return iter.key();
      }

      ++iter;
   }

   return defaultValue;
}

template <typename Key, typename Val, typename Hash, typename KeyEqual>
QList<Key> QHash<Key, Val, Hash, KeyEqual>::keys() const
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

template <typename Key, typename Val, typename Hash, typename KeyEqual>
QList<Key> QHash<Key, Val, Hash, KeyEqual>::keys(const Val &value) const
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

template <typename Key, typename Val, typename Hash, typename KeyEqual>
QList<Key> QHash<Key, Val, Hash, KeyEqual>::uniqueKeys() const
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

template <typename Key, typename Val, typename Hash, typename KeyEqual>
QList<Val> QHash<Key, Val, Hash, KeyEqual>::values() const
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

// java style iterators

template <typename Key, typename Val, typename Hash, typename KeyEqual>
class QHashIterator
{
   typedef typename QHash<Key, Val, Hash, KeyEqual>::const_iterator const_iterator;
   typedef const_iterator Item;

   const QHash<Key, Val, Hash, KeyEqual> *c;
   const_iterator i;
   const_iterator n;

   inline bool item_exists() const {
      return n != c->constEnd();
   }

 public:
   inline QHashIterator(const QHash<Key, Val, Hash, KeyEqual> &container)
      : c(&container), i(c->constBegin()), n(c->constEnd()) {}

   inline ~QHashIterator() {
   }

   inline QHashIterator &operator=(const QHash<Key, Val, Hash, KeyEqual> &container) {
      c = container;
      i = c->constBegin();
      n = c->constEnd();

      return *this;
   }

   inline void toFront() {
      i = c->constBegin();
      n = c->constEnd();
   }

   inline void toBack() {
      i = c->constEnd();
      n = c->constEnd();
   }

   inline bool hasNext() const {
      return i != c->constEnd();
   }

   inline Item next() {
      n = i++;
      return n;
   }

   inline Item peekNext() const {
      return i;
   }

   inline bool hasPrevious() const {
      return i != c->constBegin();
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
      while ((n = i) != c->constEnd()) {
         if (*i++ == t) {
            return true;
         }
      }

      return false;
   }

   inline bool findPrevious(const Val &t) {
      while (i != c->constBegin()) {
         if (*(n = --i) == t) {
            return true;
         }
      }

      n = c->constEnd();
      return false;
   }
};

template <typename Key, typename Val, typename Hash, typename KeyEqual>
class QMutableHashIterator
{
   typedef typename QHash<Key, Val, Hash, KeyEqual>::iterator iterator;
   typedef typename QHash<Key, Val, Hash, KeyEqual>::const_iterator const_iterator;
   typedef iterator Item;

   QHash<Key, Val, Hash, KeyEqual> *c;
   iterator i;
   iterator n;

   bool item_exists() const {
      return const_iterator(n) != c->constEnd();
   }

 public:
   inline QMutableHashIterator(QHash<Key, Val, Hash, KeyEqual> &container)
      : c(&container), i(c->begin()), n(c->end()) {}

   inline ~QMutableHashIterator() {
   }

   inline QMutableHashIterator &operator=(QHash<Key, Val, Hash, KeyEqual> &container) {
      c = & container;
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
      n = c->end();
   }

   inline bool hasNext() const {
      return const_iterator(i) != c->constEnd();
   }

   inline Item next() {
      n = i++;
      return n;
   }

   inline Item peekNext() const {
      return i;
   }

   inline bool hasPrevious() const {
      return const_iterator(i) != c->constBegin();
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
      if (const_iterator(n) != c->constEnd()) {
         i = c->erase(n);
         n = c->end();
      }
   }

   inline void setValue(const Val &t) {
      if (const_iterator(n) != c->constEnd()) {
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
      while (const_iterator(n = i) != c->constEnd()) {
         if (*i++ == t) {
            return true;
         }
      }

      return false;
   }

   inline bool findPrevious(const Val &t) {
      while (const_iterator(i) != c->constBegin()) {
         if (*(n = --i) == t) {
            return true;
         }
      }

      n = c->end();
      return false;
   }
};

#endif
