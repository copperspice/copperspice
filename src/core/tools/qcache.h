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

#ifndef QCACHE_H
#define QCACHE_H

#include <qhash.h>

template <class Key, class T>
class QCache
{
   struct Node {
      inline Node()
         : keyPtr(0) {}

      inline Node(T *data, int cost)
         : keyPtr(0), t(data), c(cost), p(0), n(0) {}

      const Key *keyPtr;
      T *t;
      int c;
      Node *p;
      Node *n;
   };

   QHash<Key, Node> hash;

   Node *f;
   Node *l;

   int mx;
   int total;

   inline void unlink(Node &n) {
      if (n.p) {
         n.p->n = n.n;
      }

      if (n.n) {
         n.n->p = n.p;
      }

      if (l == &n) {
         l = n.p;
      }

      if (f == &n) {
         f = n.n;
      }

      total -= n.c;
      T *obj = n.t;

      hash.remove(*n.keyPtr);
      delete obj;
   }

   inline T *relink(const Key &key) {
      typename QHash<Key, Node>::iterator i = hash.find(key);

      if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd()) {
         return 0;
      }

      Node &n = *i;
      if (f != &n) {
         if (n.p) {
            n.p->n = n.n;
         }

         if (n.n) {
            n.n->p = n.p;
         }
         if (l == &n) {
            l = n.p;
         }

         n.p = 0;
         n.n = f;
         f->p = &n;
         f = &n;
      }
      return n.t;
   }

   Q_DISABLE_COPY(QCache)

 public:
   inline explicit QCache(int maxCost = 100);

   inline ~QCache() {
      clear();
   }

   inline int maxCost() const {
      return mx;
   }

   void setMaxCost(int m);
   inline int totalCost() const {
      return total;
   }

   inline int size() const {
      return hash.size();
   }

   inline int count() const {
      return hash.size();
   }

   inline bool isEmpty() const {
      return hash.isEmpty();
   }

   inline QList<Key> keys() const {
      return hash.keys();
   }

   void clear();

   bool insert(const Key &key, T *object, int cost = 1);
   T *object(const Key &key) const;

   inline bool contains(const Key &key) const {
      return hash.contains(key);
   }

   T *operator[](const Key &key) const;

   bool remove(const Key &key);
   T *take(const Key &key);

 private:
   void trim(int m);

};

template <class Key, class T>
inline QCache<Key, T>::QCache(int amaxCost)
   : f(0), l(0), mx(amaxCost), total(0) {}

template <class Key, class T>
inline void QCache<Key, T>::clear()
{
   while (f) {
      delete f->t;
      f = f->n;
   }

   hash.clear();
   l = 0;
   total = 0;
}

template <class Key, class T>
inline void QCache<Key, T>::setMaxCost(int m)
{
   mx = m;
   trim(mx);
}

template <class Key, class T>
inline T *QCache<Key, T>::object(const Key &key) const
{
   return const_cast<QCache<Key, T>*>(this)->relink(key);
}

template <class Key, class T>
inline T *QCache<Key, T>::operator[](const Key &key) const
{
   return object(key);
}

template <class Key, class T>
inline bool QCache<Key, T>::remove(const Key &key)
{
   typename QHash<Key, Node>::iterator i = hash.find(key);

   if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd()) {
      return false;
   } else {
      unlink(*i);
      return true;
   }
}

template <class Key, class T>
inline T *QCache<Key, T>::take(const Key &key)
{
   typename QHash<Key, Node>::iterator i = hash.find(key);

   if (i == hash.end()) {
      return 0;
   }

   Node &n = *i;
   T *t = n.t;
   n.t = 0;
   unlink(n);
   return t;
}

template <class Key, class T>
bool QCache<Key, T>::insert(const Key &akey, T *aobject, int acost)
{
   remove(akey);

   if (acost > mx) {
      delete aobject;
      return false;
   }

   trim(mx - acost);
   Node sn(aobject, acost);

   typename QHash<Key, Node>::iterator i = hash.insert(akey, sn);
   total += acost;
   Node *n   = &i.value();
   n->keyPtr = &i.key();

   if (f) {
      f->p = n;
   }
   n->n = f;
   f = n;
   if (!l) {
      l = f;
   }
   return true;
}

template <class Key, class T>
void QCache<Key, T>::trim(int m)
{
   Node *n = l;

   while (n && total > m) {
      Node *u = n;
      n = n->p;
      unlink(*u);
   }
}

#endif
