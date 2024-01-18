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

#ifndef QItemVerifier_P_H
#define QItemVerifier_P_H

#include <qsinglecontainer_p.h>

namespace QPatternist {

class ItemVerifier : public SingleContainer
{
 public:

   ItemVerifier(const Expression::Ptr &operand,
                const ItemType::Ptr &reqType,
                const ReportContext::ErrorCode errorCode);

   Item evaluateSingleton(const DynamicContext::Ptr &) const override;
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;

   inline Item mapToItem(const Item &, const DynamicContext::Ptr &) const;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   const SourceLocationReflection *actualReflection() const override;

 private:
   typedef QExplicitlySharedDataPointer<const ItemVerifier> ConstPtr;
   inline void verifyItem(const Item &item, const DynamicContext::Ptr &context) const;

   const ItemType::Ptr             m_reqType;
   const ReportContext::ErrorCode  m_errorCode;
};

}

#endif
