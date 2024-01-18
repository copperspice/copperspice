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

#ifndef QEmptyIterator_P_H
#define QEmptyIterator_P_H

#include <qabstractxmlforwarditerator_p.h>
#include <qprimitives_p.h>

namespace QPatternist {

template<typename T>
class EmptyIterator : public QAbstractXmlForwardIterator<T>
{
 public:
   /**
    * @returns always a default constructed value, T().
    */
   T next() override {
      return T();
   }

   /**
    * @returns always a default constructed value, T().
    */
   T current() const override {
      return T();
   }

   /**
    * @returns always 0.
    */
   xsInteger position() const override {
      return 0;
   }

   /**
    * @returns always @c this, the reverse of <tt>()</tt> is <tt>()</tt>.
    */
   typename QAbstractXmlForwardIterator<T>::Ptr toReversed() override {
      return typename QAbstractXmlForwardIterator<T>::Ptr(const_cast<EmptyIterator<T> *>(this));
   }

   /**
    * @returns always 0
    */
   xsInteger count() override {
      return 0;
   }

   /**
    * @returns @c this
    */
   typename QAbstractXmlForwardIterator<T>::Ptr copy() const override {
      return typename QAbstractXmlForwardIterator<T>::Ptr(const_cast<EmptyIterator *>(this));
   }

 protected:
   friend class CommonValues;
};

template<typename T>
static inline typename QAbstractXmlForwardIterator<T>::Ptr makeEmptyIterator()
{
   return typename QAbstractXmlForwardIterator<T>::Ptr(new EmptyIterator<T>());
}

}

#endif
