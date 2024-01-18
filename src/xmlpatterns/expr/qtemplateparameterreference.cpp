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

#include "qcommonsequencetypes_p.h"

#include "qtemplateparameterreference_p.h"

using namespace QPatternist;

TemplateParameterReference::TemplateParameterReference(const VariableDeclaration *varDecl) : m_varDecl(varDecl)
{
}

bool TemplateParameterReference::evaluateEBV(const DynamicContext::Ptr &context) const
{
   return context->templateParameterStore()[m_varDecl->name]->evaluateEBV(context);
}

Item TemplateParameterReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return context->templateParameterStore()[m_varDecl->name]->evaluateSingleton(context);
}

Item::Iterator::Ptr TemplateParameterReference::evaluateSequence(const DynamicContext::Ptr &context) const
{
   Q_ASSERT(!m_varDecl->name.isNull());
   Q_ASSERT(context->templateParameterStore()[m_varDecl->name]);
   return context->templateParameterStore()[m_varDecl->name]->evaluateSequence(context);
}

ExpressionVisitorResult::Ptr TemplateParameterReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::Properties TemplateParameterReference::properties() const
{
   return DisableElimination;
}

SequenceType::Ptr TemplateParameterReference::staticType() const
{
   /* We can't use m_varDecl->expression()'s static type here, because
    * it's the default argument. */
   if (!m_varDecl->sequenceType) {
      return CommonSequenceTypes::ZeroOrMoreItems;
   } else {
      return m_varDecl->sequenceType;
   }
}
