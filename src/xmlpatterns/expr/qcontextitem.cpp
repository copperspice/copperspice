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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qcontextitem_p.h"

using namespace QPatternist;

Item ContextItem::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return context->contextItem();
}

Expression::Ptr ContextItem::compress(const StaticContext::Ptr &context)
{
   m_itemType = context->contextItemType();
   return EmptyContainer::compress(context);
}

Expression::Ptr ContextItem::typeCheck(const StaticContext::Ptr &context,
                                       const SequenceType::Ptr &reqType)
{
   m_itemType = context->contextItemType();
   return EmptyContainer::typeCheck(context, reqType);
}

SequenceType::Ptr ContextItem::staticType() const
{
   /* We test m_itemType here because Patternist View calls staticType() before the typeCheck()
    * stage. */
   if (m_itemType) {
      return makeGenericSequenceType(m_itemType, Cardinality::exactlyOne());
   } else {
      return CommonSequenceTypes::ExactlyOneItem;
   }
}

Expression::Properties ContextItem::properties() const
{
   return DisableElimination | RequiresContextItem | EvaluationCacheRedundant;
}

ExpressionVisitorResult::Ptr ContextItem::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID ContextItem::id() const
{
   return IDContextItem;
}

ItemType::Ptr ContextItem::expectedContextItemType() const
{
   return BuiltinTypes::item;
}

const SourceLocationReflection *ContextItem::actualReflection() const
{
   if (m_expr) {
      return m_expr.data();
   } else {
      return this;
   }
}

void ContextItem::announceFocusType(const ItemType::Ptr &type)
{
   Q_ASSERT(type);
   m_itemType = type;
}
