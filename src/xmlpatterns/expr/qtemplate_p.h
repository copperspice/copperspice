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

#ifndef QTemplate_P_H
#define QTemplate_P_H

#include <QSharedData>
#include <QVector>

#include "qdynamiccontext_p.h"
#include "qexpression_p.h"
#include "qsourcelocationreflection_p.h"
#include "qtemplateinvoker_p.h"
#include "qvariabledeclaration_p.h"

namespace QPatternist {

class Template : public QSharedData, public SourceLocationReflection

{
 public:
   typedef QExplicitlySharedDataPointer<Template> Ptr;
   typedef QVector<Template::Ptr> Vector;

   inline Template(const ImportPrecedence ip,
                   const SequenceType::Ptr &reqType) : importPrecedence(ip)
      , m_reqType(reqType) {
   }

   Expression::Ptr body;

   /**
    * Returns @c this.
    */
   const SourceLocationReflection *actualReflection() const override;

   const ImportPrecedence importPrecedence;

   VariableDeclaration::List templateParameters;

   /**
    * If @p isCallTemplate, the caller is @c xsl:call-template, as opposed
    * to for instance @c xsl:apply-templates. This affects error
    * reporting.
    */
   DynamicContext::Ptr createContext(const TemplateInvoker *const invoker,
                                     const DynamicContext::Ptr &context,
                                     const bool isCallTemplate) const;

   /**
    * Since we have our template parameters in templateParameters, we need
    * this separate step to do the regular phases:
    * Expression::typeCheck(), and Expression::compress().
    */
   void compileParameters(const StaticContext::Ptr &context);

   /**
    * A value which takes into account the body and its template
    * parameters.
    */
   Expression::Properties properties() const;

   Expression::Properties dependencies() const;

   static void raiseXTSE0680(const ReportContext::Ptr &context,
                             const QXmlName &name,
                             const SourceLocationReflection *const reflection);

 private:
   DynamicContext::TemplateParameterHash parametersAsHash() const;
   const SequenceType::Ptr m_reqType;
};

}

#endif
