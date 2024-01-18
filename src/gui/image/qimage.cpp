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

#include <qimage.h>

#include <qdatastream.h>
#include <qbuffer.h>
#include <qhash.h>
#include <qmap.h>
#include <qmatrix.h>
#include <qtransform.h>
#include <qimagereader.h>
#include <qimagewriter.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qplatform_integration.h>
#include <qplatform_pixmap.h>

#include <qimagepixmapcleanuphooks_p.h>
#include <qapplication_p.h>
#include <qdrawhelper_p.h>
#include <qmemrotate_p.h>
#include <qimagescale_p.h>
#include <qsimd_p.h>
#include <qpaintengine_raster_p.h>
#include <qimage_p.h>
#include <qfont_p.h>

#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

static inline bool isLocked(QImageData *data)
{
   return data != nullptr && data->is_locked;
}

#if defined(Q_CC_DEC) && defined(__alpha) && (__DECCXX_VER-0 >= 50190001)
#pragma message disable narrowptr
#endif

#define QIMAGE_SANITYCHECK_MEMORY(image) \
    if ((image).isNull()) { \
        qWarning("QImage: Out of memory, returning empty image"); \
        return QImage(); \
    }

static QImage rotated90(const QImage &src);
static QImage rotated180(const QImage &src);
static QImage rotated270(const QImage &src);

QAtomicInt qimage_serial_number = 1;

QImageData::QImageData()
   : ref(0), width(0), height(0), depth(0), nbytes(0), devicePixelRatio(1.0), data(nullptr),
     format(QImage::Format_ARGB32), bytes_per_line(0),
     ser_no(qimage_serial_number.fetchAndAddRelaxed(1)), detach_no(0),
     dpmx(qt_defaultDpiX() * 100 / qreal(2.54)), dpmy(qt_defaultDpiY() * 100 / qreal(2.54)),
     offset(0, 0), own_data(true), ro_data(false), has_alpha_clut(false),
     is_cached(false), is_locked(false), cleanupFunction(nullptr), cleanupInfo(nullptr), paintEngine(nullptr)
{
}

QImageData *QImageData::create(const QSize &size, QImage::Format format)
{
   if (!size.isValid() || format == QImage::Format_Invalid) {
      return nullptr;   // invalid parameter(s)
   }

   uint width  = size.width();
   uint height = size.height();
   uint depth  = qt_depthForFormat(format);

   const int bytes_per_line = ((width * depth + 31) >> 5) << 2; // bytes per scanline (must be multiple of 4)

   // sanity check for potential overflows
   if (INT_MAX / depth < width
      || bytes_per_line <= 0
      || height <= 0
      || INT_MAX / uint(bytes_per_line) < height
      || INT_MAX / sizeof(uchar *) < uint(height)) {
      return nullptr;
   }

   QScopedPointer<QImageData> d(new QImageData);
   switch (format) {
      case QImage::Format_Mono:
      case QImage::Format_MonoLSB:
         d->colortable.resize(2);
         d->colortable[0] = QColor(Qt::black).rgba();
         d->colortable[1] = QColor(Qt::white).rgba();
         break;

      default:
         break;
   }

   d->width  = width;
   d->height = height;
   d->depth  = depth;
   d->format = format;
   d->has_alpha_clut = false;
   d->is_cached = false;

   d->bytes_per_line = bytes_per_line;

   d->nbytes = d->bytes_per_line * height;
   d->data   = (uchar *)malloc(d->nbytes);

   if (! d->data) {
      return nullptr;
   }

   d->ref.ref();

   return d.take();
}

QImageData::~QImageData()
{
   if (cleanupFunction) {
      cleanupFunction(cleanupInfo);
   }

   if (is_cached) {
      QImagePixmapCleanupHooks::executeImageHooks((((qint64) ser_no) << 32) | ((qint64) detach_no));
   }

   delete paintEngine;

   if (data && own_data) {
      free(data);
   }

   data = nullptr;
}

bool QImageData::checkForAlphaPixels() const
{
   bool has_alpha_pixels = false;

   switch (format) {

      case QImage::Format_Mono:
      case QImage::Format_MonoLSB:
      case QImage::Format_Indexed8:
         has_alpha_pixels = has_alpha_clut;
         break;

      case QImage::Format_Alpha8:
         has_alpha_pixels = true;
         break;

      case QImage::Format_ARGB32:
      case QImage::Format_ARGB32_Premultiplied: {
         uchar *bits = data;

         for (int y = 0; y < height && !has_alpha_pixels; ++y) {
            for (int x = 0; x < width; ++x) {
               has_alpha_pixels |= (((uint *)bits)[x] & 0xff000000) != 0xff000000;
            }
            bits += bytes_per_line;
         }
      }
      break;

      case QImage::Format_RGBA8888:
      case QImage::Format_RGBA8888_Premultiplied: {
         uchar *bits = data;

         for (int y = 0; y < height && !has_alpha_pixels; ++y) {
            for (int x = 0; x < width; ++x) {
               has_alpha_pixels |= bits[x * 4 + 3] != 0xff;
            }

            bits += bytes_per_line;
         }
      }
      break;

      case QImage::Format_A2BGR30_Premultiplied:
      case QImage::Format_A2RGB30_Premultiplied: {
         uchar *bits = data;
         for (int y = 0; y < height && !has_alpha_pixels; ++y) {
            for (int x = 0; x < width; ++x) {
               has_alpha_pixels |= (((uint *)bits)[x] & 0xc0000000) != 0xc0000000;
            }
            bits += bytes_per_line;
         }
      }
      break;

      case QImage::Format_ARGB8555_Premultiplied:
      case QImage::Format_ARGB8565_Premultiplied: {
         uchar *bits = data;
         uchar *end_bits = data + bytes_per_line;

         for (int y = 0; y < height && !has_alpha_pixels; ++y) {
            while (bits < end_bits) {
               has_alpha_pixels |= bits[0] != 0;
               bits += 3;
            }
            bits = end_bits;
            end_bits += bytes_per_line;
         }
      }
      break;

      case QImage::Format_ARGB6666_Premultiplied: {
         uchar *bits = data;
         uchar *end_bits = data + bytes_per_line;

         for (int y = 0; y < height && !has_alpha_pixels; ++y) {
            while (bits < end_bits) {
               has_alpha_pixels |= (bits[0] & 0xfc) != 0;
               bits += 3;
            }
            bits = end_bits;
            end_bits += bytes_per_line;
         }
      }
      break;

      case QImage::Format_ARGB4444_Premultiplied: {
         uchar *bits = data;
         uchar *end_bits = data + bytes_per_line;

         for (int y = 0; y < height && !has_alpha_pixels; ++y) {
            while (bits < end_bits) {
               has_alpha_pixels |= (bits[0] & 0xf0) != 0;
               bits += 2;
            }
            bits = end_bits;
            end_bits += bytes_per_line;
         }
      }
      break;

      case QImage::Format_RGB32:
      case QImage::Format_RGB16:
      case QImage::Format_RGB444:
      case QImage::Format_RGB555:
      case QImage::Format_RGB666:
      case QImage::Format_RGB888:
      case QImage::Format_RGBX8888:
      case QImage::Format_BGR30:
      case QImage::Format_RGB30:
      case QImage::Format_Grayscale8:
         break;

      case QImage::Format_Invalid:
      case QImage::NImageFormats:
         // error, may want to throw
         break;
   }

   return has_alpha_pixels;
}

QImage::QImage()
   : QPaintDevice()
{
   d = nullptr;
}

QImage::QImage(int width, int height, Format format)
   : QPaintDevice()
{
   d = QImageData::create(QSize(width, height), format);
}

QImage::QImage(const QSize &size, Format format)
   : QPaintDevice()
{
   d = QImageData::create(size, format);
}

QImageData *QImageData::create(uchar *data, int width, int height,  int bpl, QImage::Format format, bool readOnly,
   QImageCleanupFunction cleanupFunction, void *cleanupInfo)
{
   QImageData *d = nullptr;

   if (format == QImage::Format_Invalid) {
      return d;
   }

   const int depth = qt_depthForFormat(format);
   const int calc_bytes_per_line = ((width * depth + 31) / 32) * 4;
   const int min_bytes_per_line  = (width * depth + 7) / 8;

   if (bpl <= 0) {
      bpl = calc_bytes_per_line;
   }

   if (width <= 0 || height <= 0 || ! data
      || INT_MAX / sizeof(uchar *) < uint(height)
      || INT_MAX / uint(depth) < uint(width)
      || bpl <= 0
      || bpl < min_bytes_per_line
      || INT_MAX / uint(bpl) < uint(height)) {

      return d;   // invalid parameter(s)
   }

   d = new QImageData;
   d->ref.ref();

   d->own_data = false;
   d->ro_data  = readOnly;
   d->data     = data;
   d->width    = width;
   d->height   = height;
   d->depth    = depth;
   d->format   = format;

   d->bytes_per_line = bpl;
   d->nbytes = d->bytes_per_line * height;

   d->cleanupFunction = cleanupFunction;
   d->cleanupInfo = cleanupInfo;

   return d;
}

QImage::QImage(uchar *data, int width, int height, Format format, QImageCleanupFunction cleanupFunction, void *cleanupInfo)
   : QPaintDevice()
{
   d = QImageData::create(data, width, height, 0, format, false, cleanupFunction, cleanupInfo);
}

QImage::QImage(const uchar *data, int width, int height, Format format, QImageCleanupFunction cleanupFunction, void *cleanupInfo)
   : QPaintDevice()
{
   d = QImageData::create(const_cast<uchar *>(data), width, height, 0, format, true, cleanupFunction, cleanupInfo);
}

QImage::QImage(uchar *data, int width, int height, int bytesPerLine, Format format, QImageCleanupFunction cleanupFunction,
   void *cleanupInfo)
   : QPaintDevice()
{
   d = QImageData::create(data, width, height, bytesPerLine, format, false, cleanupFunction, cleanupInfo);
}

QImage::QImage(const uchar *data, int width, int height, int bytesPerLine, Format format, QImageCleanupFunction cleanupFunction,
   void *cleanupInfo)
   : QPaintDevice()
{
   d = QImageData::create(const_cast<uchar *>(data), width, height, bytesPerLine, format, true, cleanupFunction, cleanupInfo);
}

QImage::QImage(const QString &fileName, const QString &format)
   : QPaintDevice()
{
   d = nullptr;
   load(fileName, format);
}

#ifndef QT_NO_IMAGEFORMAT_XPM
extern bool qt_read_xpm_image_or_array(QIODevice *device, const char *const *source, QImage &image);

QImage::QImage(const char *const xpm[])
   : QPaintDevice()
{
   d = nullptr;

   if (!xpm) {
      return;
   }

   if (! qt_read_xpm_image_or_array(nullptr, xpm, *this)) {
      qWarning("QImage::QImage() XPM format is not supported");
   }
}
#endif

QImage::QImage(const QImage &image)
   : QPaintDevice()
{
   if (image.paintingActive() || isLocked(image.d)) {
      d = nullptr;
      image.copy().swap(*this);

   } else {
      d = image.d;

      if (d) {
         d->ref.ref();
      }
   }
}

QImage::~QImage()
{
   if (d && ! d->ref.deref()) {
      delete d;
   }
}

QImage &QImage::operator=(const QImage &image)
{
   if (image.paintingActive() || isLocked(image.d)) {
      operator=(image.copy());

   } else {
      if (image.d) {
         image.d->ref.ref();
      }

      if (d && ! d->ref.deref()) {
         delete d;
      }

      d = image.d;
   }

   return *this;
}

// internal
int QImage::devType() const
{
   return QInternal::Image;
}

QImage::operator QVariant() const
{
   return QVariant(QVariant::Image, this);
}

// internal
void QImage::detach()
{
   if (d) {
      if (d->is_cached && d->ref.load() == 1) {
         QImagePixmapCleanupHooks::executeImageHooks(cacheKey());
      }

      if (d->ref.load() != 1 || d->ro_data) {
         *this = copy();
      }

      if (d) {
         ++d->detach_no;
      }
   }
}

