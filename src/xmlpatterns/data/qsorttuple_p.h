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

#ifndef QSortTuple_P_H
#define QSortTuple_P_H

#include <qitem_p.h>
#include <qitem_p.h>
#include <qitemtype_p.h>

namespace QPatternist {

class SortTuple : public AtomicValue
{
 public:
   SortTuple(const Item::Iterator::Ptr &aValue, const Item::Vector &aSortKeys) : m_sortKeys(aSortKeys), m_value(aValue)
   {
      Q_ASSERT(m_value);
      Q_ASSERT(!m_sortKeys.isEmpty());
   }

   typedef QExplicitlySharedDataPointer<SortTuple> Ptr;

   QString stringValue() const override;

   virtual Item::Iterator::Ptr typedValue() const;

   virtual bool isAtomicValue() const;

   virtual bool isNode() const;

   bool hasError() const override;

   ItemType::Ptr type() const override;

   const Item::Vector &sortKeys() const {
      return m_sortKeys;
   }

   const Item::Iterator::Ptr &value() const {
      return m_value;
   }

 private:
   const Item::Vector          m_sortKeys;
   const Item::Iterator::Ptr   m_value;
};
}

#endif
