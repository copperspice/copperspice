/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QFirstItemPredicate_P_H
#define QFirstItemPredicate_P_H

#include <qsinglecontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class FirstItemPredicate : public SingleContainer
{
 public:
   /**
    * Creates a FirstItemPredicate that filters @p source.
    */
   FirstItemPredicate(const Expression::Ptr &source);

   /**
    * @returns the first item, if any, from evaluating the source expression.
    */
   virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

   /**
    * @returns a list containing one CommonSequenceTypes::ZeroOrMoreItems instance.
    */
   virtual SequenceType::List expectedOperandTypes() const;

   /**
    * @returns a SequenceType where the item type is the same as the source expression
    * and where the cardinality is either Cardinality::zeroOrOne() or Cardinality::exactlyOne(),
    * depending on the source expression.
    */
   virtual SequenceType::Ptr staticType() const;
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

   /**
    * Rewrites <tt>expression[1][1]</tt> into <tt>expression[1]</tt>.
    */
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);

   /**
    * @returns always IDFirstItemPredicate.
    */
   virtual ID id() const;
};
}

QT_END_NAMESPACE

#endif
