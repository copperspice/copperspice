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

#include "qbuiltintypes_p.h"
#include "qcommonnamespaces_p.h"

#include "qbasictypesfactory_p.h"

using namespace QPatternist;

SchemaTypeFactory::Ptr BasicTypesFactory::self(const NamePool::Ptr &np)
{
   /* We don't store a global static here, because it's dependent on the NamePool. */
   return SchemaTypeFactory::Ptr(new BasicTypesFactory(np));
}

BasicTypesFactory::BasicTypesFactory(const NamePool::Ptr &np)
{
   m_types.reserve(48);

#define add(aName)   m_types.insert(BuiltinTypes::aName->name(np), AtomicType::Ptr(BuiltinTypes::aName))
#define addNA(aName) m_types.insert(BuiltinTypes::aName->name(np), BuiltinTypes::aName)
   add(xsString);
   add(xsBoolean);
   add(xsDecimal);
   add(xsDouble);
   add(xsFloat);
   add(xsDate);
   add(xsTime);
   add(xsDateTime);
   add(xsDuration);
   add(xsAnyURI);
   add(xsGDay);
   add(xsGMonthDay);
   add(xsGMonth);
   add(xsGYearMonth);
   add(xsGYear);
   add(xsBase64Binary);
   add(xsHexBinary);
   add(xsQName);
   add(xsInteger);
   addNA(xsAnyType);
   addNA(xsAnySimpleType);
   add(xsYearMonthDuration);
   add(xsDayTimeDuration);
   add(xsAnyAtomicType);
   addNA(xsUntyped);
   add(xsUntypedAtomic);
   add(xsNOTATION);
   /* Add derived primitives. */
   add(xsNonPositiveInteger);
   add(xsNegativeInteger);
   add(xsLong);
   add(xsInt);
   add(xsShort);
   add(xsByte);
   add(xsNonNegativeInteger);
   add(xsUnsignedLong);
   add(xsUnsignedInt);
   add(xsUnsignedShort);
   add(xsUnsignedByte);
   add(xsPositiveInteger);
   add(xsNormalizedString);
   add(xsToken);
   add(xsLanguage);
   add(xsNMTOKEN);
   add(xsName);
   add(xsNCName);
   add(xsID);
   add(xsIDREF);
   add(xsENTITY);

#undef add
#undef addNA
}

SchemaType::Ptr BasicTypesFactory::createSchemaType(const QXmlName name) const
{
   return m_types.value(name);
}

SchemaType::Hash BasicTypesFactory::types() const
{
   return m_types;
}