static void copyMetadata(QImageData *dst, const QImageData *src)
{
   // Doesn't copy colortable and alpha_clut, or offset.
   dst->dpmx = src->dpmx;
   dst->dpmy = src->dpmy;
   dst->devicePixelRatio = src->devicePixelRatio;
   dst->text = src->text;
}

QImage QImage::copy(const QRect &r) const
{
   if (! d) {
      return QImage();
   }

   if (r.isNull()) {
      QImage image(d->width, d->height, d->format);
      if (image.isNull()) {
         return image;
      }

      // Qt for Embedded Linux can create images with non-default bpl
      // make sure we don't crash.
      if (image.d->nbytes != d->nbytes) {
         int bpl = qMin(bytesPerLine(), image.bytesPerLine());
         for (int i = 0; i < height(); i++) {
            memcpy(image.scanLine(i), scanLine(i), bpl);
         }

      } else {
         memcpy(image.bits(), bits(), d->nbytes);
      }
      image.d->colortable = d->colortable;

      image.d->offset = d->offset;
      image.d->has_alpha_clut = d->has_alpha_clut;
      copyMetadata(image.d, d);

      return image;
   }

   int x = r.x();
   int y = r.y();
   int w = r.width();
   int h = r.height();

   int dx = 0;
   int dy = 0;
   if (w <= 0 || h <= 0) {
      return QImage();
   }

   QImage image(w, h, d->format);
   if (image.isNull()) {
      return image;
   }

   if (x < 0 || y < 0 || x + w > d->width || y + h > d->height) {
      // bitBlt will not cover entire image - clear it.
      image.fill(0);
      if (x < 0) {
         dx = -x;
         x = 0;
      }
      if (y < 0) {
         dy = -y;
         y = 0;
      }
   }

   image.d->colortable = d->colortable;

   int pixels_to_copy = qMax(w - dx, 0);
   if (x > d->width) {
      pixels_to_copy = 0;
   } else if (pixels_to_copy > d->width - x) {
      pixels_to_copy = d->width - x;
   }

   int lines_to_copy = qMax(h - dy, 0);

   if (y > d->height) {
      lines_to_copy = 0;
   } else if (lines_to_copy > d->height - y) {
      lines_to_copy = d->height - y;
   }

   bool byteAligned = true;
   if (d->format == Format_Mono || d->format == Format_MonoLSB) {
      byteAligned = !(dx & 7) && !(x & 7) && !(pixels_to_copy & 7);
   }

   if (byteAligned) {
      const uchar *src = d->data + ((x * d->depth) >> 3) + y * d->bytes_per_line;
      uchar *dest = image.d->data + ((dx * d->depth) >> 3) + dy * image.d->bytes_per_line;
      const int bytes_to_copy = (pixels_to_copy * d->depth) >> 3;
      for (int i = 0; i < lines_to_copy; ++i) {
         memcpy(dest, src, bytes_to_copy);
         src += d->bytes_per_line;
         dest += image.d->bytes_per_line;
      }
   } else if (d->format == Format_Mono) {
      const uchar *src = d->data + y * d->bytes_per_line;
      uchar *dest = image.d->data + dy * image.d->bytes_per_line;
      for (int i = 0; i < lines_to_copy; ++i) {
         for (int j = 0; j < pixels_to_copy; ++j) {
            if (src[(x + j) >> 3] & (0x80 >> ((x + j) & 7))) {
               dest[(dx + j) >> 3] |= (0x80 >> ((dx + j) & 7));
            } else {
               dest[(dx + j) >> 3] &= ~(0x80 >> ((dx + j) & 7));
            }
         }
         src += d->bytes_per_line;
         dest += image.d->bytes_per_line;
      }
   } else { // Format_MonoLSB
      Q_ASSERT(d->format == Format_MonoLSB);
      const uchar *src = d->data + y * d->bytes_per_line;
      uchar *dest = image.d->data + dy * image.d->bytes_per_line;

      for (int i = 0; i < lines_to_copy; ++i) {
         for (int j = 0; j < pixels_to_copy; ++j) {
            if (src[(x + j) >> 3] & (0x1 << ((x + j) & 7))) {
               dest[(dx + j) >> 3] |= (0x1 << ((dx + j) & 7));
            } else {
               dest[(dx + j) >> 3] &= ~(0x1 << ((dx + j) & 7));
            }
         }

         src += d->bytes_per_line;
         dest += image.d->bytes_per_line;
      }
   }

   copyMetadata(image.d, d);
   image.d->offset = offset();
   image.d->has_alpha_clut = d->has_alpha_clut;

   return image;
}

bool QImage::isNull() const
{
   return !d;
}

int QImage::width() const
{
   return d ? d->width : 0;
}

int QImage::height() const
{
   return d ? d->height : 0;
}

QSize QImage::size() const
{
   return d ? QSize(d->width, d->height) : QSize(0, 0);
}

QRect QImage::rect() const
{
   return d ? QRect(0, 0, d->width, d->height) : QRect();
}

int QImage::depth() const
{
   return d ? d->depth : 0;
}

int QImage::colorCount() const
{
   return d ? d->colortable.size() : 0;
}

void QImage::setColorTable(const QVector<QRgb> &colors)
{
   if (! d) {
      return;
   }
   detach();

   // In case detach() ran out of memory
   if (! d) {
      return;
   }

   d->colortable = colors;
   d->has_alpha_clut = false;

   for (int i = 0; i < d->colortable.size(); ++i) {
      if (qAlpha(d->colortable.at(i)) != 255) {
         d->has_alpha_clut = true;
         break;
      }
   }
}

QVector<QRgb> QImage::colorTable() const
{
   return d ? d->colortable : QVector<QRgb>();
}

qreal QImage::devicePixelRatio() const
{
   if (!d) {
      return 1.0;
   }
   return d->devicePixelRatio;
}

void QImage::setDevicePixelRatio(qreal scaleFactor)
{
   if (!d) {
      return;
   }

   if (scaleFactor == d->devicePixelRatio) {
      return;
   }

   detach();
   d->devicePixelRatio = scaleFactor;
}

int QImage::byteCount() const
{
   return d ? d->nbytes : 0;
}

int QImage::bytesPerLine() const
{
   return (d && d->height) ? d->nbytes / d->height : 0;
}

QRgb QImage::color(int i) const
{
   Q_ASSERT(i < colorCount());
   return d ? d->colortable.at(i) : QRgb(uint(-1));
}

void QImage::setColor(int i, QRgb c)
{
   if (!d) {
      return;
   }

   if (i < 0 || d->depth > 8 || i >= 1 << d->depth) {
      qWarning("QImage::setColor() Index out of bound %d", i);
      return;
   }
   detach();

   // In case detach() run out of memory
   if (!d) {
      return;
   }

   if (i >= d->colortable.size()) {
      setColorCount(i + 1);
   }
   d->colortable[i] = c;
   d->has_alpha_clut |= (qAlpha(c) != 255);
}

uchar *QImage::scanLine(int i)
{
   if (!d) {
      return nullptr;
   }

   detach();

   // In case detach() ran out of memory
   if (!d) {
      return nullptr;
   }

   return d->data + i * d->bytes_per_line;
}

const uchar *QImage::scanLine(int i) const
{
   if (!d) {
      return nullptr;
   }

   Q_ASSERT(i >= 0 && i < height());
   return d->data + i * d->bytes_per_line;
}

const uchar *QImage::constScanLine(int i) const
{
   if (!d) {
      return nullptr;
   }

   Q_ASSERT(i >= 0 && i < height());
   return d->data + i * d->bytes_per_line;
}

uchar *QImage::bits()
{
   if (!d) {
      return nullptr;
   }
   detach();

   // In case detach ran out of memory...
   if (!d) {
      return nullptr;
   }

   return d->data;
}

const uchar *QImage::bits() const
{
   return d ? d->data : nullptr;
}

const uchar *QImage::constBits() const
{
   return d ? d->data : nullptr;
}

void QImage::fill(uint pixel)
{
   if (!d) {
      return;
   }

   detach();

   // In case detach() ran out of memory
   if (!d ) {
      return;
   }

   if (d->depth == 1 || d->depth == 8) {
      int w = d->width;
      if (d->depth == 1) {
         if (pixel & 1) {
            pixel = 0xffffffff;
         } else {
            pixel = 0;
         }
         w = (w + 7) / 8;
      } else {
         pixel &= 0xff;
      }

      qt_rectfill<quint8>(d->data, pixel, 0, 0, w, d->height, d->bytes_per_line);
      return;

   } else if (d->depth == 16) {
      qt_rectfill<quint16>(reinterpret_cast<quint16 *>(d->data), pixel,
         0, 0, d->width, d->height, d->bytes_per_line);
      return;

   } else if (d->depth == 24) {
      qt_rectfill<quint24>(reinterpret_cast<quint24 *>(d->data), pixel,
         0, 0, d->width, d->height, d->bytes_per_line);
      return;
   }

   if (d->format == Format_RGB32) {
      pixel |= 0xff000000;
   }

   if (d->format == Format_RGBX8888)
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      pixel |= 0xff000000;
#else
      pixel |= 0x000000ff;
#endif

   if (d->format == Format_BGR30 || d->format == Format_RGB30) {
      pixel |= 0xc0000000;
   }

   qt_rectfill<uint>(reinterpret_cast<uint *>(d->data), pixel,
      0, 0, d->width, d->height, d->bytes_per_line);
}

void QImage::fill(Qt::GlobalColor color)
{
   fill(QColor(color));
}

void QImage::fill(const QColor &color)
{
   if (!d) {
      return;
   }
   detach();

   // In case we run out of memory
   if (!d) {
      return;
   }

   switch (d->format) {
      case QImage::Format_RGB32:
      case QImage::Format_ARGB32:
         fill(color.rgba());
         break;
      case QImage::Format_ARGB32_Premultiplied:
         fill(qPremultiply(color.rgba()));
         break;
      case QImage::Format_RGBX8888:
         fill(ARGB2RGBA(color.rgba() | 0xff000000));
         break;
      case QImage::Format_RGBA8888:
         fill(ARGB2RGBA(color.rgba()));
         break;
      case QImage::Format_RGBA8888_Premultiplied:
         fill(ARGB2RGBA(qPremultiply(color.rgba())));
         break;
      case QImage::Format_BGR30:
      case QImage::Format_A2BGR30_Premultiplied:
         fill(qConvertRgb64ToRgb30<PixelOrderBGR>(color.rgba64()));
         break;
      case QImage::Format_RGB30:
      case QImage::Format_A2RGB30_Premultiplied:
         fill(qConvertRgb64ToRgb30<PixelOrderRGB>(color.rgba64()));
         break;
      case QImage::Format_RGB16:
         fill((uint) qConvertRgb32To16(color.rgba()));
         break;
      case QImage::Format_Indexed8: {
         uint pixel = 0;
         for (int i = 0; i < d->colortable.size(); ++i) {
            if (color.rgba() == d->colortable.at(i)) {
               pixel = i;
               break;
            }
         }
         fill(pixel);
         break;
      }
      case QImage::Format_Mono:
      case QImage::Format_MonoLSB:
         if (color == Qt::color1) {
            fill((uint) 1);
         } else {
            fill((uint) 0);
         }
         break;
      default: {
         QPainter p(this);
         p.setCompositionMode(QPainter::CompositionMode_Source);
         p.fillRect(rect(), color);
      }
   }
}

