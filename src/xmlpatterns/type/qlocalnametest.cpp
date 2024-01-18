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
#include "qxpathhelper_p.h"
#include "qlocalnametest_p.h"

using namespace QPatternist;

LocalNameTest::LocalNameTest(const ItemType::Ptr &primaryType,
                             const QXmlName::LocalNameCode &ncName) : AbstractNodeTest(primaryType),
   m_ncName(ncName)
{
}

ItemType::Ptr LocalNameTest::create(const ItemType::Ptr &primaryType, const QXmlName::LocalNameCode localName)
{
   Q_ASSERT(primaryType);

   return ItemType::Ptr(new LocalNameTest(primaryType, localName));
}

bool LocalNameTest::itemMatches(const Item &item) const
{
   Q_ASSERT(item.isNode());
   return m_primaryType->itemMatches(item) &&
          item.asNode().name().localName() == m_ncName;
}

QString LocalNameTest::displayName(const NamePool::Ptr &np) const
{
   QString displayOther(m_primaryType->displayName(np));

   return displayOther.insert(displayOther.size() - 1,
                              QString::fromLatin1("*:") + np->stringForLocalName(m_ncName));
}

ItemType::InstanceOf LocalNameTest::instanceOf() const
{
   return ClassLocalNameTest;
}

bool LocalNameTest::operator==(const ItemType &other) const
{
   return other.instanceOf() == ClassLocalNameTest &&
          static_cast<const LocalNameTest &>(other).m_ncName == m_ncName;
}

PatternPriority LocalNameTest::patternPriority() const
{
   return -0.25;
}
