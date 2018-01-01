/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qwindowsurface_mac_p.h>
#include <qt_mac_p.h>
#include <qt_cocoa_helpers_mac_p.h>
#include <qwidget.h>

QT_BEGIN_NAMESPACE

struct QMacWindowSurfacePrivate {
   QWidget *widget;
   QPixmap device;
};

QMacWindowSurface::QMacWindowSurface(QWidget *widget)
   : QWindowSurface(widget), d_ptr(new QMacWindowSurfacePrivate)
{
   d_ptr->widget = widget;
}

QMacWindowSurface::~QMacWindowSurface()
{
   delete d_ptr;
}

QPaintDevice *QMacWindowSurface::paintDevice()
{
   return &d_ptr->device;
}

void QMacWindowSurface::flush(QWidget *widget, const QRegion &rgn, const QPoint &offset)
{
   Q_UNUSED(offset);

   // Get a context for the widget
   extern CGContextRef qt_mac_graphicsContextFor(QWidget *);
   CGContextRef context = qt_mac_graphicsContextFor(widget);

   CGContextRetain(context);
   CGContextSaveGState(context);

   // Flip context.
   CGContextTranslateCTM(context, 0, widget->height());
   CGContextScaleCTM(context, 1, -1);

   // Clip to region.
   const QVector<QRect> &rects = rgn.rects();
   for (int i = 0; i < rects.size(); ++i) {
      const QRect &rect = rects.at(i);
      CGContextAddRect(context, CGRectMake(rect.x(), rect.y(), rect.width(), rect.height()));
   }
   CGContextClip(context);

   // Draw the image onto the window.
   const CGRect dest = CGRectMake(0, 0, widget->width(), widget->height());
   const CGImageRef image = d_ptr->device.toMacCGImageRef();
   qt_mac_drawCGImage(context, &dest, image);
   CFRelease(image);

   // Restore context
   CGContextRestoreGState(context);
   CGContextFlush(context);
   CGContextRelease(context);
}

void QMacWindowSurface::setGeometry(const QRect &rect)
{
   QWindowSurface::setGeometry(rect);
   const QSize size = rect.size();
   if (d_ptr->device.size() != size) {
      d_ptr->device = QPixmap(size);
   }
}

bool QMacWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
   if (d_ptr->device.size().isNull()) {
      return false;
   }

   QCFType<CGImageRef> image = d_ptr->device.toMacCGImageRef();
   const QRect rect(area.boundingRect());
   const CGRect dest = CGRectMake(rect.x(), rect.y(), rect.width(), rect.height());
   QCFType<CGImageRef> subimage = CGImageCreateWithImageInRect(image, dest);
   QCFType<CGContextRef> context = qt_mac_cg_context(&d_ptr->device);
   CGContextTranslateCTM(context, dx, dy);
   qt_mac_drawCGImage(context, &dest, subimage);
   return true;
}

QT_END_NAMESPACE
