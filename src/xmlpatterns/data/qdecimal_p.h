/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDecimal_P_H
#define QDecimal_P_H

#include <qschemanumeric_p.h>

QT_BEGIN_NAMESPACE

Q_CORE_EXPORT char *qdtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve, char **resultp);

namespace QPatternist {

class Decimal : public Numeric
{
 public:

   static Decimal::Ptr fromValue(const xsDecimal num);

   static AtomicValue::Ptr fromLexical(const QString &strNumeric);

   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const;

   virtual QString stringValue() const;

   /**
    * @returns always BuiltinTypes::xsDecimal
    */
   virtual ItemType::Ptr type() const;

   virtual xsDouble toDouble() const;
   virtual xsInteger toInteger() const;
   virtual xsFloat toFloat() const;
   virtual xsDecimal toDecimal() const;
   virtual qulonglong toUnsignedInteger() const;

   virtual Numeric::Ptr round() const;
   virtual Numeric::Ptr roundHalfToEven(const xsInteger scale) const;
   virtual Numeric::Ptr floor() const;
   virtual Numeric::Ptr ceiling() const;
   virtual Numeric::Ptr abs() const;

   /**
    * @returns always @c false, xs:decimal doesn't have
    * not-a-number in its value space.
    */
   virtual bool isNaN() const;

   /**
    * @returns always @c false, xs:decimal doesn't have
    * infinity in its value space.
    */
   virtual bool isInf() const;

   virtual Item toNegated() const;

   /**
    * Converts @p value into a canonical string representation for @c xs:decimal. This
    * function is used internally by various classes. Users probably wants to call
    * stringValue() which in turn calls this function.
    */
   static QString toString(const xsDecimal value);

   virtual bool isSigned() const;

 protected:

   Decimal(const xsDecimal num);

 private:
   const xsDecimal m_value;
};
}

QT_END_NAMESPACE

#endif
