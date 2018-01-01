/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qanyuri_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qpatternistlocale_p.h"
#include "qxmlutils_p.h"
#include "qcomputednamespaceconstructor_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

ComputedNamespaceConstructor::ComputedNamespaceConstructor(
   const Expression::Ptr &prefix, const Expression::Ptr &namespaceURI) : PairContainer(prefix, namespaceURI)
{
}

void ComputedNamespaceConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   const Item prefixItem(m_operand1->evaluateSingleton(context));
   const QString prefix(prefixItem ? prefixItem.stringValue() : QString());

   const Item namespaceItem(m_operand2->evaluateSingleton(context));
   const QString namespaceURI(namespaceItem ? namespaceItem.stringValue() : QString());

   if (namespaceURI.isEmpty()) {
      context->error(QtXmlPatterns::tr("In a namespace constructor, the value for a namespace cannot be an empty string."),
                     ReportContext::XTDE0930,
                     this);
   }

   /* One optimization could be to store a pointer to
    * the name pool as a member in order to avoid the virtual call(s). */
   const NamePool::Ptr np(context->namePool());

   if (!prefix.isEmpty() && !QXmlUtils::isNCName(prefix)) {
      context->error(QtXmlPatterns::tr("The prefix must be a valid %1, which %2 is not.")
                     .arg(formatType(np, BuiltinTypes::xsNCName),
                          formatKeyword(prefix)),
                     ReportContext::XTDE0920,
                     this);
   }
   const QXmlName binding(np->allocateBinding(prefix, namespaceURI));

   AnyURI::toQUrl<ReportContext::XTDE0905, DynamicContext::Ptr>(namespaceURI,
         context,
         this);

   if (binding.prefix() == StandardPrefixes::xmlns) {
      context->error(QtXmlPatterns::tr("The prefix %1 cannot be bound.")
                     .arg(formatKeyword(prefix)),
                     ReportContext::XTDE0920,
                     this);
   }

   if ((binding.prefix() == StandardPrefixes::xml && binding.namespaceURI() != StandardNamespaces::xml)
         ||
         (binding.prefix() != StandardPrefixes::xml && binding.namespaceURI() == StandardNamespaces::xml)) {
      context->error(QtXmlPatterns::tr("Only the prefix %1 can be bound to %2 and vice versa.")
                     .arg(formatKeyword(prefix), formatKeyword(namespaceURI)),
                     ReportContext::XTDE0925,
                     this);
   }

   context->outputReceiver()->namespaceBinding(binding);
}

SequenceType::Ptr ComputedNamespaceConstructor::staticType() const
{
   return CommonSequenceTypes::ExactlyOneAttribute;
}

SequenceType::List ComputedNamespaceConstructor::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrOneString);
   result.append(CommonSequenceTypes::ZeroOrOneString);
   return result;
}

Expression::Properties ComputedNamespaceConstructor::properties() const
{
   return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr ComputedNamespaceConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

QT_END_NAMESPACE
