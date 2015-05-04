/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qitem_p.h"
#include "qnamepool_p.h"

#include "qnodenamespaceresolver_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

NodeNamespaceResolver::NodeNamespaceResolver(const Item &item) : m_node(item.asNode())
{
   Q_ASSERT(!m_node.isNull());
}

void NodeNamespaceResolver::addBinding(const QXmlName nb)
{
   Q_UNUSED(nb);
   Q_ASSERT_X(false, Q_FUNC_INFO, "Calling this function for this sub-class makes little sense.");
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
   Q_ASSERT_X(false, Q_FUNC_INFO, "Not implemented.");
   return NamespaceResolver::Bindings();
}

QT_END_NAMESPACE