void QImage::invertPixels(InvertMode mode)
{
   if (!d) {
      return;
   }

   detach();

   // In case detach() ran out of memory
   if (!d) {
      return;
   }

   QImage::Format originalFormat = d->format;

   // Inverting premultiplied pixels would produce invalid image data.
   if (hasAlphaChannel() && qPixelLayouts[d->format].premultiplied) {
      if (!d->convertInPlace(QImage::Format_ARGB32, Qt::EmptyFlag)) {
         *this = convertToFormat(QImage::Format_ARGB32);
      }
   }

   if (depth() < 32) {
      // number of used bytes pr line
      int bpl   = (d->width * d->depth + 7) / 8;
      int pad   = d->bytes_per_line - bpl;
      uchar *sl = d->data;

      for (int y = 0; y < d->height; ++y) {
         for (int x = 0; x < bpl; ++x) {
            *sl++ ^= 0xff;
         }

         sl += pad;
      }

   } else {
      quint32 *p      = (quint32 *)d->data;
      quint32 *end    = (quint32 *)(d->data + d->nbytes);
      quint32 xorbits = 0xffffffff;

      switch (d->format) {
         case QImage::Format_RGBA8888:
            if (mode == InvertRgba) {
               break;
            }
            [[fallthrough]];

         case QImage::Format_RGBX8888:

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
            xorbits = 0xffffff00;
            break;
#else
            xorbits = 0x00ffffff;
            break;
#endif

         case QImage::Format_ARGB32:
            if (mode == InvertRgba) {
               break;
            }
            [[fallthrough]];

         case QImage::Format_RGB32:
            xorbits = 0x00ffffff;
            break;

         case QImage::Format_BGR30:
         case QImage::Format_RGB30:
            xorbits = 0x3fffffff;
            break;

         default:
            // error, should not reach here
            xorbits = 0;
            break;
      }

      while (p < end) {
         *p++ ^= xorbits;
      }
   }

   if (originalFormat != d->format) {
      if (! d->convertInPlace(originalFormat, Qt::EmptyFlag)) {
         *this = convertToFormat(originalFormat);
      }
   }
}

// Windows defines these
#if defined(write)
# undef write
#endif

#if defined(close)
# undef close
#endif

#if defined(read)
# undef read
#endif

void QImage::setColorCount(int colorCount)
{
   if (!d) {
      qWarning("QImage::setColorCount() Image is empty");
      return;
   }

   detach();

   // In case detach() ran out of memory
   if (! d) {
      return;
   }

   if (colorCount == d->colortable.size()) {
      return;
   }

   if (colorCount <= 0) {                        // use no color table
      d->colortable = QVector<QRgb>();
      return;
   }
   int nc = d->colortable.size();
   d->colortable.resize(colorCount);

   for (int i = nc; i < colorCount; ++i) {
      d->colortable[i] = 0;
   }
}

QImage::Format QImage::format() const
{
   return d ? d->format : Format_Invalid;
}

QImage QImage::convertToFormat_helper(Format format, Qt::ImageConversionFlags flags) const
{
   if (! d || d->format == format) {
      return *this;
   }

   if (format == Format_Invalid || d->format == Format_Invalid) {
      return QImage();
   }

   Image_Converter converter = QImageConversions::instance().image_converter_map[d->format][format];

   if (! converter && (format > QImage::Format_Indexed8) && (d->format > QImage::Format_Indexed8)) {
      converter = convert_generic;
   }

   if (converter) {
      QImage image(d->width, d->height, format);

      QIMAGE_SANITYCHECK_MEMORY(image);

      image.d->offset = offset();
      copyMetadata(image.d, d);

      converter(image.d, d, flags);
      return image;
   }

   // Convert indexed formats over ARGB32 or RGB32 to the final format
   Q_ASSERT(format    != QImage::Format_ARGB32 && format != QImage::Format_RGB32);
   Q_ASSERT(d->format != QImage::Format_ARGB32 && d->format != QImage::Format_RGB32);

   if (! hasAlphaChannel()) {
      return convertToFormat(Format_RGB32, flags).convertToFormat(format, flags);
   }

   return convertToFormat(Format_ARGB32, flags).convertToFormat(format, flags);
}

bool QImage::convertToFormat_inplace(Format format, Qt::ImageConversionFlags flags)
{
   return d && d->convertInPlace(format, flags);
}

static inline int pixel_distance(QRgb p1, QRgb p2)
{
   int r1 = qRed(p1);
   int g1 = qGreen(p1);
   int b1 = qBlue(p1);
   int a1 = qAlpha(p1);

   int r2 = qRed(p2);
   int g2 = qGreen(p2);
   int b2 = qBlue(p2);
   int a2 = qAlpha(p2);

   return abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2) + abs(a1 - a2);
}

static inline int closestMatch(QRgb pixel, const QVector<QRgb> &clut)
{
   int idx = 0;
   int current_distance = INT_MAX;
   for (int i = 0; i < clut.size(); ++i) {
      int dist = pixel_distance(pixel, clut.at(i));
      if (dist < current_distance) {
         current_distance = dist;
         idx = i;
      }
   }
   return idx;
}

static QImage convertWithPalette(const QImage &src, QImage::Format format,
   const QVector<QRgb> &clut)
{
   QImage dest(src.size(), format);

   dest.setColorTable(clut);

   QString textsKeys = src.text();
   QStringList textKeyList = textsKeys.split('\n', QStringParser::SkipEmptyParts);

   for (const QString &textKey : textKeyList) {
      QStringList textKeySplitted = textKey.split(QLatin1String(": "));
      dest.setText(textKeySplitted[0], textKeySplitted[1]);
   }

   int h = src.height();
   int w = src.width();

   QHash<QRgb, int> cache;

   if (format == QImage::Format_Indexed8) {
      for (int y = 0; y < h; ++y) {
         const QRgb *src_pixels = (const QRgb *) src.scanLine(y);
         uchar *dest_pixels = (uchar *) dest.scanLine(y);
         for (int x = 0; x < w; ++x) {
            int src_pixel = src_pixels[x];
            int value = cache.value(src_pixel, -1);
            if (value == -1) {
               value = closestMatch(src_pixel, clut);
               cache.insert(src_pixel, value);
            }
            dest_pixels[x] = (uchar) value;
         }
      }

   } else {
      QVector<QRgb> table = clut;
      table.resize(2);
      for (int y = 0; y < h; ++y) {
         const QRgb *src_pixels = (const QRgb *) src.scanLine(y);
         for (int x = 0; x < w; ++x) {
            int src_pixel = src_pixels[x];
            int value = cache.value(src_pixel, -1);
            if (value == -1) {
               value = closestMatch(src_pixel, table);
               cache.insert(src_pixel, value);
            }
            dest.setPixel(x, y, value);
         }
      }
   }

   return dest;
}

QImage QImage::convertToFormat(Format format, const QVector<QRgb> &colorTable, Qt::ImageConversionFlags flags) const
{
   if (! d || d->format == format) {
      return *this;
   }

   if (format <= QImage::Format_Indexed8 && depth() == 32) {
      return convertWithPalette(*this, format, colorTable);
   }

   const Image_Converter *converterPtr = &(QImageConversions::instance().image_converter_map[d->format][format]);
   Image_Converter converter = *converterPtr;

   if (! converter) {
      return QImage();
   }

   QImage image(d->width, d->height, format);
   QIMAGE_SANITYCHECK_MEMORY(image);

   image.d->offset = offset();
   copyMetadata(image.d, d);

   converter(image.d, d, flags);

   return image;
}

bool QImage::valid(int x, int y) const
{
   return d
      && x >= 0 && x < d->width
      && y >= 0 && y < d->height;
}

int QImage::pixelIndex(int x, int y) const
{
   if (!d || x < 0 || x >= d->width || y < 0 || y >= height()) {
      qWarning("QImage::pixelIndex() Coordinate (%d,%d) is out of range", x, y);
      return -12345;
   }

   const uchar *s = scanLine(y);

   switch (d->format) {
      case Format_Mono:
         return (*(s + (x >> 3)) >> (7 - (x & 7))) & 1;
      case Format_MonoLSB:
         return (*(s + (x >> 3)) >> (x & 7)) & 1;
      case Format_Indexed8:
         return (int)s[x];
      default:
         qWarning("QImage::pixelIndex() No palette for %d bits per pixel images", d->depth);
   }
   return 0;
}

QRgb QImage::pixel(int x, int y) const
{
   if (! d || x < 0 || x >= d->width || y < 0 || y >= d->height) {
      qWarning("QImage::pixel() Coordinate (%d,%d) is out of range", x, y);
      return 12345;
   }

   const uchar *s = d->data + y * d->bytes_per_line;
   int index = -1;

   switch (d->format) {
      case Format_Mono:
         index = (*(s + (x >> 3)) >> (~x & 7)) & 1;
         break;

      case Format_MonoLSB:
         index = (*(s + (x >> 3)) >> (x & 7)) & 1;
         break;

      case Format_Indexed8:
         index = s[x];
         break;

      default:
         break;
   }

   if (index >= 0) {    // Indexed format
      if (index >= d->colortable.size()) {
         qWarning("QImage::pixel() Color table index %d is out of range.", index);
         return 0;
      }
      return d->colortable.at(index);
   }

   switch (d->format) {
      case Format_RGB32:
         return 0xff000000 | reinterpret_cast<const QRgb *>(s)[x];
      case Format_ARGB32: // Keep old behaviour.
      case Format_ARGB32_Premultiplied:
         return reinterpret_cast<const QRgb *>(s)[x];
      case Format_RGBX8888:
      case Format_RGBA8888: // Match ARGB32 behavior.
      case Format_RGBA8888_Premultiplied:
         return RGBA2ARGB(reinterpret_cast<const quint32 *>(s)[x]);
      case Format_BGR30:
      case Format_A2BGR30_Premultiplied:
         return qConvertA2rgb30ToArgb32<PixelOrderBGR>(reinterpret_cast<const quint32 *>(s)[x]);
      case Format_RGB30:
      case Format_A2RGB30_Premultiplied:
         return qConvertA2rgb30ToArgb32<PixelOrderRGB>(reinterpret_cast<const quint32 *>(s)[x]);
      case Format_RGB16:
         return qConvertRgb16To32(reinterpret_cast<const quint16 *>(s)[x]);
      default:
         break;
   }

   const QPixelLayout *layout = &qPixelLayouts[d->format];
   uint result;
   const uint *ptr = qFetchPixels[layout->bpp](&result, s, x, 1);

   return *layout->convertToARGB32PM(&result, ptr, 1, layout, nullptr);
}

void QImage::setPixel(int x, int y, uint index_or_rgb)
{
   if (!d || x < 0 || x >= width() || y < 0 || y >= height()) {
      qWarning("QImage::setPixel() Coordinate (%d,%d) is out of range", x, y);
      return;
   }

   // detach is called from within scanLine
   uchar *s = scanLine(y);

   switch (d->format) {
      case Format_Mono:
      case Format_MonoLSB:
         if (index_or_rgb > 1) {
            qWarning("QImage::setPixel() Index %d is out of range", index_or_rgb);

         } else if (format() == Format_MonoLSB) {
            if (index_or_rgb == 0) {
               *(s + (x >> 3)) &= ~(1 << (x & 7));
            } else {
               *(s + (x >> 3)) |= (1 << (x & 7));
            }
         } else {
            if (index_or_rgb == 0) {
               *(s + (x >> 3)) &= ~(1 << (7 - (x & 7)));
            } else {
               *(s + (x >> 3)) |= (1 << (7 - (x & 7)));
            }
         }
         return;

      case Format_Indexed8:
         if (index_or_rgb >= (uint)d->colortable.size()) {
            qWarning("QImage::setPixel() Index %d is out of range", index_or_rgb);
            return;
         }
         s[x] = index_or_rgb;
         return;

      case Format_RGB32:
         //make sure alpha is 255, we depend on it in qdrawhelper for cases
         // when image is set as a texture pattern on a qbrush
         ((uint *)s)[x] = 0xff000000 | index_or_rgb;
         return;

      case Format_ARGB32:
      case Format_ARGB32_Premultiplied:
         ((uint *)s)[x] = index_or_rgb;
         return;

      case Format_RGB16:
         ((quint16 *)s)[x] = qConvertRgb32To16(qUnpremultiply(index_or_rgb));
         return;

      case Format_RGBX8888:
         ((uint *)s)[x] = ARGB2RGBA(0xff000000 | index_or_rgb);
         return;
      case Format_RGBA8888:
      case Format_RGBA8888_Premultiplied:
         ((uint *)s)[x] = ARGB2RGBA(index_or_rgb);
         return;
      case Format_BGR30:
         ((uint *)s)[x] = qConvertRgb32ToRgb30<PixelOrderBGR>(index_or_rgb);
         return;
      case Format_A2BGR30_Premultiplied:
         ((uint *)s)[x] = qConvertArgb32ToA2rgb30<PixelOrderBGR>(index_or_rgb);
         return;
      case Format_RGB30:
         ((uint *)s)[x] = qConvertRgb32ToRgb30<PixelOrderRGB>(index_or_rgb);
         return;
      case Format_A2RGB30_Premultiplied:
         ((uint *)s)[x] = qConvertArgb32ToA2rgb30<PixelOrderRGB>(index_or_rgb);
         return;
      case Format_Invalid:
      case NImageFormats:
         Q_ASSERT(false);
         return;
      default:
         break;
   }
   const QPixelLayout *layout = &qPixelLayouts[d->format];
   uint result;
   const uint *ptr = layout->convertFromARGB32PM(&result, &index_or_rgb, 1, layout, nullptr);
   qStorePixels[layout->bpp](s, ptr, x, 1);
}

