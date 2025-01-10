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

#ifndef QContextItem_P_H
#define QContextItem_P_H

#include <qemptycontainer_p.h>

namespace QPatternist {

class ContextItem : public EmptyContainer
{
 public:
   ContextItem(const Expression::Ptr &expr = Expression::Ptr())
      : m_expr(expr)
   {
   }

   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   SequenceType::Ptr staticType() const override;
   Expression::Properties properties() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   Expression::Ptr compress(const StaticContext::Ptr &context) override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   ID id() const override;

   ItemType::Ptr expectedContextItemType() const override;

   const SourceLocationReflection *actualReflection() const override;
   void announceFocusType(const ItemType::Ptr &type) override;

 private:
   ItemType::Ptr           m_itemType;
   const Expression::Ptr   m_expr;
};

}

#endif
