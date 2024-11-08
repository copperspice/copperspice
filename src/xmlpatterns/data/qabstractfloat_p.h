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

   static bool isEqual(const xsDouble a, const xsDouble b);

   bool evaluateEBV(const QExplicitlySharedDataPointer<DynamicContext> &) const override;

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
   static inline int internalSignbit(const xsDouble v);
   inline bool isZero() const;

   const xsDouble m_value;
};

template <const bool isDouble>
Numeric::Ptr createFloat(const xsDouble num);

#include "qabstractfloat.cpp"

typedef AbstractFloat<true> Double;
typedef AbstractFloat<false> Float;

}

#endif
