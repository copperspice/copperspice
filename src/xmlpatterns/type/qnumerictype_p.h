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

#ifndef Patternist_NumericType_P_H
#define Patternist_NumericType_P_H

#include "qatomictype_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
class NumericType : public AtomicType
{
 public:
   virtual ~NumericType();

   virtual bool itemMatches(const Item &item) const;
   virtual bool xdtTypeMatches(const ItemType::Ptr &other) const;

   /**
    * @returns always "numeric". That is, no namespace prefix
    */
   virtual QString displayName(const NamePool::Ptr &np) const;

   /**
    * @returns always @c true
    */
   virtual bool isAbstract() const;

   /**
    * @returns always @c false
    */
   virtual bool isNodeType() const;

   /**
    * @returns always @c true
    */
   virtual bool isAtomicType() const;

   /**
    * @returns always xs:anyAtomicType
    */
   virtual SchemaType::Ptr wxsSuperType() const;

   /**
    * @returns always xs:anyAtomicType
    */
   virtual ItemType::Ptr xdtSuperType() const;

   /**
    * @returns @c null. It makes no sense to atomize the abstract type @c fs:numeric.
    */
   virtual ItemType::Ptr atomizedType() const;

   /**
    * NumericType cannot be visited. This function is only implemented
    * to satisfy the abstract super class's interface.
    *
    * @returns always a @c null pointer
    */
   virtual AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const) const;

   /**
    * NumericType cannot be visited. This function is only implemented
    * to satisfy the abstract super class's interface.
    *
    * @returns always a @c null pointer
    */
   virtual AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
         const qint16 op,
         const SourceLocationReflection *const) const;

   /**
    * The type @c fs:numeric is an abstract type which therefore
    * cannot be involved in comparisons. Hence, this function returns
    * @c null. This function is only implemented to satisfy the abstract
    * super class's interface.
    *
    * @returns always a @c null pointer
    */
   virtual AtomicComparatorLocator::Ptr comparatorLocator() const;

   /**
    * The type @c fs:numeric is an abstract type which therefore
    * cannot be involved in arithmetics. Hence, this function returns
    * @c null. This function is only implemented to satisfy the abstract
    * super class's interface.
    *
    * @returns always a @c null pointer
    */
   virtual AtomicMathematicianLocator::Ptr mathematicianLocator() const;


   /**
    * The type @c fs:numeric is an abstract type which therefore
    * cannot be involved in casting. Hence, this function returns
    * @c null. This function is only implemented to satisfy the abstract
    * super class's interface.
    *
    * @returns always a @c null pointer
    */
   virtual AtomicCasterLocator::Ptr casterLocator() const;

 protected:
   friend class BuiltinTypes;
   NumericType();
};
}

QT_END_NAMESPACE

#endif
