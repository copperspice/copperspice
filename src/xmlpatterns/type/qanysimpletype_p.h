/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QAnySimpleType_P_H
#define QAnySimpleType_P_H

#include <qanytype_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class AtomicType;

class AnySimpleType : public AnyType
{
 public:
   typedef QExplicitlySharedDataPointer<AnySimpleType> Ptr;
   typedef QList<AnySimpleType::Ptr> List;
   friend class BuiltinTypes;

   virtual ~AnySimpleType();

   virtual QXmlName name(const NamePool::Ptr &np) const;

   /**
    * @returns always @c xs:anySimpleType
    */
   virtual QString displayName(const NamePool::Ptr &np) const;

   /**
    * @returns always BuiltinTypes::xsAnyType
    */
   virtual SchemaType::Ptr wxsSuperType() const;

   /**
    * xs:anySimpleType is the special "simple ur-type". Therefore this function
    * returns SchemaType::None
    *
    * @returns SchemaType::None
    */
   virtual TypeCategory category() const;

   /**
    * The simple ur-type is a "special restriction of the ur-type definition",
    * according to XML Schema Part 2: Datatypes Second Edition about xs:anySimpleType
    *
    * @returns DERIVATION_RESTRICTION
    */
   virtual SchemaType::DerivationMethod derivationMethod() const;

   /**
    * Always returns @c true.
    */
   virtual bool isSimpleType() const;

   /**
    * Always returns @c false.
    */
   virtual bool isComplexType() const;

 protected:
   AnySimpleType();

};
}

QT_END_NAMESPACE

#endif
