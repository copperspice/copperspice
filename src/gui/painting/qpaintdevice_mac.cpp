/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qpaintdevice.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qbitmap.h>
#include <qapplication.h>
#include <qprinter.h>
#include <qdebug.h>
#include <qt_mac_p.h>
#include <qprintengine_mac_p.h>
#include <qpixmap_mac_p.h>
#include <qpixmap_raster_p.h>

QT_BEGIN_NAMESPACE

/*! \internal */
float qt_mac_defaultDpi_x()
{
   // Mac OS X currently assumes things to be 72 dpi.
   // (see http://developer.apple.com/releasenotes/GraphicsImaging/RN-ResolutionIndependentUI/)
   // This may need to be re-worked as we go further in the resolution-independence stuff.
   return 72;
}

/*! \internal */
float qt_mac_defaultDpi_y()
{
   // Mac OS X currently assumes things to be 72 dpi.
   // (see http://developer.apple.com/releasenotes/GraphicsImaging/RN-ResolutionIndependentUI/)
   // This may need to be re-worked as we go further in the resolution-independence stuff.
   return 72;
}


/*! \internal

    Returns the QuickDraw CGrafPtr of the paint device. 0 is returned
    if it can't be obtained. Do not hold the pointer around for long
    as it can be relocated.

    \warning This function is only available on Mac OS X.
*/

Q_GUI_EXPORT GrafPtr qt_mac_qd_context(const QPaintDevice *device)
{
   if (device->devType() == QInternal::Pixmap) {
      return static_cast<GrafPtr>(static_cast<const QPixmap *>(device)->macQDHandle());
   } else if (device->devType() == QInternal::Widget) {
      return static_cast<GrafPtr>(static_cast<const QWidget *>(device)->macQDHandle());
   } else if (device->devType() == QInternal::Printer) {
      QPaintEngine *engine = static_cast<const QPrinter *>(device)->paintEngine();
      return static_cast<GrafPtr>(static_cast<const QMacPrintEngine *>(engine)->handle());
   }
   return 0;
}

extern CGColorSpaceRef qt_mac_colorSpaceForDeviceType(const QPaintDevice *pdev);


Q_GUI_EXPORT CGContextRef qt_mac_cg_context(const QPaintDevice *pdev)
{
   if (pdev->devType() == QInternal::Pixmap) {
      const QPixmap *pm = static_cast<const QPixmap *>(pdev);
      CGColorSpaceRef colorspace = qt_mac_colorSpaceForDeviceType(pdev);
      uint flags = kCGImageAlphaPremultipliedFirst;

#ifdef kCGBitmapByteOrder32Host //only needed because CGImage.h added symbols in the minor version
      flags |= kCGBitmapByteOrder32Host;
#endif
      CGContextRef ret = 0;

      // It would make sense to put this into a mac #ifdef'ed
      // virtual function in the QPixmapData at some point
      if (pm->data->classId() == QPixmapData::MacClass) {
         const QMacPixmapData *pmData = static_cast<const QMacPixmapData *>(pm->data.data());
         ret = CGBitmapContextCreate(pmData->pixels, pmData->w, pmData->h,
                                     8, pmData->bytesPerRow, colorspace,
                                     flags);
         if (!ret)
            qWarning("QPaintDevice: Unable to create context for pixmap (%d/%d/%d)",
                     pmData->w, pmData->h, (pmData->bytesPerRow * pmData->h));
      } else if (pm->data->classId() == QPixmapData::RasterClass) {
         QImage *image = pm->data->buffer();
         ret = CGBitmapContextCreate(image->bits(), image->width(), image->height(),
                                     8, image->bytesPerLine(), colorspace, flags);
      }

      CGContextTranslateCTM(ret, 0, pm->height());
      CGContextScaleCTM(ret, 1, -1);
      return ret;

   } else if (pdev->devType() == QInternal::Widget) {
      CGContextRef ret = static_cast<CGContextRef>(static_cast<const QWidget *>(pdev)->macCGHandle());
      CGContextRetain(ret);
      return ret;

   } else if (pdev->devType() == QInternal::MacQuartz) {
      return static_cast<const QMacQuartzPaintDevice *>(pdev)->cgContext();
   }
   return 0;
}

QT_END_NAMESPACE
