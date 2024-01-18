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

#ifndef QNodeSort_P_H
#define QNodeSort_P_H

#include <qsinglecontainer_p.h>

namespace QPatternist {

class NodeSortExpression : public SingleContainer
{
 public:
   NodeSortExpression(const Expression::Ptr &operand);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   SequenceType::Ptr staticType() const override;
   SequenceType::List expectedOperandTypes() const override;
   Expression::Ptr compress(const StaticContext::Ptr &context) override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   Properties properties() const override;

   /**
    * Ensures that result delivered from @p operand, is in document order.
    */
   static Expression::Ptr wrapAround(const Expression::Ptr &operand, const StaticContext::Ptr &context);

 private:
   static inline bool lessThanUsingNodeModel(const Item &n1, const Item &n2);
};

}

#endif
