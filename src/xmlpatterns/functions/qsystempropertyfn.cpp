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

#include "qatomicstring_p.h"
#include "qqnameconstructor_p.h"

#include "qsystempropertyfn_p.h"

using namespace QPatternist;

Item SystemPropertyFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const QString lexQName(m_operands.first()->evaluateSingleton(context).stringValue());

   const QXmlName name
   (QNameConstructor::expandQName<DynamicContext::Ptr, ReportContext::XTDE1390,
    ReportContext::XTDE1390>(lexQName, context, staticNamespaces(), this));

   return AtomicString::fromValue(retrieveProperty(name));
}

QString SystemPropertyFN::retrieveProperty(const QXmlName name)
{
   if (name.namespaceURI() != StandardNamespaces::xslt) {
      return QString();
   }

   switch (name.localName()) {
      case StandardLocalNames::version:
         /*
          * The supported XSL-T version.
          *
          * @see <a href="http://www.w3.org/TR/xslt20/#system-property">The Note paragraph
          * at the very end of XSL Transformations (XSLT) Version 2.0,
          * 16.6.5 system-property</a>
          */
         return QString::number(1.20);

      case StandardLocalNames::vendor:
         return QLatin1String("CopperSpice");

      case StandardLocalNames::vendor_url:
         return QLatin1String("http://www.copperspice.com/");

      case StandardLocalNames::product_name:
         return QLatin1String("QtXmlPatterns");

      case StandardLocalNames::product_version:
         return QLatin1String("0.1");

      case StandardLocalNames::is_schema_aware:
      case StandardLocalNames::supports_backwards_compatibility:
      case StandardLocalNames::supports_serialization:
         return QLatin1String("no");

      default:
         return QString();
   }
}
