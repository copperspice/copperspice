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

#ifndef QDECLARATIVEPINCHAREA_P_P_H
#define QDECLARATIVEPINCHAREA_P_P_H

#include <qdatetime.h>
#include <qbasictimer.h>
#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qdeclarativeitem_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativePinch;
class QDeclarativePinchAreaPrivate : public QDeclarativeItemPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativePinchArea)
 public:
   QDeclarativePinchAreaPrivate()
      : absorb(true), stealMouse(false), inPinch(false)
      , pinchRejected(false), pinchActivated(false), touchEventsActive(false)
      , pinch(0), pinchStartDist(0), pinchStartScale(1.0)
      , pinchLastScale(1.0), pinchStartRotation(0.0), pinchStartAngle(0.0)
      , pinchLastAngle(0.0), pinchRotation(0.0) {
   }

   ~QDeclarativePinchAreaPrivate();

   void init() {
      Q_Q(QDeclarativePinchArea);
      q->setAcceptedMouseButtons(Qt::LeftButton);
      q->setAcceptTouchEvents(true);
      q->setFiltersChildEvents(true);
   }

   bool absorb : 1;
   bool stealMouse : 1;
   bool inPinch : 1;
   bool pinchRejected : 1;
   bool pinchActivated : 1;
   bool touchEventsActive : 1;
   QDeclarativePinch *pinch;
   QPointF sceneStartPoint1;
   QPointF sceneStartPoint2;
   QPointF lastPoint1;
   QPointF lastPoint2;
   qreal pinchStartDist;
   qreal pinchStartScale;
   qreal pinchLastScale;
   qreal pinchStartRotation;
   qreal pinchStartAngle;
   qreal pinchLastAngle;
   qreal pinchRotation;
   QPointF sceneStartCenter;
   QPointF pinchStartCenter;
   QPointF sceneLastCenter;
   QPointF pinchStartPos;
   QList<QTouchEvent::TouchPoint> touchPoints;
   int id1;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEPINCHAREA_P_H
