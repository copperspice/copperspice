/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qstandardgestures_p.h>
#include <qgesture.h>
#include <qgesture_p.h>
#include <qevent.h>
#include <qwidget.h>
#include <qabstractscrollarea.h>
#include <qgraphicssceneevent.h>
#include <qdebug.h>

#ifndef QT_NO_GESTURES

// If the change in scale for a single touch event is out of this range,
// we consider it to be spurious.
static const qreal kSingleStepScaleMax = 2.0;
static const qreal kSingleStepScaleMin = 0.1;

QGesture *QPanGestureRecognizer::create(QObject *target)
{
   if (target && target->isWidgetType()) {

#if (defined(Q_OS_DARWIN) || defined(Q_OS_WIN)) && ! defined(QT_NO_NATIVE_GESTURES)
      // for scroll areas on Windows we want to use native gestures instead
      if (!qobject_cast<QAbstractScrollArea *>(target->parent())) {
         static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
      }
#else
      static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
#endif

   }
   return new QPanGesture;
}

static QPointF panOffset(const QList<QTouchEvent::TouchPoint> &touchPoints, int maxCount)
{
   QPointF result;
   const int count = qMin(touchPoints.size(), maxCount);
   for (int p = 0; p < count; ++p) {
      result += touchPoints.at(p).pos() - touchPoints.at(p).startPos();
   }
   return result / qreal(count);
}
QGestureRecognizer::Result QPanGestureRecognizer::recognize(QGesture *state,
   QObject *,
   QEvent *event)
{
   QPanGesture *q = static_cast<QPanGesture *>(state);
   QPanGesturePrivate *d = q->d_func();

   QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

   switch (event->type()) {
      case QEvent::TouchBegin: {
         const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
         result = QGestureRecognizer::MayBeGesture;
         QTouchEvent::TouchPoint p = ev->touchPoints().at(0);
         d->lastOffset = d->offset = QPointF();
         d->pointCount = m_pointCount;
         break;
      }

      case QEvent::TouchEnd: {
         if (q->state() != Qt::NoGesture) {
            const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
            if (ev->touchPoints().size() == d->pointCount) {
               d->lastOffset = d->offset;
               d->offset = panOffset(ev->touchPoints(), d->pointCount);
            }
            result = QGestureRecognizer::FinishGesture;
         } else {
            result = QGestureRecognizer::CancelGesture;
         }
         break;
      }

      case QEvent::TouchUpdate: {
         const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
         if (ev->touchPoints().size() >= d->pointCount) {
            d->lastOffset = d->offset;
            d->offset = panOffset(ev->touchPoints(), d->pointCount);
            if (d->offset.x() > 10  || d->offset.y() > 10 ||
               d->offset.x() < -10 || d->offset.y() < -10) {
               q->setHotSpot(ev->touchPoints().first().startScreenPos());
               result = QGestureRecognizer::TriggerGesture;
            } else {
               result = QGestureRecognizer::MayBeGesture;
            }
         }
         break;
      }

      default:
         break;
   }
   return result;
}

void QPanGestureRecognizer::reset(QGesture *state)
{
   QPanGesture *pan = static_cast<QPanGesture *>(state);
   QPanGesturePrivate *d = pan->d_func();

   d->lastOffset = d->offset = QPointF();
   d->acceleration = 0;

   QGestureRecognizer::reset(state);
}

QPinchGestureRecognizer::QPinchGestureRecognizer()
{
}

QGesture *QPinchGestureRecognizer::create(QObject *target)
{
   if (target && target->isWidgetType()) {
      static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
   }
   return new QPinchGesture;
}

