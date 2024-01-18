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

#ifndef QMultiItemType_P_H
#define QMultiItemType_P_H

#include <qlist.h>

#include <qitemtype_p.h>

namespace QPatternist {

class MultiItemType : public ItemType
{
 public:
   /**
    * Creates a MultiItemType representing the types in @p typeList. @p typeList must
    * contain two or more types.
    */
   MultiItemType(const ItemType::List &typeList);

   /**
    * The display name are the names concatenated with "|" as separator. For example,
    * if this MultiItemType represents the types <tt>document()</tt>, <tt>xs:integer</tt>,
    * and <tt>xs:anyAtomicType</tt>, the display name is
    * "document() | xs:integer | xs:anyAtomicType".
    */
   QString displayName(const NamePool::Ptr &np) const override;

   /**
    * If any of the types this MultiItemType represents matches @p item, it is
    * considered a match.
    *
    * @returns @c true if any of the housed ItemType instances matches @p item, otherwise @c false
    */
   bool itemMatches(const Item &item) const override;

   /**
    * If any of the types this MultiItemType represents matches @p other, it is
    * considered a match.
    *
    * @returns @c true if any of the housed ItemType instances matches @p other, otherwise @c false
    */
   bool xdtTypeMatches(const ItemType::Ptr &other) const override;

   /**
    * @returns @c true if any of the represented types is a node type.
    */
   bool isNodeType() const override;

   /**
    * @returns @c true if any of the represented types is an atomic type.
    */
   bool isAtomicType() const override;

   /**
    * Determines the union type of all the represented types super types. For example,
    * if the represented types are <tt>xs:integer</tt>, <tt>document()</tt>
    * and <tt>xs:string</tt>, <tt>item()</tt> is returned.
    */
   ItemType::Ptr xdtSuperType() const override;

   /**
    * Determines the union type of all the represented types atomized types. For example,
    * if the represented types are <tt>xs:integer</tt> and <tt>document()</tt>,
    * <tt>xs:anyAtomicType</tt> is returned, because that's the super type of <tt>xs:integer</tt>
    * and <tt>xs:untypedAtomic</tt>.
    */
   ItemType::Ptr atomizedType() const override;

 private:
   const ItemType::List m_types;
   const ItemType::List::const_iterator m_end;
};

}

#endif
