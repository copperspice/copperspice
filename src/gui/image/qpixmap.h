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

#ifndef QPIXMAP_H
#define QPIXMAP_H

#if defined(Q_OS_DARWIN)
#include <cs_carbon_wrapper.h>
#endif

#include <qpaintdevice.h>
#include <qcolor.h>
#include <qnamespace.h>
#include <qstring.h>
#include <qsharedpointer.h>
#include <qimage.h>
#include <qtransform.h>
#include <qvariant.h>

class QImageWriter;
class QImageReader;
class QPlatformPixmap;

enum QPlatformPixmap_ClassId : int;

class Q_GUI_EXPORT QPixmap : public QPaintDevice
{
 public:
   QPixmap();
   explicit QPixmap(QPlatformPixmap *imageData);
   QPixmap(int width, int height);
   explicit QPixmap(const QSize &size);

   QPixmap(const QString &fileName, const QString &format = QString(), Qt::ImageConversionFlags flags = Qt::AutoColor);

#ifndef QT_NO_IMAGEFORMAT_XPM
   explicit QPixmap(const char *const xpm[]);
#endif

   QPixmap(const QPixmap &pixmap);
   ~QPixmap();

   QPixmap &operator=(const QPixmap &other);

   QPixmap &operator=(QPixmap &&other) {
      qSwap(data, other.data);
      return *this;
   }

   void swap(QPixmap &other) {
      qSwap(data, other.data);
   }

   operator QVariant() const;

   bool isNull() const;
   int devType() const override;

   int width()  const;
   int height() const;
   QSize size() const;
   QRect rect() const;
   int depth()  const;

   static int defaultDepth();

   void fill(const QColor &fillColor = Qt::white);
   void fill(const QPaintDevice *device, const QPoint &offset);

   void fill(const QPaintDevice *device, int xOffset, int yOffset) {
      fill(device, QPoint(xOffset, yOffset));
   }

   QBitmap mask() const;
   void setMask(const QBitmap &mask);
   qreal devicePixelRatio() const;
   void setDevicePixelRatio(qreal scaleFactor);

   bool hasAlpha() const;
   bool hasAlphaChannel() const;

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
   QBitmap createHeuristicMask(bool clipTight = true) const;
#endif

   QBitmap createMaskFromColor(const QColor &maskColor, Qt::MaskMode mode = Qt::MaskInColor) const;

   static QPixmap grabWindow(WId window, int x = 0, int y = 0, int width = -1, int height = -1);
   static QPixmap grabWidget(QObject *widget, const QRect &rect);

   static inline QPixmap grabWidget(QObject *widget, int x = 0, int y = 0, int width = -1, int height = -1) {
      return grabWidget(widget, QRect(x, y, width, height));
   }

   inline QPixmap scaled(int width, int height, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
      Qt::TransformationMode transformMode = Qt::FastTransformation) const {
      return scaled(QSize(width, height), aspectMode, transformMode);
   }

   QPixmap scaled(const QSize &size, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
      Qt::TransformationMode transformMode = Qt::FastTransformation) const;

   QPixmap scaledToWidth(int width, Qt::TransformationMode transformMode = Qt::FastTransformation) const;
   QPixmap scaledToHeight(int height, Qt::TransformationMode transformMode = Qt::FastTransformation) const;
   QPixmap transformed(const QMatrix &matrix, Qt::TransformationMode transformMode = Qt::FastTransformation) const;
   static QMatrix trueMatrix(const QMatrix &matrix, int width, int height);
   QPixmap transformed(const QTransform &transform, Qt::TransformationMode transformMode = Qt::FastTransformation) const;
   static QTransform trueMatrix(const QTransform &transform, int width, int height);

   QImage toImage() const;
   static QPixmap fromImage(const QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);
   static QPixmap fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags = Qt::AutoColor);

   static QPixmap fromImage(QImage &&image, Qt::ImageConversionFlags flags = Qt::AutoColor) {
      return fromImageInPlace(image, flags);
   }

   bool load(const QString &fileName, const QString &format = QString(), Qt::ImageConversionFlags flags = Qt::AutoColor);
   bool loadFromData(const uchar *imageData, uint len, const QString &format = QString(),
         Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline bool loadFromData(const QByteArray &imageData, const QString &format = QString(),
         Qt::ImageConversionFlags flags = Qt::AutoColor);

   bool save(const QString &fileName, const QString &format = QString(), int quality = -1) const;
   bool save(QIODevice *device, const QString &format = QString(), int quality = -1) const;

   bool convertFromImage(const QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline QPixmap copy(int x, int y, int width, int height) const;
   QPixmap copy(const QRect &rect = QRect()) const;

   inline void scroll(int dx, int dy, int x, int y, int width, int height, QRegion *exposed = nullptr);
   void scroll(int dx, int dy, const QRect &rect, QRegion *exposed = nullptr);

   qint64 cacheKey() const;

   bool isDetached() const;
   void detach();

   bool isQBitmap() const;

   QPaintEngine *paintEngine() const override;

   bool operator!() const {
      return isNull();
   }

   QPlatformPixmap_ClassId classId() const;
   QPlatformPixmap *handle() const;

 protected:
   int metric(PaintDeviceMetric) const override;
   static QPixmap fromImageInPlace(QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);

 private:
   QExplicitlySharedDataPointer<QPlatformPixmap> data;

   bool doImageIO(QImageWriter *io, int quality) const;

   QPixmap(const QSize &s, int type);
   void doInit(int, int, int);

   QExplicitlySharedDataPointer<QPlatformPixmap> &data_ptr() {
      return data;
   }

   friend class QBitmap;
   friend class QImagePixmapCleanupHooks;
   friend class QOpenGLWidget;
   friend class QPlatformPixmap;
   friend class QPixmapCacheEntry;
   friend class QPaintDevice;
   friend class QPainter;
   friend class QRasterBuffer;
   friend class QWidgetPrivate;

   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPixmap &pixmap);
};

inline QPixmap QPixmap::copy(int x, int y, int width, int height) const
{
   return copy(QRect(x, y, width, height));
}

inline void QPixmap::scroll(int dx, int dy, int x, int y, int width, int height, QRegion *exposed)
{
   scroll(dx, dy, QRect(x, y, width, height), exposed);
}

inline bool QPixmap::loadFromData(const QByteArray &imageData, const QString &format,
      Qt::ImageConversionFlags flags)
{
   return loadFromData(reinterpret_cast<const uchar *>(imageData.constData()), imageData.size(), format, flags);
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPixmap &pixmap);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPixmap &pixmap);

template <>
inline bool CustomType_T<QPixmap>::compare(const CustomType &other) const {

   auto ptr = dynamic_cast<const CustomType_T<QPixmap>*>(&other);

   if (ptr != nullptr) {
      return m_value.cacheKey() == (ptr->m_value).cacheKey();
   }

   return false;
}

#endif
