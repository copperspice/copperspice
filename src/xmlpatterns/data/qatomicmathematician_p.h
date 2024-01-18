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
      /**
       * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-divide">XQuery 1.0
       * and XPath 2.0 Functions and Operators, 6.2.4 op:numeric-divide</a>
       */
      Div         = 1,

      /**
       * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-integer-divide">XQuery 1.0
       * and XPath 2.0 Functions and Operators, 6.2.5 op:numeric-integer-divide</a>
       */
      IDiv        = 2,

      /**
       * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-subtract">XQuery 1.0
       * and XPath 2.0 Functions and Operators, 6.2.2 op:numeric-subtract</a>
       */
      Substract   = 4,

      /**
       * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-mod">XQuery 1.0
       * and XPath 2.0 Functions and Operators, 6.2.6 op:numeric-mod</a>
       */
      Mod         = 8,

      /**
       * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-multiply">XQuery 1.0
       * and XPath 2.0 Functions and Operators, 6.2.3 op:numeric-multiply</a>
       */
      Multiply    = 16,

      /**
       * @see <a href="http://www.w3.org/TR/xpath-functions/#func-numeric-add">XQuery 1.0
       * and XPath 2.0 Functions and Operators, 6.2.1 op:numeric-add</a>
       */
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
