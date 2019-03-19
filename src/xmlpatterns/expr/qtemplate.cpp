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

#include "qdynamiccontextstore_p.h"
#include "qpatternistlocale_p.h"

#include "qtemplate_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

const SourceLocationReflection *Template::actualReflection() const
{
   return this;
}

DynamicContext::TemplateParameterHash Template::parametersAsHash() const
{
   DynamicContext::TemplateParameterHash retval;
   const int len = templateParameters.count();

   for (int i = 0; i < len; ++i) {
      const VariableDeclaration::Ptr &at = templateParameters.at(i);
      retval.insert(at->name, at->expression());
   }

   return retval;
}

void Template::raiseXTSE0680(const ReportContext::Ptr &context, const QXmlName &name, const SourceLocationReflection *const reflection)
{
   context->error(QtXmlPatterns::tr("The parameter %1 is passed, but no corresponding %2 exists.")
                  .formatArgs(formatKeyword(context->namePool(), name),
                  formatKeyword(QLatin1String("xsl:param"))), ReportContext::XTSE0680, reflection);
}

DynamicContext::Ptr Template::createContext(const TemplateInvoker *const invoker,
      const DynamicContext::Ptr &context, const bool isCallTemplate) const
{
   Q_ASSERT(invoker);
   Q_ASSERT(context);

   /* We have:
    * - xsl:params in the target template (if any) which may provide
    *   default values.
    * - xsl:with-params in the caller (if any) which provides values.
    *
    * We need to, for each parameter:
    * - If the called template provides no default value and the caller
    *   has no value, it's an error
    * - If the called template has a default value and the caller provides
    *   none, it should be used
    * - In any case the caller provides a value, it needs to be used.
    *
    * Problems to look out for:
    *
    * - Each xsl:param is in scope for the subsequent xsl:params. Hence,
    *   the evaluation of one xsl:param can depend on another xsl:param,
    *   and so on
    * - The focus for xsl:params is different from the focus for
    *   the xsl:with-params
    * - The xsl:with-params are not in scope for the xsl:params.
    */

   WithParam::Hash withParams(invoker->withParams());

   /**
    * Parameters or not, we must in any case create a new stack frame
    * for the template invocation since otherwise we will trash our existing
    * variables. Hence it's as with calling user functions.
    *
    * This is especially reproducible with recursive functions.
    */
   DynamicContext::Ptr newStack(context->createStack());

   /* We have no parameters, and we have no further error checking to
    * do in the case of not being xsl:apply-templates, so we need to do nothing. */
   if (templateParameters.isEmpty() && (!isCallTemplate || withParams.isEmpty())) {
      return newStack;
   }

   const DynamicContext::TemplateParameterHash hashedParams(parametersAsHash());
   DynamicContext::TemplateParameterHash sewnTogether(hashedParams);

   const DynamicContext::TemplateParameterHash::iterator end(sewnTogether.end());

   for (DynamicContext::TemplateParameterHash::iterator it(sewnTogether.begin());
         it != end;
         ++it) {
      Expression::Ptr &param = it.value();

      WithParam::Ptr &withParam = withParams[it.key()];

      if (withParam) {
         param = Expression::Ptr(new DynamicContextStore(withParam->sourceExpression(), context));
      } else if (!param) {
         /* Ops, no xsl:with-param and no default value to cover up for it.
          */
         context->error(QtXmlPatterns::tr("The parameter %1 is required, but no corresponding %2 is supplied.")
                        .formatArgs(formatKeyword(context->namePool(), it.key()), formatKeyword(QLatin1String("xsl:with-param"))),
                        ReportContext::XTSE0690, this);
      }
   }

   if (isCallTemplate) {
      /* Find xsl:with-param that has no corresponding xsl:param. */
      /* Optimization: candidate for threading? */

      const WithParam::Hash::const_iterator end(withParams.constEnd());

      for (WithParam::Hash::const_iterator it(withParams.constBegin()); it != end; ++it) {
         if (!hashedParams.contains(it.key())) {
            raiseXTSE0680(context, it.key(), this);
         }
      }

   }

   newStack->templateParameterStore() = sewnTogether;
   return newStack;
}

void Template::compileParameters(const StaticContext::Ptr &context)
{
   Q_ASSERT(context);

   const int len = templateParameters.count();

   for (int i = 0; i < len; ++i) {
      const VariableDeclaration::Ptr &at = templateParameters.at(i);

      /* If our value is required, we don't have a default value. */
      if (at->expression()) {
         // TODO why do we pass in its own type here?
         at->setExpression(at->expression()->typeCheck(context, at->expression()->staticType()));

         at->setExpression(at->expression()->compress(context));
      }
   }
}

Expression::Properties Template::properties() const
{
   return Expression::DisableElimination; /* We're having issues with recursion detection, so this path currently loops infintely. */

   Expression::Properties collect(body->properties());

   VariableDeclaration::List::const_iterator end(templateParameters.constEnd());

   for (VariableDeclaration::List::const_iterator it(templateParameters.constBegin());
         it != end;
         ++it) {
      if ((*it)->expression()) {
         collect |= (*it)->expression()->properties();
      }
   }

   // TODO simplify.
   return collect & (Expression::RequiresFocus | Expression::IsEvaluated | Expression::DisableElimination);
}

Expression::Properties Template::dependencies() const
{
   return Expression::DisableElimination; /* We're having issues with recursion detection, so this path currently loops infintely. */

   Expression::Properties collect(body->dependencies());

   VariableDeclaration::List::const_iterator end(templateParameters.constEnd());

   for (VariableDeclaration::List::const_iterator it(templateParameters.constBegin());
         it != end;
         ++it) {
      if ((*it)->expression()) {
         collect |= (*it)->expression()->dependencies();
      }
   }

   return collect & (Expression::RequiresFocus | Expression::IsEvaluated | Expression::DisableElimination);
}

QT_END_NAMESPACE
