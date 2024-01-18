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

#include <qgraphicsvideoitem.h>

#include <qcoreevent.h>
#include <qpointer.h>
#include <qmediaobject.h>
#include <qmediaservice.h>
#include <qvideorenderercontrol.h>
#include <qvideosurfaceformat.h>

#include <qpaintervideosurface_p.h>

#if ! defined(QT_NO_OPENGL) && ! defined(QT_OPENGL_ES_1_CL) && ! defined(QT_OPENGL_ES_1)
#include <qgl.h>
#endif

class QGraphicsVideoItemPrivate
{
 public:
   QGraphicsVideoItemPrivate()
      : q_ptr(nullptr), surface(nullptr), mediaObject(nullptr), service(nullptr), rendererControl(nullptr),
        aspectRatioMode(Qt::KeepAspectRatio), updatePaintDevice(true), rect(0.0, 0.0, 320, 240)
   {
   }

   QGraphicsVideoItem *q_ptr;

   QPainterVideoSurface *surface;
   QPointer<QMediaObject> mediaObject;
   QMediaService *service;
   QVideoRendererControl *rendererControl;
   Qt::AspectRatioMode aspectRatioMode;
   bool updatePaintDevice;
   QRectF rect;
   QRectF boundingRect;
   QRectF sourceRect;
   QSizeF nativeSize;

   void clearService();
   void updateRects();

   void _q_present();
   void _q_formatChanged(const QVideoSurfaceFormat &format);
   void _q_updateNativeSize();
   void _q_serviceDestroyed();
};

void QGraphicsVideoItemPrivate::clearService()
{
   if (rendererControl) {
      surface->stop();
      rendererControl->setSurface(nullptr);
      service->releaseControl(rendererControl);
      rendererControl = nullptr;
   }

   if (service) {
      QObject::disconnect(service, &QMediaService::destroyed, q_ptr, &QGraphicsVideoItem::_q_serviceDestroyed);
      service = nullptr;
   }
}

void QGraphicsVideoItemPrivate::updateRects()
{
   q_ptr->prepareGeometryChange();

   if (nativeSize.isEmpty()) {
      //this is necessary for item to receive the
      //first paint event and configure video surface.
      boundingRect = rect;

   } else if (aspectRatioMode == Qt::IgnoreAspectRatio) {
      boundingRect = rect;
      sourceRect = QRectF(0, 0, 1, 1);

   } else if (aspectRatioMode == Qt::KeepAspectRatio) {
      QSizeF size = nativeSize;
      size.scale(rect.size(), Qt::KeepAspectRatio);

      boundingRect = QRectF(0, 0, size.width(), size.height());
      boundingRect.moveCenter(rect.center());

      sourceRect = QRectF(0, 0, 1, 1);

   } else if (aspectRatioMode == Qt::KeepAspectRatioByExpanding) {
      boundingRect = rect;

      QSizeF size = rect.size();
      size.scale(nativeSize, Qt::KeepAspectRatio);

      sourceRect = QRectF(
            0, 0, size.width() / nativeSize.width(), size.height() / nativeSize.height());
      sourceRect.moveCenter(QPointF(0.5, 0.5));
   }
}

void QGraphicsVideoItemPrivate::_q_present()
{
   if (q_ptr->isObscured()) {
      q_ptr->update(boundingRect);
      surface->setReady(true);

   } else {
      q_ptr->update(boundingRect);

   }
}

void QGraphicsVideoItemPrivate::_q_updateNativeSize()
{
   const QSize &size = surface->surfaceFormat().sizeHint();
   if (nativeSize != size) {
      nativeSize = size;

      updateRects();
      emit q_ptr->nativeSizeChanged(nativeSize);
   }
}

void QGraphicsVideoItemPrivate::_q_serviceDestroyed()
{
   rendererControl = nullptr;
   service         = nullptr;

   surface->stop();
}

QGraphicsVideoItem::QGraphicsVideoItem(QGraphicsItem *parent)
   : QGraphicsObject(parent), d_ptr(new QGraphicsVideoItemPrivate)
{
   d_ptr->q_ptr   = this;
   d_ptr->surface = new QPainterVideoSurface;

   connect(d_ptr->surface, &QPainterVideoSurface::frameChanged,         this, &QGraphicsVideoItem::_q_present);
   connect(d_ptr->surface, &QPainterVideoSurface::surfaceFormatChanged, this, &QGraphicsVideoItem::_q_updateNativeSize, Qt::QueuedConnection);
}

