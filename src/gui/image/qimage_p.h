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

#ifndef QIMAGE_P_H
#define QIMAGE_P_H

#include <qglobal.h>

#include <qmap.h>
#include <qvector.h>

class QImageWriter;

struct Q_GUI_EXPORT QImageData {        // internal image data
   QImageData();
   ~QImageData();

   static QImageData *create(const QSize &size, QImage::Format format);
   static QImageData *create(uchar *data, int w, int h,  int bpl, QImage::Format format, bool readOnly,
      QImageCleanupFunction cleanupFunction = nullptr, void *cleanupInfo = nullptr);

   QAtomicInt ref;

   int width;
   int height;
   int depth;
   int nbytes;               // number of bytes data

   qreal devicePixelRatio;
   QVector<QRgb> colortable;
   uchar *data;

   QImage::Format format;
   int bytes_per_line;
   int ser_no;               // serial number
   int detach_no;

   qreal  dpmx;              // dots per meter X (or 0)
   qreal  dpmy;              // dots per meter Y (or 0)
   QPoint  offset;           // offset in pixels

   uint own_data : 1;
   uint ro_data : 1;
   uint has_alpha_clut : 1;
   uint is_cached : 1;
   uint is_locked : 1;

   QImageCleanupFunction cleanupFunction;
   void *cleanupInfo;
   bool checkForAlphaPixels() const;

   // Convert the image in-place, minimizing memory reallocation
   // Return false if the conversion cannot be done in-place.
   bool convertInPlace(QImage::Format newFormat, Qt::ImageConversionFlags);

   QMap<QString, QString> text;

   bool doImageIO(const QImage *image, QImageWriter *io, int quality) const;

   QPaintEngine *paintEngine;
};

typedef void (*Image_Converter)(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags);
typedef bool (*InPlace_Image_Converter)(QImageData *data, Qt::ImageConversionFlags);

void convert_generic(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags);
bool convert_generic_inplace(QImageData *data, QImage::Format dst_format, Qt::ImageConversionFlags);

void dither_to_Mono(QImageData *dst, const QImageData *src, Qt::ImageConversionFlags flags, bool fromalpha);

const uchar *qt_get_bitflip_array();
Q_GUI_EXPORT void qGamma_correct_back_to_linear_cs(QImage *image);

inline int qt_depthForFormat(QImage::Format format)
{
   int depth = 0;

   switch (format) {
      case QImage::Format_Invalid:
      case QImage::NImageFormats:
         // may want to throw an exception
         break;

      case QImage::Format_Mono:
      case QImage::Format_MonoLSB:
         depth = 1;
         break;

      case QImage::Format_Indexed8:
      case QImage::Format_Alpha8:
      case QImage::Format_Grayscale8:
         depth = 8;
         break;

      case QImage::Format_RGB32:
      case QImage::Format_ARGB32:
      case QImage::Format_ARGB32_Premultiplied:
      case QImage::Format_RGBX8888:
      case QImage::Format_RGBA8888:
      case QImage::Format_RGBA8888_Premultiplied:
      case QImage::Format_BGR30:
      case QImage::Format_A2BGR30_Premultiplied:
      case QImage::Format_RGB30:
      case QImage::Format_A2RGB30_Premultiplied:
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

inline QImage::Format qt_alphaVersion(QImage::Format format)
{
   switch (format) {
      case QImage::Format_RGB16:
         return QImage::Format_ARGB8565_Premultiplied;
      case QImage::Format_RGB555:
         return QImage::Format_ARGB8555_Premultiplied;
      case QImage::Format_RGB666:
         return QImage::Format_ARGB6666_Premultiplied;
      case QImage::Format_RGB444:
         return QImage::Format_ARGB4444_Premultiplied;
      case QImage::Format_RGBX8888:
         return QImage::Format_RGBA8888_Premultiplied;
      case QImage::Format_BGR30:
         return QImage::Format_A2BGR30_Premultiplied;
      case QImage::Format_RGB30:
         return QImage::Format_A2RGB30_Premultiplied;
      default:
         break;
   }
   return QImage::Format_ARGB32_Premultiplied;
}

inline QImage::Format qt_maybeAlphaVersionWithSameDepth(QImage::Format format)
{
   const QImage::Format toFormat = qt_alphaVersion(format);
   return qt_depthForFormat(format) == qt_depthForFormat(toFormat) ? toFormat : format;
}

inline QImage::Format qt_alphaVersionForPainting(QImage::Format format)
{
   QImage::Format toFormat = qt_alphaVersion(format);

#if defined(__ARM_NEON__) || defined(__SSE2__)
   // If we are switching depth anyway and we have optimized ARGB32PM routines, upgrade to that.
   if (qt_depthForFormat(format) != qt_depthForFormat(toFormat)) {
      toFormat = QImage::Format_ARGB32_Premultiplied;
   }
#endif

   return toFormat;
}

class QImageConversions
{
 public:
   QImageConversions();

   Image_Converter         image_converter_map[QImage::NImageFormats][QImage::NImageFormats];
   InPlace_Image_Converter image_inplace_converter_map[QImage::NImageFormats][QImage::NImageFormats];

   static const QImageConversions & instance();
};

#endif