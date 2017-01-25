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

template <class Key, class Val>
struct QMapNode {
   Key key;
   Val value;

 private:
   // never access these members through this structure, see below
   QMapData::Node *backward;
   QMapData::Node *forward[1];
};

template <class Key, class Val>
struct QMapPayloadNode {
   Key key;
   Val value;

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

template <class Key, class Val, class C>
class QMap
{
   typedef QMapNode<Key, Val> Node;
   typedef QMapPayloadNode<Key, Val> PayloadNode;

   union {
      QMapData *d;
      QMapData::Node *e;
   };

 protected:
   C m_compare;

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

   explicit inline QMap(C compare) 
       : d( QMapData::sharedNull() ), m_compare(compare)
   { }

   QMap(const QMap<Key, Val, C> &other) 
      : d(other.d), m_compare(other.m_compare) 
   {
      d->ref.ref();
      if (! d->sharable) {
         detach();
      }
   }

   QMap(std::initializer_list<std::pair<Key, Val> > list)
      : d(QMapData::sharedNull() ) 
   {
      for (typename std::initializer_list<std::pair<Key, Val> >::const_iterator it = list.begin(); it != list.end(); ++it) {
         insert(it->first, it->second);
      }
   }

   explicit QMap(const std::map<Key, Val> &other);

   ~QMap() {
      if (! d->ref.deref()) {
         freeData(d);
      }
   }

   QMap<Key, Val, C> &operator=(const QMap<Key, Val, C> &other);

   QMap<Key, Val, C> &operator=(QMap<Key, Val, C> && other) {
      qSwap(d, other.d);
      m_compare = std::move(other.m_compare);
      return *this;
   }

   void swap(QMap<Key, Val, C> &other) {
      qSwap(d, other.d);
      swap(m_compare, other.m_compare);
   }
   
   std::map<Key, Val> toStdMap() const;

   bool operator==(const QMap<Key, Val, C> &other) const;

