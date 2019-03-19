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

#include <algorithm>
#include <qgraphicsitemanimation.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qgraphicsitem.h>
#include <QtCore/qtimeline.h>
#include <QtCore/qpoint.h>
#include <QtCore/qpointer.h>
#include <QtCore/qpair.h>
#include <QtGui/qmatrix.h>

QT_BEGIN_NAMESPACE

class QGraphicsItemAnimationPrivate
{
 public:
   inline QGraphicsItemAnimationPrivate()
      : q(0), timeLine(0), item(0), step(0) {
   }

   QGraphicsItemAnimation *q;

   QPointer<QTimeLine> timeLine;
   QGraphicsItem *item;

   QPointF startPos;
   QMatrix startMatrix;

   qreal step;

   struct Pair {
      Pair(qreal a, qreal b) : step(a), value(b) {}
      bool operator <(const Pair &other) const {
         return step < other.step;
      }
      bool operator==(const Pair &other) const {
         return step == other.step;
      }
      qreal step;
      qreal value;
   };
   QList<Pair> xPosition;
   QList<Pair> yPosition;
   QList<Pair> rotation;
   QList<Pair> verticalScale;
   QList<Pair> horizontalScale;
   QList<Pair> verticalShear;
   QList<Pair> horizontalShear;
   QList<Pair> xTranslation;
   QList<Pair> yTranslation;

   qreal linearValueForStep(qreal step, QList<Pair> *source, qreal defaultValue = 0);
   void insertUniquePair(qreal step, qreal value, QList<Pair> *binList, const char *method);
};

qreal QGraphicsItemAnimationPrivate::linearValueForStep(qreal step, QList<Pair> *source, qreal defaultValue)
{
   if (source->isEmpty()) {
      return defaultValue;
   }
   step = qMin(qMax(step, 0), 1);

   if (step == 1) {
      return source->last().value;
   }

   qreal stepBefore = 0;
   qreal stepAfter = 1;
   qreal valueBefore = source->first().step == 0 ? source->first().value : defaultValue;
   qreal valueAfter = source->last().value;

   // Find the closest step and value before the given step.
   for (int i = 0; i < source->size() && step >= source->at(i).step; ++i) {
      stepBefore = source->at(i).step;
      valueBefore = source->at(i).value;
   }

   // Find the closest step and value after the given step.
   for (int j = source->size() - 1; j >= 0 && step < source->at(j).step; --j) {
      stepAfter = source->at(j).step;
      valueAfter = source->at(j).value;
   }

   // Do a simple linear interpolation.
   return valueBefore + (valueAfter - valueBefore) * ((step - stepBefore) / (stepAfter - stepBefore));
}

void QGraphicsItemAnimationPrivate::insertUniquePair(qreal step, qreal value, QList<Pair> *binList, const char *method)
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::%s: invalid step = %f", method, step);
      return;
   }

   Pair pair(step, value);

   QList<Pair>::iterator result = std::lower_bound(binList->begin(), binList->end(), pair);

   if (result != binList->end() || pair < *result) {
      result->value = value;
   } else {
      *binList << pair;
      std::sort(binList->begin(), binList->end());
   }
}

/*!
  Constructs an animation object with the given \a parent.
*/
QGraphicsItemAnimation::QGraphicsItemAnimation(QObject *parent)
   : QObject(parent), d(new QGraphicsItemAnimationPrivate)
{
   d->q = this;
}

/*!
  Destroys the animation object.
*/
QGraphicsItemAnimation::~QGraphicsItemAnimation()
{
   delete d;
}

/*!
  Returns the item on which the animation object operates.

  \sa setItem()
*/
QGraphicsItem *QGraphicsItemAnimation::item() const
{
   return d->item;
}

/*!
  Sets the specified \a item to be used in the animation.

  \sa item()
*/
void QGraphicsItemAnimation::setItem(QGraphicsItem *item)
{
   d->item = item;
   d->startPos = d->item->pos();
}

