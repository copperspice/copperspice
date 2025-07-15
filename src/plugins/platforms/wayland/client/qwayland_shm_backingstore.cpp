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

#include <errno.h>
#include <sys/mman.h>

namespace QtWaylandClient {

QWaylandShmBuffer::QWaylandShmBuffer(QWaylandDisplay *display, const QSize &size, QImage::Format format, int scale)
   : mShmPool(nullptr), mMarginsImage(nullptr)
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

   // pending implementation
}

QWaylandShmBuffer::~QWaylandShmBuffer()
{
   delete mMarginsImage;

   if (mImage.constBits()) {
      munmap((void *) mImage.constBits(), mImage.byteCount());
   }

   if (mShmPool) {
      wl_shm_pool_destroy(mShmPool);
   }
}

QImage *QWaylandShmBuffer::imageInsideMargins(const QMargins &marginsIn)
{
   QMargins margins = marginsIn * int(mImage.devicePixelRatio());

   if (! margins.isNull() && margins != mMargins) {
      if (mMarginsImage) {
         delete mMarginsImage;
      }

      uchar *bits     = const_cast < uchar * > (mImage.constBits());
      uchar *b_s_data = bits + margins.top() * mImage.bytesPerLine() + margins.left() * 4;
      int b_s_width   = mImage.size().width() - margins.left() - margins.right();
      int b_s_height  = mImage.size().height() - margins.top() - margins.bottom();

      mMarginsImage   = new QImage(b_s_data, b_s_width, b_s_height, mImage.bytesPerLine(), mImage.format());
      mMarginsImage->setDevicePixelRatio(mImage.devicePixelRatio());
   }

   if (margins.isNull()) {
      delete mMarginsImage;
      mMarginsImage = nullptr;
   }

   mMargins = margins;

   if (! mMarginsImage) {
      return &mImage;
   }

   return mMarginsImage;
}

// **
QWaylandShmBackingStore::QWaylandShmBackingStore(QWindow *window)
   : QPlatformBackingStore(window), mPainting(false), mPendingFlush(false),
     mFrontBuffer(nullptr), mBackBuffer(nullptr)
{
}

QWaylandShmBackingStore::~QWaylandShmBackingStore()
{
   qDeleteAll(mBuffers);
}

void QWaylandShmBackingStore::beginPaint(const QRegion &region)
{
   mPainting = true;
   ensureSize();

   // pending implementation
}

QImage *QWaylandShmBackingStore::contentSurface() const
{
   return windowDecoration() ? mBackBuffer->imageInsideMargins(windowDecorationMargins()) : mBackBuffer->image();
}

QImage *QWaylandShmBackingStore::entireSurface() const
{
   return mBackBuffer->image();
}

void QWaylandShmBackingStore::endPaint()
{
   mPainting = false;

   // pending implementation
}

void QWaylandShmBackingStore::ensureSize()
{
   // pending implementation
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

   if (mPainting) {
      mPendingRegion |= region;
      mPendingFlush = true;

      return;
   }

   mPendingFlush  = false;
   mPendingRegion = QRegion();
   if (windowDecoration() && windowDecoration()->isDirty()) {
      updateDecorations();
   }

   mFrontBuffer = mBackBuffer;

   // pending implementation
}

QWaylandShmBuffer *QWaylandShmBackingStore::getBuffer(const QSize &size)
{
   auto tmpBuffers = mBuffers;

   for (QWaylandShmBuffer *b : tmpBuffers) {
      if (! b->busy()) {
         if (b->size() == size) {
            return b;
         } else {
            mBuffers.removeOne(b);

            if (mBackBuffer == b) {
               mBackBuffer = nullptr;
            }

            delete b;
         }
      }
   }

   // pending implementation

   return nullptr;
}

QPaintDevice *QWaylandShmBackingStore::paintDevice()
{
   return contentSurface();
}

void QWaylandShmBackingStore::resize(const QSize &size)
{
   QMargins margins = windowDecorationMargins();

   // pending implementation
   int oldSize = 0;

   if (mBackBuffer != nullptr) {
      oldSize = mBackBuffer->image()->byteCount();
   }

   // pending implementation
}

void QWaylandShmBackingStore::resize(const QSize &size, const QRegion &)
{
   mRequestedSize = size;
}

void QWaylandShmBackingStore::updateDecorations()
{
   QPainter decorationPainter(entireSurface());
   decorationPainter.setCompositionMode(QPainter::CompositionMode_Source);
   QImage sourceImage = windowDecoration()->contentImage();

   qreal dp = sourceImage.devicePixelRatio();
   int dpWidth = int(sourceImage.width() / dp);
   int dpHeight = int(sourceImage.height() / dp);

   QMatrix sourceMatrix;
   sourceMatrix.scale(dp, dp);

   QRect target; // needs to be in device independent pixels

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
   // pending implementation

   return nullptr;
}

QWaylandAbstractDecoration *QWaylandShmBackingStore::windowDecoration() const
{
   // pending implementation

   return nullptr;
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
