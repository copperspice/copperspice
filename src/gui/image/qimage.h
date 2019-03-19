/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qtransform.h>
#include <qpaintdevice.h>
#include <qrgb.h>
#include <qbytearray.h>
#include <qrect.h>
#include <qstring.h>

class QIODevice;
class QStringList;
class QMatrix;
class QTransform;
class QVariant;
class QImageDataMisc;

struct QImageData;
template <class T> class QList;
template <class T> class QVector;

#ifndef QT_NO_IMAGE_TEXT

class Q_GUI_EXPORT QImageTextKeyLang
{
 public:
   QImageTextKeyLang(const char *k, const char *l) : key(k), lang(l) { }
   QImageTextKeyLang() { }

   QByteArray key;
   QByteArray lang;

   bool operator< (const QImageTextKeyLang &other) const {
      return key < other.key || (key == other.key && lang < other.lang);
   }
   bool operator== (const QImageTextKeyLang &other) const {
      return key == other.key && lang == other.lang;
   }
   inline bool operator!= (const QImageTextKeyLang &other) const {
      return !operator==(other);
   }
};

#endif


class Q_GUI_EXPORT QImage : public QPaintDevice
{

 public:
   enum InvertMode { InvertRgb, InvertRgba };
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
      NImageFormats
   };

   QImage();
   QImage(const QSize &size, Format format);
   QImage(int width, int height, Format format);
   QImage(uchar *data, int width, int height, Format format);
   QImage(const uchar *data, int width, int height, Format format);
   QImage(uchar *data, int width, int height, int bytesPerLine, Format format);
   QImage(const uchar *data, int width, int height, int bytesPerLine, Format format);

   QImage(const QImage &);

   explicit QImage(const QString &fileName, const char *format = 0);

#ifndef QT_NO_IMAGEFORMAT_XPM
   explicit QImage(const char *const xpm[]);
#endif

   ~QImage();

   QImage &operator=(const QImage &);

   inline QImage &operator=(QImage && other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QImage &other) {
      qSwap(d, other.d);
   }

   bool isNull() const;

   int devType() const override;

   bool operator==(const QImage &) const;
   bool operator!=(const QImage &) const;
   operator QVariant() const;
   void detach();
   bool isDetached() const;

   QImage copy(const QRect &rect = QRect()) const;
   inline QImage copy(int x, int y, int w, int h) const {
      return copy(QRect(x, y, w, h));
   }

   Format format() const;

   QImage convertToFormat(Format f, Qt::ImageConversionFlags flags = Qt::AutoColor) const Q_REQUIRED_RESULT;
   QImage convertToFormat(Format f, const QVector<QRgb> &colorTable,
                          Qt::ImageConversionFlags flags = Qt::AutoColor) const Q_REQUIRED_RESULT;

   int width() const;
   int height() const;
   QSize size() const;
   QRect rect() const;

   int depth() const;

#ifdef QT_DEPRECATED
   QT_DEPRECATED int numColors() const;
#endif

   int colorCount() const;
   int bitPlaneCount() const;

   QRgb color(int i) const;
   void setColor(int i, QRgb c);

#ifdef QT_DEPRECATED
   QT_DEPRECATED void setNumColors(int);
#endif

   void setColorCount(int);

   bool allGray() const;
   bool isGrayscale() const;

   uchar *bits();
   const uchar *bits() const;
   const uchar *constBits() const;

#ifdef QT_DEPRECATED
   QT_DEPRECATED int numBytes() const;
