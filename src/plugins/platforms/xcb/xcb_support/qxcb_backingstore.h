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

#ifndef QXCB_BACKINGSTORE_H
#define QXCB_BACKINGSTORE_H

#include <qplatform_backingstore.h>
#include <qxcb_object.h>

#include <xcb/xcb.h>

class QXcbShmImage;

class QXcbBackingStore : public QXcbObject, public QPlatformBackingStore
{
 public:
   QXcbBackingStore(QWindow *widget);
   ~QXcbBackingStore();

   QPaintDevice *paintDevice() override;
   void flush(QWindow *window, const QRegion &region, const QPoint &offset) override;

#ifndef QT_NO_OPENGL
   void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
      QPlatformTextureList *textures, QOpenGLContext *context,
      bool translucentBackground) override;
   QImage toImage() const override;
#endif

   QPlatformGraphicsBuffer *graphicsBuffer() const override;

   void resize(const QSize &size, const QRegion &staticContents) override;
   bool scroll(const QRegion &area, int dx, int dy) override;

   void beginPaint(const QRegion &) override;
   void endPaint() override;

 private:
   QXcbShmImage *m_image;
   QRegion m_paintRegion;
   QImage m_rgbImage;
   QSize m_size;
};

#endif
