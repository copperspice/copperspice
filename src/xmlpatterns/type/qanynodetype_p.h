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

#ifndef QAnyNodeType_P_H
#define QAnyNodeType_P_H

#include <qatomictype_p.h>
#include <qitem_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class AnyNodeType : public ItemType
{
 public:

   typedef QExplicitlySharedDataPointer<AnyNodeType> Ptr;

   virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;
   virtual bool itemMatches(const Item &item) const;
   virtual QString displayName(const NamePool::Ptr &np) const;

   virtual ItemType::Ptr xdtSuperType() const;

   virtual bool isNodeType() const;
   virtual bool isAtomicType() const;

   /**
    * @see <a href="http://www.w3.org/TR/xpath-datamodel/#acc-summ-typed-value">XQuery 1.0
    * and XPath 2.0 Data Model, G.15 dm:typed-value Accessor</a>
    */
   virtual ItemType::Ptr atomizedType() const;

   /**
    * @returns the node kind this node ItemType tests for. If it matches any node, zero is returned.
    */
   virtual QXmlNodeModelIndex::NodeKind nodeKind() const;

   virtual PatternPriority patternPriority() const;

 protected:
   friend class BuiltinTypes;

   inline AnyNodeType() {
   }

};
}

QT_END_NAMESPACE

#endif
