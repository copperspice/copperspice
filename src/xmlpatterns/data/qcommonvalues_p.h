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
   /**
    * An empty, zero-length string.
    *
    * @note It is not @c null, but empty.
    */
   static const AtomicValue::Ptr EmptyString;

   /**
    * The string "true", the lexical representation of
    * @c xs:boolean's value @c true.
    */
   static const AtomicValue::Ptr TrueString;

   /**
    * The string "false", the lexical representation of
    * @c xs:boolean's value @c false.
    */
   static const AtomicValue::Ptr UntypedAtomicFalse;

   /**
    * The string "true", the lexical representation of
    * @c xs:boolean's value @c true.
    */
   static const AtomicValue::Ptr UntypedAtomicTrue;

   /**
    * The string "false", the lexical representation of
    * @c xs:boolean's value @c false.
    */
   static const AtomicValue::Ptr FalseString;

   /**
    * @returns a Boolean instance carrying the boolean value @c true.
    * Use this value instead of Boolean::fromValue() if you
    * know what boolean value you need.
    */
   static const AtomicValue::Ptr BooleanTrue;

   /**
    * @returns a Boolean instance carrying the boolean value @c true.
    * Use this value instead of Boolean::fromValue() if you
    * know what boolean value you need.
    */
   static const AtomicValue::Ptr BooleanFalse;

   /**
    * Not-a-Numeric typed as @c xs:double.
    */
   static const AtomicValue::Ptr DoubleNaN;

   /**
    * Not-a-Number typed as @c xs:float, <tt>xs:float("NaN")</tt>.
    */
   static const AtomicValue::Ptr FloatNaN;

   /**
    * Zero(0) typed as @c xs:integer, <tt>xs:integer("0")</tt>.
    */
   static const Item IntegerZero;

   /**
    * An empty, "", @c xs:anyURI.
    */
   static const AtomicValue::Ptr EmptyAnyURI;

   /**
    * The empty sequence.
    */
   static const EmptyIterator<Item>::Ptr emptyIterator;

   /**
    * <tt>xs:float("-INF")</tt>
    */
   static const AtomicValue::Ptr NegativeInfFloat;

   /**
    * <tt>xs:float("INF")</tt>
    */
   static const AtomicValue::Ptr InfFloat;

   /**
    * <tt>xs:double("-INF")</tt>
    */
   static const AtomicValue::Ptr NegativeInfDouble;

   /**
    * <tt>xs:double("INF")</tt>
    */
   static const AtomicValue::Ptr InfDouble;

   /**
    * <tt>xs:float("1")</tt>
    */
   static const AtomicValue::Ptr FloatOne;
   /**
    * <tt>xs:double("1")</tt>
    */
   static const AtomicValue::Ptr DoubleOne;
   /**
    * <tt>xs:decimal("1")</tt>
    */
   static const AtomicValue::Ptr DecimalOne;

   /**
    * <tt>xs:integer("1")</tt>
    */
   static const Item IntegerOne;

   /**
    * <tt>xs:integer("-1")</tt>
    */
   static const Item IntegerOneNegative;

   /**
    * <tt>xs:double("0")</tt>
    */
   static const AtomicValue::Ptr DoubleZero;

   /**
    * <tt>xs:float("0")</tt>
    */
   static const AtomicValue::Ptr FloatZero;
   /**
    * <tt>xs:integer("0")</tt>
    */
   static const AtomicValue::Ptr DecimalZero;

   /**
    * The @c xs:dayTimeDuration value PT0S
    */
   static const DayTimeDuration::Ptr DayTimeDurationZero;

   /**
    * The @c xs:yearMonthDuration value P0M
    */
   static const DayTimeDuration::Ptr YearMonthDurationZero;

 private:
   /**
    * The constructor is private because this class is not meant to be instantiated,
    * but should only be used via its static const members.
    */
   inline CommonValues();

   CommonValues(const CommonValues &) = delete;
   CommonValues &operator=(const CommonValues &) = delete;
};
}

#endif
