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

#ifndef QAnyURI_P_H
#define QAnyURI_P_H

#include <QUrl>
#include <QDebug>
#include <qatomicstring_p.h>
#include <qbuiltintypes_p.h>
#include <qpatternistlocale_p.h>
#include <qreportcontext_p.h>

namespace QPatternist {

class AnyURI : public AtomicString
{
 public:
   typedef QExplicitlySharedDataPointer<AnyURI> Ptr;

   static AnyURI::Ptr fromValue(const QString &value);
   static AnyURI::Ptr fromValue(const QUrl &uri);

   template<const ReportContext::ErrorCode code, typename TReportContext>
   static inline QUrl toQUrl(const QString &value, const TReportContext &context,
               const SourceLocationReflection *const r, bool *const isValid = nullptr, const bool issueError = true) {

      /* QUrl doesn't flag ":/..." so we workaround it. */
      const QString simplified(value.simplified());
      const QUrl uri(simplified, QUrl::StrictMode);

      if (uri.isEmpty() || (uri.isValid() && (! simplified.startsWith(':') || ! uri.isRelative()))) {

         if (isValid) {
            *isValid = true;
         }

         return uri;

      } else {
         if (isValid) {
            *isValid = false;
         }

         if (issueError) {
            context->error(QtXmlPatterns::tr("%1 is not a valid value of type %2").
                  formatArgs(formatURI(value), formatType(context->namePool(), BuiltinTypes::xsAnyURI)), code, r);
         }

         return QUrl();
      }
   }

   static bool isValid(const QString &candidate);

   template<const ReportContext::ErrorCode code, typename TReportContext>
   static inline AnyURI::Ptr fromLexical(const QString &value, const TReportContext &context,
                  const SourceLocationReflection *const r) {

      return AnyURI::Ptr(new AnyURI(toQUrl<code>(value, context, r).toString()));
   }

   static AnyURI::Ptr fromLexical(const QString &value);
   static AnyURI::Ptr resolveURI(const QString &relative, const QString &base);

   ItemType::Ptr type() const override;

   inline QUrl toQUrl() const {
      Q_ASSERT_X(QUrl(m_value).isValid(), Q_FUNC_INFO, csPrintable(QString("%1 is not a valid QUrl").formatArg(m_value)));
      return QUrl(m_value);
   }

 protected:
   friend class CommonValues;

   AnyURI(const QString &value);
};

static inline QString formatURI(const NamePool::Ptr &np, const QXmlName::NamespaceCode &uri)
{
   return formatURI(np->stringForNamespace(uri));
}

} // namespace


#endif
