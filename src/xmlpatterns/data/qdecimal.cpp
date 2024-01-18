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

#include <math.h>

#include "qabstractfloat_p.h"
#include "qatomictype_p.h"
#include "qbuiltintypes_p.h"
#include "qvalidationerror_p.h"

#include "qdecimal_p.h"

using namespace QPatternist;

Decimal::Decimal(const xsDecimal num) : m_value(num)
{
}

Decimal::Ptr Decimal::fromValue(const xsDecimal num)
{
   return Decimal::Ptr(new Decimal(num));
}

AtomicValue::Ptr Decimal::fromLexical(const QString &strNumeric)
{
   /* QString::toDouble() handles the whitespace facet. */
   const QString strNumericTrimmed(strNumeric.trimmed());

   /* Block these out, as QString::toDouble() supports them. */
   if (strNumericTrimmed.compare(QLatin1String("-INF"), Qt::CaseInsensitive) == 0
         || strNumericTrimmed.compare(QLatin1String("INF"), Qt::CaseInsensitive)  == 0
         || strNumericTrimmed.compare(QLatin1String("+INF"), Qt::CaseInsensitive)  == 0
         || strNumericTrimmed.compare(QLatin1String("nan"), Qt::CaseInsensitive)  == 0
         || strNumericTrimmed.contains(QLatin1Char('e'))
         || strNumericTrimmed.contains(QLatin1Char('E'))) {
      return ValidationError::createError();
   }

   bool conversionOk = false;
   const xsDecimal num = strNumericTrimmed.toDouble(&conversionOk);

   if (conversionOk) {
      return AtomicValue::Ptr(new Decimal(num));
   } else {
      return ValidationError::createError();
   }
}

bool Decimal::evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const
{
   return !Double::isEqual(m_value, 0.0);
}

QString Decimal::stringValue() const
{
   return toString(m_value);
}

QString Decimal::toString(const xsDecimal value)
{
   /*
    * If SV is in the value space of xs:integer, that is, if there are no
    * significant digits after the decimal point, then the value is converted
    * from an xs:decimal to an xs:integer and the resulting xs:integer is
    * converted to an xs:string using the rule above.
    */
   if (Double::isEqual(::floor(value), value)) {
      /* The static_cast is identical to Integer::toInteger(). */
      return QString::number(static_cast<xsInteger>(value));
   } else {
      int sign;
      int decimalPoint;
      char *result = nullptr;
      static_cast<void>(qdtoa(value, 0, 0, &decimalPoint, &sign, nullptr, &result));
      /* If the copy constructor is used instead of QString::operator=(),
       * it doesn't compile. I have no idea why. */
      const QString qret(QString::fromLatin1(result));
      free(result);

      QString valueAsString;

      if (sign) {
         valueAsString += QLatin1Char('-');
      }

      if (0 < decimalPoint) {
         valueAsString += qret.left(decimalPoint);
         valueAsString += QLatin1Char('.');
         if (qret.size() <= decimalPoint) {
            valueAsString += QLatin1Char('0');
         } else {
            valueAsString += qret.mid(decimalPoint);
         }
      } else {
         valueAsString += QLatin1Char('0');
         valueAsString += QLatin1Char('.');

         for (int d = decimalPoint; d < 0; d++) {
            valueAsString += QLatin1Char('0');
         }

         valueAsString += qret;
      }

      return valueAsString;
   }
}

ItemType::Ptr Decimal::type() const
{
   return BuiltinTypes::xsDecimal;
}

xsDouble Decimal::toDouble() const
{
   return static_cast<xsDouble>(m_value);
}

xsInteger Decimal::toInteger() const
{
   return static_cast<xsInteger>(m_value);
}

xsFloat Decimal::toFloat() const
{
   return static_cast<xsFloat>(m_value);
}

xsDecimal Decimal::toDecimal() const
{
   return m_value;
}

quint64 Decimal::toUnsignedInteger() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return 0;
}

Numeric::Ptr Decimal::round() const
{
   return Numeric::Ptr(new Decimal(roundFloat(m_value)));
}

Numeric::Ptr Decimal::roundHalfToEven(const xsInteger /*scale*/) const
{
   return Numeric::Ptr();
}

Numeric::Ptr Decimal::floor() const
{
   return Numeric::Ptr(new Decimal(static_cast<xsDecimal>(::floor(m_value))));
}

Numeric::Ptr Decimal::ceiling() const
{
   return Numeric::Ptr(new Decimal(static_cast<xsDecimal>(ceil(m_value))));
}

Numeric::Ptr Decimal::abs() const
{
   return Numeric::Ptr(new Decimal(static_cast<xsDecimal>(fabs(m_value))));
}

bool Decimal::isNaN() const
{
   return false;
}

bool Decimal::isInf() const
{
   return false;
}

Item Decimal::toNegated() const
{
   if (AbstractFloat<true>::isEqual(m_value, 0.0)) {
      return fromValue(0).data();
   } else {
      return fromValue(-m_value).data();
   }
}

bool Decimal::isSigned() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return false;
}
