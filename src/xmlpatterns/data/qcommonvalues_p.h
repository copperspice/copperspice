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

#ifndef QCommonValues_P_H
#define QCommonValues_P_H

#include <qdaytimeduration_p.h>
#include <qyearmonthduration_p.h>
#include <qemptyiterator_p.h>

namespace QPatternist {

class CommonValues
{
 public:
   static const AtomicValue::Ptr EmptyString;
   static const AtomicValue::Ptr TrueString;
   static const AtomicValue::Ptr UntypedAtomicFalse;
   static const AtomicValue::Ptr UntypedAtomicTrue;
   static const AtomicValue::Ptr FalseString;
   static const AtomicValue::Ptr BooleanTrue;
   static const AtomicValue::Ptr BooleanFalse;
   static const AtomicValue::Ptr DoubleNaN;
   static const AtomicValue::Ptr FloatNaN;

   static const Item IntegerZero;
   static const AtomicValue::Ptr EmptyAnyURI;
   static const EmptyIterator<Item>::Ptr emptyIterator;

   static const AtomicValue::Ptr NegativeInfFloat;
   static const AtomicValue::Ptr InfFloat;
   static const AtomicValue::Ptr NegativeInfDouble;
   static const AtomicValue::Ptr InfDouble;
   static const AtomicValue::Ptr FloatOne;
   static const AtomicValue::Ptr DoubleOne;
   static const AtomicValue::Ptr DecimalOne;
   static const Item IntegerOne;

   static const Item IntegerOneNegative;
   static const AtomicValue::Ptr DoubleZero;
   static const AtomicValue::Ptr FloatZero;
   static const AtomicValue::Ptr DecimalZero;
   static const DayTimeDuration::Ptr DayTimeDurationZero;
   static const DayTimeDuration::Ptr YearMonthDurationZero;

 private:
   inline CommonValues();

   CommonValues(const CommonValues &) = delete;
   CommonValues &operator=(const CommonValues &) = delete;
};
}

#endif
