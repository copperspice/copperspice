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

#include <qlist.h>

#include "qtriplecontainer_p.h"

using namespace QPatternist;

TripleContainer::TripleContainer(const Expression::Ptr &operand1,
                                 const Expression::Ptr &operand2,
                                 const Expression::Ptr &operand3) : m_operand1(operand1),
   m_operand2(operand2),
   m_operand3(operand3)
{
   Q_ASSERT(operand1);
   Q_ASSERT(operand2);
   Q_ASSERT(operand3);
}

Expression::List TripleContainer::operands() const
{
   Expression::List result;
   result.append(m_operand1);
   result.append(m_operand2);
   result.append(m_operand3);
   return result;
}

void TripleContainer::setOperands(const Expression::List &ops)
{
   Q_ASSERT(ops.count() == 3);
   m_operand1 = ops.first();
   m_operand2 = ops.at(1);
   m_operand3 = ops.at(2);
}

bool TripleContainer::compressOperands(const StaticContext::Ptr &context)
{
   rewrite(m_operand1, m_operand1->compress(context), context);
   rewrite(m_operand2, m_operand2->compress(context), context);
   rewrite(m_operand3, m_operand3->compress(context), context);

   return m_operand1->isEvaluated() && m_operand2->isEvaluated() && m_operand3->isEvaluated();
}
