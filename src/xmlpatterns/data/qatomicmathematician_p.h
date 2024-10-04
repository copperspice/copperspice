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

#ifndef QAtomicMathematician_P_H
#define QAtomicMathematician_P_H

#include <QFlags>
#include <qdynamiccontext_p.h>
#include <qitem_p.h>
#include <qatomictypedispatch_p.h>

namespace QPatternist {
class AtomicMathematician : public AtomicTypeVisitorResult
{
 public:
   virtual ~AtomicMathematician();

   typedef QExplicitlySharedDataPointer<AtomicMathematician> Ptr;

   enum Operator {
      Div         = 1,
      IDiv        = 2,
      Substract   = 4,
      Mod         = 8,
      Multiply    = 16,
      Add         = 32
   };

   typedef QFlags<Operator> Operators;

   virtual Item calculate(const Item &operand1,
                          const Operator op,
                          const Item &operand2,
                          const QExplicitlySharedDataPointer<DynamicContext> &context) const = 0;

   static QString displayName(const AtomicMathematician::Operator op);

};
Q_DECLARE_OPERATORS_FOR_FLAGS(AtomicMathematician::Operators)
}

#endif
