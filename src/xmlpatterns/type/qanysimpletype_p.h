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

#ifndef QAnySimpleType_P_H
#define QAnySimpleType_P_H

#include <qanytype_p.h>

namespace QPatternist {

class AtomicType;

class AnySimpleType : public AnyType
{
 public:
   typedef QExplicitlySharedDataPointer<AnySimpleType> Ptr;
   typedef QList<AnySimpleType::Ptr> List;
   friend class BuiltinTypes;

   virtual ~AnySimpleType();

   QXmlName name(const NamePool::Ptr &np) const override;

   /**
    * @returns always @c xs:anySimpleType
    */
   QString displayName(const NamePool::Ptr &np) const override;

   /**
    * @returns always BuiltinTypes::xsAnyType
    */
   SchemaType::Ptr wxsSuperType() const override;

   /**
    * xs:anySimpleType is the special "simple ur-type". Therefore this function
    * returns SchemaType::None
    *
    * @returns SchemaType::None
    */
   TypeCategory category() const override;

   /**
    * The simple ur-type is a "special restriction of the ur-type definition",
    * according to XML Schema Part 2: Datatypes Second Edition about xs:anySimpleType
    *
    * @returns DERIVATION_RESTRICTION
    */
   SchemaType::DerivationMethod derivationMethod() const override;

   /**
    * Always returns @c true.
    */
   bool isSimpleType() const override;

   /**
    * Always returns @c false.
    */
   bool isComplexType() const override;

 protected:
   AnySimpleType();

};

}

#endif
