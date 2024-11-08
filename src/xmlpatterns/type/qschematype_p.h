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

#ifndef QSchemaType_P_H
#define QSchemaType_P_H

#include <qnamepool_p.h>
#include <qschemacomponent_p.h>
#include <qxmlname.h>
#include <qcontainerfwd.h>

namespace QPatternist {

class AtomicType;

class SchemaType : public SchemaComponent
{
 public:
   typedef QExplicitlySharedDataPointer<SchemaType> Ptr;
   typedef QHash<QXmlName, SchemaType::Ptr> Hash;
   typedef QList<SchemaType::Ptr> List;

   enum TypeCategory {
      None = 0,
      SimpleTypeAtomic,
      SimpleTypeList,
      SimpleTypeUnion,
      ComplexType
   };

   enum DerivationMethod {
      DerivationRestriction  = 1,
      DerivationExtension    = 2,
      DerivationUnion        = 4,
      DerivationList         = 8,
      NoDerivation           = 16
   };

   enum DerivationConstraint {
      RestrictionConstraint = 1,
      ExtensionConstraint   = 2,
      ListConstraint        = 4,
      UnionConstraint       = 8
   };
   using DerivationConstraints = QFlags<DerivationConstraint>;

   SchemaType();
   virtual ~SchemaType();

   virtual DerivationMethod derivationMethod() const = 0;
   virtual DerivationConstraints derivationConstraints() const = 0;

   virtual bool isAbstract() const = 0;

   virtual QXmlName name(const NamePool::Ptr &np) const = 0;
   virtual QString displayName(const NamePool::Ptr &np) const = 0;

   virtual SchemaType::Ptr wxsSuperType() const = 0;
   virtual bool wxsTypeMatches(const SchemaType::Ptr &other) const = 0;

   virtual TypeCategory category() const = 0;

   virtual bool isSimpleType() const;
   virtual bool isComplexType() const;
   virtual bool isDefinedBySchema() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SchemaType::DerivationConstraints)

}

#endif
