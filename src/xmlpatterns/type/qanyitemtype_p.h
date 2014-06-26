/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QAnyItemType_P_H
#define QAnyItemType_P_H

#include <qatomictype_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class AnyItemType : public ItemType
{
 public:
   /**
    * @returns always "item()"
    */
   virtual QString displayName(const NamePool::Ptr &np) const;

   /**
    * @returns always @c true
    */
   virtual bool itemMatches(const Item &item) const;

   /**
    * @returns always 0, item() is the super type in the
    * XPath Data Model hierarchy
    */
   virtual ItemType::Ptr xdtSuperType() const;

   /**
    * @returns always @c false
    */
   virtual bool isNodeType() const;

   /**
    * @returns always @c false
    */
   virtual bool isAtomicType() const;

   /**
    * @returns always @c true
    */
   virtual bool xdtTypeMatches(const ItemType::Ptr &type) const;

   /**
    * @returns xs:anyAtomicType
    */
   virtual ItemType::Ptr atomizedType() const;

 protected:
   friend class BuiltinTypes;
   AnyItemType();
};
}

QT_END_NAMESPACE

#endif
