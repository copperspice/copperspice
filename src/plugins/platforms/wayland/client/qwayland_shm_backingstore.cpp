/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qwayland_shm_backingstore_p.h>

#include <qalgorithms.h>
#include <qdir.h>
#include <qpainter.h>
#include <qtemporaryfile.h>
#include <qwayland_shm_formathelper.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include <qwayland_abstract_decoration_p.h>
#include <qwayland_display_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_window_p.h>

#include <errno.h>
#include <sys/mman.h>

namespace QtWaylandClient {

QWaylandShmBuffer::QWaylandShmBuffer(QWaylandDisplay *display, const QSize &size, QImage::Format format, int scale)
   : m_shmPool(nullptr), m_marginsImage(nullptr)
{
   int stride = size.width() * 4;
   int alloc  = stride * size.height();

   QTemporaryFile tmpFile = QTemporaryFile(QDir::tempPath() + "/wayland-shm-XXXXXX");
   tmpFile.open();

   if (! tmpFile.isOpen()) {
      qWarning("QWaylandShmBuffer() Unable to open temporary file, %s", csPrintable(tmpFile.errorString()));
      return;
   }

   if (! tmpFile.resize(alloc)) {
      qWarning("QWaylandShmBuffer() Unable to resize temporary file, %s", csPrintable(tmpFile.errorString()));
      return;
   }

   int fd = tmpFile.handle();

   uchar *data = (uchar *)mmap(nullptr, alloc, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

   if (data == (uchar *) MAP_FAILED) {
      qWarning("QWaylandShmBuffer() mmap failed, %s", strerror(errno));
      return;
   }

   wl_shm_format wl_format = QWaylandShmFormatHelper::fromQImageFormat(format);
   m_image = QImage(data, size.width(), size.height(), stride, format);
   m_image.setDevicePixelRatio(qreal(scale));

   m_shmPool = wl_shm_create_pool(display->shm(), fd, alloc);
   init(wl_shm_pool_create_buffer(m_shmPool, 0, size.width(), size.height(), stride, wl_format));
}

QWaylandShmBuffer::~QWaylandShmBuffer()
{
   delete m_marginsImage;

   if (m_image.constBits()) {
      munmap((void *) m_image.constBits(), m_image.byteCount());
   }

   if (m_shmPool) {
      wl_shm_pool_destroy(m_shmPool);
   }
}

QImage *QWaylandShmBuffer::imageInsideMargins(const QMargins &marginsIn)
{
   QMargins margins = marginsIn * int(m_image.devicePixelRatio());

   if (! margins.isNull() && margins != m_margins) {
      if (m_marginsImage) {
         delete m_marginsImage;
      }

      uchar *bits     = const_cast < uchar * > (m_image.constBits());
      uchar *b_s_data = bits + margins.top() * m_image.bytesPerLine() + margins.left() * 4;
      int b_s_width   = m_image.size().width() - margins.left() - margins.right();
      int b_s_height  = m_image.size().height() - margins.top() - margins.bottom();

      m_marginsImage  = new QImage(b_s_data, b_s_width, b_s_height, m_image.bytesPerLine(), m_image.format());
      m_marginsImage->setDevicePixelRatio(m_image.devicePixelRatio());
   }

   if (margins.isNull()) {
      delete m_marginsImage;
      m_marginsImage = nullptr;
   }

   m_margins = margins;

   if (! m_marginsImage) {
      return &m_image;
   }

   return m_marginsImage;
}

// **
QWaylandShmBackingStore::QWaylandShmBackingStore(QWindow *window)
   : QPlatformBackingStore(window), m_painting(false), m_pendingFlush(false),
     m_display(QWaylandScreen::waylandScreenFromWindow(window)->display()),
     m_frontBuffer(nullptr), m_backBuffer(nullptr)
{
}

QWaylandShmBackingStore::~QWaylandShmBackingStore()
{
   if (QWaylandWindow *w = waylandWindow()) {
      w->setBackingStore(nullptr);
   }
   qDeleteAll(m_buffers);
}

void QWaylandShmBackingStore::beginPaint(const QRegion &region)
{
   m_painting = true;
   ensureSize();

   waylandWindow()->setCanResize(false);

   if (m_backBuffer->image()->hasAlphaChannel()) {
      QPainter p(paintDevice());
      p.setCompositionMode(QPainter::CompositionMode_Source);

      const QColor blank = Qt::transparent;

      for (const QRect &rect : region.rects()) {
         p.fillRect(rect, blank);
      }
   }
}

QImage *QWaylandShmBackingStore::contentSurface() const
{
   return windowDecoration() ? m_backBuffer->imageInsideMargins(windowDecorationMargins()) : m_backBuffer->image();
}

QImage *QWaylandShmBackingStore::entireSurface() const
{
   return m_backBuffer->image();
}

void QWaylandShmBackingStore::endPaint()
{
   m_painting = false;

   if (m_pendingFlush) {
      flush(window(), m_pendingRegion, QPoint());
   }

   waylandWindow()->setCanResize(true);
}

void QWaylandShmBackingStore::ensureSize()
{
   waylandWindow()->setBackingStore(this);
   waylandWindow()->createDecoration();

   resize(m_requestedSize);
}

void QWaylandShmBackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
   // Invoked when the window is of type RasterSurface or when the window is
   // RasterGLSurface and there are no child widgets requiring OpenGL composition.

   // For the case of RasterGLSurface + having to compose, the composeAndFlush() is
   // called instead. The default implementation from QPlatformBackingStore is sufficient
   // however so no need to reimplement that.

   (void) window;
   (void) offset;

   if (m_painting) {
      m_pendingRegion |= region;
      m_pendingFlush = true;

      return;
   }

   m_pendingFlush  = false;
   m_pendingRegion = QRegion();

   if (windowDecoration() && windowDecoration()->isDirty()) {
      updateDecorations();
   }

   m_frontBuffer = m_backBuffer;

   QMargins margins = windowDecorationMargins();

   waylandWindow()->attachOffset(m_frontBuffer);
   m_frontBuffer->setBusy();

   for (const QRect &item : region.rects()) {
      waylandWindow()->damage(item.translated(margins.left(), margins.top()));
   }

   waylandWindow()->commit();
}

QWaylandShmBuffer *QWaylandShmBackingStore::getBuffer(const QSize &size)
{
   auto tmpBuffers = m_buffers;

   for (QWaylandShmBuffer *item : tmpBuffers) {
      if (! item->busy()) {
         if (item->size() == size) {
            return item;

         } else {
            m_buffers.removeOne(item);

            if (m_backBuffer == item) {
               m_backBuffer = nullptr;
            }

            delete item;
         }
      }
   }

   static const int MAX_BUFFERS = 5;

   if (m_buffers.count() < MAX_BUFFERS) {
      QImage::Format format = QPlatformScreen::platformScreenForWindow(window())->format();

      QWaylandShmBuffer *newBuffer = new QWaylandShmBuffer(m_display, size, format, waylandWindow()->scale());
      m_buffers.prepend(newBuffer);

      return newBuffer;
   }

   return nullptr;
}

QPaintDevice *QWaylandShmBackingStore::paintDevice()
{
   return contentSurface();
}

void QWaylandShmBackingStore::resize(const QSize &size)
{
   QMargins margins = windowDecorationMargins();
   int scale = waylandWindow()->scale();

   QSize sizeWithMargins = (size + QSize(margins.left() + margins.right(), margins.top() + margins.bottom())) * scale;

   QWaylandShmBuffer *newBuffer = getBuffer(sizeWithMargins);

   while (newBuffer == nullptr) {
      m_display->blockingReadEvents();
      newBuffer = getBuffer(sizeWithMargins);
   }

   int oldSize = 0;

   if (m_backBuffer != nullptr) {
      oldSize = m_backBuffer->image()->byteCount();
   }

   if (m_backBuffer != nullptr && m_backBuffer != newBuffer && oldSize == newBuffer->image()->byteCount()) {
      memcpy(newBuffer->image()->bits(), m_backBuffer->image()->constBits(), newBuffer->image()->byteCount());
   }

   m_backBuffer = newBuffer;

   // ensure the new buffer is at the beginning of the list so the next time around getBuffer() will pick it if possible
   if (m_buffers.first() != newBuffer) {
      m_buffers.removeOne(newBuffer);
      m_buffers.prepend(newBuffer);
   }

   if (windowDecoration() && window()->isVisible()) {
      windowDecoration()->update();
   }
}

void QWaylandShmBackingStore::resize(const QSize &size, const QRegion &)
{
   m_requestedSize = size;
}

void QWaylandShmBackingStore::updateDecorations()
{
   QPainter decorationPainter(entireSurface());
   decorationPainter.setCompositionMode(QPainter::CompositionMode_Source);
   QImage sourceImage = windowDecoration()->contentImage();

   qreal dp     = sourceImage.devicePixelRatio();
   int dpWidth  = int(sourceImage.width() / dp);
   int dpHeight = int(sourceImage.height() / dp);

   QMatrix sourceMatrix;
   sourceMatrix.scale(dp, dp);

   QRect target;  // device independent pixels

   // top
   target.setX(0);
   target.setY(0);
   target.setWidth(dpWidth);
   target.setHeight(windowDecorationMargins().top());
   decorationPainter.drawImage(target, sourceImage, sourceMatrix.mapRect(target));

   // left
   target.setWidth(windowDecorationMargins().left());
   target.setHeight(dpHeight);
   decorationPainter.drawImage(target, sourceImage, sourceMatrix.mapRect(target));

   // right
   target.setX(dpWidth - windowDecorationMargins().right());
   target.setWidth(windowDecorationMargins().right());
   decorationPainter.drawImage(target, sourceImage, sourceMatrix.mapRect(target));

   // bottom
   target.setX(0);
   target.setY(dpHeight - windowDecorationMargins().bottom());
   target.setWidth(dpWidth);
   target.setHeight(windowDecorationMargins().bottom());
   decorationPainter.drawImage(target, sourceImage, sourceMatrix.mapRect(target));
}

QWaylandWindow *QWaylandShmBackingStore::waylandWindow() const
{
   return static_cast<QWaylandWindow *>(window()->handle());
}

QWaylandAbstractDecoration *QWaylandShmBackingStore::windowDecoration() const
{
   return waylandWindow()->decoration();
}

QMargins QWaylandShmBackingStore::windowDecorationMargins() const
{
   if (windowDecoration() != nullptr) {
      return windowDecoration()->margins();
   }

   return QMargins();
}

#ifndef QT_NO_OPENGL
QImage QWaylandShmBackingStore::toImage() const
{
   return *contentSurface();
}
#endif

}
