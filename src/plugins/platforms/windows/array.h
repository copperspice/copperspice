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

#ifndef ARRAY_H
#define ARRAY_H

#include <qglobal.h>
#include <algorithm>

// simple, non-shared array

template <class T>
class Array
{
 public:
   static constexpr const int initialSize = 5;

   using const_iterator = T*;

   explicit Array(size_t size = 0)
      : data(nullptr), m_capacity(0), m_size(0) {

      if (size) {
         resize(size);
      }
   }

   Array(const Array &) = delete;
   Array &operator=(const Array &) = delete;

   ~Array() {
      delete [] data;
   }

   size_t size() const {
      return m_size;
   }

   const_iterator begin() const {
      return data;
   }

   const_iterator end() const   {
      return data + m_size;
   }

   void append(const T &value) {
      const size_t oldSize = m_size;
      resize(m_size + 1);
      data[oldSize] = value;
   }

   void resize(size_t size) {
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

   T *data;

 private:
   size_t m_capacity;
   size_t m_size;
};

#endif
