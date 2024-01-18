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

#ifndef QInteger_P_H
#define QInteger_P_H

#include <qschemanumeric_p.h>

namespace QPatternist {
class Integer : public Numeric
{
 public:

   typedef Numeric::Ptr Ptr;

   static AtomicValue::Ptr fromLexical(const QString &strNumeric);

   static Item fromValue(const xsInteger num);

   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const  override;

   QString stringValue() const override;
   ItemType::Ptr type() const override;

   xsDouble toDouble() const override;
   xsInteger toInteger() const override;
   xsFloat toFloat() const override;
   xsDecimal toDecimal() const override;

   Numeric::Ptr round() const override;
   Numeric::Ptr roundHalfToEven(const xsInteger scale) const override;
   Numeric::Ptr floor() const override;
   Numeric::Ptr ceiling() const override;
   Numeric::Ptr abs() const override;
   quint64 toUnsignedInteger() const override;

   /**
    * @returns always @c false, @c xs:integer doesn't have
    * not-a-number in its value space.
    */
   bool isNaN() const  override;

   /**
    * @returns always @c false, @c xs:integer doesn't have
    * infinity in its value space.
    */
   bool isInf() const  override;
   Item toNegated() const  override;

   /**
    * @short Returns always @c true.
    */
   bool isSigned() const  override;

 protected:
   Integer(const xsInteger num);

 private:
   const xsInteger m_value;
};
}

#endif