QGestureRecognizer::Result QPinchGestureRecognizer::recognize(QGesture *state,
   QObject *, QEvent *event)
{
   QPinchGesture *q = static_cast<QPinchGesture *>(state);
   QPinchGesturePrivate *d = q->d_func();

   QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

   switch (event->type()) {
      case QEvent::TouchBegin: {
         result = QGestureRecognizer::MayBeGesture;
         break;
      }
      case QEvent::TouchEnd: {
         if (q->state() != Qt::NoGesture) {
            result = QGestureRecognizer::FinishGesture;
         } else {
            result = QGestureRecognizer::CancelGesture;
         }
         break;
      }
      case QEvent::TouchUpdate: {
         const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
         d->changeFlags = 0;
         if (ev->touchPoints().size() == 2) {
            QTouchEvent::TouchPoint p1 = ev->touchPoints().at(0);
            QTouchEvent::TouchPoint p2 = ev->touchPoints().at(1);

            d->hotSpot = p1.screenPos();
            d->isHotSpotSet = true;

            QPointF centerPoint = (p1.screenPos() + p2.screenPos()) / 2.0;
            if (d->isNewSequence) {
               d->startPosition[0] = p1.screenPos();
               d->startPosition[1] = p2.screenPos();
               d->lastCenterPoint = centerPoint;
            } else {
               d->lastCenterPoint = d->centerPoint;
            }
            d->centerPoint = centerPoint;

            d->changeFlags |= QPinchGesture::CenterPointChanged;

            if (d->isNewSequence) {
               d->scaleFactor = 1.0;
               d->lastScaleFactor = 1.0;
            } else {
               d->lastScaleFactor = d->scaleFactor;
               QLineF line(p1.screenPos(), p2.screenPos());
               QLineF lastLine(p1.lastScreenPos(),  p2.lastScreenPos());

               qreal newScaleFactor = line.length() / lastLine.length();
               if (newScaleFactor > kSingleStepScaleMax || newScaleFactor < kSingleStepScaleMin) {
                  return QGestureRecognizer::Ignore;
               }
               d->scaleFactor = newScaleFactor;
            }

            d->totalScaleFactor = d->totalScaleFactor * d->scaleFactor;
            d->changeFlags |= QPinchGesture::ScaleFactorChanged;

            qreal angle = QLineF(p1.screenPos(), p2.screenPos()).angle();
            if (angle > 180) {
               angle -= 360;
            }
            qreal startAngle = QLineF(p1.startScreenPos(), p2.startScreenPos()).angle();
            if (startAngle > 180) {
               startAngle -= 360;
            }

            const qreal rotationAngle = startAngle - angle;

            if (d->isNewSequence) {
               d->lastRotationAngle = 0.0;
            } else {
               d->lastRotationAngle = d->rotationAngle;
            }

            d->rotationAngle = rotationAngle;
            d->totalRotationAngle += d->rotationAngle - d->lastRotationAngle;
            d->changeFlags |= QPinchGesture::RotationAngleChanged;

            d->totalChangeFlags |= d->changeFlags;
            d->isNewSequence = false;
            result = QGestureRecognizer::TriggerGesture;
         } else {
            d->isNewSequence = true;
            if (q->state() == Qt::NoGesture) {
               result = QGestureRecognizer::Ignore;
            } else {
               result = QGestureRecognizer::FinishGesture;
            }
         }
         break;
      }

      default:
         break;
   }
   return result;
}

void QPinchGestureRecognizer::reset(QGesture *state)
{
   QPinchGesture *pinch = static_cast<QPinchGesture *>(state);
   QPinchGesturePrivate *d = pinch->d_func();

   d->totalChangeFlags = d->changeFlags = 0;

   d->startCenterPoint = d->lastCenterPoint = d->centerPoint = QPointF();
   d->totalScaleFactor = d->lastScaleFactor = d->scaleFactor = 1;
   d->totalRotationAngle = d->lastRotationAngle = d->rotationAngle = 0;

   d->isNewSequence = true;
   d->startPosition[0] = d->startPosition[1] = QPointF();

   QGestureRecognizer::reset(state);
}


QSwipeGestureRecognizer::QSwipeGestureRecognizer()
{
}

QGesture *QSwipeGestureRecognizer::create(QObject *target)
{
   if (target && target->isWidgetType()) {
      static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
   }
   return new QSwipeGesture;
}

