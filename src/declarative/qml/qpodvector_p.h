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

#ifndef QPODVECTOR_P_H
#define QPODVECTOR_P_H

#include <QtCore/qglobal.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

template<class T, int Increment = 1024>
class QPODVector
{
 public:
   QPODVector()
      : m_count(0), m_capacity(0), m_data(0) {}
   ~QPODVector() {
      if (m_data) {
         ::free(m_data);
      }
   }

   const T &at(int idx) const {
      return m_data[idx];
   }

   T &operator[](int idx) {
      return m_data[idx];
   }

   void clear() {
      m_count = 0;
   }

   void prepend(const T &v) {
      insert(0, v);
   }

   void append(const T &v) {
      insert(m_count, v);
   }

   void insert(int idx, const T &v) {
      if (m_count == m_capacity) {
         m_capacity += Increment;
         m_data = (T *)q_check_ptr(realloc(m_data, m_capacity * sizeof(T)));
      }
      int moveCount = m_count - idx;
      if (moveCount) {
         ::memmove(m_data + idx + 1, m_data + idx, moveCount * sizeof(T));
      }
      m_count++;
      m_data[idx] = v;
   }

   void reserve(int count) {
      if (count >= m_capacity) {
         m_capacity = (count + (Increment - 1)) & (0xFFFFFFFF - Increment + 1);
         m_data = (T *)q_check_ptr(realloc(m_data, m_capacity * sizeof(T)));
      }
   }

   void insertBlank(int idx, int count) {
      int newSize = m_count + count;
      reserve(newSize);
      int moveCount = m_count - idx;
      if (moveCount)
         ::memmove(m_data + idx + count,  m_data + idx,
                   moveCount * sizeof(T));
      m_count = newSize;
   }

   void remove(int idx, int count = 1) {
      int moveCount = m_count - (idx + count);
      if (moveCount)
         ::memmove(m_data + idx, m_data + idx + count,
                   moveCount * sizeof(T));
      m_count -= count;
   }

   void removeOne(const T &v) {
      int idx = 0;
      while (idx < m_count) {
         if (m_data[idx] == v) {
            remove(idx);
            return;
         }
         ++idx;
      }
   }

   int find(const T &v) {
      for (int idx = 0; idx < m_count; ++idx)
         if (m_data[idx] == v) {
            return idx;
         }
      return -1;
   }

   bool contains(const T &v) {
      return find(v) != -1;
   }

   int count() const {
      return m_count;
   }

   void copyAndClear(QPODVector<T, Increment> &other) {
      if (other.m_data) {
         ::free(other.m_data);
      }
      other.m_count = m_count;
      other.m_capacity = m_capacity;
      other.m_data = m_data;
      m_count = 0;
      m_capacity = 0;
      m_data = 0;
   }

   QPODVector<T, Increment> &operator<<(const T &v) {
      append(v);
      return *this;
   }
 private:
   QPODVector(const QPODVector &);
   QPODVector &operator=(const QPODVector &);
   int m_count;
   int m_capacity;
   T *m_data;
};

QT_END_NAMESPACE

#endif
