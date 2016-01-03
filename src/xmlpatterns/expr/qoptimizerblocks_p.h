/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QOptimizerBlocks_P_H
#define QOptimizerBlocks_P_H

#include "qatomiccomparator_p.h"
#include "qexpression_p.h"
#include "qoptimizerframework_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
class ByIDIdentifier : public ExpressionIdentifier
{
 public:
   ByIDIdentifier(const Expression::ID id);
   virtual bool matches(const Expression::Ptr &expr) const;
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
   virtual bool matches(const Expression::Ptr &expr) const;

 private:
   const SequenceType::Ptr m_seqType;
};

class ComparisonIdentifier : public ExpressionIdentifier
{
 public:

   ComparisonIdentifier(const QVector<Expression::ID> comparatorHosts,
                        const AtomicComparator::Operator op);

   virtual bool matches(const Expression::Ptr &expr) const;

 private:
   const QVector<Expression::ID> m_hosts;
   const AtomicComparator::Operator m_op;
};


class IntegerIdentifier : public ExpressionIdentifier
{
 public:
   IntegerIdentifier(const xsInteger num);
   virtual bool matches(const Expression::Ptr &expr) const;

 private:
   const xsInteger m_num;
};

class BooleanIdentifier : public ExpressionIdentifier
{
 public:
   BooleanIdentifier(const bool value);
   virtual bool matches(const Expression::Ptr &expr) const;

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
   virtual Expression::Ptr create(const Expression::List &operands,
                                  const StaticContext::Ptr &context,
                                  const SourceLocationReflection *const r) const;

   /**
    * Creates an expression by id @p id with the arguments @p operands.
    */
   static Expression::Ptr create(const Expression::ID id,
                                 const Expression::List &operands,
                                 const StaticContext::Ptr &context,
                                 const SourceLocationReflection *const r);

 private:
   const Expression::ID m_id;
};
}

QT_END_NAMESPACE

#endif
