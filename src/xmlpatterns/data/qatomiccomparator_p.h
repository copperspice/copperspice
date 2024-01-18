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

#ifndef QAtomicComparator_P_H
#define QAtomicComparator_P_H

#include <qflags.h>
#include <qitem_p.h>
#include <qstringfwd.h>

#include <qatomictypedispatch_p.h>

namespace QPatternist {
class AtomicComparator : public AtomicTypeVisitorResult
{
 public:
   AtomicComparator();
   virtual ~AtomicComparator();

   typedef QExplicitlySharedDataPointer<AtomicComparator> Ptr;

   enum Operator {
      /**
       * Operator <tt>eq</tt> and <tt>=</tt>.
       */
      OperatorEqual           = 1,

      /**
       * Operator <tt>ne</tt> and <tt>!=</tt>.
       */
      OperatorNotEqual        = 1 << 1,

      /**
       * Operator <tt>gt</tt> and <tt>\></tt>.
       */
      OperatorGreaterThan     = 1 << 2,

      /**
       * Operator <tt>lt</tt> and <tt>\<</tt>.
       */
      OperatorLessThan        = 1 << 3,

      /**
       * One of the operators we use for sorting. The only difference from
       * OperatorLessThan is that it sees NaN as ordered and smaller than
       * other numbers.
       */
      OperatorLessThanNaNLeast    = 1 << 4,

      /**
       * One of the operators we use for sorting. The only difference from
       * OperatorLessThanLeast is that it sees NaN as ordered and larger than
       * other numbers.
       */
      OperatorLessThanNaNGreatest    = 1 << 5,

      /**
       * Operator <tt>ge</tt> and <tt>\>=</tt>.
       */
      OperatorGreaterOrEqual  = OperatorEqual | OperatorGreaterThan,

      /**
       * Operator <tt>le</tt> and <tt>\<=</tt>.
       */
      OperatorLessOrEqual     = OperatorEqual | OperatorLessThan
   };

   typedef QFlags<Operator> Operators;

   /**
    * Signifies the result of a value comparison. This is used for value comparisons,
    * and in the future likely also for sorting.
    *
    * @see <a href="http://www.w3.org/TR/xpath20/#id-value-comparisons">W3C XML Path
    * Language (XPath) 2.0, 3.5.1 Value Comparisons</a>
    */
   enum ComparisonResult {
      LessThan     = 1,
      Equal        = 2,
      GreaterThan  = 4,
      Incomparable = 8
   };

   /**
    * Compares @p op1 and @p op2 and determines the relationship between the two. This
    * is used for sorting and comparisons. The implementation performs an assert crash,
    * and must therefore be re-implemented if comparing the relevant values should be
    * possible.
    *
    * @param op1 the first operand
    * @param op the operator. How a comparison is carried out shouldn't depend on what the
    * operator is, but in some cases it is of interest.
    * @param op2 the second operand
    */
   virtual ComparisonResult compare(const Item &op1,
                                    const AtomicComparator::Operator op,
                                    const Item &op2) const;

   /**
    * Determines whether @p op1 and @p op2 are equal. It is the same as calling compare()
    * and checking whether the return value is Equal, but since comparison testing is such
    * a common operation, this specialized function exists.
    *
    * @returns true if @p op1 and @p op2 are equal.
    *
    * @param op1 the first operand
    * @param op2 the second operand
    */
   virtual bool equals(const Item &op1,
                       const Item &op2) const = 0;

   /**
    * Identifies the kind of comparison.
    */
   enum ComparisonType {
      /**
       * Identifies a general comparison; operator @c =, @c >, @c <=, and so on.
       */
      AsGeneralComparison = 1,

      /**
       * Identifies a value comparison; operator @c eq, @c lt, @c le, and so on.
       */
      AsValueComparison
   };

   /**
    * Utility function for getting the lexical representation for
    * the comparison operator @p op. Depending on the @p type argument,
    * the string returned is either a general comparison or a value comparison
    * operator.
    *
    * @param op the operator which the display name should be determined for.
    * @param type signifies whether the returned display name should be for
    * a value comparison or a general comparison. For example, if @p op is
    * OperatorEqual and @p type is AsValueComparision, "eq" is returned.
    */
   static QString displayName(const AtomicComparator::Operator op,
                              const ComparisonType type);

};

Q_DECLARE_OPERATORS_FOR_FLAGS(AtomicComparator::Operators)
}



#endif
