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
 * NOTE: This file is included by qabstractfloatcasters_p.h
 * if you need some includes, put them in qabstractfloatcasters_p.h (outside of the namespace)
 */

template <const bool isDouble>
Item NumericToAbstractFloatCaster<isDouble>::castFrom(const Item &from,
      const QExplicitlySharedDataPointer<DynamicContext> &) const
{
   // toDouble() returns same thing than toFloat()
   return toItem(AbstractFloat<isDouble>::fromValue(from.template as<Numeric>()->toDouble()));
}

template <const bool isDouble>
Item StringToAbstractFloatCaster<isDouble>::castFrom(const Item &from,
      const QExplicitlySharedDataPointer<DynamicContext> &) const
{
   return toItem(AbstractFloat<isDouble>::fromLexical(from.stringValue()));
}

template <const bool isDouble>
Item BooleanToAbstractFloatCaster<isDouble>::castFrom(const Item &from,
      const QExplicitlySharedDataPointer<DynamicContext> &context) const
{
   // RVCT doesn't like using template parameter in trinary operator when the trinary operator result is
   // passed directly into another constructor.
   bool tempDouble = isDouble;
   if (from.template as<AtomicValue>()->evaluateEBV(context)) {
      return tempDouble ? toItem(CommonValues::DoubleOne) : toItem(CommonValues::FloatOne);
   } else {
      return tempDouble ? toItem(CommonValues::DoubleZero) : toItem(CommonValues::FloatZero);
   }
}

