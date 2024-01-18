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

#include "qbuiltintypes_p.h"
#include "qcommonnamespaces_p.h"
#include "qcommonsequencetypes_p.h"
#include "qpatternistlocale_p.h"
#include "qqnamevalue_p.h"
#include "qatomicstring_p.h"

#include "qattributenamevalidator_p.h"

using namespace QPatternist;

AttributeNameValidator::AttributeNameValidator(const Expression::Ptr &source) : SingleContainer(source)
{
}

Item AttributeNameValidator::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item name(m_operand->evaluateSingleton(context));
   const QXmlName qName(name.as<QNameValue>()->qName());

   if (qName.namespaceURI() == StandardNamespaces::xmlns) {
      context->error(QtXmlPatterns::tr("The namespace URI in the name for a computed attribute cannot be %1.")
                     .formatArg(formatURI(CommonNamespaces::XMLNS)),
                     ReportContext::XQDY0044, this);

      return Item(); /* Silence warning. */

   } else if (qName.namespaceURI() == StandardNamespaces::empty && qName.localName() == StandardLocalNames::xmlns) {
      context->error(QtXmlPatterns::tr("The name for a computed attribute cannot have the namespace URI %1 with the local name %2.")
                     .formatArg(formatURI(CommonNamespaces::XMLNS)).formatArg(formatKeyword("xmlns")), ReportContext::XQDY0044, this);

      return Item(); /* Silence warning. */

   } else if (!qName.hasPrefix() && qName.hasNamespace()) {
      return Item(QNameValue::fromValue(context->namePool(), QXmlName(qName.namespaceURI(), qName.localName(), StandardPrefixes::ns0)));
   } else {
      return name;
   }
}

SequenceType::Ptr AttributeNameValidator::staticType() const
{
   return m_operand->staticType();
}

SequenceType::List AttributeNameValidator::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ExactlyOneQName);
   return result;
}

ExpressionVisitorResult::Ptr AttributeNameValidator::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
