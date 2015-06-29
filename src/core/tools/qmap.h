/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QMAP_H
#define QMAP_H

#include <QtCore/qlist.h>
#include <QtCore/qrefcount.h>
#include <QtCore/qcontainerfwd.h>

#include <map>
#include <new>

#include <initializer_list>

QT_BEGIN_NAMESPACE

struct Q_CORE_EXPORT QMapData {
   struct Node {
      Node *backward;
      Node *forward[1];
   };
   enum { LastLevel = 11, Sparseness = 3 };

   QMapData *backward;
   QMapData *forward[QMapData::LastLevel + 1];
   QtPrivate::RefCount ref;
   int topLevel;
   int size;
   uint randomBits;
   uint insertInOrder : 1;
   uint sharable : 1;
   uint strictAlignment : 1;
   uint reserved : 29;

   static QMapData *createData(); // ### Qt5/remove me
   static QMapData *createData(int alignment);

   void continueFreeData(int offset);
   Node *node_create(Node *update[], int offset); // ### Qt5/remove me
   Node *node_create(Node *update[], int offset, int alignment);
   void node_delete(Node *update[], int offset, Node *node);

#ifdef QT_QMAP_DEBUG
   uint adjust_ptr(Node *node);
   void dump();
#endif

   static QMapData *sharedNull();
};


/*
    QMap uses qMapLessThanKey() to compare keys. The default
    implementation uses operator<(). For pointer types,
    qMapLessThanKey() casts the pointers to integers before it
    compares them, because operator<() is undefined on pointers
    that come from different memory blocks. (In practice, this
    is only a problem when running a program such as
    BoundsChecker.)
*/

template <class Key> inline bool qMapLessThanKey(const Key &key1, const Key &key2)
{
   return key1 < key2;
}

template <class Ptr> inline bool qMapLessThanKey(Ptr *key1, Ptr *key2)
{
   Q_ASSERT(sizeof(quintptr) == sizeof(Ptr *));
   return quintptr(key1) < quintptr(key2);
}

template <class Ptr> inline bool qMapLessThanKey(const Ptr *key1, const Ptr *key2)
{
   Q_ASSERT(sizeof(quintptr) == sizeof(const Ptr *));
   return quintptr(key1) < quintptr(key2);
}

template <class Key, class T>
struct QMapNode {
   Key key;
   T value;

 private:
   // never access these members through this structure, see below
   QMapData::Node *backward;
   QMapData::Node *forward[1];
};

template <class Key, class T>
struct QMapPayloadNode {
   Key key;
   T value;

 private:
   // QMap::e is a pointer to QMapData::Node, which matches the member
   // below. However, the memory allocation node in QMapData::node_create
   // allocates sizeof(QMapPayloNode) and incorrectly calculates the offset
   // of 'backward' below. If the alignment of QMapPayloadNode is larger
   // than the alignment of a pointer, the 'backward' member is aligned to
   // the end of this structure, not to 'value' above, and will occupy the
   // tail-padding area.
   //
   //  e.g., on a 32-bit archictecture with Key = int and
   //        sizeof(T) = alignof(T) = 8
   //   0        4        8        12       16       20       24  byte
   //   |   key  |   PAD  |      value      |backward|  PAD   |   correct layout
   //   |   key  |   PAD  |      value      |        |backward|   how it's actually used
   //   |<-----  value of QMap::payload() = 20 ----->|
   QMapData::Node *backward;
};

template <class Key>
class qMapCompare
{
   public:
      bool operator()(const Key &a, const Key &b)  const {
         return qMapLessThanKey(a, b);
      }
};

template <class Key, class T, class Compare>
class QMap
{
   typedef QMapNode<Key, T> Node;
   typedef QMapPayloadNode<Key, T> PayloadNode;

   union {
      QMapData *d;
      QMapData::Node *e;
   };

 protected:
   Compare m_compare;

 private:
   static inline int payload() {
      return sizeof(PayloadNode) - sizeof(QMapData::Node *);
   }

   static inline int alignment() {
#ifdef Q_ALIGNOF
      return int(qMax(sizeof(void *), Q_ALIGNOF(Node)));
#else
      return 0;
#endif
   }

   static inline Node *concrete(QMapData::Node *node) {
      return reinterpret_cast<Node *>(reinterpret_cast<char *>(node) - payload());
   }

 public:
   inline QMap()
       : d( QMapData::sharedNull() ) 
   { }

