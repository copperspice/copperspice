/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef Patternist_EmptyIterator_P_H
#define Patternist_EmptyIterator_P_H

#include "qabstractxmlforwarditerator_p.h"
#include "qprimitives_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {

template<typename T> class EmptyIterator : public QAbstractXmlForwardIterator<T>
{
 public:
   /**
    * @returns always a default constructed value, T().
    */
   virtual T next() {
      return T();
   }

   /**
    * @returns always a default constructed value, T().
    */
   virtual T current() const {
      return T();
   }

   /**
    * @returns always 0.
    */
   virtual xsInteger position() const {
      return 0;
   }

   /**
    * @returns always @c this, the reverse of <tt>()</tt> is <tt>()</tt>.
    */
   virtual typename QAbstractXmlForwardIterator<T>::Ptr toReversed() {
      return typename QAbstractXmlForwardIterator<T>::Ptr(const_cast<EmptyIterator<T> *>(this));
   }

   /**
    * @returns always 0
    */
   virtual xsInteger count() {
      return 0;
   }

   /**
    * @returns @c this
    */
   virtual typename QAbstractXmlForwardIterator<T>::Ptr copy() const {
      return typename QAbstractXmlForwardIterator<T>::Ptr(const_cast<EmptyIterator *>(this));
   }

 protected:
   friend class CommonValues;
};

template<typename T>
static inline
typename QAbstractXmlForwardIterator<T>::Ptr
makeEmptyIterator()
{
   return typename QAbstractXmlForwardIterator<T>::Ptr(new EmptyIterator<T>());
}

}

QT_END_NAMESPACE

#endif
