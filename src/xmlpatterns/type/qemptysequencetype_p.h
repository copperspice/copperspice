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

#ifndef Patternist_EmptySequenceType_P_H
#define Patternist_EmptySequenceType_P_H

#include "qatomictype_p.h"
#include "qsequencetype_p.h"

QT_BEGIN_NAMESPACE

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
   virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

   /**
    * @returns always @c false
    */
   virtual bool itemMatches(const Item &item) const;

   /**
    * @returns always "empty-sequence()"
    */
   virtual QString displayName(const NamePool::Ptr &np) const;

   virtual ItemType::Ptr xdtSuperType() const;

   virtual bool isNodeType() const;
   virtual bool isAtomicType() const;

   /**
    * @return always Cardinality::empty()
    */
   virtual Cardinality cardinality() const;

   /**
    * @returns always 'this' since it is also an ItemType
    */
   virtual ItemType::Ptr itemType() const;

   /**
    * @returns always @c xs:anyAtomicType
    */
   virtual ItemType::Ptr atomizedType() const;

 protected:
   friend class CommonSequenceTypes;
   EmptySequenceType();
};
}

QT_END_NAMESPACE

#endif
