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

#include <qxcb_backingstore.h>

#include <qdebug.h>
#include <qpainter.h>
#include <qscreen.h>
#include <qplatform_graphicsbuffer.h>
#include <qxcb_connection.h>
#include <qxcb_screen.h>
#include <qxcb_window.h>

#include <qhighdpiscaling_p.h>
#include <qimage_p.h>

#include <xcb/shm.h>
#include <xcb/xcb_image.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <errno.h>

#include <algorithm>

class QXcbShmImage : public QXcbObject
{
 public:
   QXcbShmImage(QXcbScreen *connection, const QSize &size, uint depth, QImage::Format format);
   ~QXcbShmImage() {
      destroy();
   }

   QImage *image() {
      return &m_qimage;
   }
   QPlatformGraphicsBuffer *graphicsBuffer() {
      return m_graphics_buffer;
   }

   QSize size() const {
      return m_qimage.size();
   }

   bool hasAlpha() const {
      return m_hasAlpha;
   }

   void put(xcb_window_t window, const QPoint &dst, const QRect &source);
   void preparePaint(const QRegion &region);

 private:
   void destroy();

   xcb_shm_segment_info_t m_shm_info;

   xcb_image_t *m_xcb_image;

   QImage m_qimage;
   QPlatformGraphicsBuffer *m_graphics_buffer;

   xcb_gcontext_t m_gc;
   xcb_window_t m_gc_window;

   QRegion m_dirty;

   bool m_hasAlpha;
};

class QXcbShmGraphicsBuffer : public QPlatformGraphicsBuffer
{
 public:
   QXcbShmGraphicsBuffer(QImage *image)
      : QPlatformGraphicsBuffer(image->size(), QImage::toPixelFormat(image->format()))
      , m_access_lock(QPlatformGraphicsBuffer::None)
      , m_image(image)
   { }

   bool doLock(AccessTypes access, const QRect &rect) override {
      (void) rect;
      if (access & ~(QPlatformGraphicsBuffer::SWReadAccess | QPlatformGraphicsBuffer::SWWriteAccess)) {
         return false;
      }

      m_access_lock |= access;
      return true;
   }
   void doUnlock() override {
      m_access_lock = None;
   }

   const uchar *data() const override {
      return m_image->bits();
   }
   uchar *data() override {
      return m_image->bits();
   }
   int bytesPerLine() const override {
      return m_image->bytesPerLine();
   }

   Origin origin() const override {
      return QPlatformGraphicsBuffer::OriginTopLeft;
   }
 private:
   AccessTypes m_access_lock;
   QImage *m_image;
};

QXcbShmImage::QXcbShmImage(QXcbScreen *screen, const QSize &size, uint depth, QImage::Format format)
   : QXcbObject(screen->connection())
   , m_graphics_buffer(nullptr)
   , m_gc(0)
   , m_gc_window(0)
{
   Q_XCB_NOOP(connection());

   const xcb_setup_t *setup = xcb_get_setup(xcb_connection());
   xcb_format_t *fmt = xcb_setup_pixmap_formats(setup);
   xcb_format_t *fmtend = fmt + xcb_setup_pixmap_formats_length(setup);
   for (; fmt != fmtend; ++fmt)
      if (fmt->depth == depth) {
         break;
      }

   Q_ASSERT(fmt != fmtend);

   m_xcb_image = xcb_image_create(size.width(), size.height(),
         XCB_IMAGE_FORMAT_Z_PIXMAP, fmt->scanline_pad, fmt->depth, fmt->bits_per_pixel, 0,
         QSysInfo::ByteOrder == QSysInfo::BigEndian ? XCB_IMAGE_ORDER_MSB_FIRST : XCB_IMAGE_ORDER_LSB_FIRST,
         XCB_IMAGE_ORDER_MSB_FIRST, nullptr, ~0, nullptr);

   const int segmentSize = m_xcb_image->stride * m_xcb_image->height;
   if (!segmentSize) {
      return;
   }

   int id = shmget(IPC_PRIVATE, segmentSize, IPC_CREAT | 0600);
   if (id == -1) {
      qWarning("QXcbShmImage: shmget() failed (%d: %s) for size %d (%dx%d)",
         errno, strerror(errno), segmentSize, size.width(), size.height());
   } else {
      m_shm_info.shmaddr = m_xcb_image->data = (quint8 *)shmat(id, nullptr, 0);
   }

   m_shm_info.shmid = id;
   m_shm_info.shmseg = xcb_generate_id(xcb_connection());

   const xcb_query_extension_reply_t *shm_reply = xcb_get_extension_data(xcb_connection(), &xcb_shm_id);
   bool shm_present = shm_reply != nullptr && shm_reply->present;
   xcb_generic_error_t *error = nullptr;

   if (shm_present) {
      error = xcb_request_check(xcb_connection(), xcb_shm_attach_checked(xcb_connection(), m_shm_info.shmseg, m_shm_info.shmid, false));
   }
   if (!shm_present || error || id == -1) {
      free(error);

      if (id != -1) {
         shmdt(m_shm_info.shmaddr);
         shmctl(m_shm_info.shmid, IPC_RMID, nullptr);
      }
      m_shm_info.shmaddr = nullptr;

      m_xcb_image->data = (uint8_t *)malloc(segmentSize);

   } else {
      if (shmctl(m_shm_info.shmid, IPC_RMID, nullptr) == -1) {
         qWarning() << "QXcbBackingStore: Error while marking the shared memory segment to be destroyed";
      }
   }

   m_hasAlpha = QImage::toPixelFormat(format).alphaUsage() == QPixelFormat::UsesAlpha;
   if (! m_hasAlpha) {
      format = qt_maybeAlphaVersionWithSameDepth(format);
   }

   m_qimage = QImage( (uchar *) m_xcb_image->data, m_xcb_image->width, m_xcb_image->height, m_xcb_image->stride, format);
   m_graphics_buffer = new QXcbShmGraphicsBuffer(&m_qimage);
}

