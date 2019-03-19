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

#include <qmacgesturerecognizer_mac_p.h>
#include <qgesture.h>
#include <qgesture_p.h>
#include <qevent.h>
#include <qevent_p.h>
#include <qwidget.h>
#include <qdebug.h>

#ifndef QT_NO_GESTURES

QT_BEGIN_NAMESPACE

QMacSwipeGestureRecognizer::QMacSwipeGestureRecognizer()
{
}

QGesture *QMacSwipeGestureRecognizer::create(QObject * /*target*/)
{
   return new QSwipeGesture;
}

QGestureRecognizer::Result
QMacSwipeGestureRecognizer::recognize(QGesture *gesture, QObject *obj, QEvent *event)
{
   if (event->type() == QEvent::NativeGesture && obj->isWidgetType()) {
      QNativeGestureEvent *ev = static_cast<QNativeGestureEvent *>(event);
      switch (ev->gestureType) {
         case QNativeGestureEvent::Swipe: {
            QSwipeGesture *g = static_cast<QSwipeGesture *>(gesture);
            g->setSwipeAngle(ev->angle);
            g->setHotSpot(ev->position);
            return QGestureRecognizer::FinishGesture | QGestureRecognizer::ConsumeEventHint;
            break;
         }
         default:
            break;
      }
   }

   return QGestureRecognizer::Ignore;
}

void QMacSwipeGestureRecognizer::reset(QGesture *gesture)
{
   QSwipeGesture *g = static_cast<QSwipeGesture *>(gesture);
   g->setSwipeAngle(0);
   QGestureRecognizer::reset(gesture);
}

////////////////////////////////////////////////////////////////////////

QMacPinchGestureRecognizer::QMacPinchGestureRecognizer()
{
}

QGesture *QMacPinchGestureRecognizer::create(QObject * /*target*/)
{
   return new QPinchGesture;
}

QGestureRecognizer::Result
QMacPinchGestureRecognizer::recognize(QGesture *gesture, QObject *obj, QEvent *event)
{
   if (event->type() == QEvent::NativeGesture && obj->isWidgetType()) {
      QPinchGesture *g = static_cast<QPinchGesture *>(gesture);
      QNativeGestureEvent *ev = static_cast<QNativeGestureEvent *>(event);
      switch (ev->gestureType) {
         case QNativeGestureEvent::GestureBegin:
            reset(gesture);
            g->setStartCenterPoint(static_cast<QWidget *>(obj)->mapFromGlobal(ev->position));
            g->setCenterPoint(g->startCenterPoint());
            g->setChangeFlags(QPinchGesture::CenterPointChanged);
            g->setTotalChangeFlags(g->totalChangeFlags() | g->changeFlags());
            g->setHotSpot(ev->position);
            return QGestureRecognizer::MayBeGesture | QGestureRecognizer::ConsumeEventHint;
         case QNativeGestureEvent::Rotate: {
            g->setLastScaleFactor(g->scaleFactor());
            g->setLastRotationAngle(g->rotationAngle());
            g->setRotationAngle(g->rotationAngle() + ev->percentage);
            g->setChangeFlags(QPinchGesture::RotationAngleChanged);
            g->setTotalChangeFlags(g->totalChangeFlags() | g->changeFlags());
            g->setHotSpot(ev->position);
            return QGestureRecognizer::TriggerGesture | QGestureRecognizer::ConsumeEventHint;
         }
         case QNativeGestureEvent::Zoom:
            g->setLastScaleFactor(g->scaleFactor());
            g->setLastRotationAngle(g->rotationAngle());
            g->setScaleFactor(g->scaleFactor() * (1 + ev->percentage));
            g->setChangeFlags(QPinchGesture::ScaleFactorChanged);
            g->setTotalChangeFlags(g->totalChangeFlags() | g->changeFlags());
            g->setHotSpot(ev->position);
            return QGestureRecognizer::TriggerGesture | QGestureRecognizer::ConsumeEventHint;
         case QNativeGestureEvent::GestureEnd:
            return QGestureRecognizer::FinishGesture | QGestureRecognizer::ConsumeEventHint;
         default:
            break;
      }
   }

   return QGestureRecognizer::Ignore;
}

