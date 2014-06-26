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

#ifndef QCallTemplate_P_H
#define QCallTemplate_P_H

#include <qcallsite_p.h>
#include <qtemplateinvoker_p.h>
#include <qtemplate_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class CallTemplate : public TemplateInvoker
{
 public:
   typedef QExplicitlySharedDataPointer<CallTemplate> Ptr;

   CallTemplate(const QXmlName &name,
                const WithParam::Hash &withParams);

   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;
   virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

   virtual SequenceType::Ptr staticType() const;
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual Properties properties() const;
   virtual Properties dependencies() const;

   /**
    * This is a bit complicated by that we have two required types, one
    * specified by @c xsl:param in the template declaration, and one on @c
    * xsl:with-param.
    *
    * @see UserFunctionCallsite::expectedOperandTypes()
    * @see <a href="http://www.w3.org/TR/xslt20/#with-param">XSL
    * Transformations (XSLT) Version 2.0, 10.1.1 Passing Parameters to Templates</a>
    */
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType);


   inline void setTemplate(const Template::Ptr &templ) {
      m_template = templ;
   }

   virtual bool configureRecursion(const CallTargetDescription::Ptr &sign);
   virtual Expression::Ptr body() const;
   virtual CallTargetDescription::Ptr callTargetDescription() const;

 private:
   Template::Ptr   m_template;
};
}

QT_END_NAMESPACE

#endif
