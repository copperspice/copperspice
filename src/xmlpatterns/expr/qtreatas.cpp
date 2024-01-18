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
#include "qtypechecker_p.h"
#include "qtreatas_p.h"

using namespace QPatternist;

TreatAs::TreatAs(const Expression::Ptr &operand,
                 const SequenceType::Ptr &reqType) : SingleContainer(operand),
   m_reqType(reqType)
{
   Q_ASSERT(reqType);
}

Expression::Ptr TreatAs::typeCheck(const StaticContext::Ptr &context,
                                   const SequenceType::Ptr &reqType)
{
   Q_ASSERT(context);
   Q_ASSERT(reqType);

   /* Apply function conversion with the special error code XPDY0050. After that, we
    * let the regular typeCheck() function be invoked on the operand before we rewrite
    * to it. Hence is applyFunctionConversion() called twice, which doesn't break anything,
    * but indeed is redundant. */
   const Expression::Ptr treated(TypeChecker::applyFunctionConversion(m_operand,
                                 m_reqType,
                                 context,
                                 ReportContext::XPDY0050));
   return treated->typeCheck(context, reqType);
}

ExpressionVisitorResult::Ptr TreatAs::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

SequenceType::Ptr TreatAs::staticType() const
{
   return m_reqType;
}

SequenceType::List TreatAs::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

