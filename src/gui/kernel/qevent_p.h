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

#ifndef QEVENT_P_H
#define QEVENT_P_H

#include <qglobal.h>
#include <qurl.h>
#include <qevent.h>

class QTouchEventTouchPointPrivate
{
 public:
   QTouchEventTouchPointPrivate(int id)
      : ref(1), id(id), state(Qt::TouchPointReleased), pressure(qreal(-1.))
   {
   }

   QTouchEventTouchPointPrivate *detach() {
      QTouchEventTouchPointPrivate *d = new QTouchEventTouchPointPrivate(*this);
      d->ref = 1;

      if (! this->ref.deref()) {
         delete this;
      }
      return d;
   }

   QAtomicInt ref;
   int id;
   Qt::TouchPointStates state;

   QRectF rect;
   QRectF sceneRect;
   QRectF screenRect;

   QPointF normalizedPos;
   QPointF startPos;
   QPointF startScenePos;
   QPointF startScreenPos;
   QPointF startNormalizedPos;
   QPointF lastPos;
   QPointF lastScenePos;
   QPointF lastScreenPos;
   QPointF lastNormalizedPos;

   qreal pressure;

   QVector2D velocity;
   QTouchEvent::TouchPoint::InfoFlags flags;
   QVector<QPointF> rawScreenPositions;
};

#ifndef QT_NO_TABLETEVENT
class QTabletEventPrivate
{
 public:
   inline QTabletEventPrivate(Qt::MouseButton button, Qt::MouseButtons buttons)
      : b(button), buttonState(buttons)
   {
   }

   Qt::MouseButton b;
   Qt::MouseButtons buttonState;
};
#endif // QT_NO_TABLETEVENT

#endif
