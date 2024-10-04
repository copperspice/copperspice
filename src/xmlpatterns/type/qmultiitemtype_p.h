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

#ifndef QMultiItemType_P_H
#define QMultiItemType_P_H

#include <qlist.h>

#include <qitemtype_p.h>

namespace QPatternist {

class MultiItemType : public ItemType
{
 public:
   MultiItemType(const ItemType::List &typeList);

   QString displayName(const NamePool::Ptr &np) const override;

   bool itemMatches(const Item &item) const override;

   bool xdtTypeMatches(const ItemType::Ptr &other) const override;

   bool isNodeType() const override;
   bool isAtomicType() const override;

   ItemType::Ptr xdtSuperType() const override;
   ItemType::Ptr atomizedType() const override;

 private:
   const ItemType::List m_types;
   const ItemType::List::const_iterator m_end;
};

}

#endif
