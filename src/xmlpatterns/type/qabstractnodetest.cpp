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

#include "qabstractnodetest_p.h"

using namespace QPatternist;

AbstractNodeTest::AbstractNodeTest(const ItemType::Ptr &primaryType) : m_primaryType(primaryType)
{
   Q_ASSERT(m_primaryType);
}

bool AbstractNodeTest::xdtTypeMatches(const ItemType::Ptr &other) const
{
   Q_ASSERT(other);

   if (other->isNodeType()) {
      if (*other == *this) {
         return true;
      } else {
         return xdtTypeMatches(other->xdtSuperType());
      }
   } else {
      return false;
   }
}

ItemType::Ptr AbstractNodeTest::atomizedType() const
{
   return m_primaryType->atomizedType();
}

ItemType::Ptr AbstractNodeTest::xdtSuperType() const
{
   return m_primaryType;
}
