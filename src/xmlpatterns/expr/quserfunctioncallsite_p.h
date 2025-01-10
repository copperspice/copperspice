/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QUserFunctionCallsite_P_H
#define QUserFunctionCallsite_P_H

#include <qcallsite_p.h>
#include <qfunctionsignature_p.h>
#include <qunlimitedcontainer_p.h>
#include <quserfunction_p.h>
#include <qvariabledeclaration_p.h>

namespace QPatternist {
class UserFunctionCallsite : public CallSite
{
 public:
   typedef QExplicitlySharedDataPointer<UserFunctionCallsite> Ptr;
   typedef QList<UserFunctionCallsite::Ptr> List;

   UserFunctionCallsite(const QXmlName name,
                        const FunctionSignature::Arity arity);

   bool evaluateEBV(const DynamicContext::Ptr &context) const override;
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const override;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   Expression::Properties properties() const override;

   SequenceType::List expectedOperandTypes() const override;

   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   ID id() const override;

   void setSource(const UserFunction::Ptr &userFunction, const VariableSlotID cacheSlotOffset);

   bool isSignatureValid(const FunctionSignature::Ptr &sign) const;

   FunctionSignature::Arity arity() const;

   Expression::Ptr body() const override {
      return m_body;
   }

   bool configureRecursion(const CallTargetDescription::Ptr &sign) override;
   CallTargetDescription::Ptr callTargetDescription() const override;

 private:
   DynamicContext::Ptr bindVariables(const DynamicContext::Ptr &context) const;

   const FunctionSignature::Arity  m_arity;
   VariableSlotID m_expressionSlotOffset;

   Expression::Ptr m_body;
   UserFunction::Ptr m_functionDeclaration;
};

static inline QString formatFunction(const UserFunctionCallsite::Ptr &func)
{
   (void) func;
   return QLatin1String("<span class='XQuery-function'></span>");
}

}

#endif
