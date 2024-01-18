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

#include <qgraphicsitemanimation.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qgraphicsitem.h>
#include <qtimeline.h>
#include <qpoint.h>
#include <qpointer.h>
#include <qpair.h>
#include <qmatrix.h>

#include <algorithm>

static inline bool check_step_valid(qreal step, const char *method)
{
   if (!(step >= 0 && step <= 1)) {
      qWarning("QGraphicsItemAnimation::%s Invalid step value = %f", method, step);
      return false;
   }
   return true;
}

class QGraphicsItemAnimationPrivate
{
 public:
   inline QGraphicsItemAnimationPrivate()
      : q(nullptr), timeLine(nullptr), item(nullptr), step(0) {
   }

   QGraphicsItemAnimation *q;

   QPointer<QTimeLine> timeLine;
   QGraphicsItem *item;

   QPointF startPos;
   QMatrix startMatrix;

   qreal step;

   struct Pair {
      bool operator <(const Pair &other) const {
         return step < other.step;
      }
      bool operator==(const Pair &other) const {
         return step == other.step;
      }
      qreal step;
      qreal value;
   };

   QVector<Pair> xPosition;
   QVector<Pair> yPosition;
   QVector<Pair> rotation;
   QVector<Pair> verticalScale;
   QVector<Pair> horizontalScale;
   QVector<Pair> verticalShear;
   QVector<Pair> horizontalShear;
   QVector<Pair> xTranslation;
   QVector<Pair> yTranslation;

   qreal linearValueForStep(qreal step, QVector<Pair> *source, qreal defaultValue = 0);
   void insertUniquePair(qreal step, qreal value, QVector<Pair> *binList, const char *method);
};

qreal QGraphicsItemAnimationPrivate::linearValueForStep(qreal step, QVector<Pair> *source, qreal defaultValue)
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

void QGraphicsItemAnimationPrivate::insertUniquePair(qreal step, qreal value, QVector<Pair> *binList, const char *method)
{
   if (! check_step_valid(step, method)) {
      return;
   }

   const Pair pair = { step, value };

   const QVector<Pair>::iterator result = std::lower_bound(binList->begin(), binList->end(), pair);
   if (result == binList->end() || pair < *result) {
      binList->insert(result, pair);
   } else {
      result->value = value;
   }

}

QGraphicsItemAnimation::QGraphicsItemAnimation(QObject *parent)
   : QObject(parent), d(new QGraphicsItemAnimationPrivate)
{
   d->q = this;
}

QGraphicsItemAnimation::~QGraphicsItemAnimation()
{
   delete d;
}

QGraphicsItem *QGraphicsItemAnimation::item() const
{
   return d->item;
}

void QGraphicsItemAnimation::setItem(QGraphicsItem *item)
{
   d->item = item;
   d->startPos = d->item->pos();
}

QTimeLine *QGraphicsItemAnimation::timeLine() const
{
   return d->timeLine;
}

void QGraphicsItemAnimation::setTimeLine(QTimeLine *timeLine)
{
   if (d->timeLine == timeLine) {
      return;
   }
   if (d->timeLine) {
      delete d->timeLine;
   }
   if (! timeLine) {
      return;
   }
   d->timeLine = timeLine;
   connect(timeLine, &QTimeLine::valueChanged, this, &QGraphicsItemAnimation::setStep);
}

QPointF QGraphicsItemAnimation::posAt(qreal step) const
{

   check_step_valid(step, "posAt");
   return QPointF(d->linearValueForStep(step, &d->xPosition, d->startPos.x()),
         d->linearValueForStep(step, &d->yPosition, d->startPos.y()));
}

void QGraphicsItemAnimation::setPosAt(qreal step, const QPointF &pos)
{
   d->insertUniquePair(step, pos.x(), &d->xPosition, "setPosAt");
   d->insertUniquePair(step, pos.y(), &d->yPosition, "setPosAt");
}

QList<QPair<qreal, QPointF>> QGraphicsItemAnimation::posList() const
{
   QList<QPair<qreal, QPointF>> list;
   const int xPosCount = d->xPosition.size();

   for (int i = 0; i < xPosCount; ++i) {
      list << QPair<qreal, QPointF>(d->xPosition.at(i).step, QPointF(d->xPosition.at(i).value, d->yPosition.at(i).value));
   }

   return list;
}

