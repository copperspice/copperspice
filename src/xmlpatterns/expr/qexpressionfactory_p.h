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

#ifndef QExpressionFactory_P_H
#define QExpressionFactory_P_H

#include <QXmlQuery>
#include <qexpression_p.h>
#include <qtokenizer_p.h>
#include <QSharedData>
#include <QUrl>

class QIODevice;

namespace QPatternist {

class ExpressionFactory : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<ExpressionFactory> Ptr;

   ExpressionFactory() = default;

   virtual ~ExpressionFactory() {
   }

   enum CompilationStage {
      QueryBodyInitial        = 1,
      QueryBodyTypeCheck      = 1 << 1,
      QueryBodyCompression    = 1 << 2,
      UserFunctionTypeCheck   = 1 << 3,
      UserFunctionCompression = 1 << 4,
      GlobalVariableTypeCheck = 1 << 5
   };

   virtual Expression::Ptr createExpression(const QString &expr,
         const StaticContext::Ptr &context,
         const QXmlQuery::QueryLanguage lang,
         const SequenceType::Ptr &requiredType,
         const QUrl &queryURI,
         const QXmlName &initialTemplateName);

   virtual Expression::Ptr createExpression(QIODevice *const device,
         const StaticContext::Ptr &context,
         const QXmlQuery::QueryLanguage lang,
         const SequenceType::Ptr &requiredType,
         const QUrl &queryURI,
         const QXmlName &initialTemplateName);

   static void registerLastPath(const Expression::Ptr &operand);

 protected:
   enum TemplateCompilationStage {
      TemplateInitial         = 1,
      TemplateTypeCheck       = 1 << 1,
      TemplateCompress        = 1 << 2
   };

   virtual void processTreePass(const Expression::Ptr &tree,
                                const CompilationStage stage);

   virtual void processTemplateRule(const Expression::Ptr &body,
                                    const TemplatePattern::Ptr &pattern,
                                    const QXmlName &mode,
                                    const TemplateCompilationStage stage);

   virtual void processNamedTemplate(const QXmlName &name,
                                     const Expression::Ptr &tree,
                                     const TemplateCompilationStage stage);

   Expression::Ptr createExpression(const Tokenizer::Ptr &tokenizer,
                                    const StaticContext::Ptr &context,
                                    const QXmlQuery::QueryLanguage lang,
                                    const SequenceType::Ptr &requiredType,
                                    const QUrl &queryURI,
                                    const QXmlName &initialTemplateName);
 private:
   ExpressionFactory(const ExpressionFactory &) = delete;
   ExpressionFactory &operator=(const ExpressionFactory &) = delete;
};

}

#endif
