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

#include "qxsdattribute_p.h"
#include "qxsdcomplextype_p.h"

using namespace QPatternist;

void XsdAttribute::Scope::setVariety(Variety variety)
{
   m_variety = variety;
}

XsdAttribute::Scope::Variety XsdAttribute::Scope::variety() const
{
   return m_variety;
}

void XsdAttribute::Scope::setParent(const NamedSchemaComponent::Ptr &parent)
{
   m_parent = parent.data();
}

NamedSchemaComponent::Ptr XsdAttribute::Scope::parent() const
{
   return NamedSchemaComponent::Ptr(m_parent);
}

void XsdAttribute::ValueConstraint::setVariety(Variety variety)
{
   m_variety = variety;
}

XsdAttribute::ValueConstraint::Variety XsdAttribute::ValueConstraint::variety() const
{
   return m_variety;
}

void XsdAttribute::ValueConstraint::setValue(const QString &value)
{
   m_value = value;
}

QString XsdAttribute::ValueConstraint::value() const
{
   return m_value;
}

void XsdAttribute::ValueConstraint::setLexicalForm(const QString &form)
{
   m_lexicalForm = form;
}

QString XsdAttribute::ValueConstraint::lexicalForm() const
{
   return m_lexicalForm;
}

void XsdAttribute::setType(const AnySimpleType::Ptr &type)
{
   m_type = type;
}

AnySimpleType::Ptr XsdAttribute::type() const
{
   return m_type;
}

void XsdAttribute::setScope(const Scope::Ptr &scope)
{
   m_scope = scope;
}

XsdAttribute::Scope::Ptr XsdAttribute::scope() const
{
   return m_scope;
}

void XsdAttribute::setValueConstraint(const ValueConstraint::Ptr &constraint)
{
   m_valueConstraint = constraint;
}

XsdAttribute::ValueConstraint::Ptr XsdAttribute::valueConstraint() const
{
   return m_valueConstraint;
}
