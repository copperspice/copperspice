/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qwinnativepangesturerecognizer_win_p.h>

#include <qevent.h>
#include <qgraphicsitem.h>
#include <qgesture.h>
#include <qgesture_p.h>
#include <qevent_p.h>
#include <qapplication_p.h>
#include <qwidget_p.h>

#ifndef QT_NO_GESTURES

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_NATIVE_GESTURES)

QWinNativePanGestureRecognizer::QWinNativePanGestureRecognizer()
{
}

QGesture *QWinNativePanGestureRecognizer::create(QObject *target)
{
   if (!target) {
      return new QPanGesture;   // a special case
   }
   if (!target->isWidgetType()) {
      return 0;
   }
   if (qobject_cast<QGraphicsObject *>(target)) {
      return 0;
   }

   QWidget *q = static_cast<QWidget *>(target);
   QWidgetPrivate *d = q->d_func();
   d->nativeGesturePanEnabled = true;
   d->winSetupGestures();

   return new QPanGesture;
}

QGestureRecognizer::Result QWinNativePanGestureRecognizer::recognize(QGesture *state,
      QObject *,
      QEvent *event)
{
   QPanGesture *q = static_cast<QPanGesture *>(state);
   QPanGesturePrivate *d = q->d_func();

   QGestureRecognizer::Result result = QGestureRecognizer::Ignore;
   if (event->type() == QEvent::NativeGesture) {
      QNativeGestureEvent *ev = static_cast<QNativeGestureEvent *>(event);
      switch (ev->gestureType) {
         case QNativeGestureEvent::GestureBegin:
            break;
         case QNativeGestureEvent::Pan:
            result = QGestureRecognizer::TriggerGesture;
            event->accept();
            break;
         case QNativeGestureEvent::GestureEnd:
            if (q->state() == Qt::NoGesture) {
               return QGestureRecognizer::Ignore;   // some other gesture has ended
            }
            result = QGestureRecognizer::FinishGesture;
            break;
         default:
            return QGestureRecognizer::Ignore;
      }
      if (q->state() == Qt::NoGesture) {
         d->lastOffset = d->offset = QPointF();
         d->startPosition = ev->position;
      } else {
         d->lastOffset = d->offset;
         d->offset = QPointF(ev->position.x() - d->startPosition.x(),
                             ev->position.y() - d->startPosition.y());
      }
   }
   return result;
}

void QWinNativePanGestureRecognizer::reset(QGesture *state)
{
   QPanGesture *pan = static_cast<QPanGesture *>(state);
   QPanGesturePrivate *d = pan->d_func();

   d->lastOffset = d->offset = QPointF();
   d->startPosition = QPoint();
   d->acceleration = 0;

   QGestureRecognizer::reset(state);
}

#endif // QT_NO_NATIVE_GESTURES

QT_END_NAMESPACE

#endif // QT_NO_GESTURES