QColor QImage::pixelColor(int x, int y) const
{
   if (!d || x < 0 || x >= d->width || y < 0 || y >= height()) {
      qWarning("QImage::pixelColor() Coordinate (%d,%d) is out of range", x, y);
      return QColor();
   }

   QRgba64 c;
   const uchar *s = constScanLine(y);
   switch (d->format) {
      case Format_BGR30:
      case Format_A2BGR30_Premultiplied:
         c = qConvertA2rgb30ToRgb64<PixelOrderBGR>(reinterpret_cast<const quint32 *>(s)[x]);
         break;
      case Format_RGB30:
      case Format_A2RGB30_Premultiplied:
         c = qConvertA2rgb30ToRgb64<PixelOrderRGB>(reinterpret_cast<const quint32 *>(s)[x]);
         break;
      default:
         c = QRgba64::fromArgb32(pixel(x, y));
         break;
   }
   // QColor is always unpremultiplied
   if (hasAlphaChannel() && qPixelLayouts[d->format].premultiplied) {
      c = c.unpremultiplied();
   }
   return QColor(c);
}
void QImage::setPixelColor(int x, int y, const QColor &color)
{
   if (!d || x < 0 || x >= width() || y < 0 || y >= height() || !color.isValid()) {
      qWarning("QImage::setPixelColor() Coordinate (%d,%d) is out of range", x, y);
      return;
   }

   // QColor is always unpremultiplied
   QRgba64 c = color.rgba64();

   if (! hasAlphaChannel()) {
      c.setAlpha(65535);
   } else if (qPixelLayouts[d->format].premultiplied) {
      c = c.premultiplied();
   }

   // detach is called from within scanLine
   uchar *s = scanLine(y);
   switch (d->format) {
      case Format_Mono:
      case Format_MonoLSB:
      case Format_Indexed8:
         qWarning("QImage::setPixelColor() Unable to set the color for a monochrome or indexed format");
         return;

      case Format_BGR30:
         ((uint *)s)[x] = qConvertRgb64ToRgb30<PixelOrderBGR>(c) | 0xc0000000;
         return;

      case Format_A2BGR30_Premultiplied:
         ((uint *)s)[x] = qConvertRgb64ToRgb30<PixelOrderBGR>(c);
         return;

      case Format_RGB30:
         ((uint *)s)[x] = qConvertRgb64ToRgb30<PixelOrderRGB>(c) | 0xc0000000;
         return;

      case Format_A2RGB30_Premultiplied:
         ((uint *)s)[x] = qConvertRgb64ToRgb30<PixelOrderRGB>(c);
         return;

      default:
         setPixel(x, y, c.toArgb32());
         return;
   }
}

bool QImage::allGray() const
{
   if (! d) {
      return true;
   }

   switch (d->format) {

      case Format_Mono:
      case Format_MonoLSB:
      case Format_Indexed8:
         for (int i = 0; i < d->colortable.size(); ++i) {
            if (!qIsGray(d->colortable.at(i))) {
               return false;
            }
         }
         return true;
      case Format_Alpha8:
         return false;
      case Format_Grayscale8:
         return true;
      case Format_RGB32:
      case Format_ARGB32:
      case Format_ARGB32_Premultiplied:

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      case Format_RGBX8888:
      case Format_RGBA8888:
      case Format_RGBA8888_Premultiplied:
#endif

         for (int j = 0; j < d->height; ++j) {
            const QRgb *b = (const QRgb *)constScanLine(j);
            for (int i = 0; i < d->width; ++i) {
               if (!qIsGray(b[i])) {
                  return false;
               }
            }
         }
         return true;
      case Format_RGB16:
         for (int j = 0; j < d->height; ++j) {
            const quint16 *b = (const quint16 *)constScanLine(j);
            for (int i = 0; i < d->width; ++i) {
               if (!qIsGray(qConvertRgb16To32(b[i]))) {
                  return false;
               }
            }
         }
         return true;
      default:
         break;
   }

   const int buffer_size = 2048;
   uint buffer[buffer_size];
   const QPixelLayout *layout = &qPixelLayouts[d->format];
   FetchPixelsFunc fetch = qFetchPixels[layout->bpp];
   for (int j = 0; j < d->height; ++j) {
      const uchar *b = constScanLine(j);
      int x = 0;
      while (x < d->width) {
         int l = qMin(d->width - x, buffer_size);
         const uint *ptr = fetch(buffer, b, x, l);
         ptr = layout->convertToARGB32PM(buffer, ptr, l, layout, nullptr);
         for (int i = 0; i < l; ++i) {
            if (!qIsGray(ptr[i])) {
               return false;
            }
         }
         x += l;
      }
   }
   return true;
}

bool QImage::isGrayscale() const
{
   if (!d) {
      return false;
   }

   if (d->format == QImage::Format_Alpha8) {
      return false;
   }

   if (d->format == QImage::Format_Grayscale8) {
      return true;
   }

   switch (depth()) {
      case 32:
      case 24:
      case 16:
         return allGray();
      case 8: {
         Q_ASSERT(d->format == QImage::Format_Indexed8);

         for (int i = 0; i < colorCount(); i++)
            if (d->colortable.at(i) != qRgb(i, i, i)) {
               return false;
            }
         return true;
      }
   }

   return false;
}

QImage QImage::scaled(const QSize &s, Qt::AspectRatioMode aspectMode, Qt::TransformationMode mode) const
{
   if (!d) {
      qWarning("QImage::scaled() Image is empty");
      return QImage();
   }

   if (s.isEmpty()) {
      return QImage();
   }

   QSize newSize = size();
   newSize.scale(s, aspectMode);
   newSize.rwidth() = qMax(newSize.width(), 1);
   newSize.rheight() = qMax(newSize.height(), 1);
   if (newSize == size()) {
      return *this;
   }

   QTransform wm = QTransform::fromScale((qreal)newSize.width() / width(), (qreal)newSize.height() / height());
   QImage img = transformed(wm, mode);

   return img;
}

QImage QImage::scaledToWidth(int w, Qt::TransformationMode mode) const
{
   if (!d) {
      qWarning("QImage::scaleWidth() Image is empty");
      return QImage();
   }

   if (w <= 0) {
      return QImage();
   }

   qreal factor = (qreal) w / width();
   QTransform wm = QTransform::fromScale(factor, factor);
   return transformed(wm, mode);
}

QImage QImage::scaledToHeight(int h, Qt::TransformationMode mode) const
{
   if (!d) {
      qWarning("QImage::scaleHeight() Image is empty");
      return QImage();
   }

   if (h <= 0) {
      return QImage();
   }

   qreal factor = (qreal) h / height();
   QTransform wm = QTransform::fromScale(factor, factor);
   return transformed(wm, mode);
}

QMatrix QImage::trueMatrix(const QMatrix &matrix, int w, int h)
{
   return trueMatrix(QTransform(matrix), w, h).toAffine();
}

QImage QImage::transformed(const QMatrix &matrix, Qt::TransformationMode mode) const
{
   return transformed(QTransform(matrix), mode);
}

QImage QImage::createAlphaMask(Qt::ImageConversionFlags flags) const
{
   if (!d || d->format == QImage::Format_RGB32) {
      return QImage();
   }

   if (d->depth == 1) {
      // A monochrome pixmap, with alpha channels on those two colors.
      // Pretty unlikely, so use less efficient solution.
      return convertToFormat(Format_Indexed8, flags).createAlphaMask(flags);
   }

   QImage mask(d->width, d->height, Format_MonoLSB);
   if (! mask.isNull()) {
      dither_to_Mono(mask.d, d, flags, true);
   }
   return mask;
}

#ifndef QT_NO_IMAGE_HEURISTIC_MASK

QImage QImage::createHeuristicMask(bool clipTight) const
{
   if (!d) {
      return QImage();
   }

   if (d->depth != 32) {
      QImage img32 = convertToFormat(Format_RGB32);
      return img32.createHeuristicMask(clipTight);
   }

#define PIX(x,y)  (*((const QRgb*)scanLine(y)+x) & 0x00ffffff)

   int w = width();
   int h = height();
   QImage m(w, h, Format_MonoLSB);
   QIMAGE_SANITYCHECK_MEMORY(m);
   m.setColorCount(2);
   m.setColor(0, QColor(Qt::color0).rgba());
   m.setColor(1, QColor(Qt::color1).rgba());
   m.fill(0xff);

   QRgb background = PIX(0, 0);
   if (background != PIX(w - 1, 0) &&
      background != PIX(0, h - 1) &&
      background != PIX(w - 1, h - 1)) {
      background = PIX(w - 1, 0);
      if (background != PIX(w - 1, h - 1) &&
         background != PIX(0, h - 1) &&
         PIX(0, h - 1) == PIX(w - 1, h - 1)) {
         background = PIX(w - 1, h - 1);
      }
   }

   int x, y;
   bool done = false;
   uchar *ypp, *ypc, *ypn;

   while (!done) {
      done = true;
      ypn = m.scanLine(0);
      ypc = nullptr;

      for (y = 0; y < h; y++) {
         ypp = ypc;
         ypc = ypn;
         ypn = (y == h - 1) ? nullptr : m.scanLine(y + 1);
         const QRgb *p = (const QRgb *)scanLine(y);

         for (x = 0; x < w; x++) {
            // slowness here - it's possible to do six of these tests
            // together in one go. oh well.
            if ((x == 0 || y == 0 || x == w - 1 || y == h - 1 ||
                  !(*(ypc + ((x - 1) >> 3)) & (1 << ((x - 1) & 7))) ||
                  !(*(ypc + ((x + 1) >> 3)) & (1 << ((x + 1) & 7))) ||
                  !(*(ypp + (x     >> 3)) & (1 << (x     & 7))) ||
                  !(*(ypn + (x     >> 3)) & (1 << (x     & 7)))) &&
               (       (*(ypc + (x     >> 3)) & (1 << (x     & 7)))) &&
               ((*p & 0x00ffffff) == background)) {
               done = false;
               *(ypc + (x >> 3)) &= ~(1 << (x & 7));
            }
            p++;
         }
      }
   }

   if (!clipTight) {
      ypn = m.scanLine(0);
      ypc = nullptr;

      for (y = 0; y < h; y++) {
         ypp = ypc;
         ypc = ypn;
         ypn = (y == h - 1) ? nullptr : m.scanLine(y + 1);
         const QRgb *p = (const QRgb *)scanLine(y);

         for (x = 0; x < w; x++) {
            if ((*p & 0x00ffffff) != background) {
               if (x > 0) {
                  *(ypc + ((x - 1) >> 3)) |= (1 << ((x - 1) & 7));
               }
               if (x < w - 1) {
                  *(ypc + ((x + 1) >> 3)) |= (1 << ((x + 1) & 7));
               }
               if (y > 0) {
                  *(ypp + (x >> 3)) |= (1 << (x & 7));
               }
               if (y < h - 1) {
                  *(ypn + (x >> 3)) |= (1 << (x & 7));
               }
            }
            p++;
         }
      }
   }

#undef PIX

   return m;
}
#endif //QT_NO_IMAGE_HEURISTIC_MASK

QImage QImage::createMaskFromColor(QRgb color, Qt::MaskMode mode) const
{
   if (! d) {
      return QImage();
   }
   QImage maskImage(size(), QImage::Format_MonoLSB);
   QIMAGE_SANITYCHECK_MEMORY(maskImage);
   maskImage.fill(0);
   uchar *s = maskImage.bits();

   if (depth() == 32) {
      for (int h = 0; h < d->height; h++) {
         const uint *sl = (const uint *) scanLine(h);
         for (int w = 0; w < d->width; w++) {
            if (sl[w] == color) {
               *(s + (w >> 3)) |= (1 << (w & 7));
            }
         }
         s += maskImage.bytesPerLine();
      }
   } else {
      for (int h = 0; h < d->height; h++) {
         for (int w = 0; w < d->width; w++) {
            if ((uint) pixel(w, h) == color) {
               *(s + (w >> 3)) |= (1 << (w & 7));
            }
         }
         s += maskImage.bytesPerLine();
      }
   }

   if  (mode == Qt::MaskOutColor) {
      maskImage.invertPixels();
   }

   return maskImage;
}