QGestureRecognizer::Result QSwipeGestureRecognizer::recognize(QGesture *state,
   QObject *,
   QEvent *event)
{
   QSwipeGesture *q = static_cast<QSwipeGesture *>(state);
   QSwipeGesturePrivate *d = q->d_func();

   QGestureRecognizer::Result result = QGestureRecognizer::Ignore;

   switch (event->type()) {
      case QEvent::TouchBegin: {
         d->velocityValue = 1;
         d->time.start();
         d->state = QSwipeGesturePrivate::Started;
         result = QGestureRecognizer::MayBeGesture;
         break;
      }
      case QEvent::TouchEnd: {
         if (q->state() != Qt::NoGesture) {
            result = QGestureRecognizer::FinishGesture;
         } else {
            result = QGestureRecognizer::CancelGesture;
         }
         break;
      }
      case QEvent::TouchUpdate: {
         const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);

         if (d->state == QSwipeGesturePrivate::NoGesture) {
            result = QGestureRecognizer::CancelGesture;
         } else if (ev->touchPoints().size() == 3) {
            d->state = QSwipeGesturePrivate::ThreePointsReached;
            QTouchEvent::TouchPoint p1 = ev->touchPoints().at(0);
            QTouchEvent::TouchPoint p2 = ev->touchPoints().at(1);
            QTouchEvent::TouchPoint p3 = ev->touchPoints().at(2);

            if (d->lastPositions[0].isNull()) {
               d->lastPositions[0] = p1.startScreenPos().toPoint();
               d->lastPositions[1] = p2.startScreenPos().toPoint();
               d->lastPositions[2] = p3.startScreenPos().toPoint();
            }
            d->hotSpot = p1.screenPos();
            d->isHotSpotSet = true;

            int xDistance = (p1.screenPos().x() - d->lastPositions[0].x() +
                  p2.screenPos().x() - d->lastPositions[1].x() +
                  p3.screenPos().x() - d->lastPositions[2].x()) / 3;
            int yDistance = (p1.screenPos().y() - d->lastPositions[0].y() +
                  p2.screenPos().y() - d->lastPositions[1].y() +
                  p3.screenPos().y() - d->lastPositions[2].y()) / 3;

            const int distance = xDistance >= yDistance ? xDistance : yDistance;
            int elapsedTime = d->time.restart();
            if (!elapsedTime) {
               elapsedTime = 1;
            }
            d->velocityValue = 0.9 * d->velocityValue + distance / elapsedTime;
            d->swipeAngle = QLineF(p1.startScreenPos(), p1.screenPos()).angle();

            static const int MoveThreshold = 50;
            static const int directionChangeThreshold = MoveThreshold / 8;
            if (qAbs(xDistance) > MoveThreshold || qAbs(yDistance) > MoveThreshold) {
               // measure the distance to check if the direction changed
               d->lastPositions[0] = p1.screenPos().toPoint();
               d->lastPositions[1] = p2.screenPos().toPoint();
               d->lastPositions[2] = p3.screenPos().toPoint();
               result = QGestureRecognizer::TriggerGesture;
               // QTBUG-46195, small changes in direction should not cause the gesture to be canceled.
               if (d->verticalDirection == QSwipeGesture::NoDirection || qAbs(yDistance) > directionChangeThreshold) {
                  const QSwipeGesture::SwipeDirection vertical = yDistance > 0
                     ? QSwipeGesture::Down : QSwipeGesture::Up;
                  if (d->verticalDirection != QSwipeGesture::NoDirection && d->verticalDirection != vertical) {
                     result = QGestureRecognizer::CancelGesture;
                  }
                  d->verticalDirection = vertical;
               }
               if (d->horizontalDirection == QSwipeGesture::NoDirection || qAbs(xDistance) > directionChangeThreshold) {
                  const QSwipeGesture::SwipeDirection horizontal = xDistance > 0
                     ? QSwipeGesture::Right : QSwipeGesture::Left;
                  if (d->horizontalDirection != QSwipeGesture::NoDirection && d->horizontalDirection != horizontal) {
                     result = QGestureRecognizer::CancelGesture;
                  }
                  d->horizontalDirection = horizontal;
               }

            } else {
               if (q->state() != Qt::NoGesture) {
                  result = QGestureRecognizer::TriggerGesture;
               } else {
                  result = QGestureRecognizer::MayBeGesture;
               }
            }

         } else if (ev->touchPoints().size() > 3) {
            result = QGestureRecognizer::CancelGesture;

         } else { // less than 3 touch points
            switch (d->state) {
               case QSwipeGesturePrivate::NoGesture:
                  result = QGestureRecognizer::MayBeGesture;
                  break;
               case QSwipeGesturePrivate::Started:
                  result = QGestureRecognizer::Ignore;
                  break;
               case QSwipeGesturePrivate::ThreePointsReached:
                  result = (ev->touchPointStates() & Qt::TouchPointPressed)
                     ? QGestureRecognizer::CancelGesture : QGestureRecognizer::Ignore;
                  break;
            }
         }
         break;
      }

      default:

         break;
   }
   return result;
}

