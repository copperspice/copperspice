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

#ifndef QSchemaNumeric_P_H
#define QSchemaNumeric_P_H

#include <qitem_p.h>
#include <qprimitives_p.h>

namespace QPatternist {

class Numeric : public AtomicValue
{
 public:

   typedef QExplicitlySharedDataPointer<Numeric> Ptr;

   /**
    * Creates a Numeric sub-class that is appropriate for @p number.
    *
    * @note usages of e/E is not handled; Double::fromLexical should
    * be used in that case. There is no function similar to fromLexical that also
    * takes double values into account(because that distinction is done in the scanner).
    *
    * Currently used in the parser to create appropriate expressions.
    */
   static AtomicValue::Ptr fromLexical(const QString &number);

   /**
    * @returns the particular number's value as a native representation of
    * the type xs:double. This can be considered that the value is cast to
    * xs:double.
    */
   virtual xsDouble toDouble() const = 0;

   /**
    * @returns the particular number's value as a native representation of
    * the type xs:integer. This can be considered that the value is cast to
    * xs:integer.
    */
   virtual xsInteger toInteger() const = 0;

   /**
    * @returns the number as an unsigned integer. If the value is not
    * unsigned, the code asserts and behavior is undefined.
    */
   virtual quint64 toUnsignedInteger() const = 0;

   /**
    * @returns the particular number's value as a native representation of
    * the type xs:float. This can be considered that the value is cast to
    * xs:float.
    */
   virtual xsFloat toFloat() const = 0;

   /**
    * @returns the particular number's value as a native representation of
    * the type xs:decimal. This can be considered that the value is cast to
    * xs:decimal.
    */
   virtual xsFloat toDecimal() const = 0;

   /**
    * Performs the algorithm specified for the function fn:round on this Numeric,
    * and whose result is returned.
    *
    * @see <a href="http://www.w3.org/TR/xpath-functions/#func-round">XQuery 1.0
    * and XPath 2.0 Functions and Operators, 6.4.4 fn:round</a>
    */
   virtual Numeric::Ptr round() const = 0;

   /**
    * Performs rounding as defined for the fn:round-half-to-even  on this Numeric,
    * and whose result is returned.
    *
    * @see <a href="http://www.w3.org/TR/xpath-functions/#func-round-half-to-even">XQuery 1.0
    * and XPath 2.0 Functions and Operators, 6.4.5 fn:round-half-to-even</a>
    */
   virtual Numeric::Ptr roundHalfToEven(const xsInteger scale) const = 0;

   /**
    * Performs the algorithm specified for the function fn:floor on this Numeric,
    * and whose result is returned.
    *
    * @see <a href="http://www.w3.org/TR/xpath-functions/#func-floor">XQuery 1.0
    * and XPath 2.0 Functions and Operators, 6.4.3 fn:floor</a>
    */
   virtual Numeric::Ptr floor() const = 0;

   /**
    * Performs the algorithm specified for the function fn:ceiling on this Numeric,
    * and whose result is returned.
    *
    * @see <a href="http://www.w3.org/TR/xpath-functions/#func-ceiling">XQuery 1.0
    * and XPath 2.0 Functions and Operators, 6.4.2 fn:ceiling</a>
    */
   virtual Numeric::Ptr ceiling() const = 0;

   /**
    * Performs the algorithm specified for the function fn:abs on this Numeric,
    * and whose result is returned.
    *
    * @see <a href="http://www.w3.org/TR/xpath-functions/#func-ceiling">XQuery 1.0
    * and XPath 2.0 Functions and Operators, 6.4.1 fn:abs</a>
    */
   virtual Numeric::Ptr abs() const = 0;

   /**
    * Determines whether this Numeric is not-a-number, @c NaN. For numeric types
    * that cannot represent @c NaN, this function should return @c false.
    *
    * @returns @c true if this Numeric is @c NaN
    */
   virtual bool isNaN() const = 0;

   /**
    * Determines whether this Numeric is an infinite number. Signedness
    * is irrelevant, -INF as well as INF is considered infinity.
    *
    * For numeric types that cannot represent infinity, such as xs:integer
    * , this function should return @c false.
    *
    * @returns @c true if this Numeric is an infinite number
    */
   virtual bool isInf() const = 0;

   /**
    * Unary minus.
    */
   virtual Item toNegated() const = 0;

   /**
    * @short Returns @c true if this value is signed. If @c false is
    * returned, the value is unsigned.
    *
    * For float and decimal values, @c xs:double, @c xs:float and @c
    * xs:decimal, the code asserts and behavior is undefined.
    */
   virtual bool isSigned() const = 0;

 protected:
   /**
    * @short Implements @c fn:round() for types implemented with floating
    * point.
    *
    * MS Windows and at least IRIX does not have C99's nearbyint() function(see the man
    * page), so we reinvent it.
    */
   static xsDouble roundFloat(const xsDouble val);
};
}

#endif
