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

#ifndef QCACHE_H
#define QCACHE_H

#include <qhash.h>

template <class Key, class T>
class QCache
{
   struct Node {
      Node()
         : keyPtr(0)
      {
      }

      Node(T *data, int cost)
         : keyPtr(nullptr), t(data), c(cost), p(nullptr), n(nullptr)
      {
      }

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

   void unlink(Node &n) {
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

   T *relink(const Key &key) {
      typename QHash<Key, Node>::iterator i = hash.find(key);

      if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd()) {
         return nullptr;
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

         n.p = nullptr;
         n.n = f;
         f->p = &n;
         f = &n;
      }

      return n.t;
   }

 public:
   inline explicit QCache(int maxCost = 100);

   QCache(const QCache &) = delete;
   QCache &operator=(const QCache &) = delete;

   ~QCache() {
      clear();
   }

   int maxCost() const {
      return mx;
   }

   void setMaxCost(int cost);
   int totalCost() const {
      return total;
   }

   int size() const {
      return hash.size();
   }

   int count() const {
      return hash.size();
   }

   bool isEmpty() const {
      return hash.isEmpty();
   }

   QList<Key> keys() const {
      return hash.keys();
   }

   void clear();

   bool insert(const Key &key, T *object, int cost = 1);
   T *object(const Key &key) const;

   bool contains(const Key &key) const {
      return hash.contains(key);
   }

   T *operator[](const Key &key) const;

   bool remove(const Key &key);
   T *take(const Key &key);

 private:
   void trim(int m);

};

template <class Key, class T>
inline QCache<Key, T>::QCache(int maxCost)
   : f(nullptr), l(nullptr), mx(maxCost), total(0)
{
}

template <class Key, class T>
inline void QCache<Key, T>::clear()
{
   while (f) {
      delete f->t;
      f = f->n;
   }

   hash.clear();
   l     = nullptr;
   total = 0;
}

template <class Key, class T>
inline void QCache<Key, T>::setMaxCost(int cost)
{
   mx = cost;
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
      return nullptr;
   }

   Node &n = *i;
   T *t = n.t;
   n.t = nullptr;
   unlink(n);

   return t;
}

template <class Key, class T>
bool QCache<Key, T>::insert(const Key &key, T *object, int cost)
{
   remove(key);

   if (cost > mx) {
      delete object;
      return false;
   }

   trim(mx - cost);
   Node sn(object, cost);

   typename QHash<Key, Node>::iterator i = hash.insert(key, sn);

   total    += cost;
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
