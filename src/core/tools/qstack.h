/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSTACK_H
#define QSTACK_H

#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

template<class T>
class QStack : public QVector<T>
{
 public:
   inline QStack() {}
   inline ~QStack() {}
   inline void swap(QStack<T> &other) {
      QVector<T>::swap(other);   // prevent QVector<->QStack swaps
   }
   inline void push(const T &t) {
      QVector<T>::append(t);
   }
   T pop();
   T &top();
   const T &top() const;
};

template<class T>
inline T QStack<T>::pop()
{
   Q_ASSERT(!this->isEmpty());
   T t = this->data()[this->size() - 1];
   this->resize(this->size() - 1);
   return t;
}

template<class T>
inline T &QStack<T>::top()
{
   Q_ASSERT(!this->isEmpty());
   this->detach();
   return this->data()[this->size() - 1];
}

template<class T>
inline const T &QStack<T>::top() const
{
   Q_ASSERT(!this->isEmpty());
   return this->data()[this->size() - 1];
}

QT_END_NAMESPACE

#endif // QSTACK_H
