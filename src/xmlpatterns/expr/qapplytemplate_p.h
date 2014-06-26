/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QApplyTemplate_P_H
#define QApplyTemplate_P_H

#include <qtemplatemode_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class ApplyTemplate : public TemplateInvoker
{
 public:
   typedef QExplicitlySharedDataPointer<ApplyTemplate> Ptr;

   ApplyTemplate(const TemplateMode::Ptr &mode, const WithParam::Hash &withParams, const TemplateMode::Ptr &defaultMode);

   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;

   virtual SequenceType::Ptr staticType() const;
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual Properties properties() const;


   inline Item mapToItem(const QXmlNodeModelIndex &node, const DynamicContext::Ptr &context) const;
   inline Item::Iterator::Ptr mapToSequence(const Item &item, const DynamicContext::Ptr &context) const;

   inline TemplateMode::Ptr mode() const;

   virtual bool configureRecursion(const CallTargetDescription::Ptr &sign);
   virtual Expression::Ptr body() const;
   virtual CallTargetDescription::Ptr callTargetDescription() const;

   Expression::Ptr compress(const StaticContext::Ptr &context);

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

QT_END_NAMESPACE

#endif
