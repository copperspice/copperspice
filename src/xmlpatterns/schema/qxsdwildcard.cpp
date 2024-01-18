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

#include "qxsdwildcard_p.h"

using namespace QPatternist;

QString XsdWildcard::absentNamespace()
{
   return QLatin1String("__ns_absent");
}

void XsdWildcard::NamespaceConstraint::setVariety(Variety variety)
{
   m_variety = variety;
}

XsdWildcard::NamespaceConstraint::Variety XsdWildcard::NamespaceConstraint::variety() const
{
   return m_variety;
}

void XsdWildcard::NamespaceConstraint::setNamespaces(const QSet<QString> &namespaces)
{
   m_namespaces = namespaces;
}

QSet<QString> XsdWildcard::NamespaceConstraint::namespaces() const
{
   return m_namespaces;
}

void XsdWildcard::NamespaceConstraint::setDisallowedNames(const QSet<QString> &names)
{
   m_disallowedNames = names;
}

QSet<QString> XsdWildcard::NamespaceConstraint::disallowedNames() const
{
   return m_disallowedNames;
}

XsdWildcard::XsdWildcard()
   : m_namespaceConstraint(new NamespaceConstraint())
   , m_processContents(Strict)
{
   m_namespaceConstraint->setVariety(NamespaceConstraint::Any);
}

bool XsdWildcard::isWildcard() const
{
   return true;
}

void XsdWildcard::setNamespaceConstraint(const NamespaceConstraint::Ptr &namespaceConstraint)
{
   m_namespaceConstraint = namespaceConstraint;
}

XsdWildcard::NamespaceConstraint::Ptr XsdWildcard::namespaceConstraint() const
{
   return m_namespaceConstraint;
}

void XsdWildcard::setProcessContents(ProcessContents contents)
{
   m_processContents = contents;
}

XsdWildcard::ProcessContents XsdWildcard::processContents() const
{
   return m_processContents;
}
