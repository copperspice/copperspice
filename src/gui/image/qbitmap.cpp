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

#include <qbitmap.h>

#include <qimage.h>
#include <qpainter.h>
#include <qplatform_integration.h>
#include <qplatform_pixmap.h>
#include <qscreen.h>
#include <qvariant.h>


#include <qapplication_p.h>



QBitmap::QBitmap()
   : QPixmap(QSize(0, 0), QPlatformPixmap::BitmapType)
{
}

QBitmap::QBitmap(int w, int h)
   : QPixmap(QSize(w, h), QPlatformPixmap::BitmapType)
{
}



QBitmap::QBitmap(const QSize &size)
   : QPixmap(size, QPlatformPixmap::BitmapType)
{
}



QBitmap::QBitmap(const QPixmap &pixmap)
{
   QBitmap::operator=(pixmap);
}



QBitmap::QBitmap(const QString &fileName, const char *format)
   : QPixmap(QSize(0, 0), QPlatformPixmap::BitmapType)
{
   load(fileName, format, Qt::MonoOnly);
}

/*!
    \overload

    Assigns the given \a pixmap to this bitmap and returns a reference
    to this bitmap.

    If the pixmap has a depth greater than 1, the resulting bitmap
    will be dithered automatically.

    \sa QPixmap::depth()
 */

QBitmap &QBitmap::operator=(const QPixmap &pixmap)
{
   if (pixmap.isNull()) {                        // a null pixmap
      QBitmap(0, 0).swap(*this);
   } else if (pixmap.depth() == 1) {                // 1-bit pixmap
      QPixmap::operator=(pixmap);                // shallow assignment
   } else {                                        // n-bit depth pixmap
      QImage image;
      image = pixmap.toImage();                                // convert pixmap to image
      *this = fromImage(image);                                // will dither image
   }
   return *this;
}

/*!
  Destroys the bitmap.
*/
QBitmap::~QBitmap()
{
}

/*!
    \fn void QBitmap::swap(QBitmap &other)
    \since 4.8

    Swaps bitmap \a other with this bitmap. This operation is very
    fast and never fails.
*/

/*!
   Returns the bitmap as a QVariant.
*/
QBitmap::operator QVariant() const
{
   return QVariant(QVariant::Bitmap, this);
}

/*!
    \fn QBitmap &QBitmap::operator=(const QImage &image)
    \overload

    Converts the given \a image to a bitmap, and assigns the result to
    this bitmap. Returns a reference to the bitmap.

    Use the static fromImage() function instead.
*/

/*!
    Returns a copy of the given \a image converted to a bitmap using
    the specified image conversion \a flags.

    \sa fromData()
*/
QBitmap QBitmap::fromImage(const QImage &image, Qt::ImageConversionFlags flags)
{
   if (image.isNull()) {
      return QBitmap();
   }

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

   QScopedPointer<QPlatformPixmap> data(QGuiApplicationPrivate::platformIntegration()->createPlatformPixmap(QPlatformPixmap::BitmapType));

   data->fromImage(img, flags | Qt::MonoOnly);
   return QPixmap(data.take());
}


QBitmap QBitmap::fromData(const QSize &size, const uchar *bits, QImage::Format monoFormat)
{
   Q_ASSERT(monoFormat == QImage::Format_Mono || monoFormat == QImage::Format_MonoLSB);

   QImage image(size, monoFormat);
   image.setColor(0, QColor(Qt::color0).rgb());
   image.setColor(1, QColor(Qt::color1).rgb());

   // Need to memcpy each line separatly since QImage is 32bit aligned and
   // this data is only byte aligned...
   int bytesPerLine = (size.width() + 7) / 8;
   for (int y = 0; y < size.height(); ++y) {
      memcpy(image.scanLine(y), bits + bytesPerLine * y, bytesPerLine);
   }
   return QBitmap::fromImage(image);
}

/*!
    Returns a copy of this bitmap, transformed according to the given
    \a matrix.

    \sa QPixmap::transformed()
 */
QBitmap QBitmap::transformed(const QTransform &matrix) const
{
   QBitmap bm = QPixmap::transformed(matrix);
   return bm;
}

/*!
  \overload
  \obsolete

  This convenience function converts the \a matrix to a QTransform
  and calls the overloaded function.
*/
QBitmap QBitmap::transformed(const QMatrix &matrix) const
{
   return transformed(QTransform(matrix));
}

