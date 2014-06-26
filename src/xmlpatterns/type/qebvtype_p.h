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

#ifndef QEBVType_P_H
#define QEBVType_P_H

#include <qatomictype_p.h>
#include <qsequencetype_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class EBVType : public ItemType, public SequenceType
{
 public:
   typedef QExplicitlySharedDataPointer<EBVType> Ptr;

   /**
    * @todo docs if it's an ebvable type, etc.
    */
   virtual bool itemMatches(const Item &item) const;
   virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

   virtual QString displayName(const NamePool::Ptr &np) const;

   /**
    * @note The semantical meaning of this type's item type can
    * surely be discussed. The function is provided due to
    * it being mandated by the SequenceType base class.
    *
    * @returns always 'this' since EBVType is also an ItemType
    */
   virtual ItemType::Ptr itemType() const;

   /**
    * @note The semantical meaning of this type's cardinality
    * can surely be discussed. The function is provided due to
    * it being mandated by the SequenceType base class.
    *
    * @returns always Cardinality::zeroOrMore()
    */
   virtual Cardinality cardinality() const;

   virtual bool isAtomicType() const;

   /**
    * @returns always @c null
    */
   virtual ItemType::Ptr atomizedType() const;

   /**
    * @returns always BuiltinTypes::item
    */
   virtual ItemType::Ptr xdtSuperType() const;

   /**
    * @returns always @c false
    */
   virtual bool isNodeType() const;

 protected:
   friend class CommonSequenceTypes;
   EBVType();
};
}

QT_END_NAMESPACE

#endif
