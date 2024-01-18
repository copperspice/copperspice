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

#include "qqnameconstructor_p.h"

#include "qelementavailablefn_p.h"

using namespace QPatternist;

ElementAvailableFN::ElementAvailableFN() : m_xsltInstructions(allXSLTInstructions())
{
}

QSet<QString> ElementAvailableFN::allXSLTInstructions()
{
   enum {
      StringSetSize = 27
   };

   QSet<QString> retval;
   retval.reserve(StringSetSize);

   /* Alphabetically. */
   retval.insert(QLatin1String("analyze-string"));
   retval.insert(QLatin1String("apply-imports"));
   retval.insert(QLatin1String("apply-templates"));
   retval.insert(QLatin1String("attribute"));
   retval.insert(QLatin1String("attribute-set"));
   retval.insert(QLatin1String("call-template"));
   retval.insert(QLatin1String("character-map"));
   retval.insert(QLatin1String("choose"));
   retval.insert(QLatin1String("comment"));
   retval.insert(QLatin1String("copy"));
   retval.insert(QLatin1String("copy-of"));
   retval.insert(QLatin1String("document"));
   retval.insert(QLatin1String("element"));
   retval.insert(QLatin1String("fallback"));
   retval.insert(QLatin1String("for-each"));
   retval.insert(QLatin1String("for-each-group"));
   retval.insert(QLatin1String("if"));
   retval.insert(QLatin1String("message"));
   retval.insert(QLatin1String("namespace"));
   retval.insert(QLatin1String("next-match"));
   retval.insert(QLatin1String("number"));
   retval.insert(QLatin1String("perform-sort"));
   retval.insert(QLatin1String("processing-instruction"));
   retval.insert(QLatin1String("result-document"));
   retval.insert(QLatin1String("sequence"));
   retval.insert(QLatin1String("text"));
   retval.insert(QLatin1String("variable"));

   Q_ASSERT(retval.count() == StringSetSize);
   return retval;
}

bool ElementAvailableFN::evaluateEBV(const DynamicContext::Ptr &context) const
{
   const Item arg(m_operands.first()->evaluateSingleton(context));
   const QString stringName(arg.stringValue());

   const QXmlName elementName(QNameConstructor::expandQName<DynamicContext::Ptr,
                              ReportContext::XTDE1440,
                              ReportContext::XTDE1440>(stringName,
                                    context,
                                    staticNamespaces(),
                                    this,
                                    false));

   if (elementName.namespaceURI() != StandardNamespaces::xslt) {
      return false;
   }

   QString prefix;
   QString localName;
   XPathHelper::splitQName(stringName, prefix, localName);

   return m_xsltInstructions.contains(localName);
}