#endif
   int byteCount() const;

   uchar *scanLine(int);
   const uchar *scanLine(int) const;
   const uchar *constScanLine(int) const;
   int bytesPerLine() const;

   bool valid(int x, int y) const;
   inline bool valid(const QPoint &pt) const;

   int pixelIndex(int x, int y) const;
   inline int pixelIndex(const QPoint &pt) const;

   QRgb pixel(int x, int y) const;
   inline QRgb pixel(const QPoint &pt) const;

   void setPixel(int x, int y, uint index_or_rgb);
   inline void setPixel(const QPoint &pt, uint index_or_rgb);

   QVector<QRgb> colorTable() const;
   void setColorTable(const QVector<QRgb> colors);

   void fill(uint pixel);
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

   inline QImage scaled(int w, int h, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                        Qt::TransformationMode mode = Qt::FastTransformation) const {
      return scaled(QSize(w, h), aspectMode, mode);
   }

   QImage scaled(const QSize &s, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                 Qt::TransformationMode mode = Qt::FastTransformation) const;
   QImage scaledToWidth(int w, Qt::TransformationMode mode = Qt::FastTransformation) const;
   QImage scaledToHeight(int h, Qt::TransformationMode mode = Qt::FastTransformation) const;
   QImage transformed(const QMatrix &matrix, Qt::TransformationMode mode = Qt::FastTransformation) const;
   static QMatrix trueMatrix(const QMatrix &, int w, int h);
   QImage transformed(const QTransform &matrix, Qt::TransformationMode mode = Qt::FastTransformation) const;
   static QTransform trueMatrix(const QTransform &, int w, int h);
   QImage mirrored(bool horizontally = false, bool vertically = true) const;
   QImage rgbSwapped() const;
   void invertPixels(InvertMode = InvertRgb);

   bool load(QIODevice *device, const char *format);
   bool load(const QString &fileName, const char *format = 0);
   bool loadFromData(const uchar *buf, int len, const char *format = 0);
   inline bool loadFromData(const QByteArray &data, const char *aformat = 0) {
      return loadFromData(reinterpret_cast<const uchar *>(data.constData()), data.size(), aformat);
   }

   bool save(const QString &fileName, const char *format = 0, int quality = -1) const;
   bool save(QIODevice *device, const char *format = 0, int quality = -1) const;

   static QImage fromData(const uchar *data, int size, const char *format = 0);
   inline static QImage fromData(const QByteArray &data, const char *format = 0) {
      return fromData(reinterpret_cast<const uchar *>(data.constData()), data.size(), format);
   }

   int serialNumber() const;
   qint64 cacheKey() const;

   QPaintEngine *paintEngine() const override;

   // Auxiliary data
   int dotsPerMeterX() const;
   int dotsPerMeterY() const;
   void setDotsPerMeterX(int);
   void setDotsPerMeterY(int);
   QPoint offset() const;
   void setOffset(const QPoint &);

#ifndef QT_NO_IMAGE_TEXT
   QStringList textKeys() const;
   QString text(const QString &key = QString()) const;
   void setText(const QString &key, const QString &value);

#ifdef QT_DEPRECATED
   QT_DEPRECATED QString text(const char *key, const char *lang = 0) const;
   QT_DEPRECATED QList<QImageTextKeyLang> textList() const;
   QT_DEPRECATED QStringList textLanguages() const;
   QT_DEPRECATED QString text(const QImageTextKeyLang &) const;
   QT_DEPRECATED void setText(const char *key, const char *lang, const QString &);
#endif

#endif

   typedef QImageData *DataPtr;
   inline DataPtr &data_ptr() {
      return d;
   }

 protected:
   virtual int metric(PaintDeviceMetric metric) const override;

 private:
   friend class QWSOnScreenSurface;
   QImageData *d;

   friend class QRasterPixmapData;
   friend class QBlittablePixmapData;
   friend class QPixmapCacheEntry;
   friend Q_GUI_EXPORT qint64 qt_image_id(const QImage &image);
   friend const QVector<QRgb> *qt_image_colortable(const QImage &image);

};

Q_DECLARE_SHARED(QImage)
Q_DECLARE_TYPEINFO(QImage, Q_MOVABLE_TYPE);

inline bool QImage::valid(const QPoint &pt) const
{
   return valid(pt.x(), pt.y());
}
inline int QImage::pixelIndex(const QPoint &pt) const
{
   return pixelIndex(pt.x(), pt.y());
}
inline QRgb QImage::pixel(const QPoint &pt) const
{
   return pixel(pt.x(), pt.y());
}
inline void QImage::setPixel(const QPoint &pt, uint index_or_rgb)
{
   setPixel(pt.x(), pt.y(), index_or_rgb);
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QImage &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QImage &);

#endif
