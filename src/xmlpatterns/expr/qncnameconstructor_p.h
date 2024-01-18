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

#ifndef QNCNameConstructor_P_H
#define QNCNameConstructor_P_H

#include "qsinglecontainer_p.h"
#include "qpatternistlocale_p.h"
#include "qxmlutils_p.h"


namespace QPatternist {

class NCNameConstructor : public SingleContainer
{
 public:

   NCNameConstructor(const Expression::Ptr &source);

   Item evaluateSingleton(const DynamicContext::Ptr &) const override;

   SequenceType::List expectedOperandTypes() const override;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   SequenceType::Ptr staticType() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   /**
    *  Validates @p lexicalNCName as a processing instruction's target
    *  name, and raise an error if it's not an @c  NCName.
    */
   template<typename TReportContext, const ReportContext::ErrorCode NameIsXML,
            const ReportContext::ErrorCode LexicallyInvalid>

   static inline void validateTargetName(const QString &lexicalNCName,
                 const TReportContext &context, const SourceLocationReflection *const r);
 private:

   /**
    * This translation string is put here in order to avoid duplicate messages and
    * hence reduce work for translators and increase consistency.
    */
   static
   const QString nameIsXML(const QString &lexTarget) {
      return QtXmlPatterns::tr("The target name in a processing instruction "
                               "cannot be %1 in any combination of upper "
                               "and lower case. Therefore, %2 is invalid.")
             .formatArgs(formatKeyword("xml"), formatKeyword(lexTarget));
   }
};

template<typename TReportContext, const ReportContext::ErrorCode NameIsXML, const ReportContext::ErrorCode LexicallyInvalid>
inline void NCNameConstructor::validateTargetName(const QString &lexicalTarget, const TReportContext &context,
      const SourceLocationReflection *const r)
{
   Q_ASSERT(context);

   if (QXmlUtils::isNCName(lexicalTarget)) {
      if (QString::compare(QLatin1String("xml"), lexicalTarget, Qt::CaseInsensitive) == 0) {
         context->error(nameIsXML(lexicalTarget), NameIsXML, r);
      }

   } else {
      context->error(QtXmlPatterns::tr("%1 is not a valid target name in a processing instruction. It "
                     "must be a %2 value, e.g. %3.")
                     .formatArg(formatKeyword(lexicalTarget)).formatArg(formatType(context->namePool(), BuiltinTypes::xsNCName))
                     .formatArg(formatKeyword("my-name.123")), LexicallyInvalid, r);
   }
}
}

#endif
