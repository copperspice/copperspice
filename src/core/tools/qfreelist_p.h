/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#ifndef QFREELIST_P_H
#define QFREELIST_P_H

#include <qglobal.h>
#include <qassert.h>
#include <qatomic.h>

QT_BEGIN_NAMESPACE

template <typename T>
struct QFreeListElement {
   typedef const T &ConstReferenceType;
   typedef T &ReferenceType;

   T _t;
   int next;

   inline ConstReferenceType t() const {
      return _t;
   }
   inline ReferenceType t() {
      return _t;
   }
};

template <>
struct QFreeListElement<void> {
   typedef void ConstReferenceType;
   typedef void ReferenceType;

   int next;

   inline void t() const { }
   inline void t() { }
};

struct QFreeListDefaultConstants {
   // used by QFreeList, make sure to define all of when customizing
   enum {
      InitialNextValue = 0,
      IndexMask = 0x00ffffff,
      SerialMask = ~IndexMask & ~0x80000000,
      SerialCounter = IndexMask + 1,
      MaxIndex = IndexMask,
      BlockCount = 4,
   };

   static const int Sizes[BlockCount];
};

template <typename T, typename ConstantsType = QFreeListDefaultConstants>
class QFreeList
{
   typedef T ValueType;
   typedef QFreeListElement<T> ElementType;
   typedef typename ElementType::ConstReferenceType ConstReferenceType;
   typedef typename ElementType::ReferenceType ReferenceType;

   // return which block the index \a x falls in, and modify \a x to be the index into that block
   static inline int blockfor(int &x) {
      for (int i = 0; i < ConstantsType::BlockCount; ++i) {
         int size = ConstantsType::Sizes[i];
         if (x < size) {
            return i;
         }
         x -= size;
      }
      Q_ASSERT(false);
      return -1;
   }

   // allocate a block of the given \a size, initialized starting with the given \a offset
   static inline ElementType *allocate(int offset, int size) {
      // qDebug("QFreeList: allocating %d elements (%ld bytes) with offset %d", size, size * sizeof(ElementType), offset);
      ElementType *v = new ElementType[size];
      for (int i = 0; i < size; ++i) {
         v[i].next = offset + i + 1;
      }
      return v;
   }

   // take the current serial number from \a o, increment it, and store it in \a n
   static inline int incrementserial(int o, int n) {
      return (n & ConstantsType::IndexMask) | ((o + ConstantsType::SerialCounter) & ConstantsType::SerialMask);
   }

   // the blocks
   QAtomicPointer<ElementType> _v[ConstantsType::BlockCount];
   // the next free id
   QAtomicInt _next;

   // QFreeList is not copyable
   Q_DISABLE_COPY(QFreeList)

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
   int id, newid, at;
   ElementType *v;
   do {
      id = _next.load();

      at = id & ConstantsType::IndexMask;
      const int block = blockfor(at);
      v = _v[block].loadAcquire();

      if (!v) {
         v = allocate((id & ConstantsType::IndexMask) - at, ConstantsType::Sizes[block]);
         if (!_v[block].testAndSetRelease(0, v)) {
            // race with another thread lost
            delete [] v;
            v = _v[block].loadAcquire();
            Q_ASSERT(v != 0);
         }
      }

      newid = v[at].next | (id & ~ConstantsType::IndexMask);
   } while (!_next.testAndSetRelaxed(id, newid));
   // qDebug("QFreeList::next(): returning %d (_next now %d, serial %d)",
   //        id & ConstantsType::IndexMask,
   //        newid & ConstantsType::IndexMask,
   //        (newid & ~ConstantsType::IndexMask) >> 24);
   return id & ConstantsType::IndexMask;
}

template <typename T, typename ConstantsType>
inline void QFreeList<T, ConstantsType>::release(int id)
{
   int at = id & ConstantsType::IndexMask;
   const int block = blockfor(at);
   ElementType *v = _v[block].load();

   int x, newid;
   do {
      x = _next.loadAcquire();
      v[at].next = x & ConstantsType::IndexMask;

      newid = incrementserial(x, id);
   } while (!_next.testAndSetRelease(x, newid));
   // qDebug("QFreeList::release(%d): _next now %d (was %d), serial %d",
   //        id & ConstantsType::IndexMask,
   //        newid & ConstantsType::IndexMask,
   //        x & ConstantsType::IndexMask,
   //        (newid & ~ConstantsType::IndexMask) >> 24);
}

QT_END_NAMESPACE

#endif // QFREELIST_P_H
