/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
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

   virtual QPixmapData *createCompatiblePixmapData() const;
   virtual void resize(int width, int height);
   virtual void fromImage(const QImage &image, Qt::ImageConversionFlags flags);

   virtual bool fromFile(const QString &filename, const char *format,
                         Qt::ImageConversionFlags flags);

   virtual bool fromData(const uchar *buffer, uint len, const char *format,
                         Qt::ImageConversionFlags flags);

   virtual void copy(const QPixmapData *data, const QRect &rect);
   virtual bool scroll(int dx, int dy, const QRect &rect);

   virtual int metric(QPaintDevice::PaintDeviceMetric metric) const;
   virtual void fill(const QColor &color);
   virtual QBitmap mask() const;
   virtual void setMask(const QBitmap &mask);
   virtual bool hasAlphaChannel() const;
   virtual QPixmap transformed(const QTransform &matrix, Qt::TransformationMode mode) const;
   virtual void setAlphaChannel(const QPixmap &alphaChannel);
   virtual QPixmap alphaChannel() const;
   virtual QImage toImage() const;
   virtual QPaintEngine *paintEngine() const;

   virtual QImage *buffer();

   void readBackInfo();

   QPixmapData *m_data;
   virtual QPixmapData *runtimeData() const;

 private:
   const QRuntimeGraphicsSystem *m_graphicsSystem;

};

class QRuntimeWindowSurface : public QWindowSurface
{
 public:
   QRuntimeWindowSurface(const QRuntimeGraphicsSystem *gs, QWidget *window);
   ~QRuntimeWindowSurface();

   virtual QPaintDevice *paintDevice();
   virtual void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
   virtual void setGeometry(const QRect &rect);

   virtual bool scroll(const QRegion &area, int dx, int dy);

   virtual void beginPaint(const QRegion &);
   virtual void endPaint(const QRegion &);

   virtual QImage *buffer(const QWidget *widget);
   virtual QPixmap grabWidget(const QWidget *widget, const QRect &rectangle = QRect()) const;

   virtual QPoint offset(const QWidget *widget) const;

   virtual WindowSurfaceFeatures features() const;

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

 public:
   QRuntimeGraphicsSystem();

   QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
   QWindowSurface *createWindowSurface(QWidget *widget) const;

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
