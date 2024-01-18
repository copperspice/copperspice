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

#include "qnamespacesupport_p.h"

using namespace QPatternist;

NamespaceSupport::NamespaceSupport()
{
}

NamespaceSupport::NamespaceSupport(NamePool &namePool)
   : m_namePool(&namePool)
{
   // the XML namespace
   m_ns.insert(StandardPrefixes::xml, StandardNamespaces::xml);
}

void NamespaceSupport::setPrefix(const QXmlName::PrefixCode prefixCode, const QXmlName::NamespaceCode namespaceCode)
{
   m_ns.insert(prefixCode, namespaceCode);
}

void NamespaceSupport::setPrefixes(const QXmlStreamNamespaceDeclarations &declarations)
{
   for (int i = 0; i < declarations.count(); i++) {
      const QXmlStreamNamespaceDeclaration declaration = declarations.at(i);

      const QXmlName::PrefixCode prefixCode = m_namePool->allocatePrefix(declaration.prefix().toString());
      const QXmlName::NamespaceCode namespaceCode = m_namePool->allocateNamespace(declaration.namespaceUri().toString());
      m_ns.insert(prefixCode, namespaceCode);
   }
}

void NamespaceSupport::setTargetNamespace(const QXmlName::NamespaceCode namespaceCode)
{
   m_ns.insert(0, namespaceCode);
}

QXmlName::PrefixCode NamespaceSupport::prefix(const QXmlName::NamespaceCode namespaceCode) const
{
   NamespaceHash::const_iterator itc, it = m_ns.constBegin();
   while ((itc = it) != m_ns.constEnd()) {
      ++it;
      if (*itc == namespaceCode) {
         return itc.key();
      }
   }
   return 0;
}

QXmlName::NamespaceCode NamespaceSupport::uri(const QXmlName::PrefixCode prefixCode) const
{
   return m_ns.value(prefixCode);
}

bool NamespaceSupport::processName(const QString &qname, NameType type, QXmlName &name) const
{
   int len = qname.size();

   for (int pos = 0; pos < len; ++pos) {

      if (qname[pos] == ':') {
         const QXmlName::PrefixCode prefixCode = m_namePool->allocatePrefix(qname.left(pos));

         if (! m_ns.contains(prefixCode)) {
            return false;
         }

         const QXmlName::NamespaceCode  namespaceCode = uri(prefixCode);
         const QXmlName::LocalNameCode  localNameCode = m_namePool->allocateLocalName(qname.mid(pos + 1));
         name = QXmlName(namespaceCode, localNameCode, prefixCode);

         return true;
      }
   }

   // there was no ':'
   QXmlName::NamespaceCode namespaceCode = 0;

   // attributes don't take default namespace
   if (type == ElementName && !m_ns.isEmpty()) {
      namespaceCode = m_ns.value(0);   // get default namespace
   }

   const QXmlName::LocalNameCode localNameCode = m_namePool->allocateLocalName(qname);
   name = QXmlName(namespaceCode, localNameCode, 0);

   return true;
}

void NamespaceSupport::pushContext()
{
   m_nsStack.push(m_ns);
}

void NamespaceSupport::popContext()
{
   m_ns.clear();
   if (!m_nsStack.isEmpty()) {
      m_ns = m_nsStack.pop();
   }
}

QList<QXmlName> NamespaceSupport::namespaceBindings() const
{
   QList<QXmlName> bindings;

   QHashIterator<QXmlName::PrefixCode, QXmlName::NamespaceCode> it(m_ns);
   while (it.hasNext()) {
      it.next();
      bindings.append(QXmlName(it.value(), StandardLocalNames::empty, it.key()));
   }

   return bindings;
}
