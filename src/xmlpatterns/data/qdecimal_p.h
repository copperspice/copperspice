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

#ifndef QDecimal_P_H
#define QDecimal_P_H

#include <qschemanumeric_p.h>

Q_CORE_EXPORT char *qdtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve, char **resultp);

namespace QPatternist {

class Decimal : public Numeric
{
 public:

   static Decimal::Ptr fromValue(const xsDecimal num);

   static AtomicValue::Ptr fromLexical(const QString &strNumeric);

   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const override;

   QString stringValue() const override;

   /**
    * @returns always BuiltinTypes::xsDecimal
    */
   ItemType::Ptr type() const override;

   xsDouble toDouble() const override;
   xsInteger toInteger() const override;
   xsFloat toFloat() const override;
   xsDecimal toDecimal() const override;
   quint64 toUnsignedInteger() const override;

   Numeric::Ptr round() const override;
   Numeric::Ptr roundHalfToEven(const xsInteger scale) const override;
   Numeric::Ptr floor() const override;
   Numeric::Ptr ceiling() const override;
   Numeric::Ptr abs() const override;

   /**
    * @returns always @c false, xs:decimal doesn't have
    * not-a-number in its value space.
    */
   bool isNaN() const override;

   /**
    * @returns always @c false, xs:decimal doesn't have
    * infinity in its value space.
    */
   bool isInf() const override;

   Item toNegated() const override;

   /**
    * Converts @p value into a canonical string representation for @c xs:decimal. This
    * function is used internally by various classes. Users probably wants to call
    * stringValue() which in turn calls this function.
    */
   static QString toString(const xsDecimal value);

   bool isSigned() const override;

 protected:
   Decimal(const xsDecimal num);

 private:
   const xsDecimal m_value;
};
}

#endif
