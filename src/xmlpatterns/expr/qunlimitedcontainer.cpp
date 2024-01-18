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

#include "qunlimitedcontainer_p.h"

using namespace QPatternist;

UnlimitedContainer::UnlimitedContainer(const Expression::List &ops) : m_operands(ops)
{
}

void UnlimitedContainer::setOperands(const Expression::List &list)
{
   m_operands = list;
}

Expression::List UnlimitedContainer::operands() const
{
   return m_operands;
}

bool UnlimitedContainer::compressOperands(const StaticContext::Ptr &context)
{
   const Expression::List::iterator end(m_operands.end());
   Expression::List::iterator it(m_operands.begin());
   int evaled = 0;

   for (; it != end; ++it) {
      Q_ASSERT((*it));
      rewrite((*it), (*it)->compress(context), context);
      if ((*it)->isEvaluated()) {
         ++evaled;
      }
   }

   return evaled == m_operands.count();
}
