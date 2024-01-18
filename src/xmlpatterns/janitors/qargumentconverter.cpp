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

#include <qargumentconverter_p.h>

#include <qitemmappingiterator_p.h>
#include <qsequencemappingiterator_p.h>

using namespace QPatternist;

ArgumentConverter::ArgumentConverter(const Expression::Ptr &operand,
                                     const ItemType::Ptr &reqType) : UntypedAtomicConverter(operand, reqType)
{
}

ExpressionVisitorResult::Ptr ArgumentConverter::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Item::Iterator::Ptr ArgumentConverter::mapToSequence(const Item &item,
      const DynamicContext::Ptr &context) const
{
   if (item.isAtomicValue() && !BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(item.type())) {
      return makeSingletonIterator(item);
   } else {
      /* We're using UntypedAtomicConverter::mapToItem(). */
      return makeItemMappingIterator<Item>(ConstPtr(this),
                                           item.sequencedTypedValue(),
                                           context);
   }
}

Item::Iterator::Ptr ArgumentConverter::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return makeSequenceMappingIterator<Item>(ConstPtr(this),
          m_operand->evaluateSequence(context),
          context);
}

Item ArgumentConverter::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item item(m_operand->evaluateSingleton(context));

   if (item) {
      return mapToItem(item, context);
   } else { /* Empty is allowed. ArgumentConverter doesn't care about cardinality. */
      return Item();
   }
}

SequenceType::List ArgumentConverter::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

SequenceType::Ptr ArgumentConverter::staticType() const
{
   return CommonSequenceTypes::ZeroOrMoreAtomicTypes;
}
