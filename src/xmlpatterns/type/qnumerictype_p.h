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

#ifndef QNumericType_P_H
#define QNumericType_P_H

#include <qatomictype_p.h>

namespace QPatternist {
class NumericType : public AtomicType
{
 public:
   virtual ~NumericType();

   bool itemMatches(const Item &item) const override;
   bool xdtTypeMatches(const ItemType::Ptr &other) const override;

   /**
    * @returns always "numeric". That is, no namespace prefix
    */
   QString displayName(const NamePool::Ptr &np) const override;

   /**
    * @returns always @c true
    */
   bool isAbstract() const override;

   /**
    * @returns always @c false
    */
   bool isNodeType() const override;

   /**
    * @returns always @c true
    */
   bool isAtomicType() const override;

   /**
    * @returns always xs:anyAtomicType
    */
   SchemaType::Ptr wxsSuperType() const override;

   /**
    * @returns always xs:anyAtomicType
    */
   ItemType::Ptr xdtSuperType() const override;

   /**
    * @returns @c null. It makes no sense to atomize the abstract type @c fs:numeric.
    */
   ItemType::Ptr atomizedType() const override;

   /**
    * NumericType cannot be visited. This function is only implemented
    * to satisfy the abstract super class's interface.
    *
    * @returns always a @c null pointer
    */
   AtomicTypeVisitorResult::Ptr accept(const AtomicTypeVisitor::Ptr &visitor,
         const SourceLocationReflection *const) const override;

   /**
    * NumericType cannot be visited. This function is only implemented
    * to satisfy the abstract super class's interface.
    *
    * @returns always a @c null pointer
    */
   AtomicTypeVisitorResult::Ptr accept(const ParameterizedAtomicTypeVisitor::Ptr &visitor,
                  const qint16 op, const SourceLocationReflection *const) const override;

   /**
    * The type @c fs:numeric is an abstract type which therefore
    * cannot be involved in comparisons. Hence, this function returns
    * @c null. This function is only implemented to satisfy the abstract
    * super class's interface.
    *
    * @returns always a @c null pointer
    */
   AtomicComparatorLocator::Ptr comparatorLocator() const override;

   /**
    * The type @c fs:numeric is an abstract type which therefore
    * cannot be involved in arithmetics. Hence, this function returns
    * @c null. This function is only implemented to satisfy the abstract
    * super class's interface.
    *
    * @returns always a @c null pointer
    */
   AtomicMathematicianLocator::Ptr mathematicianLocator() const override;


   /**
    * The type @c fs:numeric is an abstract type which therefore
    * cannot be involved in casting. Hence, this function returns
    * @c null. This function is only implemented to satisfy the abstract
    * super class's interface.
    *
    * @returns always a @c null pointer
    */
   AtomicCasterLocator::Ptr casterLocator() const override;

 protected:
   friend class BuiltinTypes;
   NumericType();
};
}

#endif
