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
#include "qpaircontainer_p.h"

using namespace QPatternist;

PairContainer::PairContainer(const Expression::Ptr &operand1,
                             const Expression::Ptr &operand2) : m_operand1(operand1),
   m_operand2(operand2)
{
   Q_ASSERT(m_operand1);
   Q_ASSERT(m_operand2);
}

Expression::List PairContainer::operands() const
{
   Expression::List list;
   list.append(m_operand1);
   list.append(m_operand2);
   return list;
}

void PairContainer::setOperands(const Expression::List &ops)
{
   Q_ASSERT(ops.count() == 2);
   m_operand1 = ops.first();
   m_operand2 = ops.last();
   Q_ASSERT(m_operand1);
   Q_ASSERT(m_operand2);
}

bool PairContainer::compressOperands(const StaticContext::Ptr &context)
{
   Q_ASSERT(m_operand1);
   Q_ASSERT(m_operand2);
   rewrite(m_operand1, m_operand1->compress(context), context);
   rewrite(m_operand2, m_operand2->compress(context), context);

   return m_operand1->isEvaluated() && m_operand2->isEvaluated();
}
