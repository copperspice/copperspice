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

#ifndef QPIXMAP_H
#define QPIXMAP_H

#if defined(Q_OS_MAC)
#include <cs_carbon_wrapper.h>
#endif

#include <QtGui/qpaintdevice.h>
#include <QtGui/qcolor.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>
#include <QtCore/qsharedpointer.h>
#include <QtGui/qimage.h>
#include <QtGui/qtransform.h>

class QImageWriter;
class QImageReader;
class QColor;
class QVariant;

class QPlatformPixmap;

class Q_GUI_EXPORT QPixmap : public QPaintDevice
{
 public:
   QPixmap();
   explicit QPixmap(QPlatformPixmap *data);
   QPixmap(int w, int h);
   explicit QPixmap(const QSize &);

   QPixmap(const QString &fileName, const char *format = nullptr, Qt::ImageConversionFlags flags = Qt::AutoColor);

#ifndef QT_NO_IMAGEFORMAT_XPM
   explicit QPixmap(const char *const xpm[]);
#endif

   QPixmap(const QPixmap &);
   ~QPixmap();

   QPixmap &operator=(const QPixmap &);

   inline QPixmap &operator=(QPixmap &&other) {
      qSwap(data, other.data);
      return *this;
   }

   inline void swap(QPixmap &other) {
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
   void fill(const QPaintDevice *device, const QPoint &ofs);

   inline void fill(const QPaintDevice *device, int xofs, int yofs) {
      fill(device, QPoint(xofs, yofs));
   }

   QBitmap mask() const;
   void setMask(const QBitmap &);
   qreal devicePixelRatio() const;
   void setDevicePixelRatio(qreal scaleFactor);

   bool hasAlpha() const;
   bool hasAlphaChannel() const;

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
   QBitmap createHeuristicMask(bool clipTight = true) const;
#endif

   QBitmap createMaskFromColor(const QColor &maskColor, Qt::MaskMode mode = Qt::MaskInColor) const;

   static QPixmap grabWindow(WId, int x = 0, int y = 0, int w = -1, int h = -1);
   static QPixmap grabWidget(QObject *widget, const QRect &rect);
   static inline QPixmap grabWidget(QObject *widget, int x = 0, int y = 0, int w = -1, int h = -1) {
      return grabWidget(widget, QRect(x, y, w, h));
   }

   inline QPixmap scaled(int w, int h, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
      Qt::TransformationMode mode = Qt::FastTransformation) const {
      return scaled(QSize(w, h), aspectMode, mode);
   }

   QPixmap scaled(const QSize &s, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
      Qt::TransformationMode mode = Qt::FastTransformation) const;
   QPixmap scaledToWidth(int w, Qt::TransformationMode mode = Qt::FastTransformation) const;
   QPixmap scaledToHeight(int h, Qt::TransformationMode mode = Qt::FastTransformation) const;
   QPixmap transformed(const QMatrix &, Qt::TransformationMode mode = Qt::FastTransformation) const;
   static QMatrix trueMatrix(const QMatrix &m, int w, int h);
   QPixmap transformed(const QTransform &, Qt::TransformationMode mode = Qt::FastTransformation) const;
   static QTransform trueMatrix(const QTransform &m, int w, int h);

   QImage toImage() const;
   static QPixmap fromImage(const QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);
   static QPixmap fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags = Qt::AutoColor);

   static QPixmap fromImage(QImage &&image, Qt::ImageConversionFlags flags = Qt::AutoColor) {
      return fromImageInPlace(image, flags);
   }

   bool load(const QString &fileName, const char *format = nullptr, Qt::ImageConversionFlags flags = Qt::AutoColor);
   bool loadFromData(const uchar *buf, uint len, const char *format = nullptr, Qt::ImageConversionFlags flags = Qt::AutoColor);
   inline bool loadFromData(const QByteArray &data, const char *format = nullptr, Qt::ImageConversionFlags flags = Qt::AutoColor);
   bool save(const QString &fileName, const char *format = nullptr, int quality = -1) const;
   bool save(QIODevice *device, const char *format = nullptr, int quality = -1) const;

   bool convertFromImage(const QImage &img, Qt::ImageConversionFlags flags = Qt::AutoColor);

   inline QPixmap copy(int x, int y, int width, int height) const;
   QPixmap copy(const QRect &rect = QRect()) const;

   inline void scroll(int dx, int dy, int x, int y, int width, int height, QRegion *exposed = nullptr);
   void scroll(int dx, int dy, const QRect &rect, QRegion *exposed = nullptr);

   qint64 cacheKey() const;

   bool isDetached() const;
   void detach();

   bool isQBitmap() const;

   QPaintEngine *paintEngine() const override;

   inline bool operator!() const {
      return isNull();
   }

 protected:
   int metric(PaintDeviceMetric) const override;
   static QPixmap fromImageInPlace(QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);

 private:
   QExplicitlySharedDataPointer<QPlatformPixmap> data;

   bool doImageIO(QImageWriter *io, int quality) const;

   QPixmap(const QSize &s, int type);
   void doInit(int, int, int);

   QPlatformPixmap *handle() const;

   using DataPtr = QExplicitlySharedDataPointer<QPlatformPixmap>;
   inline DataPtr &data_ptr() {
      return data;
   }

   friend class QPlatformPixmap;
   friend class QBitmap;
   friend class QPaintDevice;
   friend class QPainter;
   friend class QOpenGLWidget;

   friend class QWidgetPrivate;
   friend class QRasterBuffer;

   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPixmap &);

};


inline QPixmap QPixmap::copy(int ax, int ay, int awidth, int aheight) const
{
   return copy(QRect(ax, ay, awidth, aheight));
}

inline void QPixmap::scroll(int dx, int dy, int ax, int ay, int awidth, int aheight, QRegion *exposed)
{
   scroll(dx, dy, QRect(ax, ay, awidth, aheight), exposed);
}

inline bool QPixmap::loadFromData(const QByteArray &buf, const char *format, Qt::ImageConversionFlags flags)
{
   return loadFromData(reinterpret_cast<const uchar *>(buf.constData()), buf.size(), format, flags);
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPixmap &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPixmap &);

#endif
