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

#ifndef QQNameTest_P_H
#define QQNameTest_P_H

#include <qabstractnodetest_p.h>

template<typename Key, typename Value>
class QHash;

QT_BEGIN_NAMESPACE

namespace QPatternist {
class QNameTest : public AbstractNodeTest
{
 public:
   typedef QHash<QString, QNameTest::Ptr> Hash;

   static ItemType::Ptr create(const ItemType::Ptr &primaryType, const QXmlName qName);

   /**
    * @note This function assumes that @p item is a QXmlNodeModelIndex.
    */
   virtual bool itemMatches(const Item &item) const;

   virtual QString displayName(const NamePool::Ptr &np) const;

   virtual bool operator==(const ItemType &other) const;

   virtual PatternPriority patternPriority() const;

 protected:
   virtual InstanceOf instanceOf() const;

 private:
   QNameTest(const ItemType::Ptr &primaryType, const QXmlName qName);

   const QXmlName m_qName;
};
}

QT_END_NAMESPACE

#endif
