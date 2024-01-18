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

#include <qgraphicstransform.h>
#include <qdebug.h>
#include <qmath.h>
#include <qnumeric.h>

#include <qgraphics_item_p.h>
#include <qgraphics_transform_p.h>

#ifndef QT_NO_GRAPHICSVIEW

QGraphicsTransformPrivate::~QGraphicsTransformPrivate()
{
}

void QGraphicsTransformPrivate::setItem(QGraphicsItem *i)
{
   if (item == i) {
      return;
   }

   if (item) {
      Q_Q(QGraphicsTransform);
      QGraphicsItemPrivate *d_ptr = item->d_ptr.data();

      item->prepareGeometryChange();
      Q_ASSERT(d_ptr->transformData);
      d_ptr->transformData->graphicsTransforms.removeAll(q);
      d_ptr->dirtySceneTransform = 1;
      item = nullptr;
   }

   item = i;
}

void QGraphicsTransformPrivate::updateItem(QGraphicsItem *item)
{
   item->prepareGeometryChange();
   item->d_ptr->dirtySceneTransform = 1;
}

QGraphicsTransform::QGraphicsTransform(QObject *parent)
   : QObject(parent), d_ptr(new QGraphicsTransformPrivate)
{
   d_ptr->q_ptr = this;
}

// internal
QGraphicsTransform::QGraphicsTransform(QGraphicsTransformPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QGraphicsTransform::~QGraphicsTransform()
{
   Q_D(QGraphicsTransform);
   d->setItem(nullptr);
}

void QGraphicsTransform::update()
{
   Q_D(QGraphicsTransform);
   if (d->item) {
      d->updateItem(d->item);
   }
}

class QGraphicsScalePrivate : public QGraphicsTransformPrivate
{
 public:
   QGraphicsScalePrivate()
      : xScale(1), yScale(1), zScale(1) {}
   QVector3D origin;
   qreal xScale;
   qreal yScale;
   qreal zScale;
};

QGraphicsScale::QGraphicsScale(QObject *parent)
   : QGraphicsTransform(*new QGraphicsScalePrivate, parent)
{
}

QGraphicsScale::~QGraphicsScale()
{
}

QVector3D QGraphicsScale::origin() const
{
   Q_D(const QGraphicsScale);
   return d->origin;
}
void QGraphicsScale::setOrigin(const QVector3D &point)
{
   Q_D(QGraphicsScale);
   if (d->origin == point) {
      return;
   }
   d->origin = point;
   update();
   emit originChanged();
}

qreal QGraphicsScale::xScale() const
{
   Q_D(const QGraphicsScale);
   return d->xScale;
}
void QGraphicsScale::setXScale(qreal scale)
{
   Q_D(QGraphicsScale);
   if (d->xScale == scale) {
      return;
   }
   d->xScale = scale;
   update();
   emit xScaleChanged();
   emit scaleChanged();
}

qreal QGraphicsScale::yScale() const
{
   Q_D(const QGraphicsScale);
   return d->yScale;
}

void QGraphicsScale::setYScale(qreal scale)
{
   Q_D(QGraphicsScale);
   if (d->yScale == scale) {
      return;
   }
   d->yScale = scale;
   update();
   emit yScaleChanged();
   emit scaleChanged();
}

qreal QGraphicsScale::zScale() const
{
   Q_D(const QGraphicsScale);
   return d->zScale;
}

void QGraphicsScale::setZScale(qreal scale)
{
   Q_D(QGraphicsScale);
   if (d->zScale == scale) {
      return;
   }
   d->zScale = scale;
   update();
   emit zScaleChanged();
   emit scaleChanged();
}

void QGraphicsScale::applyTo(QMatrix4x4 *matrix) const
{
   Q_D(const QGraphicsScale);
   matrix->translate(d->origin);
   matrix->scale(d->xScale, d->yScale, d->zScale);
   matrix->translate(-d->origin);
}

class QGraphicsRotationPrivate : public QGraphicsTransformPrivate
{
 public:
   QGraphicsRotationPrivate()
      : angle(0), axis(0, 0, 1) {}
   QVector3D origin;
   qreal angle;
   QVector3D axis;
};

QGraphicsRotation::QGraphicsRotation(QObject *parent)
   : QGraphicsTransform(*new QGraphicsRotationPrivate, parent)
{
}

QGraphicsRotation::~QGraphicsRotation()
{
}

QVector3D QGraphicsRotation::origin() const
{
   Q_D(const QGraphicsRotation);
   return d->origin;
}

void QGraphicsRotation::setOrigin(const QVector3D &point)
{
   Q_D(QGraphicsRotation);
   if (d->origin == point) {
      return;
   }
   d->origin = point;
   update();
   emit originChanged();
}

qreal QGraphicsRotation::angle() const
{
   Q_D(const QGraphicsRotation);
   return d->angle;
}

void QGraphicsRotation::setAngle(qreal angle)
{
   Q_D(QGraphicsRotation);
   if (d->angle == angle) {
      return;
   }
   d->angle = angle;
   update();
   emit angleChanged();
}

QVector3D QGraphicsRotation::axis() const
{
   Q_D(const QGraphicsRotation);
   return d->axis;
}

void QGraphicsRotation::setAxis(const QVector3D &axis)
{
   Q_D(QGraphicsRotation);
   if (d->axis == axis) {
      return;
   }
   d->axis = axis;
   update();
   emit axisChanged();
}

void QGraphicsRotation::setAxis(Qt::Axis axis)
{
   switch (axis) {
      case Qt::XAxis:
         setAxis(QVector3D(1, 0, 0));
         break;
      case Qt::YAxis:
         setAxis(QVector3D(0, 1, 0));
         break;
      case Qt::ZAxis:
         setAxis(QVector3D(0, 0, 1));
         break;
   }
}

void QGraphicsRotation::applyTo(QMatrix4x4 *matrix) const
{
   Q_D(const QGraphicsRotation);

   if (d->angle == 0. || d->axis.isNull() || qIsNaN(d->angle)) {
      return;
   }

   matrix->translate(d->origin);
   matrix->projectedRotate(d->angle, d->axis.x(), d->axis.y(), d->axis.z());
   matrix->translate(-d->origin);
}

#endif //QT_NO_GRAPHICSVIEW
