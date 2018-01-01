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

#ifndef QUnaryExpression_P_H
#define QUnaryExpression_P_H

QT_BEGIN_NAMESPACE

#include <qarithmeticexpression_p.h>

namespace QPatternist {

class UnaryExpression : public ArithmeticExpression
{
 public:
   UnaryExpression(const AtomicMathematician::Operator op, const Expression::Ptr &operand,
                   const StaticContext::Ptr &context);

   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

 private:
   Q_DISABLE_COPY(UnaryExpression)
};
}

QT_END_NAMESPACE

#endif