void QXcbShmImage::destroy()
{
   const int segmentSize = m_xcb_image ? (m_xcb_image->stride * m_xcb_image->height) : 0;
   if (segmentSize && m_shm_info.shmaddr) {
      Q_XCB_CALL(xcb_shm_detach(xcb_connection(), m_shm_info.shmseg));
   }

   if (segmentSize) {
      if (m_shm_info.shmaddr) {
         shmdt(m_shm_info.shmaddr);
         shmctl(m_shm_info.shmid, IPC_RMID, nullptr);
      } else {
         free(m_xcb_image->data);
      }
   }

   xcb_image_destroy(m_xcb_image);

   if (m_gc) {
      Q_XCB_CALL(xcb_free_gc(xcb_connection(), m_gc));
   }
   delete m_graphics_buffer;
   m_graphics_buffer = nullptr;
}

void QXcbShmImage::put(xcb_window_t window, const QPoint &target, const QRect &source)
{
   Q_XCB_NOOP(connection());
   if (m_gc_window != window) {
      if (m_gc) {
         Q_XCB_CALL(xcb_free_gc(xcb_connection(), m_gc));
      }

      m_gc = xcb_generate_id(xcb_connection());
      Q_XCB_CALL(xcb_create_gc(xcb_connection(), m_gc, window, 0, nullptr));

      m_gc_window = window;
   }

   Q_XCB_NOOP(connection());
   if (m_shm_info.shmaddr) {
      xcb_image_shm_put(xcb_connection(),
         window,
         m_gc,
         m_xcb_image,
         m_shm_info,
         source.x(),
         source.y(),
         target.x(),
         target.y(),
         source.width(),
         source.height(),
         false);
   } else {
      // If we upload the whole image in a single chunk, the result might be
      // larger than the server's maximum request size and stuff breaks.
      // To work around that, we upload the image in chunks where each chunk
      // is small enough for a single request.
      int src_x = source.x();
      int src_y = source.y();
      int target_x = target.x();
      int target_y = target.y();
      int width = source.width();
      int height = source.height();

      // We must make sure that each request is not larger than max_req_size.
      // Each request takes req_size + m_xcb_image->stride * height bytes.
      uint32_t max_req_size = xcb_get_maximum_request_length(xcb_connection());
      uint32_t req_size = sizeof(xcb_put_image_request_t);
      int rows_per_put = (max_req_size - req_size) / m_xcb_image->stride;

      // This assert could trigger if a single row has more pixels than fit in
      // a single PutImage request. However, max_req_size is guaranteed to be
      // at least 16384 bytes. That should be enough for quite large images.
      Q_ASSERT(rows_per_put > 0);

      // Convert the image to the native byte order.
      xcb_image_t *converted_image = xcb_image_native(xcb_connection(), m_xcb_image, 1);

      while (height > 0) {
         int rows = (std::min)(height, rows_per_put);

         xcb_image_t *subimage = xcb_image_subimage(converted_image, src_x, src_y, width, rows,
               nullptr, 0, nullptr);

         xcb_image_put(xcb_connection(), window, m_gc, subimage, target_x, target_y, 0);
         xcb_image_destroy(subimage);

         src_y += rows;
         target_y += rows;
         height -= rows;
      }

      if (converted_image != m_xcb_image) {
         xcb_image_destroy(converted_image);
      }
   }
   Q_XCB_NOOP(connection());

   m_dirty = m_dirty | source;
}

void QXcbShmImage::preparePaint(const QRegion &region)
{
   // to prevent X from reading from the image region while we're writing to it
   if (m_dirty.intersects(region)) {
      connection()->sync();
      m_dirty = QRegion();
   }
}

QXcbBackingStore::QXcbBackingStore(QWindow *window)
   : QPlatformBackingStore(window), m_image(nullptr)
{
   QXcbScreen *screen = static_cast<QXcbScreen *>(window->screen()->handle());
   setConnection(screen->connection());
}

QXcbBackingStore::~QXcbBackingStore()
{
   delete m_image;
}

QPaintDevice *QXcbBackingStore::paintDevice()
{
   if (!m_image) {
      return nullptr;
   }

   return m_rgbImage.isNull() ? m_image->image() : &m_rgbImage;
}

