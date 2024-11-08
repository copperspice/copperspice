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

#ifndef QGenericSequenceType_P_H
#define QGenericSequenceType_P_H

#include <qcommonsequencetypes_p.h>
#include <qsequencetype_p.h>

namespace QPatternist {

class GenericSequenceType : public SequenceType
{
 public:
   GenericSequenceType(const ItemType::Ptr &itemType, const Cardinality &card);

   QString displayName(const NamePool::Ptr &np) const  override;

   Cardinality cardinality() const  override;
   ItemType::Ptr itemType() const  override;

 private:
   const ItemType::Ptr m_itemType;
   const Cardinality m_cardinality;
};

static inline SequenceType::Ptr makeGenericSequenceType(const ItemType::Ptr &itemType, const Cardinality &cardinality)
{
   if (cardinality.isEmpty()) {
      return CommonSequenceTypes::Empty;
   } else {
      return SequenceType::Ptr(new GenericSequenceType(itemType, cardinality));
   }
}

}

#endif
