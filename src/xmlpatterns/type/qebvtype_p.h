/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QEBVType_P_H
#define QEBVType_P_H

#include <qatomictype_p.h>
#include <qsequencetype_p.h>

namespace QPatternist {

class EBVType : public ItemType, public SequenceType
{
 public:
   typedef QExplicitlySharedDataPointer<EBVType> Ptr;

   bool itemMatches(const Item &item) const override;
   bool xdtTypeMatches(const ItemType::Ptr &other) const override;

   QString displayName(const NamePool::Ptr &np) const override;

   ItemType::Ptr itemType() const override;

   Cardinality cardinality() const override;

   bool isAtomicType() const override;
   ItemType::Ptr atomizedType() const override;

   ItemType::Ptr xdtSuperType() const override;

   bool isNodeType() const override;

 protected:
   friend class CommonSequenceTypes;
   EBVType();
};

}

#endif
