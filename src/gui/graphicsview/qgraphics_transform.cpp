/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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
      item = 0;
   }

   item = i;
}

void QGraphicsTransformPrivate::updateItem(QGraphicsItem *item)
{
   item->prepareGeometryChange();
   item->d_ptr->dirtySceneTransform = 1;
}

/*!
    Constructs a new QGraphicsTransform with the given \a parent.
*/
QGraphicsTransform::QGraphicsTransform(QObject *parent)
   : QObject(parent), d_ptr(new QGraphicsTransformPrivate)
{
   d_ptr->q_ptr = this;
}

/*!
    \internal
*/
QGraphicsTransform::QGraphicsTransform(QGraphicsTransformPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}


QGraphicsTransform::~QGraphicsTransform()
{
   Q_D(QGraphicsTransform);
   d->setItem(0);
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

/*!
    \property QGraphicsScale::xScale
    \brief the horizontal scale factor.

    The scale factor can be any real number; the default value is 1.0. If you
    set the factor to 0.0, the item will be collapsed to a single point. If you
    provide a negative value, the item will be mirrored horizontally around its
    origin.

    \sa yScale, zScale, origin
*/
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

/*!
    \property QGraphicsScale::yScale
    \brief the vertical scale factor.

    The scale factor can be any real number; the default value is 1.0. If you
    set the factor to 0.0, the item will be collapsed to a single point. If you
    provide a negative value, the item will be flipped vertically around its
    origin.

    \sa xScale, zScale, origin
*/
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

/*!
    \property QGraphicsScale::zScale
    \brief the depth scale factor.

    The scale factor can be any real number; the default value is 1.0. If you
    set the factor to 0.0, the item will be collapsed to a single point. If you
    provide a negative value, the item will be flipped end for end around its
    origin.

    \sa xScale, yScale, origin
*/
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

/*!
    \reimp
*/
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

/*!
    Destroys the graphics rotation.
*/
QGraphicsRotation::~QGraphicsRotation()
{
}

/*!
    \property QGraphicsRotation::origin
    \brief the origin of the rotation in 3D space.

    All rotations will be done relative to this point (i.e., this point
    will stay fixed, relative to the parent, when the item is rotated).

    \sa angle
*/
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

/*!
    \property QGraphicsRotation::angle
    \brief the angle for clockwise rotation, in degrees.

    The angle can be any real number; the default value is 0.0. A value of 180
    will rotate 180 degrees, clockwise. If you provide a negative number, the
    item will be rotated counter-clockwise. Normally the rotation angle will be
    in the range (-360, 360), but you can also provide numbers outside of this
    range (e.g., a angle of 370 degrees gives the same result as 10 degrees).
    Setting the angle to NaN results in no rotation.

    \sa origin
*/
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

/*!
    \fn void QGraphicsRotation::setAxis(Qt::Axis axis)

    Convenience function to set the axis to \a axis.

    Note: the Qt::YAxis rotation for QTransform is inverted from the
    correct mathematical rotation in 3D space.  The QGraphicsRotation
    class implements a correct mathematical rotation.  The following
    two sequences of code will perform the same transformation:

    \code
    QTransform t;
    t.rotate(45, Qt::YAxis);

    QGraphicsRotation r;
    r.setAxis(Qt::YAxis);
    r.setAngle(-45);
    \endcode
*/
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

/*!
    \reimp
*/
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
