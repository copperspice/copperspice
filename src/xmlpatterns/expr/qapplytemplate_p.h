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

#ifndef QApplyTemplate_P_H
#define QApplyTemplate_P_H

#include <qtemplatemode_p.h>

namespace QPatternist {

class ApplyTemplate : public TemplateInvoker
{
 public:
   typedef QExplicitlySharedDataPointer<ApplyTemplate> Ptr;

   ApplyTemplate(const TemplateMode::Ptr &mode, const WithParam::Hash &withParams, const TemplateMode::Ptr &defaultMode);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;

   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   Properties properties() const override;

   inline Item mapToItem(const QXmlNodeModelIndex &node, const DynamicContext::Ptr &context) const;
   inline Item::Iterator::Ptr mapToSequence(const Item &item, const DynamicContext::Ptr &context) const;

   inline TemplateMode::Ptr mode() const;

   bool configureRecursion(const CallTargetDescription::Ptr &sign) override;
   Expression::Ptr body() const override;
   CallTargetDescription::Ptr callTargetDescription() const override;

   Expression::Ptr compress(const StaticContext::Ptr &context) override;

 private:
   typedef QExplicitlySharedDataPointer<const ApplyTemplate> ConstPtr;

   Template::Ptr findTemplate(const DynamicContext::Ptr &context, const TemplateMode::Ptr &templateMode) const;

   const TemplateMode::Ptr m_mode;

   TemplateMode::Ptr m_defaultMode;

   inline TemplateMode::Ptr effectiveMode(const DynamicContext::Ptr &context) const;
};

TemplateMode::Ptr ApplyTemplate::mode() const
{
   return m_mode;
}
}

#endif
