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

#include "qcommonnamespaces_p.h"

#include "qcommonsequencetypes_p.h"
#include "qfunctionfactory_p.h"
#include "qgeneralcomparison_p.h"
#include "qliteral_p.h"
#include "qschemanumeric_p.h"
#include "qvaluecomparison_p.h"
#include "qoptimizerblocks_p.h"

using namespace QPatternist;

ByIDIdentifier::ByIDIdentifier(const Expression::ID id) : m_id(id)
{
}

bool ByIDIdentifier::matches(const Expression::Ptr &expr) const
{
   return expr->is(m_id);
}

ComparisonIdentifier::ComparisonIdentifier(const QVector<Expression::ID> hosts,
      const AtomicComparator::Operator op) : m_hosts(hosts),
   m_op(op)
{
}

bool ComparisonIdentifier::matches(const Expression::Ptr &e) const
{
   const Expression::ID eID = e->id();

   if (eID == Expression::IDGeneralComparison) {
      if (m_hosts.contains(Expression::IDGeneralComparison)) {
         return e->as<GeneralComparison>()->operatorID() == m_op;
      } else {
         return false;
      }
   } else if (eID == Expression::IDValueComparison) {
      if (m_hosts.contains(Expression::IDValueComparison)) {
         return e->as<ValueComparison>()->operatorID() == m_op;
      } else {
         return false;
      }
   } else {
      return false;
   }
}

BySequenceTypeIdentifier::BySequenceTypeIdentifier(const SequenceType::Ptr &seqType) : m_seqType(seqType)
{
   Q_ASSERT(seqType);
}

bool BySequenceTypeIdentifier::matches(const Expression::Ptr &expr) const
{
   const SequenceType::Ptr t(expr->staticType());

   return m_seqType->itemType()->xdtTypeMatches(t->itemType())
          &&
          m_seqType->cardinality().isMatch(t->cardinality());
}

IntegerIdentifier::IntegerIdentifier(const xsInteger num) : m_num(num)
{
}

bool IntegerIdentifier::matches(const Expression::Ptr &expr) const
{
   return expr->is(Expression::IDIntegerValue) &&
          expr->as<Literal>()->item().as<Numeric>()->toInteger() == m_num;
}

BooleanIdentifier::BooleanIdentifier(const bool value) : m_value(value)
{
}

bool BooleanIdentifier::matches(const Expression::Ptr &expr) const
{
   return expr->is(Expression::IDBooleanValue) &&
          expr->evaluateEBV(DynamicContext::Ptr()) == m_value;
}

ByIDCreator::ByIDCreator(const Expression::ID id) : m_id(id)
{
   Q_ASSERT(id != Expression::IDIgnorableExpression);
}

Expression::Ptr ByIDCreator::create(const Expression::List &operands,
                                    const StaticContext::Ptr &context,
                                    const SourceLocationReflection *const r) const
{
   return create(m_id, operands, context, r);
}

Expression::Ptr ByIDCreator::create(const Expression::ID id,
                                    const Expression::List &operands,
                                    const StaticContext::Ptr &context,
                                    const SourceLocationReflection *const r)
{
   Q_ASSERT(context);

   QXmlName::LocalNameCode fnName;

   switch (id) {
      case Expression::IDExistsFN: {
         fnName = StandardLocalNames::exists;
         break;
      }
      case Expression::IDEmptyFN: {
         fnName = StandardLocalNames::empty;
         break;
      }
      default: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "Can not create an expression of requested type, m_id is invalid.");
         return Expression::Ptr();
      }
   }

   /* The reason we don't simply do 'new ExistsFN()' ourselves, is that all FunctionCall
    * instances needs their FunctionSignature in order to function, and the FunctionFactories
    * sets that. */
   const QXmlName qName(StandardNamespaces::fn, fnName);

   const Expression::Ptr result(context->functionSignatures()->createFunctionCall(qName, operands, context, r));
   context->wrapExpressionWith(r, result);
   return result;
}