template <class T>
inline void do_mirror_data(QImageData *dst, QImageData *src,
   int dstX0, int dstY0, int dstXIncr, int dstYIncr, int w, int h)
{
   if (dst == src) {
      // When mirroring in-place, stop in the middle for one of the directions, since we
      // are swapping the bytes instead of merely copying.
      const int srcXEnd = (dstX0 && !dstY0) ? w / 2 : w;
      const int srcYEnd = dstY0 ? h / 2 : h;

      for (int srcY = 0, dstY = dstY0; srcY < srcYEnd; ++srcY, dstY += dstYIncr) {
         T *srcPtr = (T *) (src->data + srcY * src->bytes_per_line);
         T *dstPtr = (T *) (dst->data + dstY * dst->bytes_per_line);
         for (int srcX = 0, dstX = dstX0; srcX < srcXEnd; ++srcX, dstX += dstXIncr) {
            std::swap(srcPtr[srcX], dstPtr[dstX]);
         }
      }
      if (dstX0 && dstY0 && (h & 1)) {
         int srcY = h / 2;
         int srcXEnd2 = w / 2;
         T *srcPtr = (T *) (src->data + srcY * src->bytes_per_line);
         for (int srcX = 0, dstX = dstX0; srcX < srcXEnd2; ++srcX, dstX += dstXIncr) {
            std::swap(srcPtr[srcX], srcPtr[dstX]);
         }
      }

   } else {
      for (int srcY = 0, dstY = dstY0; srcY < h; ++srcY, dstY += dstYIncr) {
         T *srcPtr = (T *) (src->data + srcY * src->bytes_per_line);
         T *dstPtr = (T *) (dst->data + dstY * dst->bytes_per_line);
         for (int srcX = 0, dstX = dstX0; srcX < w; ++srcX, dstX += dstXIncr) {
            dstPtr[dstX] = srcPtr[srcX];
         }
      }
   }
}

inline void do_mirror(QImageData *dst, QImageData *src, bool horizontal, bool vertical)
{
   Q_ASSERT(src->width == dst->width && src->height == dst->height && src->depth == dst->depth);
   int w = src->width;
   int h = src->height;
   int depth = src->depth;

   if (src->depth == 1) {
      w = (w + 7) / 8; // byte aligned width
      depth = 8;
   }

   int dstX0 = 0, dstXIncr = 1;
   int dstY0 = 0, dstYIncr = 1;
   if (horizontal) {
      // 0 -> w-1, 1 -> w-2, 2 -> w-3, ...
      dstX0 = w - 1;
      dstXIncr = -1;
   }
   if (vertical) {
      // 0 -> h-1, 1 -> h-2, 2 -> h-3, ...
      dstY0 = h - 1;
      dstYIncr = -1;
   }

   switch (depth) {
      case 32:
         do_mirror_data<quint32>(dst, src, dstX0, dstY0, dstXIncr, dstYIncr, w, h);
         break;
      case 24:
         do_mirror_data<quint24>(dst, src, dstX0, dstY0, dstXIncr, dstYIncr, w, h);
         break;
      case 16:
         do_mirror_data<quint16>(dst, src, dstX0, dstY0, dstXIncr, dstYIncr, w, h);
         break;
      case 8:
         do_mirror_data<quint8>(dst, src, dstX0, dstY0, dstXIncr, dstYIncr, w, h);
         break;
      default:
         Q_ASSERT(false);
         break;
   }

   // The bytes are now all in the correct place. In addition, the bits in the individual
   // bytes have to be flipped too when horizontally mirroring a 1 bit-per-pixel image.
   if (horizontal && dst->depth == 1) {
      Q_ASSERT(dst->format == QImage::Format_Mono || dst->format == QImage::Format_MonoLSB);
      const int shift = 8 - (dst->width % 8);
      const uchar *bitflip = qt_get_bitflip_array();
      for (int y = 0; y < h; ++y) {
         uchar *begin = dst->data + y * dst->bytes_per_line;
         uchar *end = begin + dst->bytes_per_line;
         for (uchar *p = begin; p < end; ++p) {
            *p = bitflip[*p];
            // When the data is non-byte aligned, an extra bit shift (of the number of
            // unused bits at the end) is needed for the entire scanline.
            if (shift != 8 && p != begin) {
               if (dst->format == QImage::Format_Mono) {
                  for (int i = 0; i < shift; ++i) {
                     p[-1] <<= 1;
                     p[-1] |= (*p & (128 >> i)) >> (7 - i);
                  }
               } else {
                  for (int i = 0; i < shift; ++i) {
                     p[-1] >>= 1;
                     p[-1] |= (*p & (1 << i)) << (7 - i);
                  }
               }
            }
         }
         if (shift != 8) {
            if (dst->format == QImage::Format_Mono) {
               end[-1] <<= shift;
            } else {
               end[-1] >>= shift;
            }
         }
      }
   }
}

QImage QImage::mirrored_helper(bool horizontal, bool vertical) const
{
   if (!d) {
      return QImage();
   }

   if ((d->width <= 1 && d->height <= 1) || (!horizontal && !vertical)) {
      return *this;
   }

   // Create result image, copy colormap
   QImage result(d->width, d->height, d->format);
   QIMAGE_SANITYCHECK_MEMORY(result);

   // check if we ran out of of memory..
   if (!result.d) {
      return QImage();
   }

   result.d->colortable = d->colortable;
   result.d->has_alpha_clut = d->has_alpha_clut;

   copyMetadata(result.d, d);

   do_mirror(result.d, d, horizontal, vertical);

   return result;
}

void QImage::mirrored_inplace(bool horizontal, bool vertical)
{
   if (!d || (d->width <= 1 && d->height <= 1) || (!horizontal && !vertical)) {
      return;
   }

   detach();
   if (!d->own_data) {
      *this = copy();
   }

   do_mirror(d, d, horizontal, vertical);
}

inline void rgbSwapped_generic(int width, int height, const QImage *src, QImage *dst, const QPixelLayout *layout)
{
   Q_ASSERT(layout->redWidth == layout->blueWidth);
   FetchPixelsFunc fetch = qFetchPixels[layout->bpp];
   StorePixelsFunc store = qStorePixels[layout->bpp];

   const uint redBlueMask = (1 << layout->redWidth) - 1;
   const uint alphaGreenMask = (((1 << layout->alphaWidth) - 1) << layout->alphaShift)
      | (((1 << layout->greenWidth) - 1) << layout->greenShift);

   const int buffer_size = 2048;
   uint buffer[buffer_size];
   for (int i = 0; i < height; ++i) {
      uchar *q = dst->scanLine(i);
      const uchar *p = src->constScanLine(i);
      int x = 0;
      while (x < width) {
         int l = qMin(width - x, buffer_size);
         const uint *ptr = fetch(buffer, p, x, l);
         for (int j = 0; j < l; ++j) {
            uint red = (ptr[j] >> layout->redShift) & redBlueMask;
            uint blue = (ptr[j] >> layout->blueShift) & redBlueMask;
            buffer[j] = (ptr[j] & alphaGreenMask)
               | (red << layout->blueShift)
               | (blue << layout->redShift);
         }
         store(q, buffer, x, l);
         x += l;
      }
   }
}

QImage QImage::rgbSwapped_helper() const
{
   if (isNull()) {
      return *this;
   }

   QImage res;
   switch (d->format) {
      case Format_Invalid:
      case NImageFormats:
         Q_ASSERT(false);
         break;
      case Format_Alpha8:
      case Format_Grayscale8:
         return *this;
      case Format_Mono:
      case Format_MonoLSB:
      case Format_Indexed8:
         res = copy();

         for (int i = 0; i < res.d->colortable.size(); i++) {
            QRgb c = res.d->colortable.at(i);
            res.d->colortable[i] = QRgb(((c << 16) & 0xff0000) | ((c >> 16) & 0xff) | (c & 0xff00ff00));
         }
         break;
      case Format_RGB32:
      case Format_ARGB32:
      case Format_ARGB32_Premultiplied:

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      case Format_RGBX8888:
      case Format_RGBA8888:
      case Format_RGBA8888_Premultiplied:
#endif
         res = QImage(d->width, d->height, d->format);
         QIMAGE_SANITYCHECK_MEMORY(res);

         for (int i = 0; i < d->height; i++) {
            uint *q = (uint *)res.scanLine(i);
            const uint *p = (const uint *)constScanLine(i);
            const uint *end = p + d->width;
            while (p < end) {
               uint c = *p;
               *q = ((c << 16) & 0xff0000) | ((c >> 16) & 0xff) | (c & 0xff00ff00);
               p++;
               q++;
            }
         }
         break;

      case Format_RGB16:
         res = QImage(d->width, d->height, d->format);
         QIMAGE_SANITYCHECK_MEMORY(res);
         for (int i = 0; i < d->height; i++) {
            ushort *q = (ushort *)res.scanLine(i);
            const ushort *p = (const ushort *)constScanLine(i);
            const ushort *end = p + d->width;

            while (p < end) {
               ushort c = *p;
               *q = ((c << 11) & 0xf800) | ((c >> 11) & 0x1f) | (c & 0x07e0);
               p++;
               q++;
            }
         }
         break;
      case Format_BGR30:
      case Format_A2BGR30_Premultiplied:
      case Format_RGB30:
      case Format_A2RGB30_Premultiplied:
         res = QImage(d->width, d->height, d->format);
         QIMAGE_SANITYCHECK_MEMORY(res);
         for (int i = 0; i < d->height; i++) {
            uint *q = (uint *)res.scanLine(i);
            const uint *p = (const uint *)constScanLine(i);
            const uint *end = p + d->width;

            while (p < end) {
               *q = qRgbSwapRgb30(*p);
               p++;
               q++;

            }
         }
         break;
      default:
         res = QImage(d->width, d->height, d->format);
         rgbSwapped_generic(d->width, d->height, this, &res, &qPixelLayouts[d->format]);
         break;
   }
   copyMetadata(res.d, d);
   return res;
}

void QImage::rgbSwapped_inplace()
{
   if (isNull()) {
      return;
   }

   detach();
   if (!d->own_data) {
      *this = copy();
   }

   switch (d->format) {
      case Format_Invalid:
      case NImageFormats:
         Q_ASSERT(false);
         break;
      case Format_Alpha8:
      case Format_Grayscale8:
         return;
      case Format_Mono:
      case Format_MonoLSB:
      case Format_Indexed8:
         for (int i = 0; i < d->colortable.size(); i++) {
            QRgb c = d->colortable.at(i);
            d->colortable[i] = QRgb(((c << 16) & 0xff0000) | ((c >> 16) & 0xff) | (c & 0xff00ff00));
         }
         break;

      case Format_RGB32:
      case Format_ARGB32:
      case Format_ARGB32_Premultiplied:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      case Format_RGBX8888:
      case Format_RGBA8888:
      case Format_RGBA8888_Premultiplied:
#endif
         for (int i = 0; i < d->height; i++) {
            uint *p = (uint *)scanLine(i);
            uint *end = p + d->width;
            while (p < end) {
               uint c = *p;
               *p = ((c << 16) & 0xff0000) | ((c >> 16) & 0xff) | (c & 0xff00ff00);
               p++;
            }
         }
         break;
      case Format_RGB16:
         for (int i = 0; i < d->height; i++) {
            ushort *p = (ushort *)scanLine(i);
            ushort *end = p + d->width;
            while (p < end) {
               ushort c = *p;
               *p = ((c << 11) & 0xf800) | ((c >> 11) & 0x1f) | (c & 0x07e0);
               p++;
            }
         }
         break;
      case Format_BGR30:
      case Format_A2BGR30_Premultiplied:
      case Format_RGB30:
      case Format_A2RGB30_Premultiplied:
         for (int i = 0; i < d->height; i++) {
            uint *p = (uint *)scanLine(i);
            uint *end = p + d->width;
            while (p < end) {
               *p = qRgbSwapRgb30(*p);
               p++;
            }
         }
         break;
      default:
         rgbSwapped_generic(d->width, d->height, this, this, &qPixelLayouts[d->format]);
         break;
   }
}

