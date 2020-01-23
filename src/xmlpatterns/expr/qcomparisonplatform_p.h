/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QComparisonPlatform_P_H
#define QComparisonPlatform_P_H

#include <qatomiccomparators_p.h>
#include <qitem_p.h>
#include <qcommonsequencetypes_p.h>
#include <qdynamiccontext_p.h>
#include <qbuiltintypes_p.h>
#include <qitemtype_p.h>
#include <qpatternistlocale_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
template <typename TSubClass,
          bool issueError,
          AtomicComparator::ComparisonType comparisonType = AtomicComparator::AsValueComparison,
          ReportContext::ErrorCode errorCode = ReportContext::XPTY0004>
class ComparisonPlatform
{
 protected:
   /**
    * Makes ComparisonPlatform use the AtomicComparator @p comparator.
    */
   void prepareComparison(const AtomicComparator::Ptr &comparator);

   /**
    * Default constructor. Does nothing. It is implemented in order make template
    * instantiation easier.
    */
   inline ComparisonPlatform() {
   }

   /**
    * Utility function for fetching the appropriate AtomicComparator
    * for two atomic values of type @p type1 and @p type2, for the operator @p op.
    *
    * This function is used throughout the implementation, ranging from the ValueComparison
    * itself, to for example the aggregate functions.
    *
    * @param context the ordinary ReportContext, used for issuing errors.
    * @param type1 the type of the first operand value in a comparison for which the
    * returned AtomicComparator is intended for
    * @param type2 the type of the second operand value in a comparison for which the
    * returned AtomicComparator is intended for. Whether @p type1 and @p type2 corresponds
    * to what is the first second operand type does not have significance, the order
    * can be arbitrary
    */
   AtomicComparator::Ptr
   fetchComparator(const ItemType::Ptr &type1,
                   const ItemType::Ptr &type2,
                   const ReportContext::Ptr &context) const;

   /**
    * @short Compares @p i1 and @p i2 with operator @p op, using comparator @p
    * comp. All input arguments must be valid, and not @c null.
    *
    * This is a fast, raw function which has the requirement that the
    * caller knows what to compare and with what.
    */
   bool compare(const Item &i1,
                const Item &i2,
                const AtomicComparator::Ptr &comp,
                const AtomicComparator::Operator op) const;

   /**
    * @short Compares @p it1 against @p it2, using comparator() and operatorID().
    *
    * If the comparator wasn't looked up at compile time, it will be
    * attempted before comparing. If this fails, errors are reported via
    * @p context.
    */
   bool
   flexibleCompare(const Item &it1,
                   const Item &it2,
                   const DynamicContext::Ptr &context) const;

   /**
    * @short like flexibleCompare(), but returns the result
    * as an AtomicComparator::Operator instead of @c bool.
    *
    * This is useful when it is significant how a less than comparison
    * fails; whether the two values are equal or greater than.
    */
   AtomicComparator::ComparisonResult
   detailedFlexibleCompare(const Item &it1,
                           const Item &it2,
                           const DynamicContext::Ptr &context) const;

   /**
    * @returns the AtomicComparator that has been allocated at compile time,
    * with prepareComparison(). If no AtomicComparator has been allocated
    * for some reason, this function returns @c null.
    */
   inline const AtomicComparator::Ptr &comparator() const {
      return m_comparator;
   }

   /**
    * Calling this function makes ComparisonPlatform use a comparator that
    * compares strings case insensitively.
    *
    * @see ValueComparison::isCaseInsensitiveCompare()
    */
   inline void useCaseInsensitiveComparator() {
      m_comparator = AtomicComparator::Ptr(new CaseInsensitiveStringComparator());
   }

 private:
   /**
    * @returns the operator that is used.
    */
   inline AtomicComparator::Operator operatorID() const {
      Q_ASSERT(static_cast<const TSubClass *>(this)->operatorID());
      return static_cast<const TSubClass *>(this)->operatorID();
   }

   /**
    * The comparator that is used for comparing atomic values. The AtomicComparator
    * that is used, depends on the static type of the operands. m_comparator can be
    * @c null if it wasn't possible to determine what comparator to use at compile time.
    */
   AtomicComparator::Ptr m_comparator;
};

#include "qcomparisonplatform.cpp"

}

QT_END_NAMESPACE

#endif
