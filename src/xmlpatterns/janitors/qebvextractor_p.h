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

#ifndef QEBVExtractor_P_H
#define QEBVExtractor_P_H

#include <qsinglecontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class EBVExtractor : public SingleContainer
{
 public:
   EBVExtractor(const Expression::Ptr &operand);

   bool evaluateEBV(const DynamicContext::Ptr &context) const override;
   SequenceType::List expectedOperandTypes() const override;
   const SourceLocationReflection *actualReflection() const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   SequenceType::Ptr staticType() const override;

   template<typename TSubClass, typename ThisType>
   static Expression::Ptr typeCheck(const StaticContext::Ptr &context, 
                  const SequenceType::Ptr &reqType, ThisType *const caller) {

      if (*CommonSequenceTypes::EBV->itemType() == *reqType->itemType()) {
         return caller->operands().first()->typeCheck(context, reqType);
      } else {
         return caller->TSubClass::typeCheck(context, reqType);
      }
   }

};
}

QT_END_NAMESPACE

#endif
