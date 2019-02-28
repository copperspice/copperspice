/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef ARRAY_H
#define ARRAY_H

#include <QtGlobal>
#include <algorithm>

/* A simple, non-shared array. */

template <class T>
class Array
{
   Q_DISABLE_COPY(Array)

 public:
   enum { initialSize = 5 };

   typedef T *const_iterator;

   explicit Array(size_t size = 0) : data(0), m_capacity(0), m_size(0) {
      if (size) {
         resize(size);
      }
   }
   ~Array() {
      delete [] data;
   }

   T *data;
   inline size_t size() const          {
      return m_size;
   }
   inline const_iterator begin() const {
      return data;
   }
   inline const_iterator end() const   {
      return data + m_size;
   }

   inline void append(const T &value) {
      const size_t oldSize = m_size;
      resize(m_size + 1);
      data[oldSize] = value;
   }

   inline void resize(size_t size) {
      if (size > m_size) {
         reserve(size > 1 ? size + size / 2 : size_t(initialSize));
      }
      m_size = size;
   }

   void reserve(size_t capacity) {
      if (capacity > m_capacity) {
         const T *oldData = data;
         data = new T[capacity];

         if (oldData) {
            std::copy(oldData, oldData + m_size, data);
            delete [] oldData;
         }

         m_capacity = capacity;
      }
   }

 private:
   size_t m_capacity;
   size_t m_size;
};

#endif
