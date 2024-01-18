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

#include <qnodenamespaceresolver_p.h>

#include <qitem_p.h>
#include <qnamepool_p.h>

using namespace QPatternist;

NodeNamespaceResolver::NodeNamespaceResolver(const Item &item) : m_node(item.asNode())
{
   Q_ASSERT(!m_node.isNull());
}

void NodeNamespaceResolver::addBinding(const QXmlName nb)
{
   (void) nb;
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
}

QXmlName::NamespaceCode NodeNamespaceResolver::lookupNamespaceURI(const QXmlName::PrefixCode prefix) const
{
   const QXmlName::NamespaceCode ns = m_node.namespaceForPrefix(prefix);

   if (ns == NoBinding) {
      if (prefix == StandardPrefixes::empty) {
         return StandardNamespaces::empty;
      } else {
         return NoBinding;
      }
   } else {
      return ns;
   }
}

NamespaceResolver::Bindings NodeNamespaceResolver::bindings() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return NamespaceResolver::Bindings();
}