QGraphicsVideoItem::~QGraphicsVideoItem()
{
   if (d_ptr->rendererControl) {
      d_ptr->rendererControl->setSurface(nullptr);
      d_ptr->service->releaseControl(d_ptr->rendererControl);
   }

   delete d_ptr->surface;
   delete d_ptr;
}

QMediaObject *QGraphicsVideoItem::mediaObject() const
{
   return d_func()->mediaObject;
}

bool QGraphicsVideoItem::setMediaObject(QMediaObject *object)
{
   Q_D(QGraphicsVideoItem);

   if (object == d->mediaObject) {
      return true;
   }

   d->clearService();
   d->mediaObject = object;

   if (d->mediaObject) {
      d->service = d->mediaObject->service();

      if (d->service) {
         QMediaControl *control = d->service->requestControl(QVideoRendererControl_iid);

         if (control) {
            d->rendererControl = dynamic_cast<QVideoRendererControl *>(control);

            if (d->rendererControl) {
               // do not set the surface untill the item is painted
               // at least once and the surface is configured
               if (! d->updatePaintDevice) {
                  d->rendererControl->setSurface(d->surface);

               } else {
                  update(boundingRect());
               }

               connect(d->service, &QMediaService::destroyed, this, &QGraphicsVideoItem::_q_serviceDestroyed);

               return true;
            }
            if (control) {
               d->service->releaseControl(control);
            }
         }
      }
   }

   d->mediaObject = nullptr;

   return false;
}

Qt::AspectRatioMode QGraphicsVideoItem::aspectRatioMode() const
{
   return d_func()->aspectRatioMode;
}

void QGraphicsVideoItem::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   Q_D(QGraphicsVideoItem);

   d->aspectRatioMode = mode;
   d->updateRects();
}

QPointF QGraphicsVideoItem::offset() const
{
   return d_func()->rect.topLeft();
}

void QGraphicsVideoItem::setOffset(const QPointF &offset)
{
   Q_D(QGraphicsVideoItem);

   d->rect.moveTo(offset);
   d->updateRects();
}

QSizeF QGraphicsVideoItem::size() const
{
   return d_func()->rect.size();
}

void QGraphicsVideoItem::setSize(const QSizeF &size)
{
   Q_D(QGraphicsVideoItem);

   d->rect.setSize(size.isValid() ? size : QSizeF(0, 0));
   d->updateRects();
}

QSizeF QGraphicsVideoItem::nativeSize() const
{
   return d_func()->nativeSize;
}

QRectF QGraphicsVideoItem::boundingRect() const
{
   return d_func()->boundingRect;
}

void QGraphicsVideoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   (void) option;

   Q_D(QGraphicsVideoItem);

   if (d->surface && d->updatePaintDevice) {
      d->updatePaintDevice = false;

#if ! defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_1_CL) && ! defined(QT_OPENGL_ES_1)
      if (widget) {
         connect(widget, &QWidget::destroyed, d->surface, &QPainterVideoSurface::viewportDestroyed);
      }

      d->surface->setGLContext(const_cast<QGLContext *>(QGLContext::currentContext()));

      if (d->surface->supportedShaderTypes() & QPainterVideoSurface::GlslShader) {
         d->surface->setShaderType(QPainterVideoSurface::GlslShader);

      } else {
         d->surface->setShaderType(QPainterVideoSurface::FragmentProgramShader);
      }
#endif

      if (d->rendererControl && d->rendererControl->surface() != d->surface) {
         d->rendererControl->setSurface(d->surface);
      }
   }

   if (d->surface && d->surface->isActive()) {
      d->surface->paint(painter, d->boundingRect, d->sourceRect);
      d->surface->setReady(true);
   }
}

QVariant QGraphicsVideoItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
   return QGraphicsItem::itemChange(change, value);
}

void QGraphicsVideoItem::timerEvent(QTimerEvent *event)
{
   QGraphicsObject::timerEvent(event);
}

void QGraphicsVideoItem::_q_present()
{
   Q_D(QGraphicsVideoItem);
   d->_q_present();
}

void QGraphicsVideoItem::_q_updateNativeSize()
{
   Q_D(QGraphicsVideoItem);
   d->_q_updateNativeSize();
}

void QGraphicsVideoItem::_q_serviceDestroyed()
{
   Q_D(QGraphicsVideoItem);
   d->_q_serviceDestroyed();
}
