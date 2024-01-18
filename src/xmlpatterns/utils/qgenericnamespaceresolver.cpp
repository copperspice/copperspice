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

#include <qgenericnamespaceresolver_p.h>

#include <qnamepool_p.h>

using namespace QPatternist;

GenericNamespaceResolver::GenericNamespaceResolver(const Bindings &list) : m_bindings(list)
{
}

void GenericNamespaceResolver::addBinding(const QXmlName nb)
{
   if (nb.namespaceURI() == StandardNamespaces::UndeclarePrefix) {
      m_bindings.remove(nb.prefix());
   } else {
      m_bindings.insert(nb.prefix(), nb.namespaceURI());
   }
}

QXmlName::NamespaceCode GenericNamespaceResolver::lookupNamespaceURI(const QXmlName::PrefixCode prefix) const
{
   return m_bindings.value(prefix, NoBinding);
}

NamespaceResolver::Ptr GenericNamespaceResolver::defaultXQueryBindings()
{
   Bindings list;

   list.insert(StandardPrefixes::xml,    StandardNamespaces::xml);
   list.insert(StandardPrefixes::xs,     StandardNamespaces::xs);
   list.insert(StandardPrefixes::xsi,    StandardNamespaces::xsi);
   list.insert(StandardPrefixes::fn,     StandardNamespaces::fn);
   list.insert(StandardPrefixes::local,  StandardNamespaces::local);
   list.insert(StandardPrefixes::empty,  StandardNamespaces::empty);

   return NamespaceResolver::Ptr(new GenericNamespaceResolver(list));
}

NamespaceResolver::Ptr GenericNamespaceResolver::defaultXSLTBindings()
{
   Bindings list;

   list.insert(StandardPrefixes::xml,    StandardNamespaces::xml);
   list.insert(StandardPrefixes::empty,  StandardNamespaces::empty);

   return NamespaceResolver::Ptr(new GenericNamespaceResolver(list));
}

NamespaceResolver::Bindings GenericNamespaceResolver::bindings() const
{
   return m_bindings;
}
