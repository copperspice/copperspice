/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qpixmap_raster_p.h>

#include <qbitmap.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qimagereader.h>
#include <qpaintengine.h>
#include <qpixmap.h>

#include <qdrawhelper_p.h>
#include <qfont_p.h>
#include <qimage_p.h>
#include <qimage_p.h>
#include <qnativeimage_p.h>
#include <qsimd_p.h>
#include <qwidget_p.h>

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

   m_rasterImage = QImage(width, height, format);

   m_pixmap_w = width;
   m_pixmap_h = height;
   m_pixmap_d = m_rasterImage.depth();

   is_null = (m_pixmap_w <= 0 || m_pixmap_h <= 0);

   if (pixelType() == BitmapType && ! m_rasterImage.isNull()) {
      m_rasterImage.setColorCount(2);
      m_rasterImage.setColor(0, QColor(Qt::color0).rgba());
      m_rasterImage.setColor(1, QColor(Qt::color1).rgba());
   }

   setSerialNumber(m_rasterImage.cacheKey() >> 32);
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
   if (! m_rasterImage.isNull()) {
      qt_scrollRectInImage(m_rasterImage, rect, QPoint(dx, dy));
   }

   return true;
}

void QRasterPlatformPixmap::fill(const QColor &color)
{
   uint pixel;

   if (m_rasterImage.depth() == 1) {
      int gray = qGray(color.rgba());

      // Pick the best approximate color in the image's colortable.
      if (qAbs(qGray(m_rasterImage.color(0)) - gray) < qAbs(qGray(m_rasterImage.color(1)) - gray)) {
         pixel = 0;
      } else {
         pixel = 1;
      }

   } else if (m_rasterImage.depth() >= 15) {
      int alpha = color.alpha();

      if (alpha != 255) {
         if (! m_rasterImage.hasAlphaChannel()) {
            QImage::Format toFormat = qt_alphaVersionForPainting(m_rasterImage.format());

            if (! m_rasterImage.isNull() && qt_depthForFormat(m_rasterImage.format()) == qt_depthForFormat(toFormat)) {
               m_rasterImage.detach();
               m_rasterImage.d->format = toFormat;

            } else {
               m_rasterImage = QImage(m_rasterImage.width(), m_rasterImage.height(), toFormat);
            }
         }
      }

      pixel = qPremultiply(color.rgba());
      const QPixelLayout *layout = &qPixelLayouts[m_rasterImage.format()];
      layout->convertFromARGB32PM(&pixel, &pixel, 1, layout, nullptr);

   } else if (m_rasterImage.format() == QImage::Format_Alpha8) {
      pixel = qAlpha(color.rgba());

   } else if (m_rasterImage.format() == QImage::Format_Grayscale8) {
      pixel = qGray(color.rgba());

   } else {
      pixel = 0;

   }

   m_rasterImage.fill(pixel);
}

bool QRasterPlatformPixmap::hasAlphaChannel() const
{
   return m_rasterImage.hasAlphaChannel();
}

QImage QRasterPlatformPixmap::toImage() const
{
   if (! m_rasterImage.isNull()) {
      QImageData *data = const_cast<QImage &>(m_rasterImage).data_ptr();

      if (data->paintEngine && data->paintEngine->isActive() && data->paintEngine->paintDevice() == &m_rasterImage) {
         return m_rasterImage.copy();
      }
   }

   return m_rasterImage;
}

QImage QRasterPlatformPixmap::toImage(const QRect &rect) const
{
   if (rect.isNull()) {
      return m_rasterImage;
   }

   QRect clipped = rect.intersected(QRect(0, 0, m_pixmap_w, m_pixmap_h));
   const uint du = uint(m_pixmap_d);

   if ((du % 8 == 0) && (((uint(clipped.x()) * du)) % 32 == 0)) {
      QImage newImage(m_rasterImage.scanLine(clipped.y()) + clipped.x() * (du / 8),
            clipped.width(), clipped.height(), m_rasterImage.bytesPerLine(), m_rasterImage.format());

      newImage.setDevicePixelRatio(m_rasterImage.devicePixelRatio());

      return newImage;

   } else {
      return m_rasterImage.copy(clipped);
   }
}

QPaintEngine *QRasterPlatformPixmap::paintEngine() const
{
   return m_rasterImage.paintEngine();
}

int QRasterPlatformPixmap::metric(QPaintDevice::PaintDeviceMetric metric) const
{
   QImageData *imageData = m_rasterImage.d;

   if (! imageData) {
      return 0;
   }

   // override the image dpi with the screen dpi when rendering to a pixmap
   switch (metric) {
      case QPaintDevice::PdmWidth:
         return m_pixmap_w;

      case QPaintDevice::PdmHeight:
         return m_pixmap_h;

      case QPaintDevice::PdmWidthMM:
         return qRound(imageData->width * 25.4 / qt_defaultDpiX());

      case QPaintDevice::PdmHeightMM:
         return qRound(imageData->height * 25.4 / qt_defaultDpiY());

      case QPaintDevice::PdmNumColors:
         return imageData->colortable.size();

      case QPaintDevice::PdmDepth:
         return m_pixmap_d;

      case QPaintDevice::PdmDpiX:
         return qt_defaultDpiX();

      case QPaintDevice::PdmPhysicalDpiX:
         return qt_defaultDpiX();

      case QPaintDevice::PdmDpiY:
         return qt_defaultDpiX();

      case QPaintDevice::PdmPhysicalDpiY:
         return qt_defaultDpiY();

      case QPaintDevice::PdmDevicePixelRatio:
         return m_rasterImage.devicePixelRatio();

      case QPaintDevice::PdmDevicePixelRatioScaled:
         return m_rasterImage.devicePixelRatio() * QPaintDevice::devicePixelRatioFScale();

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
            format = sourceImage.hasAlphaChannel() ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32;

         } else {
            QImage::Format opaqueFormat = QNativeImage::systemFormat();
            QImage::Format alphaFormat  = qt_alphaVersionForPainting(opaqueFormat);

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
      m_rasterImage = sourceImage;

      if (!inPlace) {
         m_rasterImage.detach();
      }

      if (m_rasterImage.d) {
         m_rasterImage.d->format = QImage::Format_RGB32;
      }

   } else if (inPlace && sourceImage.d->convertInPlace(format, flags)) {
      m_rasterImage = sourceImage;

   } else {
      m_rasterImage = sourceImage.convertToFormat(format);
   }

   if (m_rasterImage.d) {
      m_pixmap_w = m_rasterImage.d->width;
      m_pixmap_h = m_rasterImage.d->height;
      m_pixmap_d = m_rasterImage.d->depth;

   } else {
      m_pixmap_w = 0;
      m_pixmap_h = 0;
      m_pixmap_d = 0;
   }

   is_null = (m_pixmap_w <= 0 || m_pixmap_h <= 0);

   if (m_rasterImage.d) {
      m_rasterImage.d->devicePixelRatio = sourceImage.devicePixelRatio();
   }

   //ensure the pixmap and the image resulting from toImage() have the same cacheKey();
   setSerialNumber(m_rasterImage.cacheKey() >> 32);

   if (m_rasterImage.d) {
      setDetachNumber(m_rasterImage.d->detach_no);
   }
}

QImage *QRasterPlatformPixmap::buffer()
{
   return &m_rasterImage;
}

qreal QRasterPlatformPixmap::devicePixelRatio() const
{
   return m_rasterImage.devicePixelRatio();
}

void QRasterPlatformPixmap::setDevicePixelRatio(qreal scaleFactor)
{
   m_rasterImage.setDevicePixelRatio(scaleFactor);
}
