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

#ifndef QAtomicType_P_H
#define QAtomicType_P_H

#include <qanysimpletype_p.h>
#include <qatomiccasterlocator_p.h>
#include <qatomiccomparatorlocator_p.h>
#include <qatomicmathematicianlocator_p.h>
#include <qatomictypedispatch_p.h>
#include <qitemtype_p.h>

namespace QPatternist {

class Item;
class SourceLocationReflection;

class AtomicType : public ItemType, public AnySimpleType
{
 public:

   typedef QExplicitlySharedDataPointer<AtomicType> Ptr;

   virtual ~AtomicType();

   /**
    * Implements a generic algorithm which relies on wxsTypeMatches().
    *
    * @returns @c true depending on if @p item is an atomic type, and that
    * AtomicValue::itemType()'s SequenceType::itemType() matches this type.
    */
   bool itemMatches(const Item &item) const override;

   /**
    * @returns the result of SharedQXmlName::displayName(), of the SharedQName
    * object returned from the name() function.
    */
   QString displayName(const NamePool::Ptr &np) const override;

   /**
    * returns always @c false
    */
   bool isNodeType() const override;

   /**
    * returns always @c true
    */
   bool isAtomicType() const override;

   /**
    * Determines whether @p other is equal to this type, or is a
    * sub-type of this type.
    *
    * The implementation is generic, relying on operator==()
    * and xdtSuperType().
    */
   bool xdtTypeMatches(const ItemType::Ptr &other) const override;

   /**
    * @returns always 'this'
    */
   ItemType::Ptr atomizedType() const override;

   /**
    * @returns always SchemaType::SimpleTypeAtomic
    */
   TypeCategory category() const override;

   /**
    * @returns DerivationRestriction
    */
   DerivationMethod derivationMethod() const override;

   virtual AtomicTypeVisitorResult::Ptr
   accept(const QExplicitlySharedDataPointer<AtomicTypeVisitor> &visitor,
                  const SourceLocationReflection *const) const = 0;

   virtual AtomicTypeVisitorResult::Ptr
   accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor, const qint16 param,
                  const SourceLocationReflection *const) const = 0;

   virtual AtomicComparatorLocator::Ptr comparatorLocator() const = 0;
   virtual AtomicMathematicianLocator::Ptr mathematicianLocator() const = 0;
   virtual AtomicCasterLocator::Ptr casterLocator() const = 0;

 protected:
   AtomicType();

};

}

#endif
