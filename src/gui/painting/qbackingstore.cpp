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

#include <qbackingstore.h>

#include <qdebug.h>
#include <qpixmap.h>
#include <qplatform_backingstore.h>
#include <qplatform_integration.h>
#include <qscopedpointer.h>
#include <qscreen.h>
#include <qwindow.h>

#include <qguiapplication_p.h>
#include <qhighdpiscaling_p.h>
#include <qwindow_p.h>

class QBackingStorePrivate
{
 public:
   QBackingStorePrivate(QWindow *w)
      : window(w) {
   }

   QWindow *window;
   QPlatformBackingStore *platformBackingStore;
   QScopedPointer<QImage> highDpiBackingstore;
   QRegion staticContents;
   QSize size;
};

void QBackingStore::flush(const QRegion &region, QWindow *win, const QPoint &offset)
{
   if (! win) {
      win = window();
   }

   if (! win->handle()) {
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QBackingStore::flush() Unable to flush a window without a handle");
#endif
      return;
   }

   if (win && win->isTopLevel() && ! qt_window_private(win)->receivedExpose) {
#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
      qDebug("QBackingStore::flush() Unable to flush a non-exposed window");
#endif
   }

   d_ptr->platformBackingStore->flush(win, QHighDpi::toNativeLocalRegion(region, win),
      QHighDpi::toNativeLocalPosition(offset, win));
}

QPaintDevice *QBackingStore::paintDevice()
{
   QPaintDevice *device = d_ptr->platformBackingStore->paintDevice();

   if (QHighDpiScaling::isActive() && device->devType() == QInternal::Image) {
      return d_ptr->highDpiBackingstore.data();
   }

   return device;
}

QBackingStore::QBackingStore(QWindow *window)
   : d_ptr(new QBackingStorePrivate(window))
{
   d_ptr->platformBackingStore = QGuiApplicationPrivate::platformIntegration()->createPlatformBackingStore(window);
}

QBackingStore::~QBackingStore()
{
   delete d_ptr->platformBackingStore;
}

QWindow *QBackingStore::window() const
{
   return d_ptr->window;
}

void QBackingStore::beginPaint(const QRegion &region)
{
   if (d_ptr->highDpiBackingstore &&
      d_ptr->highDpiBackingstore->devicePixelRatio() != d_ptr->window->devicePixelRatio()) {
      resize(size());
   }

   d_ptr->platformBackingStore->beginPaint(QHighDpi::toNativeLocalRegion(region, d_ptr->window));

   // When QtGui is applying a high-dpi scale factor the backing store
   // creates a "large" backing store image. This image needs to be
   // painted on as a high-dpi image, which is done by setting
   // devicePixelRatio. Do this on a separate image instance that shares
   // the image data to avoid having the new devicePixelRatio be propagated
   // back to the platform plugin.

   QPaintDevice *device = d_ptr->platformBackingStore->paintDevice();
   if (QHighDpiScaling::isActive() && device->devType() == QInternal::Image) {
      QImage *source = static_cast<QImage *>(device);

      const bool needsNewImage = d_ptr->highDpiBackingstore.isNull()
         || source->data_ptr() != d_ptr->highDpiBackingstore->data_ptr()
         || source->size() != d_ptr->highDpiBackingstore->size()
         || source->devicePixelRatio() != d_ptr->highDpiBackingstore->devicePixelRatio();

      if (needsNewImage) {

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
         qDebug() << "QBackingStore::beginPaint() window =" << d_ptr->window << "\n  "
                  << "Source size =" << source->size() << " DP ratio =" << source->devicePixelRatio();
#endif

         d_ptr->highDpiBackingstore.reset(
            new QImage(source->bits(), source->width(), source->height(), source->bytesPerLine(), source->format()));

         qreal targetDevicePixelRatio = d_ptr->window->devicePixelRatio();
         d_ptr->highDpiBackingstore->setDevicePixelRatio(targetDevicePixelRatio);

#if defined(CS_SHOW_DEBUG_GUI_PAINTING)
         qDebug() << "   Destination Size =" << d_ptr->highDpiBackingstore->size()
                  << "DP ratio =" << targetDevicePixelRatio;
#endif
      }
   }
}

void QBackingStore::endPaint()
{
   d_ptr->platformBackingStore->endPaint();
}

void QBackingStore::resize(const QSize &size)
{
   d_ptr->size = size;
   d_ptr->platformBackingStore->resize(QHighDpi::toNativePixels(size, d_ptr->window), d_ptr->staticContents);
}

QSize QBackingStore::size() const
{
   return d_ptr->size;
}

bool QBackingStore::scroll(const QRegion &area, int dx, int dy)
{
   // Disable scrolling for non-integer scroll deltas. For this case
   // the the existing rendered pixels can't be re-used, and we return
   // false to signal that a repaint is needed.
   const qreal nativeDx = QHighDpi::toNativePixels(qreal(dx), d_ptr->window);
   const qreal nativeDy = QHighDpi::toNativePixels(qreal(dy), d_ptr->window);
   if (qFloor(nativeDx) != nativeDx || qFloor(nativeDy) != nativeDy) {
      return false;
   }

   return d_ptr->platformBackingStore->scroll(QHighDpi::toNativeLocalRegion(area, d_ptr->window),
         nativeDx, nativeDy);
}

void QBackingStore::setStaticContents(const QRegion &region)
{
   d_ptr->staticContents = region;
}

QRegion QBackingStore::staticContents() const
{
   return d_ptr->staticContents;
}

bool QBackingStore::hasStaticContents() const
{
   return !d_ptr->staticContents.isEmpty();
}

void Q_GUI_EXPORT qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset)
{
   // make sure we do not detach
   uchar *mem = const_cast<uchar *>(const_cast<const QImage &>(img).bits());

   int lineskip = img.bytesPerLine();
   int depth = img.depth() >> 3;

   const QRect imageRect(0, 0, img.width(), img.height());
   const QRect r = rect & imageRect & imageRect.translated(-offset);
   const QPoint p = rect.topLeft() + offset;

   if (r.isEmpty()) {
      return;
   }

   const uchar *src;
   uchar *dest;

   if (r.top() < p.y()) {
      src = mem + r.bottom() * lineskip + r.left() * depth;
      dest = mem + (p.y() + r.height() - 1) * lineskip + p.x() * depth;
      lineskip = -lineskip;
   } else {
      src = mem + r.top() * lineskip + r.left() * depth;
      dest = mem + p.y() * lineskip + p.x() * depth;
   }

   const int w = r.width();
   int h = r.height();
   const int bytes = w * depth;

   // overlapping segments?
   if (offset.y() == 0 && qAbs(offset.x()) < w) {
      do {
         ::memmove(dest, src, bytes);
         dest += lineskip;
         src += lineskip;
      } while (--h);
   } else {
      do {
         ::memcpy(dest, src, bytes);
         dest += lineskip;
         src += lineskip;
      } while (--h);
   }
}

QPlatformBackingStore *QBackingStore::handle() const
{
   return d_ptr->platformBackingStore;
}

