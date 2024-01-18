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

#include <qcocoabackingstore.h>
#include <QPainter>
#include <qcocoahelpers.h>

QCocoaBackingStore::QCocoaBackingStore(QWindow *window)
   : QPlatformBackingStore(window)
{
}

QCocoaBackingStore::~QCocoaBackingStore()
{
   if (QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window()->handle())) {
      [cocoaWindow->m_qtView clearBackingStore: this];
   }
}

QPaintDevice *QCocoaBackingStore::paintDevice()
{
   QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window()->handle());
   int windowDevicePixelRatio = int(cocoaWindow->devicePixelRatio());

   // Receate the backing store buffer if the effective buffer size has changed,
   // either due to a window resize or devicePixelRatio change.
   QSize effectiveBufferSize = m_requestedSize * windowDevicePixelRatio;
   if (m_qImage.size() != effectiveBufferSize) {
      QImage::Format format = (window()->format().hasAlpha() || cocoaWindow->m_drawContentBorderGradient)
         ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32;
      m_qImage = QImage(effectiveBufferSize, format);
      m_qImage.setDevicePixelRatio(windowDevicePixelRatio);
      if (format == QImage::Format_ARGB32_Premultiplied) {
         m_qImage.fill(Qt::transparent);
      }
   }
   return &m_qImage;
}

void QCocoaBackingStore::flush(QWindow *win, const QRegion &region, const QPoint &offset)
{
   if (!m_qImage.isNull()) {
      if (QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(win->handle())) {
         [cocoaWindow->m_qtView flushBackingStore: this region: region offset: offset];
      }
   }
}

QImage QCocoaBackingStore::toImage() const
{
   return m_qImage;
}

void QCocoaBackingStore::resize(const QSize &size, const QRegion &)
{
   m_requestedSize = size;
}

bool QCocoaBackingStore::scroll(const QRegion &area, int dx, int dy)
{
   extern void qt_scrollRectInImage(QImage & img, const QRect & rect, const QPoint & offset);
   const qreal devicePixelRatio = m_qImage.devicePixelRatio();
   QPoint qpoint(dx * devicePixelRatio, dy * devicePixelRatio);
   const QVector<QRect> qrects = area.rects();
   for (int i = 0; i < qrects.count(); ++i) {
      const QRect &qrect = QRect(qrects.at(i).topLeft() * devicePixelRatio, qrects.at(i).size() * devicePixelRatio);
      qt_scrollRectInImage(m_qImage, qrect, qpoint);
   }
   return true;
}

void QCocoaBackingStore::beginPaint(const QRegion &region)
{
   if (m_qImage.hasAlphaChannel()) {
      QPainter p(&m_qImage);
      p.setCompositionMode(QPainter::CompositionMode_Source);
      const QVector<QRect> rects = region.rects();
      const QColor blank = Qt::transparent;
      for (QVector<QRect>::const_iterator it = rects.begin(), end = rects.end(); it != end; ++it) {
         p.fillRect(*it, blank);
      }
   }
}

qreal QCocoaBackingStore::getBackingStoreDevicePixelRatio()
{
   return m_qImage.devicePixelRatio();
}
