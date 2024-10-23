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

#ifndef QXPathHelper_P_H
#define QXPathHelper_P_H

#include <qstringfwd.h>

#include <qcommonnamespaces_p.h>
#include <qitem_p.h>
#include <qpatternistlocale_p.h>
#include <qreportcontext_p.h>

namespace QPatternist {
class XPathHelper
{
 public:
   static bool isQName(const QString &qName);
   static void splitQName(const QString &qName, QString &prefix, QString &localName);
   static bool isReservedNamespace(const QXmlName::NamespaceCode ns);

   template<const ReportContext::ErrorCode code, typename TReportContext>

   static void checkCollationSupport(const QString &collation, const TReportContext &context,
            const SourceLocationReflection *const r) {
      Q_ASSERT(context);
      Q_ASSERT(r);

      if (collation != QString::fromLatin1(CommonNamespaces::UNICODE_COLLATION)) {
         context->error(QtXmlPatterns::tr("Only the Unicode Codepoint Collation is supported(%1). %2 is unsupported.")
                        .formatArg(formatURI(QString::fromLatin1(CommonNamespaces::UNICODE_COLLATION)))
                        .formatArg(formatURI(collation)), code, r);
      }
   }

   static QPatternist::ItemType::Ptr typeFromKind(const QXmlNodeModelIndex::NodeKind nodeKind);
   static QUrl normalizeQueryURI(const QUrl &uri);

   static bool isWhitespaceOnly(QStringView string) {
      const int len = string.length();

      for (int i = 0; i < len; ++i) {
         if (! string.at(i).isSpace()) {
            // emerald - return value might be incorrect
            return false;
         }
      }

      return true;
   }

   static bool isWhitespaceOnly(const QString &string) {
      return isWhitespaceOnly(QStringView(string));
   }

 private:
   inline XPathHelper();

   XPathHelper(const XPathHelper &) = delete;
   XPathHelper &operator=(const XPathHelper &) = delete;
};
}

#endif
