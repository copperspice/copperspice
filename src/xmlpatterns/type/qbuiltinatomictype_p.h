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

#ifndef QBuiltinAtomicType_P_H
#define QBuiltinAtomicType_P_H

#include <qatomictype_p.h>

namespace QPatternist {

class BuiltinAtomicType : public AtomicType
{
 public:

   typedef QExplicitlySharedDataPointer<BuiltinAtomicType> Ptr;

   bool isAbstract() const override;

   SchemaType::Ptr wxsSuperType() const override;
   ItemType::Ptr xdtSuperType() const override;

   AtomicComparatorLocator::Ptr comparatorLocator() const override;
   AtomicMathematicianLocator::Ptr mathematicianLocator() const override;
   AtomicCasterLocator::Ptr casterLocator() const override;

 protected:
   friend class BuiltinTypes;

   BuiltinAtomicType(const AtomicType::Ptr &baseType,
                     const AtomicComparatorLocator::Ptr &comp,
                     const AtomicMathematicianLocator::Ptr &mather,
                     const AtomicCasterLocator::Ptr &casterLocator);

 private:
   const AtomicType::Ptr                   m_superType;
   const AtomicComparatorLocator::Ptr      m_comparatorLocator;
   const AtomicMathematicianLocator::Ptr   m_mathematicianLocator;
   const AtomicCasterLocator::Ptr          m_casterLocator;
};

}


#endif
