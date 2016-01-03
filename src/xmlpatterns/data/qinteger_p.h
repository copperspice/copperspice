/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QInteger_P_H
#define QInteger_P_H

#include <qschemanumeric_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class Integer : public Numeric
{
 public:

   typedef Numeric::Ptr Ptr;

   static AtomicValue::Ptr fromLexical(const QString &strNumeric);

   static Item fromValue(const xsInteger num);

   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const;

   virtual QString stringValue() const;
   virtual ItemType::Ptr type() const;

   virtual xsDouble toDouble() const;
   virtual xsInteger toInteger() const;
   virtual xsFloat toFloat() const;
   virtual xsDecimal toDecimal() const;

   virtual Numeric::Ptr round() const;
   virtual Numeric::Ptr roundHalfToEven(const xsInteger scale) const;
   virtual Numeric::Ptr floor() const;
   virtual Numeric::Ptr ceiling() const;
   virtual Numeric::Ptr abs() const;
   virtual qulonglong toUnsignedInteger() const;

   /**
    * @returns always @c false, @c xs:integer doesn't have
    * not-a-number in its value space.
    */
   virtual bool isNaN() const;

   /**
    * @returns always @c false, @c xs:integer doesn't have
    * infinity in its value space.
    */
   virtual bool isInf() const;
   virtual Item toNegated() const;

   /**
    * @short Returns always @c true.
    */
   virtual bool isSigned() const;
 protected:
   Integer(const xsInteger num);

 private:
   const xsInteger m_value;
};
}

QT_END_NAMESPACE

#endif
