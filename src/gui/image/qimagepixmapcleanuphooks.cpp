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

#include <qimagepixmapcleanuphooks_p.h>
#include <qimage_p.h>

#include <qplatform_pixmap.h>

static QImagePixmapCleanupHooks *qt_image_and_pixmap_cleanup_hooks()
{
   static QImagePixmapCleanupHooks retval;
   return &retval;
}

QImagePixmapCleanupHooks *QImagePixmapCleanupHooks::instance()
{
   return qt_image_and_pixmap_cleanup_hooks();
}

void QImagePixmapCleanupHooks::addPlatformPixmapModificationHook(_qt_pixmap_cleanup_hook_pmd hook)
{
   pixmapModificationHooks.append(hook);
}

void QImagePixmapCleanupHooks::addPlatformPixmapDestructionHook(_qt_pixmap_cleanup_hook_pmd hook)
{
   pixmapDestructionHooks.append(hook);
}


void QImagePixmapCleanupHooks::addImageHook(_qt_image_cleanup_hook_64 hook)
{
   imageHooks.append(hook);
}

void QImagePixmapCleanupHooks::removePlatformPixmapModificationHook(_qt_pixmap_cleanup_hook_pmd hook)
{
   pixmapModificationHooks.removeAll(hook);
}

void QImagePixmapCleanupHooks::removePlatformPixmapDestructionHook(_qt_pixmap_cleanup_hook_pmd hook)
{
   pixmapDestructionHooks.removeAll(hook);
}

void QImagePixmapCleanupHooks::removeImageHook(_qt_image_cleanup_hook_64 hook)
{
   imageHooks.removeAll(hook);
}

void QImagePixmapCleanupHooks::executePlatformPixmapModificationHooks(QPlatformPixmap *pmd)
{
   QImagePixmapCleanupHooks *h = qt_image_and_pixmap_cleanup_hooks();
   // the global destructor for the pixmap and image hooks might have
   // been called already if the app is "leaking" global
   // pixmaps/images

   if (! h) {
      return;
   }

   for (int i = 0; i < h->pixmapModificationHooks.count(); ++i) {
      h->pixmapModificationHooks[i](pmd);
   }
}

void QImagePixmapCleanupHooks::executePlatformPixmapDestructionHooks(QPlatformPixmap *pmd)
{
   QImagePixmapCleanupHooks *h = qt_image_and_pixmap_cleanup_hooks();
   // the global destructor for the pixmap and image hooks might have
   // been called already if the app is "leaking" global
   // pixmaps/images
   if (!h) {
      return;
   }

   for (int i = 0; i < h->pixmapDestructionHooks.count(); ++i) {
      h->pixmapDestructionHooks[i](pmd);
   }
}

void QImagePixmapCleanupHooks::executeImageHooks(qint64 key)
{
   QImagePixmapCleanupHooks *h = qt_image_and_pixmap_cleanup_hooks();
   // the global destructor for the pixmap and image hooks might have
   // been called already if the app is "leaking" global pixmaps/images

   if (!h) {
      return;
   }

   for (int i = 0; i < h->imageHooks.count(); ++i) {
      h->imageHooks[i](key);
   }
}

void QImagePixmapCleanupHooks::enableCleanupHooks(QPlatformPixmap *handle)
{
   handle->is_cached = true;
}

void QImagePixmapCleanupHooks::enableCleanupHooks(const QPixmap &pixmap)
{
   auto tmp = const_cast<QPixmap &>(pixmap).data_ptr().data();
   enableCleanupHooks(tmp);
}

void QImagePixmapCleanupHooks::enableCleanupHooks(const QImage &image)
{
   const_cast<QImage &>(image).data_ptr()->is_cached = true;
}

bool QImagePixmapCleanupHooks::isImageCached(const QImage &image)
{
   return const_cast<QImage &>(image).data_ptr()->is_cached;
}

bool QImagePixmapCleanupHooks::isPixmapCached(const QPixmap &pixmap)
{
   return const_cast<QPixmap &>(pixmap).data_ptr().data()->is_cached;
}

