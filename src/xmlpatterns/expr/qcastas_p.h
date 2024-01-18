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

#ifndef QCastAs_P_H
#define QCastAs_P_H

#include <qsinglecontainer_p.h>
#include <qcastingplatform_p.h>

namespace QPatternist {

class CastAs : public SingleContainer, public CastingPlatform<CastAs, true /* issueError */>
{
 public:

   /**
    * @todo Wrong/old documentation
    *
    * Creates a cast expression for the type @p name via the schema type
    * factory @p factory. This function is used by parser when creating
    * 'cast to' expressions, and the ConstructorFunctionsFactory, when creating
    * constructor functions.
    *
    * @param targetType the type which the CastAs should cast to
    * @param source the operand to evaluate and then cast from
    */
   CastAs(const Expression::Ptr &source, const SequenceType::Ptr &targetType);

   Item evaluateSingleton(const DynamicContext::Ptr &) const override;

   SequenceType::List expectedOperandTypes() const override;

   /**
    * @returns a SequenceType where the ItemType is this CastAs's
    * target type, as per targetType(), and the Cardinality is inferred from the
    * source operand to reflect whether this CastAs always will evaluate to
    * exactly-one or zero-or-one values.
    */
   SequenceType::Ptr staticType() const override;

   /**
    * Overridden in order to check that casting to an abstract type
    * is not attempted.
    */
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   /**
    * If the target type is the same as the source type, it is rewritten
    * to the operand.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   ItemType::Ptr targetType() const {
      return m_targetType->itemType();
   }

   SequenceType::Ptr targetSequenceType() const {
      return m_targetType;
   }

 private:
   /**
    * Performs casting to @c xs:QName. This case is special, and is always done at compile time.
    */
   Expression::Ptr castToQName(const StaticContext::Ptr &context) const;

   const SequenceType::Ptr m_targetType;
};

}

#endif
