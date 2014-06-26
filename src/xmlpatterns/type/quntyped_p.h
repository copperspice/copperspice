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

#ifndef QUntyped_P_H
#define QUntyped_P_H

#include <qanytype_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class AtomicType;

class Untyped : public AnyType
{
 public:

   typedef QExplicitlySharedDataPointer<Untyped> Ptr;

   /**
    * @returns always BuiltinTypes::xsAnyType.
    */
   virtual SchemaType::Ptr wxsSuperType() const;

   virtual QXmlName name(const NamePool::Ptr &np) const;

   /**
    * @returns always <tt>xs:untypedAtomic</tt>
    */
   virtual ItemType::Ptr atomizedType() const;

   /**
    * @returns always SchemaType::ComplexType
    */
   virtual TypeCategory category() const;

   /**
    * @returns always NoDerivation
    */
   virtual DerivationMethod derivationMethod() const;

 protected:
   friend class BuiltinTypes;

   Untyped();
};
}

QT_END_NAMESPACE

#endif
