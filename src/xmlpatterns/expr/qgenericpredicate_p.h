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

#ifndef QGenericPredicate_P_H
#define QGenericPredicate_P_H

#include <qpaircontainer_p.h>

namespace QPatternist {

class GenericPredicate : public PairContainer
{
 public:
   static Expression::Ptr create(const Expression::Ptr &sourceExpression,
                                 const Expression::Ptr &predicateExpression,
                                 const StaticContext::Ptr &context,
                                 const QSourceLocation &location);

   static Expression::Ptr createFirstItem(const Expression::Ptr &sourceExpression);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   inline Item mapToItem(const Item &subject, const DynamicContext::Ptr &) const;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   ID id() const override;

   Properties properties() const override;

   QString description() const override;

 protected:
   GenericPredicate(const Expression::Ptr &sourceExpression, const Expression::Ptr &predicate);

   ItemType::Ptr newFocusType() const override;

 private:
   typedef QExplicitlySharedDataPointer<const GenericPredicate> ConstPtr;
};

}

#endif
