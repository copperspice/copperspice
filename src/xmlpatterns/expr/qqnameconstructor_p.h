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

#ifndef QQNameConstructor_P_H
#define QQNameConstructor_P_H

#include <qsinglecontainer_p.h>
#include <qbuiltintypes_p.h>
#include <qpatternistlocale_p.h>
#include <qxpathhelper_p.h>

namespace QPatternist {

class QNameConstructor : public SingleContainer
{
 public:

   QNameConstructor(const Expression::Ptr &source, const NamespaceResolver::Ptr &nsResolver);

   Item evaluateSingleton(const DynamicContext::Ptr &) const override;
   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   template<typename TReportContext, const ReportContext::ErrorCode InvalidQName,
            const ReportContext::ErrorCode NoBinding>

   static QXmlName expandQName(const QString &lexicalQName,
                        const TReportContext &context,
                        const NamespaceResolver::Ptr &nsResolver,
                        const SourceLocationReflection *const r,
                        const bool asForAttribute = false);

   static QXmlName::NamespaceCode namespaceForPrefix(const QXmlName::PrefixCode prefix,
         const StaticContext::Ptr &context,
         const SourceLocationReflection *const r);

   const SourceLocationReflection *actualReflection() const override;

 private:
   const NamespaceResolver::Ptr m_nsResolver;
};

template<typename TReportContext,
         const ReportContext::ErrorCode InvalidQName,
         const ReportContext::ErrorCode NoBinding>
QXmlName QNameConstructor::expandQName(const QString &lexicalQName,
                                       const TReportContext &context,
                                       const NamespaceResolver::Ptr &nsResolver,
                                       const SourceLocationReflection *const r,
                                       const bool asForAttribute)
{
   Q_ASSERT(nsResolver);
   Q_ASSERT(context);

   if (XPathHelper::isQName(lexicalQName)) {
      QString prefix;
      QString local;

      XPathHelper::splitQName(lexicalQName, prefix, local);

      const QXmlName::NamespaceCode nsCode = asForAttribute &&
                  prefix.isEmpty() ? QXmlName::NamespaceCode(StandardNamespaces::empty)
                  : (nsResolver->lookupNamespaceURI(context->namePool()->allocatePrefix(prefix)));

      if (nsCode == NamespaceResolver::NoBinding) {
         context->error(QtXmlPatterns::tr("No namespace binding exists for the prefix %1 in %2")
                  .formatArgs(formatKeyword(prefix), formatKeyword(lexicalQName)), NoBinding, r);

         return QXmlName();

      } else {
         return context->namePool()->allocateQName(context->namePool()->stringForNamespace(nsCode), local, prefix);
      }
   } else {
      context->error(QtXmlPatterns::tr("%1 is an invalid %2")
                     .formatArg(formatData(lexicalQName)).formatArg(formatType(context->namePool(), BuiltinTypes::xsQName)), InvalidQName, r);

      return QXmlName();
   }
}

}

#endif