QMatrix QGraphicsItemAnimation::matrixAt(qreal step) const
{
   check_step_valid(step, "matrixAt");

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

qreal QGraphicsItemAnimation::rotationAt(qreal step) const
{

   check_step_valid(step, "rotationAt");
   return d->linearValueForStep(step, &d->rotation);
}

void QGraphicsItemAnimation::setRotationAt(qreal step, qreal angle)
{
   d->insertUniquePair(step, angle, &d->rotation, "setRotationAt");
}

QList<QPair<qreal, qreal>> QGraphicsItemAnimation::rotationList() const
{
   QList<QPair<qreal, qreal>> list;
   const int numRotations = d->rotation.size();
   for (int i = 0; i < numRotations; ++i) {
      list << QPair<qreal, qreal>(d->rotation.at(i).step, d->rotation.at(i).value);
   }

   return list;
}

qreal QGraphicsItemAnimation::xTranslationAt(qreal step) const
{
   check_step_valid(step, "xTranslationAt");
   return d->linearValueForStep(step, &d->xTranslation);
}

qreal QGraphicsItemAnimation::yTranslationAt(qreal step) const
{
   check_step_valid(step, "yTranslationAt");
   return d->linearValueForStep(step, &d->yTranslation);
}

void QGraphicsItemAnimation::setTranslationAt(qreal step, qreal dx, qreal dy)
{
   d->insertUniquePair(step, dx, &d->xTranslation, "setTranslationAt");
   d->insertUniquePair(step, dy, &d->yTranslation, "setTranslationAt");
}

QList<QPair<qreal, QPointF>> QGraphicsItemAnimation::translationList() const
{
   QList<QPair<qreal, QPointF>> list;
   const int numTranslations = d->xTranslation.size();
   for (int i = 0; i < numTranslations; ++i) {
      list << QPair<qreal, QPointF>(d->xTranslation.at(i).step, QPointF(d->xTranslation.at(i).value,
               d->yTranslation.at(i).value));
   }

   return list;
}

qreal QGraphicsItemAnimation::verticalScaleAt(qreal step) const
{
   check_step_valid(step, "verticalScaleAt");

   return d->linearValueForStep(step, &d->verticalScale, 1);
}

qreal QGraphicsItemAnimation::horizontalScaleAt(qreal step) const
{
   check_step_valid(step, "horizontalScaleAt");
   return d->linearValueForStep(step, &d->horizontalScale, 1);
}

void QGraphicsItemAnimation::setScaleAt(qreal step, qreal sx, qreal sy)
{
   d->insertUniquePair(step, sx, &d->horizontalScale, "setScaleAt");
   d->insertUniquePair(step, sy, &d->verticalScale, "setScaleAt");
}

QList<QPair<qreal, QPointF>> QGraphicsItemAnimation::scaleList() const
{
   QList<QPair<qreal, QPointF>> list;
   const int numScales = d->horizontalScale.size();
   for (int i = 0; i < numScales; ++i) {
      list << QPair<qreal, QPointF>(d->horizontalScale.at(i).step, QPointF(d->horizontalScale.at(i).value,
               d->verticalScale.at(i).value));
   }

   return list;
}

qreal QGraphicsItemAnimation::verticalShearAt(qreal step) const
{
   check_step_valid(step, "verticalShearAt");
   return d->linearValueForStep(step, &d->verticalShear, 0);
}

qreal QGraphicsItemAnimation::horizontalShearAt(qreal step) const
{
   check_step_valid(step, "horizontalShearAt");
   return d->linearValueForStep(step, &d->horizontalShear, 0);
}

void QGraphicsItemAnimation::setShearAt(qreal step, qreal sh, qreal sv)
{
   d->insertUniquePair(step, sh, &d->horizontalShear, "setShearAt");
   d->insertUniquePair(step, sv, &d->verticalShear, "setShearAt");
}

QList<QPair<qreal, QPointF>> QGraphicsItemAnimation::shearList() const
{
   QList<QPair<qreal, QPointF>> list;
   const int numShears = d->horizontalShear.size();

   for (int i = 0; i < numShears; ++i) {
      list << QPair<qreal, QPointF>(d->horizontalShear.at(i).step, QPointF(d->horizontalShear.at(i).value,
               d->verticalShear.at(i).value));
   }

   return list;
}

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

void QGraphicsItemAnimation::setStep(qreal step)
{
   if (!check_step_valid(step, "setStep"))  {
      return;
   }

   beforeAnimationStep(step);

   d->step = step;
   if (d->item) {
      if (!d->xPosition.isEmpty() || !d->yPosition.isEmpty()) {
         d->item->setPos(posAt(step));
      }

      if (!d->rotation.isEmpty()
         || !d->verticalScale.isEmpty()
         || !d->horizontalScale.isEmpty()
         || !d->verticalShear.isEmpty()
         || !d->horizontalShear.isEmpty()
         || !d->xTranslation.isEmpty()
         || !d->yTranslation.isEmpty()) {
         d->item->setMatrix(d->startMatrix * matrixAt(step));
      }
   }

   afterAnimationStep(step);
}

void QGraphicsItemAnimation::reset()
{
   if (! d->item) {
      return;
   }

   d->startPos = d->item->pos();
   d->startMatrix = d->item->matrix();
}

void QGraphicsItemAnimation::beforeAnimationStep(qreal step)
{
   (void) step;
}

void QGraphicsItemAnimation::afterAnimationStep(qreal step)
{
   (void) step;
}

#endif // QT_NO_GRAPHICSVIEW