void QSwipeGestureRecognizer::reset(QGesture *state)
{
   QSwipeGesture *q = static_cast<QSwipeGesture *>(state);
   QSwipeGesturePrivate *d = q->d_func();

   d->verticalDirection = d->horizontalDirection = QSwipeGesture::NoDirection;
   d->swipeAngle = 0;

   d->lastPositions[0] = d->lastPositions[1] = d->lastPositions[2] = QPoint();
   d->state = QSwipeGesturePrivate::NoGesture;
   d->velocityValue = 0;
   d->time.invalidate();

   QGestureRecognizer::reset(state);
}

QTapGestureRecognizer::QTapGestureRecognizer()
{
}

QGesture *QTapGestureRecognizer::create(QObject *target)
{
   if (target && target->isWidgetType()) {
      static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
   }
   return new QTapGesture;
}

QGestureRecognizer::Result QTapGestureRecognizer::recognize(QGesture *state,
   QObject *,
   QEvent *event)
{
   QTapGesture *q = static_cast<QTapGesture *>(state);
   QTapGesturePrivate *d = q->d_func();

   const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);

   QGestureRecognizer::Result result = QGestureRecognizer::CancelGesture;

   switch (event->type()) {
      case QEvent::TouchBegin: {
         d->position = ev->touchPoints().at(0).pos();
         q->setHotSpot(ev->touchPoints().at(0).screenPos());
         result = QGestureRecognizer::TriggerGesture;
         break;
      }
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd: {
         if (q->state() != Qt::NoGesture && ev->touchPoints().size() == 1) {
            QTouchEvent::TouchPoint p = ev->touchPoints().at(0);
            QPoint delta = p.pos().toPoint() - p.startPos().toPoint();
            enum { TapRadius = 40 };
            if (delta.manhattanLength() <= TapRadius) {
               if (event->type() == QEvent::TouchEnd) {
                  result = QGestureRecognizer::FinishGesture;
               } else {
                  result = QGestureRecognizer::TriggerGesture;
               }
            }
         }
         break;
      }
      case QEvent::MouseButtonPress:
      case QEvent::MouseMove:
      case QEvent::MouseButtonRelease:
         result = QGestureRecognizer::Ignore;
         break;
      default:
         result = QGestureRecognizer::Ignore;
         break;
   }
   return result;
}

void QTapGestureRecognizer::reset(QGesture *state)
{
   QTapGesture *q = static_cast<QTapGesture *>(state);
   QTapGesturePrivate *d = q->d_func();

   d->position = QPointF();

   QGestureRecognizer::reset(state);
}

QTapAndHoldGestureRecognizer::QTapAndHoldGestureRecognizer()
{
}

QGesture *QTapAndHoldGestureRecognizer::create(QObject *target)
{
   if (target && target->isWidgetType()) {
      static_cast<QWidget *>(target)->setAttribute(Qt::WA_AcceptTouchEvents);
   }
   return new QTapAndHoldGesture;
}

