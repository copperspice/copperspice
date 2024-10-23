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

   QString displayName(const NamePool::Ptr &np) const override;
   bool isAbstract() const override;

   SchemaType::Ptr wxsSuperType() const override;
   bool wxsTypeMatches(const SchemaType::Ptr &other) const override;

   TypeCategory category() const override;

   DerivationMethod derivationMethod() const override;
   DerivationConstraints derivationConstraints() const override;

   bool isComplexType() const override;

 protected:
   AnyType() {
   }
};

}

#endif
