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

#include "qebvextractor_p.h"

#include <qboolean_p.h>
#include <qbuiltintypes_p.h>
#include <qcommonsequencetypes_p.h>
#include <qgenericsequencetype_p.h>

using namespace QPatternist;

EBVExtractor::EBVExtractor(const Expression::Ptr &operand) : SingleContainer(operand)
{
}

bool EBVExtractor::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return m_operand->evaluateEBV(context);
}

Expression::Ptr EBVExtractor::typeCheck(const StaticContext::Ptr &context,
                                        const SequenceType::Ptr &reqType)
{
   return typeCheck<SingleContainer>(context, reqType, this);
}

SequenceType::Ptr EBVExtractor::staticType() const
{
   return makeGenericSequenceType(BuiltinTypes::xsBoolean, Cardinality::exactlyOne());
}

SequenceType::List EBVExtractor::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

const SourceLocationReflection *EBVExtractor::actualReflection() const
{
   return m_operand->actualReflection();
}

ExpressionVisitorResult::Ptr EBVExtractor::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