QGestureRecognizer::Result QTapAndHoldGestureRecognizer::recognize(QGesture *state, QObject *object,
   QEvent *event)
{
   QTapAndHoldGesture *q = static_cast<QTapAndHoldGesture *>(state);
   QTapAndHoldGesturePrivate *d = q->d_func();

   if (object == state && event->type() == QEvent::Timer) {
      q->killTimer(d->timerId);
      d->timerId = 0;
      return QGestureRecognizer::FinishGesture | QGestureRecognizer::ConsumeEventHint;
   }

   enum { TapRadius = 40 };

   switch (event->type()) {
#ifndef QT_NO_GRAPHICSVIEW
      case QEvent::GraphicsSceneMousePress: {
         const QGraphicsSceneMouseEvent *gsme = static_cast<const QGraphicsSceneMouseEvent *>(event);
         d->position = gsme->screenPos();
         q->setHotSpot(d->position);
         if (d->timerId) {
            q->killTimer(d->timerId);
         }
         d->timerId = q->startTimer(QTapAndHoldGesturePrivate::Timeout);
         return QGestureRecognizer::MayBeGesture; // we don't show a sign of life until the timeout
      }
#endif
      case QEvent::MouseButtonPress: {
         const QMouseEvent *me = static_cast<const QMouseEvent *>(event);
         d->position = me->globalPos();
         q->setHotSpot(d->position);
         if (d->timerId) {
            q->killTimer(d->timerId);
         }
         d->timerId = q->startTimer(QTapAndHoldGesturePrivate::Timeout);
         return QGestureRecognizer::MayBeGesture; // we don't show a sign of life until the timeout
      }
      case QEvent::TouchBegin: {
         const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
         d->position = ev->touchPoints().at(0).startScreenPos();
         q->setHotSpot(d->position);
         if (d->timerId) {
            q->killTimer(d->timerId);
         }
         d->timerId = q->startTimer(QTapAndHoldGesturePrivate::Timeout);
         return QGestureRecognizer::MayBeGesture; // we don't show a sign of life until the timeout
      }
#ifndef QT_NO_GRAPHICSVIEW
      case QEvent::GraphicsSceneMouseRelease:
#endif
      case QEvent::MouseButtonRelease:
      case QEvent::TouchEnd:
         return QGestureRecognizer::CancelGesture; // get out of the MayBeGesture state
      case QEvent::TouchUpdate: {
         const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
         if (d->timerId && ev->touchPoints().size() == 1) {
            QTouchEvent::TouchPoint p = ev->touchPoints().at(0);
            QPoint delta = p.pos().toPoint() - p.startPos().toPoint();
            if (delta.manhattanLength() <= TapRadius) {
               return QGestureRecognizer::MayBeGesture;
            }
         }
         return QGestureRecognizer::CancelGesture;
      }
      case QEvent::MouseMove: {
         const QMouseEvent *me = static_cast<const QMouseEvent *>(event);
         QPoint delta = me->globalPos() - d->position.toPoint();
         if (d->timerId && delta.manhattanLength() <= TapRadius) {
            return QGestureRecognizer::MayBeGesture;
         }
         return QGestureRecognizer::CancelGesture;
      }
#ifndef QT_NO_GRAPHICSVIEW
      case QEvent::GraphicsSceneMouseMove: {
         const QGraphicsSceneMouseEvent *gsme = static_cast<const QGraphicsSceneMouseEvent *>(event);
         QPoint delta = gsme->screenPos() - d->position.toPoint();
         if (d->timerId && delta.manhattanLength() <= TapRadius) {
            return QGestureRecognizer::MayBeGesture;
         }
         return QGestureRecognizer::CancelGesture;
      }
#endif
      default:
         return QGestureRecognizer::Ignore;
   }
}

void QTapAndHoldGestureRecognizer::reset(QGesture *state)
{
   QTapAndHoldGesture *q = static_cast<QTapAndHoldGesture *>(state);
   QTapAndHoldGesturePrivate *d = q->d_func();

   d->position = QPointF();
   if (d->timerId) {
      q->killTimer(d->timerId);
   }
   d->timerId = 0;

   QGestureRecognizer::reset(state);
}


#endif // QT_NO_GESTURES
