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

#ifndef QItemType_P_H
#define QItemType_P_H

#include <qshareddata.h>
#include <qcontainerfwd.h>

#include <qnamepool_p.h>

namespace QPatternist {

class Item;

class ItemType : public virtual QSharedData
{
 public:
   using Ptr  = QExplicitlySharedDataPointer<ItemType>;
   using List = QList<ItemType::Ptr>;

   virtual ~ItemType();

   enum Category {
      NodeNameTest = 1,
      Other        = 2
   };

   virtual bool operator==(const ItemType &other) const;

   inline bool operator!=(const ItemType &other) const;

   virtual QString displayName(const NamePool::Ptr &np) const = 0;

   virtual bool itemMatches(const Item &item) const = 0;

   virtual bool xdtTypeMatches(const ItemType::Ptr &other) const = 0;

   virtual bool isNodeType() const = 0;
   virtual bool isAtomicType() const = 0;

   virtual ItemType::Ptr xdtSuperType() const = 0;

   virtual const ItemType &operator|(const ItemType &other) const;

   virtual ItemType::Ptr atomizedType() const = 0;

   virtual Category itemTypeCategory() const;

   enum InstanceOf {
      ClassLocalNameTest,
      ClassNamespaceNameTest,
      ClassQNameTest,
      ClassOther
   };

   virtual InstanceOf instanceOf() const;

   ItemType() {
   }

 private:
   ItemType(const ItemType &) = delete;
   ItemType &operator=(const ItemType &) = delete;
};

inline ItemType::Ptr operator|(const ItemType::Ptr &op1, const ItemType::Ptr &op2)
{
   return ItemType::Ptr(const_cast<ItemType *>(&(*op1 | *op2)));
}

bool ItemType::operator!=(const ItemType &other) const
{
   return this != &other;
}

inline void operator|=(ItemType::Ptr &op1, const ItemType::Ptr &op2)
{
   op1 = op1 | op2;
}

}

#endif
