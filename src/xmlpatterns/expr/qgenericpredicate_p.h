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

   /**
    * Creates a predicate expression that filters the items gained
    * from evaluating @p sourceExpression through the filter @p predicateExpression.
    *
    * This function performs type analyzis on the passed expressions, and may
    * return more specialized expressions depending on the analyzis.
    *
    * If @p predicateExpression is an invalid predicate, an error is issued
    * via the @p context.
    */
   static Expression::Ptr create(const Expression::Ptr &sourceExpression,
                                 const Expression::Ptr &predicateExpression,
                                 const StaticContext::Ptr &context,
                                 const QSourceLocation &location);

   static Expression::Ptr createFirstItem(const Expression::Ptr &sourceExpression);

   /**
    * Creates a source iterator which is passed to the ItemMappingIterator
    * and the Focus. The ItemMappingIterator modifies it with
    * its QAbstractXmlForwardIterator::next() calls, and since the Focus references the same QAbstractXmlForwardIterator,
    * the focus is automatically moved.
    */
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;

   /**
    * Doesn't return the first item from calling evaluateSequence(), but does the mapping
    * manually. This avoid allocating an ItemMappingIterator.
    */
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

   inline Item mapToItem(const Item &subject, const DynamicContext::Ptr &) const;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   ID id() const override;

   /**
    * @returns always CreatesFocusForLast.
    */
   Properties properties() const override;

   QString description() const override;

 protected:

   /**
    * Creates a GenericPredicate which filters the items from the @p sourceExpression
    * through @p predicate.
    *
    * This constructor is protected. The proper way to create predicates is via the static
    * create() function.
    */
   GenericPredicate(const Expression::Ptr &sourceExpression,
                    const Expression::Ptr &predicate);

   /**
    * @returns the ItemType of the first operand's staticType().
    */
   ItemType::Ptr newFocusType() const override;

 private:
   typedef QExplicitlySharedDataPointer<const GenericPredicate> ConstPtr;
};
}

#endif
