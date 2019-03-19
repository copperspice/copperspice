/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qcommonsequencetypes_p.h"
#include "qdynamiccontextstore_p.h"
#include "qevaluationcache_p.h"

#include "quserfunctioncallsite_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

UserFunctionCallsite::UserFunctionCallsite(const QXmlName nameP,
      const FunctionSignature::Arity ar) : CallSite(nameP)
   , m_arity(ar)
   , m_expressionSlotOffset(-2)

{
}

Item::Iterator::Ptr UserFunctionCallsite::evaluateSequence(const DynamicContext::Ptr &context) const
{
   return m_body->evaluateSequence(bindVariables(context));
}

Item UserFunctionCallsite::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return m_body->evaluateSingleton(bindVariables(context));
}

bool UserFunctionCallsite::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return m_body->evaluateEBV(bindVariables(context));
}

void UserFunctionCallsite::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   m_body->evaluateToSequenceReceiver(bindVariables(context));
}

DynamicContext::Ptr UserFunctionCallsite::bindVariables(const DynamicContext::Ptr &context) const
{
   const DynamicContext::Ptr stackContext(context->createStack());
   Q_ASSERT(stackContext);

   const Expression::List::const_iterator end(m_operands.constEnd());
   Expression::List::const_iterator it(m_operands.constBegin());

   VariableSlotID slot = m_expressionSlotOffset;

   for (; it != end; ++it) {
      stackContext->setExpressionVariable(slot,
                                          Expression::Ptr(new DynamicContextStore(*it, context)));
      ++slot;
   }

   return stackContext;
}

SequenceType::List UserFunctionCallsite::expectedOperandTypes() const
{
   SequenceType::List result;

   if (m_functionDeclaration) {
      const FunctionArgument::List args(m_functionDeclaration->signature()->arguments());
      const FunctionArgument::List::const_iterator end(args.constEnd());
      FunctionArgument::List::const_iterator it(args.constBegin());

      for (; it != end; ++it) {
         result.append((*it)->type());
      }
   } else {
      result.append(CommonSequenceTypes::ZeroOrMoreItems);
   }

   return result;
}

Expression::Ptr UserFunctionCallsite::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   /* The parser calls TypeChecker::applyFunctionConversion() on user function
    * bodies, possibly indirectly, before all function call sites have been
    * resolved. Hence it's possible that we're called before before the usual
    * typeCheck() pass, and hence before we have been resolved/checked and
    * subsequently m_functionDeclaration set. Therefore, encounter for that below.
    *
    * UnresolvedVariableReference::typeCheck() has the same dilemma.
    */

   /* Ensure that the return value of the function is properly
    * converted/does match from where it is called(which is here). */
   if (isRecursive() || !m_functionDeclaration) {
      return CallSite::typeCheck(context, reqType);
   } else {
      /* Update, such that we use a recent version of the body that has typeCheck()
       * and compress() rewrites included. */
      m_body = m_functionDeclaration->body();

      /* Note, we can't assign to m_functionDeclaration->body() because UserFunction can apply
       * to several different callsites. Hence we need our own version. */
      m_body = m_body->typeCheck(context, reqType);

      /* We just act as a pipe for m_body, so we don't have to typecheck ourselves. However,
       * the arguments must match the function declaration. */
      typeCheckOperands(context);
      return Expression::Ptr(this);
   }
}

Expression::Ptr UserFunctionCallsite::compress(const StaticContext::Ptr &context)
{
   if (!isRecursive()) {
      rewrite(m_body, m_body->compress(context), context);
   }

   return CallSite::compress(context);
}

Expression::Properties UserFunctionCallsite::properties() const
{
   return DisableElimination;
}

SequenceType::Ptr UserFunctionCallsite::staticType() const
{
   /* Our return type, is the static type of the function body. We could have also used
    * m_functionDeclaration->signature()->returnType(), but it doesn't get updated
    * when function conversion is applied.
    * We can't use m_body's type if we're recursive, because m_body computes its type
    * from its children, and we're at least one of the children. Hence, we would
    * recurse infinitely if we did.
    *
    * m_body can be null here if we're called before setSource().
    */
   if (isRecursive() || !m_body) {
      return CommonSequenceTypes::ZeroOrMoreItems;   // TODO use the declaration, it can have a type explicitly.
   } else {
      return m_body->staticType();
   }
}

ExpressionVisitorResult::Ptr UserFunctionCallsite::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID UserFunctionCallsite::id() const
{
   return IDUserFunctionCallsite;
}

bool UserFunctionCallsite::isSignatureValid(const FunctionSignature::Ptr &sign) const
{
   Q_ASSERT(sign);

   return sign->name() == name()
          &&
          sign->isArityValid(m_arity);
}

bool UserFunctionCallsite::configureRecursion(const CallTargetDescription::Ptr &sign)
{
   Q_ASSERT(sign);

   setIsRecursive(isSignatureValid(sign));
   return isRecursive();
}

void UserFunctionCallsite::setSource(const UserFunction::Ptr &userFunction,
                                     const VariableSlotID cacheSlotOffset)
{
   m_functionDeclaration = userFunction;
   m_body = userFunction->body();
   m_expressionSlotOffset = userFunction->expressionSlotOffset();

   const int len = m_operands.size();

   const VariableDeclaration::List varDecls(userFunction->argumentDeclarations());

   for (int i = 0; i < len; ++i) {
      /* We don't want evaluation caches for range variables, it's not necessary since
       * the item is already cached in DynamicContext::rangeVariable(). */
      if (m_operands.at(i)->is(IDRangeVariableReference)) {
         continue;
      }

      /* Note that we pass in cacheSlotOffset + i here instead of varDecls.at(i)->slot since
       * we want independent caches for each callsite. */
      m_operands[i] = Expression::Ptr(new EvaluationCache<false>(m_operands.at(i),
                                      varDecls.at(i).data(),
                                      cacheSlotOffset + i));
   }
}

FunctionSignature::Arity UserFunctionCallsite::arity() const
{
   return m_arity;
}

CallTargetDescription::Ptr UserFunctionCallsite::callTargetDescription() const
{
   return m_functionDeclaration->signature();
}

QT_END_NAMESPACE
