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

#ifndef QDATABUFFER_P_H
#define QDATABUFFER_P_H

#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

template <typename Type> class QDataBuffer
{
 public:
   QDataBuffer(int res) {
      capacity = res;
      if (res) {
         buffer = (Type *) qMalloc(capacity * sizeof(Type));
      } else {
         buffer = 0;
      }
      siz = 0;
   }

   ~QDataBuffer() {
      if (buffer) {
         qFree(buffer);
      }
   }

   inline void reset() {
      siz = 0;
   }

   inline bool isEmpty() const {
      return siz == 0;
   }

   inline int size() const {
      return siz;
   }
   inline Type *data() const {
      return buffer;
   }

   inline Type &at(int i) {
      Q_ASSERT(i >= 0 && i < siz);
      return buffer[i];
   }
   inline const Type &at(int i) const {
      Q_ASSERT(i >= 0 && i < siz);
      return buffer[i];
   }
   inline Type &last() {
      Q_ASSERT(!isEmpty());
      return buffer[siz - 1];
   }
   inline const Type &last() const {
      Q_ASSERT(!isEmpty());
      return buffer[siz - 1];
   }
   inline Type &first() {
      Q_ASSERT(!isEmpty());
      return buffer[0];
   }
   inline const Type &first() const {
      Q_ASSERT(!isEmpty());
      return buffer[0];
   }

   inline void add(const Type &t) {
      reserve(siz + 1);
      buffer[siz] = t;
      ++siz;
   }

   inline void pop_back() {
      Q_ASSERT(siz > 0);
      --siz;
   }

   inline void resize(int size) {
      reserve(size);
      siz = size;
   }

   inline void reserve(int size) {
      if (size > capacity) {
         if (capacity == 0) {
            capacity = 1;
         }
         while (capacity < size) {
            capacity *= 2;
         }
         buffer = (Type *) qRealloc(buffer, capacity * sizeof(Type));
      }
   }

   inline void shrink(int size) {
      capacity = size;
      if (size) {
         buffer = (Type *) qRealloc(buffer, capacity * sizeof(Type));
      } else {
         qFree(buffer);
         buffer = 0;
      }
   }

   inline void swap(QDataBuffer<Type> &other) {
      qSwap(capacity, other.capacity);
      qSwap(siz, other.siz);
      qSwap(buffer, other.buffer);
   }

   inline QDataBuffer &operator<<(const Type &t) {
      add(t);
      return *this;
   }

 private:
   int capacity;
   int siz;
   Type *buffer;
};

QT_END_NAMESPACE

#endif // QDATABUFFER_P_H
