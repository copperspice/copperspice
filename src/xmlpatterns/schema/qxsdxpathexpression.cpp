/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qxsdxpathexpression_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

void XsdXPathExpression::setNamespaceBindings(const QList<QXmlName> &set)
{
   m_namespaceBindings = set;
}

QList<QXmlName> XsdXPathExpression::namespaceBindings() const
{
   return m_namespaceBindings;
}

void XsdXPathExpression::setDefaultNamespace(const AnyURI::Ptr &defaultNs)
{
   m_defaultNamespace = defaultNs;
}

AnyURI::Ptr XsdXPathExpression::defaultNamespace() const
{
   return m_defaultNamespace;
}

void XsdXPathExpression::setBaseURI(const AnyURI::Ptr &uri)
{
   m_baseURI = uri;
}

AnyURI::Ptr XsdXPathExpression::baseURI() const
{
   return m_baseURI;
}

void XsdXPathExpression::setExpression(const QString &expression)
{
   m_expression = expression;
}

QString XsdXPathExpression::expression() const
{
   return m_expression;
}

QT_END_NAMESPACE
