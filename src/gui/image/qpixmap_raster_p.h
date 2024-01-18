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

#ifndef QPIXMAP_RASTER_P_H
#define QPIXMAP_RASTER_P_H

#include <qplatform_pixmap.h>

class Q_GUI_EXPORT QRasterPlatformPixmap : public QPlatformPixmap
{
 public:
   QRasterPlatformPixmap(PixelType type);
   ~QRasterPlatformPixmap();

   QPlatformPixmap *createCompatiblePlatformPixmap() const override;

   void resize(int width, int height) override;

   bool fromData(const uchar *buffer, uint len, const QString &format, Qt::ImageConversionFlags flags) override;
   void fromImage(const QImage &image, Qt::ImageConversionFlags flags) override;

   void fromImageInPlace(QImage &image, Qt::ImageConversionFlags flags) override;
   void fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags) override;

   void copy(const QPlatformPixmap *data, const QRect &rect) override;
   bool scroll(int dx, int dy, const QRect &rect) override;
   void fill(const QColor &color) override;

   bool hasAlphaChannel() const override;

   QImage toImage() const override;
   QImage toImage(const QRect &rect) const override;
   QPaintEngine *paintEngine() const override;

   QImage *buffer() override;
   qreal devicePixelRatio() const override;
   void setDevicePixelRatio(qreal scaleFactor) override;

 protected:
   int metric(QPaintDevice::PaintDeviceMetric metric) const override;
   void createPixmapForImage(QImage &sourceImage, Qt::ImageConversionFlags flags, bool inPlace);
   void setImage(const QImage &image);
   QImage image;

 private:
   friend class QPixmap;
   friend class QBitmap;
   friend class QPixmapCacheEntry;
   friend class QRasterPaintEngine;
};

#endif

