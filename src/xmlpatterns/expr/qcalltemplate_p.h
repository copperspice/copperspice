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

#ifndef QCallTemplate_P_H
#define QCallTemplate_P_H

#include <qcallsite_p.h>
#include <qtemplateinvoker_p.h>
#include <qtemplate_p.h>

namespace QPatternist {

class CallTemplate : public TemplateInvoker
{
 public:
   typedef QExplicitlySharedDataPointer<CallTemplate> Ptr;

   CallTemplate(const QXmlName &name, const WithParam::Hash &withParams);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   bool evaluateEBV(const DynamicContext::Ptr &context) const override;
   void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const override;

   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   Properties properties() const override;
   Properties dependencies() const override;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   void setTemplate(const Template::Ptr &templ) {
      m_template = templ;
   }

   bool configureRecursion(const CallTargetDescription::Ptr &sign) override;
   Expression::Ptr body() const override;
   CallTargetDescription::Ptr callTargetDescription() const override;

 private:
   Template::Ptr   m_template;
};

}

#endif
