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

#ifndef QGRAPHICSSYSTEM_RUNTIME_P_H
#define QGRAPHICSSYSTEM_RUNTIME_P_H

#include <qgraphicssystem_p.h>
#include <qpixmapdata_p.h>

QT_BEGIN_NAMESPACE

class QRuntimeGraphicsSystem;

class Q_GUI_EXPORT QRuntimePixmapData : public QPixmapData
{

 public:
   QRuntimePixmapData(const QRuntimeGraphicsSystem *gs, PixelType type);
   ~QRuntimePixmapData();

   QPixmapData *createCompatiblePixmapData() const override;
   void resize(int width, int height) override;
   void fromImage(const QImage &image, Qt::ImageConversionFlags flags) override;

   bool fromFile(const QString &filename, const char *format, Qt::ImageConversionFlags flags) override;
   bool fromData(const uchar *buffer, uint len, const char *format, Qt::ImageConversionFlags flags) override;

   void copy(const QPixmapData *data, const QRect &rect) override;
   bool scroll(int dx, int dy, const QRect &rect) override;

   int metric(QPaintDevice::PaintDeviceMetric metric) const override;
   void fill(const QColor &color) override;
   QBitmap mask() const override;
   void setMask(const QBitmap &mask) override;
   bool hasAlphaChannel() const override;
   QPixmap transformed(const QTransform &matrix, Qt::TransformationMode mode) const override;
   void setAlphaChannel(const QPixmap &alphaChannel) override;
   QPixmap alphaChannel() const override;
   QImage toImage() const override;
   QPaintEngine *paintEngine() const override;

   QImage *buffer() override;

   void readBackInfo();

   QPixmapData *m_data;
   virtual QPixmapData *runtimeData() const override;

 private:
   const QRuntimeGraphicsSystem *m_graphicsSystem;

};

class QRuntimeWindowSurface : public QWindowSurface
{
 public:
   QRuntimeWindowSurface(const QRuntimeGraphicsSystem *gs, QWidget *window);
   ~QRuntimeWindowSurface();

   QPaintDevice *paintDevice() override;
   void flush(QWidget *widget, const QRegion &region, const QPoint &offset) override;
   void setGeometry(const QRect &rect) override;

   bool scroll(const QRegion &area, int dx, int dy) override;

   void beginPaint(const QRegion &) override;
   void endPaint(const QRegion &) override;

   QImage *buffer(const QWidget *widget) override;
   QPixmap grabWidget(const QWidget *widget, const QRect &rectangle = QRect()) const override;

   QPoint offset(const QWidget *widget) const override;
   WindowSurfaceFeatures features() const override;

   QScopedPointer<QWindowSurface> m_windowSurface;
   QScopedPointer<QWindowSurface> m_pendingWindowSurface;

 private:
   const QRuntimeGraphicsSystem *m_graphicsSystem;
};

class QRuntimeGraphicsSystem : public QGraphicsSystem
{
 public:

   enum WindowSurfaceDestroyPolicy {
      DestroyImmediately,
      DestroyAfterFirstFlush
   };
 
   QRuntimeGraphicsSystem();

   QPixmapData *createPixmapData(QPixmapData::PixelType type) const override;
   QWindowSurface *createWindowSurface(QWidget *widget) const override;

   void removePixmapData(QRuntimePixmapData *pixmapData) const;
   void removeWindowSurface(QRuntimeWindowSurface *windowSurface) const;

   void setGraphicsSystem(const QString &name);
   QString graphicsSystemName() const {
      return m_graphicsSystemName;
   }

   void setWindowSurfaceDestroyPolicy(WindowSurfaceDestroyPolicy policy) {
      m_windowSurfaceDestroyPolicy = policy;
   }

   int windowSurfaceDestroyPolicy() const {
      return m_windowSurfaceDestroyPolicy;
   }

 private:
   int m_windowSurfaceDestroyPolicy;
   QGraphicsSystem *m_graphicsSystem;
   mutable QList<QRuntimePixmapData *> m_pixmapDatas;
   mutable QList<QRuntimeWindowSurface *> m_windowSurfaces;
   QString m_graphicsSystemName;

   QString m_pendingGraphicsSystemName;

   friend class QRuntimePixmapData;
   friend class QRuntimeWindowSurface;
   friend class QMeeGoGraphicsSystem;
};

QT_END_NAMESPACE

#endif
