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

#include <qgraphicssystem_runtime_p.h>
#include <qgraphicssystem_raster_p.h>
#include <qgraphicssystemfactory_p.h>
#include <qapplication_p.h>
#include <qwidget_p.h>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtGui/QBitmap>

QT_BEGIN_NAMESPACE

static int qt_pixmap_serial = 0;

#define READBACK(f)                                         \
    f                                                       \
    readBackInfo();


class QDeferredGraphicsSystemChange : public QObject
{
   GUI_CS_OBJECT(QDeferredGraphicsSystemChange)

 public:
   QDeferredGraphicsSystemChange(QRuntimeGraphicsSystem *gs, const QString &graphicsSystemName)
      : m_graphicsSystem(gs), m_graphicsSystemName(graphicsSystemName) {
   }

   void launch() {
      QTimer::singleShot(0, this, SLOT(doChange()));
   }

 private:
   GUI_CS_SLOT_1(Private, void doChange())
   GUI_CS_SLOT_2(doChange)

   QRuntimeGraphicsSystem *m_graphicsSystem;
   QString m_graphicsSystemName;
};

void QDeferredGraphicsSystemChange::doChange()
{
   m_graphicsSystem->setGraphicsSystem(m_graphicsSystemName);
   deleteLater();
}

QRuntimePixmapData::QRuntimePixmapData(const QRuntimeGraphicsSystem *gs, PixelType type)
   : QPixmapData(type, RuntimeClass), m_graphicsSystem(gs)
{
   setSerialNumber(++qt_pixmap_serial);
}

QRuntimePixmapData::~QRuntimePixmapData()
{
   if (QApplicationPrivate::graphics_system) {
      m_graphicsSystem->removePixmapData(this);
   }
   delete m_data;
}

void QRuntimePixmapData::readBackInfo()
{
   w = m_data->width();
   h = m_data->height();
   d = m_data->depth();
   is_null = m_data->isNull();
}


QPixmapData *QRuntimePixmapData::createCompatiblePixmapData() const
{
   QRuntimePixmapData *rtData = new QRuntimePixmapData(m_graphicsSystem, pixelType());
   rtData->m_data = m_data->createCompatiblePixmapData();
   return rtData;
}


void QRuntimePixmapData::resize(int width, int height)
{
   READBACK(
      m_data->resize(width, height);
   )
}


void QRuntimePixmapData::fromImage(const QImage &image,
                                   Qt::ImageConversionFlags flags)
{
   READBACK(
      m_data->fromImage(image, flags);
   )
}


bool QRuntimePixmapData::fromFile(const QString &filename, const char *format,
                                  Qt::ImageConversionFlags flags)
{
   bool success(false);
   READBACK(
      success = m_data->fromFile(filename, format, flags);
   )
   return success;
}

bool QRuntimePixmapData::fromData(const uchar *buffer, uint len, const char *format,
                                  Qt::ImageConversionFlags flags)
{
   bool success(false);
   READBACK(
      success = m_data->fromData(buffer, len, format, flags);
   )
   return success;
}


void QRuntimePixmapData::copy(const QPixmapData *data, const QRect &rect)
{
   if (data->runtimeData()) {
      READBACK(
         m_data->copy(data->runtimeData(), rect);
      )
   } else {
      READBACK(
         m_data->copy(data, rect);
      )
   }
}

bool QRuntimePixmapData::scroll(int dx, int dy, const QRect &rect)
{
   return m_data->scroll(dx, dy, rect);
}


int QRuntimePixmapData::metric(QPaintDevice::PaintDeviceMetric metric) const
{
   return m_data->metric(metric);
}

void QRuntimePixmapData::fill(const QColor &color)
{
   return m_data->fill(color);
}

QBitmap QRuntimePixmapData::mask() const
{
   return m_data->mask();
}

void QRuntimePixmapData::setMask(const QBitmap &mask)
{
   READBACK(
      m_data->setMask(mask);
   )
}

bool QRuntimePixmapData::hasAlphaChannel() const
{
   return m_data->hasAlphaChannel();
}

QPixmap QRuntimePixmapData::transformed(const QTransform &matrix,
                                        Qt::TransformationMode mode) const
{
   return m_data->transformed(matrix, mode);
}

void QRuntimePixmapData::setAlphaChannel(const QPixmap &alphaChannel)
{
   READBACK(
      m_data->setAlphaChannel(alphaChannel);
   )
}

QPixmap QRuntimePixmapData::alphaChannel() const
{
   return m_data->alphaChannel();
}

QImage QRuntimePixmapData::toImage() const
{
   return m_data->toImage();
}

QPaintEngine *QRuntimePixmapData::paintEngine() const
{
   return m_data->paintEngine();
}

QImage *QRuntimePixmapData::buffer()
{
   return m_data->buffer();
}

QPixmapData *QRuntimePixmapData::runtimeData() const
{
   return m_data;
}

QRuntimeWindowSurface::QRuntimeWindowSurface(const QRuntimeGraphicsSystem *gs, QWidget *window)
   : QWindowSurface(window), m_graphicsSystem(gs)
{

}

QRuntimeWindowSurface::~QRuntimeWindowSurface()
{
   if (QApplicationPrivate::graphics_system) {
      m_graphicsSystem->removeWindowSurface(this);
   }
}

QPaintDevice *QRuntimeWindowSurface::paintDevice()
{
   return m_windowSurface->paintDevice();
}

