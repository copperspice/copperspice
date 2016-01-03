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

#ifndef QItemVerifier_P_H
#define QItemVerifier_P_H

#include <qsinglecontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class ItemVerifier : public SingleContainer
{
 public:

   ItemVerifier(const Expression::Ptr &operand,
                const ItemType::Ptr &reqType,
                const ReportContext::ErrorCode errorCode);

   virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;

   virtual SequenceType::List expectedOperandTypes() const;
   virtual SequenceType::Ptr staticType() const;

   inline Item mapToItem(const Item &, const DynamicContext::Ptr &) const;
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual const SourceLocationReflection *actualReflection() const;

 private:
   typedef QExplicitlySharedDataPointer<const ItemVerifier> ConstPtr;
   inline void verifyItem(const Item &item, const DynamicContext::Ptr &context) const;

   const ItemType::Ptr             m_reqType;
   const ReportContext::ErrorCode  m_errorCode;
};
}

QT_END_NAMESPACE

#endif
