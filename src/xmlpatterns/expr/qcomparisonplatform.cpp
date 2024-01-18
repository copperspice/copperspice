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

template <typename TSubClass, bool issueError,
          AtomicComparator::ComparisonType comparisonType, ReportContext::ErrorCode errorCode>
bool ComparisonPlatform<TSubClass, issueError, comparisonType, errorCode>::
flexibleCompare(const Item &it1,
                const Item &it2,
                const DynamicContext::Ptr &context) const
{
   if (m_comparator)
      /* The comparator was located at compile time. */
   {
      return compare(it1, it2, m_comparator, operatorID());
   } else {
      const AtomicComparator::Ptr cp(fetchComparator(it1.type(),
                                     it2.type(),
                                     context));

      return cp ? compare(it1, it2, cp, operatorID()) : false;
   }
}

template <typename TSubClass, bool issueError,
          AtomicComparator::ComparisonType comparisonType, ReportContext::ErrorCode errorCode>
AtomicComparator::ComparisonResult
ComparisonPlatform<TSubClass, issueError, comparisonType, errorCode>::
detailedFlexibleCompare(const Item &it1,
                        const Item &it2,
                        const DynamicContext::Ptr &context) const
{
   AtomicComparator::Ptr comp;

   if (m_comparator) {
      comp = m_comparator;
   } else {
      comp = fetchComparator(it1.type(),
                             it2.type(),
                             context);
   }

   Q_ASSERT_X(operatorID() == AtomicComparator::OperatorLessThanNaNLeast ||
              operatorID() == AtomicComparator::OperatorLessThanNaNGreatest,
              Q_FUNC_INFO, "Only OperatorLessThan is currently supported for this function.");
   return comp->compare(it1, operatorID(), it2);
}

template <typename TSubClass, bool issueError,
          AtomicComparator::ComparisonType comparisonType, ReportContext::ErrorCode errorCode>
bool ComparisonPlatform<TSubClass, issueError, comparisonType, errorCode>::
compare(const Item &oand1,
        const Item &oand2,
        const AtomicComparator::Ptr &comp,
        const AtomicComparator::Operator op) const
{
   Q_ASSERT(oand1);
   Q_ASSERT(oand2);
   Q_ASSERT(comp);

   switch (op) {
      case AtomicComparator::OperatorEqual:
         return comp->equals(oand1, oand2);
      case AtomicComparator::OperatorNotEqual:
         return !comp->equals(oand1, oand2);

      case AtomicComparator::OperatorLessThanNaNLeast:
      case AtomicComparator::OperatorLessThanNaNGreatest:
      case AtomicComparator::OperatorLessThan:
         return comp->compare(oand1, op, oand2) == AtomicComparator::LessThan;

      case AtomicComparator::OperatorGreaterThan:
         return comp->compare(oand1, op, oand2) == AtomicComparator::GreaterThan;

      case AtomicComparator::OperatorLessOrEqual: {
         const AtomicComparator::ComparisonResult ret = comp->compare(oand1, op, oand2);
         return ret == AtomicComparator::LessThan || ret == AtomicComparator::Equal;
      }

      case (AtomicComparator::OperatorGreaterOrEqual): {
         const AtomicComparator::ComparisonResult ret = comp->compare(oand1, op, oand2);
         return ret == AtomicComparator::GreaterThan || ret == AtomicComparator::Equal;
      }
   }

   /* GCC unbarfer, this line should never be reached. */
   Q_ASSERT(false);
   return false;
}

template <typename TSubClass, bool issueError,
          AtomicComparator::ComparisonType comparisonType, ReportContext::ErrorCode errorCode>
AtomicComparator::Ptr ComparisonPlatform<TSubClass, issueError, comparisonType, errorCode>::
fetchComparator(const ItemType::Ptr &t1,
                const ItemType::Ptr &t2,
                const ReportContext::Ptr &context) const
{
   Q_ASSERT(t1);
   Q_ASSERT(t2);

   if (*BuiltinTypes::xsAnyAtomicType == *t1    ||
         *BuiltinTypes::xsAnyAtomicType == *t2    ||
         *BuiltinTypes::item == *t1               ||
         *BuiltinTypes::item == *t2               ||
         *BuiltinTypes::numeric == *t1            ||
         *BuiltinTypes::numeric == *t2            ||
         *CommonSequenceTypes::Empty == *t1       ||
         *CommonSequenceTypes::Empty == *t2) {
      /* The static type of(at least) one of the operands could not
       * be narrowed further, so we do the operator
       * lookup at runtime.
       */
      return AtomicComparator::Ptr();
   }

   const AtomicComparatorLocator::Ptr locator
   (static_cast<const AtomicType *>(t1.data())->comparatorLocator());

   if (!locator) {
      if (issueError) {
         context->error(QtXmlPatterns::tr("No comparisons can be done involving the type %1.")
                        .formatArg(formatType(context->namePool(), t1)), errorCode, static_cast<const TSubClass *>(this)->actualReflection());
      }
      return AtomicComparator::Ptr();
   }

   const AtomicComparator::Ptr comp(static_cast<const AtomicType *>(t2.data())->accept(locator, operatorID(),
                                    static_cast<const TSubClass *>(this)->actualReflection()));

   if (comp) {
      return comp;
   } else if (issueError) {
      context->error(QtXmlPatterns::tr("Operator %1 is not available between atomic values of type %2 and %3.")
                     .formatArgs(formatKeyword(AtomicComparator::displayName(operatorID(), comparisonType)),
                     formatType(context->namePool(), t1), formatType(context->namePool(), t2)),
                     errorCode, static_cast<const TSubClass *>(this)->actualReflection());
   }

   return AtomicComparator::Ptr();
}

template <typename TSubClass, bool issueError,
          AtomicComparator::ComparisonType comparisonType, ReportContext::ErrorCode errorCode>
void ComparisonPlatform<TSubClass, issueError, comparisonType, errorCode>::
prepareComparison(const AtomicComparator::Ptr &c)
{
   m_comparator = c;
}

