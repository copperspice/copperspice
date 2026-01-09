/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QWAYLAND_SHM_BACKINGSTORE_H
#define QWAYLAND_SHM_BACKINGSTORE_H

#include <qimage.h>
#include <qlinkedlist.h>
#include <qmutex.h>
#include <qplatform_backingstore.h>
#include <qplatform_window.h>

#include <qwayland_buffer_p.h>

namespace QtWaylandClient {

class QWaylandAbstractDecoration;
class QWaylandDisplay;
class QWaylandWindow;

class Q_WAYLAND_CLIENT_EXPORT QWaylandShmBuffer : public QWaylandBuffer
{
 public:
   QWaylandShmBuffer(QWaylandDisplay *display, const QSize &size, QImage::Format format, int scale = 1);
   ~QWaylandShmBuffer();

   QSize size() const override {
      return m_image.size();
   }

   int scale() const override {
      return int(m_image.devicePixelRatio());
   }

   QImage *image() {
      return &m_image;
   }

   QImage *imageInsideMargins(const QMargins &margins);

 private:
   struct wl_shm_pool *m_shmPool;

   QImage m_image;
   QMargins m_margins;
   QImage *m_marginsImage;
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandShmBackingStore : public QPlatformBackingStore
{
 public:
   QWaylandShmBackingStore(QWindow *window);
   ~QWaylandShmBackingStore();

   void beginPaint(const QRegion &) override;

   QImage *contentSurface() const;

   void endPaint() override;

   void ensureSize();
   QImage *entireSurface() const;

   void flush(QWindow *window, const QRegion &region, const QPoint &offset) override;

#ifndef QT_NO_OPENGL
   QImage toImage() const override;
#endif

   void iterateBuffer();

   QPaintDevice *paintDevice() override;

   void resize(const QSize &size, const QRegion &staticContents) override;
   void resize(const QSize &size);

   QWaylandWindow *waylandWindow() const;

   QWaylandAbstractDecoration *windowDecoration() const;
   QMargins windowDecorationMargins() const;

 private:
   QWaylandShmBuffer *getBuffer(const QSize &size);
   void updateDecorations();

   bool m_painting;
   bool m_pendingFlush;

   QWaylandDisplay *m_display;
   QLinkedList<QWaylandShmBuffer *> m_buffers;
   QWaylandShmBuffer *m_frontBuffer;
   QWaylandShmBuffer *m_backBuffer;

   QMutex m_mutex;
   QRegion m_pendingRegion;
   QSize m_requestedSize;

   Qt::WindowFlags m_currentWindowFlags;
};

}

#endif