void QXcbBackingStore::beginPaint(const QRegion &region)
{
   if (!m_image && !m_size.isEmpty()) {
      resize(m_size, QRegion());
   }

   if (!m_image) {
      return;
   }
   m_size = QSize();
   m_paintRegion = region;
   m_image->preparePaint(m_paintRegion);

   if (m_image->hasAlpha()) {
      QPainter p(paintDevice());
      p.setCompositionMode(QPainter::CompositionMode_Source);
      const QVector<QRect> rects = m_paintRegion.rects();
      const QColor blank = Qt::transparent;
      for (QVector<QRect>::const_iterator it = rects.begin(); it != rects.end(); ++it) {
         p.fillRect(*it, blank);
      }
   }
}

void QXcbBackingStore::endPaint()
{
   QXcbWindow *platformWindow = static_cast<QXcbWindow *>(window()->handle());
   if (!platformWindow || !platformWindow->imageNeedsRgbSwap()) {
      return;
   }

   // Slow path: the paint device was m_rgbImage. Now copy with swapping red
   // and blue into m_image.
   const QVector<QRect> rects = m_paintRegion.rects();
   if (rects.isEmpty()) {
      return;
   }
   QPainter p(m_image->image());
   for (QVector<QRect>::const_iterator it = rects.begin(); it != rects.end(); ++it) {
      const QRect rect = *it;
      p.drawImage(rect.topLeft(), m_rgbImage.copy(rect).rgbSwapped());
   }
}

#ifndef QT_NO_OPENGL
QImage QXcbBackingStore::toImage() const
{
   return m_image && m_image->image() ? *m_image->image() : QImage();
}
#endif

QPlatformGraphicsBuffer *QXcbBackingStore::graphicsBuffer() const
{
   return m_image ? m_image->graphicsBuffer() : nullptr;
}

void QXcbBackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
   if (!m_image || m_image->size().isEmpty()) {
      return;
   }

   QSize imageSize = m_image->size();

   QRegion clipped = region;
   clipped &= QRect(QPoint(), QHighDpi::toNativePixels(window->size(), window));
   clipped &= QRect(0, 0, imageSize.width(), imageSize.height()).translated(-offset);

   QRect bounds = clipped.boundingRect();

   if (bounds.isNull()) {
      return;
   }

   Q_XCB_NOOP(connection());

   QXcbWindow *platformWindow = static_cast<QXcbWindow *>(window->handle());
   if (!platformWindow) {
      qWarning("QXcbBackingStore::flush: QWindow has no platform window (QTBUG-32681)");
      return;
   }

   QVector<QRect> rects = clipped.rects();
   for (int i = 0; i < rects.size(); ++i) {
      QRect rect = QRect(rects.at(i).topLeft(), rects.at(i).size());
      m_image->put(platformWindow->xcb_window(), rect.topLeft(), rect.translated(offset));
   }

   Q_XCB_NOOP(connection());

   if (platformWindow->needsSync()) {
      platformWindow->updateSyncRequestCounter();
   } else {
      xcb_flush(xcb_connection());
   }
}

#ifndef QT_NO_OPENGL
void QXcbBackingStore::composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
   QPlatformTextureList *textures, QOpenGLContext *context,
   bool translucentBackground)
{
   QPlatformBackingStore::composeAndFlush(window, region, offset, textures, context, translucentBackground);

   Q_XCB_NOOP(connection());

   QXcbWindow *platformWindow = static_cast<QXcbWindow *>(window->handle());
   if (platformWindow->needsSync()) {
      platformWindow->updateSyncRequestCounter();
   } else {
      xcb_flush(xcb_connection());
   }
}
#endif // QT_NO_OPENGL

void QXcbBackingStore::resize(const QSize &size, const QRegion &)
{
   if (m_image && size == m_image->size()) {
      return;
   }
   Q_XCB_NOOP(connection());


   QXcbScreen *screen = window()->screen() ? static_cast<QXcbScreen *>(window()->screen()->handle()) : nullptr;
   QPlatformWindow *pw = window()->handle();

   if (!pw) {
      window()->create();
      pw = window()->handle();

   }
   QXcbWindow *win = static_cast<QXcbWindow *>(pw);

   delete m_image;
   if (!screen) {
      m_image = nullptr;
      m_size = size;
      return;
   }

   m_image = new QXcbShmImage(screen, size, win->depth(), win->imageFormat());

   // Slow path for bgr888 VNC: Create an additional image, paint into that and
   // swap R and B while copying to m_image after each paint.
   if (win->imageNeedsRgbSwap()) {
      m_rgbImage = QImage(size, win->imageFormat());
   }
   Q_XCB_NOOP(connection());
}

extern void qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset);

bool QXcbBackingStore::scroll(const QRegion &area, int dx, int dy)
{
   if (! m_image || m_image->image()->isNull()) {
      return false;
   }

   m_image->preparePaint(area);

   QPoint delta(dx, dy);
   const QVector<QRect> rects = area.rects();
   for (int i = 0; i < rects.size(); ++i) {
      qt_scrollRectInImage(*m_image->image(), rects.at(i), delta);
   }
   return true;
}

