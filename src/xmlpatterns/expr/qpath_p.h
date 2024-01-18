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

#ifndef QPath_P_H
#define QPath_P_H

#include "qpaircontainer_p.h"

namespace QPatternist {

class Path : public PairContainer
{
 public:
   enum Kind {
      RegularPath = 1,
      XSLTForEach,
      ForApplyTemplate
   };

   Path(const Expression::Ptr &operand1, const Expression::Ptr &operand2, const Kind kind = RegularPath);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const override;

   inline Item::Iterator::Ptr mapToSequence(const Item &item, const DynamicContext::Ptr &context) const;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   Properties properties() const override;
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   /**
    * @returns the item type of the last step's static type.
    */
   ItemType::Ptr newFocusType() const override;

   ID id() const override;

   inline void setLast();

   Kind kind() const {
      return m_kind;
   }

 private:
   typedef QExplicitlySharedDataPointer<const Path> ConstPtr;

   /**
    * One might think this block exists for preventing multiple
    * NodeSortExpressions to be created. However, that is not an issue,
    * since NodeSortExpression optimizes this away anyway.
    *
    * The real reason is to avoid infinite recursion. When our typeCheck()
    * forwards on the type check to the just created
    * NodeSortExpression, it in turn calls typeCheck() on its child, which
    * is this Path. Rince and repeat.
    *
    * We only create node sorts when we're a regular path expression, and
    * not when standing in as a generic map expression. */
   bool        m_hasCreatedSorter;

   /**
    * Whether this path is the step. For instance, in <tt>a/b/c</tt>, the
    * last path has @c c as the right operand.
    */
   bool        m_isLast;

   bool        m_checkXPTY0018;
   const Kind  m_kind;
};

void Path::setLast()
{
   m_isLast = true;
}

}

#endif
