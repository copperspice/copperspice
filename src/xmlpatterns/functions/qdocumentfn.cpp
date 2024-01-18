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

#include "qforclause_p.h"
#include "qfunctionfactory_p.h"
#include "qrangevariablereference_p.h"
#include "qdocumentfn_p.h"

using namespace QPatternist;

Expression::Ptr DocumentFN::typeCheck(const StaticContext::Ptr &context,
                                      const SequenceType::Ptr &reqType)
{
   /* See the class documentation for the rewrite that we're doing here. */

   /* Generate type checking code for our operands such that they match. */
   typeCheckOperands(context);

   const QSourceLocation myLocation(context->locationFor(this));
   const FunctionFactory::Ptr functions(context->functionSignatures());

   Expression::Ptr uriSource;

   {
      Expression::List distinctValuesArgs;
      distinctValuesArgs.append(m_operands.first());

      uriSource = functions->createFunctionCall(QXmlName(StandardNamespaces::fn, StandardLocalNames::distinct_values),
                  distinctValuesArgs,
                  context,
                  this);
      context->addLocation(uriSource.data(), myLocation);
   }

   const VariableSlotID rangeSlot = context->allocateRangeSlot();
   const Expression::Ptr uriReference(new RangeVariableReference(uriSource, rangeSlot));
   context->addLocation(uriReference.data(), myLocation);

   Expression::List docArgs;

   if (m_operands.count() == 2) {
      Expression::List baseUriArgs;
      baseUriArgs.append(uriReference);
      baseUriArgs.append(m_operands.at(1));

      const Expression::Ptr fnBaseUri(functions->createFunctionCall(QXmlName(StandardNamespaces::fn,
                                      StandardLocalNames::resolve_uri),
                                      baseUriArgs,
                                      context,
                                      this));
      context->addLocation(fnBaseUri.data(), myLocation);
      docArgs.append(fnBaseUri);
   } else {
      docArgs.append(uriReference);
   }

   const Expression::Ptr fnDoc(functions->createFunctionCall(QXmlName(StandardNamespaces::fn, StandardLocalNames::doc),
                               docArgs,
                               context,
                               this));
   context->addLocation(fnDoc.data(), myLocation);


   Expression::Ptr newMe(new ForClause(rangeSlot,
                                       uriSource,
                                       fnDoc,
                                       -1 /* We have no position variable. */));

   Expression::Ptr oldMe(this);
   rewrite(oldMe, newMe, context);
   return newMe->typeCheck(context, reqType);
}
