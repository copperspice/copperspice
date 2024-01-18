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

#ifndef QAtomicComparators_P_H
#define QAtomicComparators_P_H

#include <qabstractfloat_p.h>
#include <qatomiccomparator_p.h>
#include <qprimitives_p.h>

namespace QPatternist {

class StringComparator : public AtomicComparator
{

 public:
   ComparisonResult compare(const Item &op1, const AtomicComparator::Operator op, const Item &op2) const override;

   /**
    * Compares two strings, and returns @c true if they are considered equal as per
    * an ordinary string comparison. In other words, this is an implementation with
    * the Unicode code point collation.
    *
    * @see <a href="http://www.w3.org/TR/xpath-functions/#string-compare">XQuery 1.0 and XPath
    * 2.0 Functions and Operators, 7.3 ValueComparison::Equality and Comparison of Strings</a>
    */
   bool equals(const Item &op1, const Item &op2) const override;
};

class CaseInsensitiveStringComparator : public AtomicComparator
{
 public:
   /**
    * Converts both string values to upper case and afterwards compare them.
    */
   ComparisonResult compare(const Item &op1, const AtomicComparator::Operator op, const Item &op2) const override;

   /**
    * Converts both string values case insensitively.
    */
   bool equals(const Item &op1, const Item &op2) const override;
};


class BinaryDataComparator : public AtomicComparator
{
 public:
   bool equals(const Item &op1, const Item &op2) const override;
};

class BooleanComparator : public AtomicComparator
{
 public:
   ComparisonResult compare(const Item &op1, const AtomicComparator::Operator op, const Item &op2) const override;

   bool equals(const Item &op1, const Item &op2) const override;
};


class AbstractFloatComparator : public AtomicComparator
{
 public:
   ComparisonResult compare(const Item &op1, const AtomicComparator::Operator op, const Item &op2) const override;

   bool equals(const Item &op1, const Item &op2) const override;
};

template<const AtomicComparator::Operator t_op>
class AbstractFloatSortComparator : public AbstractFloatComparator
{
 public:
   ComparisonResult compare(const Item &o1, const AtomicComparator::Operator op, const Item &o2) const override {
      Q_ASSERT_X(t_op == OperatorLessThanNaNLeast || t_op == OperatorLessThanNaNGreatest, Q_FUNC_INFO,
                 "Can only be instantiated with those two.");

      Q_ASSERT(op == t_op);
      (void) op;

      const xsDouble v1 = o1.template as<Numeric>()->toDouble();
      const xsDouble v2 = o2.template as<Numeric>()->toDouble();

      if (qIsNaN(v1) && !qIsNaN(v2)) {
         return t_op == OperatorLessThanNaNLeast ? LessThan : GreaterThan;
      }
      if (!qIsNaN(v1) && qIsNaN(v2)) {
         return t_op == OperatorLessThanNaNLeast ? GreaterThan : LessThan;
      }

      if (Double::isEqual(v1, v2)) {
         return Equal;
      } else if (v1 < v2) {
         return LessThan;
      } else {
         return GreaterThan;
      }
   }

};


class DecimalComparator : public AtomicComparator
{
 public:
   ComparisonResult compare(const Item &op1, const AtomicComparator::Operator op, const Item &op2) const override;

   bool equals(const Item &op1, const Item &op2) const override;
};


class IntegerComparator : public AtomicComparator
{
 public:
   ComparisonResult compare(const Item &op1, const AtomicComparator::Operator op, const Item &op2) const override;

   bool equals(const Item &op1, const Item &op2) const override;
};


class QNameComparator : public AtomicComparator
{
 public:
   bool equals(const Item &op1, const Item &op2) const override;
};

class AbstractDateTimeComparator : public AtomicComparator
{
 public:
   ComparisonResult compare(const Item &op1, const AtomicComparator::Operator op, const Item &op2) const override;
   bool equals(const Item &op1, const Item &op2) const override;
};


class AbstractDurationComparator : public AtomicComparator
{
 public:
   ComparisonResult compare(const Item &op1, const AtomicComparator::Operator op, const Item &op2) const override;
   bool equals(const Item &op1, const Item &op2) const override;

 private:
   static inline QDateTime addDurationToDateTime(const QDateTime &dateTime,
         const AbstractDuration *const duration);
};
}

#endif
