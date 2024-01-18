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

#ifndef QExpressionVariableReference_P_H
#define QExpressionVariableReference_P_H

#include <qvariabledeclaration_p.h>
#include <qvariablereference_p.h>

namespace QPatternist {

class ExpressionVariableReference : public VariableReference
{
 public:
   ExpressionVariableReference(const VariableSlotID slot, const VariableDeclaration *varDecl);

   bool evaluateEBV(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   SequenceType::Ptr staticType() const override;
   ID id() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   inline const Expression::Ptr &sourceExpression() const;
   inline const VariableDeclaration *variableDeclaration() const;

 private:
   const VariableDeclaration *m_varDecl;
};

inline const Expression::Ptr &ExpressionVariableReference::sourceExpression() const
{
   return m_varDecl->expression();
}

inline const VariableDeclaration *ExpressionVariableReference::variableDeclaration() const
{
   return m_varDecl;
}

}

#endif
