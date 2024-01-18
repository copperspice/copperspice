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

#include <qxpathhelper_p.h>

#include <qstringlist.h>

#include <qxmlutils_p.h>
#include <qbuiltintypes_p.h>
#include <qcommonvalues_p.h>
#include <qnamepool_p.h>

using namespace QPatternist;

bool XPathHelper::isReservedNamespace(const QXmlName::NamespaceCode ns)
{
   /* The order is because of that XFN and WXS are the most common. */
   return ns == StandardNamespaces::fn     ||
          ns == StandardNamespaces::xs     ||
          ns == StandardNamespaces::xml    ||
          ns == StandardNamespaces::xsi;
}

bool XPathHelper::isQName(const QString &qName)
{
   const QStringList result(qName.split(QLatin1Char(':')));
   const int c = result.count();

   if (c == 2) {
      return QXmlUtils::isNCName(result.first()) &&
             QXmlUtils::isNCName(result.last());
   } else if (c == 1) {
      return QXmlUtils::isNCName(result.first());
   } else {
      return false;
   }
}

void XPathHelper::splitQName(const QString &qName, QString &prefix, QString &ncName)
{
   Q_ASSERT_X(isQName(qName), Q_FUNC_INFO,
              "qName must be a valid QName.");

   const QStringList result(qName.split(QLatin1Char(':')));

   if (result.count() == 1) {
      Q_ASSERT(QXmlUtils::isNCName(result.first()));
      ncName = result.first();
   } else {
      Q_ASSERT(result.count() == 2);
      Q_ASSERT(QXmlUtils::isNCName(result.first()));
      Q_ASSERT(QXmlUtils::isNCName(result.last()));

      prefix = result.first();
      ncName = result.last();
   }
}

ItemType::Ptr XPathHelper::typeFromKind(const QXmlNodeModelIndex::NodeKind nodeKind)
{
   switch (nodeKind) {
      case QXmlNodeModelIndex::Element:
         return BuiltinTypes::element;
      case QXmlNodeModelIndex::Attribute:
         return BuiltinTypes::attribute;
      case QXmlNodeModelIndex::Text:
         return BuiltinTypes::text;
      case QXmlNodeModelIndex::ProcessingInstruction:
         return BuiltinTypes::pi;
      case QXmlNodeModelIndex::Comment:
         return BuiltinTypes::comment;

      case QXmlNodeModelIndex::Document:
         return BuiltinTypes::document;

      default: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "A node type that does not exist in the XPath Data Model was encountered.");
         return ItemType::Ptr(); /* Dummy, silence compiler warning. */
      }
   }
}

QUrl XPathHelper::normalizeQueryURI(const QUrl &uri)
{
   Q_ASSERT_X(uri.isEmpty() || uri.isValid(), Q_FUNC_INFO,
              "The URI passed to QXmlQuery::setQuery() must be valid or empty.");
   if (uri.isEmpty()) {
      return QUrl::fromLocalFile(QCoreApplication::applicationFilePath());
   } else if (uri.isRelative()) {
      return QUrl::fromLocalFile(QCoreApplication::applicationFilePath()).resolved(uri);
   } else {
      return uri;
   }
}
