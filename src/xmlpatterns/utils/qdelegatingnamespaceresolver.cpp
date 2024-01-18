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

#include <qdelegatingnamespaceresolver_p.h>

#include <qnamepool_p.h>

using namespace QPatternist;

DelegatingNamespaceResolver::DelegatingNamespaceResolver(const NamespaceResolver::Ptr &resolver) : m_nsResolver(
      resolver)
{
   Q_ASSERT(m_nsResolver);
}

DelegatingNamespaceResolver::DelegatingNamespaceResolver(const NamespaceResolver::Ptr &ns,
      const Bindings &overrides) : m_nsResolver(ns)
   , m_bindings(overrides)
{
   Q_ASSERT(m_nsResolver);
}

QXmlName::NamespaceCode DelegatingNamespaceResolver::lookupNamespaceURI(const QXmlName::PrefixCode prefix) const
{
   const QXmlName::NamespaceCode val(m_bindings.value(prefix, NoBinding));

   if (val == NoBinding) {
      return m_nsResolver->lookupNamespaceURI(prefix);
   } else {
      return val;
   }
}

NamespaceResolver::Bindings DelegatingNamespaceResolver::bindings() const
{
   Bindings bs(m_nsResolver->bindings());
   const Bindings::const_iterator end(m_bindings.constEnd());
   Bindings::const_iterator it(m_bindings.constBegin());

   for (; it != end; ++it) {
      bs.insert(it.key(), it.value());
   }

   return bs;
}

void DelegatingNamespaceResolver::addBinding(const QXmlName nb)
{
   if (nb.namespaceURI() == StandardNamespaces::UndeclarePrefix) {
      m_bindings.remove(nb.prefix());
   } else {
      m_bindings.insert(nb.prefix(), nb.namespaceURI());
   }
}
