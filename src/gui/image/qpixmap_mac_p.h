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

#ifndef QPIXMAP_MAC_P_H
#define QPIXMAP_MAC_P_H

#include <qpixmapdata_p.h>
#include <qpixmapdatafactory_p.h>
#include <qt_mac_p.h>

QT_BEGIN_NAMESPACE

class QMacPixmapData : public QPixmapData
{
 public:
   QMacPixmapData(PixelType type);
   ~QMacPixmapData();

   QPixmapData *createCompatiblePixmapData() const;

   void resize(int width, int height);
   void fromImage(const QImage &image, Qt::ImageConversionFlags flags);
   void copy(const QPixmapData *data, const QRect &rect);
   bool scroll(int dx, int dy, const QRect &rect);

   int metric(QPaintDevice::PaintDeviceMetric metric) const;
   void fill(const QColor &color);
   QBitmap mask() const;
   void setMask(const QBitmap &mask);
   bool hasAlphaChannel() const;
   //     QPixmap transformed(const QTransform &matrix,Qt::TransformationMode mode) const;
   void setAlphaChannel(const QPixmap &alphaChannel);
   QPixmap alphaChannel() const;
   QImage toImage() const;
   QPaintEngine *paintEngine() const;

 private:
   uint has_alpha : 1, has_mask : 1, uninit : 1;

   void macSetHasAlpha(bool b);
   void macGetAlphaChannel(QMacPixmapData *, bool asMask) const;
   void macSetAlphaChannel(const QMacPixmapData *, bool asMask);
   void macCreateCGImageRef();
   void macCreatePixels();
   void macReleaseCGImageRef();
   /*
       pixels stores the pixmap data. pixelsToFree is either 0 or some memory
       block that was bound to a CGImageRef and released, and for which the
       release callback has been called. There are two uses to pixelsToFree:

       1. If pixels == pixelsToFree, then we know that the CGImageRef is done\
          with the data and we can modify pixels without breaking CGImageRef's
          mutability invariant.

       2. If pixels != pixelsToFree and pixelsToFree != 0, then we can reuse
          pixelsToFree later on instead of malloc'ing memory.
   */
   quint32 *pixels;
   uint pixelsSize;
   quint32 *pixelsToFree;
   uint bytesPerRow;
   QRectF cg_mask_rect;
   CGImageRef cg_data, cg_dataBeingReleased, cg_mask;
   static QSet<QMacPixmapData *> validDataPointers;

   QPaintEngine *pengine;

   friend class QPixmap;
   friend class QRasterBuffer;
   friend class QRasterPaintEngine;
   friend class QCoreGraphicsPaintEngine;

   friend CGImageRef qt_mac_create_imagemask(const QPixmap &, const QRectF &);
   friend quint32 *qt_mac_pixmap_get_base(const QPixmap *);
   friend int qt_mac_pixmap_get_bytes_per_line(const QPixmap *);
   friend void qt_mac_cgimage_data_free(void *, const void *, size_t);
   friend IconRef qt_mac_create_iconref(const QPixmap &);
   friend CGContextRef qt_mac_cg_context(const QPaintDevice *);

   friend QColor qt_mac_colorForThemeTextColor(ThemeTextColor themeColor);
};

QT_END_NAMESPACE

#endif // QPIXMAP_MAC_P_H
