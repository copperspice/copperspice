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

#include <qrasterwindow.h>

#include <QBackingStore>
#include <QPainter>

#include <qpaintdevicewindow_p.h>

class QRasterWindowPrivate : public QPaintDeviceWindowPrivate
{
   Q_DECLARE_PUBLIC(QRasterWindow)

 public:
   void beginPaint(const QRegion &region) override {
      Q_Q(QRasterWindow);
      if (backingstore->size() != q->size()) {
         backingstore->resize(q->size());
         markWindowAsDirty();
      }
      backingstore->beginPaint(region);
   }

   void endPaint() override {
      backingstore->endPaint();
   }

   void flush(const QRegion &region) override {
      Q_Q(QRasterWindow);
      backingstore->flush(region, q);
   }

   QScopedPointer<QBackingStore> backingstore;
};

/*!
  Constructs a new QRasterWindow with \a parent.
*/
QRasterWindow::QRasterWindow(QWindow *parent)
   : QPaintDeviceWindow(* (new QRasterWindowPrivate), parent)
{
   setSurfaceType(QSurface::RasterSurface);
   d_func()->backingstore.reset(new QBackingStore(this));
}

/*!
  \internal
*/
int QRasterWindow::metric(PaintDeviceMetric metric) const
{
   Q_D(const QRasterWindow);

   switch (metric) {
      case PdmDepth:
         return d->backingstore->paintDevice()->depth();
      default:
         break;
   }
   return QPaintDeviceWindow::metric(metric);
}

/*!
  \internal
*/
QPaintDevice *QRasterWindow::redirected(QPoint *) const
{
   Q_D(const QRasterWindow);
   return d->backingstore->paintDevice();
}

