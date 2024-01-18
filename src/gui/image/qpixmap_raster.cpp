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

#include <qpixmap.h>

#include <qpaintengine.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qbuffer.h>
#include <qimagereader.h>

#include <qfont_p.h>
#include <qimage_p.h>
#include <qnativeimage_p.h>
#include <qpixmap_raster_p.h>
#include <qimage_p.h>
#include <qsimd_p.h>
#include <qwidget_p.h>
#include <qdrawhelper_p.h>

QPixmap qt_toRasterPixmap(const QImage &image)
{
   QPlatformPixmap *data =
      new QRasterPlatformPixmap(image.depth() == 1
      ? QPlatformPixmap::BitmapType : QPlatformPixmap::PixmapType);

   data->fromImage(image, Qt::AutoColor);

   return QPixmap(data);
}

QPixmap qt_toRasterPixmap(const QPixmap &pixmap)
{
   if (pixmap.isNull()) {
      return QPixmap();
   }

   if (pixmap.classId() == QPlatformPixmap::RasterClass) {
      return pixmap;
   }

   return qt_toRasterPixmap(pixmap.toImage());
}

QRasterPlatformPixmap::QRasterPlatformPixmap(PixelType type)
   : QPlatformPixmap(type, RasterClass)
{
}

QRasterPlatformPixmap::~QRasterPlatformPixmap()
{
}

QPlatformPixmap *QRasterPlatformPixmap::createCompatiblePlatformPixmap() const
{
   return new QRasterPlatformPixmap(pixelType());
}

void QRasterPlatformPixmap::resize(int width, int height)
{
   QImage::Format format;

   if (pixelType() == BitmapType) {
      format = QImage::Format_MonoLSB;
   } else {
      format = QNativeImage::systemFormat();
   }

   image = QImage(width, height, format);
   w = width;
   h = height;
   d = image.depth();
   is_null = (w <= 0 || h <= 0);

   if (pixelType() == BitmapType && !image.isNull()) {
      image.setColorCount(2);
      image.setColor(0, QColor(Qt::color0).rgba());
      image.setColor(1, QColor(Qt::color1).rgba());
   }

   setSerialNumber(image.cacheKey() >> 32);
}

bool QRasterPlatformPixmap::fromData(const uchar *buffer, uint len, const QString &format,
   Qt::ImageConversionFlags flags)
{
   QByteArray a = QByteArray::fromRawData(reinterpret_cast<const char *>(buffer), len);

   QBuffer b(&a);
   b.open(QIODevice::ReadOnly);
   QImage image = QImageReader(&b, format).read();

   if (image.isNull()) {
      return false;
   }

   createPixmapForImage(image, flags, /* inplace = */true);
   return !isNull();
}

void QRasterPlatformPixmap::fromImage(const QImage &sourceImage, Qt::ImageConversionFlags flags)
{
   QImage image = sourceImage;
   createPixmapForImage(image, flags, /* inplace = */false);
}

void QRasterPlatformPixmap::fromImageInPlace(QImage &sourceImage, Qt::ImageConversionFlags flags)
{
   createPixmapForImage(sourceImage, flags, /* inplace = */true);
}

void QRasterPlatformPixmap::fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags)
{
   (void) flags;

   QImage image = imageReader->read();

   if (image.isNull()) {
      return;
   }

   createPixmapForImage(image, flags, /* inplace = */true);
}

// from qwindowsurface.cpp
extern void qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset);

void QRasterPlatformPixmap::copy(const QPlatformPixmap *data, const QRect &rect)
{
   fromImage(data->toImage(rect).copy(), Qt::NoOpaqueDetection);
}

bool QRasterPlatformPixmap::scroll(int dx, int dy, const QRect &rect)
{
   if (!image.isNull()) {
      qt_scrollRectInImage(image, rect, QPoint(dx, dy));
   }

   return true;
}

void QRasterPlatformPixmap::fill(const QColor &color)
{
   uint pixel;

   if (image.depth() == 1) {
      int gray = qGray(color.rgba());

      // Pick the best approximate color in the image's colortable.
      if (qAbs(qGray(image.color(0)) - gray) < qAbs(qGray(image.color(1)) - gray)) {
         pixel = 0;
      } else {
         pixel = 1;
      }

   } else if (image.depth() >= 15) {
      int alpha = color.alpha();

      if (alpha != 255) {
         if (! image.hasAlphaChannel()) {
            QImage::Format toFormat = qt_alphaVersionForPainting(image.format());

            if (!image.isNull() && qt_depthForFormat(image.format()) == qt_depthForFormat(toFormat)) {
               image.detach();
               image.d->format = toFormat;
            } else {
               image = QImage(image.width(), image.height(), toFormat);
            }
         }
      }

      pixel = qPremultiply(color.rgba());
      const QPixelLayout *layout = &qPixelLayouts[image.format()];
      layout->convertFromARGB32PM(&pixel, &pixel, 1, layout, nullptr);

   } else if (image.format() == QImage::Format_Alpha8) {
      pixel = qAlpha(color.rgba());

   } else if (image.format() == QImage::Format_Grayscale8) {
      pixel = qGray(color.rgba());

   } else {
      pixel = 0;

   }

   image.fill(pixel);
}