void QMacPinchGestureRecognizer::reset(QGesture *gesture)
{
   QPinchGesture *g = static_cast<QPinchGesture *>(gesture);
   g->setChangeFlags(0);
   g->setTotalChangeFlags(0);
   g->setScaleFactor(1.0f);
   g->setTotalScaleFactor(1.0f);
   g->setLastScaleFactor(1.0f);
   g->setRotationAngle(0.0f);
   g->setTotalRotationAngle(0.0f);
   g->setLastRotationAngle(0.0f);
   g->setCenterPoint(QPointF());
   g->setStartCenterPoint(QPointF());
   g->setLastCenterPoint(QPointF());
   QGestureRecognizer::reset(gesture);
}

QMacPanGestureRecognizer::QMacPanGestureRecognizer() : _panCanceled(true)
{
}

QGesture *QMacPanGestureRecognizer::create(QObject *target)
{
   if (!target) {
      return new QPanGesture;
   }

   if (QWidget *w = qobject_cast<QWidget *>(target)) {
      w->setAttribute(Qt::WA_AcceptTouchEvents);
      w->setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents);
      return new QPanGesture;
   }
   return 0;
}

QGestureRecognizer::Result
QMacPanGestureRecognizer::recognize(QGesture *gesture, QObject *target, QEvent *event)
{
   const int panBeginDelay = 300;
   const int panBeginRadius = 3;

   QPanGesture *g = static_cast<QPanGesture *>(gesture);

   switch (event->type()) {
      case QEvent::TouchBegin: {
         const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
         if (ev->touchPoints().size() == 1) {
            reset(gesture);
            _startPos = QCursor::pos();
            _panTimer.start(panBeginDelay, target);
            _panCanceled = false;
            return QGestureRecognizer::MayBeGesture;
         }
         break;
      }
      case QEvent::TouchEnd: {
         if (_panCanceled) {
            break;
         }

         const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
         if (ev->touchPoints().size() == 1) {
            return QGestureRecognizer::FinishGesture;
         }
         break;
      }
      case QEvent::TouchUpdate: {
         if (_panCanceled) {
            break;
         }

         const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
         if (ev->touchPoints().size() == 1) {
            if (_panTimer.isActive()) {
               // INVARIANT: Still in maybeGesture. Check if the user
               // moved his finger so much that it makes sense to cancel the pan:
               const QPointF p = QCursor::pos();
               if ((p - _startPos).manhattanLength() > panBeginRadius) {
                  _panCanceled = true;
                  _panTimer.stop();
                  return QGestureRecognizer::CancelGesture;
               }
            } else {
               const QPointF p = QCursor::pos();
               const QPointF posOffset = p - _startPos;
               g->setLastOffset(g->offset());
               g->setOffset(QPointF(posOffset.x(), posOffset.y()));
               g->setHotSpot(_startPos);
               return QGestureRecognizer::TriggerGesture;
            }
         } else if (_panTimer.isActive()) {
            // I only want to cancel the pan if the user is pressing
            // more than one finger, and the pan hasn't started yet:
            _panCanceled = true;
            _panTimer.stop();
            return QGestureRecognizer::CancelGesture;
         }
         break;
      }
      case QEvent::Timer: {
         QTimerEvent *ev = static_cast<QTimerEvent *>(event);
         if (ev->timerId() == _panTimer.timerId()) {
            _panTimer.stop();
            if (_panCanceled) {
               break;
            }
            // Begin new pan session!
            _startPos = QCursor::pos();
            g->setHotSpot(_startPos);
            return QGestureRecognizer::TriggerGesture | QGestureRecognizer::ConsumeEventHint;
         }
         break;
      }
      default:
         break;
   }

   return QGestureRecognizer::Ignore;
}

void QMacPanGestureRecognizer::reset(QGesture *gesture)
{
   QPanGesture *g = static_cast<QPanGesture *>(gesture);
   _startPos = QPointF();
   _panCanceled = true;
   g->setOffset(QPointF(0, 0));
   g->setLastOffset(QPointF(0, 0));
   g->setAcceleration(qreal(1));
   QGestureRecognizer::reset(gesture);
}

QT_END_NAMESPACE

#endif // QT_NO_GESTURES
