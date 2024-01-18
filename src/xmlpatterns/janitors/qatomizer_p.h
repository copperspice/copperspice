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

#ifndef QAtomizer_P_H
#define QAtomizer_P_H

#include <qitem_p.h>
#include <qsinglecontainer_p.h>

namespace QPatternist {

class Atomizer : public SingleContainer
{
 public:
   Atomizer(const Expression::Ptr &operand);

   Item evaluateSingleton(const DynamicContext::Ptr &) const override;
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;

   SequenceType::Ptr staticType() const override;

   SequenceType::List expectedOperandTypes() const override;
   const SourceLocationReflection *actualReflection() const override;

   /**
    * Makes an early compression, by returning the result of
    * the type checked operand, if the operand has the static type
    * xs:anyAtomicType(no atomization needed).
    */
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   inline Item::Iterator::Ptr mapToSequence(const Item &item, const DynamicContext::Ptr &context) const;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

 private:
   typedef QExplicitlySharedDataPointer<const Atomizer> ConstPtr;
};

}

#endif
