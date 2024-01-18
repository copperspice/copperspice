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

#include "qcommonsequencetypes_p.h"

#include "qcalltemplate_p.h"

using namespace QPatternist;

CallTemplate::CallTemplate(const QXmlName &name,
                           const WithParam::Hash &withParams) : TemplateInvoker(withParams, name)
{
}

Item::Iterator::Ptr CallTemplate::evaluateSequence(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(m_template);
   return m_template->body->evaluateSequence(m_template->createContext(this, context, true));
}

bool CallTemplate::evaluateEBV(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(m_template);
   return m_template->body->evaluateEBV(m_template->createContext(this, context, true));
}

void CallTemplate::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(m_template);
   m_template->body->evaluateToSequenceReceiver(m_template->createContext(this, context, true));
}

Expression::Ptr CallTemplate::typeCheck(const StaticContext::Ptr &context,
                                        const SequenceType::Ptr &reqType)
{
   /* Check XTSE0680, that every @c xsl:with-param has a corresponding @c
    * xsl:param declaration. */
   {
      const WithParam::Hash::const_iterator end(m_withParams.constEnd());

      for (WithParam::Hash::const_iterator it(m_withParams.constBegin());
            it != end;
            ++it) {
         if (!VariableDeclaration::contains(m_template->templateParameters, it.value()->name())) {
            Template::raiseXTSE0680(context, it.value()->name(), this);
         }
      }
   }

   const Expression::Ptr me(TemplateInvoker::typeCheck(context, reqType));

   const VariableDeclaration::List args(m_template->templateParameters);
   const VariableDeclaration::List::const_iterator end(args.constEnd());
   VariableDeclaration::List::const_iterator it(args.constBegin());

   for (; it != end; ++it) {
      // TODO
      Q_ASSERT((*it)->sequenceType);
   }

   return me;
}

Expression::Properties CallTemplate::properties() const
{
   Q_ASSERT(!m_template || m_template->body);

   /* We may be called before our m_template is resolved, namely when we're
    * the body of a variable. In that case querytransformparser.ypp will
    * manually call TypeChecker::applyFunctionConversion(), which is before
    * ExpressionFactory::createExpression() has resolved us. */
   if (m_template && !isRecursive()) {
      return m_template->properties();
   } else {
      return Properties();
   }
}

Expression::Properties CallTemplate::dependencies() const
{
   if (m_template && !isRecursive()) {
      return m_template->dependencies();
   } else {
      return Properties();
   }
}

SequenceType::Ptr CallTemplate::staticType() const
{
   return CommonSequenceTypes::ZeroOrMoreItems;
}

ExpressionVisitorResult::Ptr CallTemplate::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

bool CallTemplate::configureRecursion(const CallTargetDescription::Ptr &sign)
{
   (void) sign;
   return false;
}

Expression::Ptr CallTemplate::body() const
{
   return m_template->body;
}

CallTargetDescription::Ptr CallTemplate::callTargetDescription() const
{
   return CallTargetDescription::Ptr();
}
