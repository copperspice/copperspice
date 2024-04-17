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

#ifndef QIMAGE_H
#define QIMAGE_H

#include <qbytearray.h>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qrgb.h>
#include <qpaintdevice.h>
#include <qpixelformat.h>
#include <qtransform.h>
#include <qrect.h>
#include <qstring.h>

class QIODevice;
class QStringList;
class QMatrix;
class QTransform;
class QVariant;
class QImageDataMisc;

struct QImageData;

typedef void (*QImageCleanupFunction)(void *);

class Q_GUI_EXPORT QImage : public QPaintDevice
{

 public:
   enum InvertMode {
      InvertRgb,
      InvertRgba
   };

   enum Format {
      Format_Invalid,
      Format_Mono,
      Format_MonoLSB,
      Format_Indexed8,
      Format_RGB32,
      Format_ARGB32,
      Format_ARGB32_Premultiplied,
      Format_RGB16,
      Format_ARGB8565_Premultiplied,
      Format_RGB666,
      Format_ARGB6666_Premultiplied,
      Format_RGB555,
      Format_ARGB8555_Premultiplied,
      Format_RGB888,
      Format_RGB444,
      Format_ARGB4444_Premultiplied,
      Format_RGBX8888,
      Format_RGBA8888,
      Format_RGBA8888_Premultiplied,
      Format_BGR30,
      Format_A2BGR30_Premultiplied,
      Format_RGB30,
      Format_A2RGB30_Premultiplied,
      Format_Alpha8,
      Format_Grayscale8,
      NImageFormats
   };

   QImage();
   QImage(const QSize &size, Format format);
   QImage(int width, int height, Format format);

   QImage(uchar *data, int width, int height, Format format, QImageCleanupFunction cleanupFunction = nullptr,
      void *cleanupInfo = nullptr);
   QImage(const uchar *data, int width, int height, Format format, QImageCleanupFunction cleanupFunction = nullptr,
      void *cleanupInfo = nullptr);
   QImage(uchar *data, int width, int height, int bytesPerLine, Format format, QImageCleanupFunction cleanupFunction = nullptr,
      void *cleanupInfo = nullptr);
   QImage(const uchar *data, int width, int height, int bytesPerLine, Format format, QImageCleanupFunction cleanupFunction = nullptr,
      void *cleanupInfo = nullptr);

   explicit QImage(const QString &fileName, const QString &format = QString());

#ifndef QT_NO_IMAGEFORMAT_XPM
   explicit QImage(const char *const xpm[]);
#endif

   QImage(const QImage &other);

   QImage(QImage &&other)
      : QPaintDevice(), d(nullptr)
   {
      qSwap(d, other.d);
   }

   ~QImage();

   QImage &operator=(const QImage &other);

   inline QImage &operator=(QImage &&other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QImage &other) {
      qSwap(d, other.d);
   }

   bool isNull() const;

   int devType() const override;

   bool operator==(const QImage &other) const;
   bool operator!=(const QImage &other) const;

   operator QVariant() const;
   void detach();
   bool isDetached() const;

   QImage copy(const QRect &rect = QRect()) const;
   inline QImage copy(int x, int y, int width, int height) const {
      return copy(QRect(x, y, width, height));
   }

   Format format() const;

   QImage convertToFormat(Format format, Qt::ImageConversionFlags flags = Qt::AutoColor) const & {
      return convertToFormat_helper(format, flags);
   }

   QImage convertToFormat(Format format, Qt::ImageConversionFlags flags = Qt::AutoColor) && {
      if (convertToFormat_inplace(format, flags)) {
         return std::move(*this);
      } else {
         return convertToFormat_helper(format, flags);
      }
   }

   QImage convertToFormat(Format format, const QVector<QRgb> &colorTable,
      Qt::ImageConversionFlags flags = Qt::AutoColor) const;

   int width() const;
   int height() const;
   QSize size() const;
   QRect rect() const;

   int depth() const;

   int colorCount() const;
   int bitPlaneCount() const;

   QRgb color(int index) const;
   void setColor(int index, QRgb colorValue);

   void setColorCount(int colorCount);

   bool allGray() const;
   bool isGrayscale() const;

   uchar *bits();
   const uchar *bits() const;
   const uchar *constBits() const;

   int byteCount() const;

   uchar *scanLine(int index);
   const uchar *scanLine(int index) const;
   const uchar *constScanLine(int index) const;
   int bytesPerLine() const;

   bool valid(int x, int y) const;
   inline bool valid(const QPoint &pos) const;

   int pixelIndex(int x, int y) const;
   inline int pixelIndex(const QPoint &pos) const;

   QRgb pixel(int x, int y) const;
   inline QRgb pixel(const QPoint &pos) const;

   void setPixel(int x, int y, uint index_or_rgb);
   inline void setPixel(const QPoint &pos, uint index_or_rgb);

   QColor pixelColor(int x, int y) const;
   QColor pixelColor(const QPoint &pos) const;

   void setPixelColor(int x, int y, const QColor &color);
   void setPixelColor(const QPoint &pos, const QColor &color);
   QVector<QRgb> colorTable() const;
   void setColorTable(const QVector<QRgb> &colors);

   qreal devicePixelRatio() const;
   void setDevicePixelRatio(qreal scaleFactor);