bool QImage::load(const QString &fileName, const QString &format)
{
   QImage image = QImageReader(fileName, format).read();

   operator=(image);
   return !isNull();
}

bool QImage::load(QIODevice *device, const QString &format)
{
   QImage image = QImageReader(device, format).read();

   operator=(image);
   return !isNull();
}

bool QImage::loadFromData(const uchar *data, int len, const QString &format)
{
   QImage image = fromData(data, len, format);

   operator=(image);
   return !isNull();
}

QImage QImage::fromData(const uchar *data, int size, const QString &format)
{
   QByteArray a = QByteArray::fromRawData(reinterpret_cast<const char *>(data), size);
   QBuffer b;
   b.setData(a);
   b.open(QIODevice::ReadOnly);
   return QImageReader(&b, format).read();
}

bool QImage::save(const QString &fileName, const QString &format, int quality) const
{
   if (isNull()) {
      return false;
   }

   QImageWriter writer(fileName, format);
   return d->doImageIO(this, &writer, quality);
}

bool QImage::save(QIODevice *device, const QString &format, int quality) const
{
   if (isNull()) {
      return false;   // nothing to save
   }

   QImageWriter writer(device, format);
   return d->doImageIO(this, &writer, quality);
}

// internal
bool QImageData::doImageIO(const QImage *image, QImageWriter *writer, int quality) const
{
   if (quality > 100  || quality < -1) {
      qWarning("QPixmap::save() Quality setting is out of range [-1, 100]");
   }

   if (quality >= 0) {
      writer->setQuality(qMin(quality, 100));
   }

   return writer->write(*image);
}

QDataStream &operator<<(QDataStream &s, const QImage &image)
{
   if (image.isNull()) {
      s << (qint32) 0; // null image marker
      return s;

   } else {
      s << (qint32) 1;
      // continue
   }

   QImageWriter writer(s.device(), "png");
   writer.write(image);
   return s;
}

QDataStream &operator>>(QDataStream &s, QImage &image)
{
   qint32 nullMarker;
   s >> nullMarker;

   if (!nullMarker) {
      image = QImage(); // null image
      return s;
   }

   image = QImageReader(s.device(), QString()).read();
   return s;
}

bool QImage::operator==(const QImage &i) const
{
   // same object, or shared?
   if (i.d == d) {
      return true;
   }

   if (!i.d || !d) {
      return false;
   }

   // obviously different stuff?
   if (i.d->height != d->height || i.d->width != d->width || i.d->format != d->format) {
      return false;
   }

   if (d->format != Format_RGB32) {
      if (d->format >= Format_ARGB32) { // all bits defined
         const int n = d->width * d->depth / 8;
         if (n == d->bytes_per_line && n == i.d->bytes_per_line) {
            if (memcmp(bits(), i.bits(), d->nbytes)) {
               return false;
            }

         } else {
            for (int y = 0; y < d->height; ++y) {
               if (memcmp(scanLine(y), i.scanLine(y), n)) {
                  return false;
               }
            }
         }
      } else {
         const int w = width();
         const int h = height();
         const QVector<QRgb> &colortable = d->colortable;
         const QVector<QRgb> &icolortable = i.d->colortable;
         for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
               if (colortable[pixelIndex(x, y)] != icolortable[i.pixelIndex(x, y)]) {
                  return false;
               }
            }
         }
      }

   } else {
      //alpha channel undefined, so we must mask it out
      for (int l = 0; l < d->height; l++) {
         int w = d->width;
         const uint *p1 = reinterpret_cast<const uint *>(scanLine(l));
         const uint *p2 = reinterpret_cast<const uint *>(i.scanLine(l));
         while (w--) {
            if ((*p1++ & 0x00ffffff) != (*p2++ & 0x00ffffff)) {
               return false;
            }
         }
      }
   }

   return true;
}

bool QImage::operator!=(const QImage &i) const
{
   return !(*this == i);
}

int QImage::dotsPerMeterX() const
{
   return d ? qRound(d->dpmx) : 0;
}

int QImage::dotsPerMeterY() const
{
   return d ? qRound(d->dpmy) : 0;
}

void QImage::setDotsPerMeterX(int x)
{
   if (!d || !x) {
      return;
   }
   detach();

   if (d) {
      d->dpmx = x;
   }
}

void QImage::setDotsPerMeterY(int y)
{
   if (!d || !y) {
      return;
   }
   detach();

   if (d) {
      d->dpmy = y;
   }
}

QPoint QImage::offset() const
{
   return d ? d->offset : QPoint();
}

void QImage::setOffset(const QPoint &p)
{
   if (!d) {
      return;
   }
   detach();

   if (d) {
      d->offset = p;
   }
}

QStringList QImage::textKeys() const
{
   return d ? QStringList(d->text.keys()) : QStringList();
}

QString QImage::text(const QString &key) const
{
   if (!d) {
      return QString();
   }

   if (!key.isEmpty()) {
      return d->text.value(key);
   }

   QString tmp;
   for (const QString &key : d->text.keys()) {
      if (! tmp.isEmpty()) {
         tmp += QLatin1String("\n\n");
      }
      tmp += key + QLatin1String(": ") + d->text.value(key).simplified();
   }
   return tmp;
}

void QImage::setText(const QString &key, const QString &value)
{
   if (!d) {
      return;
   }
   detach();

   if (d) {
      d->text.insert(key, value);
   }
}

// internal
QPaintEngine *QImage::paintEngine() const
{
   if (!d) {
      return nullptr;
   }

   if (!d->paintEngine) {
      QPaintDevice *paintDevice = const_cast<QImage *>(this);
      QPaintEngine *paintEngine = nullptr;
      QPlatformIntegration *platformIntegration = QApplicationPrivate::platformIntegration();

      if (platformIntegration) {
         paintEngine = platformIntegration->createImagePaintEngine(paintDevice);
      }
      d->paintEngine = paintEngine ? paintEngine : new QRasterPaintEngine(paintDevice);
   }

   return d->paintEngine;
}

// internal
int QImage::metric(PaintDeviceMetric metric) const
{
   if (! d) {
      return 0;
   }

   switch (metric) {
      case PdmWidth:
         return d->width;

      case PdmHeight:
         return d->height;

      case PdmWidthMM:
         return qRound(d->width * 1000 / d->dpmx);

      case PdmHeightMM:
         return qRound(d->height * 1000 / d->dpmy);

      case PdmNumColors:
         return d->colortable.size();

      case PdmDepth:
         return d->depth;

      case PdmDpiX:
         return qRound(d->dpmx * 0.0254);

      case PdmDpiY:
         return qRound(d->dpmy * 0.0254);

      case PdmPhysicalDpiX:
         return qRound(d->dpmx * 0.0254);

      case PdmPhysicalDpiY:
         return qRound(d->dpmy * 0.0254);

      case PdmDevicePixelRatio:
         return d->devicePixelRatio;

      case PdmDevicePixelRatioScaled:
         return d->devicePixelRatio * QPaintDevice::devicePixelRatioFScale();


      default:
         qWarning("QImage::metric() Unhandled metric type %d", metric);
         break;
   }
   return 0;
}

#undef IWX_MSB
#define IWX_MSB(b)        if (trigx < maxws && trigy < maxhs) {                        \
                            if (*(sptr+sbpl*(trigy>>12)+(trigx>>15)) &                 \
                                 (1 << (7-((trigx>>12)&7))))                           \
                                *dptr |= b;                                            \
                        }                                                              \
                        trigx += m11;                                                  \
                        trigy += m12;
// END OF MACRO
#undef IWX_LSB
#define IWX_LSB(b)        if (trigx < maxws && trigy < maxhs) {                        \
                            if (*(sptr+sbpl*(trigy>>12)+(trigx>>15)) &                 \
                                 (1 << ((trigx>>12)&7)))                               \
                                *dptr |= b;                                            \
                        }                                                              \
                        trigx += m11;                                                  \
                        trigy += m12;
// END OF MACRO
#undef IWX_PIX
#define IWX_PIX(b)        if (trigx < maxws && trigy < maxhs) {                        \
                            if ((*(sptr+sbpl*(trigy>>12)+(trigx>>15)) &                \
                                 (1 << (7-((trigx>>12)&7)))) == 0)                     \
                                *dptr &= ~b;                                           \
                        }                                                              \
                        trigx += m11;                                                  \
                        trigy += m12;
// END OF MACRO
bool qt_xForm_helper(const QTransform &trueMat, int xoffset, int type, int depth,
      uchar *dptr, int dbpl, int p_inc, int dHeight, const uchar *sptr, int sbpl, int sWidth, int sHeight)
{
   int m11 = int(trueMat.m11() * 4096.0);
   int m12 = int(trueMat.m12() * 4096.0);
   int m21 = int(trueMat.m21() * 4096.0);
   int m22 = int(trueMat.m22() * 4096.0);
   int dx  = qRound(trueMat.dx() * 4096.0);
   int dy  = qRound(trueMat.dy() * 4096.0);

   int m21ydx = dx + (xoffset << 16) + (m11 + m21) / 2;
   int m22ydy = dy + (m12 + m22) / 2;
   uint trigx;
   uint trigy;
   uint maxws = sWidth << 12;
   uint maxhs = sHeight << 12;

   for (int y = 0; y < dHeight; y++) {            // for each target scanline
      trigx = m21ydx;
      trigy = m22ydy;
      uchar *maxp = dptr + dbpl;
      if (depth != 1) {
         switch (depth) {
            case 8:                                // 8 bpp transform
               while (dptr < maxp) {
                  if (trigx < maxws && trigy < maxhs) {
                     *dptr = *(sptr + sbpl * (trigy >> 12) + (trigx >> 12));
                  }
                  trigx += m11;
                  trigy += m12;
                  dptr++;
               }
               break;

            case 16:                        // 16 bpp transform
               while (dptr < maxp) {
                  if (trigx < maxws && trigy < maxhs)
                     *((ushort *)dptr) = *((const ushort *)(sptr + sbpl * (trigy >> 12) +
                              ((trigx >> 12) << 1)));
                  trigx += m11;
                  trigy += m12;
                  dptr++;
                  dptr++;
               }
               break;

            case 24:                        // 24 bpp transform
               while (dptr < maxp) {
                  if (trigx < maxws && trigy < maxhs) {
                     const uchar *p2 = sptr + sbpl * (trigy >> 12) + ((trigx >> 12) * 3);
                     dptr[0] = p2[0];
                     dptr[1] = p2[1];
                     dptr[2] = p2[2];
                  }
                  trigx += m11;
                  trigy += m12;
                  dptr += 3;
               }
               break;

            case 32:                        // 32 bpp transform
               while (dptr < maxp) {
                  if (trigx < maxws && trigy < maxhs)
                     *((uint *)dptr) = *((const uint *)(sptr + sbpl * (trigy >> 12) +
                              ((trigx >> 12) << 2)));
                  trigx += m11;
                  trigy += m12;
                  dptr += 4;
               }
               break;

            default: {
               return false;
            }
         }
      } else  {
         switch (type) {
            case QT_XFORM_TYPE_MSBFIRST:
               while (dptr < maxp) {
                  IWX_MSB(128);
                  IWX_MSB(64);
                  IWX_MSB(32);
                  IWX_MSB(16);
                  IWX_MSB(8);
                  IWX_MSB(4);
                  IWX_MSB(2);
                  IWX_MSB(1);
                  dptr++;
               }
               break;
            case QT_XFORM_TYPE_LSBFIRST:
               while (dptr < maxp) {
                  IWX_LSB(1);
                  IWX_LSB(2);
                  IWX_LSB(4);
                  IWX_LSB(8);
                  IWX_LSB(16);
                  IWX_LSB(32);
                  IWX_LSB(64);
                  IWX_LSB(128);
                  dptr++;
               }
               break;
         }
      }
      m21ydx += m21;
      m22ydy += m22;
      dptr += p_inc;
   }
   return true;
}
#undef IWX_MSB
#undef IWX_LSB
#undef IWX_PIX

