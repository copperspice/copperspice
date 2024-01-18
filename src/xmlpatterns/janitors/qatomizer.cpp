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

#include <qatomizer_p.h>

#include <qatomictype_p.h>
#include <qbuiltintypes_p.h>
#include <qcommonsequencetypes_p.h>
#include <qgenericsequencetype_p.h>
#include <qsequencemappingiterator_p.h>

using namespace QPatternist;

Atomizer::Atomizer(const Expression::Ptr &operand) : SingleContainer(operand)
{
}

Item::Iterator::Ptr Atomizer::mapToSequence(const Item &item, const DynamicContext::Ptr &) const
{
   /* Function & Operators, 2.4.2 fn:data, says "If the node does not have a
    * typed value an error is raised [err:FOTY0012]."
    * When does a node not have a typed value? */
   Q_ASSERT(item);
   return item.sequencedTypedValue();
}

Item::Iterator::Ptr Atomizer::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return makeSequenceMappingIterator<Item>(ConstPtr(this),
          m_operand->evaluateSequence(context),
          context);
}

Item Atomizer::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item item(m_operand->evaluateSingleton(context));

   if (!item) { /* Empty is allowed, cardinality is considered '?' */
      return Item();
   }

   const Item::Iterator::Ptr it(mapToSequence(item, context));
   Q_ASSERT_X(it, Q_FUNC_INFO, "A valid QAbstractXmlForwardIterator must always be returned.");

   Item result(it->next());
   Q_ASSERT_X(!it->next(), Q_FUNC_INFO,
              "evaluateSingleton should never be used if the cardinality is two or more");

   return result;
}

Expression::Ptr Atomizer::typeCheck(const StaticContext::Ptr &context,
                                    const SequenceType::Ptr &reqType)
{
   /* Compress -- the earlier the better. */
   if (BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(m_operand->staticType()->itemType())) {
      return m_operand->typeCheck(context, reqType);
   }

   return SingleContainer::typeCheck(context, reqType);
}

SequenceType::Ptr Atomizer::staticType() const
{
   const SequenceType::Ptr opt(m_operand->staticType());
   return makeGenericSequenceType(opt->itemType()->atomizedType(),
                                  opt->cardinality());
}

SequenceType::List Atomizer::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

ExpressionVisitorResult::Ptr Atomizer::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

const SourceLocationReflection *Atomizer::actualReflection() const
{
   return m_operand->actualReflection();
}
