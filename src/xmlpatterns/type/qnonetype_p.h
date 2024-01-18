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

#ifndef QNoneType_P_H
#define QNoneType_P_H

#include <qatomictype_p.h>
#include <qsequencetype_p.h>

namespace QPatternist {

class NoneType : public ItemType, public SequenceType
{
 public:
   typedef QExplicitlySharedDataPointer<NoneType> Ptr;

   bool itemMatches(const Item &item) const override;
   bool xdtTypeMatches(const ItemType::Ptr &other) const override;

   /**
    * @returns always "none". That is, no namespace prefix
    */
   QString displayName(const NamePool::Ptr &np) const override;

   /**
    * @note The semantical meaning of this type's item type can
    * surely be discussed. The function is provided due to
    * it being mandated by the SequenceType base class.
    *
    * @returns always 'this' since NoneType is also an ItemType
    */
   ItemType::Ptr itemType() const override;

   /**
    * @note The semantical meaning of this type's cardinality
    * can surely be discussed. The function is provided due to
    * it being mandated by the SequenceType base class.
    *
    * @returns always Cardinality::zeroOrMore()
    */
   Cardinality cardinality() const override;

   /**
    * @returns always @c false
    */
   bool isAtomicType() const override;

   /**
    * This can be thought to be a weird function for this type(none). There
    * is no atomized type for none, perhaps the best from a conceptual perspective
    * would be to return @c null.
    *
    * This function returns BuiltinTypes::xsAnyAtomicType because
    * the generic type checking code inserts an Atomizer in the AST
    * when an error() function(or other node which has type none) is part of
    * an operator expression(value/general comparison, arithmetics). The Atomizer
    * returns the atomizedType() of its child, and by here returning xsAnyAtomicType,
    * static operator lookup is postponed to runtime. Subsequently, expressions like error()
    * works properly with other XPath expressions.
    */
   ItemType::Ptr atomizedType() const override;

   /**
    * @returns always @c false
    */
   bool isNodeType() const override;

   /**
    * @returns always item()
    */
   ItemType::Ptr xdtSuperType() const override;

   /**
    * @returns always @p other. The none type can be thought as
    * disappearing when attempting to find the union of it and
    * another type.
    */
   const ItemType &operator|(const ItemType &other) const override;

 protected:

   friend class CommonSequenceTypes;
   NoneType();
};
}

#endif
