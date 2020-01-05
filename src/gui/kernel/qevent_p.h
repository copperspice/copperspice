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

#ifndef QEVENT_P_H
#define QEVENT_P_H

#include <qglobal.h>
#include <qurl.h>
#include <qevent.h>



class QTouchEventTouchPointPrivate
{
 public:
   inline QTouchEventTouchPointPrivate(int id)
      : ref(1),
        id(id),
        state(Qt::TouchPointReleased),
        pressure(qreal(-1.)) {
   }

   inline QTouchEventTouchPointPrivate *detach() {
      QTouchEventTouchPointPrivate *d = new QTouchEventTouchPointPrivate(*this);
      d->ref = 1;
      if (!this->ref.deref()) {
         delete this;
      }
      return d;
   }

   QAtomicInt ref;
   int id;
   Qt::TouchPointStates state;
   QRectF rect, sceneRect, screenRect;
   QPointF normalizedPos,
           startPos, startScenePos, startScreenPos, startNormalizedPos,
           lastPos, lastScenePos, lastScreenPos, lastNormalizedPos;
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
      : b(button),
        buttonState(buttons)
   { }

   Qt::MouseButton b;
   Qt::MouseButtons buttonState;
};
#endif // QT_NO_TABLETEVENT


#endif
