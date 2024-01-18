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

/*
 * @file
 * @short This file is included by qatomicmathematicians_p.h
 * if you need some includes, put them in qabstractfloatmathematician_p.h, outside of the namespace.
 */

template <const bool isDouble>
Item AbstractFloatMathematician<isDouble>::calculate(const Item &o1, const Operator op, const Item &o2,
      const QExplicitlySharedDataPointer<DynamicContext> &context) const
{
   const Numeric *const num1 = o1.template as<Numeric>();
   const Numeric *const num2 = o2.template as<Numeric>();

   switch (op) {
      case Div:
         return toItem(AbstractFloat<isDouble>::fromValue(num1->toDouble() / num2->toDouble()));
      case IDiv: {
         if (num1->isNaN() || num2->isNaN()) {
            context->error(QtXmlPatterns::tr("No operand in an integer division, %1, can be %2.")
                           .formatArg(formatKeyword("idiv")).formatArg(formatData("NaN")), ReportContext::FOAR0002, this);

         } else if (num1->isInf()) {
            context->error(QtXmlPatterns::tr("The first operand in an integer division, %1, cannot be infinity (%2).")
                           .formatArg(formatKeyword("idiv")).formatArg(formatData("INF")), ReportContext::FOAR0002, this);

         } else if (num2->toInteger() == 0)
            context->error(QtXmlPatterns::tr("The second operand in a division, %1, cannot be zero (%2).")
                           .formatArg(formatKeyword("idiv")).formatArg(formatData("0")),
                           ReportContext::FOAR0001, this);

         return Integer::fromValue(static_cast<xsInteger>(num1->toDouble() / num2->toDouble()));
      }

      case Substract:
         return toItem(AbstractFloat<isDouble>::fromValue(num1->toDouble() - num2->toDouble()));
      case Mod:
         return toItem(AbstractFloat<isDouble>::fromValue(::fmod(num1->toDouble(), num2->toDouble())));
      case Multiply:
         return toItem(AbstractFloat<isDouble>::fromValue(num1->toDouble() * num2->toDouble()));
      case Add:
         return toItem(AbstractFloat<isDouble>::fromValue(num1->toDouble() + num2->toDouble()));
   }

   Q_ASSERT(false);
   return Item(); /* GCC unbarfer. */
}

