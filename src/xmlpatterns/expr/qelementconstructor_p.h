/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QElementConstructor_P_H
#define QElementConstructor_P_H

#include <QUrl>
#include <qpaircontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class ElementConstructor : public PairContainer
{
 public:
   ElementConstructor(const Expression::Ptr &operand1,
                      const Expression::Ptr &operand2,
                      const bool isXSLT);

   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
   virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

   virtual SequenceType::Ptr staticType() const;

   /**
    * The first operand must be exactly one @c xs:QName, and the second
    * argument can be zero or more items.
    */
   virtual SequenceType::List expectedOperandTypes() const;

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);
   virtual Properties properties() const;

 private:
   QUrl m_staticBaseURI;
   const bool m_isXSLT;
};
}

QT_END_NAMESPACE

#endif
