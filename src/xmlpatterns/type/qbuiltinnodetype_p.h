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

#ifndef QBuiltinNodeType_P_H
#define QBuiltinNodeType_P_H

#include <qitem_p.h>
#include <qanynodetype_p.h>
#include <qbuiltintypes_p.h>

namespace QPatternist {

template <const QXmlNodeModelIndex::NodeKind kind>
class BuiltinNodeType : public AnyNodeType
{
 public:
   bool xdtTypeMatches(const ItemType::Ptr &other) const override;
   bool itemMatches(const Item &item) const override;

   /**
    * @returns for example "text()", depending on what the constructor was passed
    */
   QString displayName(const NamePool::Ptr &np) const override;

   ItemType::Ptr xdtSuperType() const override;
   ItemType::Ptr atomizedType() const override;

   QXmlNodeModelIndex::NodeKind nodeKind() const override;

   PatternPriority patternPriority() const override;

 protected:
   friend class BuiltinTypes;

   /**
    * This constructor does nothing, but exists in order to make it impossible to
    * instantiate this class from anywhere but from BuiltinTypes.
    */
   BuiltinNodeType();
};

#include "qbuiltinnodetype.cpp"

}

#endif
