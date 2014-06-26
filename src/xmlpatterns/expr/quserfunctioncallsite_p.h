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

#ifndef QUserFunctionCallsite_P_H
#define QUserFunctionCallsite_P_H

#include <qcallsite_p.h>
#include <qfunctionsignature_p.h>
#include <qunlimitedcontainer_p.h>
#include <quserfunction_p.h>
#include <qvariabledeclaration_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class UserFunctionCallsite : public CallSite
{
 public:
   typedef QExplicitlySharedDataPointer<UserFunctionCallsite> Ptr;
   typedef QList<UserFunctionCallsite::Ptr> List;

   UserFunctionCallsite(const QXmlName name,
                        const FunctionSignature::Arity arity);

   virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
   virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   /**
    * We call compress on our body.
    */
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);

   virtual Expression::Properties properties() const;

   /**
    * @short Returns the types declared in the function declaration.
    *
    * @see CallTemplate::expectedOperandTypes()
    */
   virtual SequenceType::List expectedOperandTypes() const;

   virtual SequenceType::Ptr staticType() const;
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

   /**
    * @returns always IDUserFunctionCallsite.
    */
   virtual ID id() const;

   /**
    * If @p slotOffset is -1, it means this function has no arguments.
    */
   void setSource(const UserFunction::Ptr &userFunction,
                  const VariableSlotID cacheSlotOffset);

   /**
    * @returns @c true, if a function definition with signature @p sign
    * would be valid to call from this callsite, otherwise @c false.
    */
   bool isSignatureValid(const FunctionSignature::Ptr &sign) const;

   FunctionSignature::Arity arity() const;

   inline Expression::Ptr body() const {
      return m_body;
   }

   virtual bool configureRecursion(const CallTargetDescription::Ptr &sign);
   virtual CallTargetDescription::Ptr callTargetDescription() const;

 private:
   /**
    * Creates a new context sets the arguments, and returns it.
    */
   DynamicContext::Ptr bindVariables(const DynamicContext::Ptr &context) const;

   const FunctionSignature::Arity  m_arity;
   /**
    * The reason this variable, as well as others, aren't const, is that
    * the binding to the actual function, is resolved after this
    * UserFunctionCallsite has been created.
    */
   VariableSlotID                  m_expressionSlotOffset;

   /**
    * @note This may be different from m_functionDeclaration->body(). It
    * may differ on a per-callsite basis.
    */
   Expression::Ptr                 m_body;
   UserFunction::Ptr               m_functionDeclaration;
};

/**
 * @short Formats UserFunctionCallsite.
 *
 * @relates UserFunctionCallsite
 */
static inline QString formatFunction(const UserFunctionCallsite::Ptr &func)
{
   Q_UNUSED(func);
   // TODO TODO TODO
   // TODO Make UserFunctionCallsite always use a FunctionSignature
   return QLatin1String("<span class='XQuery-function'>")  +
          QString() +
          //escape(func->name()->toString())                 +
          QLatin1String("</span>");
}
}

QT_END_NAMESPACE

#endif
