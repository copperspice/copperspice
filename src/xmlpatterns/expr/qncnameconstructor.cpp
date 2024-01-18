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
#include "qcommonsequencetypes_p.h"
#include "qatomicstring_p.h"
#include "qncnameconstructor_p.h"


using namespace QPatternist;

NCNameConstructor::NCNameConstructor(const Expression::Ptr &source) : SingleContainer(source)
{
}

Item NCNameConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(context);

   /* Apply the whitespace facet for when casting to xs:NCName. */
   const QString lexNCName(m_operand->evaluateSingleton(context).stringValue().trimmed());

   validateTargetName<DynamicContext::Ptr, ReportContext::XQDY0064, ReportContext::XQDY0041>(lexNCName, context, this);
   return AtomicString::fromValue(lexNCName);
}

Expression::Ptr NCNameConstructor::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   if (BuiltinTypes::xsNCName->xdtTypeMatches(m_operand->staticType()->itemType())) {
      return m_operand->typeCheck(context, reqType);
   } else {
      return SingleContainer::typeCheck(context, reqType);
   }
}

SequenceType::Ptr NCNameConstructor::staticType() const
{
   return CommonSequenceTypes::ExactlyOneString;
}

SequenceType::List NCNameConstructor::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ExactlyOneString);

   return result;
}

ExpressionVisitorResult::Ptr NCNameConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}


