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

#ifndef QEmptySequenceType_P_H
#define QEmptySequenceType_P_H

#include <qatomictype_p.h>
#include <qsequencetype_p.h>

namespace QPatternist {

class EmptySequenceType : public ItemType, public SequenceType
{
 public:
   typedef QExplicitlySharedDataPointer<EmptySequenceType> Ptr;

   /**
    * Possibly surprisingly, this function also returns true for the @c none type.
    *
    * @returns @c true if @p other is NoneType or EmptySequenceType, otherwise @c false.
    */
   bool xdtTypeMatches(const ItemType::Ptr &other) const override;

   /**
    * @returns always @c false
    */
   bool itemMatches(const Item &item) const override;

   /**
    * @returns always "empty-sequence()"
    */
   QString displayName(const NamePool::Ptr &np) const override;

   ItemType::Ptr xdtSuperType() const override;

   bool isNodeType() const override;
   bool isAtomicType() const override;

   /**
    * @return always Cardinality::empty()
    */
   Cardinality cardinality() const override;

   /**
    * @returns always 'this' since it is also an ItemType
    */
   ItemType::Ptr itemType() const override;

   /**
    * @returns always @c xs:anyAtomicType
    */
   ItemType::Ptr atomizedType() const override;

 protected:
   friend class CommonSequenceTypes;
   EmptySequenceType();
};

}

#endif
