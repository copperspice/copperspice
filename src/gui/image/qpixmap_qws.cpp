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

#include <qpixmap.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qdesktopwidget.h>
#include <qscreen_qws.h>
#include <qwsdisplay_qws.h>
#include <qdrawhelper_p.h>
#include <qpixmap_raster_p.h>

QT_BEGIN_NAMESPACE

QPixmap QPixmap::grabWindow(WId window, int x, int y, int w, int h)
{
   QWidget *widget = QWidget::find(window);
   if (!widget) {
      return QPixmap();
   }

   QRect grabRect = widget->frameGeometry();
   if (!widget->isWindow()) {
      grabRect.translate(widget->parentWidget()->mapToGlobal(QPoint()));
   }
   if (w < 0) {
      w = widget->width() - x;
   }
   if (h < 0) {
      h = widget->height() - y;
   }
   grabRect &= QRect(x, y, w, h).translated(widget->mapToGlobal(QPoint()));

   QScreen *screen = qt_screen;
   QDesktopWidget *desktop = QApplication::desktop();
   if (!desktop) {
      return QPixmap();
   }
   if (desktop->numScreens() > 1) {
      const int screenNo = desktop->screenNumber(widget);
      if (screenNo != -1) {
         screen = qt_screen->subScreens().at(screenNo);
      }
      grabRect = grabRect.translated(-screen->region().boundingRect().topLeft());
   }

   if (screen->pixelFormat() == QImage::Format_Invalid) {
      qWarning("QPixmap::grabWindow(): Unable to copy pixels from framebuffer");
      return QPixmap();
   }

   if (screen->isTransformed()) {
      const QSize screenSize(screen->width(), screen->height());
      grabRect = screen->mapToDevice(grabRect, screenSize);
   }

   QWSDisplay::grab(false);
   QPixmap pixmap;
   QImage img(screen->base(),
              screen->deviceWidth(), screen->deviceHeight(),
              screen->linestep(), screen->pixelFormat());
   img = img.copy(grabRect);
   QWSDisplay::ungrab();

   if (screen->isTransformed()) {
      QMatrix matrix;
      switch (screen->transformOrientation()) {
         case 1:
            matrix.rotate(90);
            break;
         case 2:
            matrix.rotate(180);
            break;
         case 3:
            matrix.rotate(270);
            break;
         default:
            break;
      }
      img = img.transformed(matrix);
   }

   if (screen->pixelType() == QScreen::BGRPixel) {
      img = img.rgbSwapped();
   }

   return QPixmap::fromImage(img);
}

QRgb *QPixmap::clut() const
{
   if (data && data->classId() == QPixmapData::RasterClass) {
      const QRasterPixmapData *d = static_cast<const QRasterPixmapData *>(data.data());
      return d->image.colorTable().data();
   }

   return 0;
}

int QPixmap::numCols() const
{
   return colorCount();
}

int QPixmap::colorCount() const
{
   if (data && data->classId() == QPixmapData::RasterClass) {
      const QRasterPixmapData *d = static_cast<const QRasterPixmapData *>(data.data());
      return d->image.colorCount();
   }

   return 0;
}

const uchar *QPixmap::qwsBits() const
{
   if (data && data->classId() == QPixmapData::RasterClass) {
      const QRasterPixmapData *d = static_cast<const QRasterPixmapData *>(data.data());
      return d->image.bits();
   }

   return 0;
}

int QPixmap::qwsBytesPerLine() const
{
   if (data && data->classId() == QPixmapData::RasterClass) {
      const QRasterPixmapData *d = static_cast<const QRasterPixmapData *>(data.data());
      return d->image.bytesPerLine();
   }

   return 0;
}

QT_END_NAMESPACE
