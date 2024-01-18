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

#ifndef QSingletonIterator_P_H
#define QSingletonIterator_P_H

#include <qabstractxmlforwarditerator_p.h>
#include <qprimitives_p.h>

namespace QPatternist {

template<typename T>
class SingletonIterator : public QAbstractXmlForwardIterator<T>
{
 public:
   /**
    * Creates an iterator over @p item.
    *
    * @note item may not be @c null. Use the EmptyIterator for
    * the empty sequence
    */
   SingletonIterator(const T &item) : m_item(item),
      m_position(0) {
      Q_ASSERT(!qIsForwardIteratorEnd(item));
   }

   T next() override {
      switch (m_position) {
         case 0: {
            ++m_position;
            return m_item;
         }
         case 1: {
            m_position = -1;
            return T();
         }
         default: {
            Q_ASSERT(m_position == -1);
            return T();
         }
      }
   }

   T current() const override {
      if (m_position == 1) {
         return m_item;
      } else {
         return T();
      }
   }

   xsInteger position() const override {
      return m_position;
   }

   /**
    * @returns a copy of this instance, rewinded to the beginning.
    */
   typename QAbstractXmlForwardIterator<T>::Ptr toReversed() override {
      return typename QAbstractXmlForwardIterator<T>::Ptr(new SingletonIterator<T>(m_item));
   }

   /**
    * @returns always 1
    */
   xsInteger count() override {
      return 1;
   }

   typename QAbstractXmlForwardIterator<T>::Ptr copy() const  override {
      return typename QAbstractXmlForwardIterator<T>::Ptr(new SingletonIterator(m_item));
   }

 private:
   const T m_item;
   qint8 m_position;
};

/**
 * @short An object generator for SingletonIterator.
 *
 * makeSingletonIterator() is a convenience function for avoiding specifying
 * the full template instantiation for SingletonIterator. Conceptually, it
 * is identical to Qt's qMakePair().
 *
 * @relates SingletonIterator
 */
template<typename T>
inline typename SingletonIterator<T>::Ptr makeSingletonIterator(const T &item)
{
   return typename SingletonIterator<T>::Ptr(new SingletonIterator<T>(item));
}

}

#endif