qint64 QImage::cacheKey() const
{
   if (! d) {
      return 0;
   } else {
      return (((qint64) d->ser_no) << 32) | ((qint64) d->detach_no);
   }
}

// internal
bool QImage::isDetached() const
{
   return d && d->ref.load() == 1;
}

// obsolete
void QImage::setAlphaChannel(const QImage &alphaChannel)
{
   if (!d) {
      return;
   }

   int w = d->width;
   int h = d->height;

   if (w != alphaChannel.d->width || h != alphaChannel.d->height) {
      qWarning("QImage::setAlphaChannel() Alpha channel must have the same dimensions as the target image");
      return;
   }

   if (d->paintEngine && d->paintEngine->isActive()) {
      qWarning("QImage::setAlphaChannel() Unable to set alpha channel while the image is being painted");
      return;
   }

   if (d->format == QImage::Format_ARGB32_Premultiplied) {
      detach();
   } else {
      *this = convertToFormat(QImage::Format_ARGB32_Premultiplied);
   }

   if (isNull()) {
      return;
   }

   // Slight optimization since alphachannels are returned as 8-bit grays.
   if (alphaChannel.format() == QImage::Format_Alpha8 || ( alphaChannel.d->depth == 8 && alphaChannel.isGrayscale())) {
      const uchar *src_data = alphaChannel.d->data;
      uchar *dest_data = d->data;

      for (int y = 0; y < h; ++y) {
         const uchar *src = src_data;
         QRgb *dest = (QRgb *)dest_data;
         for (int x = 0; x < w; ++x) {
            int alpha = *src;
            int destAlpha = qt_div_255(alpha * qAlpha(*dest));
            *dest = ((destAlpha << 24)
                  | (qt_div_255(qRed(*dest) * alpha) << 16)
                  | (qt_div_255(qGreen(*dest) * alpha) << 8)
                  | (qt_div_255(qBlue(*dest) * alpha)));
            ++dest;
            ++src;
         }
         src_data += alphaChannel.d->bytes_per_line;
         dest_data += d->bytes_per_line;
      }

   } else {
      const QImage sourceImage = alphaChannel.convertToFormat(QImage::Format_RGB32);
      if (sourceImage.isNull()) {
         return;
      }

      const uchar *src_data = sourceImage.d->data;
      uchar *dest_data = d->data;

      for (int y = 0; y < h; ++y) {
         const QRgb *src = (const QRgb *) src_data;
         QRgb *dest = (QRgb *) dest_data;
         for (int x = 0; x < w; ++x) {
            int alpha = qGray(*src);
            int destAlpha = qt_div_255(alpha * qAlpha(*dest));
            *dest = ((destAlpha << 24)
                  | (qt_div_255(qRed(*dest) * alpha) << 16)
                  | (qt_div_255(qGreen(*dest) * alpha) << 8)
                  | (qt_div_255(qBlue(*dest) * alpha)));
            ++dest;
            ++src;
         }
         src_data += sourceImage.d->bytes_per_line;
         dest_data += d->bytes_per_line;
      }
   }
}

// obsolete
QImage QImage::alphaChannel() const
{
   if (! d) {
      return QImage();
   }

   int w = d->width;
   int h = d->height;

   QImage image(w, h, Format_Indexed8);

   image.setColorCount(256);

   // set up gray scale table.
   for (int i = 0; i < 256; ++i) {
      image.setColor(i, qRgb(i, i, i));
   }

   if (!hasAlphaChannel()) {
      image.fill(255);
      return image;
   }

   if (d->format == Format_Indexed8) {
      const uchar *src_data = d->data;
      uchar *dest_data = image.d->data;

      for (int y = 0; y < h; ++y) {
         const uchar *src = src_data;
         uchar *dest = dest_data;

         for (int x = 0; x < w; ++x) {
            *dest = qAlpha(d->colortable.at(*src));
            ++dest;
            ++src;
         }
         src_data += d->bytes_per_line;
         dest_data += image.d->bytes_per_line;
      }

   } else if (d->format == Format_Alpha8) {
      const uchar *src_data = d->data;
      uchar *dest_data = image.d->data;
      memcpy(dest_data, src_data, d->bytes_per_line * h);

   } else {
      QImage alpha32 = *this;
      bool canSkipConversion = (d->format == Format_ARGB32 || d->format == Format_ARGB32_Premultiplied);

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      canSkipConversion = canSkipConversion || (d->format == Format_RGBA8888 || d->format == Format_RGBA8888_Premultiplied);
#endif
      if (!canSkipConversion) {
         alpha32 = convertToFormat(Format_ARGB32);
      }

      const uchar *src_data = alpha32.d->data;
      uchar *dest_data = image.d->data;

      for (int y = 0; y < h; ++y) {
         const QRgb *src = (const QRgb *) src_data;
         uchar *dest = dest_data;

         for (int x = 0; x < w; ++x) {
            *dest = qAlpha(*src);
            ++dest;
            ++src;
         }
         src_data += alpha32.d->bytes_per_line;
         dest_data += image.d->bytes_per_line;
      }
   }

   return image;
}

bool QImage::hasAlphaChannel() const
{
   if (!d) {
      return false;
   }

   const QPixelFormat format = pixelFormat();

   if (format.alphaUsage() == QPixelFormat::UsesAlpha) {
      return true;
   }

   if (format.colorModel() == QPixelFormat::Indexed) {
      return d->has_alpha_clut;
   }

   return false;
}

int QImage::bitPlaneCount() const
{
   if (!d) {
      return 0;
   }
   int bpc = 0;
   switch (d->format) {
      case QImage::Format_Invalid:
         break;
      case QImage::Format_BGR30:
      case QImage::Format_RGB30:
         bpc = 30;
         break;
      case QImage::Format_RGB32:
      case QImage::Format_RGBX8888:
         bpc = 24;
         break;
      case QImage::Format_RGB666:
         bpc = 18;
         break;
      case QImage::Format_RGB555:
         bpc = 15;
         break;
      case QImage::Format_ARGB8555_Premultiplied:
         bpc = 23;
         break;
      case QImage::Format_RGB444:
         bpc = 12;
         break;
      default:
         bpc = qt_depthForFormat(d->format);
         break;
   }
   return bpc;
}

QImage QImage::smoothScaled(int w, int h) const
{
   QImage src = *this;

   switch (src.format()) {
      case QImage::Format_RGB32:
      case QImage::Format_ARGB32_Premultiplied:

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
      case QImage::Format_RGBX8888:
#endif

      case QImage::Format_RGBA8888_Premultiplied:
         break;

      default:
         if (src.hasAlphaChannel()) {
            src = src.convertToFormat(QImage::Format_ARGB32_Premultiplied);
         } else {
            src = src.convertToFormat(QImage::Format_RGB32);
         }
   }

   src = qSmoothScaleImage(src, w, h);

   if (!src.isNull()) {
      copyMetadata(src.d, d);
   }
   return src;

}

static QImage rotated90(const QImage &image)
{
   QImage out(image.height(), image.width(), image.format());
   out.setDotsPerMeterX(image.dotsPerMeterY());
   out.setDotsPerMeterY(image.dotsPerMeterX());

   if (image.colorCount() > 0) {
      out.setColorTable(image.colorTable());
   }

   int w = image.width();
   int h = image.height();
   switch (image.format()) {
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
         qt_memrotate270(reinterpret_cast<const quint32 *>(image.bits()),
            w, h, image.bytesPerLine(),
            reinterpret_cast<quint32 *>(out.bits()),
            out.bytesPerLine());
         break;

      case QImage::Format_RGB666:
      case QImage::Format_ARGB6666_Premultiplied:
      case QImage::Format_ARGB8565_Premultiplied:
      case QImage::Format_ARGB8555_Premultiplied:
      case QImage::Format_RGB888:
         qt_memrotate270(reinterpret_cast<const quint24 *>(image.bits()),
            w, h, image.bytesPerLine(),
            reinterpret_cast<quint24 *>(out.bits()),
            out.bytesPerLine());
         break;

      case QImage::Format_RGB555:
      case QImage::Format_RGB16:
      case QImage::Format_ARGB4444_Premultiplied:
         qt_memrotate270(reinterpret_cast<const quint16 *>(image.bits()),
            w, h, image.bytesPerLine(),
            reinterpret_cast<quint16 *>(out.bits()),
            out.bytesPerLine());
         break;
      case QImage::Format_Alpha8:
      case QImage::Format_Grayscale8:
      case QImage::Format_Indexed8:
         qt_memrotate270(reinterpret_cast<const quint8 *>(image.bits()),
            w, h, image.bytesPerLine(),
            reinterpret_cast<quint8 *>(out.bits()),
            out.bytesPerLine());
         break;
      default:
         for (int y = 0; y < h; ++y) {
            if (image.colorCount())
               for (int x = 0; x < w; ++x) {
                  out.setPixel(h - y - 1, x, image.pixelIndex(x, y));
               } else
               for (int x = 0; x < w; ++x) {
                  out.setPixel(h - y - 1, x, image.pixel(x, y));
               }
         }
         break;
   }
   return out;
}

static QImage rotated180(const QImage &image)
{
   return image.mirrored(true, true);
}

static QImage rotated270(const QImage &image)
{
   QImage out(image.height(), image.width(), image.format());
   out.setDotsPerMeterX(image.dotsPerMeterY());
   out.setDotsPerMeterY(image.dotsPerMeterX());

   if (image.colorCount() > 0) {
      out.setColorTable(image.colorTable());
   }
   int w = image.width();
   int h = image.height();
   switch (image.format()) {
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
         qt_memrotate90(reinterpret_cast<const quint32 *>(image.bits()),
            w, h, image.bytesPerLine(),
            reinterpret_cast<quint32 *>(out.bits()),
            out.bytesPerLine());
         break;
      case QImage::Format_RGB666:
      case QImage::Format_ARGB6666_Premultiplied:
      case QImage::Format_ARGB8565_Premultiplied:
      case QImage::Format_ARGB8555_Premultiplied:
      case QImage::Format_RGB888:
         qt_memrotate90(reinterpret_cast<const quint24 *>(image.bits()),
            w, h, image.bytesPerLine(),
            reinterpret_cast<quint24 *>(out.bits()),
            out.bytesPerLine());
         break;
      case QImage::Format_RGB555:
      case QImage::Format_RGB16:
      case QImage::Format_ARGB4444_Premultiplied:
         qt_memrotate90(reinterpret_cast<const quint16 *>(image.bits()),
            w, h, image.bytesPerLine(),
            reinterpret_cast<quint16 *>(out.bits()),
            out.bytesPerLine());
         break;
      case QImage::Format_Alpha8:
      case QImage::Format_Grayscale8:
      case QImage::Format_Indexed8:
         qt_memrotate90(reinterpret_cast<const quint8 *>(image.bits()),
            w, h, image.bytesPerLine(),
            reinterpret_cast<quint8 *>(out.bits()),
            out.bytesPerLine());
         break;
      default:
         for (int y = 0; y < h; ++y) {
            if (image.colorCount())
               for (int x = 0; x < w; ++x) {
                  out.setPixel(y, w - x - 1, image.pixelIndex(x, y));
               } else
               for (int x = 0; x < w; ++x) {
                  out.setPixel(y, w - x - 1, image.pixel(x, y));
               }
         }
         break;
   }
   return out;
}

