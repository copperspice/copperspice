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

#ifndef QAbstractFloat_P_H
#define QAbstractFloat_P_H

#include <math.h>
#include <qnumeric.h>
#include <qcommonvalues_p.h>
#include <qdecimal_p.h>
#include <qschemanumeric_p.h>
#include <qvalidationerror_p.h>
#include <qbuiltintypes_p.h>

namespace QPatternist {
template <const bool isDouble>
class AbstractFloat : public Numeric
{
 public:
   static Numeric::Ptr fromValue(const xsDouble num);
   static AtomicValue::Ptr fromLexical(const QString &strNumeric);

   /**
    * @todo more extensive docs.
    *
    * Performs floating point comparison.
    *
    * @returns @c true if @p a and @p are equal, otherwise @c false.
    */
   static bool isEqual(const xsDouble a, const xsDouble b);

   /**
    * Determines the Effective %Boolean Value of this number.
    *
    * @returns @c false if the number is 0 or @c NaN, otherwise @c true.
    */
   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const override;

   /**
    * Returns this AbstractFloat represented as an @c xs:string.
    *
    * @note In the XPath/XQuery languages, converting @c xs:double and @c xs:float
    * to @c xs:string is not specified in XML Schema 1.0 Part 2: Datatypes Second Edition,
    * but in XQuery 1.0 and XPath 2.0 Functions and Operators. This will change with W3C XML
    * Schema 1.1
    *
    * @see <a href="http://www.w3.org/TR/xpath-functions/#casting-to-string">XQuery 1.0
    * and XPath 2.0 Functions and Operators, 17.1.2 Casting to xs:string and xdt:untypedAtomic</a>
    */
   QString stringValue() const override;

   xsDouble toDouble() const override;
   xsInteger toInteger() const override;
   xsFloat toFloat() const override;
   xsDecimal toDecimal() const override;

   Numeric::Ptr round() const override;
   Numeric::Ptr roundHalfToEven(const xsInteger scale) const override;
   Numeric::Ptr floor() const override;
   Numeric::Ptr ceiling() const override;
   Numeric::Ptr abs() const override;

   bool isNaN() const override;
   bool isInf() const override;

   ItemType::Ptr type() const override;
   Item toNegated() const override;
   quint64 toUnsignedInteger() const override;

   bool isSigned() const override;

 protected:
   AbstractFloat(const xsDouble num);

 private:
   /**
    * From the Open Group's man page: "The signbit() macro shall return a
    * non-zero value if and only if the sign of its argument value is
    * negative."
    *
    * MS Windows doesn't have std::signbit() so here's
    * a reinvention of that function.
    */
   static inline int internalSignbit(const xsDouble v);
   inline bool isZero() const;

   const xsDouble m_value;
};

template <const bool isDouble>
Numeric::Ptr createFloat(const xsDouble num);

#include "qabstractfloat.cpp"

/**
 * @short An instantiation of AbsbstractFloat suitable for @c xs:double.
 *
 * @ingroup Patternist_xdm
 */
typedef AbstractFloat<true> Double;

/**
 * @short An instantiation of AbstractFloat suitable for @c xs:float.
 *
 * @ingroup Patternist_xdm
 */
typedef AbstractFloat<false> Float;
}

#endif