/*!
  Returns the timeline object used to control the rate at which the animation
  occurs.

  \sa setTimeLine()
*/
QTimeLine *QGraphicsItemAnimation::timeLine() const
{
   return d->timeLine;
}

/*!
  Sets the timeline object used to control the rate of animation to the \a timeLine
  specified.

  \sa timeLine()
*/
void QGraphicsItemAnimation::setTimeLine(QTimeLine *timeLine)
{
   if (d->timeLine == timeLine) {
      return;
   }
   if (d->timeLine) {
      delete d->timeLine;
   }
   if (!timeLine) {
      return;
   }
   d->timeLine = timeLine;
   connect(timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(setStep(qreal)));
}

/*!
  Returns the position of the item at the given \a step value.

  \sa setPosAt()
*/
QPointF QGraphicsItemAnimation::posAt(qreal step) const
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::posAt: invalid step = %f", step);
   }

   return QPointF(d->linearValueForStep(step, &d->xPosition, d->startPos.x()),
                  d->linearValueForStep(step, &d->yPosition, d->startPos.y()));
}

/*!
  \fn void QGraphicsItemAnimation::setPosAt(qreal step, const QPointF &point)

  Sets the position of the item at the given \a step value to the \a point specified.

  \sa posAt()
*/
void QGraphicsItemAnimation::setPosAt(qreal step, const QPointF &pos)
{
   d->insertUniquePair(step, pos.x(), &d->xPosition, "setPosAt");
   d->insertUniquePair(step, pos.y(), &d->yPosition, "setPosAt");
}

/*!
  Returns all explicitly inserted positions.

  \sa posAt(), setPosAt()
*/
QList<QPair<qreal, QPointF> > QGraphicsItemAnimation::posList() const
{
   QList<QPair<qreal, QPointF> > list;
   for (int i = 0; i < d->xPosition.size(); ++i) {
      list << QPair<qreal, QPointF>(d->xPosition.at(i).step, QPointF(d->xPosition.at(i).value, d->yPosition.at(i).value));
   }

   return list;
}

/*!
  Returns the matrix used to transform the item at the specified \a step value.
*/
QMatrix QGraphicsItemAnimation::matrixAt(qreal step) const
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::matrixAt: invalid step = %f", step);
   }

   QMatrix matrix;
   if (!d->rotation.isEmpty()) {
      matrix.rotate(rotationAt(step));
   }
   if (!d->verticalScale.isEmpty()) {
      matrix.scale(horizontalScaleAt(step), verticalScaleAt(step));
   }
   if (!d->verticalShear.isEmpty()) {
      matrix.shear(horizontalShearAt(step), verticalShearAt(step));
   }
   if (!d->xTranslation.isEmpty()) {
      matrix.translate(xTranslationAt(step), yTranslationAt(step));
   }
   return matrix;
}

/*!
  Returns the angle at which the item is rotated at the specified \a step value.

  \sa setRotationAt()
*/
qreal QGraphicsItemAnimation::rotationAt(qreal step) const
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::rotationAt: invalid step = %f", step);
   }

   return d->linearValueForStep(step, &d->rotation);
}

/*!
  Sets the rotation of the item at the given \a step value to the \a angle specified.

  \sa rotationAt()
*/
void QGraphicsItemAnimation::setRotationAt(qreal step, qreal angle)
{
   d->insertUniquePair(step, angle, &d->rotation, "setRotationAt");
}

/*!
  Returns all explicitly inserted rotations.

  \sa rotationAt(), setRotationAt()
*/
QList<QPair<qreal, qreal> > QGraphicsItemAnimation::rotationList() const
{
   QList<QPair<qreal, qreal> > list;
   for (int i = 0; i < d->rotation.size(); ++i) {
      list << QPair<qreal, qreal>(d->rotation.at(i).step, d->rotation.at(i).value);
   }

   return list;
}

