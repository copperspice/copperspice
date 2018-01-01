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

#ifndef QPIXMAP_X11_P_H
#define QPIXMAP_X11_P_H

#include <qpixmapdata_p.h>
#include <qpixmapdatafactory_p.h>
#include <qx11info_x11.h>

QT_BEGIN_NAMESPACE

class QX11PaintEngine;
struct QXImageWrapper;

class Q_GUI_EXPORT QX11PixmapData : public QPixmapData
{
 public:
   QX11PixmapData(PixelType type);
   //     QX11PixmapData(PixelType type, int width, int height);
   //     QX11PixmapData(PixelType type, const QImage &image,Qt::ImageConversionFlags flags);
   ~QX11PixmapData();

   QPixmapData *createCompatiblePixmapData() const override;

   void resize(int width, int height) override;
   void fromImage(const QImage &image, Qt::ImageConversionFlags flags) override;
   void copy(const QPixmapData *data, const QRect &rect) override;
   bool scroll(int dx, int dy, const QRect &rect) override;

   void fill(const QColor &color) override;
   QBitmap mask() const override;
   void setMask(const QBitmap &mask) override;
   bool hasAlphaChannel() const override;
   void setAlphaChannel(const QPixmap &alphaChannel) override;
   QPixmap alphaChannel() const override;
   QPixmap transformed(const QTransform &transform, Qt::TransformationMode mode) const override;
   QImage toImage() const override;
   QImage toImage(const QRect &rect) const override;
   QPaintEngine *paintEngine() const override;

   Qt::HANDLE handle() const {
      return hd;
   }
   Qt::HANDLE x11ConvertToDefaultDepth();

   static Qt::HANDLE createBitmapFromImage(const QImage &image);

   void *gl_surface;

#ifndef QT_NO_XRENDER
   void convertToARGB32(bool preserveContents = true);
#endif

 protected:
   int metric(QPaintDevice::PaintDeviceMetric metric) const override;

 private:
   friend class QPixmap;
   friend class QBitmap;
   friend class QX11PaintEngine;
   friend class QX11WindowSurface;
   friend class QRasterWindowSurface;
   friend class QGLContextPrivate; // Needs to access xinfo, gl_surface & flags
   friend class QEglContext; // Needs gl_surface
   friend class QGLContext; // Needs gl_surface
   friend class QX11GLPixmapData; // Needs gl_surface
   friend class QMeeGoLivePixmapData; // Needs gl_surface and flags
   friend bool  qt_createEGLSurfaceForPixmap(QPixmapData *, bool); // Needs gl_surface

   void release();

   QImage toImage(const QXImageWrapper &xi, const QRect &rect) const;

   QBitmap mask_to_bitmap(int screen) const;
   static Qt::HANDLE bitmap_to_mask(const QBitmap &, int screen);
   void bitmapFromImage(const QImage &image);

   bool canTakeQImageFromXImage(const QXImageWrapper &xi) const;
   QImage takeQImageFromXImage(const QXImageWrapper &xi) const;

   Qt::HANDLE hd;

   enum Flag {
      NoFlags = 0x0,
      Uninitialized = 0x1,
      Readonly = 0x2,
      InvertedWhenBoundToTexture = 0x4,
      GlSurfaceCreatedWithAlpha = 0x8
   };
   uint flags;

   QX11Info xinfo;
   Qt::HANDLE x11_mask;
   Qt::HANDLE picture;
   Qt::HANDLE mask_picture;
   Qt::HANDLE hd2; // sorted in the default display depth
   QPixmap::ShareMode share_mode;

   QX11PaintEngine *pengine;
};

QT_END_NAMESPACE

#endif // QPIXMAPDATA_X11_P_H

