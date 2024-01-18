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

#ifndef QExpressionSequence_P_H
#define QExpressionSequence_P_H

#include <qunlimitedcontainer_p.h>

namespace QPatternist {

class ExpressionSequence : public UnlimitedContainer
{
 public:
   /**
    * Creates an ExpressionSequence with the operands @p operands. @p operands
    * must contain two or more Expression instances.
    */
   ExpressionSequence(const Expression::List &operands);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;

   /**
    * Forwards the call to its children.
    */
   void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const override;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;

   /**
    * Removes any empty sequences, typically "()", from its list of children. If
    * after that rewrite has no children, it rewrites itself to the CommonValues::empty;
    * if it has only one, it rewrites to the child.
    *
    * This optimization is not very usable by itself, but potentially becomes effective after other
    * optimizations have rewritten themselves into empty sequences. Thus,
    * saving memory consumption and runtime overhead.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   inline Item::Iterator::Ptr mapToSequence(const Expression::Ptr &, const DynamicContext::Ptr &) const;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   /**
    * @returns Expression::DisableElimination, plus the union
    * of all this ExpressionSequence's children's properties. If any child
    * does not have IsEvaluated, it is removed from the result.
    */
   Expression::Properties properties() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   ID id() const override;

 private:
   typedef QExplicitlySharedDataPointer<const ExpressionSequence> ConstPtr;
};

}

#endif
