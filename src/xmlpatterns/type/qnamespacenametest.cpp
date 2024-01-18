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

#include <qhash.h>

#include "qbuiltintypes_p.h"
#include "qitem_p.h"
#include "qitem_p.h"

#include "qnamespacenametest_p.h"

using namespace QPatternist;

NamespaceNameTest::NamespaceNameTest(const ItemType::Ptr &primaryType,
                                     const QXmlName::NamespaceCode namespaceURI) : AbstractNodeTest(primaryType),
   m_namespaceURI(namespaceURI)
{
}

ItemType::Ptr NamespaceNameTest::create(const ItemType::Ptr &primaryType, const QXmlName::NamespaceCode namespaceURI)
{
   Q_ASSERT(primaryType);

   return ItemType::Ptr(new NamespaceNameTest(primaryType, namespaceURI));
}

bool NamespaceNameTest::itemMatches(const Item &item) const
{
   Q_ASSERT(item.isNode());
   return m_primaryType->itemMatches(item) &&
          item.asNode().name().namespaceURI() == m_namespaceURI;
}

QString NamespaceNameTest::displayName(const NamePool::Ptr &np) const
{
   return QLatin1Char('{') + np->stringForNamespace(m_namespaceURI) + QLatin1String("}:*");
}

ItemType::InstanceOf NamespaceNameTest::instanceOf() const
{
   return ClassNamespaceNameTest;
}

bool NamespaceNameTest::operator==(const ItemType &other) const
{
   return other.instanceOf() == ClassNamespaceNameTest &&
          static_cast<const NamespaceNameTest &>(other).m_namespaceURI == m_namespaceURI;
}

PatternPriority NamespaceNameTest::patternPriority() const
{
   return -0.25;
}
