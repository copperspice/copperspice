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

#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qgenericsequencetype_p.h"

#include "qtruthpredicate_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

TruthPredicate::TruthPredicate(const Expression::Ptr &sourceExpression,
                               const Expression::Ptr &predicate) : GenericPredicate(sourceExpression,
                                        predicate)
{
}

SequenceType::List TruthPredicate::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   result.append(CommonSequenceTypes::EBV);
   return result;
}

ExpressionVisitorResult::Ptr TruthPredicate::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

QT_END_NAMESPACE
