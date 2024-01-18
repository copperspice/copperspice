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

#include "qcommonsequencetypes_p.h"
#include "qpatternistlocale_p.h"
#include "qqnamevalue_p.h"

#include "qqnameconstructor_p.h"

using namespace QPatternist;

QNameConstructor::QNameConstructor(const Expression::Ptr &source, const NamespaceResolver::Ptr &nsResolver) : SingleContainer(source),
   m_nsResolver(nsResolver)
{
   Q_ASSERT(m_nsResolver);
}

Item QNameConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(context);
   const QString lexQName(m_operand->evaluateSingleton(context).stringValue());

   const QXmlName expQName(expandQName<DynamicContext::Ptr,
                           ReportContext::XQDY0074,
                           ReportContext::XQDY0074>(lexQName,
                           context, m_nsResolver, this));

   return toItem(QNameValue::fromValue(context->namePool(), expQName));
}

QXmlName::NamespaceCode QNameConstructor::namespaceForPrefix(const QXmlName::PrefixCode prefix,
      const StaticContext::Ptr &context, const SourceLocationReflection *const r)
{
   Q_ASSERT(context);
   const QXmlName::NamespaceCode ns(context->namespaceBindings()->lookupNamespaceURI(prefix));

   if (ns == NamespaceResolver::NoBinding) {
      context->error(QtXmlPatterns::tr("No namespace binding exists for the prefix %1")
                     .formatArgs(formatKeyword(context->namePool()->stringForPrefix(prefix))), ReportContext::XPST0081, r);

      return NamespaceResolver::NoBinding;

   } else {
      return ns;
   }
}

SequenceType::Ptr QNameConstructor::staticType() const
{
   return CommonSequenceTypes::ExactlyOneQName;
}

SequenceType::List QNameConstructor::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ExactlyOneString);
   return result;
}

ExpressionVisitorResult::Ptr QNameConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

const SourceLocationReflection *QNameConstructor::actualReflection() const
{
   return m_operand.data();
}
