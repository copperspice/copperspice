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
#include "qqnametest_p.h"

using namespace QPatternist;

QNameTest::QNameTest(const ItemType::Ptr &primaryType,
                     const QXmlName qName) : AbstractNodeTest(primaryType)
   , m_qName(qName)
{
   Q_ASSERT(!qName.isNull());
}

ItemType::Ptr QNameTest::create(const ItemType::Ptr &primaryType, const QXmlName qName)
{
   Q_ASSERT(!qName.isNull());
   Q_ASSERT(primaryType);

   return ItemType::Ptr(new QNameTest(primaryType, qName));
}

bool QNameTest::itemMatches(const Item &item) const
{
   Q_ASSERT(item.isNode());
   return m_primaryType->itemMatches(item) &&
          item.asNode().name() == m_qName;
}

QString QNameTest::displayName(const NamePool::Ptr &np) const
{
   QString displayOther(m_primaryType->displayName(np));

   return displayOther.insert(displayOther.size() - 1, np->displayName(m_qName));
}

ItemType::InstanceOf QNameTest::instanceOf() const
{
   return ClassQNameTest;
}

bool QNameTest::operator==(const ItemType &other) const
{
   return other.instanceOf() == ClassQNameTest &&
          static_cast<const QNameTest &>(other).m_qName == m_qName;
}

PatternPriority QNameTest::patternPriority() const
{
   return 0;
}