/*!
  Returns the horizontal translation of the item at the specified \a step value.

  \sa setTranslationAt()
*/
qreal QGraphicsItemAnimation::xTranslationAt(qreal step) const
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::xTranslationAt: invalid step = %f", step);
   }

   return d->linearValueForStep(step, &d->xTranslation);
}

/*!
  Returns the vertical translation of the item at the specified \a step value.

  \sa setTranslationAt()
*/
qreal QGraphicsItemAnimation::yTranslationAt(qreal step) const
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::yTranslationAt: invalid step = %f", step);
   }

   return d->linearValueForStep(step, &d->yTranslation);
}

/*!
  Sets the translation of the item at the given \a step value using the horizontal
  and vertical coordinates specified by \a dx and \a dy.

  \sa xTranslationAt(), yTranslationAt()
*/
void QGraphicsItemAnimation::setTranslationAt(qreal step, qreal dx, qreal dy)
{
   d->insertUniquePair(step, dx, &d->xTranslation, "setTranslationAt");
   d->insertUniquePair(step, dy, &d->yTranslation, "setTranslationAt");
}

/*!
  Returns all explicitly inserted translations.

  \sa xTranslationAt(), yTranslationAt(), setTranslationAt()
*/
QList<QPair<qreal, QPointF> > QGraphicsItemAnimation::translationList() const
{
   QList<QPair<qreal, QPointF> > list;
   for (int i = 0; i < d->xTranslation.size(); ++i) {
      list << QPair<qreal, QPointF>(d->xTranslation.at(i).step, QPointF(d->xTranslation.at(i).value,
                                    d->yTranslation.at(i).value));
   }

   return list;
}

/*!
  Returns the vertical scale for the item at the specified \a step value.

  \sa setScaleAt()
*/
qreal QGraphicsItemAnimation::verticalScaleAt(qreal step) const
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::verticalScaleAt: invalid step = %f", step);
   }

   return d->linearValueForStep(step, &d->verticalScale, 1);
}

/*!
  Returns the horizontal scale for the item at the specified \a step value.

  \sa setScaleAt()
*/
qreal QGraphicsItemAnimation::horizontalScaleAt(qreal step) const
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::horizontalScaleAt: invalid step = %f", step);
   }

   return d->linearValueForStep(step, &d->horizontalScale, 1);
}

/*!
  Sets the scale of the item at the given \a step value using the horizontal and
  vertical scale factors specified by \a sx and \a sy.

  \sa verticalScaleAt(), horizontalScaleAt()
*/
void QGraphicsItemAnimation::setScaleAt(qreal step, qreal sx, qreal sy)
{
   d->insertUniquePair(step, sx, &d->horizontalScale, "setScaleAt");
   d->insertUniquePair(step, sy, &d->verticalScale, "setScaleAt");
}

/*!
  Returns all explicitly inserted scales.

  \sa verticalScaleAt(), horizontalScaleAt(), setScaleAt()
*/
QList<QPair<qreal, QPointF> > QGraphicsItemAnimation::scaleList() const
{
   QList<QPair<qreal, QPointF> > list;
   for (int i = 0; i < d->horizontalScale.size(); ++i) {
      list << QPair<qreal, QPointF>(d->horizontalScale.at(i).step, QPointF(d->horizontalScale.at(i).value,
                                    d->verticalScale.at(i).value));
   }

   return list;
}

/*!
  Returns the vertical shear for the item at the specified \a step value.

  \sa setShearAt()
*/
qreal QGraphicsItemAnimation::verticalShearAt(qreal step) const
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::verticalShearAt: invalid step = %f", step);
   }

   return d->linearValueForStep(step, &d->verticalShear, 0);
}

