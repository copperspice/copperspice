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

#include "qxsdattributeuse_p.h"

using namespace QPatternist;

void XsdAttributeUse::ValueConstraint::setVariety(Variety variety)
{
   m_variety = variety;
}

XsdAttributeUse::ValueConstraint::Variety XsdAttributeUse::ValueConstraint::variety() const
{
   return m_variety;
}

void XsdAttributeUse::ValueConstraint::setValue(const QString &value)
{
   m_value = value;
}

QString XsdAttributeUse::ValueConstraint::value() const
{
   return m_value;
}

void XsdAttributeUse::ValueConstraint::setLexicalForm(const QString &form)
{
   m_lexicalForm = form;
}

QString XsdAttributeUse::ValueConstraint::lexicalForm() const
{
   return m_lexicalForm;
}

XsdAttributeUse::ValueConstraint::Ptr XsdAttributeUse::ValueConstraint::fromAttributeValueConstraint(
   const XsdAttribute::ValueConstraint::Ptr &constraint)
{
   XsdAttributeUse::ValueConstraint::Ptr newConstraint(new XsdAttributeUse::ValueConstraint());
   switch (constraint->variety()) {
      case XsdAttribute::ValueConstraint::Fixed:
         newConstraint->setVariety(Fixed);
         break;
      case XsdAttribute::ValueConstraint::Default:
         newConstraint->setVariety(Default);
         break;
   }
   newConstraint->setValue(constraint->value());
   newConstraint->setLexicalForm(constraint->lexicalForm());

   return newConstraint;
}

XsdAttributeUse::XsdAttributeUse()
   : m_useType(OptionalUse)
{
}

bool XsdAttributeUse::isAttributeUse() const
{
   return true;
}

void XsdAttributeUse::setUseType(UseType type)
{
   m_useType = type;
}

XsdAttributeUse::UseType XsdAttributeUse::useType() const
{
   return m_useType;
}

bool XsdAttributeUse::isRequired() const
{
   return (m_useType == RequiredUse);
}

void XsdAttributeUse::setAttribute(const XsdAttribute::Ptr &attribute)
{
   m_attribute = attribute;
}

XsdAttribute::Ptr XsdAttributeUse::attribute() const
{
   return m_attribute;
}

void XsdAttributeUse::setValueConstraint(const ValueConstraint::Ptr &constraint)
{
   m_valueConstraint = constraint;
}

XsdAttributeUse::ValueConstraint::Ptr XsdAttributeUse::valueConstraint() const
{
   return m_valueConstraint;
}

