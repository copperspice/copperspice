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

#ifndef QAnyNodeType_P_H
#define QAnyNodeType_P_H

#include <qatomictype_p.h>
#include <qitem_p.h>

namespace QPatternist {
class AnyNodeType : public ItemType
{
 public:
   typedef QExplicitlySharedDataPointer<AnyNodeType> Ptr;

   bool xdtTypeMatches(const ItemType::Ptr &other) const override;
   bool itemMatches(const Item &item) const override;
   QString displayName(const NamePool::Ptr &np) const override;

   ItemType::Ptr xdtSuperType() const override;

   bool isNodeType() const override;
   bool isAtomicType() const override;

   ItemType::Ptr atomizedType() const override;

   virtual QXmlNodeModelIndex::NodeKind nodeKind() const;
   virtual PatternPriority patternPriority() const;

 protected:
   friend class BuiltinTypes;

   AnyNodeType() {
   }
};

}

#endif
