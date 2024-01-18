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

#ifndef QOptimizerBlocks_P_H
#define QOptimizerBlocks_P_H

#include "qatomiccomparator_p.h"
#include "qexpression_p.h"
#include "qoptimizerframework_p.h"

namespace QPatternist {

class ByIDIdentifier : public ExpressionIdentifier
{
 public:
   ByIDIdentifier(const Expression::ID id);
   bool matches(const Expression::Ptr &expr) const override;

 private:
   const Expression::ID m_id;
};

class BySequenceTypeIdentifier : public ExpressionIdentifier
{
 public:
   BySequenceTypeIdentifier(const SequenceType::Ptr &seqType);

   /**
    * @returns @c true, if the static type of @p expr is matches
    * the SequenceType passed in the BySequenceTypeIdentifier()
    * constructor, otherwise @c false.
    */
   bool matches(const Expression::Ptr &expr) const override;

 private:
   const SequenceType::Ptr m_seqType;
};

class ComparisonIdentifier : public ExpressionIdentifier
{
 public:

   ComparisonIdentifier(const QVector<Expression::ID> comparatorHosts, const AtomicComparator::Operator op);

   bool matches(const Expression::Ptr &expr) const override;

 private:
   const QVector<Expression::ID> m_hosts;
   const AtomicComparator::Operator m_op;
};


class IntegerIdentifier : public ExpressionIdentifier
{
 public:
   IntegerIdentifier(const xsInteger num);
   bool matches(const Expression::Ptr &expr) const override;

 private:
   const xsInteger m_num;
};

class BooleanIdentifier : public ExpressionIdentifier
{
 public:
   BooleanIdentifier(const bool value);
   bool matches(const Expression::Ptr &expr) const override;

 private:
   const bool m_value;
};

class ByIDCreator : public ExpressionCreator
{
 public:
   /**
    * Creates a ByIDCreator that creates expressions
    * of the type that @p id identifies.
    */
   ByIDCreator(const Expression::ID id);
   Expression::Ptr create(const Expression::List &operands, const StaticContext::Ptr &context,
                  const SourceLocationReflection *const r) const override;

   /**
    * Creates an expression by id @p id with the arguments @p operands.
    */
   static Expression::Ptr create(const Expression::ID id, const Expression::List &operands,
                  const StaticContext::Ptr &context, const SourceLocationReflection *const r);

 private:
   const Expression::ID m_id;
};

}

#endif
