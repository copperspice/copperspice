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

#ifndef QDeclarativePaintedItem_P_P_H
#define QDeclarativePaintedItem_P_P_H

#include <qdeclarativeitem_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativePaintedItemPrivate : public QDeclarativeItemPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativePaintedItem)

 public:
   QDeclarativePaintedItemPrivate()
      : max_imagecache_size(100000), contentsScale(1.0), fillColor(Qt::transparent), cachefrozen(false), smoothCache(true) {
   }

   struct ImageCacheItem {
      ImageCacheItem() : age(0) {}
      ~ImageCacheItem() { }
      int age;
      QRect area;
      QRect dirty; // one dirty area (allows optimization of common cases)
      QPixmap image;
   };

   QList<ImageCacheItem *> imagecache;

   int max_imagecache_size;
   QSize contentsSize;
   qreal contentsScale;
   QColor fillColor;
   bool cachefrozen;
   bool smoothCache;
};

QT_END_NAMESPACE
#endif