   explicit inline QMap(Compare compare) 
       : d( QMapData::sharedNull() ), m_compare(compare)
   { }

   inline QMap(const QMap<Key, T, Compare> &other) 
      : d(other.d), m_compare(other.m_compare) 
   {
      d->ref.ref();
      if (! d->sharable) {
         detach();
      }
   }

   inline ~QMap() {
      if (! d->ref.deref()) {
         freeData(d);
      }
   }

   inline QMap(std::initializer_list<std::pair<Key, T> > list)
      : d(QMapData::sharedNull() ) 
   {
      for (typename std::initializer_list<std::pair<Key, T> >::const_iterator it = list.begin(); it != list.end(); ++it) {
         insert(it->first, it->second);
      }
   }

   QMap<Key, T, Compare> &operator=(const QMap<Key, T, Compare> &other);

   inline QMap<Key, T, Compare> &operator=(QMap<Key, T, Compare> && other) {
      qSwap(d, other.d);
      m_compare = std::move(other.m_compare);
      return *this;
   }

   inline void swap(QMap<Key, T, Compare> &other) {
      qSwap(d, other.d);
      swap(m_compare, other.m_compare);
   }

   explicit QMap(const typename std::map<Key, T> &other);
   std::map<Key, T> toStdMap() const;

   bool operator==(const QMap<Key, T, Compare> &other) const;

   inline bool operator!=(const QMap<Key, T, Compare> &other) const {
      return !(*this == other);
   }

   inline int size() const {
      return d->size;
   }

   inline bool isEmpty() const {
      return d->size == 0;
   }

   inline void detach() {
      if (d->ref.isShared()) {
         detach_helper();
      }
   }

   inline bool isDetached() const {
      return !d->ref.isShared();
   }

   inline void setSharable(bool sharable) {
      if (!sharable) {
         detach();
      }
      if (d != QMapData::sharedNull() ) {
         d->sharable = sharable;
      }
   }

   inline bool isSharedWith(const QMap<Key, T, Compare> &other) const {
      return d == other.d;
   }

   inline void setInsertInOrder(bool ordered) {
      if (ordered) {
         detach();
      }
      if ( d !=  QMapData::sharedNull() ) {
         d->insertInOrder = ordered;
      }
   }

   void clear();

   int remove(const Key &key);
   T take(const Key &key);

   bool contains(const Key &key) const;
   const Key key(const T &value) const;
   const Key key(const T &value, const Key &defaultKey) const;
   const T value(const Key &key) const;
   const T value(const Key &key, const T &defaultValue) const;
   T &operator[](const Key &key);
   const T operator[](const Key &key) const;

   QList<Key> uniqueKeys() const;
   QList<Key> keys() const;
   QList<Key> keys(const T &value) const;
   QList<T> values() const;
   QList<T> values(const Key &key) const;
   int count(const Key &key) const;

   class const_iterator;

   class iterator
   {
      friend class const_iterator;
      QMapData::Node *i;

    public:
      typedef std::bidirectional_iterator_tag iterator_category;
      typedef qptrdiff difference_type;
      typedef T value_type;
      typedef T *pointer;
      typedef T &reference;

      // ### Qt5/get rid of 'operator Node *'
      inline operator QMapData::Node *() const {
         return i;
      }
      inline iterator() : i(0) { }
      inline iterator(QMapData::Node *node) : i(node) { }

      inline const Key &key() const {
         return concrete(i)->key;
      }
      inline T &value() const {
         return concrete(i)->value;
      }

      inline T &operator*() const {
         return concrete(i)->value;
      }
      inline T *operator->() const {
         return &concrete(i)->value;
      }
      inline bool operator==(const iterator &o) const {
         return i == o.i;
      }
      inline bool operator!=(const iterator &o) const {
         return i != o.i;
      }

      inline iterator &operator++() {
         i = i->forward[0];
         return *this;
      }
      inline iterator operator++(int) {
         iterator r = *this;
         i = i->forward[0];
         return r;
      }
      inline iterator &operator--() {
         i = i->backward;
         return *this;
      }
      inline iterator operator--(int) {
         iterator r = *this;
         i = i->backward;
         return r;
      }
      inline iterator operator+(int j) const {
         iterator r = *this;
         if (j > 0) while (j--) {
               ++r;
            }
         else while (j++) {
               --r;
            }
         return r;
      }
      inline iterator operator-(int j) const {
         return operator+(-j);
      }
      inline iterator &operator+=(int j) {
         return *this = *this + j;
      }
      inline iterator &operator-=(int j) {
         return *this = *this - j;
      }

