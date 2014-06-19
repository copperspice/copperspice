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

#ifndef Patternist_AnyType_P_H
#define Patternist_AnyType_P_H

#include "qschematype_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
class AtomicType;

class AnyType : public SchemaType
{
 public:

   typedef QExplicitlySharedDataPointer<AnyType> Ptr;
   friend class BuiltinTypes;

   virtual ~AnyType();

   virtual QXmlName name(const NamePool::Ptr &np) const;

   /**
    * @returns always "xs:anyType"
    */
   virtual QString displayName(const NamePool::Ptr &np) const;

   /**
    * @returns always @c false
    */
   virtual bool isAbstract() const;

   /**
    * @returns @c null, since <tt>xs:anyType</tt> has no base type, it is the ur-type.
    *
    * @returns always @c null
    */
   virtual SchemaType::Ptr wxsSuperType() const;

   /**
    * @returns @c true only if @p other is xsAnyType.
    */
   virtual bool wxsTypeMatches(const SchemaType::Ptr &other) const;

   /**
    * <tt>xs:anyType</tt> is the "ur-type" and special. Therefore, this function
    * returns SchemaType::None.
    *
    * @returns SchemaType::None
    */
   virtual TypeCategory category() const;

   /**
    * @returns always NoDerivation.
    */
   virtual DerivationMethod derivationMethod() const;

   /**
    * @returns an empty set of derivation constraint flags.
    */
   virtual DerivationConstraints derivationConstraints() const;

   /**
    * Always returns @c true.
    */
   virtual bool isComplexType() const;

 protected:
   /**
    * @short This constructor is protected, because this
    * class must be sub-classed.
    */
   inline AnyType() {
   }
};
}

QT_END_NAMESPACE

#endif
