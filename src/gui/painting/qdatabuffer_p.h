/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QDATABUFFER_P_H
#define QDATABUFFER_P_H

#include <qbytearray.h>

template <typename Type> class QDataBuffer
{
   Q_DISABLE_COPY(QDataBuffer)

 public:
   QDataBuffer(int res) {
      m_capacity = res;

      if (res) {
         m_buffer = (Type *) malloc(m_capacity * sizeof(Type));
      } else {
         m_buffer = 0;
      }

      m_size = 0;
   }

   ~QDataBuffer() {
      if (m_buffer) {
         free(m_buffer);
      }
   }

   inline void reset() {
      m_size = 0;
   }

   inline bool isEmpty() const {
      return m_size == 0;
   }

   inline int size() const {
      return m_size;
   }

   inline Type *data() const {
      return m_buffer;
   }

   inline Type &at(int i) {
      Q_ASSERT(i >= 0 && i < m_size);
      return m_buffer[i];
   }

   inline const Type &at(int i) const {
      Q_ASSERT(i >= 0 && i < m_size);
      return m_buffer[i];
   }

   inline Type &last() {
      Q_ASSERT(! isEmpty());
      return m_buffer[m_size - 1];
   }

   inline const Type &last() const {
      Q_ASSERT(! isEmpty());
      return m_buffer[m_size - 1];
   }

   inline Type &first() {
      Q_ASSERT(! isEmpty());
      return m_buffer[0];
   }

   inline const Type &first() const {
      Q_ASSERT(! isEmpty());
      return m_buffer[0];
   }

   inline void add(const Type &t) {
      reserve(m_size + 1);
      m_buffer[m_size] = t;
      ++m_size;
   }

   inline void pop_back() {
      Q_ASSERT(m_size > 0);
      --m_size;
   }

   inline void resize(int size) {
      reserve(size);
      m_size = size;
   }

   inline void reserve(int size) {
      if (size > m_capacity) {
         if (m_capacity == 0) {
            m_capacity = 1;
         }

         while (m_capacity < size) {
            m_capacity *= 2;
         }

         m_buffer = (Type *) realloc(m_buffer, m_capacity * sizeof(Type));
      }
   }

   inline void shrink(int size) {
      m_capacity = size;
      if (size) {
         m_buffer = (Type *) realloc(m_buffer, m_capacity * sizeof(Type));

      } else {
         free(m_buffer);
         m_buffer = 0;
      }
   }

   inline void swap(QDataBuffer<Type> &other) {
      qSwap(m_capacity, other.m_capacity);
      qSwap(m_size, other.m_size);
      qSwap(m_buffer, other.m_buffer);
   }

   inline QDataBuffer &operator<<(const Type &t) {
      add(t);
      return *this;
   }

 private:
   int m_capacity;
   int m_size;
   Type *m_buffer;
};

#endif