   void fill(uint pixelValue);
   void fill(const QColor &color);
   void fill(Qt::GlobalColor color);

   bool hasAlphaChannel() const;
   void setAlphaChannel(const QImage &alphaChannel);
   QImage alphaChannel() const;
   QImage createAlphaMask(Qt::ImageConversionFlags flags = Qt::AutoColor) const;

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
   QImage createHeuristicMask(bool clipTight = true) const;
#endif

   QImage createMaskFromColor(QRgb color, Qt::MaskMode mode = Qt::MaskInColor) const;

   inline QImage scaled(int width, int height, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio,
      Qt::TransformationMode mode = Qt::FastTransformation) const {
      return scaled(QSize(width, height), aspectRatioMode, mode);
   }

   QImage scaled(const QSize &size, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio,
      Qt::TransformationMode mode = Qt::FastTransformation) const;

   QImage scaledToWidth(int width, Qt::TransformationMode mode = Qt::FastTransformation) const;
   QImage scaledToHeight(int height, Qt::TransformationMode mode = Qt::FastTransformation) const;
   QImage transformed(const QMatrix &matrix, Qt::TransformationMode mode = Qt::FastTransformation) const;
   static QMatrix trueMatrix(const QMatrix &matrix, int width, int height);
   QImage transformed(const QTransform &matrix, Qt::TransformationMode mode = Qt::FastTransformation) const;
   static QTransform trueMatrix(const QTransform &matrix, int width, int height);

   QImage mirrored(bool horizontal = false, bool vertical = true) const & {
      return mirrored_helper(horizontal, vertical);
   }

   QImage &&mirrored(bool horizontal = false, bool vertical = true) && {
      mirrored_inplace(horizontal, vertical);
      return std::move(*this);
   }

   QImage rgbSwapped() const & {
      return rgbSwapped_helper();
   }

   QImage &&rgbSwapped() && {
      rgbSwapped_inplace();
      return std::move(*this);
   }

   void invertPixels(InvertMode mode = InvertRgb);

   bool load(QIODevice *device, const QString &format);
   bool load(const QString &fileName, const QString &format = QString());
   bool loadFromData(const uchar *buffer, int len, const QString &format = QString());

   inline bool loadFromData(const QByteArray &data, const QString &format = QString()) {
      return loadFromData(reinterpret_cast<const uchar *>(data.constData()), data.size(), format);
   }

   bool save(const QString &fileName, const QString &format = QString(), int quality = -1) const;
   bool save(QIODevice *device, const QString &format = QString(), int quality = -1) const;

   static QImage fromData(const uchar *data, int size, const QString &format = QString());

   static inline QImage fromData(const QByteArray &data, const QString &format = QString()) {
      return fromData(reinterpret_cast<const uchar *>(data.constData()), data.size(), format);
   }

   int serialNumber() const {
      return cacheKey() >> 32;
   }

   qint64 cacheKey() const;
   QPaintEngine *paintEngine() const override;

   // Auxiliary data
   int dotsPerMeterX() const;
   int dotsPerMeterY() const;
   void setDotsPerMeterX(int x);
   void setDotsPerMeterY(int y);

   QPoint offset() const;
   void setOffset(const QPoint &offset);

   QStringList textKeys() const;
   QString text(const QString &key = QString()) const;
   void setText(const QString &key, const QString &text);

   QPixelFormat pixelFormat() const;
   static QPixelFormat toPixelFormat(QImage::Format format);
   static QImage::Format toImageFormat(QPixelFormat format);

 protected:
   typedef QImageData *DataPtr;

   inline DataPtr &data_ptr() {
      return d;
   }

   int metric(PaintDeviceMetric metric) const override;
   QImage mirrored_helper(bool horizontal, bool vertical) const;
   QImage rgbSwapped_helper() const;
   void mirrored_inplace(bool horizontal, bool vertical);
   void rgbSwapped_inplace();
   QImage convertToFormat_helper(Format format, Qt::ImageConversionFlags flags) const;
   bool convertToFormat_inplace(Format format, Qt::ImageConversionFlags flags);
   QImage smoothScaled(int width, int height) const;

 private:
   QImageData *d;

   friend class QBackingStore;
   friend class QBlittablePlatformPixmap;
   friend class QImagePixmapCleanupHooks;
   friend class QRasterPlatformPixmap;
   friend class QFontEngineFT;
   friend class QPixmapCacheEntry;
   friend struct QSpanData;
};

inline bool QImage::valid(const QPoint &pos) const
{
   return valid(pos.x(), pos.y());
}

inline int QImage::pixelIndex(const QPoint &pos) const
{
   return pixelIndex(pos.x(), pos.y());
}

inline QRgb QImage::pixel(const QPoint &pos) const
{
   return pixel(pos.x(), pos.y());
}

inline void QImage::setPixel(const QPoint &pos, uint index_or_rgb)
{
   setPixel(pos.x(), pos.y(), index_or_rgb);
}

inline QColor QImage::pixelColor(const QPoint &pos) const
{
   return pixelColor(pos.x(), pos.y());
}

inline void QImage::setPixelColor(const QPoint &pos, const QColor &color)
{
   setPixelColor(pos.x(), pos.y(), color);
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QImage &image);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QImage &image);

Q_GUI_EXPORT QDebug operator<<(QDebug, const QImage &image);

#endif
