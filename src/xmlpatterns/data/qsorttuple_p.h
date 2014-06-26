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

#ifndef QSortTuple_P_H
#define QSortTuple_P_H

#include <qitem_p.h>
#include <qitem_p.h>
#include <qitemtype_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class SortTuple : public AtomicValue
{
 public:
   /**
    * @p aSortKeys may be empty.
    */
   inline SortTuple(const Item::Iterator::Ptr &aValue,
                    const Item::Vector &aSortKeys) : m_sortKeys(aSortKeys),
      m_value(aValue) {
      Q_ASSERT(m_value);
      Q_ASSERT(!m_sortKeys.isEmpty());
   }

   /**
    * A smart pointer wrapping SortTuple instances.
    */
   typedef QExplicitlySharedDataPointer<SortTuple> Ptr;

   /**
    * This function is sometimes called by Literal::description().
    * This function simply returns "SortTuple".
    */
   virtual QString stringValue() const;

   /**
    * @short Always asserts.
    */
   virtual Item::Iterator::Ptr typedValue() const;

   /**
    * @short Always asserts.
    */
   virtual bool isAtomicValue() const;

   /**
    * @short Always asserts.
    */
   virtual bool isNode() const;

   /**
    * @short Always asserts.
    */
   virtual bool hasError() const;

   virtual ItemType::Ptr type() const;

   inline const Item::Vector &sortKeys() const {
      return m_sortKeys;
   }

   inline const Item::Iterator::Ptr &value() const {
      return m_value;
   }

 private:
   const Item::Vector          m_sortKeys;
   const Item::Iterator::Ptr   m_value;
};
}

QT_END_NAMESPACE

#endif
