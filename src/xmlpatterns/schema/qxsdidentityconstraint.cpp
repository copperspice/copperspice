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

#include "qxsdidentityconstraint_p.h"

using namespace QPatternist;

void XsdIdentityConstraint::setCategory(Category category)
{
   m_category = category;
}

XsdIdentityConstraint::Category XsdIdentityConstraint::category() const
{
   return m_category;
}

void XsdIdentityConstraint::setSelector(const XsdXPathExpression::Ptr &selector)
{
   m_selector = selector;
}

XsdXPathExpression::Ptr XsdIdentityConstraint::selector() const
{
   return m_selector;
}

void XsdIdentityConstraint::setFields(const XsdXPathExpression::List &fields)
{
   m_fields = fields;
}

void XsdIdentityConstraint::addField(const XsdXPathExpression::Ptr &field)
{
   m_fields.append(field);
}

XsdXPathExpression::List XsdIdentityConstraint::fields() const
{
   return m_fields;
}

void XsdIdentityConstraint::setReferencedKey(const XsdIdentityConstraint::Ptr &referencedKey)
{
   m_referencedKey = referencedKey;
}

XsdIdentityConstraint::Ptr XsdIdentityConstraint::referencedKey() const
{
   return m_referencedKey;
}