      // ### Qt5/not sure this is necessary anymore
#ifdef QT_STRICT_ITERATORS
    private:
#else
    public:
#endif
      inline bool operator==(const const_iterator &o) const {
         return i == o.i;
      }
      inline bool operator!=(const const_iterator &o) const {
         return i != o.i;
      }

    private:
      // ### Qt5/remove
      inline operator bool() const {
         return false;
      }
   };
   friend class iterator;

   class const_iterator
   {
      friend class iterator;
      QMapData::Node *i;

    public:
      typedef std::bidirectional_iterator_tag iterator_category;
      typedef qptrdiff difference_type;
      typedef T value_type;
      typedef const T *pointer;
      typedef const T &reference;

      // ### Qt5/get rid of 'operator Node *'
      inline operator QMapData::Node *() const {
         return i;
      }
      inline const_iterator() : i(0) { }
      inline const_iterator(QMapData::Node *node) : i(node) { }

#ifdef QT_STRICT_ITERATORS
      explicit inline const_iterator(const iterator &o)
#else
      inline const_iterator(const iterator &o)
#endif
      {
         i = o.i;
      }

      inline const Key &key() const {
         return concrete(i)->key;
      }
      inline const T &value() const {
         return concrete(i)->value;
      }

      inline const T &operator*() const {
         return concrete(i)->value;
      }
      inline const T *operator->() const {
         return &concrete(i)->value;
      }
      inline bool operator==(const const_iterator &o) const {
         return i == o.i;
      }
      inline bool operator!=(const const_iterator &o) const {
         return i != o.i;
      }

      inline const_iterator &operator++() {
         i = i->forward[0];
         return *this;
      }
      inline const_iterator operator++(int) {
         const_iterator r = *this;
         i = i->forward[0];
         return r;
      }
      inline const_iterator &operator--() {
         i = i->backward;
         return *this;
      }
      inline const_iterator operator--(int) {
         const_iterator r = *this;
         i = i->backward;
         return r;
      }
      inline const_iterator operator+(int j) const {
         const_iterator r = *this;
         if (j > 0) while (j--) {
               ++r;
            }
         else while (j++) {
               --r;
            }
         return r;
      }
      inline const_iterator operator-(int j) const {
         return operator+(-j);
      }
      inline const_iterator &operator+=(int j) {
         return *this = *this + j;
      }
      inline const_iterator &operator-=(int j) {
         return *this = *this - j;
      }

      // ### Qt5/not sure this is necessary anymore
#ifdef QT_STRICT_ITERATORS
    private:
      inline bool operator==(const iterator &o) {
         return operator==(const_iterator(o));
      }
      inline bool operator!=(const iterator &o) {
         return operator!=(const_iterator(o));
      }
#endif

    private:
      // ### Qt5/remove
      inline operator bool() const {
         return false;
      }
   };
   friend class const_iterator;

   // STL style
   inline iterator begin() {
      detach();
      return iterator(e->forward[0]);
   }

   inline const_iterator begin() const {
      return const_iterator(e->forward[0]);
   }

   inline const_iterator cbegin() const {
      return const_iterator(e->forward[0]);
   }

   inline const_iterator constBegin() const {
      return const_iterator(e->forward[0]);
   }

   inline iterator end() {
      detach();
      return iterator(e);
   }

   inline const_iterator end() const {
      return const_iterator(e);
   }

   inline const_iterator cend() const {
      return const_iterator(e);
   }

   inline const_iterator constEnd() const {
      return const_iterator(e);
   }
   iterator erase(iterator it);

   // more Qt
   typedef iterator Iterator;
   typedef const_iterator ConstIterator;

   inline int count() const {
      return d->size;
   }

   iterator find(const Key &key);

   const_iterator find(const Key &key) const;
   const_iterator constFind(const Key &key) const;

   iterator lowerBound(const Key &key);
   const_iterator lowerBound(const Key &key) const;

   iterator upperBound(const Key &key);
   const_iterator upperBound(const Key &key) const;

   iterator insert(const Key &key, const T &value);
   iterator insertMulti(const Key &key, const T &value);

   QMap<Key, T, Compare> &unite(const QMap<Key, T, Compare> &other);

