/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPIXMAP_RASTER_P_H
#define QPIXMAP_RASTER_P_H

#include <qpixmapdata_p.h>
#include <qpixmapdatafactory_p.h>

#ifdef Q_OS_WIN
# include "qt_windows.h"
#endif

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QRasterPixmapData : public QPixmapData
{
 public:
   QRasterPixmapData(PixelType type);
   ~QRasterPixmapData();

   QPixmapData *createCompatiblePixmapData() const;

   void resize(int width, int height);
   void fromFile(const QString &filename, Qt::ImageConversionFlags flags);
   bool fromData(const uchar *buffer, uint len, const char *format, Qt::ImageConversionFlags flags);
   void fromImage(const QImage &image, Qt::ImageConversionFlags flags);
   void fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags);

   void copy(const QPixmapData *data, const QRect &rect);
   bool scroll(int dx, int dy, const QRect &rect);
   void fill(const QColor &color);
   void setMask(const QBitmap &mask);
   bool hasAlphaChannel() const;
   void setAlphaChannel(const QPixmap &alphaChannel);
   QImage toImage() const;
   QImage toImage(const QRect &rect) const;
   QPaintEngine *paintEngine() const;
   QImage *buffer();

 protected:
   int metric(QPaintDevice::PaintDeviceMetric metric) const;
   void createPixmapForImage(QImage &sourceImage, Qt::ImageConversionFlags flags, bool inPlace);
   void setImage(const QImage &image);
   QImage image;

 private:
   friend class QPixmap;
   friend class QBitmap;
   friend class QPixmapCacheEntry;
   friend class QRasterPaintEngine;
};

QT_END_NAMESPACE

#endif // QPIXMAPDATA_RASTER_P_H