bool QRasterPlatformPixmap::hasAlphaChannel() const
{
   return image.hasAlphaChannel();
}

QImage QRasterPlatformPixmap::toImage() const
{
   if (! image.isNull()) {
      QImageData *data = const_cast<QImage &>(image).data_ptr();

      if (data->paintEngine && data->paintEngine->isActive() && data->paintEngine->paintDevice() == &image) {
         return image.copy();
      }
   }

   return image;
}

QImage QRasterPlatformPixmap::toImage(const QRect &rect) const
{
   if (rect.isNull()) {
      return image;
   }

   QRect clipped = rect.intersected(QRect(0, 0, w, h));
   const uint du = uint(d);

   if ((du % 8 == 0) && (((uint(clipped.x()) * du)) % 32 == 0)) {
      QImage newImage(image.scanLine(clipped.y()) + clipped.x() * (du / 8),
         clipped.width(), clipped.height(),
         image.bytesPerLine(), image.format());

      newImage.setDevicePixelRatio(image.devicePixelRatio());
      return newImage;

   } else {
      return image.copy(clipped);
   }
}

QPaintEngine *QRasterPlatformPixmap::paintEngine() const
{
   return image.paintEngine();
}

int QRasterPlatformPixmap::metric(QPaintDevice::PaintDeviceMetric metric) const
{
   QImageData *d = image.d;
   if (!d) {
      return 0;
   }

   // override the image dpi with the screen dpi when rendering to a pixmap
   switch (metric) {
      case QPaintDevice::PdmWidth:
         return w;

      case QPaintDevice::PdmHeight:
         return h;

      case QPaintDevice::PdmWidthMM:
         return qRound(d->width * 25.4 / qt_defaultDpiX());

      case QPaintDevice::PdmHeightMM:
         return qRound(d->height * 25.4 / qt_defaultDpiY());

      case QPaintDevice::PdmNumColors:
         return d->colortable.size();

      case QPaintDevice::PdmDepth:
         return this->d;

      case QPaintDevice::PdmDpiX:
         return qt_defaultDpiX();

      case QPaintDevice::PdmPhysicalDpiX:
         return qt_defaultDpiX();

      case QPaintDevice::PdmDpiY:
         return qt_defaultDpiX();

      case QPaintDevice::PdmPhysicalDpiY:
         return qt_defaultDpiY();

      case QPaintDevice::PdmDevicePixelRatio:
         return image.devicePixelRatio();

      case QPaintDevice::PdmDevicePixelRatioScaled:
         return image.devicePixelRatio() * QPaintDevice::devicePixelRatioFScale();

      default:
         qWarning("QRasterPlatformPixmap::metric() Unhandled metric type %d", metric);
         break;
   }

   return 0;
}

void QRasterPlatformPixmap::createPixmapForImage(QImage &sourceImage, Qt::ImageConversionFlags flags, bool inPlace)
{
   QImage::Format format;

   if (flags & Qt::NoFormatConversion) {
      format = sourceImage.format();
   } else

      if (pixelType() == BitmapType) {
         format = QImage::Format_MonoLSB;

      } else {
         if (sourceImage.depth() == 1) {
            format = sourceImage.hasAlphaChannel()
               ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32;

         } else {
            QImage::Format opaqueFormat = QNativeImage::systemFormat();
            QImage::Format alphaFormat = qt_alphaVersionForPainting(opaqueFormat);

            if (! sourceImage.hasAlphaChannel()) {
               format = opaqueFormat;

            } else if ((flags & Qt::NoOpaqueDetection) == 0
                  && ! const_cast<QImage &>(sourceImage).data_ptr()->checkForAlphaPixels()) {

               // image has alpha format but is really opaque, so try to do a
               // more efficient conversion

               format = opaqueFormat;
            } else {
               format = alphaFormat;
            }
         }
      }


   if (format == QImage::Format_RGB32 && (sourceImage.format() == QImage::Format_ARGB32
         || sourceImage.format() == QImage::Format_ARGB32_Premultiplied)) {

      inPlace = inPlace && sourceImage.isDetached();
      image = sourceImage;
      if (!inPlace) {
         image.detach();
      }

      if (image.d) {
         image.d->format = QImage::Format_RGB32;
      }

   } else if (inPlace && sourceImage.d->convertInPlace(format, flags)) {
      image = sourceImage;

   } else {
      image = sourceImage.convertToFormat(format);
   }

   if (image.d) {
      w = image.d->width;
      h = image.d->height;
      d = image.d->depth;
   } else {
      w = h = d = 0;
   }
   is_null = (w <= 0 || h <= 0);

   if (image.d) {
      image.d->devicePixelRatio = sourceImage.devicePixelRatio();
   }

   //ensure the pixmap and the image resulting from toImage() have the same cacheKey();
   setSerialNumber(image.cacheKey() >> 32);

   if (image.d) {
      setDetachNumber(image.d->detach_no);
   }
}

QImage *QRasterPlatformPixmap::buffer()
{
   return &image;
}

qreal QRasterPlatformPixmap::devicePixelRatio() const
{
   return image.devicePixelRatio();
}

void QRasterPlatformPixmap::setDevicePixelRatio(qreal scaleFactor)
{
   image.setDevicePixelRatio(scaleFactor);
}
