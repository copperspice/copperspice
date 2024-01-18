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

#include "qtemplateinvoker_p.h"

using namespace QPatternist;

TemplateInvoker::TemplateInvoker(const WithParam::Hash &withParams,
                                 const QXmlName &name) : CallSite(name)
   , m_withParams(withParams)
{
   const WithParam::Hash::const_iterator end(m_withParams.constEnd());

   for (WithParam::Hash::const_iterator it(m_withParams.constBegin()); it != end; ++it) {
      /* In the case of for instance:
       *  <xsl:with-param name="empty_seq" as="item()"/>
       *
       * we have no default expression. */
      Q_ASSERT(it.value()->sourceExpression());
      m_operands.append(it.value()->sourceExpression());
   }
}

Expression::Ptr TemplateInvoker::compress(const StaticContext::Ptr &context)
{
   /* CallSite::compress() may have changed our children, so update
    * our m_withParams. */
   const Expression::Ptr me(CallSite::compress(context));
   const WithParam::Hash::const_iterator end(m_withParams.constEnd());
   int exprIndex = -1;

   for (WithParam::Hash::const_iterator it(m_withParams.constBegin()); it != end; ++it) {
      if (it.value()->sourceExpression()) {
         ++exprIndex;
         it.value()->setSourceExpression(m_operands.at(exprIndex));
      }
   }

   return me;
}

SequenceType::List TemplateInvoker::expectedOperandTypes() const
{
   SequenceType::List result;

   /* We don't return the type of the m_template->templateParameters(), we
    * return the type of the @c xsl:with-param first. @em After that, we
    * manually apply the parameter types in typeCheck(). */
   const WithParam::Hash::const_iterator end(m_withParams.constEnd());

   for (WithParam::Hash::const_iterator it(m_withParams.constBegin()); it != end; ++it) {
      /* We're not guaranteed to have a with-param, we may be using the
       * default value of the xsl:param. Tunnel parameters may also play
       * in. */
      result.append(it.value()->type());
   }

   return result;
}
