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

#ifndef QCastableAs_P_H
#define QCastableAs_P_H

#include <qsinglecontainer_p.h>
#include <qcastingplatform_p.h>

namespace QPatternist {

class CastableAs : public SingleContainer, public CastingPlatform<CastableAs, false>
{
 public:
   CastableAs(const Expression::Ptr &operand, const SequenceType::Ptr &targetType);

   bool evaluateEBV(const DynamicContext::Ptr &) const override;

   /**
    * Overridden to const fold to @c true when the target type
    * is a type which casting to always succeeds. This is
    * the type identical to the target type, <tt>xs:string</tt>,
    * and <tt>xs:untypedAtomic</tt>.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   ItemType::Ptr targetType() const {
      return m_targetType->itemType();
   }

 private:
   const SequenceType::Ptr m_targetType;
};

}

#endif
