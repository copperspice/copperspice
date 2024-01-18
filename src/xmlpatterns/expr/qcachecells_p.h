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

#ifndef QCacheCells_P_H
#define QCacheCells_P_H

#include <qlist.h>
#include <qvector.h>
#include <qitem_p.h>

namespace QPatternist {
class ItemCacheCell
{
 public:
   typedef QList<ItemCacheCell> List;
   typedef QVector<ItemCacheCell> Vector;
   enum CacheState {
      Full,
      Empty
   };

   inline ItemCacheCell() : cacheState(Empty) {
   }

   Item        cachedItem;
   CacheState  cacheState;
};

class ItemSequenceCacheCell
{
 public:
   typedef QList<ItemSequenceCacheCell> List;
   typedef QVector<ItemSequenceCacheCell> Vector;

   enum CacheState {
      Full,
      Empty,
      PartiallyPopulated
   };

   inline ItemSequenceCacheCell() : cacheState(Empty)
      , inUse(false) {
   }

   Item::List          cachedItems;
   Item::Iterator::Ptr sourceIterator;
   CacheState          cacheState;

   bool                inUse;
};
}

#endif
