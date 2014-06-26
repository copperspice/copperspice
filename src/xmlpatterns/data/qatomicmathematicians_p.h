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

#ifndef QAtomicMathematicians_P_H
#define QAtomicMathematicians_P_H

#include <qatomicmathematician_p.h>
#include <qsourcelocationreflection_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class DecimalMathematician : public AtomicMathematician
   , public DelegatingSourceLocationReflection
{
 public:
   inline DecimalMathematician(const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r) {
   }

   virtual Item calculate(const Item &o1,
                          const Operator op,
                          const Item &o2,
                          const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class IntegerMathematician : public AtomicMathematician
   , public DelegatingSourceLocationReflection
{
 public:
   inline IntegerMathematician(const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r) {
   }

   virtual Item calculate(const Item &o1,
                          const Operator op,
                          const Item &o2,
                          const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class DurationNumericMathematician : public AtomicMathematician
   , public DelegatingSourceLocationReflection
{
 public:
   inline DurationNumericMathematician(const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r) {
   }

   virtual Item calculate(const Item &o1,
                          const Operator op,
                          const Item &o2,
                          const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class DurationDurationDivisor : public AtomicMathematician
{
 public:
   virtual Item calculate(const Item &o1,
                          const Operator op,
                          const Item &o2,
                          const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class DurationDurationMathematician : public AtomicMathematician
{
 public:
   virtual Item calculate(const Item &o1,
                          const Operator op,
                          const Item &o2,
                          const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class OperandSwitcherMathematician : public AtomicMathematician
{
 public:
   /**
    * Creates an OperandSwitcherMathematician.
    *
    * @param mathematician the AtomicMathematician this OperandSwitcherMathematician
    * should switch the operands for. Must be a non @c null, valid pointer.
    */
   OperandSwitcherMathematician(const AtomicMathematician::Ptr &mathematician);

   /**
    * Switch @p o1 and @p o2, and returns the value from the AtomicMathematician
    * this OperandSwitcherMathematician represents.
    */
   virtual Item calculate(const Item &o1,
                          const Operator op,
                          const Item &o2,
                          const QExplicitlySharedDataPointer<DynamicContext> &context) const;
 private:
   const AtomicMathematician::Ptr m_mather;
};

class DateTimeDurationMathematician : public AtomicMathematician
   , public DelegatingSourceLocationReflection
{
 public:

   inline DateTimeDurationMathematician(const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r) {
   }

   /**
    * @p o1 is an AbstractDateTime and @p o2 is an AbstractDuration.
    *
    */
   virtual Item calculate(const Item &o1,
                          const Operator op,
                          const Item &o2,
                          const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};


class AbstractDateTimeMathematician : public AtomicMathematician
{
 public:
   virtual Item calculate(const Item &o1,
                          const Operator op,
                          const Item &o2,
                          const QExplicitlySharedDataPointer<DynamicContext> &context) const;
};
}

QT_END_NAMESPACE

#endif
