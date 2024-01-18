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

#ifndef QAnyItemType_P_H
#define QAnyItemType_P_H

#include <qatomictype_p.h>

namespace QPatternist {

class AnyItemType : public ItemType
{
 public:
   /**
    * @returns always "item()"
    */
   QString displayName(const NamePool::Ptr &np) const override;

   /**
    * @returns always @c true
    */
   bool itemMatches(const Item &item) const override;

   /**
    * @returns always 0, item() is the super type in the
    * XPath Data Model hierarchy
    */
   ItemType::Ptr xdtSuperType() const override;

   /**
    * @returns always @c false
    */
   bool isNodeType() const override;

   /**
    * @returns always @c false
    */
   bool isAtomicType() const override;

   /**
    * @returns always @c true
    */
   bool xdtTypeMatches(const ItemType::Ptr &type) const override;

   /**
    * @returns xs:anyAtomicType
    */
   ItemType::Ptr atomizedType() const override;

 protected:
   friend class BuiltinTypes;
   AnyItemType();
};

}

#endif
