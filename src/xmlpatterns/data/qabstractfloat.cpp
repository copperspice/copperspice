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

#include <stdlib.h>

template <const bool isDouble>
AbstractFloat<isDouble>::AbstractFloat(const xsDouble num) : m_value(num)
{
}

template <const bool isDouble>
Numeric::Ptr AbstractFloat<isDouble>::fromValue(const xsDouble num)
{
   return Numeric::Ptr(new AbstractFloat<isDouble>(num));
}

template <const bool isDouble>
AtomicValue::Ptr AbstractFloat<isDouble>::fromLexical(const QString &strNumeric)
{
   if (strNumeric == QLatin1String("NaN")) {
      return isDouble ? CommonValues::DoubleNaN : CommonValues::FloatNaN;
   } else if (strNumeric == QLatin1String("-INF")) {
      return isDouble ? CommonValues::NegativeInfDouble : CommonValues::NegativeInfFloat;
   } else if (strNumeric == QLatin1String("INF")) {
      return isDouble ? CommonValues::InfDouble : CommonValues::InfFloat;
   }

   const QString toUpper(strNumeric.toUpper());
   if (toUpper == QLatin1String("-INF") ||
         toUpper == QLatin1String("INF")  ||
         toUpper == QLatin1String("+INF") ||
         toUpper == QLatin1String("NAN")) {
      return ValidationError::createError();
   }

   bool conversionOk = false;
   const xsDouble num = strNumeric.toDouble(&conversionOk);

   if (conversionOk) {
      return AtomicValue::Ptr(new AbstractFloat<isDouble>(num));
   } else {
      return ValidationError::createError();
   }
}

template <const bool isDouble>
int AbstractFloat<isDouble>::internalSignbit(const xsDouble num)
{
   Q_ASSERT_X(sizeof(xsDouble) == 8 || sizeof(xsDouble) == 4, Q_FUNC_INFO,
              "Implementation of signbit assumes xsDouble is 64 bits");

   union {
      xsDouble asDouble;
      qint64 asInt;
   } value;

   value.asDouble = num;

   // highest bit, the 64'th for those who have 64 bit floats, is the sign bit.
   if constexpr (sizeof(xsDouble) == 8) {
      return value.asInt >> 63;
   } else {
      return value.asInt >> 31;
   }
}

template <const bool isDouble>
bool AbstractFloat<isDouble>::isEqual(const xsDouble a, const xsDouble b)
{
   if (qIsInf(a)) {
      return qIsInf(b) && internalSignbit(a) == internalSignbit(b);
   } else if (qIsInf(b)) {
      return qIsInf(a) && internalSignbit(a) == internalSignbit(b);
   } else {
      return qAbs(a - b) <= 2.2204460492503131e-16 * qAbs(a);
   }
}

template <const bool isDouble>
bool AbstractFloat<isDouble>::isZero() const
{
   return AbstractFloat<isDouble>::isEqual(m_value, 0.0);
}

template <const bool isDouble>
bool AbstractFloat<isDouble>::evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const
{
   if (isZero() || qIsNaN(m_value)) {
      return false;
   } else {
      return true;
   }
}

template <const bool isDouble>
QString AbstractFloat<isDouble>::stringValue() const
{
   if (qIsNaN(m_value)) {
      return QLatin1String("NaN");
   } else if (qIsInf(m_value)) {
      return internalSignbit(m_value) == 0 ? QLatin1String("INF") : QLatin1String("-INF");
   }
   else if (0.000001 <= qAbs(m_value) && qAbs(m_value) < 1000000.0) {
      return Decimal::toString(toDecimal());
   }
   else if (isZero()) {
      return internalSignbit(m_value) == 0 ? QLatin1String("0") : QLatin1String("-0");
   } else {
      int sign;
      int decimalPoint;
      char *result = nullptr;
      static_cast<void>(qdtoa(m_value, -1, 0, &decimalPoint, &sign, nullptr, &result));

      const QString qret(QString::fromLatin1(result));
      free(result);

      QString valueAsString;

      if (sign) {
         valueAsString += QLatin1Char('-');
      }

      valueAsString += qret.at(0);
      valueAsString += QLatin1Char('.');

      if (1 == qret.size()) {
         valueAsString += QLatin1Char('0');
      } else {
         valueAsString += qret.mid(1);
      }

      valueAsString += QLatin1Char('E');
      decimalPoint--;
      valueAsString += QString::number(decimalPoint);
      return valueAsString;
   }
}

template <const bool isDouble>
xsDouble AbstractFloat<isDouble>::toDouble() const
{
   return m_value;
}

template <const bool isDouble>
xsInteger AbstractFloat<isDouble>::toInteger() const
{
   return static_cast<xsInteger>(m_value);
}

template <const bool isDouble>
xsFloat AbstractFloat<isDouble>::toFloat() const
{
   /* No cast, since xsFloat and xsDouble are typedef'ed with the same type. */
   return m_value;
}

template <const bool isDouble>
xsDecimal AbstractFloat<isDouble>::toDecimal() const
{
   return static_cast<xsDecimal>(m_value);
}

template <const bool isDouble>
Numeric::Ptr AbstractFloat<isDouble>::round() const
{
   return AbstractFloat<isDouble>::fromValue(static_cast<xsDouble>(roundFloat(m_value)));
}

template <const bool isDouble>
Numeric::Ptr AbstractFloat<isDouble>::roundHalfToEven(const xsInteger precision) const
{
   if (isNaN() || isInf() || isZero()) {
      return Numeric::Ptr(const_cast<AbstractFloat<isDouble> *>(this));
   } else {
      // cast to double helps finding the correct pow() version on irix-cc
      const xsDouble powered = pow(double(10), double(precision));
      xsDouble val = powered * m_value;
      bool isHalf = false;

      if (val - 0.5 == ::floor(val)) {
         isHalf = true;
      }

      val = m_value * powered + 0.5;
      val = ::floor(val);

      if (isHalf /*&& isOdd(val) or? TODO */) {
         val -= 1;
      }

      val /= powered;

      return fromValue(val);
   }
}

template <const bool isDouble>
Numeric::Ptr AbstractFloat<isDouble>::floor() const
{
   return AbstractFloat<isDouble>::fromValue(static_cast<xsDouble>(::floor(m_value)));
}

template <const bool isDouble>
Numeric::Ptr AbstractFloat<isDouble>::ceiling() const
{
   return AbstractFloat<isDouble>::fromValue(static_cast<xsDouble>(ceil(m_value)));
}

template <const bool isDouble>
Numeric::Ptr AbstractFloat<isDouble>::abs() const
{
   // must use fabs() instead of qAbs() because qAbs() does npt return 0 for -0.0.
   return AbstractFloat<isDouble>::fromValue(static_cast<xsDouble>(fabs(m_value)));
}

template <const bool isDouble>
bool AbstractFloat<isDouble>::isNaN() const
{
   return qIsNaN(m_value);
}

template <const bool isDouble>
bool AbstractFloat<isDouble>::isInf() const
{
   return qIsInf(m_value);
}

template <const bool isDouble>
ItemType::Ptr AbstractFloat<isDouble>::type() const
{
   return isDouble ? BuiltinTypes::xsDouble : BuiltinTypes::xsFloat;
}

template <const bool isDouble>
Item AbstractFloat<isDouble>::toNegated() const
{
   return fromValue(-m_value).data();
}

template <const bool isDouble>
bool AbstractFloat<isDouble>::isSigned() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return false;
}

template <const bool isDouble>
quint64 AbstractFloat<isDouble>::toUnsignedInteger() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return 0;
}

