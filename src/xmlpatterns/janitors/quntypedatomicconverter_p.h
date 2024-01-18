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

#ifndef QUntypedAtomicConverter_P_H
#define QUntypedAtomicConverter_P_H

#include <qcastingplatform_p.h>
#include <qitem_p.h>
#include <qsinglecontainer_p.h>

namespace QPatternist {

class UntypedAtomicConverter : public SingleContainer, public CastingPlatform<UntypedAtomicConverter, true>
{
 public:
   UntypedAtomicConverter(const Expression::Ptr &operand, const ItemType::Ptr &reqType,
                  const ReportContext::ErrorCode code = ReportContext::FORG0001);

   Item evaluateSingleton(const DynamicContext::Ptr &) const override;
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;

   SequenceType::Ptr staticType() const override;
   SequenceType::List expectedOperandTypes() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   /**
    * Overridden to call CastingPlatform::typeCheck()
    */
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   inline Item mapToItem(const Item &item, const DynamicContext::Ptr &context) const;

   ItemType::Ptr targetType() const {
      return m_reqType;
   }

   const SourceLocationReflection *actualReflection() const override;

 private:
   typedef QExplicitlySharedDataPointer<const UntypedAtomicConverter> ConstPtr;
   const ItemType::Ptr m_reqType;
};

Item UntypedAtomicConverter::mapToItem(const Item &item, const DynamicContext::Ptr &context) const
{
   return cast(item, context);
}
}

#endif
