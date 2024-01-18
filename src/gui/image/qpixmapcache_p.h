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

#ifndef QPIXMAPCACHE_P_H
#define QPIXMAPCACHE_P_H

#include <qpixmapcache.h>
#include <qpaintengine.h>
#include <qimage_p.h>
#include <qpixmap_raster_p.h>
#include <qcache.h>

uint qHash(const QPixmapCache::Key &k);

class QPixmapCache::KeyData
{
 public:
   KeyData()
      : isValid(true), key(0), ref(1)
   {
   }

   KeyData(const KeyData &other)
      : isValid(other.isValid), key(other.key), ref(1)
   {
   }

   ~KeyData()
   {
   }

   bool isValid;
   int key;
   int ref;
};

// XXX: hw: is this a general concept we need to abstract?
class QPixmapCacheEntry : public QPixmap
{
 public:
   QPixmapCacheEntry(const QPixmapCache::Key &key, const QPixmap &pix) : QPixmap(pix), key(key) {
      QPlatformPixmap *pd = handle();

      if (pd && pd->classId() == QPlatformPixmap::RasterClass) {
         QRasterPlatformPixmap *d = static_cast<QRasterPlatformPixmap *>(pd);

         if (!d->image.isNull() && d->image.d->paintEngine && ! d->image.d->paintEngine->isActive()) {
            delete d->image.d->paintEngine;
            d->image.d->paintEngine = nullptr;
         }
      }
   }

   ~QPixmapCacheEntry();
   QPixmapCache::Key key;
};


#endif
