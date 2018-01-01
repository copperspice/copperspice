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

#ifndef QIMAGE_P_H
#define QIMAGE_P_H

#include <QtCore/qglobal.h>
#include <QVector>

#ifndef QT_NO_IMAGE_TEXT
#include <QMap>
#endif

QT_BEGIN_NAMESPACE

class QImageWriter;

struct Q_GUI_EXPORT QImageData {        // internal image data
   QImageData();
   ~QImageData();
   static QImageData *create(const QSize &size, QImage::Format format, int numColors = 0);
   static QImageData *create(uchar *data, int w, int h,  int bpl, QImage::Format format, bool readOnly);

   QAtomicInt ref;

   int width;
   int height;
   int depth;
   int nbytes;               // number of bytes data
   QVector<QRgb> colortable;
   uchar *data;

   QImage::Format format;
   int bytes_per_line;
   int ser_no;               // serial number
   int detach_no;

   qreal  dpmx;                // dots per meter X (or 0)
   qreal  dpmy;                // dots per meter Y (or 0)
   QPoint  offset;           // offset in pixels

   uint own_data : 1;
   uint ro_data : 1;
   uint has_alpha_clut : 1;
   uint is_cached : 1;

   bool checkForAlphaPixels() const;

   // Convert the image in-place, minimizing memory reallocation
   // Return false if the conversion cannot be done in-place.
   bool convertInPlace(QImage::Format newFormat, Qt::ImageConversionFlags);

#ifndef QT_NO_IMAGE_TEXT
   QMap<QString, QString> text;
#endif

   bool doImageIO(const QImage *image, QImageWriter *io, int quality) const;

   QPaintEngine *paintEngine;
};

void qInitImageConversions();
Q_GUI_EXPORT void qGamma_correct_back_to_linear_cs(QImage *image);

inline int qt_depthForFormat(QImage::Format format)
{
   int depth = 0;
   switch (format) {
      case QImage::Format_Invalid:
      case QImage::NImageFormats:
         Q_ASSERT(false);
      case QImage::Format_Mono:
      case QImage::Format_MonoLSB:
         depth = 1;
         break;
      case QImage::Format_Indexed8:
         depth = 8;
         break;
      case QImage::Format_RGB32:
      case QImage::Format_ARGB32:
      case QImage::Format_ARGB32_Premultiplied:
         depth = 32;
         break;
      case QImage::Format_RGB555:
      case QImage::Format_RGB16:
      case QImage::Format_RGB444:
      case QImage::Format_ARGB4444_Premultiplied:
         depth = 16;
         break;
      case QImage::Format_RGB666:
      case QImage::Format_ARGB6666_Premultiplied:
      case QImage::Format_ARGB8565_Premultiplied:
      case QImage::Format_ARGB8555_Premultiplied:
      case QImage::Format_RGB888:
         depth = 24;
         break;
   }
   return depth;
}

QT_END_NAMESPACE

#endif
