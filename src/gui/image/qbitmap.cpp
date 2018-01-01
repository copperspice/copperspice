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
#include <qpixmapdata_p.h>
#include <qimage.h>
#include <qvariant.h>
#include <qpainter.h>
#include <qgraphicssystem_p.h>
#include <qapplication_p.h>

QT_BEGIN_NAMESPACE

QBitmap::QBitmap()
   : QPixmap(QSize(0, 0), QPixmapData::BitmapType)
{
}

QBitmap::QBitmap(int w, int h)
   : QPixmap(QSize(w, h), QPixmapData::BitmapType)
{
}

/*!
    Constructs a bitmap with the given \a size.  The pixels in the
    bitmap are uninitialized.

    \sa clear()
*/

QBitmap::QBitmap(const QSize &size)
   : QPixmap(size, QPixmapData::BitmapType)
{
}

/*!
    \fn QBitmap::clear()

    Clears the bitmap, setting all its bits to Qt::color0.
*/

/*!
    Constructs a bitmap that is a copy of the given \a pixmap.

    If the pixmap has a depth greater than 1, the resulting bitmap
    will be dithered automatically.

    \sa QPixmap::depth(), fromImage(), fromData()
*/

QBitmap::QBitmap(const QPixmap &pixmap)
{
   QBitmap::operator=(pixmap);
}

/*!
    \fn QBitmap::QBitmap(const QImage &image)

    Constructs a bitmap that is a copy of the given \a image.

    Use the static fromImage() function instead.
*/

/*!
    Constructs a bitmap from the file specified by the given \a
    fileName. If the file does not exist, or has an unknown format,
    the bitmap becomes a null bitmap.

    The \a fileName and \a format parameters are passed on to the
    QPixmap::load() function. If the file format uses more than 1 bit
    per pixel, the resulting bitmap will be dithered automatically.

    \sa QPixmap::isNull(), QImageReader::imageFormat()
*/

QBitmap::QBitmap(const QString &fileName, const char *format)
   : QPixmap(QSize(0, 0), QPixmapData::BitmapType)
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
      QBitmap bm(0, 0);
      QBitmap::operator=(bm);
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

   QGraphicsSystem *gs = QApplicationPrivate::graphicsSystem();
   QScopedPointer<QPixmapData> data(gs ? gs->createPixmapData(QPixmapData::BitmapType)
                                    : QGraphicsSystem::createDefaultPixmapData(QPixmapData::BitmapType));

   data->fromImage(img, flags | Qt::MonoOnly);
   return QPixmap(data.take());
}

/*!
    Constructs a bitmap with the given \a size, and sets the contents to
    the \a bits supplied.

    The bitmap data has to be byte aligned and provided in in the bit
    order specified by \a monoFormat. The mono format must be either
    QImage::Format_Mono or QImage::Format_MonoLSB. Use
    QImage::Format_Mono to specify data on the XBM format.

    \sa fromImage()

*/
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

QT_END_NAMESPACE
