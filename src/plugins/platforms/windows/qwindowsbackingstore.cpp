/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qwindowsbackingstore.h>

#include <qdebug.h>
#include <qwindow.h>
#include <qpainter.h>
#include <qwindowswindow.h>
#include <qwindowsnativeimage.h>
#include <qwindowscontext.h>

#include <qhighdpiscaling_p.h>
#include <qimage_p.h>

QWindowsBackingStore::QWindowsBackingStore(QWindow *window)
   : QPlatformBackingStore(window), m_alphaNeedsFill(false)
{
}

QWindowsBackingStore::~QWindowsBackingStore()
{
}

QPaintDevice *QWindowsBackingStore::paintDevice()
{
   Q_ASSERT(!m_image.isNull());
   return &m_image->image();
}

void QWindowsBackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
   Q_ASSERT(window);

   const QRect br = region.boundingRect();

   if (QWindowsContext::verbose > 1) {
      qDebug() << "QWindowsBackingStore::flush():" << this << window << offset << br;
   }

   QWindowsWindow *rw = QWindowsWindow::baseWindowOf(window);

   const bool hasAlpha = rw->format().hasAlpha();
   const Qt::WindowFlags flags = window->flags();

   if ((flags & Qt::FramelessWindowHint) && QWindowsWindow::setWindowLayered(rw->handle(), flags, hasAlpha, rw->opacity()) && hasAlpha) {
      // Windows with alpha: Use blend function to update.
      QRect r = QHighDpi::toNativePixels(window->frameGeometry(), window);
      QPoint frameOffset(QHighDpi::toNativePixels(QPoint(window->frameMargins().left(), window->frameMargins().top()),
            static_cast<const QWindow *>(nullptr)));
      QRect dirtyRect = br.translated(offset + frameOffset);

      SIZE size = {r.width(), r.height()};
      POINT ptDst = {r.x(), r.y()};
      POINT ptSrc = {0, 0};
      BLENDFUNCTION blend = {AC_SRC_OVER, 0, BYTE(qRound(255.0 * rw->opacity())), AC_SRC_ALPHA};
      if (QWindowsContext::user32dll.updateLayeredWindowIndirect) {
         RECT dirty = {dirtyRect.x(), dirtyRect.y(),
                 dirtyRect.x() + dirtyRect.width(), dirtyRect.y() + dirtyRect.height()
              };
         UPDATELAYEREDWINDOWINFO info = {sizeof(info), NULL, &ptDst, &size, m_image->hdc(), &ptSrc, 0, &blend, ULW_ALPHA, &dirty};
         const BOOL result = QWindowsContext::user32dll.updateLayeredWindowIndirect(rw->handle(), &info);
         if (!result)
            qErrnoWarning("UpdateLayeredWindowIndirect failed for ptDst=(%d, %d),"
               " size=(%dx%d), dirty=(%dx%d %d, %d)", r.x(), r.y(),
               r.width(), r.height(), dirtyRect.width(), dirtyRect.height(),
               dirtyRect.x(), dirtyRect.y());
      } else {
         QWindowsContext::user32dll.updateLayeredWindow(rw->handle(), NULL, &ptDst, &size, m_image->hdc(), &ptSrc, 0, &blend, ULW_ALPHA);
      }
   } else {

      const HDC dc = rw->getDC();
      if (!dc) {
         qErrnoWarning("%s: GetDC failed", __FUNCTION__);
         return;
      }

      if (!BitBlt(dc, br.x(), br.y(), br.width(), br.height(),
            m_image->hdc(), br.x() + offset.x(), br.y() + offset.y(), SRCCOPY)) {
         const DWORD lastError = GetLastError(); // QTBUG-35926, QTBUG-29716: may fail after lock screen.
         if (lastError != ERROR_SUCCESS && lastError != ERROR_INVALID_HANDLE) {
            qErrnoWarning(int(lastError), "%s: BitBlt failed", __FUNCTION__);
         }
      }

      rw->releaseDC();
   }

   // Write image for debug purposes.
   if (QWindowsContext::verbose > 2) {
      static int n = 0;
      const QString fileName = QString::fromLatin1("win%1_%2.png").formatArg(rw->winId()).formatArg(n++);
      m_image->image().save(fileName);

#if defined(CS_SHOW_DEBUG)
      qDebug() << "Wrote =" << m_image->image().size() << fileName;
#endif

   }
}

void QWindowsBackingStore::resize(const QSize &size, const QRegion &region)
{
   if (m_image.isNull() || m_image->image().size() != size) {

#if defined(CS_SHOW_DEBUG)
      if (QWindowsContext::verbose) {
         qDebug() << "QWindowsBackingStore::resize(): " << window() << ' ' << size << ' ' << region
                  << " from: " << (m_image.isNull() ? QSize() : m_image->image().size());
      }
#endif

      QImage::Format format = window()->format().hasAlpha() ?
         QImage::Format_ARGB32_Premultiplied : QWindowsNativeImage::systemFormat();

      // The backingstore composition (enabling render-to-texture widgets) punches holes in the
      // backingstores using the alpha channel. Hence the need for a true alpha format.

      if (QImage::toPixelFormat(format).alphaUsage() == QPixelFormat::UsesAlpha) {
         m_alphaNeedsFill = true;
      } else {
         // upgrade but here we know app painting does not rely on alpha hence no need to fill
         format = qt_maybeAlphaVersionWithSameDepth(format);
      }

      QWindowsNativeImage *oldwni = m_image.data();
      QWindowsNativeImage *newwni = new QWindowsNativeImage(size.width(), size.height(), format);

      if (oldwni && !region.isEmpty()) {
         const QImage &oldimg(oldwni->image());

         QImage &newimg(newwni->image());
         QRegion staticRegion(region);
         staticRegion &= QRect(0, 0, oldimg.width(), oldimg.height());
         staticRegion &= QRect(0, 0, newimg.width(), newimg.height());

         QPainter painter(&newimg);
         painter.setCompositionMode(QPainter::CompositionMode_Source);

         for (const QRect &rect : staticRegion.rects()) {
            painter.drawImage(rect, oldimg, rect);
         }
      }

      m_image.reset(newwni);
   }
}

Q_GUI_EXPORT void qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset);

bool QWindowsBackingStore::scroll(const QRegion &area, int dx, int dy)
{
   if (m_image.isNull() || m_image->image().isNull()) {
      return false;
   }

   const QVector<QRect> rects = area.rects();
   const QPoint offset(dx, dy);
   for (int i = 0; i < rects.size(); ++i) {
      qt_scrollRectInImage(m_image->image(), rects.at(i), offset);
   }

   return true;
}

void QWindowsBackingStore::beginPaint(const QRegion &region)
{
   if (QWindowsContext::verbose > 1) {
      qDebug() << __FUNCTION__ << region;
   }

   if (m_alphaNeedsFill) {
      QPainter p(&m_image->image());
      p.setCompositionMode(QPainter::CompositionMode_Source);
      const QColor blank = Qt::transparent;

      for (const QRect &r : region.rects()) {
         p.fillRect(r, blank);
      }
   }
}

HDC QWindowsBackingStore::getDC() const
{
   if ( !m_image.isNull()) {
      return m_image->hdc();
   }
   return 0;
}

#ifndef QT_NO_OPENGL
QImage QWindowsBackingStore::toImage() const
{
   if (m_image.isNull()) {
      qWarning() << "QWindowsBackingStore::toImage(): Image is null";
      return QImage();
   }

   return m_image.data()->image();
}
#endif


