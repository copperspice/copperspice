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

#include <quntypedatomicconverter_p.h>

#include <qitem_p.h>

#include <qcommonsequencetypes_p.h>
#include <qgenericsequencetype_p.h>
#include <qitemmappingiterator_p.h>

using namespace QPatternist;

UntypedAtomicConverter::UntypedAtomicConverter(const Expression::Ptr &operand,
      const ItemType::Ptr &reqType,
      const ReportContext::ErrorCode code) : SingleContainer(operand)
   , CastingPlatform<UntypedAtomicConverter, true>(code)
   , m_reqType(reqType)
{
   Q_ASSERT(reqType);
}

Item::Iterator::Ptr UntypedAtomicConverter::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return makeItemMappingIterator<Item>(ConstPtr(this),
                                        m_operand->evaluateSequence(context),
                                        context);
}

Item UntypedAtomicConverter::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item item(m_operand->evaluateSingleton(context));

   if (item) {
      return cast(item, context);
   } else { /* Empty is allowed. UntypedAtomicConverter doesn't care about cardinality. */
      return Item();
   }
}

Expression::Ptr UntypedAtomicConverter::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   const Expression::Ptr me(SingleContainer::typeCheck(context, reqType));

   /* Let the CastingPlatform look up its AtomicCaster. */
   prepareCasting(context, m_operand->staticType()->itemType());

   return me;
}

SequenceType::List UntypedAtomicConverter::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreAtomicTypes);
   return result;
}

SequenceType::Ptr UntypedAtomicConverter::staticType() const
{
   return makeGenericSequenceType(m_reqType,
                                  m_operand->staticType()->cardinality());
}

ExpressionVisitorResult::Ptr UntypedAtomicConverter::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

const SourceLocationReflection *UntypedAtomicConverter::actualReflection() const
{
   return m_operand.data();
}
