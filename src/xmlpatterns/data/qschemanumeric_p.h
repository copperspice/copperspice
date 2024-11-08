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

   static AtomicValue::Ptr fromLexical(const QString &number);

   virtual xsDouble toDouble() const = 0;
   virtual xsInteger toInteger() const = 0;
   virtual quint64 toUnsignedInteger() const = 0;

   virtual xsFloat toFloat() const = 0;
   virtual xsFloat toDecimal() const = 0;

   virtual Numeric::Ptr round() const = 0;
   virtual Numeric::Ptr roundHalfToEven(const xsInteger scale) const = 0;
   virtual Numeric::Ptr floor() const = 0;
   virtual Numeric::Ptr ceiling() const = 0;
   virtual Numeric::Ptr abs() const = 0;

   virtual bool isNaN() const = 0;
   virtual bool isInf() const = 0;

   virtual Item toNegated() const = 0;

   virtual bool isSigned() const = 0;

 protected:
   static xsDouble roundFloat(const xsDouble val);
};

}

#endif
