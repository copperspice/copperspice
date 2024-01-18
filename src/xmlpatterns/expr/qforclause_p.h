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

#ifndef QForClause_P_H
#define QForClause_P_H

#include <qpaircontainer_p.h>

namespace QPatternist {

class ForClause : public PairContainer
{
 public:
   /**
    * If @p positionSlot is -1, no positional variable will be used.
    */
   ForClause(const VariableSlotID varSlot,
             const Expression::Ptr &bindingSequence,
             const Expression::Ptr &returnExpression,
             const VariableSlotID positionSlot);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const override;

   SequenceType::Ptr staticType() const override;
   SequenceType::List expectedOperandTypes() const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   QList<QExplicitlySharedDataPointer<OptimizationPass> > optimizationPasses() const override;

   inline Item mapToItem(const Item &item, const DynamicContext::Ptr &context) const;
   ID id() const override;

   inline Item::Iterator::Ptr mapToSequence(const Item &item, const DynamicContext::Ptr &context) const;

   /**
    * Sets m_allowsMany properly.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

 private:
   inline void riggPositionalVariable(const DynamicContext::Ptr &context,
                                      const Item::Iterator::Ptr &source) const;

   typedef QExplicitlySharedDataPointer<const ForClause> ConstPtr;
   const VariableSlotID m_varSlot;
   const VariableSlotID m_positionSlot;
   /**
    * Initialized to @c false. This default is always safe.
    */
   bool m_allowsMany;
};

}

#endif
