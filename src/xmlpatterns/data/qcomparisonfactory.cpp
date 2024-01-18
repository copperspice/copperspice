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

#include "qatomiccomparators_p.h"
#include "qatomicstring_p.h"
#include "qcomparisonplatform_p.h"
#include "qvaluefactory_p.h"

#include "qcomparisonfactory_p.h"

using namespace QPatternist;

/**
 * @short Helper class for ComparisonFactory::fromLexical() which exposes
 * CastingPlatform appropriately.
 *
 * @relates ComparisonFactory
 */
class PerformComparison : public ComparisonPlatform<PerformComparison, true>
   , public SourceLocationReflection
{
 public:
   PerformComparison(const SourceLocationReflection *const sourceLocationReflection,
                     const AtomicComparator::Operator op) : m_sourceReflection(sourceLocationReflection)
      , m_operator(op) {
      Q_ASSERT(m_sourceReflection);
   }

   bool operator()(const AtomicValue::Ptr &operand1,
                   const AtomicValue::Ptr &operand2,
                   const SchemaType::Ptr &type,
                   const ReportContext::Ptr &context) {
      const ItemType::Ptr asItemType((AtomicType::Ptr(type)));

      /* One area where the Query Transform world differs from the Schema
       * world is that @c xs:duration is not considedered comparable, because
       * it's according to Schema is partially comparable. This means
       * ComparisonPlatform::fetchComparator() flags it as impossible, and
       * hence we need to override that.
       *
       * SchemaType::wxsTypeMatches() will return true for sub-types of @c
       * xs:duration as well, but that's ok since AbstractDurationComparator
       * works for them too. */
      if (BuiltinTypes::xsDuration->wxsTypeMatches(type)) {
         prepareComparison(AtomicComparator::Ptr(new AbstractDurationComparator()));
      } else if (BuiltinTypes::xsGYear->wxsTypeMatches(type) ||
                 BuiltinTypes::xsGYearMonth->wxsTypeMatches(type) ||
                 BuiltinTypes::xsGMonth->wxsTypeMatches(type) ||
                 BuiltinTypes::xsGMonthDay->wxsTypeMatches(type) ||
                 BuiltinTypes::xsGDay->wxsTypeMatches(type)) {
         prepareComparison(AtomicComparator::Ptr(new AbstractDateTimeComparator()));
      } else {
         prepareComparison(fetchComparator(asItemType, asItemType, context));
      }

      return flexibleCompare(operand1, operand2, context);
   }

   const SourceLocationReflection *actualReflection() const override {
      return m_sourceReflection;
   }

   AtomicComparator::Operator operatorID() const {
      return m_operator;
   }

 private:
   const SourceLocationReflection *const m_sourceReflection;
   const AtomicComparator::Operator      m_operator;
};

bool ComparisonFactory::compare(const AtomicValue::Ptr &operand1,
                                const AtomicComparator::Operator op,
                                const AtomicValue::Ptr &operand2,
                                const SchemaType::Ptr &type,
                                const ReportContext::Ptr &context,
                                const SourceLocationReflection *const sourceLocationReflection)
{
   Q_ASSERT(operand1);
   Q_ASSERT(operand2);
   Q_ASSERT(context);
   Q_ASSERT(sourceLocationReflection);
   Q_ASSERT(type);
   Q_ASSERT_X(type->category() == SchemaType::SimpleTypeAtomic, Q_FUNC_INFO,
              "We can only compare atomic values.");

   return PerformComparison(sourceLocationReflection, op)(operand1, operand2, type, context);
}

bool ComparisonFactory::constructAndCompare(const DerivedString<TypeString>::Ptr &operand1,
      const AtomicComparator::Operator op,
      const DerivedString<TypeString>::Ptr &operand2,
      const SchemaType::Ptr &type,
      const ReportContext::Ptr &context,
      const SourceLocationReflection *const sourceLocationReflection)
{
   Q_ASSERT(operand1);
   Q_ASSERT(operand2);
   Q_ASSERT(context);
   Q_ASSERT(sourceLocationReflection);
   Q_ASSERT(type);
   Q_ASSERT_X(type->category() == SchemaType::SimpleTypeAtomic, Q_FUNC_INFO,
              "We can only compare atomic values.");

   const AtomicValue::Ptr value1 = ValueFactory::fromLexical(operand1->stringValue(), type, context,
                                   sourceLocationReflection);
   const AtomicValue::Ptr value2 = ValueFactory::fromLexical(operand2->stringValue(), type, context,
                                   sourceLocationReflection);

   return compare(value1, op, value2, type, context, sourceLocationReflection);
}
