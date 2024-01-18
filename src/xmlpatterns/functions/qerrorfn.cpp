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
#include "qpatternistlocale_p.h"
#include "qqnamevalue_p.h"
#include "qatomicstring_p.h"
#include "qerrorfn_p.h"

using namespace QPatternist;

Item ErrorFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   QString msg;

   switch (m_operands.count()) {
      case 0: { /* No args. */
         context->error(QtXmlPatterns::tr("%1 was called.").formatArg(formatFunction(context->namePool(), signature())),
                        ReportContext::FOER0000, this);
         return Item();
      }

      case 3:
      case 2:
         msg = m_operands.at(1)->evaluateSingleton(context).stringValue();
        [[fallthrough]];

      case 1: {
         const QNameValue::Ptr qName(m_operands.first()->evaluateSingleton(context).as<QNameValue>());

         if (qName) {
            context->error(msg, qName->qName(), this);
         } else {
            context->error(msg, ReportContext::FOER0000, this);
         }

         return Item();
      }

      default: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid number of arguments passed to fn:error.");
         return Item();
      }
   }
}

FunctionSignature::Ptr ErrorFN::signature() const
{
   const FunctionSignature::Ptr e(FunctionCall::signature());

   if (m_operands.count() != 1) {
      return e;
   }

   FunctionSignature::Ptr nev(FunctionSignature::Ptr(new FunctionSignature(e->name(),
                              e->minimumArguments(),
                              e->maximumArguments(),
                              e->returnType(),
                              e->properties())));
   const FunctionArgument::List args(e->arguments());
   FunctionArgument::List nargs;
   const QXmlName argName(StandardNamespaces::empty, StandardLocalNames::error);
   nargs.append(FunctionArgument::Ptr(new FunctionArgument(argName, CommonSequenceTypes::ExactlyOneQName)));
   nargs.append(args[1]);
   nargs.append(args[2]);
   nev->setArguments(nargs);

   return nev;
}
