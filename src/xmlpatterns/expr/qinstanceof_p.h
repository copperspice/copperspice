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

#ifndef QInstanceOf_P_H
#define QInstanceOf_P_H

#include <qsinglecontainer_p.h>

namespace QPatternist {

class InstanceOf : public SingleContainer
{
 public:
   InstanceOf(const Expression::Ptr &operand, const SequenceType::Ptr &targetType);

   bool evaluateEBV(const DynamicContext::Ptr &) const override;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;
   Expression::Ptr compress(const StaticContext::Ptr &context) override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   /**
    * @returns the SequenceType that this <tt>instance of</tt> Expression
    * is testing its operand against.
    */
   SequenceType::Ptr targetType() const;

 private:
   const SequenceType::Ptr m_targetType;
};

}

#endif
