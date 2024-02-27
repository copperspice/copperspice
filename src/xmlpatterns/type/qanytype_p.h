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

#ifndef QAnyType_P_H
#define QAnyType_P_H

#include <qschematype_p.h>

namespace QPatternist {

class AtomicType;

class AnyType : public SchemaType
{
 public:

   typedef QExplicitlySharedDataPointer<AnyType> Ptr;
   friend class BuiltinTypes;

   virtual ~AnyType();

   QXmlName name(const NamePool::Ptr &np) const override;

   /**
    * @returns always "xs:anyType"
    */
   QString displayName(const NamePool::Ptr &np) const override;

   /**
    * @returns always @c false
    */
   bool isAbstract() const override;

   /**
    * @returns @c null, since <tt>xs:anyType</tt> has no base type, it is the ur-type.
    *
    * @returns always @c null
    */
   SchemaType::Ptr wxsSuperType() const override;

   /**
    * @returns @c true only if @p other is xsAnyType.
    */
   bool wxsTypeMatches(const SchemaType::Ptr &other) const override;

   /**
    * <tt>xs:anyType</tt> is the "ur-type" and special. Therefore, this function
    * returns SchemaType::None.
    *
    * @returns SchemaType::None
    */
   TypeCategory category() const override;

   /**
    * @returns always NoDerivation.
    */
   DerivationMethod derivationMethod() const override;

   /**
    * @returns an empty set of derivation constraint flags.
    */
   DerivationConstraints derivationConstraints() const override;

   /**
    * Always returns @c true.
    */
   bool isComplexType() const override;

 protected:
   /**
    * @short This constructor is protected, because this
    * class must be sub-classed.
    */
   inline AnyType() {
   }
};

}

#endif
