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

#ifndef QFREELIST_P_H
#define QFREELIST_P_H

#include <qatomic.h>
#include <qglobal.h>

template <typename T>
struct QFreeListElement {
   using ReferenceType      = T &;
   using ConstReferenceType = const T &;

   T _t;
   std::atomic<int> next;

   ConstReferenceType t() const {
      return _t;
   }

   ReferenceType t() {
      return _t;
   }
};

template <>
struct QFreeListElement<void> {
   using ReferenceType      = void;
   using ConstReferenceType = void;

   std::atomic<int> next;

   void t() const {
   }

   void t() {
   }
};

struct QFreeListDefaultConstants {
   // used by QFreeList, make sure to define all of when customizing
   enum FreeListFlags {
      InitialNextValue = 0,
      IndexMask        = 0x00ffffff,
      SerialMask       = ~IndexMask & ~0x80000000,
      SerialCounter    = IndexMask + 1,
      MaxIndex         = IndexMask,
      BlockCount       = 4,
   };

   static const int Sizes[BlockCount];
};

template <typename T, typename ConstantsType = QFreeListDefaultConstants>
class QFreeList
{
   using ValueType          = T;
   using ElementType        = QFreeListElement<T>;
   using ReferenceType      = typename ElementType::ReferenceType;
   using ConstReferenceType = typename ElementType::ConstReferenceType;

   // return which block the index \a x falls in, and modify \a x to be the index into that block
   static int blockfor(int &x) {
      for (int i = 0; i < ConstantsType::BlockCount; ++i) {
         int size = ConstantsType::Sizes[i];

         if (x < size) {
            return i;
         }

         x -= size;
      }

      return -1;
   }

   // allocate a block of the given \a size, initialized starting with the given \a offset
   static ElementType *allocate(int offset, int size) {
      ElementType *v = new ElementType[size];

      for (int i = 0; i < size; ++i) {
         v[i].next.store(offset + i + 1);
      }

      return v;
   }

   // take the current serial number from \a o, increment it, and store it in \a n
   static int incrementserial(int o, int n) {
      return int((uint(n) & ConstantsType::IndexMask) | ((uint(o) + ConstantsType::SerialCounter) & ConstantsType::SerialMask));
   }

   // the blocks
   QAtomicPointer<ElementType> _v[ConstantsType::BlockCount];

   // the next free id
   QAtomicInt _next;

   QFreeList(const QFreeList &) = delete;

 public:
   inline QFreeList();
   inline ~QFreeList();

   // returns the payload for the given index \a x
   inline ConstReferenceType at(int x) const;
   inline ReferenceType operator[](int x);

   /*
       Return the next free id. Use this id to access the payload (see above).
       Call release(id) when done using the id.
   */
   inline int next();
   inline void release(int id);
};

template <typename T, typename ConstantsType>
inline QFreeList<T, ConstantsType>::QFreeList()
   : _next(ConstantsType::InitialNextValue)
{ }

template <typename T, typename ConstantsType>
inline QFreeList<T, ConstantsType>::~QFreeList()
{
   for (int i = 0; i < ConstantsType::BlockCount; ++i) {
      delete [] _v[i].load();
   }
}

template <typename T, typename ConstantsType>
inline typename QFreeList<T, ConstantsType>::ConstReferenceType QFreeList<T, ConstantsType>::at(int x) const
{
   const int block = blockfor(x);
   return (_v[block].load())[x].t();
}

template <typename T, typename ConstantsType>
inline typename QFreeList<T, ConstantsType>::ReferenceType QFreeList<T, ConstantsType>::operator[](int x)
{
   const int block = blockfor(x);
   return (_v[block].load())[x].t();
}

template <typename T, typename ConstantsType>
inline int QFreeList<T, ConstantsType>::next()
{
   int id;
   int newid;
   int at;

   ElementType *v;

   id = _next.load();

   do {
      at = id & ConstantsType::IndexMask;
      const int block = blockfor(at);
      v = _v[block].loadAcquire();

      if (! v) {
         v = allocate((id & ConstantsType::IndexMask) - at, ConstantsType::Sizes[block]);

         ElementType *expected = nullptr;

         if (! _v[block].compareExchange(expected, v, std::memory_order_release)) {
            // race with another thread lost
            delete [] v;

            v = _v[block].loadAcquire();
            Q_ASSERT(v != nullptr);
         }
      }

      newid = v[at].next.load() | (id & ~ConstantsType::IndexMask);

   } while (! _next.compareExchange(id, newid, std::memory_order_relaxed));

   return id & ConstantsType::IndexMask;
}

template <typename T, typename ConstantsType>
inline void QFreeList<T, ConstantsType>::release(int id)
{
   int at = id & ConstantsType::IndexMask;
   const int block = blockfor(at);
   ElementType *v = _v[block].load();

   int x;
   int newid;

   x = _next.load(std::memory_order_acquire);

   do {
      v[at].next.store(x & ConstantsType::IndexMask);

      newid = incrementserial(x, id);

   } while (! _next.compareExchange(x, newid, std::memory_order_release, std::memory_order_acquire));
}

#endif // QFREELIST_P_H
