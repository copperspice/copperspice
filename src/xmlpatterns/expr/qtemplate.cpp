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

#include "qdynamiccontextstore_p.h"
#include "qpatternistlocale_p.h"
#include "qtemplate_p.h"

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

   WithParam::Hash withParams(invoker->withParams());

   DynamicContext::Ptr newStack(context->createStack());

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
         context->error(QtXmlPatterns::tr("The parameter %1 is required, but no corresponding %2 is supplied.")
                        .formatArgs(formatKeyword(context->namePool(), it.key()), formatKeyword(QLatin1String("xsl:with-param"))),
                        ReportContext::XTSE0690, this);
      }
   }

   if (isCallTemplate) {

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

      if (at->expression()) {
         // TODO why do we pass in its own type here?
         at->setExpression(at->expression()->typeCheck(context, at->expression()->staticType()));

         at->setExpression(at->expression()->compress(context));
      }
   }
}

Expression::Properties Template::properties() const
{
   // having issues with recursion detection, this path currently loops infintely.
   return Expression::DisableElimination;

   Expression::Properties collect(body->properties());

   VariableDeclaration::List::const_iterator end(templateParameters.constEnd());

   for (VariableDeclaration::List::const_iterator it(templateParameters.constBegin());
         it != end;
         ++it) {
      if ((*it)->expression()) {
         collect |= (*it)->expression()->properties();
      }
   }

   return collect & (Expression::RequiresFocus | Expression::IsEvaluated | Expression::DisableElimination);
}

Expression::Properties Template::dependencies() const
{
   // having issues with recursion detection, this path currently loops infintely.
   return Expression::DisableElimination;

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
