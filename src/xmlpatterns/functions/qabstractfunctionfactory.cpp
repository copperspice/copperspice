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

#include "qpatternistlocale_p.h"
#include "qabstractfunctionfactory_p.h"

using namespace QPatternist;

Expression::Ptr AbstractFunctionFactory::createFunctionCall(const QXmlName name, const Expression::List &args,
      const StaticContext::Ptr &context, const SourceLocationReflection *const r)
{
   const FunctionSignature::Ptr sign(retrieveFunctionSignature(context->namePool(), name));

   if (! sign) {
      // The function doesn't exist(at least not in this factory).
      return Expression::Ptr();
   }

   /* May throw. */
   verifyArity(sign, context, args.count(), r);

   /* Ok, the function does exist and the arity is correct. */
   return retrieveExpression(name, args, sign);
}

void AbstractFunctionFactory::verifyArity(const FunctionSignature::Ptr &s, const StaticContext::Ptr &context,
      const xsInteger arity, const SourceLocationReflection *const r) const
{
   /* Same code in both branches, but more specific error messages in order
    * to improve usability. */

   if (s->maximumArguments() != FunctionSignature::UnlimitedArity && arity > s->maximumArguments()) {

      context->error(QtXmlPatterns::tr("%1 takes at most %n argument(s), %2 is therefore invalid.", nullptr, s->maximumArguments())
              .formatArg(formatFunction(context->namePool(), s)).formatArg(arity), ReportContext::XPST0017, r);
      return;
   }

   if (arity < s->minimumArguments()) {
      context->error(QtXmlPatterns::tr("%1 requires at least %n argument(s), %2 is therefore invalid.",
               nullptr, s->minimumArguments())
              .formatArg(formatFunction(context->namePool(), s)).formatArg(arity), ReportContext::XPST0017, r);

      return;
   }
}

FunctionSignature::Hash AbstractFunctionFactory::functionSignatures() const
{
   return m_signatures;
}

