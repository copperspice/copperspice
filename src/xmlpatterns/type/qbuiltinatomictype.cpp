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

#include "qatomictype_p.h"
#include "qitem_p.h"
#include "qbuiltintypes_p.h"
#include "qschematype_p.h"

#include "qbuiltinatomictype_p.h"

using namespace QPatternist;

BuiltinAtomicType::BuiltinAtomicType(const AtomicType::Ptr &base,
                                     const AtomicComparatorLocator::Ptr &comp,
                                     const AtomicMathematicianLocator::Ptr &mather,
                                     const AtomicCasterLocator::Ptr &casterlocator)
   : m_superType(base),
     m_comparatorLocator(comp),
     m_mathematicianLocator(mather),
     m_casterLocator(casterlocator)
{
}

SchemaType::Ptr BuiltinAtomicType::wxsSuperType() const
{
   return m_superType;
}

ItemType::Ptr BuiltinAtomicType::xdtSuperType() const
{
   return m_superType;
}

bool BuiltinAtomicType::isAbstract() const
{
   return false;
}

AtomicComparatorLocator::Ptr BuiltinAtomicType::comparatorLocator() const
{
   return m_comparatorLocator;
}

AtomicMathematicianLocator::Ptr BuiltinAtomicType::mathematicianLocator() const
{
   return m_mathematicianLocator;
}

AtomicCasterLocator::Ptr BuiltinAtomicType::casterLocator() const
{
   return m_casterLocator;
}

