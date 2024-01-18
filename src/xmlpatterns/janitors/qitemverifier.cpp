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

#include <qitemverifier_p.h>

#include <qcommonsequencetypes_p.h>
#include <qgenericsequencetype_p.h>
#include <qitemmappingiterator_p.h>
#include <qpatternistlocale_p.h>

using namespace QPatternist;

ItemVerifier::ItemVerifier(const Expression::Ptr &operand, const ItemType::Ptr &reqType,
                           const ReportContext::ErrorCode errorCode) : SingleContainer(operand),
   m_reqType(reqType),
   m_errorCode(errorCode)
{
   Q_ASSERT(reqType);
}

void ItemVerifier::verifyItem(const Item &item, const DynamicContext::Ptr &context) const
{
   if (m_reqType->itemMatches(item)) {
      return;
   }

   context->error(QtXmlPatterns::tr("The item %1 did not match the required type %2.")
                  .formatArgs(formatData(item.stringValue()), formatType(context->namePool(), m_reqType)), m_errorCode, this);
}

const SourceLocationReflection *ItemVerifier::actualReflection() const
{
   return m_operand->actualReflection();
}

Item ItemVerifier::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item item(m_operand->evaluateSingleton(context));

   if (item) {
      verifyItem(item, context);
      return item;
   } else {
      return Item();
   }
}

Item ItemVerifier::mapToItem(const Item &item, const DynamicContext::Ptr &context) const
{
   verifyItem(item, context);
   return item;
}

Item::Iterator::Ptr ItemVerifier::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return makeItemMappingIterator<Item>(ConstPtr(this),
                                        m_operand->evaluateSequence(context),
                                        context);
}

SequenceType::Ptr ItemVerifier::staticType() const
{
   return makeGenericSequenceType(m_reqType, m_operand->staticType()->cardinality());
}

SequenceType::List ItemVerifier::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

ExpressionVisitorResult::Ptr ItemVerifier::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
