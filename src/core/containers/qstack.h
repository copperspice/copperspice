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

#ifndef QSTACK_H
#define QSTACK_H

#include <qcontainerfwd.h>
#include <qvector.h>

template <class T>
class QStack : public QVector<T>
{
 public:
   using difference_type = typename QVector<T>::difference_type;
   using pointer         = typename QVector<T>::pointer;
   using reference       = typename QVector<T>::reference;
   using size_type       = typename QVector<T>::difference_type;
   using value_type      = typename QVector<T>::value_type;

   using allocator_type  = typename QVector<T>::allocator_type;

   using iterator        = typename QVector<T>::iterator;
   using const_iterator  = typename QVector<T>::const_iterator;

   using const_pointer   = typename QVector<T>::const_pointer;
   using const_reference = typename QVector<T>::const_reference;

   using reverse_iterator       = typename QVector<T>::reverse_iterator;
   using const_reverse_iterator = typename QVector<T>::const_reverse_iterator;

   QStack() = default;
   ~QStack () = default;

   void swap(QStack<T> &other) {
      QVector<T>::swap(other);
   }

   void push(const T &value) {
      QVector<T>::append(value);
   }

   T pop();

   reference top();
   const_reference top() const;
};

template <class T>
inline T QStack<T>::pop()
{
   Q_ASSERT(! this->isEmpty());

   T value = this->last();
   this->pop_back();

   return value;
}

template <class T>
inline typename QStack<T>::reference QStack<T>::top()
{
   Q_ASSERT(! this->isEmpty());
   return this->last();
}

template <class T>
inline typename QStack<T>::const_reference QStack<T>::top() const
{
   Q_ASSERT(! this->isEmpty());
   return this->last();
}

#endif
