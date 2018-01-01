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

#ifndef QSTACK_H
#define QSTACK_H

#include <qvector.h>

template<class T>
class QStack : public QVector<T>
{
   public:
      using const_reference = typename QVector<T>::const_reference;
      using reference       = typename QVector<T>::reference;

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

template<class T>
inline T QStack<T>::pop()
{
   Q_ASSERT(! this->isEmpty());

   T value = this->last();
   this->pop_back();

   return value;
}

template<class T>
inline typename QStack<T>::reference QStack<T>::top()
{
   Q_ASSERT(! this->isEmpty());
   return this->last();
}

template<class T>
inline typename QStack<T>::const_reference QStack<T>::top() const
{
   Q_ASSERT(! this->isEmpty());
   return this->last();
}

#endif
