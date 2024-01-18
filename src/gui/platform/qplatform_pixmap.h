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

#ifndef QPLATFORM_PIXMAP_H
#define QPLATFORM_PIXMAP_H

#include <qpixmap.h>
#include <qatomic.h>

class QImageReader;

enum QPlatformPixmap_ClassId : int {
   RasterClass,
   DirectFBClass,
   BlitterClass,
   Direct2DClass,
   CustomClass = 1024
};

class Q_GUI_EXPORT QPlatformPixmap
{
 public:
   enum PixelType {
      // WARNING: do not change the first two must match QPixmap::Type
      PixmapType,
      BitmapType
   };

   using ClassId = QPlatformPixmap_ClassId;
   static constexpr ClassId RasterClass = QPlatformPixmap_ClassId::RasterClass;

   QPlatformPixmap(PixelType pixelType, int classId);
   virtual ~QPlatformPixmap();

   virtual QPlatformPixmap *createCompatiblePlatformPixmap() const;

   virtual void resize(int width, int height) = 0;
   virtual void fromImage(const QImage &image, Qt::ImageConversionFlags flags) = 0;
   virtual void fromImageInPlace(QImage &image, Qt::ImageConversionFlags flags) {
      fromImage(image, flags);
   }

   virtual void fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags);

   virtual bool fromFile(const QString &filename, const QString &format, Qt::ImageConversionFlags flags);
   virtual bool fromData(const uchar *buffer, uint len, const QString &format, Qt::ImageConversionFlags flags);

   virtual void copy(const QPlatformPixmap *data, const QRect &rect);
   virtual bool scroll(int dx, int dy, const QRect &rect);

   virtual int metric(QPaintDevice::PaintDeviceMetric metric) const = 0;
   virtual void fill(const QColor &color) = 0;

   virtual bool hasAlphaChannel() const = 0;
   virtual QPixmap transformed(const QTransform &matrix,
   Qt::TransformationMode mode) const;

   virtual QImage toImage() const = 0;
   virtual QImage toImage(const QRect &rect) const;
   virtual QPaintEngine *paintEngine() const = 0;

   inline int serialNumber() const {
      return ser_no;
   }

   inline PixelType pixelType() const {
      return type;
   }

   inline ClassId classId() const {
      return static_cast<ClassId>(id);
   }

   virtual qreal devicePixelRatio() const = 0;
   virtual void setDevicePixelRatio(qreal scaleFactor) = 0;

   virtual QImage *buffer();

   inline int width() const {
      return w;
   }
   inline int height() const {
      return h;
   }
   inline int colorCount() const {
      return metric(QPaintDevice::PdmNumColors);
   }
   inline int depth() const {
      return d;
   }
   inline bool isNull() const {
      return is_null;
   }
   inline qint64 cacheKey() const {
      int classKey = id;
      if (classKey >= 1024) {
         classKey = -(classKey >> 10);
      }
      return ((((qint64) classKey) << 56)
            | (((qint64) ser_no) << 32)
            | ((qint64) detach_no));
   }

   static QPlatformPixmap *create(int w, int h, PixelType type);

 protected:
   void setSerialNumber(int serNo);
   void setDetachNumber(int detNo);
   int w;
   int h;
   int d;
   bool is_null;

 private:
   friend class QPixmap;
   friend class QImagePixmapCleanupHooks; // Needs to set is_cached
   friend class QOpenGLTextureCache; //Needs to check the reference count
   friend class QExplicitlySharedDataPointer<QPlatformPixmap>;

   QAtomicInt ref;
   int detach_no;

   PixelType type;
   int id;
   int ser_no;
   uint is_cached;
};

#  define QT_XFORM_TYPE_MSBFIRST 0
#  define QT_XFORM_TYPE_LSBFIRST 1

extern bool qt_xForm_helper(const QTransform &, int, int, int, uchar *, int, int, int, const uchar *, int, int, int);

#endif
