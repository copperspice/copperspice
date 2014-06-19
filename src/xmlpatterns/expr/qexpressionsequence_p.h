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

#ifndef Patternist_ExpressionSequence_H
#define Patternist_ExpressionSequence_H

#include "qunlimitedcontainer_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
class ExpressionSequence : public UnlimitedContainer
{
 public:
   /**
    * Creates an ExpressionSequence with the operands @p operands. @p operands
    * must contain two or more Expression instances.
    */
   ExpressionSequence(const Expression::List &operands);

   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;

   /**
    * Forwards the call to its children.
    */
   virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

   virtual SequenceType::List expectedOperandTypes() const;
   virtual SequenceType::Ptr staticType() const;

   /**
    * Removes any empty sequences, typically "()", from its list of children. If
    * after that rewrite has no children, it rewrites itself to the CommonValues::empty;
    * if it has only one, it rewrites to the child.
    *
    * This optimization is not very usable by itself, but potentially becomes effective after other
    * optimizations have rewritten themselves into empty sequences. Thus,
    * saving memory consumption and runtime overhead.
    */
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);

   inline Item::Iterator::Ptr mapToSequence(const Expression::Ptr &, const DynamicContext::Ptr &) const;

   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType);
   /**
    * @returns Expression::DisableElimination, plus the union
    * of all this ExpressionSequence's children's properties. If any child
    * does not have IsEvaluated, it is removed from the result.
    */
   virtual Expression::Properties properties() const;
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual ID id() const;
 private:
   typedef QExplicitlySharedDataPointer<const ExpressionSequence> ConstPtr;
};
}

QT_END_NAMESPACE

#endif