void QRuntimeWindowSurface::flush(QWidget *widget, const QRegion &region,
                                  const QPoint &offset)
{
   m_windowSurface->flush(widget, region, offset);

   int destroyPolicy = m_graphicsSystem->windowSurfaceDestroyPolicy();
   if (m_pendingWindowSurface &&
         destroyPolicy == QRuntimeGraphicsSystem::DestroyAfterFirstFlush) {
#ifdef QT_DEBUG
      qDebug() << "QRuntimeWindowSurface::flush() - destroy pending window surface";
#endif
      m_pendingWindowSurface.reset();
   }
}

void QRuntimeWindowSurface::setGeometry(const QRect &rect)
{
   QWindowSurface::setGeometry(rect);
   m_windowSurface->setGeometry(rect);
}

bool QRuntimeWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
   return m_windowSurface->scroll(area, dx, dy);
}

void QRuntimeWindowSurface::beginPaint(const QRegion &rgn)
{
   m_windowSurface->beginPaint(rgn);
}

void QRuntimeWindowSurface::endPaint(const QRegion &rgn)
{
   m_windowSurface->endPaint(rgn);
}

QImage *QRuntimeWindowSurface::buffer(const QWidget *widget)
{
   return m_windowSurface->buffer(widget);
}

QPixmap QRuntimeWindowSurface::grabWidget(const QWidget *widget, const QRect &rectangle) const
{
   return m_windowSurface->grabWidget(widget, rectangle);
}

QPoint QRuntimeWindowSurface::offset(const QWidget *widget) const
{
   return m_windowSurface->offset(widget);
}

QWindowSurface::WindowSurfaceFeatures QRuntimeWindowSurface::features() const
{
   return m_windowSurface->features();
}

QRuntimeGraphicsSystem::QRuntimeGraphicsSystem()
   : m_windowSurfaceDestroyPolicy(DestroyImmediately), m_graphicsSystem(0)
{
   QApplicationPrivate::runtime_graphics_system = true;

   if (! qgetenv("QT_DEFAULT_RUNTIME_SYSTEM").isEmpty()) {
      m_graphicsSystemName = QString::fromUtf8(qgetenv("QT_DEFAULT_RUNTIME_SYSTEM"));
   } else {

#ifdef QT_DEFAULT_RUNTIME_SYSTEM
      m_graphicsSystemName = QLatin1String(QT_DEFAULT_RUNTIME_SYSTEM);

      if (m_graphicsSystemName.isNull())
#endif
         m_graphicsSystemName = QLatin1String("raster");
   }

   m_graphicsSystem = QGraphicsSystemFactory::create(m_graphicsSystemName);

   QApplicationPrivate::graphics_system_name = QLatin1String("runtime");
}


QPixmapData *QRuntimeGraphicsSystem::createPixmapData(QPixmapData::PixelType type) const
{
   Q_ASSERT(m_graphicsSystem);
   QPixmapData *data = m_graphicsSystem->createPixmapData(type);

   QRuntimePixmapData *rtData = new QRuntimePixmapData(this, type);
   rtData->m_data = data;
   m_pixmapDatas << rtData;

   return rtData;
}

QWindowSurface *QRuntimeGraphicsSystem::createWindowSurface(QWidget *widget) const
{
   Q_ASSERT(m_graphicsSystem);
   QRuntimeWindowSurface *rtSurface = new QRuntimeWindowSurface(this, widget);
   rtSurface->m_windowSurface.reset(m_graphicsSystem->createWindowSurface(widget));
   widget->setWindowSurface(rtSurface);
   m_windowSurfaces << rtSurface;
   return rtSurface;
}

void QRuntimeGraphicsSystem::setGraphicsSystem(const QString &name)
{
   if (m_graphicsSystemName == name) {
      return;
   }
#ifdef QT_DEBUG
   qDebug() << "QRuntimeGraphicsSystem::setGraphicsSystem( " << name << " )";
#endif
   QGraphicsSystem *oldSystem = m_graphicsSystem;
   m_graphicsSystem = QGraphicsSystemFactory::create(name);
   m_graphicsSystemName = name;

   Q_ASSERT(m_graphicsSystem);

   m_pendingGraphicsSystemName = QString();

   for (int i = 0; i < m_pixmapDatas.size(); ++i) {
      QRuntimePixmapData *proxy = m_pixmapDatas.at(i);
      QPixmapData *newData = m_graphicsSystem->createPixmapData(proxy->m_data);
      newData->fromImage(proxy->m_data->toImage(), Qt::NoOpaqueDetection);
      delete proxy->m_data;
      proxy->m_data = newData;
      proxy->readBackInfo();
   }

   for (int i = 0; i < m_windowSurfaces.size(); ++i) {
      QRuntimeWindowSurface *proxy = m_windowSurfaces.at(i);
      QWidget *widget = proxy->m_windowSurface->window();

      if (m_windowSurfaceDestroyPolicy == DestroyAfterFirstFlush) {
         proxy->m_pendingWindowSurface.reset(proxy->m_windowSurface.take());
      }

      QWindowSurface *newWindowSurface = m_graphicsSystem->createWindowSurface(widget);
      newWindowSurface->setGeometry(proxy->geometry());

      proxy->m_windowSurface.reset(newWindowSurface);
      qt_widget_private(widget)->invalidateBuffer(widget->rect());
   }

   delete oldSystem;
}

void QRuntimeGraphicsSystem::removePixmapData(QRuntimePixmapData *pixmapData) const
{
   int index = m_pixmapDatas.lastIndexOf(pixmapData);
   m_pixmapDatas.removeAt(index);
}

void QRuntimeGraphicsSystem::removeWindowSurface(QRuntimeWindowSurface *windowSurface) const
{
   int index = m_windowSurfaces.lastIndexOf(windowSurface);
   m_windowSurfaces.removeAt(index);
}

QT_END_NAMESPACE
