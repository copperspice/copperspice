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

#include <qgraphicssvgitem.h>

#if ! defined(QT_NO_GRAPHICSVIEW) && ! defined(QT_NO_SVGWIDGET)

#include <qdebug.h>
#include <qpainter.h>
#include <qstyleoption.h>
#include <qsvgrenderer.h>

#include <qgraphics_item_p.h>

class QGraphicsSvgItemPrivate : public QGraphicsItemPrivate
{
 public:
   Q_DECLARE_PUBLIC(QGraphicsSvgItem)

   QGraphicsSvgItemPrivate()
      : renderer(nullptr), shared(false)
   {
   }

   void init(QGraphicsItem *parent) {
      Q_Q(QGraphicsSvgItem);
      q->setParentItem(parent);
      renderer = new QSvgRenderer(q);

      QObject::connect(renderer, SIGNAL(repaintNeeded()), q, SLOT(_q_repaintItem()));
      q->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
      q->setMaximumCacheSize(QSize(1024, 768));
   }

   void _q_repaintItem() {
      q_func()->update();
   }

   inline void updateDefaultSize() {
      QRectF bounds;
      if (elemId.isEmpty()) {
         bounds = QRectF(QPointF(0, 0), renderer->defaultSize());
      } else {
         bounds = renderer->boundsOnElement(elemId);
      }
      if (boundingRect.size() != bounds.size()) {
         q_func()->prepareGeometryChange();
         boundingRect.setSize(bounds.size());
      }
   }

   QSvgRenderer *renderer;
   QRectF boundingRect;
   bool shared;
   QString elemId;
};

QGraphicsSvgItem::QGraphicsSvgItem(QGraphicsItem *parent)
   : QGraphicsObject(*new QGraphicsSvgItemPrivate(), nullptr)
{
   Q_D(QGraphicsSvgItem);
   d->init(parent);
}

QGraphicsSvgItem::QGraphicsSvgItem(const QString &fileName, QGraphicsItem *parent)
   : QGraphicsObject(*new QGraphicsSvgItemPrivate(), nullptr)
{
   Q_D(QGraphicsSvgItem);
   d->init(parent);
   d->renderer->load(fileName);
   d->updateDefaultSize();
}

QSvgRenderer *QGraphicsSvgItem::renderer() const
{
   return d_func()->renderer;
}

QRectF QGraphicsSvgItem::boundingRect() const
{
   Q_D(const QGraphicsSvgItem);
   return d->boundingRect;
}

static void qt_graphicsItem_highlightSelected(
   QGraphicsItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option)
{
   const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
   if (qFuzzyIsNull(qMax(murect.width(), murect.height()))) {
      return;
   }

   const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
   if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0)) {
      return;
   }

   qreal itemPenWidth;
   switch (item->type()) {
      case QGraphicsEllipseItem::Type:
         itemPenWidth = static_cast<QGraphicsEllipseItem *>(item)->pen().widthF();
         break;
      case QGraphicsPathItem::Type:
         itemPenWidth = static_cast<QGraphicsPathItem *>(item)->pen().widthF();
         break;
      case QGraphicsPolygonItem::Type:
         itemPenWidth = static_cast<QGraphicsPolygonItem *>(item)->pen().widthF();
         break;
      case QGraphicsRectItem::Type:
         itemPenWidth = static_cast<QGraphicsRectItem *>(item)->pen().widthF();
         break;
      case QGraphicsSimpleTextItem::Type:
         itemPenWidth = static_cast<QGraphicsSimpleTextItem *>(item)->pen().widthF();
         break;
      case QGraphicsLineItem::Type:
         itemPenWidth = static_cast<QGraphicsLineItem *>(item)->pen().widthF();
         break;
      default:
         itemPenWidth = 1.0;
   }
   const qreal pad = itemPenWidth / 2;

   const qreal penWidth = 0; // cosmetic pen

   const QColor fgcolor = option->palette.windowText().color();
   const QColor bgcolor( // ensure good contrast against fgcolor
      fgcolor.red()   > 127 ? 0 : 255,
      fgcolor.green() > 127 ? 0 : 255,
      fgcolor.blue()  > 127 ? 0 : 255);

   painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
   painter->setBrush(Qt::NoBrush);
   painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));

   painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
   painter->setBrush(Qt::NoBrush);
   painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
}

void QGraphicsSvgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   (void) widget;

   Q_D(QGraphicsSvgItem);
   if (!d->renderer->isValid()) {
      return;
   }

   if (d->elemId.isEmpty()) {
      d->renderer->render(painter, d->boundingRect);
   } else {
      d->renderer->render(painter, d->elemId, d->boundingRect);
   }

   if (option->state & QStyle::State_Selected) {
      qt_graphicsItem_highlightSelected(this, painter, option);
   }
}

int QGraphicsSvgItem::type() const
{
   return Type;
}

void QGraphicsSvgItem::setMaximumCacheSize(const QSize &size)
{
   QGraphicsItem::d_ptr->setExtra(QGraphicsItemPrivate::ExtraMaxDeviceCoordCacheSize, size);
   update();
}

QSize QGraphicsSvgItem::maximumCacheSize() const
{
   return QGraphicsItem::d_ptr->extra(QGraphicsItemPrivate::ExtraMaxDeviceCoordCacheSize).toSize();
}

void QGraphicsSvgItem::setElementId(const QString &id)
{
   Q_D(QGraphicsSvgItem);
   d->elemId = id;
   d->updateDefaultSize();
   update();
}

QString QGraphicsSvgItem::elementId() const
{
   Q_D(const QGraphicsSvgItem);
   return d->elemId;
}

void QGraphicsSvgItem::setSharedRenderer(QSvgRenderer *renderer)
{
   Q_D(QGraphicsSvgItem);
   if (!d->shared) {
      delete d->renderer;
   }

   d->renderer = renderer;
   d->shared = true;

   d->updateDefaultSize();

   update();
}

void QGraphicsSvgItem::setCachingEnabled(bool caching)
{
   setCacheMode(caching ? QGraphicsItem::DeviceCoordinateCache : QGraphicsItem::NoCache);
}

bool QGraphicsSvgItem::isCachingEnabled() const
{
   return cacheMode() != QGraphicsItem::NoCache;
}

void QGraphicsSvgItem::_q_repaintItem()
{
   Q_D(QGraphicsSvgItem);
   d->_q_repaintItem();
}

#endif // QT_NO_GRAPHICSSVGITEM