   // STL compatibility
   typedef Key key_type;
   typedef T mapped_type;
   typedef qptrdiff difference_type;
   typedef int size_type;
   inline bool empty() const {
      return isEmpty();
   }

#ifdef QT_QMAP_DEBUG
   inline void dump() const {
      d->dump();
   }
#endif

 private:
   void detach_helper();
   void freeData(QMapData *d);
   QMapData::Node *findNode(const Key &key) const;
   QMapData::Node *mutableFindNode(QMapData::Node *update[], const Key &key) const;
   QMapData::Node *node_create(QMapData *d, QMapData::Node *update[], const Key &key, const T &value);
};

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE QMap<Key, T, Compare> &QMap<Key, T, Compare>::operator=(const QMap<Key, T, Compare> &other)
{
   if (d != other.d) {
      QMapData *o = other.d;
      o->ref.ref();

      if (!d->ref.deref()) {
         freeData(d);
      }

      d = o;

      if (!d->sharable) {
         detach_helper();
      }
   }

   m_compare = other.m_compare;   

   return *this;
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE void QMap<Key, T, Compare>::clear()
{
   *this = QMap<Key, T, Compare>(m_compare);
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMapData::Node *
QMap<Key, T, Compare>::node_create(QMapData *adt, QMapData::Node *aupdate[], const Key &akey, const T &avalue)
{
   QMapData::Node *abstractNode = adt->node_create(aupdate, payload(), alignment());
   QT_TRY {
      Node *concreteNode = concrete(abstractNode);
      new (&concreteNode->key) Key(akey);
      QT_TRY {
         new (&concreteNode->value) T(avalue);
      } QT_CATCH(...)
      {
         concreteNode->key.~Key();
         QT_RETHROW;
      }
   } QT_CATCH(...) {
      adt->node_delete(aupdate, payload(), abstractNode);
      QT_RETHROW;
   }

   // clean up the update array for further insertions
   /*
   for (int i = 0; i <= d->topLevel; ++i) {
       if ( aupdate[i]==reinterpret_cast<QMapData::Node *>(adt) || aupdate[i]->forward[i] != abstractNode)
           break;
       aupdate[i] = abstractNode;
   }
   */

   return abstractNode;
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE QMapData::Node *QMap<Key, T, Compare>::findNode(const Key &akey) const
{
   QMapData::Node *cur  = e;
   QMapData::Node *next = e;

   for (int i = d->topLevel; i >= 0; i--) {
      while ((next = cur->forward[i]) != e && m_compare(concrete(next)->key, akey)) {
         cur = next;
      }
   }

   if (next != e && ! m_compare(akey, concrete(next)->key)) {
      return next;
   } else {
      return e;
   }
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE const T QMap<Key, T, Compare>::value(const Key &akey) const
{
   QMapData::Node *node;
   if (d->size == 0 || (node = findNode(akey)) == e) {
      return T();
   } else {
      return concrete(node)->value;
   }
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE const T QMap<Key, T, Compare>::value(const Key &akey, const T &adefaultValue) const
{
   QMapData::Node *node;
   if (d->size == 0 || (node = findNode(akey)) == e) {
      return adefaultValue;
   } else {
      return concrete(node)->value;
   }
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE const T QMap<Key, T, Compare>::operator[](const Key &akey) const
{
   return value(akey);
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE T &QMap<Key, T, Compare>::operator[](const Key &akey)
{
   detach();

   QMapData::Node *update[QMapData::LastLevel + 1];
   QMapData::Node *node = mutableFindNode(update, akey);
   if (node == e) {
      node = node_create(d, update, akey, T());
   }
   return concrete(node)->value;
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE int QMap<Key, T, Compare>::count(const Key &akey) const
{
   int cnt = 0;
   QMapData::Node *node = findNode(akey);
   if (node != e) {
      do {
         ++cnt;
         node = node->forward[0];
      } while (node != e && !m_compare(akey, concrete(node)->key));
   }
   return cnt;
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE bool QMap<Key, T, Compare>::contains(const Key &akey) const
{
   return findNode(akey) != e;
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMap<Key, T, Compare>::iterator QMap<Key, T, Compare>::insert(const Key &akey, const T &avalue)
{
   detach();

   QMapData::Node *update[QMapData::LastLevel + 1];
   QMapData::Node *node = mutableFindNode(update, akey);
   if (node == e) {
      node = node_create(d, update, akey, avalue);
   } else {
      concrete(node)->value = avalue;
   }
   return iterator(node);
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMap<Key, T, Compare>::iterator QMap<Key, T, Compare>::insertMulti(const Key &akey, const T &avalue)
{
   detach();

   QMapData::Node *update[QMapData::LastLevel + 1];
   mutableFindNode(update, akey);
   return iterator(node_create(d, update, akey, avalue));
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMap<Key, T, Compare>::const_iterator QMap<Key, T, Compare>::find(const Key &akey) const
{
   return const_iterator(findNode(akey));
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMap<Key, T, Compare>::const_iterator QMap<Key, T, Compare>::constFind(const Key &akey) const
{
   return const_iterator(findNode(akey));
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMap<Key, T, Compare>::iterator QMap<Key, T, Compare>::find(const Key &akey)
{
   detach();
   return iterator(findNode(akey));
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE QMap<Key, T, Compare> &QMap<Key, T, Compare>::unite(const QMap<Key, T, Compare> &other)
{
   QMap<Key, T, Compare> copy(other);
   const_iterator it = copy.constEnd();
   const const_iterator b = copy.constBegin();
   while (it != b) {
      --it;
      insertMulti(it.key(), it.value());
   }
   return *this;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE void QMap<Key, T, Compare>::freeData(QMapData *x)
{
   if (QTypeInfo<Key>::isComplex || QTypeInfo<T>::isComplex) {
      QMapData *cur  = x;
      QMapData *next = cur->forward[0];

      while (next != x) {
         cur = next;
         next = cur->forward[0];

         Node *concreteNode = concrete(reinterpret_cast<QMapData::Node *>(cur));
         concreteNode->key.~Key();
         concreteNode->value.~T();
      }
   }
   x->continueFreeData(payload());
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE int QMap<Key, T, Compare>::remove(const Key &akey)
{
   detach();

   QMapData::Node *update[QMapData::LastLevel + 1];
   QMapData::Node *cur = e;
   QMapData::Node *next = e;
   int oldSize = d->size;

   for (int i = d->topLevel; i >= 0; i--) {
      while ((next = cur->forward[i]) != e && m_compare(concrete(next)->key, akey)) {
         cur = next;
      }
      update[i] = cur;
   }

   if (next != e && ! m_compare(akey, concrete(next)->key)) {
      bool deleteNext = true;
      do {
         cur = next;
         next = cur->forward[0];
         deleteNext = (next != e && !m_compare(concrete(cur)->key, concrete(next)->key));
         concrete(cur)->key.~Key();
         concrete(cur)->value.~T();
         d->node_delete(update, payload(), cur);
      } while (deleteNext);
   }

   return oldSize - d->size;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE T QMap<Key, T, Compare>::take(const Key &akey)
{
   detach();

   QMapData::Node *update[QMapData::LastLevel + 1];
   QMapData::Node *cur = e;
   QMapData::Node *next = e;

   for (int i = d->topLevel; i >= 0; i--) {
      while ((next = cur->forward[i]) != e && m_compare(concrete(next)->key, akey)) {
         cur = next;
      }
      update[i] = cur;
   }

   if (next != e && !m_compare(akey, concrete(next)->key)) {
      T t = concrete(next)->value;
      concrete(next)->key.~Key();
      concrete(next)->value.~T();
      d->node_delete(update, payload(), next);
      return t;
   }
   return T();
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE typename QMap<Key, T, Compare>::iterator QMap<Key, T, Compare>::erase(iterator it)
{
   QMapData::Node *update[QMapData::LastLevel + 1];
   QMapData::Node *cur = e;
   QMapData::Node *next = e;

   if (it == iterator(e)) {
      return it;
   }

   for (int i = d->topLevel; i >= 0; i--) {
      while ((next = cur->forward[i]) != e && m_compare(concrete(next)->key, it.key())) {
         cur = next;
      }
      update[i] = cur;
   }

   while (next != e) {
      cur = next;
      next = cur->forward[0];
      if (cur == it) {
         concrete(cur)->key.~Key();
         concrete(cur)->value.~T();
         d->node_delete(update, payload(), cur);
         return iterator(next);
      }

      for (int i = 0; i <= d->topLevel; ++i) {
         if (update[i]->forward[i] != cur) {
            break;
         }
         update[i] = cur;
      }
   }
   return end();
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE void QMap<Key, T, Compare>::detach_helper()
{
   union {
      QMapData *d;
      QMapData::Node *e;
   } x;

   x.d = QMapData::createData(alignment());

   if (d->size) {
      x.d->insertInOrder = true;
      QMapData::Node *update[QMapData::LastLevel + 1];
      QMapData::Node *cur = e->forward[0];
      update[0] = x.e;

      while (cur != e) {
         QT_TRY {
            Node *concreteNode = concrete(cur);
            node_create(x.d, update, concreteNode->key, concreteNode->value);
         } QT_CATCH(...) {
            freeData(x.d);
            QT_RETHROW;
         }
         cur = cur->forward[0];
      }
      x.d->insertInOrder = false;
   }
   if (!d->ref.deref()) {
      freeData(d);
   }
   d = x.d;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE QMapData::Node *QMap<Key, T, Compare>::mutableFindNode(QMapData::Node *aupdate[],
      const Key &akey) const
{
   QMapData::Node *cur = e;
   QMapData::Node *next = e;

   for (int i = d->topLevel; i >= 0; i--) {
      while ((next = cur->forward[i]) != e && m_compare(concrete(next)->key, akey)) {
         cur = next;
      }
      aupdate[i] = cur;
   }
   if (next != e && !m_compare(akey, concrete(next)->key)) {
      return next;
   } else {
      return e;
   }
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key, T, Compare>::uniqueKeys() const
{
   QList<Key> res;
   res.reserve(size()); // May be too much, but assume short lifetime
   const_iterator i = begin();

   if (i != end()) {
      for (;;) {
         const Key &aKey = i.key();
         res.append(aKey);
         do {
            if (++i == end()) {
               goto break_out_of_outer_loop;
            }
         } while (!(aKey < i.key()));   // loop while (key == i.key())
      }
   }
break_out_of_outer_loop:
   return res;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key, T, Compare>::keys() const
{
   QList<Key> res;
   res.reserve(size());
   const_iterator i = begin();
   while (i != end()) {
      res.append(i.key());
      ++i;
   }
   return res;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key, T, Compare>::keys(const T &avalue) const
{
   QList<Key> res;
   const_iterator i = begin();
   while (i != end()) {
      if (i.value() == avalue) {
         res.append(i.key());
      }
      ++i;
   }
   return res;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE const Key QMap<Key, T, Compare>::key(const T &avalue) const
{
   return key(avalue, Key());
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE const Key QMap<Key, T, Compare>::key(const T &avalue, const Key &defaultKey) const
{
   const_iterator i = begin();
   while (i != end()) {
      if (i.value() == avalue) {
         return i.key();
      }
      ++i;
   }

   return defaultKey;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE QList<T> QMap<Key, T, Compare>::values() const
{
   QList<T> res;
   res.reserve(size());
   const_iterator i = begin();
   while (i != end()) {
      res.append(i.value());
      ++i;
   }
   return res;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE QList<T> QMap<Key, T, Compare>::values(const Key &akey) const
{
   QList<T> res;
   QMapData::Node *node = findNode(akey);
   if (node != e) {
      do {
         res.append(concrete(node)->value);
         node = node->forward[0];
      } while (node != e && !m_compare(akey, concrete(node)->key));
   }
   return res;
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMap<Key, T, Compare>::const_iterator
QMap<Key, T, Compare>::lowerBound(const Key &akey) const
{
   QMapData::Node *update[QMapData::LastLevel + 1];
   mutableFindNode(update, akey);
   return const_iterator(update[0]->forward[0]);
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMap<Key, T, Compare>::iterator QMap<Key, T, Compare>::lowerBound(const Key &akey)
{
   detach();
   return static_cast<QMapData::Node *>(const_cast<const QMap *>(this)->lowerBound(akey));
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMap<Key, T, Compare>::const_iterator
QMap<Key, T, Compare>::upperBound(const Key &akey) const
{
   QMapData::Node *update[QMapData::LastLevel + 1];
   mutableFindNode(update, akey);
   QMapData::Node *node = update[0]->forward[0];

   while (node != e && !m_compare(akey, concrete(node)->key)) {
      node = node->forward[0];
   }
   return const_iterator(node);
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE typename QMap<Key, T, Compare>::iterator QMap<Key, T, Compare>::upperBound(const Key &akey)
{
   detach();
   return static_cast<QMapData::Node *>(const_cast<const QMap *>(this)->upperBound(akey));
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE bool QMap<Key, T, Compare>::operator==(const QMap<Key, T, Compare> &other) const
{
   if (size() != other.size()) {
      return false;
   }
   if (d == other.d) {
      return true;
   }

   const_iterator it1 = begin();
   const_iterator it2 = other.begin();

   while (it1 != end()) {
      if (!(it1.value() == it2.value()) || m_compare(it1.key(), it2.key()) || m_compare(it2.key(), it1.key())) {
         return false;
      }
      ++it2;
      ++it1;
   }
   return true;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE QMap<Key, T, Compare>::QMap(const std::map<Key, T> &other)
{
   d = QMapData::createData(alignment());
   d->insertInOrder = true;
   typename std::map<Key, T>::const_iterator it = other.end();
   while (it != other.begin()) {
      --it;
      insert((*it).first, (*it).second);
   }
   d->insertInOrder = false;
}

template <class Key, class T, class Compare>
Q_OUTOFLINE_TEMPLATE std::map<Key, T> QMap<Key, T, Compare>::toStdMap() const
{
   std::map<Key, T> map;
   const_iterator it = end();
   while (it != begin()) {
      --it;
      map.insert(std::pair<Key, T>(it.key(), it.value()));
   }
   return map;
}


template <class Key, class T, class Compare>
class QMultiMap : public QMap<Key, T, Compare>
{
 public:
   QMultiMap() {}

   inline QMultiMap(std::initializer_list<std::pair<Key, T> > list) {
      for (typename std::initializer_list<std::pair<Key, T> >::const_iterator it = list.begin(); it != list.end(); ++it) {
         insert(it->first, it->second);
      }
   }

   QMultiMap(const QMap<Key, T, Compare> &other) : QMap<Key, T, Compare>(other) {}
   inline void swap(QMultiMap<Key, T, Compare> &other) {
      QMap<Key, T, Compare>::swap(other);
   }

   inline typename QMap<Key, T, Compare>::iterator replace(const Key &key, const T &value) {
      return QMap<Key, T, Compare>::insert(key, value);
   }
 
  inline typename QMap<Key, T, Compare>::iterator insert(const Key &key, const T &value) {
      return QMap<Key, T, Compare>::insertMulti(key, value);
   }

   inline QMultiMap &operator+=(const QMultiMap &other) {
      this->unite(other);
      return *this;
   }

   inline QMultiMap operator+(const QMultiMap &other) const {
      QMultiMap result = *this;
      result += other;
      return result;
   }

   using QMap<Key, T, Compare>::contains;
   using QMap<Key, T, Compare>::remove;
   using QMap<Key, T, Compare>::count;
   using QMap<Key, T, Compare>::find;
   using QMap<Key, T, Compare>::constFind;

   bool contains(const Key &key, const T &value) const;

   int remove(const Key &key, const T &value);

   int count(const Key &key, const T &value) const;

   typename QMap<Key, T, Compare>::iterator find(const Key &key, const T &value) {
      typename QMap<Key, T, Compare>::iterator i(find(key));
      typename QMap<Key, T, Compare>::iterator end(this->end());

      while (i != end && ! this->m_compare(key, i.key())) {
         if (i.value() == value) {
            return i;
         }
         ++i;
      }
      return end;
   }

   typename QMap<Key, T, Compare>::const_iterator find(const Key &key, const T &value) const {
      typename QMap<Key, T, Compare>::const_iterator i(constFind(key));
      typename QMap<Key, T, Compare>::const_iterator end(QMap<Key, T, Compare>::constEnd());

      while (i != end && ! this->m_compare(key, i.key())) {
         if (i.value() == value) {
            return i;
         }
         ++i;
      }
      return end;
   }

   typename QMap<Key, T, Compare>::const_iterator constFind(const Key &key, const T &value) const {
      return find(key, value);
   }

 private:
   T &operator[](const Key &key);
   const T operator[](const Key &key) const;
};

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE bool QMultiMap<Key, T, Compare>::contains(const Key &key, const T &value) const
{
   return constFind(key, value) != QMap<Key, T, Compare>::constEnd();
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE int QMultiMap<Key, T, Compare>::remove(const Key &key, const T &value)
{
   int n = 0;
   typename QMap<Key, T, Compare>::iterator i(find(key));
   typename QMap<Key, T, Compare>::iterator end(QMap<Key, T, Compare>::end());

   while (i != end && ! this->m_compare(key, i.key())) {
      if (i.value() == value) {
         i = this->erase(i);
         ++n;
      } else {
         ++i;
      }
   }
   return n;
}

template <class Key, class T, class Compare>
Q_INLINE_TEMPLATE int QMultiMap<Key, T, Compare>::count(const Key &key, const T &value) const
{
   int n = 0;
   typename QMap<Key, T, Compare>::const_iterator i(constFind(key));
   typename QMap<Key, T, Compare>::const_iterator end(QMap<Key, T, Compare>::constEnd());
   while (i != end && ! this->m_compare(key, i.key())) {
      if (i.value() == value) {
         ++n;
      }
      ++i;
   }
   return n;
}

template <class Key, class T, class Compare = qMapCompare<Key>>
class QMapIterator
{ 
   typedef typename QMap<Key, T, Compare>::const_iterator const_iterator;
   typedef const_iterator Item;
   QMap<Key, T, Compare> c;
   const_iterator i, n;
   inline bool item_exists() const { return n != c.constEnd(); }
   
   public:
      inline QMapIterator(const QMap<Key, T, Compare> &container)
      : c(container), i(c.constBegin()), n(c.constEnd()) {}

      inline QMapIterator &operator=(const QMap<Key, T, Compare> &container)
         { c = container; i = c.constBegin(); n = c.constEnd(); return *this; }

      inline void toFront() { i = c.constBegin(); n = c.constEnd(); }
      inline void toBack() { i = c.constEnd(); n = c.constEnd(); }
      inline bool hasNext() const { return i != c.constEnd(); }
      inline Item next() { n = i++; return n; }
      inline Item peekNext() const { return i; }
      inline bool hasPrevious() const { return i != c.constBegin(); }
      inline Item previous() { n = --i; return n; }
      inline Item peekPrevious() const { const_iterator p = i; return --p; }
      inline const T &value() const { Q_ASSERT(item_exists()); return *n; }
      inline const Key &key() const { Q_ASSERT(item_exists()); return n.key(); }

      inline bool findNext(const T &t)
         { while ((n = i) != c.constEnd()) if (*i++ == t) return true; return false; }

      inline bool findPrevious(const T &t)
         { while (i != c.constBegin()) if (*(n = --i) == t) return true;
         n = c.constEnd(); return false; }           
};

template <class Key, class T, class Compare = qMapCompare<Key>>
class QMutableMapIterator
{ 
   typedef typename QMap<Key, T, Compare>::iterator iterator;
   typedef typename QMap<Key, T, Compare>::const_iterator const_iterator;
   typedef iterator Item;
   QMap<Key, T, Compare> *c;
   iterator i, n;
   inline bool item_exists() const { return const_iterator(n) != c->constEnd(); } 
   
   public:
      inline QMutableMapIterator(QMap<Key, T, Compare> &container)
         : c(&container)
         { c->setSharable(false); i = c->begin(); n = c->end(); } 
      
      inline ~QMutableMapIterator()
         { c->setSharable(true); }

      inline QMutableMapIterator &operator=(QMap<Key, T, Compare> &container)
         { c->setSharable(true); c = &container; c->setSharable(false);
	   i = c->begin(); n = c->end(); return *this; }

      inline void toFront() { i = c->begin(); n = c->end(); }
      inline void toBack() { i = c->end(); n = i; }
      inline bool hasNext() const { return c->constEnd() != const_iterator(i); }
      inline Item next() { n = i++; return n; }
      inline Item peekNext() const { return i; }
      inline bool hasPrevious() const { return c->constBegin() != const_iterator(i); }
      inline Item previous() { n = --i; return n; }
      inline Item peekPrevious() const { iterator p = i; return --p; }

      inline void remove()
         { if (c->constEnd() != const_iterator(n)) { i = c->erase(n); n = c->end(); } }

      inline void setValue(const T &t) const { if (c->constEnd() != const_iterator(n)) *n = t; }
      inline T &value() { Q_ASSERT(item_exists()); return *n; }
      inline const T &value() const { Q_ASSERT(item_exists()); return *n; }
      inline const Key &key() const {Q_ASSERT(item_exists()); return n.key(); }

      inline bool findNext(const T &t) 
         { while (c->constEnd() != const_iterator(n = i)) if (*i++ == t) return true; return false; } 

      inline bool findPrevious(const T &t) 
         { while (c->constBegin() != const_iterator(i)) if (*(n = --i) == t) return true; n = c->end(); return false;  } 
};

QT_END_NAMESPACE

#endif // QMAP_H
