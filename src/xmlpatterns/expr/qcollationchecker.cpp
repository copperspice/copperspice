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
#include "qxpathhelper_p.h"
#include "qcollationchecker_p.h"

using namespace QPatternist;

CollationChecker::CollationChecker(const Expression::Ptr &source) : SingleContainer(source)
{
}

Item CollationChecker::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item val(m_operand->evaluateSingleton(context));
   XPathHelper::checkCollationSupport<ReportContext::FOCH0002>(val.stringValue(), context, this);
   return val;
}

SequenceType::List CollationChecker::expectedOperandTypes() const
{
   SequenceType::List list;
   list.append(CommonSequenceTypes::ExactlyOneString);
   return list;
}

SequenceType::Ptr CollationChecker::staticType() const
{
   return m_operand->staticType();
}

ExpressionVisitorResult::Ptr CollationChecker::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