QImage QImage::transformed(const QTransform &matrix, Qt::TransformationMode mode ) const
{
   if (! d) {
      return QImage();
   }

   // source image data
   int ws = width();
   int hs = height();

   // target image data
   int wd;
   int hd;

   // compute size of target image
   QTransform mat = trueMatrix(matrix, ws, hs);
   bool complex_xform = false;
   bool scale_xform = false;
   if (mat.type() <= QTransform::TxScale) {
      if (mat.type() == QTransform::TxNone) { // identity matrix
         return *this;
      } else if (mat.m11() == -1. && mat.m22() == -1.) {
         return rotated180(*this);
      }

      if (mode == Qt::FastTransformation) {
         hd = qRound(qAbs(mat.m22()) * hs);
         wd = qRound(qAbs(mat.m11()) * ws);
      } else {
         hd = int(qAbs(mat.m22()) * hs + 0.9999);
         wd = int(qAbs(mat.m11()) * ws + 0.9999);
      }
      scale_xform = true;
   } else {
      if (mat.type() <= QTransform::TxRotate && mat.m11() == 0 && mat.m22() == 0) {
         if (mat.m12() == 1. && mat.m21() == -1.) {
            return rotated90(*this);
         } else if (mat.m12() == -1. && mat.m21() == 1.) {
            return rotated270(*this);
         }
      }

      QPolygonF a(QRectF(0, 0, ws, hs));
      a = mat.map(a);
      QRect r = a.boundingRect().toAlignedRect();
      wd = r.width();
      hd = r.height();
      complex_xform = true;
   }

   if (wd == 0 || hd == 0) {
      return QImage();
   }

   // Make use of the optimized algorithm when we're scaling
   if (scale_xform && mode == Qt::SmoothTransformation) {
      if (mat.m11() < 0.0F && mat.m22() < 0.0F) { // horizontal/vertical flip
         return smoothScaled(wd, hd).mirrored(true, true);
      } else if (mat.m11() < 0.0F) { // horizontal flip
         return smoothScaled(wd, hd).mirrored(true, false);
      } else if (mat.m22() < 0.0F) { // vertical flip
         return smoothScaled(wd, hd).mirrored(false, true);
      } else { // no flipping
         return smoothScaled(wd, hd);
      }
   }

   int bpp = depth();

   int sbpl = bytesPerLine();
   const uchar *sptr = bits();

   QImage::Format target_format = d->format;

   if (complex_xform || mode == Qt::SmoothTransformation) {
      if (d->format < QImage::Format_RGB32 || !hasAlphaChannel()) {
         target_format = qt_alphaVersion(d->format);
      }
   }

   QImage dImage(wd, hd, target_format);
   QIMAGE_SANITYCHECK_MEMORY(dImage);

   if (target_format == QImage::Format_MonoLSB
      || target_format == QImage::Format_Mono
      || target_format == QImage::Format_Indexed8) {
      dImage.d->colortable = d->colortable;
      dImage.d->has_alpha_clut = d->has_alpha_clut | complex_xform;
   }


   if (d->format == QImage::Format_Indexed8) {
      if (dImage.d->colortable.size() < 256) {
         // colors are left in the color table, so pick that one as transparent
         dImage.d->colortable.append(0x0);
         memset(dImage.bits(), dImage.d->colortable.size() - 1, dImage.byteCount());
      } else {
         memset(dImage.bits(), 0, dImage.byteCount());
      }

   } else {
      memset(dImage.bits(), 0x00, dImage.byteCount());
   }

   if (target_format >= QImage::Format_RGB32) {
      const QImage sImage = (devicePixelRatio() != 1) ? QImage(constBits(), width(), height(), format()) : *this;

      Q_ASSERT(sImage.devicePixelRatio() == 1);
      Q_ASSERT(sImage.devicePixelRatio() == dImage.devicePixelRatio());
      QPainter p(&dImage);
      if (mode == Qt::SmoothTransformation) {
         p.setRenderHint(QPainter::Antialiasing);
         p.setRenderHint(QPainter::SmoothPixmapTransform);
      }
      p.setTransform(mat);
      p.drawImage(QPoint(0, 0), sImage);

   } else {
      bool invertible;
      mat = mat.inverted(&invertible);                // invert matrix
      if (!invertible) {      // error, return null image
         return QImage();
      }

      // create target image (some of the code is from QImage::copy())
      int type = format() == Format_Mono ? QT_XFORM_TYPE_MSBFIRST : QT_XFORM_TYPE_LSBFIRST;
      int dbpl = dImage.bytesPerLine();
      qt_xForm_helper(mat, 0, type, bpp, dImage.bits(), dbpl, 0, hd, sptr, sbpl, ws, hs);
   }

   copyMetadata(dImage.d, d);

   return dImage;
}

QTransform QImage::trueMatrix(const QTransform &matrix, int w, int h)
{
   const QRectF rect(0, 0, w, h);
   const QRect mapped = matrix.mapRect(rect).toAlignedRect();
   const QPoint delta = mapped.topLeft();

   return matrix * QTransform().translate(-delta.x(), -delta.y());
}

bool QImageData::convertInPlace(QImage::Format newFormat, Qt::ImageConversionFlags flags)
{
   if (format == newFormat) {
      return true;
   }

   // No in-place conversion if we have to detach
   if (ref.load() > 1 || !own_data) {
      return false;
   }

   InPlace_Image_Converter converter = QImageConversions::instance().image_inplace_converter_map[format][newFormat];

   if (converter) {
      return converter(this, flags);

   } else if (format > QImage::Format_Indexed8 && newFormat > QImage::Format_Indexed8 &&
            ! QImageConversions::instance().image_converter_map[format][newFormat])

      // Convert inplace generic, but only if there are no direct converters,
      // any direct ones are probably better even if not inplace.
   {
      return convert_generic_inplace(this, newFormat, flags);
   } else {
      return false;
   }
}

QDebug operator<<(QDebug dbg, const QImage &i)
{
   QDebugStateSaver saver(dbg);
   dbg.resetFormat();
   dbg.nospace();

   dbg << "QImage(";

   if (i.isNull()) {
      dbg << "null";
   } else {
      dbg << i.size() << ",format=" << i.format() << ",depth=" << i.depth();
      if (i.colorCount()) {
         dbg << ",colorCount=" << i.colorCount();
      }
      dbg << ",devicePixelRatio=" << i.devicePixelRatio()
         << ",bytesPerLine=" << i.bytesPerLine() << ",byteCount=" << i.byteCount();
   }

   dbg << ')';

   return dbg;
}

static constexpr QPixelFormat pixelformats[] = {
   //QImage::Format_Invalid:
   QPixelFormat(),
   //QImage::Format_Mono:
   QPixelFormat(QPixelFormat::Indexed,
      /*RED*/            1,
      /*GREEN*/          0,
      /*BLUE*/           0,
      /*FOURTH*/         0,
      /*FIFTH*/          0,
      /*ALPHA*/          0,
      /*ALPHA USAGE*/    QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/ QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/  QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/ QPixelFormat::UnsignedByte,
      /*BYTE ORDER*/     QPixelFormat::CurrentSystemEndian),
   //QImage::Format_MonoLSB:
   QPixelFormat(QPixelFormat::Indexed,
      /*RED*/            1,
      /*GREEN*/          0,
      /*BLUE*/           0,
      /*FOURTH*/         0,
      /*FIFTH*/          0,
      /*ALPHA*/          0,
      /*ALPHA USAGE*/    QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/ QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/  QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/ QPixelFormat::UnsignedByte,
      /*BYTE ORDER*/     QPixelFormat::CurrentSystemEndian),
   //QImage::Format_Indexed8:
   QPixelFormat(QPixelFormat::Indexed,
      /*RED*/            8,
      /*GREEN*/          0,
      /*BLUE*/           0,
      /*FOURTH*/         0,
      /*FIFTH*/          0,
      /*ALPHA*/          0,
      /*ALPHA USAGE*/    QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/ QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/  QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/ QPixelFormat::UnsignedByte,
      /*BYTE ORDER*/     QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGB32:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                8,
      /*GREEN*/              8,
      /*BLUE*/               8,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              8,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_ARGB32:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                8,
      /*GREEN*/              8,
      /*BLUE*/               8,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              8,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_ARGB32_Premultiplied:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                8,
      /*GREEN*/              8,
      /*BLUE*/               8,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              8,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::Premultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGB16:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                5,
      /*GREEN*/              6,
      /*BLUE*/               5,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              0,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedShort,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_ARGB8565_Premultiplied:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                5,
      /*GREEN*/              6,
      /*BLUE*/               5,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              8,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::Premultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGB666:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                6,
      /*GREEN*/              6,
      /*BLUE*/               6,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              0,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_ARGB6666_Premultiplied:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                6,
      /*GREEN*/              6,
      /*BLUE*/               6,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              6,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtEnd,
      /*PREMULTIPLIED*/     QPixelFormat::Premultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGB555:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                5,
      /*GREEN*/              5,
      /*BLUE*/               5,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              0,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedShort,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_ARGB8555_Premultiplied:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                5,
      /*GREEN*/              5,
      /*BLUE*/               5,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              8,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::Premultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGB888:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                8,
      /*GREEN*/              8,
      /*BLUE*/               8,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              0,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedByte,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGB444:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                4,
      /*GREEN*/              4,
      /*BLUE*/               4,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              0,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedShort,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_ARGB4444_Premultiplied:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                4,
      /*GREEN*/              4,
      /*BLUE*/               4,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              4,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtEnd,
      /*PREMULTIPLIED*/     QPixelFormat::Premultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedShort,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGBX8888:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                8,
      /*GREEN*/              8,
      /*BLUE*/               8,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              8,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtEnd,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedByte,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGBA8888:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                8,
      /*GREEN*/              8,
      /*BLUE*/               8,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              8,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtEnd,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedByte,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGBA8888_Premultiplied:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                8,
      /*GREEN*/              8,
      /*BLUE*/               8,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              8,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtEnd,
      /*PREMULTIPLIED*/     QPixelFormat::Premultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedByte,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_BGR30:
   QPixelFormat(QPixelFormat::BGR,
      /*RED*/                10,
      /*GREEN*/              10,
      /*BLUE*/               10,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              2,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_A2BGR30_Premultiplied:
   QPixelFormat(QPixelFormat::BGR,
      /*RED*/                10,
      /*GREEN*/              10,
      /*BLUE*/               10,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              2,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::Premultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_RGB30:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                10,
      /*GREEN*/              10,
      /*BLUE*/               10,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              2,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_A2RGB30_Premultiplied:
   QPixelFormat(QPixelFormat::RGB,
      /*RED*/                10,
      /*GREEN*/              10,
      /*BLUE*/               10,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              2,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::Premultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedInteger,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_Alpha8:
   QPixelFormat(QPixelFormat::Alpha,
      /*First*/              0,
      /*SECOND*/             0,
      /*THIRD*/              0,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              8,
      /*ALPHA USAGE*/       QPixelFormat::UsesAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::Premultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedByte,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
   //QImage::Format_Grayscale8:
   QPixelFormat(QPixelFormat::Grayscale,
      /*GRAY*/               8,
      /*SECOND*/             0,
      /*THIRD*/              0,
      /*FOURTH*/             0,
      /*FIFTH*/              0,
      /*ALPHA*/              0,
      /*ALPHA USAGE*/       QPixelFormat::IgnoresAlpha,
      /*ALPHA POSITION*/    QPixelFormat::AtBeginning,
      /*PREMULTIPLIED*/     QPixelFormat::NotPremultiplied,
      /*INTERPRETATION*/    QPixelFormat::UnsignedByte,
      /*BYTE ORDER*/        QPixelFormat::CurrentSystemEndian),
};
static_assert(sizeof(pixelformats) / sizeof(*pixelformats) == QImage::NImageFormats, "Type mismatch");

QPixelFormat QImage::pixelFormat() const
{
   return toPixelFormat(format());
}

QPixelFormat QImage::toPixelFormat(QImage::Format format)
{
   Q_ASSERT(static_cast<int>(format) < NImageFormats);
   return pixelformats[format];
}

QImage::Format QImage::toImageFormat(QPixelFormat format)
{
   for (int i = 0; i < NImageFormats; i++) {
      if (format == pixelformats[i]) {
         return Format(i);
      }
   }
   return Format_Invalid;
}

Q_GUI_EXPORT void qt_imageTransform(QImage &src, QImageIOHandler::Transformations orient)
{
   if (orient == QImageIOHandler::TransformationNone) {
      return;
   }
   if (orient == QImageIOHandler::TransformationRotate270) {
      src = rotated270(src);
   } else {
      src = std::move(src).mirrored(orient & QImageIOHandler::TransformationMirror,
            orient & QImageIOHandler::TransformationFlip);
      if (orient & QImageIOHandler::TransformationRotate90) {
         src = rotated90(src);
      }
   }
}

