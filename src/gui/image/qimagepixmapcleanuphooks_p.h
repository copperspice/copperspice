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

#ifndef QIMAGEPIXMAP_CLEANUPHOOKS_P_H
#define QIMAGEPIXMAP_CLEANUPHOOKS_P_H

#include <qpixmap.h>

typedef void (*_qt_image_cleanup_hook_64)(qint64);
typedef void (*_qt_pixmap_cleanup_hook_pmd)(QPlatformPixmap *);

class QImagePixmapCleanupHooks;

class Q_GUI_EXPORT QImagePixmapCleanupHooks
{
 public:
   static QImagePixmapCleanupHooks *instance();

   static void enableCleanupHooks(const QImage &image);
   static void enableCleanupHooks(const QPixmap &pixmap);
   static void enableCleanupHooks(QPlatformPixmap *handle);

   static bool isImageCached(const QImage &image);
   static bool isPixmapCached(const QPixmap &pixmap);

   // Gets called when a pixmap data is about to be modified:
   void addPlatformPixmapModificationHook(_qt_pixmap_cleanup_hook_pmd);

   // Gets called when a pixmap data is about to be destroyed:
   void addPlatformPixmapDestructionHook(_qt_pixmap_cleanup_hook_pmd);

   // Gets called when an image is about to be modified or destroyed:
   void addImageHook(_qt_image_cleanup_hook_64);

   void removePlatformPixmapModificationHook(_qt_pixmap_cleanup_hook_pmd);
   void removePlatformPixmapDestructionHook(_qt_pixmap_cleanup_hook_pmd);
   void removeImageHook(_qt_image_cleanup_hook_64);

   static void executePlatformPixmapModificationHooks(QPlatformPixmap *);
   static void executePlatformPixmapDestructionHooks(QPlatformPixmap *);
   static void executeImageHooks(qint64 key);

 private:
   QList<_qt_image_cleanup_hook_64> imageHooks;
   QList<_qt_pixmap_cleanup_hook_pmd> pixmapModificationHooks;
   QList<_qt_pixmap_cleanup_hook_pmd> pixmapDestructionHooks;
};

#endif
