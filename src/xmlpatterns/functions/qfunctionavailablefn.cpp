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

#include "qboolean_p.h"
#include "qdelegatingnamespaceresolver_p.h"
#include "qinteger_p.h"
#include "qqnameconstructor_p.h"

#include "qfunctionavailablefn_p.h"

using namespace QPatternist;

Item FunctionAvailableFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const QString lexQName(m_operands.first()->evaluateSingleton(context).stringValue());

   NamespaceResolver::Bindings override;
   override.insert(StandardPrefixes::empty, m_defFuncNS);

   const NamespaceResolver::Ptr resolver(new DelegatingNamespaceResolver(staticNamespaces(), override));

   const QXmlName name
   (QNameConstructor::expandQName<DynamicContext::Ptr,
    ReportContext::XTDE1400,
    ReportContext::XTDE1400>(lexQName,
                             context,
                             resolver,
                             this));

   xsInteger arity;

   if (m_operands.count() == 2) {
      arity = m_operands.at(1)->evaluateSingleton(context).as<Numeric>()->toInteger();
   } else {
      arity = FunctionSignature::UnlimitedArity;
   }

   return Boolean::fromValue(m_functionFactory->isAvailable(context->namePool(), name, arity));
}

Expression::Ptr FunctionAvailableFN::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   m_functionFactory = context->functionSignatures();
   Q_ASSERT(m_functionFactory);
   m_defFuncNS = context->namePool()->allocateNamespace(context->defaultFunctionNamespace());
   /* m_defFuncNS can be empty/null or an actual value. */

   return StaticNamespacesContainer::typeCheck(context, reqType);
}
