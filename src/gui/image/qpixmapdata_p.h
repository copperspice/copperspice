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

#ifndef QPIXMAPDATA_P_H
#define QPIXMAPDATA_P_H

#include <QtGui/qpixmap.h>
#include <QtCore/qatomic.h>

QT_BEGIN_NAMESPACE

class QImageReader;

class Q_GUI_EXPORT QPixmapData
{
 public:
   enum PixelType {
      // WARNING: Do not change the first two must match QPixmap::Type
      PixmapType, BitmapType
   };

   enum ClassId { RasterClass, X11Class, MacClass, DirectFBClass,
                  OpenGLClass, OpenVGClass, RuntimeClass, BlitterClass,
                  CustomClass = 1024
                };

   QPixmapData(PixelType pixelType, int classId);
   virtual ~QPixmapData();

   virtual QPixmapData *createCompatiblePixmapData() const;

   virtual void resize(int width, int height) = 0;
   virtual void fromImage(const QImage &image, Qt::ImageConversionFlags flags) = 0;
   virtual void fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags);

   virtual bool fromFile(const QString &filename, const char *format, Qt::ImageConversionFlags flags);

   virtual bool fromData(const uchar *buffer, uint len, const char *format, Qt::ImageConversionFlags flags);

   virtual void copy(const QPixmapData *data, const QRect &rect);
   virtual bool scroll(int dx, int dy, const QRect &rect);

   virtual int metric(QPaintDevice::PaintDeviceMetric metric) const = 0;
   virtual void fill(const QColor &color) = 0;
   virtual QBitmap mask() const;
   virtual void setMask(const QBitmap &mask);
   virtual bool hasAlphaChannel() const = 0;
   virtual QPixmap transformed(const QTransform &matrix,Qt::TransformationMode mode) const;
   virtual void setAlphaChannel(const QPixmap &alphaChannel);
   virtual QPixmap alphaChannel() const;
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

   virtual QImage *buffer();

   inline int width() const {
      return w;
   }
   inline int height() const {
      return h;
   }
   QT_DEPRECATED inline int numColors() const {
      return metric(QPaintDevice::PdmNumColors);
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

   static QPixmapData *create(int w, int h, PixelType type);

   virtual QPixmapData *runtimeData() const {
      return 0;
   }

 protected:
   void setSerialNumber(int serNo);
   int w;
   int h;
   int d;
   bool is_null;

 private:
   friend class QPixmap;
   friend class QX11PixmapData;
   friend class QSymbianRasterPixmapData;
   friend class QImagePixmapCleanupHooks; // Needs to set is_cached
   friend class QGLTextureCache; //Needs to check the reference count
   friend class QExplicitlySharedDataPointer<QPixmapData>;

   QAtomicInt ref;
   int detach_no;

   PixelType type;
   int id;
   int ser_no;
   uint is_cached;
};

#  define QT_XFORM_TYPE_MSBFIRST 0
#  define QT_XFORM_TYPE_LSBFIRST 1
#  if defined(Q_OS_WIN)
#    define QT_XFORM_TYPE_WINDOWSPIXMAP 2
#  endif

extern bool qt_xForm_helper(const QTransform &, int, int, int, uchar *, int, int, int, const uchar *, int, int, int);

QT_END_NAMESPACE

#endif // QPIXMAPDATA_P_H