/*!
  Returns the horizontal shear for the item at the specified \a step value.

  \sa setShearAt()
*/
qreal QGraphicsItemAnimation::horizontalShearAt(qreal step) const
{
   if (step < 0.0 || step > 1.0) {
      qWarning("QGraphicsItemAnimation::horizontalShearAt: invalid step = %f", step);
   }

   return d->linearValueForStep(step, &d->horizontalShear, 0);
}

/*!
  Sets the shear of the item at the given \a step value using the horizontal and
  vertical shear factors specified by \a sh and \a sv.

  \sa verticalShearAt(), horizontalShearAt()
*/
void QGraphicsItemAnimation::setShearAt(qreal step, qreal sh, qreal sv)
{
   d->insertUniquePair(step, sh, &d->horizontalShear, "setShearAt");
   d->insertUniquePair(step, sv, &d->verticalShear, "setShearAt");
}

/*!
  Returns all explicitly inserted shears.

  \sa verticalShearAt(), horizontalShearAt(), setShearAt()
*/
QList<QPair<qreal, QPointF> > QGraphicsItemAnimation::shearList() const
{
   QList<QPair<qreal, QPointF> > list;
   for (int i = 0; i < d->horizontalShear.size(); ++i) {
      list << QPair<qreal, QPointF>(d->horizontalShear.at(i).step, QPointF(d->horizontalShear.at(i).value,
                                    d->verticalShear.at(i).value));
   }

   return list;
}

/*!
  Clears the scheduled transformations used for the animation, but
  retains the item and timeline.
*/
void QGraphicsItemAnimation::clear()
{
   d->xPosition.clear();
   d->yPosition.clear();
   d->rotation.clear();
   d->verticalScale.clear();
   d->horizontalScale.clear();
   d->verticalShear.clear();
   d->horizontalShear.clear();
   d->xTranslation.clear();
   d->yTranslation.clear();
}

/*!
  \fn void QGraphicsItemAnimation::setStep(qreal step)

  Sets the current \a step value for the animation, causing the
  transformations scheduled at this step to be performed.
*/
void QGraphicsItemAnimation::setStep(qreal x)
{
   if (x < 0.0 || x > 1.0) {
      qWarning("QGraphicsItemAnimation::setStep: invalid step = %f", x);
      return;
   }

   beforeAnimationStep(x);

   d->step = x;
   if (d->item) {
      if (!d->xPosition.isEmpty() || !d->yPosition.isEmpty()) {
         d->item->setPos(posAt(x));
      }
      if (!d->rotation.isEmpty()
            || !d->verticalScale.isEmpty()
            || !d->horizontalScale.isEmpty()
            || !d->verticalShear.isEmpty()
            || !d->horizontalShear.isEmpty()
            || !d->xTranslation.isEmpty()
            || !d->yTranslation.isEmpty()) {
         d->item->setMatrix(d->startMatrix * matrixAt(x));
      }
   }

   afterAnimationStep(x);
}

/*!
    Resets the item to its starting position and transformation.

    \obsolete

    You can call setStep(0) instead.
*/
void QGraphicsItemAnimation::reset()
{
   if (!d->item) {
      return;
   }
   d->startPos = d->item->pos();
   d->startMatrix = d->item->matrix();
}

/*!
  \fn void QGraphicsItemAnimation::beforeAnimationStep(qreal step)

  This method is meant to be overridden by subclassed that needs to
  execute additional code before a new step takes place. The
  animation \a step is provided for use in cases where the action
  depends on its value.
*/
void QGraphicsItemAnimation::beforeAnimationStep(qreal step)
{
   Q_UNUSED(step);
}

/*!
  \fn void QGraphicsItemAnimation::afterAnimationStep(qreal step)

  This method is meant to be overridden in subclasses that need to
  execute additional code after a new step has taken place. The
  animation \a step is provided for use in cases where the action
  depends on its value.
*/
void QGraphicsItemAnimation::afterAnimationStep(qreal step)
{
   Q_UNUSED(step);
}

QT_END_NAMESPACE

#endif // QT_NO_GRAPHICSVIEW
