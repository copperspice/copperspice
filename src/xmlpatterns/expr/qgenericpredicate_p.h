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

#ifndef QGenericPredicate_P_H
#define QGenericPredicate_P_H

#include <qpaircontainer_p.h>

QT_BEGIN_NAMESPACE

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
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;

   /**
    * Doesn't return the first item from calling evaluateSequence(), but does the mapping
    * manually. This avoid allocating an ItemMappingIterator.
    */
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

   inline Item mapToItem(const Item &subject,
                         const DynamicContext::Ptr &) const;

   virtual SequenceType::List expectedOperandTypes() const;
   virtual SequenceType::Ptr staticType() const;
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual ID id() const;

   /**
    * @returns always CreatesFocusForLast.
    */
   virtual Properties properties() const;

   virtual QString description() const;

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
   virtual ItemType::Ptr newFocusType() const;

 private:
   typedef QExplicitlySharedDataPointer<const GenericPredicate> ConstPtr;
};
}

QT_END_NAMESPACE

#endif