   inline bool operator!=(const QMap<Key, Val, C> &other) const {
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

   inline bool isSharedWith(const QMap<Key, Val, C> &other) const {
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
   Val take(const Key &key);

   bool contains(const Key &key) const;
   const Key key(const Val &value) const;
   const Key key(const Val &value, const Key &defaultKey) const;
   const Val value(const Key &key) const;
   const Val value(const Key &key, const Val &defaultValue) const;
   Val &operator[](const Key &key);
   const Val operator[](const Key &key) const;

   QList<Key> uniqueKeys() const;
   QList<Key> keys() const;
   QList<Key> keys(const Val &value) const;
   QList<Val> values() const;
   QList<Val> values(const Key &key) const;
   int count(const Key &key) const;

   class const_iterator;

   class iterator
   {
      friend class const_iterator;
      QMapData::Node *i;

    public:
      typedef std::bidirectional_iterator_tag iterator_category;
      typedef qptrdiff difference_type;
      typedef Val value_type;
      typedef Val *pointer;
      typedef Val &reference;

      // ### Qt5/get rid of 'operator Node *'
      inline operator QMapData::Node *() const {
         return i;
      }
      inline iterator() : i(0) { }
      inline iterator(QMapData::Node *node) : i(node) { }

      inline const Key &key() const {
         return concrete(i)->key;
      }
      inline Val &value() const {
         return concrete(i)->value;
      }

      inline Val &operator*() const {
         return concrete(i)->value;
      }
      inline Val *operator->() const {
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
      typedef Val value_type;
      typedef const Val *pointer;
      typedef const Val &reference;

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
      inline const Val &value() const {
         return concrete(i)->value;
      }

      inline const Val &operator*() const {
         return concrete(i)->value;
      }
      inline const Val *operator->() const {
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

   iterator insert(const Key &key, const Val &value);
   iterator insertMulti(const Key &key, const Val &value);

   QMap<Key, Val, C> &unite(const QMap<Key, Val, C> &other);

   // STL compatibility
   typedef Key key_type;
   typedef Val mapped_type;
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
   QMapData::Node *node_create(QMapData *d, QMapData::Node *update[], const Key &key, const Val &value);
};

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE QMap<Key, Val, C> &QMap<Key, Val, C>::operator=(const QMap<Key, Val, C> &other)
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

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE void QMap<Key, Val, C>::clear()
{
   *this = QMap<Key, Val, C>(m_compare);
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMapData::Node *
QMap<Key, Val, C>::node_create(QMapData *adt, QMapData::Node *aupdate[], const Key &akey, const Val &avalue)
{
   QMapData::Node *abstractNode = adt->node_create(aupdate, payload(), alignment());
   QT_TRY {
      Node *concreteNode = concrete(abstractNode);
      new (&concreteNode->key) Key(akey);
      QT_TRY {
         new (&concreteNode->value) Val(avalue);
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

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE QMapData::Node *QMap<Key, Val, C>::findNode(const Key &akey) const
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

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE const Val QMap<Key, Val, C>::value(const Key &akey) const
{
   QMapData::Node *node;
   if (d->size == 0 || (node = findNode(akey)) == e) {
      return Val();
   } else {
      return concrete(node)->value;
   }
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE const Val QMap<Key, Val, C>::value(const Key &akey, const Val &adefaultValue) const
{
   QMapData::Node *node;
   if (d->size == 0 || (node = findNode(akey)) == e) {
      return adefaultValue;
   } else {
      return concrete(node)->value;
   }
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE const Val QMap<Key, Val, C>::operator[](const Key &akey) const
{
   return value(akey);
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE Val &QMap<Key, Val, C>::operator[](const Key &akey)
{
   detach();

   QMapData::Node *update[QMapData::LastLevel + 1];
   QMapData::Node *node = mutableFindNode(update, akey);
   if (node == e) {
      node = node_create(d, update, akey, Val());
   }
   return concrete(node)->value;
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE int QMap<Key, Val, C>::count(const Key &akey) const
{
   int cnt = 0;
   QMapData::Node *node = findNode(akey);
   if (node != e) {
      do {
         ++cnt;
         node = node->forward[0];
      } while (node != e && ! m_compare(akey, concrete(node)->key));
   }
   return cnt;
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE bool QMap<Key, Val, C>::contains(const Key &akey) const
{
   return findNode(akey) != e;
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMap<Key, Val, C>::iterator QMap<Key, Val, C>::insert(const Key &akey, const Val &avalue)
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

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMap<Key, Val, C>::iterator QMap<Key, Val, C>::insertMulti(const Key &akey, const Val &avalue)
{
   detach();

   QMapData::Node *update[QMapData::LastLevel + 1];
   mutableFindNode(update, akey);
   return iterator(node_create(d, update, akey, avalue));
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMap<Key, Val, C>::const_iterator QMap<Key, Val, C>::find(const Key &akey) const
{
   return const_iterator(findNode(akey));
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMap<Key, Val, C>::const_iterator QMap<Key, Val, C>::constFind(const Key &akey) const
{
   return const_iterator(findNode(akey));
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMap<Key, Val, C>::iterator QMap<Key, Val, C>::find(const Key &akey)
{
   detach();
   return iterator(findNode(akey));
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE QMap<Key, Val, C> &QMap<Key, Val, C>::unite(const QMap<Key, Val, C> &other)
{
   QMap<Key, Val, C> copy(other);
   const_iterator it = copy.constEnd();
   const const_iterator b = copy.constBegin();
   while (it != b) {
      --it;
      insertMulti(it.key(), it.value());
   }
   return *this;
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE void QMap<Key, Val, C>::freeData(QMapData *x)
{
   if (QTypeInfo<Key>::isComplex || QTypeInfo<Val>::isComplex) {
      QMapData *cur  = x;
      QMapData *next = cur->forward[0];

      while (next != x) {
         cur = next;
         next = cur->forward[0];

         Node *concreteNode = concrete(reinterpret_cast<QMapData::Node *>(cur));
         concreteNode->key.~Key();
         concreteNode->value.~Val();
      }
   }
   x->continueFreeData(payload());
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE int QMap<Key, Val, C>::remove(const Key &akey)
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
         concrete(cur)->value.~Val();
         d->node_delete(update, payload(), cur);
      } while (deleteNext);
   }

   return oldSize - d->size;
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE Val QMap<Key, Val, C>::take(const Key &akey)
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
      Val t = concrete(next)->value;
      concrete(next)->key.~Key();
      concrete(next)->value.~Val();
      d->node_delete(update, payload(), next);
      return t;
   }
   return Val();
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE typename QMap<Key, Val, C >::iterator QMap<Key, Val, C>::erase(iterator it)
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
         concrete(cur)->value.~Val();
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

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE void QMap<Key, Val, C>::detach_helper()
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

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE QMapData::Node *QMap<Key, Val, C>::mutableFindNode(QMapData::Node *aupdate[], const Key &akey) const
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

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key, Val, C>::uniqueKeys() const
{
   QList<Key> res;
   res.reserve(size());             // May be too much, but assume short lifetime
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

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key, Val, C>::keys() const
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

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE QList<Key> QMap<Key, Val, C>::keys(const Val &avalue) const
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

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE const Key QMap<Key, Val, C>::key(const Val &avalue) const
{
   return key(avalue, Key());
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE const Key QMap<Key, Val, C>::key(const Val &avalue, const Key &defaultKey) const
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

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE QList<Val> QMap<Key, Val, C>::values() const
{
   QList<Val> res;
   res.reserve(size());
   const_iterator i = begin();
   while (i != end()) {
      res.append(i.value());
      ++i;
   }
   return res;
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE QList<Val> QMap<Key, Val, C>::values(const Key &akey) const
{
   QList<Val> res;
   QMapData::Node *node = findNode(akey);
   if (node != e) {
      do {
         res.append(concrete(node)->value);
         node = node->forward[0];
      } while (node != e && !m_compare(akey, concrete(node)->key));
   }
   return res;
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMap<Key, Val, C>::const_iterator
QMap<Key, Val, C>::lowerBound(const Key &akey) const
{
   QMapData::Node *update[QMapData::LastLevel + 1];
   mutableFindNode(update, akey);
   return const_iterator(update[0]->forward[0]);
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMap<Key, Val, C>::iterator QMap<Key, Val, C>::lowerBound(const Key &akey)
{
   detach();
   return static_cast<QMapData::Node *>(const_cast<const QMap *>(this)->lowerBound(akey));
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMap<Key, Val, C>::const_iterator
QMap<Key, Val, C>::upperBound(const Key &akey) const
{
   QMapData::Node *update[QMapData::LastLevel + 1];
   mutableFindNode(update, akey);
   QMapData::Node *node = update[0]->forward[0];

   while (node != e && !m_compare(akey, concrete(node)->key)) {
      node = node->forward[0];
   }
   return const_iterator(node);
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE typename QMap<Key, Val, C>::iterator QMap<Key, Val, C>::upperBound(const Key &akey)
{
   detach();
   return static_cast<QMapData::Node *>(const_cast<const QMap *>(this)->upperBound(akey));
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE bool QMap<Key, Val, C>::operator==(const QMap<Key, Val, C> &other) const
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

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE QMap<Key, Val, C>::QMap(const std::map<Key, Val> &other)
{
   d = QMapData::createData(alignment());
   d->insertInOrder = true;
   typename std::map<Key, Val>::const_iterator it = other.end();
   while (it != other.begin()) {
      --it;
      insert((*it).first, (*it).second);
   }
   d->insertInOrder = false;
}

template <class Key, class Val, class C>
Q_OUTOFLINE_TEMPLATE std::map<Key, Val> QMap<Key, Val, C>::toStdMap() const
{
   std::map<Key, Val> map;
   const_iterator it = end();
   while (it != begin()) {
      --it;
      map.insert(std::pair<Key, Val>(it.key(), it.value()));
   }
   return map;
}


template <class Key, class Val, class C>
class QMultiMap : public QMap<Key, Val, C>
{
 public:
   QMultiMap() {}

   inline QMultiMap(std::initializer_list<std::pair<Key, Val> > list) {
      for (typename std::initializer_list<std::pair<Key, Val> >::const_iterator it = list.begin(); it != list.end(); ++it) {
         insert(it->first, it->second);
      }
   }

   QMultiMap(const QMap<Key, Val, C> &other) : QMap<Key, Val, C>(other) {}
   inline void swap(QMultiMap<Key, Val, C> &other) {
      QMap<Key, Val, C>::swap(other);
   }

   inline typename QMap<Key, Val, C>::iterator replace(const Key &key, const Val &value) {
      return QMap<Key, Val, C>::insert(key, value);
   }
 
  inline typename QMap<Key, Val, C>::iterator insert(const Key &key, const Val &value) {
      return QMap<Key, Val, C>::insertMulti(key, value);
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

   using QMap<Key, Val, C>::contains;
   using QMap<Key, Val, C>::remove;
   using QMap<Key, Val, C>::count;
   using QMap<Key, Val, C>::find;
   using QMap<Key, Val, C>::constFind;

   bool contains(const Key &key, const Val &value) const;

   int remove(const Key &key, const Val &value);

   int count(const Key &key, const Val &value) const;

   typename QMap<Key, Val, C>::iterator find(const Key &key, const Val &value) {
      typename QMap<Key, Val, C>::iterator i(find(key));
      typename QMap<Key, Val, C>::iterator end(this->end());

      while (i != end && ! this->m_compare(key, i.key())) {
         if (i.value() == value) {
            return i;
         }
         ++i;
      }
      return end;
   }

   typename QMap<Key, Val, C>::const_iterator find(const Key &key, const Val &value) const {
      typename QMap<Key, Val, C>::const_iterator i(constFind(key));
      typename QMap<Key, Val, C>::const_iterator end(QMap<Key, Val, C>::constEnd());

      while (i != end && ! this->m_compare(key, i.key())) {
         if (i.value() == value) {
            return i;
         }
         ++i;
      }
      return end;
   }

   typename QMap<Key, Val, C>::const_iterator constFind(const Key &key, const Val &value) const {
      return find(key, value);
   }

 private:
   Val &operator[](const Key &key);
   const Val operator[](const Key &key) const;
};

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE bool QMultiMap<Key, Val, C>::contains(const Key &key, const Val &value) const
{
   return constFind(key, value) != QMap<Key, Val, C>::constEnd();
}

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE int QMultiMap<Key, Val, C>::remove(const Key &key, const Val &value)
{
   int n = 0;
   typename QMap<Key, Val, C>::iterator i(find(key));
   typename QMap<Key, Val, C>::iterator end(QMap<Key, Val, C>::end());

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

template <class Key, class Val, class C>
Q_INLINE_TEMPLATE int QMultiMap<Key, Val, C>::count(const Key &key, const Val &value) const
{
   int n = 0;
   typename QMap<Key, Val, C>::const_iterator i(constFind(key));
   typename QMap<Key, Val, C>::const_iterator end(QMap<Key, Val, C>::constEnd());
   while (i != end && ! this->m_compare(key, i.key())) {
      if (i.value() == value) {
         ++n;
      }
      ++i;
   }
   return n;
}

template <class Key, class Val, class C = qMapCompare<Key>>
class QMapIterator
{ 
   typedef typename QMap<Key, Val, C>::const_iterator const_iterator;
   typedef const_iterator Item;
   QMap<Key, Val, C> c;
   const_iterator i, n;
   inline bool item_exists() const { return n != c.constEnd(); }
   
   public:
      inline QMapIterator(const QMap<Key, Val, C> &container)
      : c(container), i(c.constBegin()), n(c.constEnd()) {}

      inline QMapIterator &operator=(const QMap<Key, Val, C> &container)
         { c = container; i = c.constBegin(); n = c.constEnd(); return *this; }

      inline void toFront() { i = c.constBegin(); n = c.constEnd(); }
      inline void toBack() { i = c.constEnd(); n = c.constEnd(); }
      inline bool hasNext() const { return i != c.constEnd(); }
      inline Item next() { n = i++; return n; }
      inline Item peekNext() const { return i; }
      inline bool hasPrevious() const { return i != c.constBegin(); }
      inline Item previous() { n = --i; return n; }
      inline Item peekPrevious() const { const_iterator p = i; return --p; }
      inline const Val &value() const { Q_ASSERT(item_exists()); return *n; }
      inline const Key &key() const { Q_ASSERT(item_exists()); return n.key(); }

      inline bool findNext(const Val &t)
         { while ((n = i) != c.constEnd()) if (*i++ == t) return true; return false; }

      inline bool findPrevious(const Val &t)
         { while (i != c.constBegin()) if (*(n = --i) == t) return true;
         n = c.constEnd(); return false; }           
};

template <class Key, class Val, class C = qMapCompare<Key>>
class QMutableMapIterator
{ 
   typedef typename QMap<Key, Val, C>::iterator iterator;
   typedef typename QMap<Key, Val, C>::const_iterator const_iterator;
   typedef iterator Item;
   QMap<Key, Val, C> *c;
   iterator i, n;
   inline bool item_exists() const { return const_iterator(n) != c->constEnd(); } 
   
   public:
      inline QMutableMapIterator(QMap<Key, Val, C> &container)
         : c(&container)
         { c->setSharable(false); i = c->begin(); n = c->end(); } 
      
      inline ~QMutableMapIterator()
         { c->setSharable(true); }

      inline QMutableMapIterator &operator=(QMap<Key, Val, C> &container)
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

      inline void setValue(const Val &t) const { if (c->constEnd() != const_iterator(n)) *n = t; }
      inline Val &value() { Q_ASSERT(item_exists()); return *n; }
      inline const Val &value() const { Q_ASSERT(item_exists()); return *n; }
      inline const Key &key() const {Q_ASSERT(item_exists()); return n.key(); }

      inline bool findNext(const Val &t) 
         { while (c->constEnd() != const_iterator(n = i)) if (*i++ == t) return true; return false; } 

      inline bool findPrevious(const Val &t) 
         { while (c->constBegin() != const_iterator(i)) if (*(n = --i) == t) return true; n = c->end(); return false;  } 
};

QT_END_NAMESPACE

#endif // QMAP_H
