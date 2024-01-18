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

#include <qplatform_pixmap.h>

#include <qbuffer.h>
#include <qbitmap.h>
#include <qimagereader.h>
#include <qplatform_integration.h>

#include <qguiapplication_p.h>
#include <qimagepixmapcleanuphooks_p.h>

QPlatformPixmap *QPlatformPixmap::create(int w, int h, PixelType type)
{
   QPlatformPixmap *data = QGuiApplicationPrivate::platformIntegration()->
                  createPlatformPixmap(static_cast<QPlatformPixmap::PixelType>(type));
   data->resize(w, h);

   return data;
}

QPlatformPixmap::QPlatformPixmap(PixelType pixelType, int objectId)
   : w(0), h(0), d(0), is_null(true), ref(0), detach_no(0), type(pixelType),
     id(objectId), ser_no(0), is_cached(false)
{
}

QPlatformPixmap::~QPlatformPixmap()
{
   // Sometimes the pixmap cleanup hooks will be called from derrived classes, which will
   // then set is_cached to false. For example, on X11 Qt GUI needs to delete the GLXPixmap
   // or EGL Pixmap Surface for a given pixmap _before_ the native X11 pixmap is deleted,
   // otherwise some drivers will leak the GL surface. In this case, QX11PlatformPixmap will
   // call the cleanup hooks itself before deleting the native pixmap and set is_cached to false.
   if (is_cached) {
      QImagePixmapCleanupHooks::executePlatformPixmapDestructionHooks(this);
      is_cached = false;
   }
}

QPlatformPixmap *QPlatformPixmap::createCompatiblePlatformPixmap() const
{
   QPlatformPixmap *d = QGuiApplicationPrivate::platformIntegration()->createPlatformPixmap(pixelType());
   return d;
}

static QImage makeBitmapCompliantIfNeeded(QPlatformPixmap *d, const QImage &image, Qt::ImageConversionFlags flags)
{
   if (d->pixelType() == QPlatformPixmap::BitmapType) {
      QImage img = image.convertToFormat(QImage::Format_MonoLSB, flags);

      // make sure image.color(0) == Qt::color0 (white)
      // and image.color(1) == Qt::color1 (black)
      const QRgb c0 = QColor(Qt::black).rgb();
      const QRgb c1 = QColor(Qt::white).rgb();

      if (img.color(0) == c0 && img.color(1) == c1) {
         img.invertPixels();
         img.setColor(0, c1);
         img.setColor(1, c0);
      }

      return img;
   }

   return image;
}

void QPlatformPixmap::fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags)
{
   const QImage image = imageReader->read();
   fromImage(image, flags);
}

bool QPlatformPixmap::fromFile(const QString &fileName, const QString &format, Qt::ImageConversionFlags flags)
{
   QImage image = QImageReader(fileName, format).read();
   if (image.isNull()) {
      return false;
   }
   fromImage(makeBitmapCompliantIfNeeded(this, image, flags), flags);

   return !isNull();
}

bool QPlatformPixmap::fromData(const uchar *buf, uint len, const QString &format, Qt::ImageConversionFlags flags)
{
   QByteArray a = QByteArray::fromRawData(reinterpret_cast<const char *>(buf), len);
   QBuffer b(&a);
   b.open(QIODevice::ReadOnly);
   QImage image = QImageReader(&b, format).read();

   if (image.isNull()) {
      return false;
   }
   fromImage(makeBitmapCompliantIfNeeded(this, image, flags), flags);

   return !isNull();
}

void QPlatformPixmap::copy(const QPlatformPixmap *data, const QRect &rect)
{
   fromImage(data->toImage(rect), Qt::NoOpaqueDetection);
}

bool QPlatformPixmap::scroll(int dx, int dy, const QRect &rect)
{
   (void) dx;
   (void) dy;
   (void) rect;

   return false;
}

QPixmap QPlatformPixmap::transformed(const QTransform &matrix, Qt::TransformationMode mode) const
{
   return QPixmap::fromImage(toImage().transformed(matrix, mode));
}

void QPlatformPixmap::setSerialNumber(int serNo)
{
   ser_no = serNo;
}

void QPlatformPixmap::setDetachNumber(int detNo)
{
   detach_no = detNo;
}

QImage QPlatformPixmap::toImage(const QRect &rect) const
{
   if (rect.contains(QRect(0, 0, w, h))) {
      return toImage();
   } else {
      return toImage().copy(rect);
   }
}

QImage *QPlatformPixmap::buffer()
{
   return nullptr;
}


