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

#ifndef QElementConstructor_P_H
#define QElementConstructor_P_H

#include <QUrl>
#include <qpaircontainer_p.h>

namespace QPatternist {

class ElementConstructor : public PairContainer
{
 public:
   ElementConstructor(const Expression::Ptr &operand1, const Expression::Ptr &operand2, const bool isXSLT);

   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const override;

   SequenceType::Ptr staticType() const override;

   /**
    * The first operand must be exactly one @c xs:QName, and the second
    * argument can be zero or more items.
    */
   SequenceType::List expectedOperandTypes() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   Properties properties() const override;

 private:
   QUrl m_staticBaseURI;
   const bool m_isXSLT;
};

}

#endif
