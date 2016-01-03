/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QMultiItemType_P_H
#define QMultiItemType_P_H

#include <QList>
#include <qitemtype_p.h>

QT_BEGIN_NAMESPACE

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
   virtual QString displayName(const NamePool::Ptr &np) const;

   /**
    * If any of the types this MultiItemType represents matches @p item, it is
    * considered a match.
    *
    * @returns @c true if any of the housed ItemType instances matches @p item, otherwise @c false
    */
   virtual bool itemMatches(const Item &item) const;

   /**
    * If any of the types this MultiItemType represents matches @p other, it is
    * considered a match.
    *
    * @returns @c true if any of the housed ItemType instances matches @p other, otherwise @c false
    */
   virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

   /**
    * @returns @c true if any of the represented types is a node type.
    */
   virtual bool isNodeType() const;

   /**
    * @returns @c true if any of the represented types is an atomic type.
    */
   virtual bool isAtomicType() const;

   /**
    * Determines the union type of all the represented types super types. For example,
    * if the represented types are <tt>xs:integer</tt>, <tt>document()</tt>
    * and <tt>xs:string</tt>, <tt>item()</tt> is returned.
    */
   virtual ItemType::Ptr xdtSuperType() const;

   /**
    * Determines the union type of all the represented types atomized types. For example,
    * if the represented types are <tt>xs:integer</tt> and <tt>document()</tt>,
    * <tt>xs:anyAtomicType</tt> is returned, because that's the super type of <tt>xs:integer</tt>
    * and <tt>xs:untypedAtomic</tt>.
    */
   virtual ItemType::Ptr atomizedType() const;

 private:
   const ItemType::List m_types;
   const ItemType::List::const_iterator m_end;
};
}

QT_END_NAMESPACE

#endif
